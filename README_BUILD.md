## Building Requirements

### General
- git
- CMake (>= 3.7)
- On macOS, Homebrew packages for `zlib`, `yaml-cpp`, and `imath` are recommended

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

On Apple Silicon Macs, `build_ocio.sh` will use Homebrew dependency prefixes
when they are available, which avoids several old Intel-era path assumptions.

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

### Create a signed and notarized macOS package
```
./build.sh -p -s
```

The signing/notarization flow supports:
- `MACOS_SIGN_IDENTITY_APP`
- `MACOS_SIGN_IDENTITY_INSTALLER`
- `MACOS_NOTARY_APPLE_ID`
- `MACOS_NOTARY_APP_PASSWORD`
- `MACOS_NOTARY_TEAM_ID`
- `MACOS_NOTARY_KEYCHAIN_PROFILE` (optional alternative to Apple ID + app password)

If the `pass` command is configured locally, `MACOS_NOTARY_APPLE_ID` and
`MACOS_NOTARY_APP_PASSWORD` can also be resolved from the same Apple entries
used by sibling projects.

### GitHub Releases automation for macOS Apple Silicon
The workflow at [`.github/workflows/release-macos.yml`](.github/workflows/release-macos.yml)
builds an Apple Silicon package on tag push, notarizes it, staples it, and
uploads it to the matching GitHub Release.

Required GitHub Actions secrets:
- `MACOS_CERTIFICATES_P12_BASE64`
- `MACOS_CERTIFICATES_P12_PASSWORD`
- `MACOS_KEYCHAIN_PASSWORD`
- `MACOS_SIGN_IDENTITY_APP`
- `MACOS_SIGN_IDENTITY_INSTALLER`
- `MACOS_NOTARY_APPLE_ID`
- `MACOS_NOTARY_APP_PASSWORD`
- `MACOS_NOTARY_TEAM_ID`

### Do a test-run
```
cd apps/debug_dir
../../_bin/xcomp
```

## Libraries upgrade

Launch: `scripts/manage_dependency_libaries.sh --update`
