# ZIMA-Parts naming review

Status: discussion only, September 29, 2026. No repository, executable,
application identity, settings location or release protocol has been renamed.

## GitHub repository

Renaming the existing repository is preferable to creating a replacement:
GitHub preserves its history and redirects existing repository web and Git
traffic. Update local remotes and documentation afterward. Do not reuse the old
repository name, because doing so removes its redirect. GitHub Pages URLs and
references to a repository-hosted Action require separate review.
See [GitHub's repository renaming documentation](https://docs.github.com/en/repositories/creating-and-managing-repositories/renaming-a-repository).

## Application and update contracts

The visible name and a future black/azure **ZP** mark can change independently
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
