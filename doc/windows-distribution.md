# Windows distribution implementation

This implements the structure and launchers from the
[binding policy](distribution-policy.md), including the separate update helper.
See [GitHub updates](github-updates.md) for signing, installation and retention.
A development candidate is unsigned until publisher finalization; Debian
runtime verification remains pending.

## Version and build

The single version source is `VERSION` in `src/zima-cad-parts.h`
(`YYYYMMDDNN`). The application reports it in About and through
`QCoreApplication::applicationVersion()`. Running
`ZIMA-CAD-Parts.exe --build-info` returns JSON without starting the GUI.
The marketing name with the number nine appears only on the About page.

Build the complete project using qmake and nmake from an MSVC developer
shell, outside the source directory, with the same Qt kit and OCCT used for
distribution. Packaging requires Python 3, Git, MSVC `dumpbin` and that kit's
`windeployqt`. Before packaging, prepare Ghostscript with its license and
sources:

```powershell
python tools/distribution/prepare-ghostscript.py --sevenzip "C:/Program Files/7-Zip/7z.exe"
```

See [built-in tools](integrated-tools.md) for details and conversion commands.
After building the GUI, CLI and `zima-cad-parts-update.pro` helper
(the helper requires `OPENSSL_ROOT`, for example `C:/zb/i/x64-windows`):

```powershell
python tools/distribution/package-windows.py --exe .build-release/release/ZIMA-CAD-Parts.exe --cli .build-cli/release/ZIMA-CAD-Parts-cli.exe --updater .build-updater/release/ZIMA-CAD-Parts-update.exe --qt C:/Qt/6.10.1/msvc2022_64 --occt C:/zb/i/x64-windows --output .dist-output/2026091501
```

The executable must come from this checkout. If it cannot locate DLLs in
the build directory, add the selected Qt and OCCT `bin` directories to PATH
for packaging. The script checks the executable version against the sources;
it does not prove a fully reproducible build. The output directory must be
new; an existing package is not overwritten.

Sources are copied using the Git index, including actual contents of
initialized submodules. A development bundle uses current working files,
and its manifest marks a modified checkout. New untracked files can be
included individually with `--include path`. Other untracked files are not
automatically bundled. Before release, verify export completeness by
building directly from the exported sources.

`--release` requires a clean checkout and a `ZIMA-CAD-Parts-<VERSION>` tag
at HEAD. It creates a release candidate, not a signature or an officially
verified distribution. The GitHub workflow uses this script; a date-based
tag creates a draft release with an archive. Until the publisher finalizes signatures,
this is not automatic publication of a signed release.

`windeployqt` deploys Qt. Other DLLs are resolved recursively from imports
using `dumpbin`, preferring Qt libraries from the selected kit. An unresolved
dependency stops packaging. Dynamically loaded components require runtime
tests; an import list alone cannot prove their completeness.

## Launching and switching versions

Run `ZIMA-CAD-Parts.exe` in the generated `ZIMA-CAD-Parts` root directory.
This is a small native Win32 launcher with a statically linked MSVC runtime;
it requires neither Qt nor permission to run PowerShell scripts. The actual
application is in `windows/<version>/`.

`launcher.ini` selects the Windows version. `windows_custom=false` selects
the official directory branch. A development bundle using that structure
is still marked as development in its manifest, not as a verified release.

```powershell
./ZIMA-CAD-Parts.exe -Version 2026091501
./ZIMA-CAD-Parts.exe -Custom -Version my-build
./ZIMA-CAD-Parts.exe -Check
```

Custom builds live in `custom/windows/my-build/`, including their executable
and libraries. `-Check` validates the selection and prints its path without
launching it. The launcher checks the name format and, for release directories,
the version and platform in `build.ini`. It does not verify a cryptographic
signature. For the new process, it removes development Qt paths and limits
PATH to the application directory and Windows system directories.

The Linux root launcher selects a standard or custom build and checks
`build.ini`. It fails if no Debian bundle is installed. Linux execution has
not been verified here; see the [Debian guide](debian-distribution.md).
Current and previous builds can be placed in the folders manually. This
stage does not download or remove versions.

`checksums.json` contains SHA-256 hashes of packaged files for integrity
checking, not proof of origin. `version.json` records the version, commit,
platform, Qt kit and development state.

## License files

The full Parts GPL text is copied to `LICENSE` beside the root launchers,
to the source snapshot, and to `windows/<version>/licenses/Parts-LICENSE`.
Source headers specify `GPL-3.0-or-later`. Other notices are in the runtime's
`licenses/` directory. Ghostscript retains its own license and corresponding
source archive under `tools/ghostscript`.

The Windows runtime includes the Qt base translation catalogs for every
supported UI language so standard Qt controls follow the selected language.

## Verification

Launcher and export checks:

```sh
python tests/test_distribution.py
python tests/check_translations.py
```

Set `PARTS_LAUNCHER` to the compiled native launcher to exercise the shipped
launcher. Without it, Windows tests use the PowerShell fallback, which is
subject to the machine's script execution policy. Checks cover current and
previous versions, custom builds, invalid paths and manifests, date-based
numbering and submodule contents in source exports.

Before delivery, verify the root launcher without development Qt/OCCT in
PATH and build from the bundled sources. Running on a developer workstation
does not replace testing on a clean Windows installation. Local source
export and package checks do not verify GitHub workflow execution; CI must
be run separately. See [verification](verification.md) for recorded results.

## Final archive

The agreed final name is `ZIMA-CAD-Parts-YYYYMMDDNN.zip`, without a platform.
The local assembled bundle lives directly in `.dist-output/ZIMA-CAD-Parts/`.
The version-specific output in the packaging example is a staging directory,
not an extra level in the final distribution.

After adding the Linux build, preserve the Windows entries in `launcher.ini`
and regenerate `checksums.json`. Scripts do not yet automatically merge
platform outputs and rebuild the shared archive.

Clean release exports use committed Git bytes rather than checkout line endings.
The updater requires Qt Core private headers and an exact matching Qt runtime.
It ships OpenSSL 3 with its license, the installation marker and versioned helper.
After signing, root launchers coordinate recovery through `.updates/engine.ini`;
unsigned local candidates still run, but cannot install an update.
