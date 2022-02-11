#!/bin/bash

BASHSCRIPTDIR=$(cd "$(dirname "$0")"; pwd)
GUGEN_LIBRARY_PATH="${BASHSCRIPTDIR}/bin:${BASHSCRIPTDIR}/redistr"
export LD_LIBRARY_PATH="${GUGEN_LIBRARY_PATH}:${LD_LIBRARY_PATH}"

bin/xcomp
