#! /usr/bin/env python
#
# Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
# Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

from pylion import ras_solver

# Function to be optimized
def f(x: list[float]) -> float:
    return (x[0]-.5)**2 + (x[1]-.45)**2

# List to accommodate the function's minimizer
best_x = [None]*2

# Test the solvers 10 times with different seeds
for seed in range(10):
    best_y = ras_solver(f, 2, best_x, seed=seed)
    print('BEST AFFINE', best_x, best_y)
    best_y = ras_solver(f, 2, best_x, inertial=True, seed=seed)
    print('BEST INERTIAL', best_x, best_y)
