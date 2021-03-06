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

#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
using namespace std;
#include "DataSet.h"
#include "Normalizer.h"
#include "SetHandler.h"
#include "Scores.h"
#include "Globals.h"
#include "PosteriorEstimator.h"
#include "ssl.h"
#include "MassHandler.h"


inline bool operator>(const ScoreHolder& one, const ScoreHolder& other) {
  return (one.score > other.score);
}

inline bool operator<(const ScoreHolder& one, const ScoreHolder& other) {
  return (one.score < other.score);
}

inline double truncateTo(double truncateMe, const char* length) {
  char truncated[64];
  char format[64];
  strcpy(format,"%.");
  strcat(format,length);
  strcat(format,"lf\n");
  sprintf(truncated, format, truncateMe);
  return atof(truncated);
}

ostream& operator<<(ostream& os, const ScoreHolder& sh) {
  if (sh.isDecoy() && !Scores::isOutXmlDecoys()) {
    return os;
  }
  
  os << "    <psm p:psm_id=\"" << sh.pPSM->id << "\"";
  if (Scores::isOutXmlDecoys()) {
    if (sh.isDecoy())
      os << " p:decoy=\"true\"";
    else 
      os << " p:decoy=\"false\"";
  }
  os << ">" << endl;
  
  os << "      <svm_score>"   << fixed 	<< sh.score 	<< "</svm_score>" << endl;
  os << "      <q_value>" 	<< scientific << sh.pPSM->q 	<< "</q_value>" << endl;
  os << "      <pep>" 	       << scientific << sh.pPSM->pep << "</pep>" << endl;
  
  if(Scores::getShowExpMass()) 
  {
    os << "      <exp_mass>" << fixed << setprecision (4) << sh.pPSM->expMass << "</exp_mass>" << endl;
  }   
  
  os << "      <calc_mass>" << fixed << setprecision (3) << sh.pPSM->calcMass << "</calc_mass>" << endl;
  
  if (DataSet::getCalcDoc()) os << "      <retentionTime observed=\"" 
				  << PSMDescription::unnormalize(sh.pPSM->retentionTime)
				  << "\" predicted=\""
				  << PSMDescription::unnormalize(sh.pPSM->predictedTime) << "\"/>"
				  << endl;

  if (sh.pPSM->getPeptideSequence().size() > 0) {
	  string n = sh.pPSM->getFlankN();
	  string c = sh.pPSM->getFlankC();
	  string centpep = sh.pPSM->getPeptideSequence();
	  os << "      <peptide_seq n=\"" << n << "\" c=\"" << c << "\" seq=\"" << centpep << "\"/>" << endl;
  }
  
  BOOST_FOREACH (const std::string & pid, sh.pPSM->proteinIds) {
    os << "      <protein_id>" << getRidOfUnprintablesAndUnicode(pid) << "</protein_id>" << endl;
  }
  
  os << "      <p_value>" << scientific << sh.pPSM->p << "</p_value>" <<endl;
  os << "    </psm>" << endl;
  return os;
}

ostream& operator<<(ostream& os, const ScoreHolderPeptide& sh) {
  if (sh.isDecoy() && !Scores::isOutXmlDecoys()) {
    return os;
  }
  
  os << "    <peptide p:peptide_id=\"" << sh.pPSM->getPeptideSequence() << "\"";
  if (Scores::isOutXmlDecoys()) {
    if (sh.isDecoy())
      os << " p:decoy=\"true\"";
    else 
      os << " p:decoy=\"false\"";
  }
  os << ">" << endl;
  
  os << "      <svm_score>" << fixed       << sh.score     << "</svm_score>" << endl;
  os << "      <q_value>"   << scientific  << sh.pPSM->q   << "</q_value>" << endl;
  os << "      <pep>" 	     << scientific  << sh.pPSM->pep << "</pep>" << endl;
  
  if(Scores::getShowExpMass()) 
  {
    os << "      <exp_mass>" << fixed << setprecision (4) << sh.pPSM->expMass << "</exp_mass>" << endl;
  }
  os << "      <calc_mass>" << fixed << setprecision (3)  << sh.pPSM->calcMass << "</calc_mass>" << endl;
  
  BOOST_FOREACH (const std::string &pid, sh.pPSM->proteinIds) {
    os << "      <protein_id>" << getRidOfUnprintablesAndUnicode(pid) << "</protein_id>" << endl;
  }
  
  os << "      <p_value>" << scientific << sh.pPSM->p << "</p_value>" <<endl;
  os << "      <psm_ids>" << endl;
  
  // output all psms that contain the peptide
  BOOST_FOREACH (const std::string &psm, sh.psms_list){
    os << "        <psm_id>" << psm << "</psm_id>" << endl;
  }
  os << "      </psm_ids>" << endl;
  os << "    </peptide>" << endl;
  
  return os;
}

