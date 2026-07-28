// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include <vector>
#include <random>

#include "RASH.h"
#include "IShaker.h"
#include "Solver.h"

#ifndef MAX_SHAKER_STEPS
#define MAX_SHAKER_STEPS 4086
#endif

using namespace std;

namespace RSO {

class RASHSolver: public Solver
{
private:
	mt19937_64 Rand;
	int tests;
	double currentValue;
	bool x_unset;
	double distance2 (const vector<double>&, const vector<double>&) const;
	double resolution, resolution4, resolution2;
	RASH *shaker;
	IShaker *ishaker;
	vector<double> eta, x;
	int iteration_restart;
	bool has_restarted;
	int n_evals_at_restart;
	bool random_around_middle;

public:
	RASHSolver (Function &f, int seed, double fraction, bool inertial = false,
		bool allinone = false, double reducefactor = .9, double expandfactor = -1.0, int iterations_to_restart = -1, bool random_around_middle = false);
	virtual ~RASHSolver ();
	const vector<double> &next(double &val);
	virtual double restart (bool rebox, const vector<double>&, double best = 1.0e30, const double *y0 = NULL);
	const vector<double> &randomRestart (void);

	inline bool has_just_restarted (void) const { return has_restarted; }

	virtual void set (const double*);
	static const string name;

	virtual double last_step_size() const;

	virtual void print_debug_info (FILE *f);

	void set_n_evals_at_restart (int nevrst) { n_evals_at_restart = nevrst; }

	virtual void set_options(int soft) {
		if (shaker) shaker->set_options(soft);
	};
};

}
