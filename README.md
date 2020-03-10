# boost-gil-checksum-bug

TL;TR: Why Boost.GIL tests based on checksum for bit-aligned pixels fail on MSVC 64-bit optimized builds?

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
