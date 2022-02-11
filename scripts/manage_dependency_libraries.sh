#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     MACHINE=linux;;
    Darwin*)    MACHINE=macos;;
    MINGW*)     MACHINE=win;;
    *)          exit 1
esac

BASHSCRIPTDIR=$(cd "$(dirname "$0")" || exit; pwd)

displayusage() {
    echo " ======================================= "
    echo "|    Usage:                             |"
    echo "| manage_dependency_libaries.sh OPTS    |"
    echo "|    available options [OPTS]:          |"
    echo "| -h) --help)     print this help       |"
    echo "| -u) --update)   update the Libraries  |"
    echo "| [no arguments]  install the Libraries |"
    echo " ======================================= "
}

for arg in "$@"; do
	shift
	case "$arg" in
		"--help")          set -- "$@" "-h" ;;
		"--update")        set -- "$@" "-u" ;;
		*)                 set -- "$@" "$arg";;
	esac
done

# Parse short options
OPTIND=1
while getopts "hu?" opt
do
	case "$opt" in
		"h") displayusage; exit 0;;
		"u") UPDATE="TRUE" ;;
        "?") displayusage; exit 0;;
    esac
done

shift "$((OPTIND-1))"

if [[ "${UPDATE}" != "TRUE" ]] ; then
    if [ "${MACHINE}" == "macos" ]; then
        brew install cmake
    elif [ "${MACHINE}" == "linux" ]; then
        sudo apt-get install cmake
        sudo apt-get install build-essential
        sudo apt-get install xorg-dev
        sudo apt-get install libglu1-mesa-dev
    elif [ "${MACHINE}" == "win" ]; then
        echo "All done"
    else
        echo "unsupported architecture"
    fi
else
    if [ "${MACHINE}" == "macos" ]; then
        brew upgrade cmake
    elif [ "${MACHINE}" == "linux" ]; then
        sudo apt-get --only-upgrade install cmake
        sudo apt-get --only-upgrade install build-essential
        sudo apt-get --only-upgrade install xorg-dev
        sudo apt-get --only-upgrade install libglu1-mesa-dev
    elif [ "${MACHINE}" == "win" ]; then
        echo "All done"
    else
        echo "unsupported architecture"
    fi
fi
