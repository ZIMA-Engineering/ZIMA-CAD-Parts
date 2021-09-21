Data source
===========

Data source is a directory in the filesystem that follows some rules described
in this document. These rules allow ZIMA-CAD-Parts (ZCP) to render display
source's contents in GUI with human-friendly labels, icons and other parameters.
ZCP can work with many data sources, it is up to the user to logically divide
his data into separate data sources as he sees fit.

Directories within the data source are independent and can be moved around.
The only issue can be metadata includes, see `metadata.ini` section for more
information.

Example filesystem structure of a data source:

    my-data-source/
    ├── 0000-index/
    │   ├── prototypes/
    │   │   ├── prototype1/
    │   │   └── prototype2/
    │   ├── thumbnails/
    │   │   ├── part-01.png
    │   │   └── part-02.png
    │   ├── index.html
    │   ├── index-parts.html
    │   ├── logo.png
    │   └── metadata.ini
    ├── 0001-my-directory/
    │   └── 0000-index/
    ├── 0002-my-directory/
    │   └── 0000-index/
    ├── 0003-my-directory/
    ├── 0004-my-directory/
    ├── 0005-my-directory/
    ├── part-01.stp
    └── part-02.stp

Meaning of the special directories and files is described below.

## Metadata directory
Each directory within the data source and the data source itself can be
described by contents of special subdirectory called `0000-index`.
This directory does not show up in ZCP, it is used to gather information only.
The contents of this directory are described below.

## Directory icons
ZCP looks for directory icons in `0000-index` at:

 - `logo.png` for an icon that is to be shown alongside directory's label
 - `logo-text.png` for an icon that contains its own text, directory label
   is not shown

Only one of these icons should exist in a single directory.

## Technical specification
Technical specification viewer displays HTML files found in `0000-index`.
These files are used to describe the contents of the directory using text
or images. The HTML file can load images, CSS or JavaScript files that can be
stored alongside it. ZCP looks for the following index files, in order:

 - `index_<language>.html`
 - `index.html`

Where `<language>` is a language and country code, such as `en_US` or `cs_CZ`.
So localized file is tried first, generic one later.

## Parts index
Parts index is shown above the part list, the same rules as for technical
specification apply here, except for different file names:

 - `index-parts_<language>.html`
 - `index-parts.html`

If no file is found, the parts index viewer hides.

## Part thumbnails
Every directory entry can have a thumbnail. Thumbnails are looked for in
directory `0000-index/thumbnails`. The base name of the thumbnail must be the
same as the base name of the directory entry. For example, for part called
`screw-01.stp`, the thumbnail must be `screw-01.png`.

## Part parameters
Directory can define a set of parameters that its direct subdirectories
and files (parts) can set. If a directory contains e.g. screw parts,
the parameters could be length, diameter, strength, material and so on. Every
part can then specify these parameters. Parts and their parameters are shown
in a table, where parts are rows and parameters columns. These parameters
are stored in `0000-index/metadata.ini`.
See [metadata.md](metadata.md) for the description of its format.

## Prototypes
Directory can have multiple prototypes, which can be used when creating
subdirectories. Selected prototype is simply copied to the new directory.
Prototype can have pre-configured `metadata.ini`, index files and so on.
Prototypes are stored in `0000-index/prototypes/`.

## Scripts
Executable files are stored in `0000-index/scripts`. Scripts stored
in `0000-index/scripts` of the data source's root directory are offered
for all directories within the data source. ZCP offers to run the scripts
in a per-directory context menu. All executable files from the directory
are offered to the user.

The scripts are run through a terminal configured in ZCP settings.
The current working directory is set to the directory the script is run
on. The following environment variables are set:

  - `ZCP_WORKDIR` - the global working directory
  - `ZCP_DATASOURCE_ROOT` - absolute path to the datasource root directory
  - `ZCP_DIRECTORY` - absolute path to the directory the script is run on (same
    as the processes' working directory)
