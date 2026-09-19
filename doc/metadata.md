metadata.ini
============

Metadata is stored in the INI format, specifically in the version that
[QSettings](https://doc.qt.io/qt-6/qsettings.html) uses.


## Format versions
Version declares how should ZCP read this file. Each version defines what
sections and settings can be set. Versions are usually not backward
compatible, instead, ZCP knows how to migrate from older to newer version.
The current version is `2` and is stored in `Directory.Version`.

Possible sections and settings are described in the following `metadata.ini`
example.

    ; Section describing the directory
    [Directory]
    ; Version of this file format
    Version = 2

    ; Directory labels are localized
    Label\en = English label
    Label\cs = Český název

    ; Determines whether to show direct subdirectories in the parts list
    SubdirectoriesAsParts = true\false

    ; Determines whether to show an auto-generated directory index when no
    ; user-provided index file is present. Enabled by default when unset.
    AutoIndex = true\false

    ; List of directory parameters
    Parameters = length, diameter

    ; Default sorting
    SortOrder = ascending/descending

    ; Section for configuration of parameters defined above
    [Parameters]
    length\Label\en = Length [mm]
    length\Label\cs = Délka [mm]
    length\Type = number

    diameter\Label\en = Diameter [mm]
    diameter\Label\cs = Průměr [mm]
    diameter\Type = number

    ; In this section, parts' parameters are set. Parameter values can be
    ; localized. They are always localized when set through ZCP.
    [Parts]
    screw-01\length = 10
    screw-01\diameter = 20
    screw-02\length\cs = 10
    screw-02\diameter\cs = 20

## Parameter types

 - Text
 - Number (integer or float)
 - Integer
 - Float
 - Money
 - Date
 - Time
 - Datetime

## Data inclusion
It is possible to include parameters and thumbnails from other directory:

    [Directory]
    ; Include both parameters and thumbnails from the parent directory
    IncludeParameters = ..
    IncludeThumbnails = ..

Included directory overrides data present in the current directory.

## Importing native ZIMA-CAD parameters

The existing part-parameter reload also reads current `.prtz` Parts and `.asmz`
Assemblies. It uses the same configured metadata columns and part base-name
mapping as the Pro/E reader; no separate metadata schema is required.

Both internal parameter keys and localized parameter names match column handles
case-insensitively. For example, the Czech names `nazev`, `polotovar`, `hmotnost`
and `mnozstvi` can populate the same columns as historical Pro/E parameters.
An explicit internal key takes precedence over a name alias. An alias shared
by multiple parameters is ignored rather than choosing an arbitrary value.

Values use the selected metadata language, falling back to the shared value
when that language has no entry. Literal UTF-8 text, commas, quotes, backslashes
and `@` prefixes are preserved. Empty values do not erase existing metadata;
zero is a valid value. Only configured column handles are imported.

Numbered ZIMA-CAD archives, including orphan archives, and Drawings are excluded.
When a current native document and a Pro/E file share a base name, the native
document's nonempty values take precedence. Malformed native metadata is skipped
before importing any values from that file. Source documents remain unchanged;
imported values are stored through the existing `metadata.ini` mechanism.

Reload the parts after saving parameter changes in ZIMA-CAD to update the table.
The reader does not calculate geometry or evaluate CAD relations: it imports
the values saved by ZIMA-CAD.
