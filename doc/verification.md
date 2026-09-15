# Build verification

## Integrated-tools baseline: September 15, 2026

The complete GUI, CLI and integration test projects were built with Qt 6.10.1,
MSVC x64 and Open CASCADE 8.0.0. Recorded results for the command panel and
built-in tools are:

- **27 integration tests passed**, with no failures or skips.
- **4 read-only CLI tests passed**.
- **9 built-in tool CLI tests passed**, including actual Ghostscript
  conversion, conversion failure cleanup, STEP backups and system trash.
- **4 distribution tests passed** using the native Windows launcher.
- All four translation catalogs passed validation, with **454 complete
  messages per language**, including placeholders and plural forms.

The system trash test passed with access to the Windows recycle bin; a
restricted sandbox blocked that test's initial attempt. Native launcher
tests passed independently of the machine's PowerShell script policy.

The deployed root executable was checked in the running Windows application.
Manual checks included green active tabs, SVG icons, the command panel and
PDF/STEP tool dialogs. The full GUI build was repeated after removing the
separate built-in tools toolbar icon. The deployed and packaged GUI binaries
match the resulting build. The distribution ZIP passed its CRC check, and
its file checksums were regenerated.

These results were obtained on the development workstation. A clean Windows
installation, the modified GitHub workflows and Linux/KDE runtime behavior
still need separate verification. No verified Debian binary is available yet.

## Integration coverage

- Language switching, SVG icons and STEP/IGES/STL import.
- Deferred tab loading, thumbnails, filters and the non-blocking splash screen.
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
Additional cases cover host-only installation, skipping newer releases for
another platform, and independently finalizing Windows-only and Debian-only
publisher fixtures. These fixtures do not execute a Debian binary.
GUI integration also checks all five update-page languages, disabled installation
for development copies, and opening Settings without starting a download.

Windows GUI integration: 28 passing checks. CLI read commands: 4; built-in tools:
9; the four translation catalogs contain 485 complete entries each. Debian
execution remains pending; these Windows results do not establish Linux support.
