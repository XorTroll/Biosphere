# Biosphere

Biosphere (previously a devkitA64 homebrew library) is a homebrew toolchain using LLVM ans clang.

**IMPORTANT!** do not attempt to use this for homebrew development yet, since the libraries are still under development.

## libbio

Biosphere's homebrew library is named `libbio`.

Building this library will generate 3 library objects: `libbio-NSO.a`,`libbio-NRO.a` and `libbio-libNRO.a`.

## Building

Clone with `git clone --recurse-submodules` and run `make`.

### Required dependencies (for building the toolchain)

- Ubuntu/Linux is suggested. For Windows users, setpup WSL.  I (XorTroll) personally compile im Windows, with WSL/Ubuntu.

- Install `llvm`, `llvm-config`, `clang`, `cmake` and `make` (if problems happen, try to install `autoconf`).

Run `make`. This will generate (after compiling everything) a `Biosphere` directory with everything needed for homebrew development with Biosphere/libbio.

## Developing homebrew

Download the latest release. Will contain the built toolchain inside `Biosphere` directory.

Set `BIOSPHERE_ROOT` environment variable to the directory.

In order to build homebrew, check the test project in `test`.

## Credits

- **libtransistor** project and team for the base of using LLVM for homebrew development. This project is a fork of `libtransistor-base` in the end :P

- **linkle** project, since it's the tool used for NRO/NSO generation.

- **switchbrew** and **libnx** as a base for resources, since they also are the base for the old [Biosphere project](https://github.com/XorTroll/Biosphere-old), whose code is being ported/reused for this new project.