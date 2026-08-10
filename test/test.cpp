// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include <iostream>

#include "function.h"
#include "lion.h"

using namespace std;

class MyFunction: public Function {
protected:
    // variable holding the function's latest evaluation
    double result_value;
    // evaluation function
    virtual const double *evaluate (const double *x);
public:
    // Constructor
    MyFunction();
};

MyFunction::MyFunction():
    // base constructor: domain and codomain dimensions, random rotations (useful for tests)
    Function(2, 1, false)
{
    // Basic initialization: lower and upper bound for each coordinate
    xmin[0] = xmin[1] = 0.0;
    xmax[0] = xmax[1] = 1.0;
}

const double *MyFunction::evaluate(const double *x) {
    double
        x0 = x[0]-.5,
        x1 = x[1]-.45;
    result_value = x0*x0 + x1*x1;
    // The function returns a pointer to the value
    return &result_value;
}

int main() {
    MyFunction f;
    double best[2];
    // Test the solvers ten times with different seeds
    for (int seed = 0; seed < 10; seed++ ) {
        double best_value = reactive_affine_shaker(f, best, seed);
        cout << "BEST AFFINE " << best_value << " @ " << best[0] << ',' << best[1] << endl;
        best_value = inertial_shaker(f, best, seed);
        cout << "BEST INERTIAL " << best_value << " @ " << best[0] << ',' << best[1] << endl;
    }
    return 0;
}
