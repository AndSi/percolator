/*******************************************************************************
 * Percolator unofficial version
 * Copyright (c) 2006-7 University of Washington. All rights reserved.
 * Written by Lukas K�ll (lukall@u.washington.edu) in the 
 * Department of Genome Science at the University of Washington. 
 *
 * $Id: Globals.h,v 1.6 2007/07/16 22:57:21 lukall Exp $
 *******************************************************************************/
#ifndef GLOBALS_H_
#define GLOBALS_H_

#define VERB (Globals::getInstance()->getVerbose())

class Globals
{
public:
	virtual ~Globals();
    static Globals * getInstance();
    static void clean();
    int getVerbose() {return verbose;}
    void setVerbose(int verb) {verbose=verb;}
    void decVerbose() {verbose--;}
    void incVerbose() {verbose++;}
private:
	Globals();
    int verbose;
    static Globals * glob; 
};

#endif /*GLOBALS_H_*/
