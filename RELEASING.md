# Releasing xComp locally

This project uses a local macOS release flow so Apple signing and notarization
credentials do not need to be stored in GitHub.

## Requirements

- `gh` authenticated for the target GitHub repo
- Apple signing certificates installed in your macOS keychain
- notarization credentials available either:
  - via environment variables, or
  - via the local `pass` entries already used by sibling projects

## Typical release flow

1. Update the version in `apps/src_common/GTVersions.h`.
2. Add a short release note entry in `apps/deploy_base/history.txt`.
3. Commit the release bump.
4. Run:

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
