# GitHub updates

## Signed Linux acceptance completed (2026-09-17)

Version 2026091701 passed signing, packaged bootstrap trust and native Debian
13.6 KDE/Wayland acceptance, including web/PDF, system credentials across
processes, STEP/OpenGL rendering and relocation with isolated profiles.
See [release acceptance and exact test scope](releases/2026091701.md).
Earlier unsigned/pending statements below describe historical checks.


ZIMA-CAD-Parts checks for published updates in the background after startup.
The owner starts installation from **Settings > Updates** and confirms it.
The download, verification, normal shutdown, activation and restart then run
automatically. There is no startup popup or automatic installation.

The implementation supports the versioned [distribution layout](distribution-policy.md).
Windows builds and isolated installer tests are verified locally. Debian 13
x86_64 packaging and launchers are provided, but a real Debian KDE/Wayland run
is still required before publishing a supported Linux binary. No release is
published by the build or signing tools described here.

## User interface

A small **Update available** link appears at the right of the Directory / Parts
tab row. Its tooltip includes the version. It opens Settings without downloading.
All directory tabs share one update service. Startup network failures stay silent;
the Updates page shows the last successful check and error details.

Settings contains a startup-check checkbox, versions, plain-text release notes,
download size, progress/cancellation, **Check for updates**, **Install update**,
and **Return to previous version**. Installation is disabled for an unsigned
candidate or loose development executable. The controls and ordinary messages
are translated into Czech, German, French and Russian, with English as default.
Low-level diagnostic details and CLI errors are currently in English.

Closing the application or cancelling a download does not approve installation.
Downloaded/staged data may remain after abrupt termination, but startup never
activates it. A later explicit request rechecks the release. Version 1 restarts
an interrupted download; it does not support byte-range resume. The UI remains
usable during checking, downloading and archive verification.

## Files and activation

```text
ZIMA-CAD-Parts/
  ZIMA-CAD-Parts.exe
  ZIMA-CAD-Parts.sh
  launcher.ini
  installation.json
  LICENSE
  windows/YYYYMMDDNN/
    ZIMA-CAD-Parts.exe
    ZIMA-CAD-Parts-cli.exe
    ZIMA-CAD-Parts-update.exe
    ...runtime dependencies...
  linux/YYYYMMDDNN/
    ZIMA-CAD-Parts
    bin/ZIMA-CAD-Parts
    bin/ZIMA-CAD-Parts-cli
    bin/ZIMA-CAD-Parts-update
    ...runtime dependencies...
  source/YYYYMMDDNN/
  release-info/<platform>-YYYYMMDDNN.json
  custom/windows/
  custom/linux/
  .updates/
```

`installation.json` identifies a managed distribution (product, protocol 1 and
installation ID). A Git checkout is never an installation target. Use a writable
local installation directory. Root/platform/staging links and reparse points
are rejected. Project directories, external metadata and `custom/` are untouched.

The helper verifies every archive file but imports only the current platform and
its matching sources. It changes only that platform's `launcher.ini` selection.
It refuses an existing same-version directory whose content differs. Partial
imports can be retried because both staged and already imported trees are
revalidated. It never extracts a release over the live installation root.

An installation lock gates new launches. GUI and CLI processes register their
version with process-lifetime lock files. The helper waits up to 30 seconds for
normal shutdown; it does not terminate user processes. If another instance stays
open, installation fails safely and remains prepared. The result is recorded in
`.updates/<platform>-result.json` and shown in Settings. Registered instances
must be closed before installing; a lock left by a different machine should be
investigated rather than removed while that machine may still be using it.

Before selecting a candidate, `.updates/engine.ini` keeps the last working helper
selected for the root launcher. An atomic journal records previous/candidate
selections. The candidate must acknowledge its version, process ID and a one-use
random token after the main window has been constructed. This does not wait for
thumbnails. On success, the journal is committed and the new helper is selected.
On failure, the previous selection is restored. If the failed process already
exited, the previous GUI is restarted; a slow still-running candidate is not killed.
On the next root launch after interruption, the old helper recovers the journal.

