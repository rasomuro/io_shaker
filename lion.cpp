// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include "RASHSolver.h"

#include "lion.h"

double solver (
    Function &function,
    double *best_x,
    int seed,
    bool inertial,
    double fraction,
    double reducefactor,
    double expandfactor,
    int iterations_to_restart,
    int constraint_enforcement,
    bool allinone,
    bool random_around_middle
) {
    RASHSolver solver(function, seed, fraction, inertial, allinone, reducefactor, expandfactor, iterations_to_restart, random_around_middle);
    solver.set_options(constraint_enforcement);
    double v;
    for (int iteration = 0; iteration < 10000; iteration++) {
        solver.next(v);
    }
    for (int i = 0; i < function.getDimension(); i++) {
        best_x[i] = solver.best_point[i];
    }
    return solver.best_value;
}
