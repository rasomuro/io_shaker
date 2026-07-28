// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include <iostream>
#include <cmath>

#include "IShaker.h"
#include "Solver.h"

using namespace std;

using namespace RSO;

//const double IShaker::fdil = 2.0;
//const double IShaker::fcon = 0.5;
//const double IShaker::sigmaf = 1.1;
//const double IShaker::alphaf = 1.2;
//const double IShaker::alphacut = 1.0E-8;
//const double IShaker::sigmacut = 1.0E+3;
//const double IShaker::minw = 1.0E-6;

IShaker::IShaker(int d, unsigned long seed, bool just_one, double reducefactor, double expandfactor):
	Shaker (d, seed),
	depth (n),
	fdil (expandfactor >= 0 ? expandfactor : 1.0 / reducefactor),
	fcon (reducefactor),
	sigmaf (1.1),
	alphaf (1.2),
	alphacut (1.0E-8),
	sigmacut (1.0E+3),
	minw (1.0E-6),
	use_successes (false),
	y (new double[n]),
	Pol (new double[n]),
	Time (new double[depth]),
	Eta (new double[n]),
	Delta (new double*[depth]),
	change_just_one (just_one)
{
	for ( int i = 0; i < depth; i++ )
		Delta[i] = new double[n];
	randomized_order = new int[n];
	successes = new int[n];
	for ( int i = 0; i < n; i++ )
		randomized_order[i] = i;
}  

IShaker::~IShaker()
{
	for ( int i = 0; i < depth; i++ )
		delete[] Delta[i];
	delete[] Delta;
	delete[] Eta;
	delete[] Time;
	delete[] Pol;
	delete[] y;
	delete[] randomized_order;
	delete[] successes;
}  

void IShaker::restart(Solver &sol, bool rebox, const double *x0, const double *eta, double best)
{
	curr = 0;
	startupdone = 0;

	sigma = 0.005;
	alpha = 0.5;

	if ( rebox ) {
		memcpy (Eta, eta, n * sizeof(double));
		for ( int i = 0; i < n; i++ )
			Time[i] = -1.0;

		for ( int i = 0; i < depth; i++ )
			for ( int j = 0; j < n; j++ )
				Delta[i][j] = 0.0;

		for ( int i = 0; i < n; i++ )
			if (fabs(Eta[i]) == 0.0)
				return;
	}

	solver = &sol;
	memcpy (x, x0, n*sizeof(double));
	memcpy (y, x0, n*sizeof(double));
	current_value = (best < 1.0e29) ? best : solver->evaluate(x0);
	for ( int i = 0; i < n; i++ )
		successes[i] = 1;
}

void IShaker::setParameters (Solver &sol, const double *xs, double eta)
{
	double *xx = new double[n];
	double *ee = new double[n];
	for ( int i = 0; i < n; i++ ) {
		ee[i] = eta * (dmax[i]-dmin[i]);
		xx[i] = xs[i];
	}
	start (sol, xx, ee);
	delete[] ee;
	delete[] xx;
}


#ifdef WIN32
static double copysign (double x, double y)
{
	return (y >= 0) ? fabs(x) : -fabs(x);
}
#endif


double *IShaker::next (double &val)
{
	double val1, r = 0.0, s, w, w1;

	memcpy (y, x, n * sizeof(double));

	val1 = current_value;

	int nlim = n, ssucc = 0;
	if ( use_successes ) {
		nlim = change_just_one ? 1 : use_successes ? uniform_int_distribution<int>(1,n/2)(Rand) : n;
		for ( int i = 0; i < n; i++ ) {
			ssucc += successes[i];
			randomized_order[i] = i;
		}
	}
	for ( int ii = 0; ii < nlim; ii++ ) {
		int i=ii;
		if ( change_just_one && !use_successes )
			i = uniform_int_distribution<int>(0, n-1)(Rand);
		else if ( use_successes ) {
			int k = uniform_int_distribution<int>(0, ssucc-1)(Rand);
			int p = 0;
			while ( k >= successes[i = randomized_order[p]] ) {
				p++;
				k -= successes[i];
			}
			randomized_order[p] = randomized_order[n-ii-1];
			randomized_order[n-ii-1] = i;
			ssucc -= successes[i];
		}
		else {
			int p = uniform_int_distribution<int>(0, n-ii-1)(Rand);
			i = randomized_order[p];
			randomized_order[p] = randomized_order[n-ii-1];
			randomized_order[n-ii-1] = i;
		}
		r = delta[i] = Eta[i] * uniform_real_distribution<double>()(Rand);
		if ( startupdone )
			r = copysign(r, Pol[i]);
		s = y[i];

		y[i] = s + r;
		if ( y[i] > dmax[i] )
			y[i] = dmax[i];
		else if ( y[i] < dmin[i] )
			y[i] = dmin[i];

		w = solver->evaluate (y);
		if ( w >= current_value ) {
			y[i] = s - r;
			if ( y[i] > dmax[i] )
				y[i] = dmax[i];
			else if ( y[i] < dmin[i] )
				y[i] = dmin[i];
			w = solver->evaluate (y);
			if ( w >= current_value )
				y[i] = s;
		}

		if ( w < current_value ) {
			successes[i]++;
			current_value = w;
			Eta[i] *= fdil;
		}
		else
			Eta[i] *= fcon;
		if ( change_just_one )
			break;
	}
	if ( current_value < val1 ) {
		for ( int i = 0; i < n; i++ )
			Delta[curr][i] = y[i] - x[i];

		if ( !startupdone && (curr > min(8, depth/8)) )
			startupdone = 1;

		if ( startupdone ) {
			for ( int i = 0; i < n; i++ )
				Pol[i] = 0.0;
			w1 = 0.0;
			int k = curr;
			do {
				double dt = Time[curr]-Time[k];
				w1 += w = exp (-dt*dt * sigma);
				for ( int i = 0; i < n; i++ )
					if ( Delta[k][i] )
						Pol[i] += Delta[k][i] * (w * alpha);
				k = (k <= 0) ? (depth - 1) : (k - 1);
			} while ( (Time[k] > 0.0) && (w > minw) && (k != curr) );

			for ( int i = 0; i < n; i++ ) {
				Pol[i] /= w1;
				y[i] += Pol[i];
				if ( y[i] > dmax[i] )
					y[i] = dmax[i];
				else if ( y[i] < dmin[i] )
					y[i] = dmin[i];
			}

			w = solver->evaluate (y);

			if ( w < current_value ) {
				memcpy (x, y, n * sizeof(double));
				alpha *= alphaf;
				sigma /= sigmaf;
				current_value = w;
			}
			else {
				for ( int i = 0; i < n; i++ )
					x[i] += Delta[curr][i];
				alpha /= alphaf;
				sigma *= sigmaf;
			}
			if ( alphacut > alpha )
				alpha = alphacut;
			if ( sigmacut > sigma )
				sigma = sigmacut;
		}
		else
			memcpy (x, y, n * sizeof(double));

		curr = (curr + 1) % depth;
	}

	val = current_value;

	return x;
}

void IShaker::print_debug_info (FILE *f)
{
	for ( int i = 0; i < n; i++ )
		fprintf (f, "%s%f", i?"\t":"", Eta[i]);
	fputc ('\n', f);
}
