#include "tandemReader.h"

static const XMLCh groupStr[] = { chLatin_g, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chNull};
static const XMLCh groupTypeStr[] = { chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull};
static const XMLCh groupModelStr[] = { chLatin_m, chLatin_o, chLatin_d, chLatin_e, chLatin_l, chNull};
string schemaDefinition = TANDEM_SCHEMA_LOCATION + string("tandem2011.12.01.1.xsd");
string scheme_namespace = TANDEM_NAMESPACE;
string schema_major = "";
string schema_minor = "";

tandemReader::tandemReader(ParseOptions po):Reader(po)
{
  
}

tandemReader::~tandemReader()
{

}


//Some functions to handle strings
std::vector<std::string> &tandemReader::split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
      if(item.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_#@*?")!=std::string::npos){
	elems.push_back(item); 
      }
    }
    return elems;
}

std::vector<std::string>tandemReader::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}





xercesc::DOMElement*
add_namespace2 (xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc,
               xercesc::DOMElement* e,
               const XMLCh* ns, int lvl)
{
  using namespace xercesc;

  //DOMElement* ne =static_cast<DOMElement*> (doc->renameNode (e, ns, e->getLocalName ()));
  DOMElement* ne=e;
  std::cerr << "Tag: " << XMLString::transcode(e->getTagName()) << std::endl;

  for (DOMNode* n = ne->getFirstChild (); n != 0; n = n->getNextSibling ())
  {
    if (n->getNodeType () == DOMNode::ELEMENT_NODE)
    {
      n = add_namespace2 (doc, static_cast<DOMElement*> (n), ns, 1);
    }
  }

  return ne;
}

xercesc::DOMElement*
add_namespace2 (xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc,
               xercesc::DOMElement* e,
               const std::string& ns, int lvl)
{
  return add_namespace2 (doc, e, xsd::cxx::xml::string (ns).c_str (), 1);
}





void add_namespace3 (xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc,
               xercesc::DOMElement* e,
               const XMLCh* ns, int lvl)
{
  std::cerr << "Lvl: " << lvl << " New func call" << std::endl;
  int nlvl=lvl+1;
  
  DOMElement* ne;
  XMLCh* GAML_XMLCH=XMLString::transcode("www.bioml.com/gaml/");
  std::cerr << "Lvl: " << lvl << " Tag: ";
  std::cerr << XMLString::transcode(e->getTagName()) << std::endl;
  std::cerr << "Lvl: " << lvl << " Doc tag: ";
  std::cerr << std::endl;
  std::cerr << XMLString::transcode(doc->getDocumentElement()->getTagName()) << std::endl;
  
  /*
  if(XMLString::equals(e->getNamespaceURI(),GAML_XMLCH))//Check if gaml namespace FIXME
  {
    ne=e;
  }else
  {
    //ne =static_cast<DOMElement*> (doc->renameNode (e, ns, e->getLocalName ()));
    ne=e;
  }*/
  ne=e;
 
  std::cerr << "Lvl: " << lvl << " For" << std::endl;
  DOMNodeList* childList=ne->getChildNodes();
 
  for(int iter=0; iter<childList->getLength();iter++)
  {
    std::cerr << "Lvl: " << lvl << " For iter" << std::endl;
    if (childList->item(iter)->getNodeType () == DOMNode::ELEMENT_NODE)
    {
      std::cerr << "Lvl: " << lvl << " If iter" << std::endl;
      add_namespace3 (doc, static_cast<DOMElement*> (childList->item(iter)), ns, nlvl);
      //add_namespace (doc, static_cast<DOMElement*> (n), ns, nlvl);
      std::cerr << "Lvl: " << lvl << " If Tag iter ";
      std::cerr << XMLString::transcode(childList->item(iter)->getLocalName()) << std::endl; //TMP
    }
  }
  
  XMLString::release(&GAML_XMLCH); 
  
  std::cerr << "Lvl: " << lvl << " End" << std::endl;
}


