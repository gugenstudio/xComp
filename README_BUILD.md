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

### Windows build on GitHub Actions
The workflow at [windows-build.yml](/Users/davide/dev/repos/xComp/.github/workflows/windows-build.yml)
builds the project on a GitHub-hosted Windows runner, installs NSIS, runs the
existing `./build.sh -p` packaging path, and uploads the generated installer as
a workflow artifact.

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

### Upload a local package to GitHub Releases
After building the package locally, you can upload it with:
```
./build.sh -p -s -l
```

This uses the local `gh` CLI session and will:
- infer the GitHub repo from `origin` when possible
- infer the release tag from the current commit when `HEAD` is exactly on a tag
- create the release if it does not exist yet
- upload or replace the package asset if it already exists

Optional environment variables:
- `GITHUB_RELEASE_REPO` (`owner/name`)
- `GITHUB_RELEASE_TAG`
- `GITHUB_RELEASE_TITLE`

For the full local release checklist, see [RELEASING.md](/Users/davide/dev/repos/xComp/RELEASING.md).

### Do a test-run
```
cd apps/debug_dir
../../_bin/xcomp
```

## Libraries upgrade

Launch: `scripts/manage_dependency_libaries.sh --update`
