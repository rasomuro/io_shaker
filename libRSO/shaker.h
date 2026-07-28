// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include <random>

namespace RSO {
	class Solver;
}

using namespace RSO;
using namespace std;

class Shaker {
protected:
	int n;
	double *x;
	double current_value;
	double *delta;
	const double *dmin, *dmax;
	mt19937_64 Rand;
	double *const norms;

public:
	Shaker (int d, unsigned long seed);
	virtual ~Shaker();
	inline void setDomain (const double *_dmin, const double *_dmax) { dmin = _dmin; dmax = _dmax; }
	virtual void restart(Solver &sol, bool rebox, const double *x, const double *eta, double best = 1.0e30) = 0;
	virtual double *next(double &val) = 0;
	inline double *next(void) {double v;return next(v);};
	const double *get_delta() const {return delta;};
	double get_current_value() const { return current_value; }
	const double *get_current_x() const { return x; }
};