void add_namespace3 (xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc,
               xercesc::DOMElement* e,
               const std::string& ns, int lvl)
{
  add_namespace3 (doc, e, xsd::cxx::xml::string (ns).c_str (),lvl);
}
















//TODO fix this and add to interface
xercesc::DOMElement* add_namespace (xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc,
               xercesc::DOMElement* e,
               const XMLCh* ns, int lvl)
{
  std::cerr << "Lvl: " << lvl << " New func call" << std::endl;
  int nlvl=lvl+1;
  
  DOMElement* ne;
  XMLCh* GAML_XMLCH=XMLString::transcode("www.bioml.com/gaml/");
  std::cerr << "Lvl: " << lvl << " Tag: ";
  std::cerr << XMLString::transcode(e->getTagName()) << std::endl;
  std::cerr << "Lvl: " << lvl << " Doc tag: ";
  std::cerr << std::endl;
  std::cerr << XMLString::transcode(doc->getDocumentElement()->getTagName()) << std::endl;
  
  /*
  if(XMLString::equals(e->getNamespaceURI(),GAML_XMLCH))//Check if gaml namespace FIXME
  {
    ne=e;
  }else
  {
    //ne =static_cast<DOMElement*> (doc->renameNode (e, ns, e->getLocalName ()));
    ne=e;
  }*/
  ne=e;
 
  std::cerr << "Lvl: " << lvl << " For" << std::endl;
  for (DOMNode* n = ne->getFirstChild (); n != NULL; n = n->getNextSibling ())
  {
    std::cerr << "Lvl: " << lvl << " For iter" << std::endl;
    if (n->getNodeType () == DOMNode::ELEMENT_NODE)
    {
      std::cerr << "Lvl: " << lvl << " If iter" << std::endl;
      n = add_namespace (doc, static_cast<DOMElement*> (n), ns, nlvl);
      //add_namespace (doc, static_cast<DOMElement*> (n), ns, nlvl);
      std::cerr << "Lvl: " << lvl << " If Tag iter ";
      std::cerr << XMLString::transcode(n->getLocalName()) << std::endl; //TMP
    }
  }
  
  XMLString::release(&GAML_XMLCH); 
  
  std::cerr << "Lvl: " << lvl << " End" << std::endl;
  return ne;
}


xercesc::DOMElement* add_namespace (xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc,
               xercesc::DOMElement* e,
               const std::string& ns, int lvl)
{
  return add_namespace (doc, e, xsd::cxx::xml::string (ns).c_str (),lvl);
}


bool tandemReader::checkValidity(const std::string file)
{
  bool isvalid;
  std::ifstream fileIn(file.c_str(), std::ios::in);
  if (!fileIn) 
  {
    std::cerr << "Could not open file " << file << std::endl;
    exit(-1);
  }
  std::string line,line_xml,line_bioml;
  if (!getline(fileIn, line)) 
  {
    std::cerr << "Could not read file " << file << std::endl;
    exit(-1);
  }
  
  if (line.find("<?xml") != std::string::npos)
  {
    getline(fileIn, line_xml);
    getline(fileIn, line_bioml);
    
    if (line_xml.find("tandem") == std::string::npos||line_bioml.find("<bioml") == std::string::npos)
    {
      std::cerr << "XML file not generated by X!tandem: " << file << std::endl;
      exit(-1);
    }else
    {
      isvalid=true;
    }
    if(line_bioml.find("xmlns=\"http://www.thegpm.org/TANDEM/2011.12.01.1\"")!=std::string::npos){ 
      fixedXML=true;
    }else
    {
      fixedXML=false;
    }
  } 
  else
  {
    isvalid = false;
  }

  fileIn.close();
  return isvalid;
}

