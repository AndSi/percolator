/*******************************************************************************
 Copyright 2006-2012 Lukas Käll <lukas.kall@scilifelab.se>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 *******************************************************************************/

#include "SetHandler.h"

SetHandler::SetHandler() {}

SetHandler::~SetHandler() {
  BOOST_FOREACH (DataSet * subset, subsets) {
    if (subset != NULL) {
      delete subset;
    }
    subset = NULL;
  }
}

/**
 * Gets the vector index of the DataSet matching the label
 * @param label DataSet label
 * @return index of matching DataSet
 */
unsigned int SetHandler::getSubsetIndexFromLabel(int label) {
  for (unsigned int ix = 0; ix < subsets.size(); ++ix) {
    if (subsets[ix]->getLabel() == label) return ix;
  }
  ostringstream temp;
  temp << "Error: No DataSet found with label " << label << std::endl;
  throw MyException(temp.str());
}

/**
 * Insert DataSet object into this SetHandler
 * @param ds pointer to DataSet to be inserted
 */
void SetHandler::push_back_dataset( DataSet * ds ) {
  subsets.push_back(ds);
}

/**
 * Prints the results to a stream
 * @param test Scores object to be printed
 * @param myout stream to be printed to
 */
void SetHandler::print(Scores& test, int label, ostream& myout) {
  vector<ResultHolder> outList(0);
  BOOST_FOREACH (DataSet * subset, subsets) {
    if (subset->getLabel() == label) {
      subset->print(test, outList);
    }
  }
  sort(outList.begin(), outList.end(), std::greater<ResultHolder> ());
  myout
      << "PSMId\tscore\tq-value\tposterior_error_prob\tpeptide\tproteinIds"
      << endl;
  BOOST_FOREACH (const ResultHolder & psmResult, outList) {
    myout << psmResult << endl;
  }
}

void SetHandler::fillFeatures(vector<ScoreHolder> &scores, int label) {
  subsets[getSubsetIndexFromLabel(label)]->fillFeatures(scores);
}

void SetHandler::fillFeaturesPeptide(vector<ScoreHolder> &scores, int label) {
  subsets[getSubsetIndexFromLabel(label)]->fillFeaturesPeptide(scores);
}

/*const double* SetHandler::getFeatures(const int setPos, const int ixPos) const {
  return subsets[setPos]->getFeatures(ixPos);
}*/

int const SetHandler::getLabel(int setPos) {
  assert(setPos >= 0 && setPos < (signed int)subsets.size());
  return subsets[setPos]->getLabel();
}

int SetHandler::readTab(const string& dataFN, SanityCheck *& pCheck) {
  if (VERB > 1) {
    cerr << "Reading Tab delimited input from datafile " << dataFN << endl;
  }
  
  ifstream dataStream(dataFN.c_str(), ios::in);
  if (!dataStream) {
    ostringstream temp;
    temp << "ERROR: Can not open file " << dataFN << endl;
    throw MyException(temp.str());
  }
  string tmp, psmid, line;
  
  // First we need to find out the number of features
  getline(dataStream, line); // skip feature name line
  getline(dataStream, line);
  istringstream iss(line);
  iss >> psmid >> tmp >> tmp; // read id, label and scannr of second row
  
  // check if first row contains the default weights
  bool hasInitialValues = false;
  if (psmid == "DefaultDirection") { 
    hasInitialValues = true;
    getline(dataStream, line);
    iss.str(line);
    iss >> tmp >> tmp >> tmp; // remove id, label and scannr
  }
  
  // count number of features from first PSM
  double a;
  unsigned int numFeatures = 0;
  iss >> a; // test fourth column
  while (iss.good()) {
    ++numFeatures;
    iss >> a;
  }
  iss.clear(); // clear the error bit
  if (DataSet::getCalcDoc()) numFeatures -= 2;
  
  // read from start again
  dataStream.seekg(0, std::ios::beg);
  
  // fill in the feature names from the first line
  getline(dataStream, line); // save row with feature names for later parsing
  iss.str(line);
  FeatureNames& featureNames = DataSet::getFeatureNames();
  int skip = (DataSet::getCalcDoc() ? 2 : 0);
  int numFeatLeft = static_cast<int>(numFeatures);
  while (iss.good()) {
    iss >> tmp;
    if (skip-- <= -3 && numFeatLeft-- > 0) { // removes enumerator, label, scannr and, if present, DOC features
      featureNames.insertFeature(tmp);
    }
  }
  iss.clear(); // clear the error bit
  
  featureNames.initFeatures(DataSet::getCalcDoc());
  assert(numFeatures == DataSet::getNumFeatures());
  
  // fill in the default weights if present
  std::vector<double> init_values;
  if (hasInitialValues) {
    getline(dataStream, line);
    iss.str(line);
    iss >> tmp >> tmp >> tmp; // remove id, label and scannr
    unsigned int ix = 0;
    while (iss.good()) {
      iss >> a;
      if (VERB > 2) {
        std::cerr << "Initial direction for " << DataSet::getFeatureNames().getFeatureName(ix) << " is " << a << std::endl;
      }
      init_values.push_back(a);
      ix++;
    }
    iss.clear(); // clear the error bit
  }
  
  if (numFeatures < 1) {
    dataStream.close();
    throw MyException("ERROR: Reading tab file, too few features present.");
  } else if (hasInitialValues && init_values.size() > numFeatures) {
    dataStream.close();
    throw MyException("ERROR: Reading tab file, too many default values present.");
  }
  
  DataSet * targetSet = new DataSet();
  assert(targetSet);
  targetSet->setLabel(1);
  DataSet * decoySet = new DataSet();
  assert(decoySet);
  decoySet->setLabel(-1);

  // read in the data
  int label;
  while (getline(dataStream, line)) {
    iss.str(line);
    iss >> tmp >> label;
    if (label == 1) {
      targetSet->readPsm(dataStream, line);
    } else if (label == -1) {
      decoySet->readPsm(dataStream, line);
    }
  }
  dataStream.close();
  
  push_back_dataset(targetSet);
  push_back_dataset(decoySet);
  
  pCheck = new SanityCheck();
  pCheck->checkAndSetDefaultDir();
  if (hasInitialValues) pCheck->addDefaultWeights(init_values);
  return 1;
}

void SetHandler::writeTab(const string& dataFN, SanityCheck * pCheck) {
  ofstream dataStream(dataFN.data(), ios::out);
  dataStream << "SpecId\tLabel\tScanNr\t";
  if (DataSet::getCalcDoc()) {
    dataStream << "RT\tdM\t";
  }
  dataStream << DataSet::getFeatureNames().getFeatureNames(true)
      << "\tPeptide\tProteins" << std::endl;
  vector<double> initial_values = pCheck->getDefaultWeights();
  if (initial_values.size() > 0) {
    dataStream << "DefaultDirection\t-\t-";
    BOOST_FOREACH (const double iv, initial_values) {
      dataStream << '\t' << iv;
    }
    dataStream << std::endl;
  }
  BOOST_FOREACH (DataSet * subset, subsets) {
    subset->writeTabData(dataStream);
  }
  dataStream.close();
}
