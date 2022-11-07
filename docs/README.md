# xComp

![xComp icon](xcomp_sshot_01.jpg)

## Overview

**xComp** is an image viewer capable of building composites of a stack of images with transparent regions.

This tool was built to visualize region updates of a render on top of previous renders, in real-time. Images found in a chosen folder to scan for, are quickly composited with alpha blending in a stack. OpenColorIO transformations are optionally added to the composite image, which can be saved out as a PNG.

Note that because ordering of compositing is dictated by file name, images should be titled by a timestamp, or with a sequential number.

**xComp** is also useful to quickly view EXR images and their various layers.

## Quick Start

### How to setup xComp

There are three possible ways:

1. Drag and drop your first render into **xComp**. Notice that it may take a few seconds to load, if there are serveral images in the folder.
1. Click on the *Config...* button in the *Display* panel, or select *File->Configuration* from the menu, then paste the path of the folder with your images into the ***Scan Folder*** edit box.
1. If you are on Windows, manually edit `mt_compare_config.json` found inside `%APPDATA%\Xcomp\profile_default`.

### Correctly naming the images from the renderer

**xComp** determines the order of the layers based on the numbering of the rendered images.
For this reason, we suggest saving the renders with a numerical postfix like `_001`, `_002`, etc..

