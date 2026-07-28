// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include "fpython.h"

const std::string FPython::name = "Python";

// #include <iostream>
// using namespace std;

FPython::FPython (PyObject *_pyFunction, int _dimension, PyObject *_pyLBArray, PyObject *_pyUBArray) :
	Function (_dimension, 1, false)
{
	Py_INCREF(_pyFunction);
	pyFunction = _pyFunction;
	char buffer[15];
	for ( int i = 0; i < dimension; i++ ) {
		xmin[i] = Py_IsNone(_pyLBArray) ? 0.0 : PyFloat_AsDouble(PyList_GetItem(_pyLBArray, i));
		xmax[i] = Py_IsNone(_pyUBArray) ? 1.0 : PyFloat_AsDouble(PyList_GetItem(_pyUBArray, i));
		int l = sprintf (buffer, "x%d", i+1);
		cname[i] = new char[l+1];
		strcpy (cname[i], buffer);
		xbins[i] = 10;
		xstart[i]= 0.0;
		xcurrent[i] = 0.0;
	}
	xmin[dimension] = -10.0;
	xmax[dimension] = 10.0;
	cname[dimension] = new char[2];
	strcpy (cname[dimension], "f");
	xbins[dimension] = 10;
	xstart[dimension]= 10.0;
	xcurrent[dimension] = 0.0;
	pyXArray = PyList_New(dimension);
	if (!pyXArray) { PyErr_Print(); exit(1); }
}

FPython::~FPython(void)
{
	Py_DECREF(pyXArray);
	Py_DECREF(pyFunction);
}

const double *FPython::evaluate (const double *x)
{
	for (int i = 0; i < dimension; i++)
		PyList_SetItem(pyXArray, i, PyFloat_FromDouble(x[i]));
	PyObject *pyResult = PyObject_CallFunctionObjArgs(pyFunction, pyXArray, NULL);
	if (!pyResult) { PyErr_Print(); exit(1); }
	local_result = PyFloat_AsDouble(pyResult);
	Py_DECREF(pyResult);
	return &local_result;
}
