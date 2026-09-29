# Independent ZIMA-Parts product

On September 29, 2026 the user explicitly replaced the continuity plan below
with a standalone product. From build 2026092903, executable, package, tag,
update, settings and credential-service identities use ZIMA-Parts. Old data
is neither deleted nor migrated. The following records the earlier decision
and its verified historical implementation.

# ZIMA-Parts naming review

Status: visible branding implementation approved September 29, 2026.
Build 2026092902 displays **ZIMA-Parts** and the black/azure **ZP** mark.
The repository rename was explicitly approved and completed. Executable names,
application identity, settings locations and release protocol remain unchanged.

## Visible branding implementation

Main-window and dialog captions and all five localized welcome pages use
ZIMA-Parts. Qt's application display name changes independently of its technical
application name. Desktop labels and platform icon assets use the new brand.
The Windows portable launcher embeds the same icon as the GUI executable.
The AI client's displayed title changes; its technical client name is retained.

`gfx/app-icon.svg` is the vector master. Build `tools/render-brand-icon.pro`
with Qt GUI/SVG, run the resulting tool with the SVG and an output directory,
then run `python tools/package-brand-icons.py <output-directory>`. This produces
PNG, multi-size ICO, ICNS and Linux hicolor assets directly from the vector.
Native Linux/macOS runtime acceptance is separate from generating those assets.

The production updater still queries the existing ZIMA-CAD-Parts repository.
GitHub redirects that endpoint to ZIMA-Engineering/ZIMA-Parts. The repository
retains numeric identity 56504127, its history and immutable releases.
The local Git origin now uses the canonical ZIMA-Parts repository URL.

## GitHub repository

Renaming the existing repository is preferable to creating a replacement:
GitHub preserves its history and redirects existing repository web and Git
traffic. Update local remotes and documentation afterward. Do not reuse the old
repository name, because doing so removes its redirect. GitHub Pages URLs and
references to a repository-hosted Action require separate review.
See [GitHub's repository renaming documentation](https://docs.github.com/en/repositories/creating-and-managing-repositories/renaming-a-repository).

## Application and update contracts

The visible name and black/azure **ZP** mark change independently
of technical identity. The current updater explicitly validates all of:

- Product identity `ZIMA-CAD-Parts` in installation markers and signed records.
- Tag prefix `ZIMA-CAD-Parts-` and archive name `ZIMA-CAD-Parts-<version>.zip`.
- Package root `ZIMA-CAD-Parts/` and platform executable names.
- The existing trust keys and protocol.

These checks are in `src/update/updatecore.cpp`, `updateinstall.cpp` and
`installationclient.cpp`. `updatehttp.cpp` queries the old repository URL,
accepts only the old tag prefix, and follows up to five approved HTTPS redirects.
Repository renaming alone should therefore be separable from product-format
renaming, but an actual old-client discovery/download test is required before
claiming successful compatibility after a rename.

`QCoreApplication::applicationName()` also remains `ZIMA-CAD-Parts`. Changing
this technical value can change Qt settings and application-data/cache paths;
browser profiles and credential-store use must be audited before moving them.
Changing only visible branding avoids that unintended reset.

## Recommended order

1. Agree on the displayed **ZIMA-Parts** name and **ZP** artwork. Retain the
   current technical IDs, installation paths and signing keys initially.
2. Update visible branding, localized UI, launch/desktop assets and documents;
   verify saved settings, datasource access, browser credentials and tools.
3. Rename the existing GitHub repository, update URLs, and test discovery and
   download through an older official client. Keep existing releases immutable.
4. If new executable/package/tag names are also required, design and test a
   transition release accepted by the old updater before publishing the new
   format. Test install, restart, rollback and version retention explicitly.

Simply changing every occurrence of the product name would cause older clients
to ignore new tags or reject packages. The repository rename is the small part;
the signed update and persisted user-data contracts determine the safe rollout.

## Completed repository and directory transition

On September 29, 2026 the user explicitly approved both repository and local
project-directory renaming. The repository is now
[ZIMA-Engineering/ZIMA-Parts](https://github.com/ZIMA-Engineering/ZIMA-Parts).
The old API URL resolves to the same repository ID, 56504127. Release
`ZIMA-CAD-Parts-2026092902` remains immutable and points to the original source
commit. Pages were disabled and no repository-hosted Action definitions existed.

The unmodified 2026092901 updater, still querying the old repository address,
found 2026092902 as installable and completed a real archive download and
preparation (`status:prepared`). Executable, signed product/tag/archive and
settings identities stay unchanged. Do not reuse the old repository name.

The development directory is now `../ZIMA-Parts` beside `../ZIMA-CAD`.
The source tree, ignored runtime dependencies, signing keys, custom data and
untracked user files moved together. The Git remote uses the new canonical URL.
Qmake regenerated all four development build projects at their new absolute
paths; GUI, CLI, updater and integration builds succeeded. No matching desktop
or Start Menu shortcuts required rewriting. Codex's saved project entry still
references the old directory; its available project tools do not expose a path
update. Reopen the new directory in Codex to replace that saved entry.

Post-move verification passed all 62 integration checks, 18 CLI checks and
four translation catalogs. The initial incremental test retained an embedded
old fixture path; a clean rebuild of the test project resolved it without
application source changes. The development executable reports 2026092902,
the portable launcher resolves its new absolute path, and updater status is
`trusted:true`, `installedVersion:2026092902`, `status:idle`. The directory and
repository transition adds no user-visible application strings.
