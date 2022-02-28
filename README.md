# xComp

![xComp icon](https://github.com/gugenstudio/xComp/blob/master/apps/deploy_base/icons/xcomp_icon.png)

**xComp** is an image viewer capable of building composites of a stack of images with transparent regions.

This tool was built to visualize region updates of a render on top of previous renders, in real-time. Images found in a choosen folder to scan for, are quickly composited with alpha blending in a stack. sRGB and tone mapping is optionally added to the composite image, which is also saved out as a PNG.

Note that because ordering of compositing is dictated by file name, images should be titled by a timestamp, or with a sequential number.

**xComp** is also useful to quickly view EXR images and their various layers.

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

- Build files generation `./build.sh -d`
- Binaries generation `./build.sh`

- Build files, such as Makefile or the VS solution, are found in `_build`.
- Build products are generated in `_bin`.

For a test run:

1. `cd apps/debug_dir`
1. `../../_bin/xcomp`

## Libraries upgrade

Launch: `scripts/manage_dependency_libaries.sh --update`

