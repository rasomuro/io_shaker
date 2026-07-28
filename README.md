# pylion

`pylion` provides Python bindings for the C++ RSO optimization library.

## Install

### Python package

A C++ compiler and Python development headers are required when installing from
the source tree or a source distribution. SWIG is not required because its
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
cmake -S . -B build
cmake --build build --config Release
```

## Usage and example

### Python library

See `test/test.py`.

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

### Native library

See `test/test.cpp`. To compile, compile with `..` and `../libRSO` as include directories and link the object file with `../cbuild/libRSO.a`:

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
