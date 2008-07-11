/*******************************************************************************
 * Percolator unofficial version
 * Copyright (c) 2006-8 University of Washington. All rights reserved.
 * Written by Lukas K�ll (lukall@u.washington.edu) in the 
 * Department of Genome Science at the University of Washington. 
 *
 * $Id: Normalizer.h,v 1.14 2008/07/11 19:43:14 lukall Exp $
 *******************************************************************************/
#ifndef NORMALIZER_H_
#define NORMALIZER_H_
class DataSet;
#include <set>
using namespace std;

class Normalizer
{
public:
	virtual ~Normalizer();
    virtual void setSet(set<DataSet *> & setVec, size_t numFeatures, size_t numRetentionFeatures){;}
    void normalizeSet(set<DataSet *> & setVec);
    void normalize(const double * in, double * out, size_t offset, size_t numFeatures);
    double normalize(const double in, size_t index) { return (in-sub[index])/div[index]; }
    virtual void unnormalizeweight(const vector<double>& in,vector<double>& out){;}
    virtual void normalizeweight(const vector<double>& in,vector<double>& out){;}
    static Normalizer * getNormalizer();
    static void setType(int type);
	const static int UNI = 0;
	const static int STDV = 1;
protected:
    Normalizer();
    static Normalizer * theNormalizer;
	static int subclass_type;
    size_t numFeatures, numRetentionFeatures;
    vector<double> sub;
    vector<double> div;
};

#endif /*NORMALIZER_H_*/
