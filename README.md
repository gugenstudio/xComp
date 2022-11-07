![xComp icon](docs/xcomp_sshot_01.jpg)

**xComp** is an image viewer capable of building composites of a stack of images with transparent regions.

This tool was built to visualize region updates of a render on top of previous renders, in real-time. Images found in a chosen folder to scan for, are quickly composited with alpha blending in a stack. OpenColorIO transformations are optionally added to the composite image, which can be saved out as a PNG.

## User Manual

The user documentation can be found at [docs/README.md](https://github.com/gugenstudio/xComp/blob/master/docs/README.md)

## Building Requirements

### General
- git
- CMake (>= 3.7)

### Windows
- Git Bash
- Visual Studio 2019

### Linux
- gcc

## Setup

1. `git submodule update --init --recursive`
1. (On Linux) `sudo apt update`
1. `scripts/manage_dependency_libaries.sh`

## Building

### Build OpenColorIO
```
pushd externals/local/ocio
./build_ocio.sh
popd
```

### Generate the build files
```
./build.sh -d
```
Generated Makefile or the VS solution, are found in `_build`.

### Generate the binaries
```
./build.sh
```
Binaries are generated in `_bin`.

### Do a test-run
```
cd apps/debug_dir
../../_bin/xcomp
```

## Libraries upgrade

Launch: `scripts/manage_dependency_libaries.sh --update`

