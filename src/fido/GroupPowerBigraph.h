// Written by Oliver Serang 2009
// see license for more information

#ifndef _GroupPowerBigraph_H
#define _GroupPowerBigraph_H

#include "ProteinIdentifier.h"
#include "BasicGroupBigraph.h"
#include "PivdoSplitter.h"
#include "Scores.h" // from Percolator

using namespace std;


class GroupPowerBigraph : public ProteinIdentifier
{
public:
  static double LOG_MAX_ALLOWED_CONFIGURATIONS;

 GroupPowerBigraph(Scores* fullset,double __alpha, double __beta, double __gamma) :
  ProteinIdentifier(),gm(RealRange(__alpha,1,__alpha),RealRange(__beta,1,__beta),__gamma), 
  sumLogLikelihoodOverAllAlphaBetaCachedFunctor( & GroupPowerBigraph::sumLogLikelihoodOverAllAlphaBeta, "sumLogLikelihoodOverAllAlphaBeta")
      {
	read(fullset);
	//	cout << "Constructed GroupPowerBigraph" << endl;
      }
      
  GroupPowerBigraph(char * fname,double __alpha, double __beta, double __gamma) :
  ProteinIdentifier(), gm(RealRange(__alpha,1,__alpha),RealRange(__beta,1,__beta),__gamma), 
  sumLogLikelihoodOverAllAlphaBetaCachedFunctor( & GroupPowerBigraph::sumLogLikelihoodOverAllAlphaBeta, "sumLogLikelihoodOverAllAlphaBeta")
      {
	ifstream fin(fname);
	read(fin);
	//	cout << "Constructed GroupPowerBigraph" << endl;
      }
  Array<double> proteinProbs(const GridModel & myGM);
  Array<double> proteinProbsOverAllAlphaBeta();
  void printProteinWeights() const;

  void getProteinProbs();
  void getProteinProbsOverAllAlphaBeta();

  double probabilityAlphaBetaGivenD();

  void scanAlphaBeta();

  Array<BasicBigraph> iterativePartitionSubgraphs(BasicBigraph & bb, double newPeptideThreshold );

  double getLogNumberStates() const;

  double likelihoodAlphaBetaGivenD( const GridModel & myGM ) const;
  double logLikelihoodAlphaBetaGivenD( const GridModel & myGM ) const;

  // only pass this so that it can be cached easily 
  // even though you are passing a member variable
  double sumLogLikelihoodOverAllAlphaBeta( const GridModel & myGM ) const;

  // only pass this so that it can be cached easily 
  // even though you are passing a member variable

  double probabilityAlphaBetaGivenD( const GridModel & myGM ) const;

  void gridScan() const;

  void readFromMCMC(istream & graph, istream & pepProph);

  void outputPivdo(ostream & os) const;

  pair<Array<Array<string> >, Array<double> > getDescendingProteinsAndWeights() const;
  void setAlphaBetaGamma(double alpha, double beta, double gamma);
  
  //Mattia Tomasoni
  void read(Scores* fullset, bool debug = false);
  //Mattia Tomasoni: making a few fields publicly accessible
  Array<string> severedProteins;
  Array<double> probabilityR;
  Array<Array<string> > groupProtNames;
  Array<BasicGroupBigraph> subgraphs;

protected:

  int numberClones;
  void initialize();

  void getProteinProbsGivenAlphaBeta();

  void getGroupProtNames();

  void read(istream & is);
  //Mattia Tomasoni
  void speedUp(BasicBigraph& bb);

  GridModel gm;

  double alpha,beta,gamma;
  // cached functors
  LastCachedMemberFunction<GroupPowerBigraph, double, GridModel> sumLogLikelihoodOverAllAlphaBetaCachedFunctor;

  Numerical zeroChecker;
};


ostream & operator <<(ostream & os, pair<double,double> rhs);


#endif

