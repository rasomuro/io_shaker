// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include <fstream>
#include <cstdlib>
#include <cmath>

#include "shaker.h"

using namespace std;

using namespace RSO;

class RASH: public Shaker {
private:

	const double fdil, ldil;
	const double fcon;

	Solver *solver;

	double *r;

	double *y;

	double *DT, *base[2];

	double sumsq;

	int bi;

	int constraint_enforcement;

	double ls_best_so_far;

public:
	RASH (int d, unsigned long seed, double reducefactor = .9, double expandfactor = -.1);
	virtual ~RASH();

	virtual void restart(Solver &sol, bool rebox, const double *x0, const double *eta, double best = 1.0e30);
	void start(Solver &sol, const double *_x, const double *eta) {restart(sol, true, _x, eta);};
	void restart(bool rebox, const double *x, const double *eta, double best = 1.0e30);

	virtual double *next(double &val);

	void shrinkBox (void);

	double last_step_size() const { return sqrt(sumsq); }
	void set_options(int soft) {
		constraint_enforcement = soft;
	};
};
