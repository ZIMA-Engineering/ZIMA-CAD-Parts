# Application icon style

Application action icons use black (`#111111`) contours and the ZIMA-CAD
azure accent (`#00D1FF`). The native SVGs use a 24-unit canvas, consistent
stroke widths and transparent backgrounds. Navigation, directory commands,
built-in tools and metadata editing share this style.

Run `python tools/generate-icons.py` to regenerate the vector assets. Resources
are explicitly registered in `zima-parts.qrc`. Application-owned actions
use these resources instead of platform-specific standard icons. The Parts file
list uses Qt's native `QFileIconProvider` for each actual file.
The directory Tree keeps custom datasource logos first, then requests the native
folder icon. Windows uses its shell icons and registered file associations; Linux
uses the desktop/platform icon provider and theme available to Qt. Unknown types
receive the platform fallback. Numbered archive files use their actual filename,
so their icon can differ from the original CAD file type.

Application-owned action icons remain black and azure. Language flags, supplier
logos, thumbnails and the current
application identity are not action icons and retain their original appearance.

This change does not change system widget styling, selection colors, command
behavior, translations, file-type detection or persistent metadata. The
ZIMA-Parts name and ZP application mark are documented separately in the
[branding transition](RENAME_REVIEW.md), starting with build 2026092902.

Localization review: no user-visible strings were added or changed. SVG type
abbreviations are file-format identifiers, not translated UI labels.
