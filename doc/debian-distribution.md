# Debian distribution: awaiting verification

The target environment is Debian 13 (trixie), amd64. See
[Debian Releases](https://www.debian.org/releases/) for release information.
Qt Wayland and image plugins are installed by the package manager during
the build. Relevant packages are
[qt6-wayland](https://packages.debian.org/trixie/qt6-wayland),
[qt6-svg-plugins](https://packages.debian.org/trixie/qt6-svg-plugins) and
[qt6-image-formats-plugins](https://packages.debian.org/trixie/qt6-image-formats-plugins).

**Status:** the Windows workstation has neither WSL nor Docker. The script
and build have not been run on Debian, and no verified Linux binary is
available yet. The new CI workflow targets Debian 13 to build and prepare
an experimental archive; it does not replace KDE/Wayland runtime tests.

## Build

From a Git checkout with initialized submodules:

```sh
docker build --pull --build-arg BASE_IMAGE=debian:13 --build-arg DISTRO=debian -f .github/ci/linux-build.Dockerfile -t parts-debian-build .
docker run --rm --user "$(id -u):$(id -g)" --volume "$PWD:/workspace" --workdir /workspace --env HOME=/tmp parts-debian-build bash .github/ci/build-debian.sh
```

Output goes to `.dist-output/debian/ZIMA-CAD-Parts/` and the corresponding
`.tar.gz`. `package-debian.py` checks Debian 13, architecture and executable
version, exports tracked sources including submodules, and bundles Qt,
WebEngine, Wayland/X11 plugins, OCCT and recursively resolved libraries.
It sets portable ELF paths with `patchelf` and refuses to overwrite existing
output.

Glibc, graphics interfaces and drivers remain system-provided. The manifest
lists specific host libraries. The WebEngine sandbox remains enabled;
run the application as a regular user. Finding libraries with `ldd` alone
does not establish completeness of dynamically loaded plugins or KDE themes.

PS2PDF currently requires the system `ghostscript` package on the target
machine: `sudo apt install ghostscript`. It is listed in the manifest as a
system dependency. Windows bundles it; a portable Linux Ghostscript runtime
has not yet been prepared.

The full Parts license is included as `LICENSE` beside the root launcher,
in the source snapshot and in the runtime's `licenses/Parts-LICENSE`.
Source headers specify `GPL-3.0-or-later`; dependencies retain their own
licenses and notices.

## Run

```sh
./ZIMA-CAD-Parts.sh
./ZIMA-CAD-Parts.sh -Version 2026091501
./ZIMA-CAD-Parts.sh -Custom -Version my-build
./ZIMA-CAD-Parts.sh -Check
```

`launcher.ini` uses the `linux` and `linux_custom` keys. Each build in the
official directory branch has `build.ini` with its version and the platform
`debian-13-x86_64`. The manifest is not a cryptographic signature. Custom
builds live in `custom/linux/`. The versioned launcher sets library and
resource paths only for its own process.

Before a public release, verify the bundle on clean Debian with KDE/Wayland:
startup, web pages/PDFs, CAD import and rendering, system icons, passwords
through the system service, version switching and copying the whole bundle
to a different path. The distribution license inventory for all bundled
system packages and signing also need completion. These results have not
yet been confirmed.

## Updater build

The build image includes `qt6-base-private-dev`, `libssl-dev` and `pkg-config`.
Build `zima-cad-parts-update.pro` with the same Qt kit as the GUI and pass the
result to the packager with `--updater`. The helper and OpenSSL are bundled.
Signing uses the shared [publisher workflow](github-updates.md); add installation
and rollback to the pending KDE/Wayland acceptance run.
