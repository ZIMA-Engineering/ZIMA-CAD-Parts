# Build verification

## Interaction colours, September 24, 2026

Full Windows GUI, CLI, updater and integration builds passed. All 60 integration
checks passed with no failures or skips, and all translation catalogs validated.
New coverage checks the shared azure/green selection contract and retained
native tree metrics, including real hover leave without changing selection.
Evidence: `.build-ui-20260919/interaction-build.log` and
`interaction-full-tests.txt`. The initial incomplete test environment lacked
WebEngine, translations and CLI resources; verification used the complete
runtime and matching Qt resources. Native Linux visual checks remain pending.
See [the release record](releases/2026092401.md) for portable acceptance.

## Signed Windows release 2026091902, September 19, 2026

The clean release build and packaged runtime passed 59 integration checks,
18 CLI tests, five distribution checks and all translation catalogs. Signature,
source/binary identity, complete checksum inventories, clean-PATH probes and
public update discovery from 2026091901 passed. See the
[release acceptance record](releases/2026091902.md) for hashes and scope.

## Native ZIMA-CAD parameter import, September 19, 2026

The Windows GUI, CLI, updater and integration projects were rebuilt with
Qt 6.10.1 and MSVC x64. All **59 integration checks**, **14 tool CLI tests**,
**4 read-only CLI tests** and four translation catalogs passed, with no
integration failures or skips.

New coverage checks native Part/Assembly files saved by ZIMA-CAD, Czech and
English values, historical Pro/E column handles, literal text, alias collisions,
empty and zero values, archive exclusion, malformed documents, native-over-Pro/E
precedence, metadata persistence and FileModel updates after reloading an edited
source. Existing Pro/E, browsing, thumbnail and datasource checks also passed.

Logs are `.build-ui-20260919/zima-parameters-build.log`,
`.build-ui-20260919/zima-parameters-tests.log` and
`.build-ui-20260919/integration-results.txt`. This verifies a local Windows
development build; it does not replace the immutable release below. Linux,
interactive technical-specification browsing and downloads were not repeated.

## Signed Windows release 2026091901, September 19, 2026

