#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     MACHINE=linux;;
    Darwin*)    MACHINE=macos;;
    CYGWIN*)    MACHINE=win;;
    MSYS*)      MACHINE=win;;
    MINGW*)     MACHINE=win;;
    *)          echo "unsupported architecture"; exit 1
esac

function doMake() {

    TYPE=$1
    CMAKE_ARGS=(
        -DBUILD_SHARED_LIBS=ON
        -DCMAKE_BUILD_TYPE=${TYPE}
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DOCIO_BUILD_APPS=OFF
        -DOCIO_BUILD_TESTS=OFF
        -DOCIO_BUILD_GPU_TESTS=OFF
        -DOCIO_BUILD_PYTHON=OFF
        -DOCIO_BUILD_JAVA=OFF
        -DOCIO_BUILD_DOCS=OFF
        -DOCIO_WARNING_AS_ERROR=OFF
    )

    if [ "$(uname -m)" = "arm64" ]; then
        CMAKE_ARGS+=(-DOCIO_USE_SSE=OFF)
    else
        CMAKE_ARGS+=(-DOCIO_USE_SSE=ON)
    fi

    if [ "${MACHINE}" = "macos" ] && command -v brew >/dev/null 2>&1; then
        declare -a BREW_PREFIXES=()

        if ZLIB_ROOT="$(brew --prefix zlib 2>/dev/null)"; then
            BREW_PREFIXES+=("${ZLIB_ROOT}")
            CMAKE_ARGS+=(-DZLIB_ROOT="${ZLIB_ROOT}")
        fi

        if IMATH_ROOT="$(brew --prefix imath 2>/dev/null)"; then
            BREW_PREFIXES+=("${IMATH_ROOT}")
            CMAKE_ARGS+=(-DImath_ROOT="${IMATH_ROOT}")
        fi

        if YAML_CPP_ROOT="$(brew --prefix yaml-cpp 2>/dev/null)"; then
            BREW_PREFIXES+=("${YAML_CPP_ROOT}")
            CMAKE_ARGS+=(-Dyaml-cpp_ROOT="${YAML_CPP_ROOT}")
        fi

        if [ ${#BREW_PREFIXES[@]} -gt 0 ]; then
            BREW_PREFIX_PATH="$(IFS=';'; echo "${BREW_PREFIXES[*]}")"
            CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="${BREW_PREFIX_PATH}")
        fi
    fi

    echo -e "\e[1m*** Buidling ${TYPE} build... \e[0m"

    mkdir -p _build/${TYPE}
    pushd _build/${TYPE}

    cmake "${CMAKE_ARGS[@]}" ../../../../ocio

    cmake --build . --config ${TYPE} -j 6

    popd
}

# assuming it's Visual Studio in Windows
if [ "${MACHINE}" == "win" ]; then
    doMake "Debug"
fi

doMake "Release"
