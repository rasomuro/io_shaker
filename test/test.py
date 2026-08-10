#! /usr/bin/env python
#
# Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
# Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

from pylion import reactive_affine_shaker, inertial_shaker

# Function to be optimized
def f(x: list[float]) -> float:
    return (x[0]-.5)**2 + (x[1]-.45)**2

# Test the solvers 10 times with different seeds
for seed in range(10):
    best_x, best_y = reactive_affine_shaker(f, 2, seed=seed)
    print('BEST AFFINE', best_x, best_y)
    best_x, best_y = inertial_shaker(f, 2, seed=seed)
    print('BEST INERTIAL', best_x, best_y)