[Published immutable release](https://github.com/ZIMA-Engineering/ZIMA-CAD-Parts/releases/tag/ZIMA-CAD-Parts-2026091901) was built from clean commit
`6e2cce0455febd583e24d971cbcd9b39aad6dc44`. Its Windows x64 ZIP is
**316,981,328 bytes** and contains the matching committed sources.
SHA-256: `d4d3b3de10950a53e3179dc9ac3d9b52c35f4fa2e694fa3996e2dbbaed5401c5`.

- Clean GUI, CLI, updater and integration builds passed with Qt 6.10.1,
  MSVC x64 and Open CASCADE 8.0.0.
- The packaged runtime passed **56 integration checks**, **14 tool CLI tests**,
  **4 read-only CLI tests**, **5 distribution checks** and all four translation
  catalogs (**562 messages** each), with no failures or skips.
- Ed25519, ZIP CRC, all **1,484 file checksums**, all
  **566 committed-source comparisons**, matching binary hashes
  and startup probes with development paths removed passed.
- The production updater reports the signed bootstrap as trusted and idle at
  version 2026091901. Public GitHub asset sizes and SHA-256 digests match the
  local verified artifacts; anonymous manifest/signature downloads and the ZIP
  download endpoint were checked after publication.
- The production updater from the previous signed Windows 2026091601 package
  detects 2026091901 with `status: available` and `installable: true`. This was
  an availability check; it did not install an update.

The final package is `.dist-output/ZIMA-CAD-Parts-2026091901.zip`, with the
extracted directory and signed manifest/signature beside it. Release logs and
verification receipts are in `.build-release-2026091901/`. This release contains
Windows only. Tests ran on the development workstation with an isolated portable
installation, not a fresh Windows OS. The earlier Debian acceptance below does
not validate these new GUI/tool changes on Linux.

## Windows: local build 2026091901, September 19, 2026

The GUI, CLI, updater and integration-test projects passed a clean build with
Qt 6.10.1, MSVC x64 and Open CASCADE 8.0.0. The final dialog refinements were
rebuilt and tested in an isolated runtime beside the deployed Qt/Ghostscript
dependencies. Test data was temporary; recycling tests used the Windows system
trash under the desktop user's account.

- **56 integration checks passed**, with no failures or skips. Coverage includes
  direct main-window startup, the Settings/command-panel order, one-column
  Cleaner/PDF lists, selected/completed/failed rows, ZIMA-CAD archive revalidation,
  completion notifications, local locks and captured recursive AI reviews.
- **14 built-in tool CLI tests passed**, including real PDF conversion, source
  deletion/locks, every ZIMA-CAD document extension, highest/orphan archives,
  arbitrarily long numeric suffixes, exclusions and native recycling.
- **4 read-only CLI tests passed**.
- All four catalogs passed with **562 complete translations** each.
- Czech PTC, ZIMA-CAD Cleaner and PS2PDF windows were rendered and visually
  inspected. Existing browsing, metadata, thumbnail and CAD-import checks are
  included in the integration suite. Native technical-specification browsing
  and interactive downloads were not manually repeated for this change.

Build/test logs and dialog captures are under `.build-ui-20260919/` locally.
This is a local Windows development build, not a published or signed release.
Native Linux/KDE execution has not been repeated for these changes; the earlier
Debian release acceptance below applies to that earlier release.

## Windows: PS2PDF release 2026091801, September 18, 2026

The GUI, CLI, updater and integration-test projects were rebuilt with Qt
6.10.1, MSVC x64 and Open CASCADE 8.0.0 after the PS2PDF workflow update.

- Five focused CLI tests passed with real Ghostscript. They cover replacing an
  existing PDF, preserving it when conversion fails, creating the relative
  `pdf` output directory, deleting a source only after success and rejecting
  conflicting destinations or non-PostScript PLT data.
- The dedicated GUI integration check passed for automatic list loading, the
  Cleaner-style layout, the default `pdf` path, the source-deletion checkbox
  and the **Create PDF** action.
- All four translation catalogs passed with 563 complete messages each.

The complete tool test run passed every PS2PDF and STEP check. Its existing
PTC-Cleaner recycle-bin case could not use the Windows system trash inside the
restricted test environment. A complete GUI run from the bare build directory
also lacked deployed application resources for unrelated WebEngine, icon,
language and Cleaner checks; the focused PS2PDF GUI check passed independently.

## Signed Linux acceptance completed (2026-09-17)

Version 2026091701 passed signing, packaged bootstrap trust and native Debian
13.6 KDE/Wayland acceptance, including web/PDF, system credentials across
processes, STEP/OpenGL rendering and relocation with isolated profiles.
See [release acceptance and exact test scope](releases/2026091701.md).
Earlier unsigned/pending statements below describe historical checks.

## Debian 13: candidate 2026091701, September 17, 2026

Built GUI, CLI, production updater and test helpers with GCC 14.2 and Qt 6.8.2
on Debian 13 amd64, with system OCCT and libsecret enabled. The root GUI build
used `qmake6 zima-cad-parts.pro && make -j$(nproc)`.

- 51 GUI integration checks passed using the offscreen Qt platform.
- 4 CLI checks, 9 built-in tool checks and 18 isolated updater checks passed.
- Distribution checks: 3 passed, 1 Windows-only check skipped.
- All four application catalogs passed with 558 complete translations each.
- The repaired context-menu fixture also passed on native Wayland.
- The portable package launcher check, GUI build-info and CLI/updater version
  probes passed. A 15-second KDE/Wayland GUI startup probe stayed running until
  the intentional timeout, without disabling the WebEngine sandbox.

An initial complete Wayland test run found the Russian standard Qt Cancel
button in English with the KDE platform theme; application catalog checks
passed. Clean-machine PDF/web, credentials, relocation and signed update
acceptance remain pending. The candidate is an unsigned GitHub draft, not a
stable automatic update. Dependency-license inventory and publisher signing
must be completed before public release.

## Windows: PTC Cleaner revision 2026091601, September 16, 2026

The signed Windows release was built from clean commit
`53484737ddccf9f3fbd3c8d1014d5ed065ad7fe3`. Its final ZIP is **316,779,015 bytes**
and passed Ed25519, ZIP CRC, all 1,477 file-checksum checks,
all 564 committed-source comparisons, binary matching and
startup probes with development directories removed from PATH. The production
updater reports version **2026091601**, `trusted: true`, and idle status.

The packaged runtime passed **51 integration tests**, **9 built-in tool tests**,
**4 read-only CLI tests**, **4 distribution tests**, and all four translation
catalog checks. Archive SHA-256:
`4d35512e51bb1de5b73ddaa94b43698b968307847ed6cfa45b8c3e9462efdbb6`.

The complete local GUI and CLI were rebuilt with Qt 6.10.1 and MSVC x64.
Verification of automatic preview, the simplified Czech layout, option changes,
completed/unchecked/failed rows, captured AI approvals, unchanged PDF/STEP
controls and coalesced directory notifications passed:

- **51 integration tests passed**, with no failures or skips.
- **9 built-in tool CLI tests and 4 read-only CLI tests passed**.
- All four translation catalogs passed validation with **558 complete messages**.
- The Czech Cleaner window was rendered and visually inspected.
- A Windows benchmark recycling 128 small temporary old revisions took
  **7.145 seconds before** and **1.842 seconds after** batch recycling. Both runs
  preserved the latest revision. This is a workstation fixture result, not a
  guarantee for other drives, file sizes or network storage.

Run GUI integration beside the deployed runtime as described below. Executing
directly from the build directory first missed WebEngine resources, Qt standard
translations and SVG plugins; the complete deployed-runtime run passed.
The user's example project was inspected read-only; deletion checks used only
temporary fixtures. Linux execution has not been verified for this change.

## Windows: verified September 15, 2026

The complete GUI, CLI, updater and integration test projects were built with
Qt 6.10.1, MSVC x64 and Open CASCADE 8.0.0. The current local Windows build is
**2026091507**. Recorded checks for the implemented features are:

- **44 integration tests passed**, with no failures or skips, including AI
  drafting, inline approval, path insertion and updater settings.
- **4 read-only CLI tests passed**.
- **18 isolated updater tests passed**, including platform-independent releases.
- **9 built-in tool CLI tests passed**, including actual Ghostscript
  conversion, conversion failure cleanup, STEP backups and system trash.
- **4 distribution tests passed** using the native Windows launcher.
- All four translation catalogs passed validation, with **555 complete
  messages per language**, including placeholders and plural forms.

The system trash test passed with access to the Windows recycle bin; a
restricted sandbox blocked that test's initial attempt. Native launcher
tests passed independently of the machine's PowerShell script policy.

Build 2026091507 adds **Open** and **Set as working directory** to data source
header menus. The GUI, CLI and updater were rebuilt, and all 44 existing
integration checks passed again. The two new labels use the same translations
as ordinary directory actions.

Earlier manual checks of the deployed Windows application included green active
tabs, SVG icons, the command panel and PDF/STEP tool dialogs. The full GUI build
was repeated after removing the separate built-in tools toolbar icon.

The signed 2026091506 packaging baseline was rebuilt from clean commit
`68e6eda400ef91c099489bc7b1ea71726270d7ce`. Its ZIP passed signature, CRC and
all 1,476 file-checksum checks. All 563 source files matched committed bytes,
and all three application binaries matched the build outputs. The packaged
updater recognized the signed bootstrap record as trusted. Native launcher,
GUI build-info and CLI/updater version checks passed with only Windows system
directories on PATH.

Repeat these archive and bootstrap checks for each Windows package after
building its matching release tag. Keep the signed ZIP, manifest and signature
together. Preparing a local bundle does not publish a GitHub release; publishing
is a separate step described in [GitHub updates](github-updates.md).

These results were obtained on the development workstation. A clean Windows
installation, the modified GitHub workflows and Linux/KDE runtime behavior
still need separate verification. No verified Debian binary is available yet.

## Integration coverage

- Language switching, SVG icons and STEP/IGES/STL import.
- Deferred tab loading, thumbnails, filters and direct startup without a splash.
- Preserving other tree branches when deleting and working with the displayed
  directory.
- Shared PDF, Pro/E and ZIMA-CAD parameters, table and dialog editing,
  preservation after Refresh, and legacy metadata records.
- Selecting the highest numeric Pro/E revision for parameter import.
- Saving and clearing a local protection lock; blocking deletion or moving
  of protected files or a containing directory; allowing copying out.
- Independent subdirectory locks, blocking protected file overwrites, and
  updating controls after selection, locking and unlocking.
- Recursive bulk locking and unlocking, preservation of other metadata,
  exclusion of `0000-index`, error reporting and cancellation.
- Immediate **Apply to subdirectories** behavior even if Properties is later
  cancelled. New subdirectories do not automatically inherit the lock.
- Agreement between CLI JSON and the real GUI model/filter.
- Command panel context capture, relative paths, history, draft restoration,
  parsing errors, help and visibility.
- Tool previews and revalidation of changed files, retained revisions and
  locks before applying changes.

## Repeating checks

Check translations from the repository root:

```sh
python tests/check_translations.py
```

Build outside the source directory. For integration tests, finish the release
application first, then run qmake on `tests/parts-integration.pro`. Pass
`PARTS_OBJECTS_DIR` with the absolute path to the application's release
objects, the same `OCCT_ROOT`, and `CONFIG+=release`. Use the same Qt kit
and compiler; on Windows MSVC, build with `nmake release`.

The test executable needs the application's runtime libraries and Qt Test.
On Windows, it can temporarily be placed beside the deployed application.
For headless tests, set `QT_QPA_PLATFORM=offscreen` and point
`QT_QPA_PLATFORM_PLUGIN_PATH` to the Qt kit's `plugins/platforms` directory.
The normal distribution may not include `qoffscreen.dll`. Remove only the
temporary test executable afterwards.

Set `PARTS_CLI_EXE` to the built CLI executable before running integration
tests; otherwise the CLI comparison is skipped. The same variable selects
the executable for these process tests:

```sh
python tests/test_cli.py
python tests/test_tools.py
```

Tool tests need Ghostscript and a usable system trash. On Windows, the CLI
can use the prepared `tools/ghostscript` runtime beside it. Set
`PARTS_LAUNCHER` to the compiled native root launcher before running:

```sh
python tests/test_distribution.py
```

The standalone `tests/parts-performance.pro` suite does not require
WebEngine; see the [filter documentation](filters.md) for its build steps.

## Updater checks

The updater suite uses temporary installations, an ephemeral test signing key,
a localhost HTTP server and a short-lived fake GUI. It never reads the publisher
key, contacts GitHub or touches real projects. Python requires `cryptography`.
Build `zima-cad-parts-update.pro` with `CONFIG+=update_tests` into a separate
build directory and build `tests/update-fixture.pro` separately. Never distribute
the test helper. Set these variables to the resulting executable paths:

```text
PARTS_UPDATER_TEST_EXE=.../ZIMA-CAD-Parts-update[.exe]
PARTS_UPDATE_FIXTURE_EXE=.../update-fixture[.exe]
PARTS_UPDATE_TEST_RUNTIME=.../windows/YYYYMMDDNN
```

On Windows the last directory supplies Qt Core/Network and MSVC runtime DLLs to
the fake runtime. Run `python tests/test_updates.py`. The production helper has
no test key, HTTP endpoint or settings-directory override.

The 18 updater regression cases cover signed discovery, date ordering, draft
filtering, download/install/rollback, signature/hash failures, traversal,
modified staging, interrupted import, failed startup recovery, retention,
modified old versions, incompatible protocols, catalog replay/offline errors, restart without approval,
case collisions, unsigned bootstrap rejection and source retention across platforms.
They also cover offering and installing an archive containing only the current
platform, skipping a newer release for the other platform, and independently
finalizing signed Windows-only and Debian-only publisher fixtures. The Debian
publisher fixture checks the archive contract; it does not execute a Linux binary.
GUI integration also checks all five update-page languages, disabled installation
for development copies, and opening Settings without starting a download.

## AI command panel checks

The Windows GUI and CLI were built with Qt 6.10.1, MSVC and OCCT for version
2026091506. GUI integration now has 44 passing checks, including sixteen AI
cases. CLI read commands: 4 passing checks; built-in tools: 9. The system
trash case required the normal Windows user context because the restricted
sandbox cannot access that user's recycle bin.

The AI fixture covers command mode switching, captured directory roots,
minimal initial context, paginated reads, command discovery, rejected direct
apply, local locks, changed-file revalidation, explicit review, cancelled
plan replay, protocol errors, native-tool rejection and cancellation.
Settings are checked in all five UI languages, including status translation
and the executable label after switching languages. The four translation
catalogs contain 553 complete entries each; placeholder and markup checks pass.

Path checks cover local URL drag/drop, insertion into drafts, duplicate removal,
selection replacement, undo, quoted paths with spaces and Unicode, native Windows
paths, captured requests, deletion before sending and `/new`, exact-file access versus
directory descendants, rejection of remote URLs, multiple highlighted rows,
and menu routing from files, tree directories and data source roots. Screenshots
of Czech path insertion and the inline system-command review were inspected.

Real system-command fixtures verify that cancelled reviews do not create files,
approved commands do create only their temporary fixture files, UTF-8 output
returns to AI, invalid arguments fail, nonzero exits are reported, and timeout
and cancellation stop execution. A child-process fixture also verifies that
Stop prevents a delayed child write. These tests do not use a live AI account.

The actual installed native Windows Codex runtime successfully completed
App Server initialization and account discovery in a separate empty profile.
That initial profile was signed out. For version 2026091504, a separate live
smoke check used the existing Parts ChatGPT login and the adapter's exported
thread/tool contract. A request to create a text file in an empty temporary
directory produced a `system_command` proposal. The probe declined it without
execution; Codex correctly reported that no file had been created. No account
details were logged. Actual approved writes are covered by local Qt fixtures.
See [AI setup and behavior](ai-command-panel.md).

Inline review checks verify that neither kind of AI approval is modal,
declining leaves files unchanged, provider failure closes pending approval,
and one click applies only the selected STEP files without another popup.
Existing ordinary Parts tool dialogs retain their confirmation behavior.

Draft regression checks type through real keyboard events during context
preparation, active inference and inline approval. Enter cannot send another
request or approve an operation while busy. The next draft survives normal
completion, errors and Stop, and only a later explicit submission sends it.

Debian execution remains pending; these Windows results do not establish
Linux support.

## System appearance, 2026092601 (Windows)

The clean committed-source GUI, CLI, updater and integration builds passed.
All 60 integration checks, 14 tool CLI tests, four read-only CLI tests, five
distribution tests and four translation catalogs passed. A native Windows
style probe verified live palette changes without changing selected items or
checked-button state. No UI strings were introduced.

The signed portable package passed Ed25519, CRC, all file checksums and exact
committed-source comparisons. Clean-PATH launcher and executable probes passed;
the production updater reported a trusted installation. See the complete
[release receipt](releases/2026092601.md) for hashes and environment limits.