bool tandemReader::checkIsMeta(std::string file)
{
  bool ismeta;
  std::string line;
  std::ifstream fileIn(file.c_str(), std::ios::in);
  getline(fileIn, line);
  
  if (line.find("<?xml") != std::string::npos)
  {
    ismeta= false;
  } 
  else
  {
    ismeta = true;
  }

  fileIn.close();
  return ismeta;
}


void  tandemReader::addFeatureDescriptions(bool doEnzyme,const std::string& aaAlphabet) //TODO Needs some changes
{
  push_backFeatureDescription("hyperscore");
  push_backFeatureDescription("nextscore");
  push_backFeatureDescription("Mass");
  push_backFeatureDescription("PepLen");
  
  for (int charge = minCharge; charge <= maxCharge; ++charge) 
  {
    std::ostringstream cname;
    cname << "Charge" << charge;
    push_backFeatureDescription(cname.str().c_str());

  }
  if (doEnzyme) 
  {
    push_backFeatureDescription("enzN");
    push_backFeatureDescription("enzC");
    push_backFeatureDescription("enzInt");
  }
  
  //Mass difference
  push_backFeatureDescription("dM");
  push_backFeatureDescription("absdM");
  
  if (po.calcPTMs) 
  {
    push_backFeatureDescription("ptm");
  }
  
  if (po.pngasef) 
  {
    push_backFeatureDescription("PNGaseF");
  }
  
  if (po.calcAAFrequencies)
  {
    for (std::string::const_iterator it = aaAlphabet.begin(); it != aaAlphabet.end(); it++)
    {
      std::string temp = boost::lexical_cast<std::string>(*it)+"-Freq";
      push_backFeatureDescription(temp.c_str());
    }
  }
}

void tandemReader::getMaxMinCharge(const std::string fn){
  
  int nTot = 0, charge = 0;
  
  namespace xml = xsd::cxx::xml;
 
  ifstream ifs;
  ifs.exceptions(ifstream::badbit|ifstream::failbit);
    
  ifs.open(fn.c_str());
  parser p;
  
  try
  {
    //Sending fixedXML as the bool for validation since if its not fixed the namespace has to be added later and then we cant validate the schema and xml file.
    xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc (p.start (ifs, fn.c_str(),fixedXML, schemaDefinition,schema_major, schema_minor, scheme_namespace));
    assert(doc.get());
    
    for (doc = p.next(); doc.get() != 0; doc = p.next ())
    {  
      try{
	//Check that the tag name is group and that its not the inputput parameters
	if(XMLString::equals(groupStr,doc->getDocumentElement()->getTagName()) && XMLString::equals(groupModelStr,doc->getDocumentElement()->getAttribute(groupTypeStr)))  
	{
	  if(!fixedXML)
	  {
	    std::cerr << "Fixing xml file" << std::endl; //TMP
	    add_namespace3 (doc,doc->getDocumentElement(),"http://www.thegpm.org/TANDEM/2011.12.01.1",1);
	  }
	  
	  std::cerr << "Tag_NE: ";
	  std::cerr << XMLString::transcode(doc->getDocumentElement()->getTagName()) << std::endl; //TMP
	  
	  
	  tandem_ns::group groupObj(*doc->getDocumentElement());
	  
	  if(groupObj.z().present()) //We are sure we are not in parameters group so z(the charge) has to be present.
	  {
	    stringstream chargeStream (stringstream::in | stringstream::out);
	    chargeStream << groupObj.z();
	    chargeStream >> charge;
	    if (minCharge > charge) minCharge = charge;
	    if (maxCharge < charge) maxCharge = charge;
	    nTot++;
	  }else
	  {
	    cerr << "Missing charge(attribute z in group element) for one or more groups in: " << fn << endl;
	    exit(1);
	  }
	}
      }catch(exception e)
      {
	cerr << "Problem parsing the codesynthesis object model for the xml file: " << fn << endl;
	cerr << e.what() << endl;
	exit(1);
      }
    }
  }catch (const xml_schema::exception& e)
  {
    cerr << "Problem reading the xml file: " << fn << endl;
    cerr << e << endl;
    exit(1);
  }
  
  if(nTot<=0)
  {
    std::cerr << "The file " << fn << " does not contain any records" << std::endl;
    exit(1);
  }
   
  ifs.close();
  return;
}


