#!/bin/bash
set -euo pipefail

ROOTDIR="$(cd "$(dirname "$0")/.." || exit; pwd)"
cd "${ROOTDIR}" || exit

VERSION_ID="$(awk '$2 == "GTV_SUITE_VERSION" {print substr($3,2,length($3)-2)}' apps/src_common/GTVersions.h)"
DEFAULT_TAG="v${VERSION_ID}"

usage() {
    echo "Usage: scripts/release_macos.sh <tag>"
    echo "Example: scripts/release_macos.sh v1.1.8"
}

find_identity() {
    IDENTITY_KIND="$1"
    security find-identity -v 2>/dev/null | \
        sed -n "s/.*\"\\(${IDENTITY_KIND}: [^\"]*\\)\"/\\1/p" | \
        head -n 1
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

RELEASE_TAG="${1:-}"

if [[ -z "${RELEASE_TAG}" ]]; then
    echo "A release tag is required."
    echo "Current app version is ${VERSION_ID}; a sensible next tag might be ${DEFAULT_TAG} if it is new."
    usage
    exit 1
fi

if [[ -z "${MACOS_SIGN_IDENTITY_APP:-}" ]]; then
    MACOS_SIGN_IDENTITY_APP="$(find_identity "Developer ID Application")"
fi

if [[ -z "${MACOS_SIGN_IDENTITY_INSTALLER:-}" ]]; then
    MACOS_SIGN_IDENTITY_INSTALLER="$(find_identity "Developer ID Installer")"
fi

if [[ -z "${MACOS_SIGN_IDENTITY_APP:-}" ]]; then
    echo "Could not auto-detect a Developer ID Application certificate."
    echo "Set MACOS_SIGN_IDENTITY_APP explicitly and try again."
    exit 1
fi

if [[ -z "${MACOS_SIGN_IDENTITY_INSTALLER:-}" ]]; then
    echo "Could not auto-detect a Developer ID Installer certificate."
    echo "Set MACOS_SIGN_IDENTITY_INSTALLER explicitly and try again."
    exit 1
fi

export MACOS_SIGN_IDENTITY_APP
export MACOS_SIGN_IDENTITY_INSTALLER
export GITHUB_RELEASE_TAG="${RELEASE_TAG}"
export GITHUB_RELEASE_TITLE="xComp ${VERSION_ID}"

"${ROOTDIR}/build.sh" -p -s -l
