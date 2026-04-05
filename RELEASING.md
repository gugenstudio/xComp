# Releasing xComp locally

This project uses a local macOS release flow so Apple signing and notarization
credentials do not need to be stored in GitHub.

Linux release assets are produced separately by GitHub Actions on
`ubuntu-22.04`, which is the current Linux CI and release baseline.

## One-page checklist

Use this exact order for a normal macOS release:

```bash
# 1. Update release metadata
$EDITOR apps/src_common/GTVersions.h
$EDITOR apps/deploy_base/history.txt

# 2. Commit the release bump
git add apps/src_common/GTVersions.h apps/deploy_base/history.txt
git commit -m "Bump version to X.Y.Z"

# 3. Create and publish the tag first
git tag vX.Y.Z
git push origin master vX.Y.Z

# 4. Build, sign, notarize, staple, and upload the macOS package
scripts/release_macos.sh vX.Y.Z
```

Expected output artifact:

- `_tmp/xcomp_X.Y.Z-arm64.pkg`

Tagged releases can also receive:

- `xcomp_X.Y.Z-x86_64.tar.gz` from [linux-build.yml](/Users/davide/dev/repos/xComp/.github/workflows/linux-build.yml)
- `xComp_X.Y.Z_Setup.exe` from [windows-build.yml](/Users/davide/dev/repos/xComp/.github/workflows/windows-build.yml)

## Requirements

- `gh` authenticated for the target GitHub repo
- Apple signing certificates installed in your macOS keychain
- notarization credentials available either:
  - via environment variables, or
  - via the local `pass` entries already used by sibling projects

## Automated Linux and Windows release assets

When you push a `v*` tag to GitHub:

1. [linux-build.yml](/Users/davide/dev/repos/xComp/.github/workflows/linux-build.yml)
   builds on `ubuntu-22.04`, packages `xcomp_<version>-x86_64.tar.gz`, and
   attaches it to the GitHub Release.
2. [windows-build.yml](/Users/davide/dev/repos/xComp/.github/workflows/windows-build.yml)
   builds on `windows-2022`, packages `xComp_<version>_Setup.exe`, and
   attaches it to the GitHub Release.

That means the remaining manual release step is the notarized macOS package.

## Typical release flow

1. Update the version in `apps/src_common/GTVersions.h`.
2. Add a short release note entry in `apps/deploy_base/history.txt`.
3. Commit the release bump.
4. Create and push the release tag.
5. Run:

```bash
git tag vX.Y.Z
git push origin master vX.Y.Z
scripts/release_macos.sh vX.Y.Z
```

## What the helper does

`scripts/release_macos.sh`:

- auto-detects a `Developer ID Application` certificate when possible
- auto-detects a `Developer ID Installer` certificate when possible
- builds the app
- creates a signed macOS package
- notarizes it
- staples it
- uploads it to GitHub Releases using your local `gh` session

## Optional environment variables

- `MACOS_SIGN_IDENTITY_APP`
- `MACOS_SIGN_IDENTITY_INSTALLER`
- `MACOS_NOTARY_APPLE_ID`
- `MACOS_NOTARY_APP_PASSWORD`
- `MACOS_NOTARY_TEAM_ID`
- `MACOS_NOTARY_KEYCHAIN_PROFILE`
- `GITHUB_RELEASE_REPO`
- `GITHUB_RELEASE_TAG`
- `GITHUB_RELEASE_TITLE`

None of these values should be committed to the repository.

## Notes on credentials

- The helper tries to auto-detect suitable signing certificates from the local
  macOS keychain.
- If notarization credentials are not provided via environment variables, the
  build script may resolve them from the local `pass` store if it is configured.
- Keep all signing identities, Apple credentials, app-specific passwords, and
  GitHub tokens local to the machine running the release.

## Notes on GitHub Releases

- The helper uses the local `gh` CLI session for upload.
- By default it infers the repository from the `origin` remote.
- The release tag should already exist on GitHub before running the helper.
- If the release does not exist yet, the helper creates it and uploads the
  generated package.
- If the release already exists, the helper uploads the package asset with
  `--clobber`.

## Troubleshooting

- `Environment variable MACOS_SIGN_IDENTITY_APP or MACOS_SIGN_IDENTITY must be defined`
  : no suitable Developer ID Application certificate was found automatically.
- `Environment variable MACOS_SIGN_IDENTITY_INSTALLER or MACOS_SIGN_IDENTITY must be defined`
  : no suitable Developer ID Installer certificate was found automatically.
- `MACOS_NOTARY_* must be defined`
  : notarization credentials were not available via environment or `pass`.
- `Release.target_commitish is invalid`
  : the tag exists locally but has not been pushed to GitHub yet.
- `Package not found`
  : the build/package step did not complete successfully.