void tandemReader::createPSM(const tandem_ns::protein &protObj,bool isDecoy,boost::shared_ptr<FragSpectrumScanDatabase> database,spectraMapType &spectraMap,std::string fn){

  typedef map<std::string,double> protMapType;
  typedef map<std::string,std::string> protMapStringType;
  
  std::ostringstream id;
  std::string fileId, protId, proteinName;
  int rank, spectraId;
  std::vector< std::string > proteinNames;
  protMapType protMap;
  protMapStringType protMapString;
  
  std::auto_ptr< percolatorInNs::features >  features_p( new percolatorInNs::features ());
  percolatorInNs::features::feature_sequence & f_seq =  features_p->feature();
  std::map<char,int> ptmMap = po.ptmScheme; //NOTE not used yet
  
  tandem_ns::protein::peptide_type peptideObj=protObj.peptide();
  tandem_ns::peptide::domain_type domainObj=peptideObj.domain();
	      
  fileId = fn;
  size_t spos = fileId.rfind('/');
  if (spos != std::string::npos) fileId.erase(0, spos + 1);
  spos = fileId.find('.');
  if (spos != std::string::npos) fileId.erase(spos);
  
  
  //Information about the protein that the spectra matched
  protMap["protExpect"]=protObj.expect();	//the log10 value of the expectation value for the protein
  protMap["protSumI"]=protObj.sumI();	//the sum of all of the fragment ions that identify this protein
  protId=protObj.id();	//the identifier for this particular identification (spectrum #).(id #)
  int pos=protId.find('.');
  
  if (pos!=string::npos)
  {
    spectraId=boost::lexical_cast<int>(protId.substr(0,pos));
    rank=boost::lexical_cast<int>(protId.substr(pos+1));
  }
	    
  //Describes the region of the protein’s sequence that was identified.
  protMap["domainExpect"]=domainObj.expect();		//the expectation value for the peptide identification
  protMap["calculatedMass"]=domainObj.mh();		//the calculated peptide mass + a proton
  protMap["massDiff"]=domainObj.delta();		//the spectrum mh minus the calculated mh
  protMap["hyperScore"]=domainObj.hyperscore();	//Tandem’s score for the identification
  protMap["nextScore"]=domainObj.nextscore();
  protMap["yScore"]=domainObj.y_score();
  protMap["yIons"]=domainObj.y_ions();
  protMap["bScore"]=domainObj.b_score();
  protMap["bIons"]=domainObj.b_ions();
  protMapString["domainPre"]=domainObj.pre();		//the four residues preceding the domain
  protMapString["domainPost"]=domainObj.post();	//the four residues following the domain
  protMapString["peptide"]=domainObj.seq();		//the sequence of the domain TODO flanks? is it first in seq or before?
  protMap["missedCleavages"]=domainObj.missed_cleavages();//the number of potential cleavage sites in this peptide sequence.

  //Get absMassDiff
  if(protMap["massDiff"]>0)
  {
    protMap["absMassDiff"]=protMap["massDiff"];
  }
  else
  {
    protMap["absMassDiff"]=-protMap["massDiff"];
  }
  
  BOOST_FOREACH(const tandem_ns::aa &aaObj, domainObj.aa()) //Do something good with aaObj TODO
  {
    //Information about modifications in the peptide
    aaObj.modified();
    aaObj.type();
    aaObj.at();
  }
  
  std::cerr << "Protid: " << protId << std::endl; //TMP
  
  //Create id
  id.str("");
  id << fileId << '_' << spectraId << '_' << spectraMap["charge"] << '_' << rank;
  std::string psmId=id.str();
  
  //Get rid of unprinatables in proteinID and make list of proteinIDs
  proteinName=protObj.label();
  proteinName=getRidOfUnprintables(proteinName);
  proteinNames.push_back(proteinName);
  
  //Adjust isDecoy if combined file
  if(po.iscombined)
  {
    isDecoy = proteinNames.front().find(po.reversedFeaturePattern, 0) != std::string::npos;
  }
  
  //Check length of peptide, if its to short it cant contain both flanks and peptide
  assert(protMapString["peptide"].size() >= 5 );
  
  
  //Push back the main scores
  f_seq.push_back(protMap["hyperScore"]);
  f_seq.push_back(protMap["nextScore"]);

  f_seq.push_back(spectraMap["charge"]);

  //Expect
  f_seq.push_back(protMap["protExpect"]);
  f_seq.push_back(protMap["domainExpect"]);
  
  //Ion related features
  f_seq.push_back(spectraMap["sumI"]);
  f_seq.push_back(protMap["protSumI"]);
  f_seq.push_back(spectraMap["maxI"]);
  
  //TODO calculate ions thing with this:
  //protMap["yScore"]
  //protMap["yIons"]
  //protMap["bScore"]
  //protMap["bIons"]
  
  // per_peptides[p]['ionRatio'] = float(int(peptides[pep]['b_ions'])+int(peptides[pep]['y_ions'])) / float(len(peptides[pep]['seq'])*2)
  
  //Hyperscore constants
  f_seq.push_back(spectraMap["a0"]);
  f_seq.push_back(spectraMap["a1"]);
  
  //Pusback massrelated
  f_seq.push_back(spectraMap["parenIonMass"]);
  f_seq.push_back(protMap["massDiff"]);
  f_seq.push_back(protMap["absMassDiff"]);
  
  //Length of peptide
  f_seq.push_back(protMapString["peptide"].size()); //NOTE atm with or wihtout flanks?
  
  //TODO Missed cleavages
  //protMap["missedCleavages"]
  
  //TODO push back peptide sequence, what todo do with pre and post?
  
  //TODO Calculate features
  
  //Save psm
  //database->savePsm(spectraId, psm_p);
  protMap.clear();
}

