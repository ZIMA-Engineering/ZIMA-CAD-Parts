ZIMA-CAD-Parts
==============

Requirements
------------
Qt 6.4 or newer - modules core, gui, network, widgets and webenginewidgets, qmake.

Debian 13 dependencies
----------------------
Install the Qt 6 development stack and toolchain via APT before building:

```
sudo apt install build-essential git qt6-base-dev qt6-webengine-dev qt6-tools-dev qt6-tools-dev-tools
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

Run
---
```
./ZIMA-CAD-Parts
```
