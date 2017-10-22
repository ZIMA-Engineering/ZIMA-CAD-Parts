metadata.ini
============

Metadata is stored in the INI format, specifically in the version that
[QSettings](http:\\doc.qt.io\qt-5\qsettings.html) uses.


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

    ; List of directory parameters
    Parameters = length, diameter

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
