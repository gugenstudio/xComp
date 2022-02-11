#!/bin/bash

M_VERSION=$(sw_vers -productVersion)
IFS='.' read -ra VV <<< "$M_VERSION"
MACOS_VERSION=${VV[0]}.${VV[1]}
SUPPORTED=$(echo ${MACOS_VERSION} 10.130001 | awk '{if ($1 > $2) print "TRUE"; else print "FALSE"}')
if [[ "$SUPPORTED" == "FALSE" ]]; then
	osascript -e 'display alert "Unsupported OS Version" message "Your macOS Version is not supported."'
	exit 1
fi

cd -- "$(dirname "$0")"
BINDIR=$(pwd)/bin
GUGEN_LIBRARY_PATH=$(pwd)"/redistr"
export DYLD_LIBRARY_PATH="${GUGEN_LIBRARY_PATH}:${DYLD_LIBRARY_PATH}"

${BINDIR}/xcomp
