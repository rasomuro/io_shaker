// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include "function/fpython.h"
#include "RASHSolver.h"

#include "io_shaker.h"

static PyObject *ras_solver (
    PyObject *function,
    int dimension,
    int seed,
    double fraction,
    PyObject *lb,
    PyObject *ub,
    bool inertial,
    double reducefactor,
    double expandfactor,
    int iterations_to_restart,
    int constraint_enforcement,
    bool allinone,
    bool random_around_middle
) {
    FPython wrapped_function(function, dimension, lb, ub);
    RASHSolver solver(wrapped_function, seed, fraction, inertial, allinone, reducefactor, expandfactor, iterations_to_restart, random_around_middle);
    solver.set_options(constraint_enforcement);
    double v;
    for (int iteration = 0; iteration < 10000; iteration++) {
        solver.next(v);
    }
    PyObject *best_x = PyList_New(dimension);
    for (int i = 0; i < dimension; i++) {
        PyList_SetItem(best_x, i, PyFloat_FromDouble(solver.best_point[i]));
    }
    PyObject *best = PyTuple_New(2);
    PyTuple_SetItem(best, 0, PyFloat_FromDouble(solver.best_value));
    PyTuple_SetItem(best, 1, best_x);
    // Py_INCREF(best);
    return best;
}

PyObject *reactive_affine_shaker (
    PyObject *function,
    int dimension,
    int seed,
    double fraction,
    PyObject *lb,
    PyObject *ub,
    double reducefactor,
    double expandfactor,
    int iterations_to_restart,
    int constraint_enforcement,
    bool allinone,
    bool random_around_middle
) {
    return ras_solver(function, dimension, seed, fraction, lb, ub, false, reducefactor, expandfactor, iterations_to_restart, constraint_enforcement, allinone, random_around_middle);
}

PyObject *inertial_shaker (
    PyObject *function,
    int dimension,
    int seed,
    double fraction,
    PyObject *lb,
    PyObject *ub,
    double reducefactor,
    double expandfactor,
    int iterations_to_restart,
    int constraint_enforcement,
    bool allinone,
    bool random_around_middle
) {
    return ras_solver(function, dimension, seed, fraction, lb, ub, true, reducefactor, expandfactor, iterations_to_restart, constraint_enforcement, allinone, random_around_middle);
}
