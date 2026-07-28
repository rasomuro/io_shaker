// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include "function/fpython.h"
#include "RASHSolver.h"

#include "pylion.h"

double ras_solver (
    PyObject *function,
    int dimension,
    PyObject *best_x,
    int seed,
    double fraction,
    PyObject *lb,
    PyObject *ub,
    int constraint_enforcement,
    bool inertial,
    bool allinone,
    double reducefactor,
    double expandfactor,
    int iterations_to_restart,
    bool random_around_middle
) {
    FPython wrapped_function(function, dimension, lb, ub);
    RASHSolver solver(wrapped_function, seed, fraction, inertial, allinone, reducefactor, expandfactor, iterations_to_restart, random_around_middle);
    solver.set_options(constraint_enforcement);
    double v;
    for (int iteration = 0; iteration < 10000; iteration++) {
        solver.next(v);
    }
    for (int i = 0; i < dimension; i++) {
        PyList_SetItem(best_x, i, PyFloat_FromDouble(solver.best_point[i]));
    }
    return solver.best_value;
}
