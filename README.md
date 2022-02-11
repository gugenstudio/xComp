# xComp

## Requirements
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

- Build files generation `./build.sh -d`
- Binaries generation `./build.sh`

- Build files, such as Makefile or the VS solution, are found in `_build`.
- Build products are generated in `_bin`.

For a test run:

1. `cd apps/debug_dir`
1. `../../_bin/xcomp`

## Libraries upgrade

Launch: `scripts/manage_dependency_libaries.sh --update`

