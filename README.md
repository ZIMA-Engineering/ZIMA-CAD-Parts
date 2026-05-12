ZIMA-CAD-Parts
==============

Requirements
------------
Qt 6.8 LTS or newer - modules `core`, `gui`, `network`, `widgets`,
`openglwidgets`, `webenginewidgets`, `webchannel`, plus `qmake`.

CAD preview for STEP, IGES, and STL files is enabled when Open CASCADE
Technology (OCCT) is available. The supported dependency path is vcpkg with
`OCCT_ROOT` passed to qmake. When OCCT is not found, the application still
builds without the OCCT preview.

Debian 13 dependencies
----------------------
Install the Qt 6 development stack and toolchain via APT before building:

```
sudo apt install build-essential git libsecret-1-dev qt6-base-dev qt6-webchannel-dev qt6-webengine-dev qt6-tools-dev qt6-tools-dev-tools
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
qmake && make -j $(nproc)
lrelease-qt6 locale/zima-cad-parts_cs_CZ.ts
```

Build with OCCT preview on Linux through vcpkg:

```
/path/to/vcpkg/vcpkg install opencascade:x64-linux
qmake "OCCT_ROOT=/path/to/vcpkg/installed/x64-linux" zima-cad-parts.pro
make -j$(nproc)
```

Build with OCCT preview on macOS through vcpkg:

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

The Windows GitHub Actions build uses vcpkg binary caching for OCCT. The first
OCCT-enabled run may build `opencascade:x64-windows` from source; later runs
restore the cached binary package.

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
