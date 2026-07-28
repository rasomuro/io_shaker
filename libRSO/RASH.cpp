// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include <iostream>
#include <ctime>
#include <cassert>
#include "Solver.h"

#include "RASH.h"
#include "RASH_bounds.h"

using namespace std;
using namespace RSO;

RASH::RASH(int d, unsigned long seed, double reducefactor, double expandfactor) :
	Shaker (d, seed),
	fdil (expandfactor > 0 ? expandfactor : 1 / reducefactor),
	ldil (log(fdil)),
	fcon (reducefactor),
	constraint_enforcement(ENFORCE_INDEPENDENT|REBOUND_BORDER)
{
	y = new double[n];
	DT = new double[n*n];
	base[0] = new double[n*n];
	base[1] = new double[n*n];
	r = new double[n];
	memset (DT, 0, n*n*sizeof(double));
	memset (base[0], 0, n*n*sizeof(double));
	memset (base[1], 0, n*n*sizeof(double));
	memset (delta, 0, n*sizeof(double));
	bi = 0;
	ls_best_so_far = 1e30;
}

RASH::~RASH()
{
	delete[] r;
	delete[] DT;
	delete[] base[0];
	delete[] base[1];
	delete[] y;
}

void RASH::shrinkBox (void)
{
	double minsize = 1;
	int i, j;
	for ( i = 0; i < n; i++ ) {
		double size = 0.0;
		for ( j = 0; j < n; j++ )
			size += base[bi][i*n+j] * base[bi][i*n+j];
		if ( size < minsize )
			minsize = size;
	}
	minsize = sqrt (minsize);
	for ( i = 0; i < n; i++ )
		for ( j = 0; j < n; j++ )
		base[bi][i*n+j] = (i == j) ? minsize : 0.0;
}

void RASH::restart(Solver &sol, bool rebox, const double *x0, const double *eta, double best)
{
	solver = &sol;
	restart (rebox, x0, eta, best);
}

void RASH::restart(bool rebox, const double *x0, const double *eta, double best)
{
	int i;

	if ( rebox ) {
		memset (base[0], 0, n*n*sizeof(double));
		for ( i = 0; i < n; i++ )
			base[0][i*(n+1)] = eta[i];
	}
	memcpy (x, x0, n*sizeof(double));
	//x = x0;
	ls_best_so_far = (best < 1.0e29) ? best : solver->evaluate(x0);
	bi = 0;
}

double *RASH::next(double &val)
{
	bool shots_successful = false;

	for ( int j = 0; j < n; j++ ) {
		delta[j]  = 0.0; // delta is delta_x
		r[j] = uniform_real_distribution<double>(-1.0,1.0)(Rand);
	}	

	for ( int i = 0; i < n; i++ ) {
		for ( int j = 0; j < n; j++ )
			delta[i] += base[bi][j*n+i] * r[j];
	}
	bool in_the_box = true;
	if (constraint_enforcement & ENFORCE_ALIGNED) { // keep changes parallel
		double min_ratio = 1.0e30;
		for ( int i = 0; i < n; i++ ) {
			const double ratio = delta[i] > 0 ? (dmax[i] - x[i]) / delta[i] : delta[i] < 0 ? (dmin[i] - x[i]) / delta[i] : 1.0e30;
			if (ratio < min_ratio) min_ratio  = ratio;
		}
		assert(min_ratio>=0);
		in_the_box = min_ratio >= 1.0;
		double new_ratio = in_the_box ? 1.0 : constraint_enforcement & REBOUND_INTERIOR ? uniform_real_distribution<double>(0,min_ratio)(Rand) : min_ratio;
		for (int i = 0; i < n; i++) {
			y[i] = x[i] + new_ratio * delta[i];
			assert(dmin[i]-1.0e20 <= y[i] && y[i] <= dmax[i]+1.0e20);
		}
	} else { // independent coordinates, do not keep changes parallel
		for ( int i = 0; i < n; i++ ) {
			y[i] = x[i] + delta[i];
			if ( y[i] > dmax[i] ) {
				in_the_box = false;
				y[i] = constraint_enforcement & REBOUND_INTERIOR ? x[i] + uniform_real_distribution<double>(0, dmax[i] - x[i])(Rand) : dmax[i];
			} else if ( y[i] < dmin[i] ) {
				in_the_box = false;
				y[i] = constraint_enforcement & REBOUND_INTERIOR ? x[i] - uniform_real_distribution<double>(0, x[i] - dmin[i])(Rand) : dmin[i];
			}
		}
	}
	double w = solver->evaluate(y);

	if( w < ls_best_so_far) shots_successful = true;
	if ( w >= ls_best_so_far ) { // failure: second shot
		in_the_box = true;
		if (constraint_enforcement & ENFORCE_ALIGNED) { // keep changes parallel
			double min_ratio = 1.0e30;
			for ( int i = 0; i < n; i++ ) {
				const double ratio = delta[i] > 0 ? (x[i] - dmin[i]) / delta[i] : delta[i] < 0 ? (x[i] - dmax[i]) / delta[i] : 1.0e30;
				if (ratio < min_ratio) min_ratio  = ratio;
			}
			assert(min_ratio>=0);
			in_the_box = min_ratio >= 1.0;
			double new_ratio = in_the_box ? 1.0 : constraint_enforcement & REBOUND_INTERIOR ? uniform_real_distribution<double>(0,min_ratio)(Rand) : min_ratio;
			for (int i = 0; i < n; i++) {
				y[i] = x[i] - new_ratio * delta[i];
				assert(dmin[i]-1.0e20 <= y[i] && y[i] <= dmax[i]+1.0e20);
			}
		} else { // independent coordinates, do not keep changes parallel
			for ( int i = 0; i < n; i++ ) {
				y[i] = x[i] - delta[i];
				if ( y[i] > dmax[i] ) {
					in_the_box = false;
					y[i] = constraint_enforcement & REBOUND_INTERIOR ? x[i] + uniform_real_distribution<double>(0, dmax[i] - x[i])(Rand) : dmax[i];
				} else if ( y[i] < dmin[i] ) {
					in_the_box = false;
					y[i] = constraint_enforcement & REBOUND_INTERIOR ? x[i] - uniform_real_distribution<double>(0, x[i] - dmin[i])(Rand) : dmin[i];
				}
			}
		}
		w = solver->evaluate(y);
	}

	sumsq = 0.0;
	for ( int i = 0; i < n; i++ )
		sumsq += delta[i] * delta[i];

	if ( w < ls_best_so_far ) { // success at the end
		shots_successful = true;
		memcpy (x, y, n*sizeof(double));
		ls_best_so_far = w;
	}

	int dilate = in_the_box && shots_successful;
	double fact = 
		(sumsq > 1e-10) ? (((dilate?fdil:fcon) - 1.0) / sumsq) : 0.0;

	for ( int i = 0; i < n; i++ )
		for ( int j = 0; j < n; j++ )
			DT[i*n+j] = ((i==j)?1.0:0.0) + fact * delta[j] * delta[i];
	const double *current_base = base[bi];
	double *new_base = base[!bi];
	for ( int i = 0; i < n; i++ )
		for ( int j = 0; j < n; j++, new_base++ ) {
			double new_base_value = 0.0;
			const double *current_base_entry = current_base + i*n;
			for ( int k = 0; k < n; k++, current_base_entry++ )
				new_base_value += *current_base_entry * DT[k*n+j];
			*new_base = new_base_value;
		}

	bi = !bi; // serve per aggiornare la matrice a partire dai vecchi valori

	val = ls_best_so_far;

	return x;
}