void tandemReader::read(const std::string fn, bool isDecoy,boost::shared_ptr<FragSpectrumScanDatabase> database)
{
  std::string line, tmp, prot;
  std::istringstream lineParse;
  std::ifstream tandemIn;
  int spectraId;
  
  namespace xml = xsd::cxx::xml;
  
  ifstream ifs;
  ifs.exceptions(ifstream::badbit|ifstream::failbit);
  ifs.open(fn.c_str());
  parser p;
  
  try
  { 
    //Sending fixedXML as the bool for validation since if its not fixed the namespace has to be added later and then we cant validate the schema and xml file.
    xml_schema::dom::auto_ptr< xercesc::DOMDocument> doc (p.start (ifs, fn.c_str(),fixedXML, schemaDefinition,schema_major, schema_minor, scheme_namespace));
    assert(doc.get());
    
    //tandem_ns::bioml biomlObj=biomlObj(*doc->getDocumentElement()); NOTE the root of the element, doesn't have any useful attributes
    
    for (doc = p.next(); doc.get() != 0; doc = p.next ()) //Loops over the grouo elements which are the spectra and the last 3 are the input parameters
    {
      //NOTE cant acess mixed content using codesynthesis, need to keep dom assoication. See the manual for tree parser and : 
      //http://www.codesynthesis.com/pipermail/xsd-users/2008-October/002005.html
      //Not implementet here
      
      try
      {
	
	//Check that the tag name is group and that its not the inputput parameters
	if(XMLString::equals(groupStr,doc->getDocumentElement()->getTagName()) && XMLString::equals(groupModelStr,doc->getDocumentElement()->getAttribute(groupTypeStr))) 
	{
	  
	  //TODO Add namespace if missing here
	  if(!fixedXML)
	  {
	    //Fix xml and parse to object model
	    std::cerr << "Fixing xml file" << std::endl; //TMP
	    add_namespace (doc,doc->getDocumentElement(),"http://www.thegpm.org/TANDEM/2011.12.01.1",1);
	  }
	  
	  int nHits=0;
	  bool a0Found=false;
	  bool a1Found=false;
	  tandem_ns::group groupObj(*doc->getDocumentElement());
	  
	  if(!spectraMap.empty())
	  {
	    spectraMap.clear();
	  }
	  
	  //Information about the spectra and the highest scoring match. 
	  //NOTE The attributes related to the highest scoring match is not parsed from the group element, all matches are parsed in createPSM function from the protein element and its children
	  if(groupObj.mh().present() && groupObj.z().present() && groupObj.sumI().present() && groupObj.maxI().present() && groupObj.fI().present()&& groupObj.id().present())
	  {
	    spectraMap["parenIonMass"]=groupObj.mh().get(); 	//the parent ion mass (plus a proton) from the spectrum
	    spectraMap["charge"]=groupObj.z().get(); 	//the parent ion charge from the spectrum
	    spectraMap["sumI"]=groupObj.sumI().get();	//the log10 value of the sum of all of the fragment ion intensities
	    spectraMap["maxI"]=groupObj.maxI().get();	//the maximum fragment ion intensity
	    spectraMap["fI"]=groupObj.fI().get();	//a multiplier to convert the normalized spectrum contained in this group back to the original intensity values NOTE Currently not used
	  }
	  else
	  {
	    cerr << "An required attribute is not present in the group/spectra element in file: " << fn << endl;
	    exit(1);
	  }
	  
	  BOOST_FOREACH(const tandem_ns::group1 &groupGAMLObj, groupObj.group1()) //Getting the group element surrounding the GAML namespace
	  {
	  
	    BOOST_FOREACH(const gaml_tandem_ns::trace &traceGAMLObj, groupGAMLObj.trace()) //GAML:trace
	    {
	      
	      BOOST_FOREACH(const gaml_tandem_ns::attribute &attributeTraceGAMLObj, traceGAMLObj.attribute()) //GAML:attribute
	      {
		gaml_tandem_ns::attribute::type_type typeAttr=attributeTraceGAMLObj.type();
		//a0 and a1 are two constans used to calculate the hyperscore expectation function. See:
		//Fenyö D, Beavis RC
		//A method for assessing the statistical significance of mass spectrometry-based protein identifications using general scoring schemes
		if(typeAttr=="a0")
		{
		  if(a0Found)
		  {
		    cerr << "Found more than one a0 attribute in a groups GAML part. File: " << fn << endl;
		    exit(1);
		  }
		  a0Found=true;
		  spectraMap["a0"]=attributeTraceGAMLObj;
		  
		} else if(typeAttr=="a1")
		{
		  if(a1Found)
		  {
		    cerr << "Found more than one a1 attribute in a groups GAML part. File: " << fn << endl;
		    exit(1);
		  }
		  a1Found=true;
		  spectraMap["a1"]=attributeTraceGAMLObj;
		}
	      }
	    }
	  }//End of GAML part
	  
	  //PSMS here
	  BOOST_FOREACH(const tandem_ns::protein &protObj, groupObj.protein()) //Protein
	  {
	    if(nHits<po.hitsPerSpectrum)
	    {
	      createPSM(protObj,isDecoy,database,spectraMap,fn);
	      nHits++;
	    }

	  }
	  spectraMap.clear();
	  
	}//End of if group and not parameters
      }catch(exception e)
      {
	cerr << "Problem parsing the codesynthesis object model for the xml file: " << fn << endl;
	cerr << e.what() << endl;
	exit(1);
      }
    }//End of for p.next
    
    ifs.close();
    
  }
  catch (const xml_schema::exception& e)
  {
    cerr << "Problem reading the xml file: " << fn << endl;
    cerr << e << endl;
    exit(1);
  }
  
}