bool Scores::outxmlDecoys = false;
bool Scores::showExpMass = false;
unsigned long Scores::seed = 1;

Scores::Scores() {
  pi0 = 1.0;
  targetDecoySizeRatio = 1;
  totalNumberOfDecoys = 0;
  totalNumberOfTargets = 0;
  posNow = 0;
}

Scores::~Scores() {}

void Scores::merge(vector<Scores>& sv, double fdr, bool computePi0) {
  scores.clear();
  for (vector<Scores>::iterator a = sv.begin(); a != sv.end(); a++) {
    sort(a->begin(), a->end(), greater<ScoreHolder> ());
    a->estimatePi0();
    a->calcQ(fdr);
    a->normalizeScores(fdr);
    copy(a->begin(), a->end(), back_inserter(scores));
  }
  sort(scores.begin(), scores.end(), greater<ScoreHolder> ());
  totalNumberOfDecoys = count_if(scores.begin(),
      scores.end(),
      mem_fun_ref(&ScoreHolder::isDecoy));
  totalNumberOfTargets = count_if(scores.begin(),
      scores.end(),
      mem_fun_ref(&ScoreHolder::isTarget));
  targetDecoySizeRatio = totalNumberOfTargets / max(1.0, (double)totalNumberOfDecoys);
  if(computePi0) estimatePi0();
  else pi0 = 1.0;
}

void Scores::printRetentionTime(ostream& outs, double fdr) {
  BOOST_FOREACH (ScoreHolder &sh, scores) {
    if (sh.isTarget()) outs
        << PSMDescription::unnormalize(sh.pPSM->retentionTime) << "\t"
        << PSMDescription::unnormalize(doc.estimateRT(sh.pPSM->retentionFeatures))
    << "\t" << sh.pPSM->peptide << endl;
  }
}

double Scores::calcScore(const double* feat) const {
  register int ix = FeatureNames::getNumFeatures();
  register double score = w_vec[ix];
  for (; ix--;) {
    score += feat[ix] * w_vec[ix];
  }
  return score;
}

/**
 * Returns the ScoreHolder object associated with a list of features
 * @param d array of features
 * @return pointer to ScoreHolder
 */
ScoreHolder* Scores::getScoreHolder(const double* d) {
  if (scoreMap.size() == 0) {
    BOOST_FOREACH (ScoreHolder &sh, scores) {
      scoreMap[sh.pPSM->features] = &sh;
    }
  }
  std::map<const double*, ScoreHolder*>::iterator res = scoreMap.find(d);
  if (res != scoreMap.end()) {
    return res->second;
  }
  return NULL;
}

void Scores::fillFeatures(SetHandler& setHandler, bool reportUniquePeptides) {
  scores.clear();
  if(reportUniquePeptides){ // if unique peptides
    setHandler.fillFeaturesPeptide(scores,1);
    setHandler.fillFeaturesPeptide(scores,-1);
  } else {
    setHandler.fillFeatures(scores,1);
    setHandler.fillFeatures(scores,-1);
  }
  totalNumberOfTargets = setHandler.getSizeFromLabel(1);
  totalNumberOfDecoys = setHandler.getSizeFromLabel(-1);
  targetDecoySizeRatio = (double)totalNumberOfTargets / totalNumberOfDecoys;
}

// Park–Miller random number generator
// from wikipedia
unsigned long Scores::lcg_rand() {
  //uint64_t
  seed = (seed * 279470273) % 4294967291;
  return seed;
}

/**
 * Divides the PSMs from pin file into xval_fold cross-validation sets
 * @param train vector containing the training sets of PSMs
 * @param test vector containing the test sets of PSMs
 * @param xval_fold number of folds in train and test
 */
