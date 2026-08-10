# io-shaker

`io-shaker` provides Python bindings for the C++ Intelligent Optimization heuristics.
The package exposes two local search heuristics for continuous function optimization:

- the **inertial shaker** [1, 3]: a "search box" is maintained around the
current best point and its sizes along the coordinates are expanded and contracted according
to the outcomes of explorative evaluations along the coordinated axes;

- the **affine shaker** [1, 2, 3], which maintains a more flexible search region whose size and
shape are again determined by explorative evaluations.

#### References

[1] R. Battiti and G. Tecchiolli. *Learning with first, second, and no derivatives: a case study
in high energy physics.* Neurocomputing, **6**:181–206, 1994.

[2] M. Brunato and R. Battiti. RASH: A self-adaptive random search method. In Carlos
Cotta, Marc Sevaux, and Kenneth Sörensen, editors, *Adaptive and Multilevel Meta-
heuristics*, volume 136 of Studies in Computational Intelligence. Springer, 2008. ISBN
978-3-540-79437-0.

[3] Roberto Battiti, Kevin Tierney, Mauro Brunato. *Intelligent Optimization - Optimization meets Machine Learning*.
LION association, Italy, 2026. Downloadable at https://intelligent-optimization.org/iobook/

## Installing

### From PyPi

Install the `io-shaker` package:

```sh
python -m pip install io-shaker
```

The package contains binding to a C++ library; therefore,
a C++ compiler and Python development headers may be required
if a suitable pre-compiled platform wheel is not found.

In such case, please follow your platform's instructions to
install the compiler and Python headers, then repeat the above command.

### From the source tree

A C++ compiler and Python development headers are required when installing from
the source tree. SWIG is not required because its
generated files, `io_shaker.py` and `io_shaker_wrap.cxx`, are included in the package.

```sh
python -m pip install .
```

To build a wheel for the current Python and platform:

```sh
python -m pip wheel .
```

Setuptools selects the platform compiler and gives the native `_io_shaker`
extension the correct filename for Linux, macOS, or Windows.

If needed, the SWIG-generated files `io_shaker.py` and `io_shaker_wrap.cxx` can be recreated by

```sh
swig -python -c++ io_shaker.i
```

### Native library

The following cross-platform commands compile a standalone C++ library
(`cmake` build system needed):

```sh
cmake -S . -B cbuild
cmake --build cbuild --config Release
```

## Usage and example

### Python library

See `test/test.py`:

```python
from io_shaker import reactive_affine_shaker, inertial_shaker

# Function to be optimized
def f(x: list[float]) -> float:
    return (x[0]-.5)**2 + (x[1]-.45)**2

# Test the solvers 10 times with different seeds
for seed in range(10):
    best_x, best_f = reactive_affine_shaker(f, 2, seed=seed)
    print('BEST AFFINE', best_f, best_y)
    best_x, best_f = inertial_shaker(f, 2, seed=seed)
    print('BEST INERTIAL', best_x, best_f)
```

The two functions `reactive_affine_shaker` and `inertial_shaker` accept the following parameters:

- `function` (mandatory, type `Callable[[list[float]],float]`): a Python function accepting a float array and returning a float value.
- `dimension` (mandatory, type `int`): the function's domain dimension.
- `seed` (type `int`, default 0): randome number generator seed, for reproducibility.
- `lb` (type `list[float]`, default `[0.0]*dimension`): domain lower bounds.
- `ub` (type `list[float]`, default `[1.0]*dimension`): domain upper bounds.
- `fraction` (type `float`, default .1): initial size of the search box as a fraction of the [`lb`,`ub`] interval along each dimension.
- `reducefactor` (type `float`, default .9): search box reduction factor upon failed improvement.
- `expandfactor` (type `float`, default 1.0/`reducefactor`): search box expansion factor upon succeeded improvement.

Both functions return a two-element tuple containing the minimizer coordinates and the corresponding value (`Tuple[List[float],float]`).

### Native library

See `test/test.cpp`:

```c++
#include <iostream>

#include "function.h"
#include "c_io_shaker.h"

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
    double best_x[2];
    // Test the solvers ten times with different seeds
    for (int seed = 0; seed < 10; seed++ ) {
        double best_f = reactive_affine_shaker(f, best_x, seed);
        cout << "BEST AFFINE " << best_f << " @ " << best_x[0] << ',' << best_x[1] << endl;
        best_f = inertial_shaker(f, best_x, seed);
        cout << "BEST INERTIAL " << best_f << " @ " << best_x[0] << ',' << best_x[1] << endl;
    }
    return 0;
}
```

To compile from inside `test`, add `..` and `../libRSO` as include directories and link the object file with `../cbuild/libIOShaker.a`:

```sh
cd test
g++ -c -I.. -I../libRSO test.cpp
g++ -o test -L../cbuild test.o -lIOShaker
./test
```

The two functions `reactive_affine_shaker` and `inertial_shaker` accept a parameter list similar to their Python counterparts:

- `function` (class `Function`): a reference to an object from a subclass of `Function`.
- `best_x` (type `double[function.dimension]`): a double array apt at containing the minimizer's coordinates.
- `seed` (type `int`, default 0): randome number generator seed, for reproducibility.
- `fraction` (type `double`, default .1): initial size of the search box as a fraction of the [`lb`,`ub`] interval along each dimension.
- `reducefactor` (type `double`, default .9): search box reduction factor upon failed improvement.
- `expandfactor` (type `double`, default 1.0/`reducefactor`): search box expansion factor upon succeeded improvement.

The differences from the Python functions are the following:

- `function` is a reference to an object from a subclass of `Function` instead of a generic callable object.
- `best_x` must be provided as argument, while in the Python version it is created and returned by the solver.
- parameters `dimension`, `lb` and `ub` are omitted because the domain is specified in the `dimension`,  `xmin` and `xmax` members of `Function`.

As shown in the example above, the `Function` subclass must define at least two methods:

- a constructor invoking the one from the base class, setting the domain's and codomain's dimension (the latter must be 1), and filling the lower and upper bound arrays;
- a protected `evaluate` function that accepts a constant double array and provides a pointer to a double variable containing the result.
