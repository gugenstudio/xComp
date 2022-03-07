#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     MACHINE=linux;;
    Darwin*)    MACHINE=macos;;
    CYGWIN*)    MACHINE=win;;
    MINGW*)     MACHINE=win;;
    *)          echo "unsupported architecture"; exit 1
esac

BASHSCRIPTDIR="$(cd "$(dirname "$0")" || exit; pwd)"
ROOTDIR=${BASHSCRIPTDIR}
SOURCEDIR=${BASHSCRIPTDIR}
BUILDDIR=${ROOTDIR}/_build/${MACHINE}
SOURCEDIR=${ROOTDIR}/apps
SOURCE2DIR=${ROOTDIR}/libs
NBFILES=${BUILDDIR}/.nbfiles

displayusage() {
    echo " ================================================================================= "
    echo "|    Usage:                                                                       |"
    echo "| do_build.sh OPTS                                                                |"
    echo "|    available options [OPTS]:                                                    |"
    echo "| -b) --build)          automatically updates the build when necessary            |"
    echo "| -c) --clean)          removes build dirs                                        |"
    echo "| -d) --dry-run)        creates the make file without building                    |"
    echo "| -f) --force)          forces an update of the build                             |"
    echo "| -h) --help)           print this help                                           |"
    echo "| -l) --deploy)         deploys the binaries remotely                             |"
    echo "| -m) --make)           performs make                                             |"
	echo "| -n) --nproc)          sets the number of parallel processing (default nproc -1) |"
    echo "| -p) --package)        creates a package without dir removal                     |"
    echo "| -t) --build-type)     specifies a different cmake build type (e.g. \"-t -Debug\") |"
    echo "| -w) --cmake-params)   specifies cmake options in quoted (e.g. \"-DVAR=value\")    |"
	echo "| [no arguments]        automatically updates the build when necessary            |"
    echo " ================================================================================= "
}

update_makefiles(){
    mkdir -p "${BUILDDIR}"
    cd "${BUILDDIR}" || exit

	if [ "${MACHINE}" == "macos" ]; then
        cmake "${ROOTDIR}" -DCMAKE_BUILD_TYPE="${BUILDTYPE:-Release}" ${CMAKEOPTS:+$CMAKEOPTS}
    elif [ "${MACHINE}" == "linux" ]; then
        cmake "${ROOTDIR}" -DCMAKE_BUILD_TYPE="${BUILDTYPE:-Release}" ${CMAKEOPTS:+$CMAKEOPTS}
    elif [ "${MACHINE}" == "win" ]; then
        cmake -G "Visual Studio 16 2019" -A x64 "${ROOTDIR}" -DCMAKE_BUILD_TYPE="${BUILDTYPE:-Release}" ${CMAKEOPTS:+$CMAKEOPTS}
    fi
    CMAKE_RET=$?
	if [ $CMAKE_RET -ne 0 ] ; then exit ${CMAKE_RET}; fi
    cd - || exit
    MAKEFILEUPDATED="TRUE"
}

build(){
    if [[ ! -f "${NBFILES}" ]] ; then mkdir -p "${BUILDDIR}";  echo 0 > "${NBFILES}" ; fi
    PREVFILES=$(<"${NBFILES}")
    CURRFILES1=$(find "${SOURCEDIR}" |  wc -l)
    CURRFILES2=$(find "${SOURCE2DIR}" |  wc -l)
    CURRFILES=$((CURRFILES1 + CURRFILES2))

    if [[ "${MAKEFILEUPDATED}" == "TRUE" ]] || [[ "${CURRFILES}" -ne "${PREVFILES}" ]] ; then update_makefiles ; fi

	if [ "${MACHINE}" == "win" ]; then
		cmake --build "${BUILDDIR}" --config "${BUILDTYPE:-Release}" -- -m:"${NPROC}"
	else
		cmake --build "${BUILDDIR}" --config "${BUILDTYPE:-Release}" -j "${NPROC}"
	fi
    CMAKE_RET=$?
    echo "${CURRFILES}" > "${NBFILES}"
	if [ $CMAKE_RET -ne 0 ] ; then exit ${CMAKE_RET}; fi
}

emake(){
    cd "${BUILDDIR}" || exit ; make -j"${NPROC}"
}