void Scores::createXvalSets(vector<Scores>& train, vector<Scores>& test,
    const unsigned int xval_fold) {
  train.resize(xval_fold);
  test.resize(xval_fold);
  vector<size_t> remain(xval_fold);
  size_t fold = xval_fold, ix = scores.size();
  while (fold--) {
    remain[fold] = ix / (fold + 1);
    ix -= remain[fold];
  }
  for (unsigned int j = 0; j < scores.size(); j++) {
    ix = lcg_rand() % (scores.size() - j);
    fold = 0;
    while (ix > remain[fold]) {
      ix -= remain[fold++];
    }
    for (unsigned int i = 0; i < xval_fold; i++) {
      if (i == fold) {
        test[i].scores.push_back(scores[j]);
      } else {
        train[i].scores.push_back(scores[j]);
      }
    }
    --remain[fold];
  }
  // calculate ratios of target over decoy for train and test set
  for (unsigned int i = 0; i < xval_fold; i++) {
    train[i].recalculateSizes();
    test[i].recalculateSizes();
  }
}

/**
 * Divides the PSMs from pin file into xval_fold cross-validation sets based on
 * their spectrum scan number
 * @param train vector containing the training sets of PSMs
 * @param test vector containing the test sets of PSMs
 * @param xval_fold: number of folds in train and test
 */
void Scores::createXvalSetsBySpectrum(vector<Scores>& train, vector<Scores>&
    test, const unsigned int xval_fold) {
  // set the number of cross validation folds for train and test to xval_fold
  train.resize(xval_fold);
  test.resize(xval_fold);
  // remain keeps track of residual space available in each fold
  vector<size_t> remain(xval_fold);
  // set values for remain: initially each fold is assigned (tot number of
  // scores / tot number of folds)
  size_t fold = xval_fold, ix = scores.size();
  while (fold--) {
    remain[fold] = ix / (fold + 1);
    ix -= remain[fold];
  }

  // store possible spectra with relative scores
  multimap<unsigned int,ScoreHolder> spectraScores;
  // populate spectraScores
  BOOST_FOREACH (ScoreHolder &sh, scores) {
    spectraScores.insert(pair<unsigned int,ScoreHolder>(sh.pPSM->scan, sh));
  }

  // put scores into the folds; choose a fold (at random) and change it only
  // when scores from a new spectra are encountered
  // note: this works because multimap is an ordered container!
  unsigned int previousSpectrum = spectraScores.begin()->first;
  size_t randIndex = lcg_rand() % xval_fold;
  for (multimap<unsigned int, ScoreHolder>::iterator it = spectraScores.begin(); 
        it != spectraScores.end(); ++it) {
    const unsigned int curScan = (*it).first;
    const ScoreHolder sh = (*it).second;
    // if current score is from a different spectra than the one encountered in
    // the previous iteration, choose new fold
    
    //NOTE what if the fold is full but previousSpectrum is the same as current spectrum??
    if(previousSpectrum != curScan){
      randIndex = lcg_rand() % xval_fold;
      // allow only indexes of folds that are non-full
      while(remain[randIndex] == 0){
        randIndex = lcg_rand() % xval_fold;
      }
    }
    // insert
    for (unsigned int i = 0; i < xval_fold; i++) {
      if (i == randIndex) {
        test[i].scores.push_back(sh);
      } else {
        train[i].scores.push_back(sh);
      }
    }
    // update number of free position for used fold
    --remain[randIndex];
    // set previous spectrum to current one for next iteration
    previousSpectrum = curScan;
  }

  // calculate ratios of target over decoy for train and test set
  for (unsigned int i = 0; i < xval_fold; i++) {
    train[i].recalculateSizes();
    test[i].recalculateSizes();
  }
}

void Scores::recalculateSizes() {
  totalNumberOfTargets = 0;
  totalNumberOfDecoys = 0;
  BOOST_FOREACH (const ScoreHolder &sh, scores) {
    if (sh.isTarget()) {
      ++totalNumberOfTargets;
    } else {
      ++totalNumberOfDecoys;
    }
  }
  targetDecoySizeRatio = totalNumberOfTargets / (double)totalNumberOfDecoys;
}
    

