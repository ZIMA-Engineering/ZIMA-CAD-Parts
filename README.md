ZIMA-CAD-Parts
==============

Requirements
------------
Qt 6.8 LTS or newer - modules `core`, `gui`, `network`, `widgets`,
`webenginewidgets`, `webchannel`, plus `qmake`.

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
