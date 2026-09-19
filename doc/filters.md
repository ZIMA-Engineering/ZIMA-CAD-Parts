# Parts filters and previews

Parts shows all file types, including hidden and system files. The
`0000-index` directory remains reserved for metadata and is not displayed.
The `.directory` helper file is also always hidden on Windows and Linux,
without a local filter rule; it remains on disk. Whether subdirectories
appear as parts is still controlled by `Directory/SubdirectoriesAsParts`.

## Local exclusions

The **Filters...** button edits only the current directory. Settings are
saved in `0000-index/filters.ini` and are not inherited by subdirectories
or through metadata includes.

Example (the default exclusion list is empty):

```ini
[Filters]
Hide=*.bak, *.tmp, Thumbs.db
ShowVersions=false
ShowZimaVersions=false
```

Enter one file name or pattern per line in the dialog. `*` matches any
number of characters, and `?` matches one character. Patterns match the
entire file name without its path, case-insensitively on both Windows and
Linux. Refresh the Parts list after editing the INI manually.

`ShowVersions=false` displays only the highest numeric revision of
`*.prt.N`, `*.asm.N`, `*.drw.N`, `*.frm.N` and `*.neu.N` (`.10` takes precedence
over `.9`). If an exclusion hides the latest revision, an older revision
is not shown in its place. Without a local revision setting, the existing
application preference is used.

The former file type allow-list in `0000-index/files.ini` is no longer used
for file visibility. Existing files are preserved and are not automatically
rewritten or deleted. Save current rules in `filters.ini`.

Search in the Parts table header matches displayed values, including the
full file name and extension, without case sensitivity. It waits 120 ms
for further typing before filtering. Exclusions and latest revisions are
not recalculated on every keystroke.

## Thumbnails and icons

Images are decoded in a worker thread when the table requests a thumbnail.
Completed thumbnails update individual cells without resetting the list.
The in-memory thumbnail cache is limited to 32 MiB. Changing directories
clears the queue and ignores late results from the previous directory;
leaving Parts cancels pending requests. Reading one file already in progress
may finish, but switching directories does not wait for it.

File enumeration and metadata reading remain synchronous and can still take
time on a slow network drive. Decoding every image is no longer required
before using the list. Automatic column sizing samples at most 50 items.

Pro/E and ZIMA-CAD have dedicated icons; other files use Qt's system icon
provider. Thumbnails prefer images beside the part, then
`0000-index/thumbnails`, then `IncludeThumbnails`. Cyclic references stop
safely.

## Verification

The standalone tests do not require WebEngine:

```sh
mkdir .build-tests
cd .build-tests
qmake ../tests/parts-performance.pro
make
./parts-performance-tests
```

On Windows, use the matching Qt kit and `mingw32-make` or `nmake`. The
executable may be in the `release` subdirectory. For tests without windows,
set `QT_QPA_PLATFORM=offscreen`.

The integration tests use the real model and proxy and reuse objects from
an already built application:

```sh
qmake ../tests/parts-integration.pro PARTS_OBJECTS_DIR=/absolute/path/to/objects
make
```

Use the same Qt kit and release configuration as the application. The
Windows build was verified with Qt 6.10.1 MSVC and Open CASCADE 8.0.0.
Integration tests use the same `OCCT_ROOT` as the application. See
[verification](verification.md) for complete build instructions and results.

## ZIMA-CAD

Document types are `.prtz` (part), `.asmz` (assembly), `.drwz` (drawing),
`.frmz` (frame) and `.tblz` (title block). The current document has no numeric
extension. Files ending in `.1`, `.2`, etc. are archive copies of previous
saves. `ShowZimaVersions=false` hides all such archives, even if the current
document is missing; archives are never treated as the current version.
The default is `true`: **Show ZIMA-CAD archive versions (.1, .2, ...)** is
checked and numbered versions remain visible.

This setting is independent of Pro/E and applies only to the current
directory. Icons are copies of the original SVG files from
`ZIMA-CAD/resources/icons`, stored directly in Parts.

## Running the Windows build

The deployed release executable is `ZIMA-CAD-Parts.exe` in the repository
root. It needs the adjacent DLLs and Qt directories. Close old instances
before testing a new build to avoid confusing versions or concurrent writes
to shared preferences.

Inactive tabs load their contents on first display. Parts opens directly in
the main window, without a splash screen. After a directory is deleted, the tree selects its parent;
QFileSystemModel updates only the changed branch. Other expanded branches
are not reset.

## Shared part parameters

Files belonging to the same part, such as `xxx.pdf`, `xxx.prt.1`,
`xxx.prt.10` and `xxx.prtz`, share parameters under the name `xxx`. Edits
in the table and dialog use the same record and update all matching rows.
Names containing dots retain their common base name.

A normal refresh does not delete records from `metadata.ini`. Values
previously saved incorrectly under an existing file's full name remain
available as fallback values when the shared record is not yet populated.

Pro/E parameters are imported only from the highest numeric revision of
each CAD file, regardless of revision visibility. An empty imported value
does not overwrite a populated parameter.

## Protecting libraries from deletion and moves

In **Directory properties**, enable **Protect against deletion and moving**.
The setting is stored in the local `0000-index/metadata.ini`:

```ini
[Directory]
PreventRemoval=true
```

The default is `false`. The lock is not inherited: it protects only files
directly in this directory. Subdirectories can be locked separately.
Unlocked subdirectories remain usable within a locked library.

Parts also rejects deleting or moving a whole folder containing a locked
directory. Copying out of the library is allowed, while overwriting a
protected file through copying or moving is blocked. Parameters remain
editable. This prevents mistakes within Parts; it is not an operating system
permission.

**Delete** and **Move** are disabled and greyed out for locked files. A
tooltip explains why. The state updates when the directory or selection
changes, or after locking or unlocking in Directory properties.

**Apply to subdirectories**, to the right of the lock, immediately writes
the current checkbox state to all existing subdirectories at every depth.
Checked locks them; unchecked unlocks them. Other metadata is preserved.
System `0000-index` directories and directory links are excluded. Newly
created subdirectories do not inherit this setting automatically.

The operation can be stopped; completed changes remain saved. The result
reports the number of updated directories and any errors. Cancelling the
Properties dialog does not undo this bulk change. The open directory's own
setting is still confirmed with **OK**.
