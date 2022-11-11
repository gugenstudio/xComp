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

