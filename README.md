ZIMA-CAD-Parts
==============

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
    qt6-base-dev qt6-base-dev-tools qmake6 \
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