create_package_linux(){
	PACKAGELINUXDIR=_tmp/xcomp
	VERSION_ID="$(awk '$2 == "GTV_SUITE_VERSION" {print substr($3,2,length($3)-2)}' apps/src_common/GTVersions.h)"

	rm -rf _tmp/xcomp
	mkdir -p ${PACKAGELINUXDIR}/bin
	mkdir -p ${PACKAGELINUXDIR}/redistr
	declare -a LINUXREDISTRS=('libGL' 'libOpenGL')
	for LINUXRLIB in "${LINUXREDISTRS[@]}"; do
		LINUXDEPITMP=$(ldd _bin/xcomp | grep "${LINUXRLIB}*" | awk 'BEGIN{ORS=" "}$1~/^\//{print $1}$3~/^\//{print $3}')
		read -ra LINUXDEPIARR -d '' <<<"${LINUXDEPITMP}"
		for LINUXDEPI in "${LINUXDEPIARR[@]}"; do cp "${LINUXDEPI%"${LINUXDEPI##*[![:space:]]}"}" ${PACKAGELINUXDIR}/redistr; done
	done

	strip -s _bin/xcomp -o ${PACKAGELINUXDIR}/bin/xcomp
	chrpath -d ${PACKAGELINUXDIR}/bin/xcomp

    # TODO: tentative name, to be replaced once we have the actual lib name
    strip -s _bin/OpenColorIO_2_2.so -o ${PACKAGELINUXDIR}/bin/OpenColorIO_2_2.so
	chmod 644 ${PACKAGELINUXDIR}/bin/OpenColorIO_2_2.so

	mkdir -p ${PACKAGELINUXDIR}/share/icons
	mkdir -p ${PACKAGELINUXDIR}/share/fonts
	cp apps/deploy_base/icons/* ${PACKAGELINUXDIR}/share/icons
	cp apps/deploy_base/fonts/* ${PACKAGELINUXDIR}/share/fonts

	cp apps/deploy_base/history.txt ${PACKAGELINUXDIR}
	cp apps/deploy_base/license.txt ${PACKAGELINUXDIR}
	cp apps/deploy_base/run_linux.sh ${PACKAGELINUXDIR}/run.sh

	pushd ${PACKAGELINUXDIR}/.. || exit
	tar -zcvf xcomp_"${VERSION_ID}"-"$(uname -m)".tar.gz xcomp
	popd || exit
}

create_package_macos(){
	PACKAGEMACOSDIR=_tmp/xcomp
	VERSION_ID="$(awk '$2 == "GTV_SUITE_VERSION" {print substr($3,2,length($3)-2)}' apps/src_common/GTVersions.h)"

	rm -rf _tmp/xcomp _tmp/xcomp.dmg _tmp/xcomp*.command
	mkdir -p ${PACKAGEMACOSDIR}/bin
	mkdir -p ${PACKAGEMACOSDIR}/share
	mkdir -p ${PACKAGEMACOSDIR}/share/icons
	mkdir -p ${PACKAGEMACOSDIR}/share/fonts
	mkdir -p ${PACKAGEMACOSDIR}/redistr

	strip -o ${PACKAGEMACOSDIR}/bin/xcomp _bin/xcomp

    # TODO: tentative name, to be replaced once we have the actual lib name
	cp _bin/OpenColorIO_2_2.dylib ${PACKAGEMACOSDIR}/bin/

	cp apps/deploy_base/icons/* ${PACKAGEMACOSDIR}/share/icons
	cp apps/deploy_base/fonts/* ${PACKAGEMACOSDIR}/share/fonts
	cp apps/deploy_base/history.txt ${PACKAGEMACOSDIR}
	cp apps/deploy_base/license.txt ${PACKAGEMACOSDIR}
	cp apps/deploy_base/run_macos.sh ${PACKAGEMACOSDIR}/run.sh
	cp apps/deploy_base/run_macos.sh ${PACKAGEMACOSDIR}/run.command
	sed "s/XCOMP_VERSION/${VERSION_ID}-$(uname -m)/g" apps/deploy_base/deploy_macos.sh > "_tmp/xcomp_${VERSION_ID}-$(uname -m).command"
	chmod 755 "_tmp/xcomp_${VERSION_ID}-$(uname -m).command"

	create-dmg --volname "xcomp" --volicon "apps/resources/xcomp_icon.icns" "_tmp/xcomp_${VERSION_ID}-$(uname -m).dmg" "${PACKAGEMACOSDIR}"
}

create_package_win(){
    PACKAGEWIN32DIR=_tmp/xcomp

    rm -rf _tmp/xcomp
    mkdir -p ${PACKAGEWIN32DIR}/bin
    mkdir -p ${PACKAGEWIN32DIR}/share
    mkdir -p ${PACKAGEWIN32DIR}/share/icons
    mkdir -p ${PACKAGEWIN32DIR}/share/fonts
    mkdir -p ${PACKAGEWIN32DIR}/other

    cp _bin/Release/xcomp* ${PACKAGEWIN32DIR}/bin

    cp _bin/Release/OpenColorIO_2_2.dll ${PACKAGEWIN32DIR}/bin

    cp apps/deploy_base/icons/* ${PACKAGEWIN32DIR}/share/icons
    cp apps/deploy_base/fonts/* ${PACKAGEWIN32DIR}/share/fonts
    cp apps/deploy_base/other/* ${PACKAGEWIN32DIR}/other
    cp apps/deploy_base/history.txt ${PACKAGEWIN32DIR}
    cp apps/deploy_base/license.txt ${PACKAGEWIN32DIR}

    cp apps/deploy_base/xcomp_installer.nsi ${PACKAGEWIN32DIR}/..

    pushd ${PACKAGEWIN32DIR}/.. || exit
    /c/Program\ Files\ \(x86\)/NSIS/makensis.exe xcomp_installer.nsi
    rm xcomp_installer.nsi
    popd || exit
}

for arg in "$@"; do
	shift
	case "$arg" in
		"--build")          set -- "$@" "-b" ;;
		"--clean")          set -- "$@" "-c" ;;
		"--dry-run")        set -- "$@" "-d" ;;
		"--force")          set -- "$@" "-f" ;;
		"--help")           set -- "$@" "-h" ;;
		"--deploy")         set -- "$@" "-l" ;;
		"--make")           set -- "$@" "-m" ;;
		"--nproc")          set -- "$@" "-n" ;;
		"--package")        set -- "$@" "-p" ;;
		"--package-server") set -- "$@" "-q" ;;
		"--build-type")     set -- "$@" "-t" ;;
		"--cmake-params")   set -- "$@" "-w" ;;
		*)                  set -- "$@" "$arg";;
	esac
done

# Parse short options
OPTIND=1
while getopts "bcdfhilmn:pqrt:w:?" opt
do
	case "$opt" in
		"b") build; exit 0;;
		"c") CLEANBUILD="TRUE";;
		"d") UPDATEMAKEFILES="TRUE"; DRY_RUN="TRUE" ;;
		"f") UPDATEMAKEFILES="TRUE";;
		"h") displayusage; exit 0;;
		"m") emake; exit 0;;
		"l") DEPLOY_PACKAGE="TRUE";;
		"n") NPROC=${OPTARG};;
		"p") CREATE_PACKAGE="TRUE"; UPDATEMAKEFILES="TRUE"; git pull;  git submodule update --init --recursive;;
		"t") BUILDTYPE=${OPTARG}; UPDATEMAKEFILES="TRUE";;
		"w") CMAKEOPTS+="${OPTARG} "; UPDATEMAKEFILES="TRUE";;
        "?") displayusage; exit 0;;
    esac
done

shift "$((OPTIND-1))"

if [[ -z "${NPROC}" ]]; then
	if [ "${MACHINE}" == "win" ]; then
		(( NPROC = 3 ))
	else
		(( NPROC = $(nproc) - 1 ))
	fi
fi


if [[ "${CLEANBUILD}" == "TRUE" ]] ; then rm -rf _bin _build; fi

if [[ "${UPDATEMAKEFILES}" == "TRUE" ]] ; then update_makefiles; fi

if [[ "${DRY_RUN}" == "TRUE" ]] ; then  exit 0; fi

build

if [[ "${CREATE_PACKAGE}" == "TRUE" ]] ; then
	if [ "${MACHINE}" == "macos" ]; then
		create_package_macos
	elif [ "${MACHINE}" == "linux" ]; then
		create_package_linux
	elif [ "${MACHINE}" == "win" ]; then
		create_package_win
	else
		echo "unsupported architecture for packaging"
	fi
fi

if [[ "${DEPLOY_PACKAGE}" == "TRUE" ]] ; then
	if [ "${MACHINE}" == "macos" ]; then
		#scp -P PORT _tmp/xcomp_*.command SERVER:PATH
		#scp -P PORT _tmp/xcomp_*.dmg SERVER:PATH
        echo "Missing deploy for macOS"
	elif [ "${MACHINE}" == "win" ]; then
		#scp -P PORT _tmp/xcomp_*_Setup.exe SERVER:PATH
        echo "Missing deploy for Windows"
	else
		#scp -P PORT _tmp/xcomp_*tar.gz SERVER:PATH
        echo "Missing deploy for Linux"
	fi
fi

