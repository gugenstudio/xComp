# xComp

![xComp icon](https://github.com/gugenstudio/xComp/blob/master/apps/deploy_base/icons/xcomp_icon.png)

**xComp** is an image viewer capable of building composites of a stack of images with transparent regions.

This tool was built to visualize region updates of a render on top of previous renders, in real-time. Images found in a choosen folder to scan for, are quickly composited with alpha blending in a stack. sRGB and tone mapping is optionally added to the composite image, which is also saved out as a PNG.

Note that because ordering of compositing is dictated by file name, images should be titled by a timestamp, or with a sequential number.

**xComp** is also useful to quickly view EXR images and their various layers.

**How to setup xComp**

There are 3 ways:

- Drag and drop your first render into xComp, if the folder is full of images it can take few second to load.
- under the *Display* window there is the *config* button, press it to assign the image's folder location, or under File/Configuration, in "scan folder" paste the image's path.
- If you are on Windows edit the `mt_compare_config.json` file inside C:\Users\***\AppData\Roaming\Xcomp\profile_default

**How to save correctly your renders**

xComp use the saved image's numbering to define the correct order of the layers.
So in my experience it's better to save the images with this forma _001, _002 etc..


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

