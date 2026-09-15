ZIMA-CAD-Parts
==============

License
-------
ZIMA-CAD-Parts is licensed under the GNU General Public License, version 3
or (at your option) any later version (`GPL-3.0-or-later`), as stated in the
source headers. The full license text is in [LICENSE](LICENSE).
Distribution bundles also include this file beside the root launchers.
Third-party components retain their own licenses; see [licenses/](licenses/)
and the license files shipped with the bundled dependencies.

Documentation is maintained in English. UI translations and examples of
localized metadata retain their respective languages; see [AGENTS.md](AGENTS.md).

Requirements
------------
Qt 6.8 LTS or newer - modules `core`, `gui`, `network`, `widgets`,
`openglwidgets`, `webenginewidgets`, `webchannel`, plus `qmake`.

CAD preview for STEP, IGES, and STL files is enabled when Open CASCADE
Technology (OCCT) is available. On Linux, the build detects OCCT development
packages installed by the distribution package manager. Custom OCCT builds,
including vcpkg installs, can still be selected by passing `OCCT_ROOT` to
qmake. When OCCT is not found, the application still builds without the OCCT
preview.

Linux dependencies
------------------
Install the Qt 6 development stack, toolchain, libsecret, and OCCT development
packages before building.

Debian 13 / Ubuntu:

```
sudo apt install build-essential git libsecret-1-dev \
    qt6-base-dev qt6-base-private-dev qt6-base-dev-tools qmake6 libssl-dev pkg-config \
    qt6-declarative-dev qt6-positioning-dev \
    qt6-webchannel-dev qt6-webengine-dev \
    qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools \
    libocct-foundation-dev libocct-modeling-data-dev \
    libocct-modeling-algorithms-dev libocct-visualization-dev \
    libocct-ocaf-dev libocct-data-exchange-dev
```

Fedora:

```
sudo dnf install gcc-c++ git make pkgconf-pkg-config libsecret-devel \
    qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtpositioning-devel \
    qt6-qtwebchannel-devel qt6-qtwebengine-devel \
    qt6-qttools-devel qt6-linguist opencascade-devel
```

openSUSE:

```
sudo zypper install gcc-c++ git make pkgconf-pkg-config libsecret-devel \
    qt6-base-devel qt6-webchannel-devel qt6-webenginewidgets-devel \
    qt6-tools-devel occt-devel
```

Arch:

```
sudo pacman -S --needed base-devel git libsecret qt6-base qt6-webchannel \
    qt6-webengine qt6-tools opencascade
```

Get the sources
---------------
Clone the repository, initialize submodules, and enter the project directory:

```
git clone https://github.com/ZIMA-Engineering/ZIMA-CAD-Parts.git
cd ZIMA-CAD-Parts
git submodule update --init --recursive
```

Build
-----
```
qmake zima-cad-parts.pro
make -j$(nproc)
lrelease-qt6 locale/zima-cad-parts_cs_CZ.ts
```

On some distributions, the Qt 6 qmake and lrelease binaries are named `qmake6`
and `lrelease6`.

Build with a custom OCCT install, such as vcpkg, by passing `OCCT_ROOT`:

```
/path/to/vcpkg/vcpkg install opencascade:x64-linux
qmake "OCCT_ROOT=/path/to/vcpkg/installed/x64-linux" zima-cad-parts.pro
make -j$(nproc)
```

Install on Linux
----------------
After building, install the application binary, translation files, desktop
entry, and hicolor app icons:

```
sudo make install PREFIX=/usr/local
```

Use another `PREFIX` if desired, for example `PREFIX=/opt/zima`.
For staged packaging installs, combine it with `INSTALL_ROOT`:

```
make install PREFIX=/usr/local INSTALL_ROOT=/tmp/zima-cad-parts-root
```

Desktop environments can cache application metadata. If the launcher or
taskbar still shows a generic icon after install, log out and back in, or
refresh the application/icon cache for your desktop environment.

macOS OCCT builds
-----------------
Build with OCCT preview through vcpkg:

```
# Intel
/path/to/vcpkg/vcpkg install opencascade:x64-osx
qmake "OCCT_ROOT=/path/to/vcpkg/installed/x64-osx" zima-cad-parts.pro
make -j$(sysctl -n hw.ncpu)

# Apple Silicon
/path/to/vcpkg/vcpkg install opencascade:arm64-osx
qmake "OCCT_ROOT=/path/to/vcpkg/installed/arm64-osx" zima-cad-parts.pro
make -j$(sysctl -n hw.ncpu)
```

Homebrew can also be used as a local macOS convenience path:

```
brew install opencascade
qmake "OCCT_ROOT=$(brew --prefix opencascade)" zima-cad-parts.pro
make
```