void Scores::normalizeScores(double fdr) {
  // sets q=fdr to 0 and the median decoy to -1, linear transform the rest to fit
  unsigned int medianIndex = std::max(0u,totalNumberOfDecoys/2u),decoys=0u;
  vector<ScoreHolder>::iterator it = scores.begin();
  double q1 = it->score;
  double median = q1 + 1.0;

  for (; it != scores.end(); ++it) {
    if (it->pPSM->q < fdr)
      q1 = it->score;
    if (it->isDecoy()) {
      if(++decoys==medianIndex) {
        median = it->score;
        break;
      }
    }
  }
  //NOTE perhaps I should also check when q1 and median are both negatives
  //NOTE in such cases the normalization could give negative scores which would
  //     cause an assertion to fail in qvality
  if (q1 <= median || it == scores.end()) {
    ostringstream temp;
    temp << "Error : the input data has too good separation between target "
         << "and decoy PSMs.\n" << std::endl;
    throw MyException(temp.str());
  }
   
  double diff = q1-median;
  BOOST_FOREACH (ScoreHolder &sh, scores) {
    sh.score -= q1;
    sh.score /= diff;
    if (sh.score <= 0 && VERB > 3) { // Why do we warn for this, it happens for most of the data
      std::cerr << "\nWARNING the score of the PSM " << sh.pPSM->id << " is less or equal than zero "
	         << "after normalization.\n" << std::endl;
    }
  }
  
}

/**
 * Calculates the SVM cost/score of each PSM and sorts them
 * @param w normal vector used for SVM cost
 * @param fdr FDR threshold specified by user (default 0.01)
 * @return number of true positives
 */
int Scores::calcScores(vector<double>& w, double fdr) {
  w_vec = w;
  unsigned int ix;
  BOOST_FOREACH (ScoreHolder &sh, scores) {
    sh.score = calcScore(sh.pPSM->features);
  }
  sort(scores.begin(), scores.end(), greater<ScoreHolder> ());
  if (VERB > 3) {
    cerr << "10 best scores and labels" << endl;
    for (ix = 0; ix < 10; ix++) {
      cerr << scores[ix].score << " " << scores[ix].label << endl;
    }
    cerr << "10 worst scores and labels" << endl;
    for (ix = scores.size() - 10; ix < scores.size(); ix++) {
      cerr << scores[ix].score << " " << scores[ix].label << endl;
    }
  }
  return calcQ(fdr);
}

/**
 * Calculates the q-value for each psm in scores: the q-value is the minimal
 * FDR of any set that includes the particular psm
 * @param fdr FDR threshold specified by user (default 0.01)
 * @return number of true positives
 */
int Scores::calcQ(double fdr) {
  assert(totalNumberOfDecoys+totalNumberOfTargets==size());

  int targets = 0, decoys = 0;
  double efp = 0.0, q; // estimated false positives, q value
  
  // NOTE check this
  BOOST_FOREACH (ScoreHolder &sh, scores) {
    if (sh.isTarget()) {
      targets++;
      sh.pPSM->p = (decoys+(double)1)/(totalNumberOfDecoys+(double)1);
    } else {
      decoys++;
      efp = pi0 * decoys * targetDecoySizeRatio;
      sh.pPSM->p = (decoys)/(double)(totalNumberOfDecoys);
    }
    if (targets) {
      q = efp / (double)targets;
    } else {
      q = pi0;
    }
    if (q > pi0) {
      q = pi0;
    }
    sh.pPSM->q = q;
    if (fdr >= q) {
      posNow = targets;
    }
  }
  for (int ix = scores.size(); --ix;) {
    if (scores[ix - 1].pPSM->q > scores[ix].pPSM->q) {
      scores[ix - 1].pPSM->q = scores[ix].pPSM->q;
    }
  }
  return posNow;
}

void Scores::generateNegativeTrainingSet(AlgIn& data, const double cneg) {
  unsigned int ix2 = 0;
  BOOST_FOREACH (const ScoreHolder &sh, scores) {
    if (sh.isDecoy()) {
      data.vals[ix2] = sh.pPSM->features;
      data.Y[ix2] = -1;
      data.C[ix2++] = cneg;
    }
  }
  data.negatives = ix2;
}

