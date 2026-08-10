// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#pragma once

#include "function.h"

extern double reactive_affine_shaker (
    Function &function,
    double *best_x,
    int seed = 0,
    double fraction = .1,
    double reducefactor = .9,
    double expandfactor = -1.0,
    int iterations_to_restart = -1,
    int constraint_enforcement = 0,
    bool allinone = false,
    bool random_around_middle = false
);

extern double inertial_shaker (
    Function &function,
    double *best_x,
    int seed = 0,
    double fraction = .1,
    double reducefactor = .9,
    double expandfactor = -1.0,
    int iterations_to_restart = -1,
    int constraint_enforcement = 0,
    bool allinone = false,
    bool random_around_middle = false
);
