// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include "Solver.h"

using namespace std;
using namespace RSO;

Solver::Solver (Function &f, bool stlm):
	function (f),
	lambda (NULL),
	reference (NULL),
	moo_decomposition_method (MDM_NONE),
	store_local_minima (stlm),
	last_evalued_point (f.getDimension(), 0.0),
	best_point(f.getDimension())
{
	dimension = f.getDimension();
	range = f.getRange();
	dmin = new double[dimension+range];
	dmax = new double[dimension+range];
	dmin_init = new double[dimension];
	dmax_init = new double[dimension];
	for ( int i = 0; i < dimension; i++ ) {
		dmin[i] = f.getDomainMin(i);
		dmax[i] = f.getDomainMax(i);
		dmin_init[i] = f.getDomainMinInit(i);
		dmax_init[i] = f.getDomainMaxInit(i);
	}
	for ( int i = 0; i < range; i++ ) {
		dmin[dimension+i] = f.getRangeMin(i);
		dmax[dimension+i] = f.getRangeMax(i);
	}
	dscale = NULL;
	all_steps = NULL;
	constraints_min = constraints_max = NULL;
	fevals = 0;
	last_result = 1.0e30;
	last_x = new double[dimension];
	last_y = new double[range];
	goingdown = false;
	should_scale = true;
	never_discard_non_optimal = false;
	current_value = 1.0e30;
	best_values = new double[range];
	best_value = 1.0e30;
	local_best = 1e30;
	last_improved_fevals = 0;
	after_eval = NULL;
	stop_threshold = -1.0e30;
	n_restarts = 0;
}

Solver::~Solver ()
{
	for ( unsigned int i = 0; i < local_minima.size(); i++ )
		delete[] local_minima[i];
	delete[] all_steps;
	delete[] lambda;
	delete[] last_x;
	delete[] last_y;
	delete[] best_values;
	delete[] dmin;
	delete[] dmax;
	delete[] dmin_init;
	delete[] dmax_init;
}

void Solver::set (const double*)
{
}

void Solver::setDomain (const double *_dmin, const double *_dmax)
{
	memcpy (dmin, _dmin, dimension * sizeof(double));
	memcpy (dmax, _dmax, dimension * sizeof(double));
	if ( dscale )
		for ( int i = 0; i < dimension; i++ )
			dscale[i] = 1.0 / (dmax[i] - dmin[i]);
}

void Solver::setRange (const double *_rmin, const double *_rmax)
{
	memcpy (dmin + dimension, _rmin, range * sizeof(double));
	memcpy (dmax + dimension, _rmax, range * sizeof(double));
	if ( dscale )
		for ( int i = 0; i < range; i++ )
			dscale[dimension+i] = 1.0 / (_rmax[i] - _rmin[i]);
}

void Solver::save_all_steps (bool s, bool n)
{
	never_discard_non_optimal = n;
	if ( s && !all_steps ) {
		storesize = 1000;
		all_steps = new double[storesize * (dimension + range)];
		nsteps = 0;
		stepsize = 1;
	}
	else if ( !s && all_steps ) {
		delete all_steps;
		all_steps = NULL;
	}
}

void Solver::get_steps (const double *&s, int &n, int &sts)
{
	s = all_steps;
	n = nsteps;
	sts = stepsize;
	nsteps = 0;
	stepsize = 1;
}

void Solver::set_constraints (const double *m, const double *M)
{
	int i;
	for ( i = 0; i < range && !isfinite(m[i]) && !isfinite(M[i]); i++ );
	if ( i < range ) {
		constraints_min = m;
		constraints_max = M;
		double *scale = new double [range];
		for ( int j = 0; j < range; j++ ) {
			double mx = isfinite(m[j]) ? fabs(m[j]) : 0.0;
			if ( isfinite(M[j]) && fabs(M[j]) > mx )
				mx = fabs(M[j]);
			scale[j] = mx ? (1.0 / mx) : 1.0;
		}
		constraints_scale = scale;
	}
	else
		constraints_min = constraints_max = NULL;
}


