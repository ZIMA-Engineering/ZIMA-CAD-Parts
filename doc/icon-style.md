# Application icon style

Action and file-type icons use black (`#111111`) contours and the ZIMA-CAD
azure accent (`#00D1FF`). The native SVGs use a 24-unit canvas, consistent
stroke widths and transparent backgrounds. Navigation, directory commands,
built-in tools, metadata editing and supported file types share this style.
File icons retain category geometry and abbreviated type labels.

Run `python tools/generate-icons.py` to regenerate the vector assets. Resources
are explicitly registered in `zima-cad-parts.qrc`. Application-owned actions
use these resources instead of platform-specific standard icons. The file
provider caches supported file icons; unrecognized files use a generic document.
Directory providers preserve custom datasource logos before choosing the
default folder icon. Language flags, supplier logos, thumbnails and the current
application identity are not action icons and retain their original appearance.

This change does not change system widget styling, selection colors, command
behavior, translations, file-type detection or persistent metadata. The
ZIMA-Parts name and ZP application mark are documented separately in the
[branding transition](RENAME_REVIEW.md), starting with build 2026092902.

Localization review: no user-visible strings were added or changed. SVG type
abbreviations are file-format identifiers, not translated UI labels.
