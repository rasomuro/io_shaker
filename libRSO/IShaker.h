// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "shaker.h"

using namespace std;

using namespace RSO;

class IShaker: public Shaker {
private:
	//static const double fdil;
	//static const double fcon;

	//Function *f;
	Solver *solver;

	int depth;
	int curr;
	int startupdone;

	double fdil, fcon;
	double sigmaf;
	double alphaf;
	double alphacut;
	double sigmacut;
	double minw;
	bool use_successes;

	double alpha;
	double sigma;

	double *y, *Pol, *Time, *Eta, **Delta;

	bool change_just_one;
	int *randomized_order, *successes;

public:
	IShaker(int d, unsigned long seed, bool just_one = false, double reducefactor = .5, double expandfactor = -1.0);
	virtual ~IShaker();

	virtual void restart(Solver &sol, bool rebox, const double *x0, const double *eta, double best = 1.0e30);
	void start(Solver &sol, const double *_x, const double *eta) {restart(sol, true, _x, eta);};
	void setParameters (Solver &sol, const double *xs, double eta = .01);

	virtual double *next(double &val);

	double last_step_size() const { return 0.0; }

	void print_debug_info (FILE *f);
};