Current and previous verified runtimes are retained per platform. Cleanup runs
after successful activation or rollback. Older modified, unrecognized or busy
versions are preserved and reported in `cleanupPending`; a later successful
activation retries cleanup. Source remains if either platform still has a runtime
of that version. Intentional edits belong in `custom/`. Binary rollback does not
restore earlier user preferences or project metadata.

Runtime hashes are checked during installation, rollback and cleanup, rather than
on every ordinary start. Ordinary launch verifies the signed provenance record.
This is an update-integrity mechanism, not protection against an attacker who
already controls and edits the user's installed executables.

Root launchers are protocol-1 bootstrap components. Updates do not replace them.
If a future release requires a newer launcher protocol, installation is disabled
and a complete new distribution must be installed separately.

## Release discovery and trust

The fixed endpoint is the public
[GitHub Releases API](https://docs.github.com/en/rest/releases/releases) for
`ZIMA-Engineering/ZIMA-CAD-Parts`. No user token is required. A Git commit or tag
alone is not an update. The client scans pages of 100 (up to 20 pages), filters
out drafts/prereleases/foreign tags, and orders valid `YYYYMMDDNN` numbers.
Only a newer signed stable release containing the current platform is installable.
Windows and Debian builds are published independently; neither is a prerequisite
for the other. Discovery skips releases that lack the current platform and keeps
looking for the newest applicable release. If none is newer than the installed
version, the client reports no update for this system.

Requests have an identified User-Agent, API version, bounded responses/timeouts,
conditional ETag caching and rate-limit backoff. Failed discovery is not reported
as a successful up-to-date check. The highest verified offered version is retained
locally to detect a later older/missing catalog. A failed manual recheck clears the
in-memory offer; installation always rechecks the server before downloading.

A final release contains these three assets:

```text
ZIMA-CAD-Parts-YYYYMMDDNN.zip
update-manifest.json
update-manifest.sig
```

The ZIP contains the single `ZIMA-CAD-Parts/` directory, including matching source
and one or both available platform binaries. Finalize as soon as the desired
platform build is ready; waiting for the other platform is not required.
Once published, bytes are immutable; adding another platform
or changing an asset requires a new release number. GitHub's
[immutable releases](https://docs.github.com/en/code-security/concepts/supply-chain-security/immutable-releases)
can enforce this repository policy after the publisher enables them.

Ed25519 signatures are verified using OpenSSL 3, with public keys embedded in
`src/update/trusted-keys.json`. Private keys never belong in Git, releases or CI.
The publisher tool uses Python `cryptography`; end users need neither Python nor
Git. SHA-256 binds the archive, internal checksums and source tree to the signature.

The manifest is canonical UTF-8 JSON (compact, keys ordered as UTF-16 code units,
followed by one newline). Verification checks the exact signed bytes before
parsing and requires that canonical representation, rejecting duplicate keys.
The signature envelope contains `keyId` and a 64-byte Ed25519 signature as lowercase
hex. The key ID selects an already trusted key; a manifest cannot introduce a key.

| Signed field | Contract |
| --- | --- |
| `schemaVersion`, `product`, `channel` | `1`, `ZIMA-CAD-Parts`, `stable` |
| `version`, `tag`, `commit` | Valid date/sequence, matching tag, full 40-character commit |
| `archive` | Name, SHA-256, byte size, unpacked size, file count |
| `checksumsSha256` | Hash of canonical `checksums.json` in the ZIP |
| `platforms` | `windows-x64` and/or `debian-13-x86_64`, runtime and entry paths |
| `source` | Matching source path and canonical source-inventory SHA-256 |
| `minimumUpdaterVersion`, `launcherProtocol` | Required protocol capabilities |

Inventories map safe relative names to SHA-256, size and executable flag. The
publisher assigns executable flags to shell scripts, shebang files and ELF files,
independently of the packaging host. Clean release source is exported from Git
objects, including matching submodules, so checkout CRLF conversion does not
change source bytes between Windows and Linux.

The client checks the archive length/hash before opening it. It rejects unsafe
paths, links, case collisions, duplicate entries and mismatched inventories.
Protocol-1 limits: ZIP smaller than 2 GiB, at most 50,000 files, at most 512 MiB per
file and 8 GiB unpacked. Required free space includes the download, twice the
unpacked size and a 64 MiB reserve. Extraction uses Qt Core's private QZipReader;
the helper must ship with the exact Qt runtime against which it was built.

`release-info/` contains signed bootstrap runtime/source inventories. Subsequent
verified receipts live in `.updates/installed/`. Cleanup and rollback reconstruct
inventories from signatures, rather than trusting editable cached file lists.

## Build and package

Build GUI, CLI and helper using the same compiler and Qt kit. On Debian, install
`qt6-base-private-dev`, `libssl-dev`, `pkg-config` in addition to the normal app
dependencies. Build the helper separately:

```sh
mkdir .build-updater
cd .build-updater
qmake6 ../zima-cad-parts-update.pro
make -j$(nproc)
```

On Windows, use the Visual Studio developer shell, `qmake`, `nmake release`, and
pass `OPENSSL_ROOT` pointing to the OpenSSL 3 installation. Both platform packagers
require `--exe`, `--cli` and `--updater`. The Windows dependency resolver includes
OpenSSL; Debian bundles its shared library. Both include its license notice.

`--release` requires a clean checkout at the matching release tag and produces an
unsigned release candidate. Normal development packaging remains available.
The root launcher can run an unsigned local candidate, but update installation
requires a signed bootstrap record. CI uploads candidates; it does not possess
the signing key or publish a stable signed update automatically.

## Publisher key and finalization

The provisioned public key ID is `zcp-8fa1f825cbde5d81`. The Windows private key is
stored in `.local-backups/release-signing/parts-ed25519.dpapi`, excluded from Git,
with a current-user-only ACL and Windows DPAPI account protection. Do not delete
that local folder during source cleanup. Do not print the key or copy it into a
release tree.

For signing from Linux, first export an encrypted portable backup on Windows:

```text
python tools/distribution/update-release.py export-key --private .local-backups/release-signing/parts-ed25519.dpapi --output D:/secure-backup/parts-ed25519.pem
```

The tool asks interactively for a password of at least 12 characters twice.
Keep the backup outside the repository and transfer it securely to Linux. Linux
signing prompts for that password; ordinary application updates use only the public
key and require no private key or password. The portable export is not automatic.

After building one or both platforms from a clean tagged commit, assemble the
matching sources, available runtimes and applicable root launchers into a fresh
package, then finalize. The Windows launcher is required when Windows is
included; the portable Linux launcher is present in every bundle:

```text
python tools/distribution/update-release.py finalize --private PATH_TO_PRIVATE_KEY --package PATH_TO_PACKAGE/ZIMA-CAD-Parts --output NEW_ASSET_DIRECTORY --version YYYYMMDDNN
```

The output directory must not exist. The tool writes the ZIP, manifest, signature
and bootstrap inventories, then verifies ZIP CRCs. It does not upload anything.
Inspect and test the result before attaching all three assets to a GitHub draft
and publishing it. `--development` signs an explicitly development-channel test
archive; the production updater refuses that channel. Do not publish it as stable.
For planned rotation, first ship the successor public key in a release signed by
the existing key. An unknown key needs a separately verified bootstrap.

## CLI

The standalone CLI and the bottom command panel share these commands:

```text
update check
update download --target 2026091601
update install --target 2026091601 --apply
update rollback --apply
update status
```

`download` prepares only. `install` or `rollback` without `--apply` returns
`confirmation-required`. With `--apply`, the CLI hands off and exits; the command
panel requests normal GUI shutdown. The asynchronous result has an operation ID;
read `.updates/<platform>-result.json` or Settings for the final outcome. Commands
operate on the installation containing the executable, never the active project.

## Verification and remaining platform work

See [verification](verification.md) for the isolated signed-release fixture suite,
GUI/CLI regression tests and commands. Tests never need the publisher key or a real
GitHub release. HTTP/key/settings overrides are compiled only with
`CONFIG+=update_tests` and are absent from production builds.

Before a public release, verify its actual signed bootstrap package on a clean
Windows installation. Debian still needs a full build plus KDE/Wayland launch,
installation and rollback on Debian 13. A private/draft repository or one without
signed published release assets cannot offer a public update yet.
