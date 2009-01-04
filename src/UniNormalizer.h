/*******************************************************************************
 * Percolator unofficial version
 * Copyright (c) 2006-8 University of Washington. All rights reserved.
 * Written by Lukas K�ll (lukall@u.washington.edu) in the 
 * Department of Genome Sciences at the University of Washington. 
 *
 * $Id: UniNormalizer.h,v 1.11 2009/01/04 22:49:31 lukall Exp $
 *******************************************************************************/
#ifndef UNINORMALIZER_H_
#define UNINORMALIZER_H_

class UniNormalizer : public Normalizer // virtual Normalizer
{
public:
	UniNormalizer();
	virtual ~UniNormalizer();
    virtual void setSet(set<DataSet *> & setVec, size_t numFeatures, size_t numRetentionFeatures);
    void unnormalizeweight(const vector<double>& in,vector<double>& out);
    void normalizeweight(const vector<double>& in, vector<double>& out);
};

#endif /*UNINORMALIZER_H_*/
