// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

extern "C" {
    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
}

#include "RASH_bounds.h"

extern double ras_solver (
    PyObject *function,
    int dimension,
    PyObject *best_x,
    int seed = 0,
    double fraction = .1,
    PyObject *lb = NULL,
    PyObject *ub = NULL,
    int constraint_enforcement = 0,
    bool inertial = false,
    bool allinone = false,
    double reducefactor = .9,
    double expandfactor = -1.0,
    int iterations_to_restart = -1,
    bool random_around_middle = false
);