double Solver::checkConstraints (const double * /*x*/, const  double *y)
{
	if ( !constraints_min )
		return 0.0;
	double violations = 0.0;
	const double
		*yy = y,
		*m = constraints_min,
		*M = constraints_max,
		*cs = constraints_scale;
	for ( int i = 0; i < range; i++, yy++, m++, M++, cs++ ) {
		double v = *yy;
		if ( v < *m )
			violations += 1.0 + (*m - v) * *cs;
		else if ( v > *M )
			violations += 1.0 + (v - *M) * *cs;
	}
	return violations;
}

void Solver::evaluationPerformed (const double *x, const  double *y)
{
	if ( after_eval )
		after_eval (this, x, y, current_value);
	if ( !all_steps || fevals % stepsize )
		return;
	if ( nsteps >= storesize ) {
		if ( never_discard_non_optimal ) {
			const double *as = all_steps;
			all_steps = new double [(storesize<<=1) * (dimension+range)];
			memcpy (all_steps, as, (storesize>>1) * (dimension+range) * sizeof(double));
			delete as;
		}
		else {
			stepsize *= 10;
			for ( int i = 0, j = 0; i < storesize; i+=10, j++ )
				memcpy (all_steps + j * (dimension+range), all_steps + i * (dimension+range), sizeof(double) * (dimension+range));
			nsteps = storesize / 10;
		}
	}
	memcpy (all_steps + nsteps * (dimension+range), x, dimension * sizeof(double));
	memcpy (all_steps + dimension + nsteps * (dimension+range), y, range * sizeof(double));
	nsteps ++;
}

double Solver::evaluate (const double *x)
{
	fevals++;

	last_results = function.eval (x);
	double v = decompose (x, last_results);
	//fprintf(stderr,"%f\t%f\t%f\n", current_value, v, best_value);
	if ( current_value < best_value )
		store_best_values (x, last_results, v);

	if ( store_local_minima )
		recordStep (x, current_value, last_results);
	evaluationPerformed (x, last_results);
	return current_value;
}

double Solver::decompose (const double *x, const double *y)
{
	double violations = checkConstraints(x, y);
	if ( violations > 0 )
		return 1.0e8 * violations;

	current_value = 0.0;
	if ( moo_decomposition_method == MDM_NONE )
		current_value = *y;
	else {
		if ( should_scale && !dscale ) {
			dscale = new double[dimension+range];
			for ( int i = 0; i < dimension+range; i++ )
				dscale[i] = 1.0 / (dmax[i] - dmin[i]);
		}
		const double
			*ymin = dmin + dimension,
			*yscale = dscale + dimension,
			*yy = y,
			*ll = lambda;
		if ( moo_decomposition_method == MDM_WEIGHTED_SUM ) {
			current_value = *(ll++) * (should_scale ? ((*(yy++) - *(ymin++)) * *(yscale++)) : *(yy++));
			for ( int i = 1; i < range; i++ )
				current_value += *(ll++) * (should_scale ? ((*(yy++) - *(ymin++)) * *(yscale++)) : *(yy++));
		}
		else if ( moo_decomposition_method == MDM_CHEBYSHEV ) {
			current_value = 0.0;
			double *rr = reference;
			for ( int i = 0; i < range; i++ ) {
				double
					v = *(yy++),
					l = *(ll++),
					r = *(rr++);
				if ( (l > 0 && v < r) || (l < 0 && v > r ) )
					rr[-1] = v;
				double f = l * fabs(v-r);
				if ( should_scale )
					f *= *(yscale++);
				if ( f >= current_value )
					current_value = f;
			}
		}
		else if ( moo_decomposition_method == MDM_AIM_AT_TARGET ) {
			current_value = 0.0;
			double *rr = reference;
			for ( int i = 0; i < range; i++ ) {
				double f = *(ll++) * (*(yy++) - *(rr++));
				if ( should_scale )
					f *= *(yscale++);
				current_value += f * f;
			}
			current_value /= range;
		}
		else {
			current_value = 0.0;
		}
	}

	return current_value;
}


