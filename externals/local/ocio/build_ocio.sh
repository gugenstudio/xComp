#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     MACHINE=linux;;
    Darwin*)    MACHINE=macos;;
    CYGWIN*)    MACHINE=win;;
    MINGW*)     MACHINE=win;;
    *)          echo "unsupported architecture"; exit 1
esac

function doMake() {

    TYPE=$1

    echo -e "\e[1m*** Buidling ${TYPE} build... \e[0m"

    mkdir -p _build/${TYPE}
    pushd _build/${TYPE}

    cmake   -DBUILD_SHARED_LIBS=ON                          \
            -DCMAKE_BUILD_TYPE=${TYPE}                      \
            -DOCIO_BUILD_APPS=OFF                           \
            -DOCIO_BUILD_TESTS=OFF                          \
            -DOCIO_BUILD_GPU_TESTS=OFF                      \
            -DOCIO_BUILD_PYTHON=OFF                         \
            -DOCIO_BUILD_JAVA=OFF                           \
            -DOCIO_BUILD_DOCS=OFF                           \
            -DOCIO_WARNING_AS_ERROR=OFF                     \
            -DOCIO_USE_SSE=ON                               \
            ../../../../ocio

    cmake --build . --config ${TYPE} -j 6

    popd
}

# assuming it's Visual Studio in Windows
if [ "${MACHINE}" == "win" ]; then
    doMake "Debug"
fi

doMake "Release"

