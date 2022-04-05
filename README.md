# Boost.Socks.Proto

## This is currently **NOT** an official Boost library.

## Overview

Boost.Socks.Proto is a portable, low-level C++ library which provides containers
and algorithms for implementing the SOCKS protocol described in the document
[rfc1928: SOCKS Protocol Version 5](https://datatracker.ietf.org/doc/html/rfc1928),
and extensions described in [rfc1929](https://datatracker.ietf.org/doc/html/rfc1929),
[rfc1961](https://datatracker.ietf.org/doc/html/rfc1961), and
[rfc3089](https://datatracker.ietf.org/doc/html/rfc3089).

Boost.Socks.Proto offers these features:

* Require only C++11
* Works without exceptions
* Fast compilation, no templates
* Strict compliance with rfc1928

## Requirements

* Requires Boost and a compiler supporting at least C++11
* Aliases for standard types use their Boost equivalents
* Link to a built static or dynamic Boost library, or use header-only (see below)
* Supports -fno-exceptions, detected automatically

### Header-Only

To use the library as header-only; that is, to eliminate the requirement to
link a program to a static or dynamic Boost.Socks.Proto library, simply
place the following line in exactly one new or existing source
file in your project.

```cpp
    #include <boost/socks_proto/src.hpp>
```

### Embedded

Boost.Socks.Proto works great on embedded devices.
It is designed to work without exceptions if desired.

### Supported Compilers

Boost.Socks.Proto has been tested with the following compilers:

* clang: 3.8, 4, 5, 6, 7, 8, 9, 10, 11, 12
* gcc: 4.8, 4.9, 5, 6, 7, 8, 9, 10, 11
* msvc: 14.0, 14.1, 14.2, 14.3

### Quality Assurance

The development infrastructure for the library includes
these per-commit analyses:

* Coverage reports
* Benchmark performance comparisons
* Compilation and tests on Drone.io, Azure Pipelines, Appveyor
* Fuzzing using clang-llvm and machine learning

## Visual Studio Solution Generation

    cmake -G "Visual Studio 16 2019" -A Win32 -B bin -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake
    cmake -G "Visual Studio 16 2019" -A x64 -B bin64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake

## License

Distributed under the Boost Software License, Version 1.0.
(See accompanying file [LICENSE_1_0.txt](LICENSE_1_0.txt) or copy at
https://www.boost.org/LICENSE_1_0.txt)