void Scores::generatePositiveTrainingSet(AlgIn& data, const double fdr,
    const double cpos) {
  unsigned int ix2 = data.negatives, p = 0;
  BOOST_FOREACH (const ScoreHolder &sh, scores) {
    if (sh.isTarget()) {
      if (fdr < sh.pPSM->q) {
        posNow = p;
        break;
      }
      data.vals[ix2] = sh.pPSM->features;
      data.Y[ix2] = 1;
      data.C[ix2++] = cpos;
      ++p;
    }
  }
  data.positives = p;
  data.m = ix2;
}

/**
 * Routine that sees to that only unique peptides are kept (used for analysis
 * on peptide-fdr rather than psm-fdr)
 */
void Scores::weedOutRedundant(bool computePi0) {
  
   // lexicographically order the scores (based on peptides names,labels and scores)
   std::sort(scores.begin(), scores.end(), lexicOrderProb());
   
   /*
    * much faster and simpler version but it does not fill up psms_list     
    * which will simply iterate over the unique peptides and the removed list many times 
    * scores.erase(std::unique(scores.begin(), scores.end(), mycmp), scores.end());
   */
   
   //NOTE the weed out PSMs might nobe cleaned at the end
   
   vector<ScoreHolder> uniquePeptideScores = vector<ScoreHolder>();
   string previousPeptide;
   int previousLabel = 0;
   // run a pointer down the scores list
   vector<ScoreHolder>::iterator current = scores.begin();
   for(;current!=scores.end(); current++){
     // compare pointer's peptide with previousPeptide
     string currentPeptide = current->pPSM->getPeptideSequence();
     if(currentPeptide.compare(previousPeptide) == 0 
       && (previousLabel == current->label)) {
       // if the peptide is a duplicate
       vector<ScoreHolder>::iterator last = --uniquePeptideScores.end();  
       // append the duplicate psm_id
       last->psms_list.push_back(current->pPSM->id);
     } else {
       // otherwise insert as a new score
       current->psms_list.push_back(current->pPSM->id);
       uniquePeptideScores.push_back(*current);
       // update previousPeptide
       previousPeptide = currentPeptide;
       previousLabel = current->label;
     }
   }
   
   scores = uniquePeptideScores;
   sort(scores.begin(), scores.end(), greater<ScoreHolder> ());
   
   totalNumberOfDecoys = count_if(scores.begin(),
      scores.end(),
      mem_fun_ref(&ScoreHolder::isDecoy));
   totalNumberOfTargets = count_if(scores.begin(),
      scores.end(),
      mem_fun_ref(&ScoreHolder::isTarget));
   targetDecoySizeRatio = totalNumberOfTargets / max(1.0, (double)totalNumberOfDecoys);
   
   if(computePi0) estimatePi0();
   else pi0 = 1.0;
}

/**
 * Routine that sees to that only unique spectra are kept for TDC
 */
void Scores::weedOutRedundantTDC(bool computePi0) {
  
   // order the scores (based on spectra id and scores)
   std::sort(scores.begin(), scores.end(), OrderScanMassCharge());
   
   /*
    * much faster and simpler version but it does not fill up psms_list     
    * which will simply iterate over the unique peptides and the removed list many times 
    * scores.erase(std::unique(scores.begin(), scores.end(), mycmp), scores.end());
   */
   
   vector<ScoreHolder> uniquePSMs = vector<ScoreHolder>();
   unsigned previousSpectra = 0;
   unsigned previousCharge = 0;
   double previousExpMass = 0.0;
   //int previousLabel;
   // run a pointer down the scores list
   vector<ScoreHolder>::iterator current = scores.begin();
   for(;current!=scores.end(); current++){
     // compare pointer's spectra with previous spectra
     unsigned currentSpectra = current->pPSM->scan;
     unsigned currentCharge = current->pPSM->charge;
     double currentExpMass = current->pPSM->expMass;
     //int currentLabel = current->label;
     if(currentSpectra == previousSpectra && previousCharge == currentCharge 
       && previousExpMass == currentExpMass) {
       // if the spectra is duplicate
     } else {
       // otherwise keep it
       uniquePSMs.push_back(*current);
       previousSpectra = currentSpectra;
       previousCharge = currentCharge;
       previousExpMass = currentExpMass;
     }
   }
   scores = uniquePSMs;
   sort(scores.begin(), scores.end(), greater<ScoreHolder> ());
   totalNumberOfDecoys = count_if(scores.begin(),
      scores.end(),
      mem_fun_ref(&ScoreHolder::isDecoy));
   totalNumberOfTargets = count_if(scores.begin(),
      scores.end(),
      mem_fun_ref(&ScoreHolder::isTarget));
   targetDecoySizeRatio = totalNumberOfTargets / max(1.0, (double)totalNumberOfDecoys);
   if(computePi0) estimatePi0();
   else pi0 = 1.0;
}

