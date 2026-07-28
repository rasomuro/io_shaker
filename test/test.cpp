// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include <iostream>

#include "function.h"
#include "lion.h"

using namespace std;

class MyFunction: public Function {
protected:
    double result_value;
    virtual const double *evaluate (const double *x);
public:
    MyFunction();
};

MyFunction::MyFunction():
    Function(2, 1, false)
{
    xmin[0] = xmin[1] = 0.0;
    xmax[0] = xmax[1] = 1.0;
}

const double *MyFunction::evaluate(const double *x) {
    double
        x0 = x[0]-.5,
        x1 = x[1]-.45;
    result_value = x0*x0 + x1*x1;
    return &result_value;
}

int main() {
    MyFunction f;
    double best[2];
    double best_value = solver(f, best, 1);
    cout << "Best value = " << best_value << " at " << best[0] << ',' << best[1] << endl;
    return 0;
}
