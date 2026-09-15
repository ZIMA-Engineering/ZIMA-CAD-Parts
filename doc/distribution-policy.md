# Binding distribution and versioning policy

Approved by the project owner on September 15, 2026. This document defines
the implementation requirements; it does not describe a completed updater.
The same rules are intended for ZIMA-CAD and other ZIMA-Engineering programs
later. This document does not modify their repositories.

## Release identity

- Releases, distribution archives and Git tags use names such as
  `ZIMA-CAD-Parts-2026091501`.
- The internal number follows `YYYYMMDDNN`: the build/release date and a
  two-digit sequence within that day. The same number identifies the source
  and corresponding official builds for each platform.
- **ZIMA-CAD-Parts 9** is only the marketing name on the About page. The nine
  is not used for technical versions, archive names, tags or update decisions.
  About also shows the actual build number.
- The official launch is planned for the owner's birthday; this specification
  does not establish a date.

## Distribution structure

The final artifact is one shared `ZIMA-CAD-Parts-YYYYMMDDNN.zip` archive,
without a platform in its name. It contains the `ZIMA-CAD-Parts` directory
with matching sources and the available official platform builds: Windows,
Debian, or both. A release must not wait for the other platform to be ready.
Each client offers the newest verified release available for its own platform.
A newer release for the other platform does not hide an applicable update.

Before publication, another platform can be added to the draft package and
the shared directory repackaged. Merging must preserve settings for an existing
platform. Every included runtime must match the release's source version and
commit; checksums and signatures must be regenerated after adding files.
Published assets are immutable. Adding a platform after publication requires
a new release number; it does not replace the published ZIP or manifest.

Local `.dist-output/` directly contains `ZIMA-CAD-Parts/` and the final ZIP,
without a permanent intermediate `windows-native-final` directory. Platform
CI outputs are intermediate artifacts for assembling the shared release,
not separate public editions.

The stable root directory contains two launchers. Versions live inside the
platform subdirectories, not in the name of this stable installation folder.
The full project license is also present beside the launchers.

```text
ZIMA-CAD-Parts/
    ZIMA-CAD-Parts.exe
    ZIMA-CAD-Parts.sh
    LICENSE
    windows/
        2026091501/
        2026091601/
    linux/
        2026091501/
        2026091601/
    source/
        2026091501/
        2026091601/
    custom/
        windows/
        linux/
    launcher.ini
```

Launchers select the configured version for their platform. The implemented
`launcher.ini` settings are described in the [Windows](windows-distribution.md)
and [Debian](debian-distribution.md) guides. Each official version contains
its own required runtime libraries, plugins and resources. Libraries from
different versions or platforms must not be mixed.

The source directory contains the complete project, resources, translations,
build definitions and actual submodule contents at matching revisions. It
must support a custom build after installing development dependencies.
Source archives must exclude working backups, personal data and temporary
builds. Project documentation is maintained in English. Third-party
licenses and notices remain with their components.

## Platforms

- Windows: an official ready-to-run build with required dependencies.
- Linux: initially only the latest stable Debian, x86-64, verified under
  KDE/Wayland. Each release must identify the exact supported Debian version.
- The Linux package bundles Qt, WebEngine, OCCT and required portable
  dependencies. Debian supplies core system libraries and graphics drivers.
  The exact bundled file list must be established and verified during
  implementation. See the Debian guide for current exceptions.
- End users extract and run the package; compilation is not required.
- A new stable Debian release becomes supported after build and runtime
  verification. Other distributions are currently outside the support target.

## Updates and version retention

Official packages are published as GitHub release assets matching a Git tag.
Updates download a ready-made package; binaries are not stored in the source
repository's regular history.

Windows and Debian updates are independent. Only the current platform must
be present in the signed manifest and archive. The other platform may be absent
and its installed version remains unchanged. Existing signature, source,
compatibility and explicit installation-confirmation requirements still apply.

A new version is prepared in a separate directory, verified and activated
after the application exits. A running installation is not overwritten.
The previous version provides a rollback option. At most two official
versions are retained per platform: current and previous. An incomplete
update must not remove a working version prematurely; older versions are
removed only after successful verification of the new version.

Source cleanup must preserve the sources of every retained official version
on both platforms. User and project data remain separate.

## Custom builds and package origin

Custom builds belong in `custom/`, can be selected for launching, and must
not be overwritten or automatically deleted by the updater. The two-version
limit does not apply to them.

Official ZIMA-Engineering builds and custom builds are distinguished in
version information, diagnostics and About. This information includes the
release number, commit and build origin. Official package authenticity must
be verified by a signature; text in editable sources does not prove origin.
The [GitHub update guide](github-updates.md) describes signing, discovery
and recovery details. It also records the owner's choice of automatic
silent startup checks with an unobtrusive tab-row indicator and installation
initiated from Settings. After confirmation the program downloads, verifies,
installs and restarts automatically. The key is provisioned and the Windows
implementation is available; Debian runtime verification remains required.

Testing and support apply to official, unmodified packages. Custom builds
are not verified by the publisher. This support policy adds no license
restrictions or legal warranty commitments.