void Scores::recalculateDescriptionOfCorrect(const double fdr) {
  doc.clear();
  BOOST_FOREACH (const ScoreHolder &sh, scores) {
    if (sh.isTarget()) {
      //      if (fdr>scores[ix1].pPSM->q) {
      if (0.0 >= sh.pPSM->q) {
        doc.registerCorrect(*(sh.pPSM));
      }
    }
  }
  doc.trainCorrect();
  setDOCFeatures();
}

void Scores::setDOCFeatures() {
  BOOST_FOREACH (const ScoreHolder &sh, scores) {
    doc.setFeatures(*(sh.pPSM));
  }
}

int Scores::getInitDirection(const double fdr, vector<double>& direction) {
  int bestPositives = -1;
  int bestFeature = -1;
  bool lowBest = false;
  for (unsigned int featNo = 0; featNo < FeatureNames::getNumFeatures(); featNo++) {
    BOOST_FOREACH (ScoreHolder &sh, scores) {
      sh.score = sh.pPSM->features[featNo];
    }
    sort(scores.begin(), scores.end());
    // check once in forward direction (high scores are good) and once in backward
    for (int i = 0; i < 2; i++) {
      int positives = 0, decoys = 0;
      double efp = 0.0, q;
      BOOST_FOREACH (ScoreHolder &sh, scores) {
        if (sh.isTarget()) {
          positives++;
        } else {
          decoys++;
          efp = pi0 * decoys * targetDecoySizeRatio;
        }
        if (positives) {
          q = efp / (double)positives;
        } else {
          q = pi0;
        }
        if (fdr <= q) {
          if (positives > bestPositives && scores.begin()->score != sh.score) {
            bestPositives = positives;
            bestFeature = featNo;
            lowBest = (i == 0);
          }
          if (i == 0) {
            reverse(scores.begin(), scores.end());
          }
          break;
        }
      }
    }
  }
  for (int ix = FeatureNames::getNumFeatures(); ix--;) {
    direction[ix] = 0;
  }
  direction[bestFeature] = (lowBest ? -1 : 1);
  if (VERB > 1) {
    cerr << "Selected feature number " << bestFeature + 1
        << " as initial search direction, could separate "
        << bestPositives << " positives in that direction" << endl;
  }
  return bestPositives;
}

double Scores::estimatePi0() {
  vector<pair<double, bool> > combined;
  vector<double> pvals;
  transform(scores.begin(),
      scores.end(),
      back_inserter(combined),
      mem_fun_ref(&ScoreHolder::toPair));
  // Estimate pi0
  PosteriorEstimator::getPValues(combined, pvals);
  pi0 = PosteriorEstimator::estimatePi0(pvals);
  return pi0;
}

void Scores::calcPep() {
  vector<pair<double, bool> > combined;
  transform(scores.begin(),
      scores.end(),
      back_inserter(combined),
      mem_fun_ref(&ScoreHolder::toPair));
  vector<double> peps;
  // Logistic regression on the data
  PosteriorEstimator::estimatePEP(combined, pi0, peps, true);
  for (size_t ix = 0; ix < scores.size(); ix++) {
    (scores[ix]).pPSM->pep = peps[ix];
  }
}

unsigned Scores::getQvaluesBelowLevel(double level) {
  unsigned hits = 0;
  BOOST_FOREACH (ScoreHolder &sh, scores) {
    if (sh.isTarget() && sh.pPSM->q < level) {
      hits++;
    }
  }
  return hits;
}
