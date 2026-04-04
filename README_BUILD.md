## Building Requirements

### General
- git
- CMake (>= 3.7)
- On macOS, Homebrew packages for `zlib`, `yaml-cpp`, and `imath` are recommended

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

On Apple Silicon Macs, `build_ocio.sh` will use Homebrew dependency prefixes
when they are available, which avoids several old Intel-era path assumptions.

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
