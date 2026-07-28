# pylion

`pylion` provides Python bindings for the C++ RSO optimization library.

## Install

### From PyPi

Install the `pylion-co` package:

```sh
python -m pip install pylion-co
```

The package contains binding to a C++ library; therefore,
a C++ compiler and Python development headers may be required
if a suitable pre-compiled platform wheel is not found.

In such case, please follow your platform's instructions to
install the compiler and Python headers, then repeat the above command.

### Source tree

A C++ compiler and Python development headers are required when installing from
the source tree. SWIG is not required because its
generated files, `pylion.py` and `pylion_wrap.cxx`, are included in the package.

```sh
python -m pip install .
```

To build a wheel for the current Python and platform:

```sh
python -m pip wheel .
```

Setuptools selects the platform compiler and gives the native `_pylion`
extension the correct filename for Linux, macOS, or Windows.

If needed, the SWIG-generated files can be recreated by

```sh
swig -python -c++ pylion.i
```

### Native library

The following cross-platform commands compile a standalone C++ library:

```sh
cmake -S . -B cbuild
cmake --build cbuild --config Release
```

## Usage and example

### Python library

See `test/test.py`:

```python
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
```

The function `ras_solver` accepts the following parameters:

- `function` (mandatory, type `Callable[[list[float]],float]`): a Python function accepting a float array and returning a float value.
- `dimension` (mandatory, type `int`): the function's domain dimension.
- `best_x` (mandatory, type `list[float]`): a float array, passed by name/reference, which will be filled with the minimizer's coordinates.
- `seed` (type `int`, default 0): randome number generator seed, for reproducibility.
- `lb` (type `list[float]`, default `[0.0]*dimension`): domain lower bounds.
- `ub` (type `list[float]`, default `[1.0]*dimension`): domain upper bounds.
- `fraction` (type `float`, default .1): initial size of the search box as a fraction of the [`lb`,`ub`] interval along each dimension.
- `inertial` (type `boolean`, default `false`): if false, use the Affine Shaker heuristic; if true, use the Inertial Shaker.
- `reducefactor` (type `float`, default .9): search box reduction factor upon failed improvement.
- `expandfactor` (type `float`, default 1.0/`reducefactor`): search box expansion factor upon succeeded improvement.

It returns the minimum value (`float`).

### Native library

See `test/test.cpp`:

```c++
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
        double best_value = solver(f, best, seed);
        cout << "BEST AFFINE " << best_value << " @ " << best[0] << ',' << best[1] << endl;
        best_value = solver(f, best, seed, true);
        cout << "BEST INERTIAL " << best_value << " @ " << best[0] << ',' << best[1] << endl;
    }
    return 0;
}
```

To compile from inside `test`, add `..` and `../libRSO` as include directories and link the object file with `../cbuild/libRSO.a`:

```sh
cd test
g++ -c -I.. -I../libRSO test.cpp
g++ -o test -L../cbuild test.o -lRSO
./test
```

The function `solver` accepts the same parameters as its Python counterpart, with the following exceptions:

- `function` (class `Function`): a reference to an object from a subclass of `Function`.
- parameters `lb` and `ub` are omitted because the domain is specified in the `xmin` and `xmax` members of `Function`.

The `Function` subclass must define at least two methods:

- a constructor invoking the one from the base class, setting the domain's and codomain's dimension (the latter must be 1), and filling the lower and upper bound arrays;
- a protected `evaluate` function that accepts a constant double array and provides a pointer to a double variable containing the result.
