// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include <iostream>
#include "cstdlib"
#include "function.h"
#include "math.h"

#ifdef WIN32
#ifndef MINGW
#include <limits>
double INFINITY = 1.0e300 * 1.0e300;
#endif
#endif

using namespace std;

Function::Function ()
{
	n_evals = 0;
	logger = NULL;
	error_message = new char[1000];
	*error_message = '\0';
    completion = 0.0;
}

Function::Function (int d, int r, bool rotate)
{
	n_evals = 0;
	logger = NULL;
	error_message = new char[1000];
	*error_message = '\0';
    completion = 0.0;
	init (d, r, rotate);
}

Function::~Function(void)
{
	delete[] xmin;
	delete[] xmax;
	delete[] xmin_init;
	delete[] xmax_init;
	delete[] xcurrent;
	delete[] xstart;
	delete[] xscale;
	delete[] xbins;
	for ( int i = 0; i < dimension+range; i++ )
		delete[] cname[i];
	delete[] cname;
	delete[] error_message;
	delete[] h;
}

void Function::init (int d, int r, bool rotate)
{
	dimension = d;
	range = r;
	xmin = new double [d+r];
	xmax = new double [d+r];
	xcurrent = new double [d+r];
	xstart = new double [d+r];
	xscale = NULL;
	xbins = new int [d+r];
	cname = new char* [d+r];
	constraint_min = constraint_max = NULL;
	h = NULL;
	if ( rotate )
		create_rotation_matrix();
	else
		M = NULL;
	ncategories = NULL;
	categorylabels = NULL;
	xmin_init = xmax_init = NULL;
	n_evals = 0;
}


void Function::create_rotation_matrix (void)
{
	srand(0);
	M = new double* [dimension];
	xxx = new double[dimension];

	for ( int i = 0; i < dimension; i++ ) {
		M[i] = new double[dimension];
		for ( int j = 0; j < dimension; j++ )
			M[i][j] = /*i==j ? 1.0 : .05 * (rand() / (double)RAND_MAX - .5); //*/ rand() / (.5 * RAND_MAX) - 1.0;
		for ( int j = 0; j < i; j++ ) {
			xxx[j] = 0.0;
			for ( int k = 0; k < dimension; k++ )
				xxx[j] += M[i][k] * M[j][k];
		}
		double l = 0;
		for ( int j = 0; j < dimension; j++ ) {
			for ( int k = 0; k < i; k++ )
				M[i][j] -= M[k][j] * xxx[k];
			l += M[i][j] * M[i][j];
		}
		l = 1.0 / sqrt(l);
		for ( int j = 0; j < dimension; j++ )
			M[i][j] *= l;
	}
}

const double *Function::rotate (const double *x)
{
	if ( !M )
		return x;
	///*
	if ( !h ) {
		h = new double[dimension];
		for ( int i = 0; i < dimension; i++ )
			h[i] = .1 * (xmax[i]-xmin[i]) * (rand() / (double)RAND_MAX - .5);
	}
	for ( int i = 0; i < dimension; i++ ) {
		double s = h[i];
		for ( int j = 0; j < dimension; j++ )
			s += M[i][j] * x[j];
		xxx[i] = s;
	}
	//*/
	/*
	double max = 0.0;
	double l = 0.0;
	for ( int i = 0; i < dimension; i++ ) {
		xx[i] = x[i];//2.0 * (x[i] - xmin[i]) / (xmax[i] - xmin[i]) - 1.0;
		double a = fabs(xx[i]);
		if ( a > max )
			max = a;
		l += a * a;
	}
	l = max / sqrt(l);
	for ( int i = 0; i < dimension; i++ )
		xx[i] *= l;
	max = 0.0;
	l = 0.0;
	for ( int i = 0; i < dimension; i++ ) {
		xxx[i] = 0.0;
		for ( int j = 0; j < dimension; j++ )
			xxx[i] += M[i][j] * xx[j];
		double a = fabs(xxx[i]);
		if ( a > max )
			max = a;
		l += a * a;
	}

	l = sqrt(l) / max;
	for ( int i = 0; i < dimension; i++ ) {
		xxx[i] = .5 * (xxx[i] * l + 1.0) * (xmax[i] - xmin[i]) + xmin[i];
		if ( xxx[i] < xmin[i] )
			xxx[i] = xmin[i];
		else if ( xxx[i] > xmax[i] )
			xxx[i] = xmax[i];
	}
	*/
	return xxx;
}

double Function::operator() (int d, double x, int r)
{
	xcurrent[d] = x;
	return eval(xcurrent)[r];
}

void Function::activateFunctionInCurrentThread (void)
{
}

void Function::currentThreadClosing (void)
{
}

int Function::getNDomainCategories (int d) const
{
	return ncategories ? ncategories[d] : 0;
}

int Function::getNRangeCategories (int d) const
{
	return ncategories ? ncategories[dimension+d] : 0;
}

const char *Function::getDomainCategory (int d, int k) const
{
	return categorylabels ? categorylabels[d][k] : NULL;
}

const char *Function::getRangeCategory (int d, int k) const
{
	return categorylabels ? categorylabels[dimension+d][k] : NULL;
}

void Function::reload (const char * /*fname*/)
{
}

const double *Function::eval (const double *x)
{
	n_evals++;
	result = evaluate (rotate(x));
	if (logger) {
		memcpy(xcurrent, x, dimension * sizeof(double));
		logger->log();
	}
	return result;
}

int Function::get_fevals() const {
	return n_evals;
}

void Function::set_logger (Logger *log) {
	log->function = this;
	logger = log;
}

Function::Logger::~Logger() {}
