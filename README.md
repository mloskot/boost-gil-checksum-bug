# boost-gil-checksum-bug

> TL;TR: Why Boost.GIL [checksum tests for bit-aligned image](https://github.com/boostorg/gil/blob/2ff2d31895dae665ef3e05d0496bc082470e229c/test/legacy/image.cpp#L373-L381) fail on MSVC 64-bit optimized builds?

[![clang](https://github.com/mloskot/boost-gil-checksum-bug/workflows/clang/badge.svg)](https://github.com/mloskot/boost-gil-checksum-bug/actions?query=workflow:clang)
[![gcc](https://github.com/mloskot/boost-gil-checksum-bug/workflows/gcc/badge.svg)](https://github.com/mloskot/boost-gil-checksum-bug/actions?query=workflow:gcc)
[![msvc](https://github.com/mloskot/boost-gil-checksum-bug/workflows/msvc/badge.svg)](https://github.com/mloskot/boost-gil-checksum-bug/actions?query=workflow:msvc)
[![Build Status](https://dev.azure.com/mloskot/boost-gil-checksum-bug/_apis/build/status/mloskot.boost-gil-checksum-bug?branchName=master)](https://dev.azure.com/mloskot/boost-gil-checksum-bug/_build/latest?definitionId=2&branchName=master)

## Boost.GIL

This applies to number of versions, as checked:

- Boost.GIL `develop` branch
- Boost 1.67 through 1.72 (current release)

## Test

The [minimal_test.cpp](minimal_test.cpp) performs the following test:
1. Create 3x3 image based on bit-aligned pixel (BGR121)
2. Fill image with red
3. Draw blue diagonal

## Observation

Pixels are accessed using `xy_locator` which gives access `x_iterator` and `y_iterator`.

Pixels are iterated using `for` loop:
  - if `x_iterator` is incremented *before* the `y_iterator`, then the blue diagonal is garbled.
  - if `x_iterator` is incremented *after* the `y_iterator`, then the blue diagonal is drawn correctly - **Where is the BUG?!**

Pixels are iterated hand-rolled iteration without `for` loop:
  - regardless if `++loc.x()` is before or after `--loc.y()`, the blue diagonal is drawn correctly.

## Mystery

The bug described above leaks **only and only** with 64-bit optimised build using  MSVC++ 14.x

```console
b2 toolset=msvc variant=release address-model=64 cxxstd=11,14,17,2a
```

It is not observed using all these numerous tested compilers:

  - GCC: 4.8, 4.9, 5, 6, 7, 8, 9
  - Clang: 3.9, 4.0, 5.0, 6.0, 7, 8, 9, 10
  - XCode: 8.3, 9.0, 9.1, 9.2, 9.3, 9.4, 10

## Run with Boost.Build

```console
b2.exe -sBOOST_ROOT=D:\boost.win toolset=msvc cxxstd=11 variant=release,debug address-model=32,64 minimal
```

## Run with CMake

```console
cmake -G "Visual Studio 16 2019" -A Win32 -B build32 -DBOOST_ROOT=D:/boost.win
cmake -G "Visual Studio 16 2019" -A x64   -B build64 -DBOOST_ROOT=D:/boost.win

set CTEST_OUTPUT_ON_FAILURE=1
for %B in (32,64) do ( for %C in (Debug,Release) do (
    cmake --build build%B --config %C && cmake --build build%B --config %C --target RUN_TESTS
))
```
