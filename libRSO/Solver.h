// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include <vector>

#include "function.h"

using namespace std;

namespace RSO {

class Solver
{
public:
	typedef void (*eval_callback) (const Solver *s, const double *x, const double *y, double v);
	enum decomposition_method {
		MDM_NONE,
		MDM_WEIGHTED_SUM,
		MDM_CHEBYSHEV,
		MDM_BOUNDARY_INTERSECTION,
		MDM_AIM_AT_TARGET
	};

protected:
	Function &function;
	int dimension;
	int range;
	double *dmin, *dmax, *dcurrent, *dstart, *dscale, *dmin_init, *dmax_init;
	double *all_steps;
	int nsteps, stepsize, storesize;
	double *lambda, *reference;
	double reference_penalty;
	int fevals;

	const double *constraints_min, *constraints_max, *constraints_scale;
	decomposition_method moo_decomposition_method;

	std::vector<double*> local_minima;
	const double *last_results;
	double *last_x, *last_y;
	double last_result;
	bool goingdown;
	bool local_minima_changed;
	bool should_scale;
	bool store_local_minima;
	bool never_discard_non_optimal;

	int last_improved_fevals;
	
	double local_best;

	eval_callback after_eval;

	double stop_threshold;
	int n_restarts;

public:
	Solver (Function &f, bool stlm);
	virtual ~Solver ();

	virtual const vector<double> &next(double &val) = 0;

	vector<double> last_evalued_point;
	vector<double> best_point;
	double current_value;
	double	best_value;
	double *best_values;

	virtual void set (const double*);

	int getFEvals (void) const { return fevals; }
	void resetFEvals (void) { fevals = 0; }

	void setDomain (const double *_dmin, const double *_dmax);
	void setRange (const double *_rmin, const double *_rmax);

// #ifdef WIN32
// 	vector<double*> points;
// #endif

	double evaluate (const double *x);
	double decompose (const double *x, const double *y);

	virtual void setWeightedSumCoefficients (const double *l, bool scale = true);
	virtual void setReferencePoint (const double *l, double *p, bool scale = true);
	virtual void setBoundaryIntersection (const double *l, double *p, double pn, bool scale = true);
	virtual void setTargetPoint (const double *l, double *p, bool scale = true);
	void modifyLambdas (const double *l);

	void recordStep (const double *x, double p, const double *y = NULL);

	inline bool areLocalMinimaChanged(void) const { return local_minima_changed; }
	inline int getNLocalMinima (void) { local_minima_changed = false; return (int)local_minima.size(); }
	inline const double *getLocalMinimum (int i) const { return local_minima[i]; }

	inline decomposition_method get_decomposition_method (void) const { return moo_decomposition_method; }

	virtual double checkConstraints (const double *x, const  double *y);
	virtual void evaluationPerformed (const double *x, const  double *y);
	void save_all_steps (bool s, bool n);
	inline bool saving_all_steps (void) const { return all_steps != NULL; }
	void get_steps (const double *&s, int &n, int &sts);
	void add_step (const double *x, const double *y);

	void set_constraints (const double *m, const double *M);

	void get_constraints (const double *&m, const double *&M) const { m = constraints_min; M = constraints_max; }
	double getConstraintMin (int i) const { return constraints_min ? constraints_min[i] : -INFINITY; }
	double getConstraintMax (int i) const { return constraints_max ? constraints_max[i] : INFINITY; }

	inline int get_n_restarts() const { return n_restarts; }

	inline const double *getCurrentValue (void) const { return last_results; }

	inline const Function &getFunction (void) const { return function; }

	void store_best_values (const double *x, const double *y, double v);

	double redecompose_best (void);

	int evals_since_improvement (void) const { return getFEvals() - last_improved_fevals; }
	virtual double restart (bool rebox, const vector<double>&, double best = 1.0e30, const double *y0 = NULL);
	void forgetBest (bool global = false);
	
	inline void set_eval_callback (eval_callback f) { after_eval = f; }

	virtual void print_debug_info (FILE *f);
	
	inline void set_stop_threshold(double t) { stop_threshold = t; }

	virtual bool can_continue();

	virtual void set_options(int) {}
};

}
