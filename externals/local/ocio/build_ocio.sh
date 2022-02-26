#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     MACHINE=linux;;
    Darwin*)    MACHINE=macos;;
    CYGWIN*)    MACHINE=win;;
    MINGW*)     MACHINE=win;;
    *)          echo "unsupported architecture"; exit 1
esac

mkdir -p _build
pushd _build

cmake   -DBUILD_SHARED_LIBS=OFF                 \
        -DOCIO_BUILD_APPS=OFF                   \
        -DOCIO_BUILD_TESTS=OFF                  \
        -DOCIO_BUILD_GPU_TESTS=OFF              \
        -DOCIO_BUILD_PYTHON=OFF                 \
        -DOCIO_BUILD_JAVA=OFF                   \
        -DOCIO_BUILD_DOCS=OFF                   \
        -DOCIO_WARNING_AS_ERROR=OFF             \
        -DOCIO_USE_SSE=OFF                      \
        ../../../ocio

# assuming it's Visual Studio in Windows
if [ "${MACHINE}" == "win" ]; then
    cmake --build . --config Debug   -j 6
fi

cmake --build . --config Release -j 6

popd

