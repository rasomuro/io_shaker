// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include <cstdio>
#include <cstring>
//#include <vector>
#include <math.h>

#ifdef WIN32
#ifndef MINGW
extern double INFINITY;
#define isfinite(x) ((x)<=DBL_MAX&&(x)>=-DBL_MAX)
#endif
#endif

class Function {

public:
	class Logger {
		friend class Function;
	protected:
		const Function *function;
	public:
		virtual ~Logger();
		virtual void log() = 0;
	};

private:
	// Children must access this via the virtual function get_fevals() because
	// they might embed an external function with its own authoritative counter
	// (e.g., test frameworks such as Coco)
	int n_evals;
	Logger *logger;

protected:
	int dimension, range;
	double *xmin, *xmax, *xcurrent, *xstart, *xscale, *xmin_init, *xmax_init;
    double completion;
	int *xbins;
	char **cname;
	double partial;
	const double *result;
	bool validPartial;

	double **M, *xxx, *h;

	const double *constraint_min, *constraint_max;

	int *ncategories;
	char ***categorylabels;
	
	char *error_message;

	void create_rotation_matrix (void);
	const double *rotate (const double *x);
	virtual const double *evaluate (const double *x) = 0;
	void init (int d, int r, bool rotate);

public:
	Function ();
	Function (int d, int r, bool rotate);

	virtual ~Function(void);
	//virtual double operator() (const double *x);
	const double *eval (const double *x);
	const double *pareval (int n, const double *x);
	virtual double operator() (int d, double x, int r);
	//virtual const double *getCurrentValue (void) const;

	int getNDomainCategories (int d) const;
	int getNRangeCategories (int d) const;
	const char *getDomainCategory (int d, int k) const;
	const char *getRangeCategory (int d, int k) const;

	virtual void activateFunctionInCurrentThread (void);
	virtual void currentThreadClosing (void);

	virtual void reload (const char *fname = NULL);

	const char *getCoordinateName (int d) const { return cname[d]; }
	const char *getRangeName (int i = 0) const { return cname[dimension+i]; }
	int getDimension (void) const { return dimension; }
	int getRange (void) const { return range; }
	double getDomainMin (int d) const { return xmin[d]; }
	double getDomainMax (int d) const { return xmax[d]; }
	const double *getDomainMin () const { return xmin; }
	const double *getDomainMax () const { return xmax; }
	double getDomainMinInit (int d) const { return (xmin_init ? xmin_init : xmin)[d]; }
	double getDomainMaxInit (int d) const { return (xmax_init ? xmax_init : xmax)[d]; }
	int getDomainBins (int d) const { return xbins[d]; }
	double getDomainStart (int d) const { return xstart[d]; }
	void setDomainStart (int d, double x) { xstart[d] = x; }
	double getX (int d) const { return xcurrent[d]; }
	void setX (int d, double x) { xcurrent[d] = x; validPartial = false; }
	double getRangeMin (int i = 0) const { return xmin[dimension+i]; }
	double getRangeMax (int i = 0) const { return xmax[dimension+i]; }
	int getRangeBins (int i = 0) const { return xbins[dimension+i]; }
	const double *getLastResult (void) const { return result; }

	inline bool is_constrained (void) const { return constraint_min || constraint_max; }
	inline double getConstraintMin (int d) const { return constraint_min ? constraint_min[d] : -INFINITY; }
	inline double getConstraintMax (int d) const { return constraint_max ? constraint_max[d] : INFINITY; }

	virtual int get_fevals() const;

	inline const char *getErrorMessage (void) const { return error_message; }

    inline void set_completion (double c) { completion = c; }
    inline double get_completion (void) const { return completion; }

	void set_logger (Logger *log);
};