Windows CI OCCT builds
----------------------
The Windows GitHub Actions build uses vcpkg binary caching for OCCT. The first
OCCT-enabled run for a given Windows runner/toolchain combination may build
`opencascade:x64-windows` from source; later matching runs restore the cached
binary package. When the MSVC toolchain or Windows SDK changes, the workflow
seeds a new cache key so rebuilt vcpkg packages can be saved.

Manual password-manager fixture
-------------------------------
Run the local browser/password test fixture with:

```
python3 tools/manual-tests/password_manager_fixture.py --port 18080
```

See `doc/password-manager.md` for the full manual verification checklist.

Run
---
```
./ZIMA-CAD-Parts
```

## Local filters, previews and languages

See [local filters and performance](doc/filters.md) and [localization](doc/localization.md).
The Windows release can be deployed to the repository root with windeployqt.
Application translations are compiled and embedded automatically by qmake.

## Local Windows release and generated files

Run ZIMA-CAD-Parts.exe from the repository root. Keep its DLL files,
QtWebEngineProcess.exe and deployed Qt folders beside it; these are runtime
dependencies. The app includes English, Czech, German, French and Russian.
The application catalogs are embedded, so loose app QM files are not needed.

Build out of source (for example in .build-release) to avoid mixing generated
UI headers, object files and source files. After deployment, intermediate
build folders can be removed. Personal filter backups and local cleanup
receipts are kept in .local-backups, which is excluded from Git.
Use windeployqt from the same Qt kit as qmake, with --release --force.
When copying OCCT runtime dependencies from vcpkg, exclude Qt6*.dll: that
installation can contain a different Qt version. Mixing Qt DLL versions
can prevent the executable from starting. Verify the deployed executable
starts using DLLs beside it, without adding the development Qt kit to PATH.

## Library protection, shared parameters and navigation

[Directory properties and library protection](doc/filters.md) describe the local
metadata lock, disabled Delete/Move controls and the immediate recursive action.
The lock does not inherit into new subdirectories. Copying out remains available.

Files such as `xxx.pdf`, `xxx.prt.1` and `xxx.prtz` share the `xxx` parameters.
Refreshing the listing preserves stored metadata; Pro/E imports use the highest
numeric revision. [Interface notes](doc/interface.md) cover the green active tabs,
navigation icons and the renamed Properties actions.

See [verification](doc/verification.md) for the build and test requirements and
the validation of the current Windows release.

## Approved distribution design

The [binding distribution and versioning policy](doc/distribution-policy.md)
defines the planned versioned Windows/Debian packages, shared source archives,
launchers and two-version retention. This is the approved implementation target;
the deployment instructions above describe the current application.

The first [Windows packaging implementation](doc/windows-distribution.md)
provides a clean versioned source/runtime bundle and configurable launcher.
It includes the updater helper; final publisher signing is a separate step.

The [GitHub updates guide](doc/github-updates.md) describes automatic
silent startup checks, an unobtrusive indicator beside Directory/Parts,
installation from Settings after confirmation, signed release assets,
rollback and two-version retention. Windows implementation and fixture tests
are available; Debian runtime verification remains pending.

[Debian packaging](doc/debian-distribution.md) is prepared for Debian 13 CI;
Linux runtime validation is still pending. Windows packages now use a native
root EXE launcher without a PowerShell dependency.

## Command line and built-in tools

The [CLI documentation](doc/cli.md) covers the Qt Core-only executable for
listing/filtering parts and reading saved shared metadata as JSON.
GUI and CLI reuse the same naming, enumeration and local parameter rules.

The [bottom command panel](doc/command-panel.md) shares the command processor
with the CLI and supplies the active tab as the default directory.

Enter `codex` in that panel for an optional [AI conversation](doc/ai-command-panel.md)
using your own ChatGPT account. Codex discovers the available commands and
reads directories as requested; Parts file changes go through the existing review.
Drag files/directories into the panel or use **Add to AI question** to insert
full quoted paths into your draft. System commands have an inline review
before running with your OS user permissions; Parts directory locks do not
constrain those commands.
Configure the native Codex executable and sign in under **Settings > AI**.
Use `/exit` to return to ordinary Parts commands.

The input stays editable while Codex is working. Completion, errors and Stop
preserve the next draft; it is sent only after your explicit submission.

The built-in [PS2PDF, PTC-Cleaner and STEP-Edit](doc/integrated-tools.md)
functions are available from the GUI and CLI. ZIMA-CAD-Sync is no longer offered.

The [GitHub updates guide](doc/github-updates.md) describes silent startup
checks, installation from Settings, signing and rollback. Packagers now require
the separately built `zima-cad-parts-update.pro` helper (`--updater`).
