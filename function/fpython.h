// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

extern "C" {
	#define PY_SSIZE_T_CLEAN
	#include <Python.h>
}

#include <string>
#include <csignal>

#include "function.h"

class FPython : public Function {
private:

	PyObject *pyFunction;
	PyObject *pyXArray;

	double local_result;
	virtual const double *evaluate (const double *x);

public:
	static const std::string name;

	FPython (PyObject *_pyFunction, int _dimension, PyObject *_pyLBArray, PyObject *_pyUBArray);
	virtual ~FPython(void);
};