void Solver::recordStep (const double *x, double p, const double *y)
{
	if ( !y )
		y = last_results;
	if ( p < last_result ) {
		goingdown = true;
		memcpy (last_x, x, dimension*sizeof(double));
		memcpy (last_y, y, range*sizeof(double));
	}
	else if ( p > last_result ) {
		if ( goingdown ) {
			bool found = false;
			unsigned int i = 0;
			for ( i = 0; i < local_minima.size(); i++ ) {
				const double *xx = last_x, *lx = local_minima[i];
				double d2 = 0.0;
				for ( int j = 0; j < dimension; j++ ) {
					double d = *(xx++) - *(lx++);
					d2 += d * d;
				}
				if ( d2 < 1.0e-1 ) {
					found = true;
					break;
				}
			}
			if ( !found ) {
				double *px = new double[dimension+range+1];
				memcpy (px, last_x, dimension * sizeof(double));
				memcpy (px+dimension, last_y, range * sizeof(double));
				px[dimension+range] = last_result;
				local_minima.push_back(px);
				local_minima_changed = true;
			}
			else {
				double *px = local_minima[i];
				if ( px[dimension+range] > last_result ) {
					memcpy (px, last_x, dimension*sizeof(double));
					memcpy (px+dimension, last_y, range*sizeof(double));
					px[dimension+range] = last_result;
					local_minima_changed = true;
				}
			}
		}
		goingdown = false;
	}
	last_result = p;
}


void Solver::setWeightedSumCoefficients (const double *l, bool scale)
{
	should_scale = scale;
	//fevals = 0;
	reference = NULL;
	if ( !l ) {
		delete lambda;
		lambda = NULL;
		moo_decomposition_method = MDM_NONE;
	} 
	else {
		if ( !lambda )
			lambda = new double[range];
		memcpy (lambda, l, range * sizeof(double));
		moo_decomposition_method = MDM_WEIGHTED_SUM;
	}
}

void Solver::setReferencePoint (const double *l, double *p, bool scale)
{
	should_scale = scale;
	//fevals = 0;
	if ( !l ) {
		delete lambda;
		lambda = NULL;
		reference = NULL;
		moo_decomposition_method = MDM_NONE;
	}
	else {
		if ( !lambda )
			lambda = new double[range];
		memcpy (lambda, l, range * sizeof(double));
		reference = p;
		moo_decomposition_method = MDM_CHEBYSHEV;
	}
}

void Solver::modifyLambdas (const double *l)
{
	if ( !l ) {
		delete lambda;
		lambda = NULL;
		reference = NULL;
		moo_decomposition_method = MDM_NONE;
	}
	else {
		if ( !lambda )
			lambda = new double[range];
		memcpy (lambda, l, range * sizeof(double));
	}
}

void Solver::setBoundaryIntersection (const double *l, double *p, double pn, bool scale)
{
	should_scale = scale;
	//fevals = 0;
	if ( !l ) {
		delete lambda;
		lambda = NULL;
		reference = NULL;
		moo_decomposition_method = MDM_NONE;
	}
	else {
		if ( !lambda )
			lambda = new double[range];
		memcpy (lambda, l, range * sizeof(double));
		reference = p;
		moo_decomposition_method = MDM_BOUNDARY_INTERSECTION;
	}
	reference_penalty = pn;
}

void Solver::setTargetPoint (const double *l, double *p, bool scale)
{
	should_scale = scale;
	//fevals = 0;
	if ( !l ) {
		delete lambda;
		lambda = NULL;
		reference = NULL;
		moo_decomposition_method = MDM_NONE;
	}
	else {
		if ( !lambda )
			lambda = new double[range];
		memcpy (lambda, l, range * sizeof(double));
		reference = p;
		moo_decomposition_method = MDM_AIM_AT_TARGET;
	}
}

void Solver::store_best_values (const double *x, const double *y, double v)
{
	for ( int j = 0; j < dimension; j++ )
		best_point[j] = x[j];
	best_value = v;
	memcpy (best_values, y, range * sizeof(double));
}

double Solver::redecompose_best (void)
{
	//forgetBest();
	local_best = best_value = decompose (&best_point[0], best_values);
	last_improved_fevals = getFEvals();
	return best_value;
}

double Solver::restart (bool /*rebox*/, const vector<double>&, double /*best*/, const double * /*y0*/ )
{
	n_restarts++;
	return 0.0;
}

void Solver::forgetBest (bool global)
{
	if ( global )
		best_value = 1e30;
	local_best = 1e30;
	last_improved_fevals = getFEvals();
}

void Solver::print_debug_info (FILE * /*f*/)
{
}

bool Solver::can_continue()
{
	return true;
}
