// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include <iostream>
#include <fstream>
#include "RASHSolver.h"

using namespace std;
using namespace RSO;

#ifndef VC7
const string RASHSolver::name = "RASH";
#endif

RASHSolver::RASHSolver (Function &f, int seed,
	double fraction, bool inertial, bool justonce, double reducefactor,
	double expandfactor, int iterations_to_restart, bool ram)
	: Solver (f, false),
	Rand (seed),
	tests (8),
	x_unset (true),
	shaker (inertial ? NULL : new RASH(dimension, seed+1, reducefactor, expandfactor)),
	ishaker (inertial ? new IShaker(dimension, seed+1, justonce, reducefactor, expandfactor) : NULL),
	eta (dimension),
	x (dimension),
	random_around_middle (ram)
{
	if ( shaker )
		shaker->setDomain(dmin, dmax);
	else
		ishaker->setDomain(dmin, dmax);
	for ( int i = 0; i < dimension; i++ )
		eta[i] = fraction * (dmax[i]-dmin[i]);
	//best_value = 1e30;
	has_restarted = false;
	iteration_restart = iterations_to_restart < 0 ? 300 * dimension : iterations_to_restart == 0 ? 0 : iterations_to_restart;
	n_evals_at_restart = 1;
}

RASHSolver::~RASHSolver (void)
{
	delete shaker;
	delete ishaker;
}

double RASHSolver::restart (bool rebox, const vector<double> &x0, double best, const double *y0)
{
	n_restarts++;
	if ( shaker ) {
		shaker->restart (*this, rebox, &x0[0], &eta[0], best);
		local_best = shaker->get_current_value();
	}
	else {
		ishaker->restart (*this, rebox, &x0[0], &eta[0], best);
		local_best = ishaker->get_current_value();
	}
	if ( best < best_value ) {
		best_value = best;
		for ( int i = 0; i < dimension; i++ )
			best_point[i] = x0[i];
		if ( y0 )
			for ( int i = 0; i < range; i++ )
				best_values[i] = y0[i];
	}
	x_unset = false;
	last_improved_fevals = getFEvals();
	return local_best;
}

const vector<double> &RASHSolver::randomRestart ()
{
	vector<double> xx (dimension);
	double bst = 1.0e30;
	for ( int j = 0; j < n_evals_at_restart; j++ ) {
		for ( int i = 0; i < dimension; i++ ){
			if(!random_around_middle){
				// use random inital points between boundaries of the
				// dimension
				xx[i] = uniform_real_distribution<double>(dmin_init[i], dmax_init[i])(Rand);
			}else{
				// use random initial points around middle points
				// within eta distance of the middle point
				double x_m = (dmin_init[i] + dmax_init[i])/2;
				xx[i] = uniform_real_distribution<double>(x_m-eta[i], x_m+eta[i])(Rand);
			}
		}
		if ( n_evals_at_restart > 1 ) {
			double y = *function.eval(&xx[0]);
			if ( y < bst ) {
				bst = y;
				x = xx;
			}
		}
		else
			x = xx;
	}
	restart (true, x);
	return x;
}

const vector<double> &RASHSolver::next(double &val)
{
	has_restarted = false;
	if ( x_unset )
		randomRestart ();

	const double *lep = shaker ? shaker->next(currentValue) : ishaker->next(currentValue);
	last_evalued_point.assign (lep, lep+dimension);

	if ( currentValue < local_best ) {
		if ( currentValue < local_best * .9999 )
			last_improved_fevals = getFEvals();
		local_best = currentValue;
	}
	else if ( iteration_restart && getFEvals() > last_improved_fevals + iteration_restart ) {
		randomRestart();
		//forgetBest();
		has_restarted = true;
	}

	val = best_value;
	return best_point;
}

void RASHSolver::set (const double *y)
{
	vector<double> _x (dimension);
	for ( int i = 0; i < dimension; i++ )
		_x[i] = y[i];
	restart (false, _x);
}

void RASHSolver::print_debug_info (FILE *f)
{
	if ( ishaker )
		ishaker->print_debug_info(f);
}

double RASHSolver::last_step_size() const
{
	return shaker ? shaker->last_step_size() : ishaker->last_step_size();
}
