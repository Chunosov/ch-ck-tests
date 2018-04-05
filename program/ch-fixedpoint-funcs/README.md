# Fixed-point functions

This program contains a set of function for fixed-point arithmetics wich are extracted from [gemmlowp](https://github.com/google/gemmlowp) library and can be used without it.

Extracted function names are prefixed with `fp_`.

The program provides tests for comparison these functions results with original functions results.

## Requirements
Gemmlowp library
```
ck install package --tags=lib,gemmlowp
```
**TODO:** This package may be unavailable for public because of it is in private repo.

## Build
```
ck compile program:ch-fixedpoint-funcs
```

## Run
```
ck run program:ch-fixedpoint-funcs
```

## TODO
- Make separate commands for each test.
