# File filters (`files.ini`)

ZIMA-CAD-Parts can define visible file formats for every data source in
`0000-index/files.ini`. A file in the data-source root defines the defaults.
A file in a child directory is read afterwards and overrides the inherited
values. Set `inherit=false` to start with an empty configuration instead.

If no `files.ini` exists between the data-source root and the current
directory, all regular files are shown. Known types use their application
icon and unknown types use the system's generic file icon. The `0000-index`
directory is always hidden, irrespective of letter case.

## Example

```ini
[files]
schema=1
inherit=true
groups=documents,proe,zima-cad

[group/documents]
title=Documents
patterns=*.pdf,*.step,*.stp,*.igs,*.iges
enabled=true
versionMode=none

[group/proe]
title=Pro/E files
patterns=*.prt,*.asm,*.drw
enabled=true
versionMode=proe
showVersions=false

[group/zima-cad]
title=ZIMA-CAD files
patterns=*.prtz,*.asmz,*.drwz
enabled=true
versionMode=zima-cad
showVersions=false
```

Groups control the visual organization in the Filters dialog. Patterns are
case-insensitive wildcard expressions. Several patterns are separated by a
comma. A disabled format can be recorded with, for example,
`disabledPatterns=*.drw,*.igs`. Do not use semicolons: INI readers interpret
them as comments.

## Version handling

- `versionMode=proe` recognizes numbered files such as `part.prt.1` through
  `part.prt.99999`. With `showVersions=false`, only the highest numeric version
  of each logical file is shown.
- `versionMode=zima-cad` recognizes history files such as `part.prtz.1`, while
  the unnumbered `part.prtz` is the current file. With `showVersions=false`,
  only the unnumbered current file is shown. If it does not exist, no history
  file is used as a fallback.
- `versionMode=none` applies no special version behavior.

Part names may contain dots and spaces. The extension and optional numeric
version are parsed from the right, so `ZE25.000.000.00.prt.1` has the part name
`ZE25.000.000.00`. Leading and trailing spaces, a trailing dot, and the
Windows-invalid characters `< > : " / \\ | ? *` are not accepted when a part
is renamed.
