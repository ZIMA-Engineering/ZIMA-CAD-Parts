# Appearance and navigation

The active main tab uses the ZIMA-CAD green background (`#4DD811`) with dark
text. **Directory** and **Parts** tabs use the same highlight. Inactive tabs
follow the system palette and use a light green hover highlight. Main tabs
show scroll buttons when space is limited.

Parts opens directly in its main window. The startup splash screen and its
General settings section have been removed; old splash preferences are ignored
and removed when settings are saved. The command-panel button is immediately
to the left of Settings, which is the last button on the top toolbar.

**Directory** and **Parts** tabs are wider and have a folder or part icon
before the label. **Filters**, **Refresh**, **Delete**, **Move** and **Copy**
buttons use a shared set of SVG icons. Icons and styling are embedded in
application resources under `gfx/navigation`; no additional icon theme
installation is required for these controls.

The former **Edit** directory context action is now **Directory properties**.
The corresponding data source action is **Data source properties**.

Directory properties includes **Protect against deletion and moving**, with
**Apply to subdirectories** on its right. The latter takes effect immediately,
regardless of whether the dialog is subsequently accepted or cancelled.
See the [directory and filter documentation](filters.md) for details,
limitations and storage of the local lock.

**Delete** and **Move** are greyed out and disabled for locked files, with
an explanatory tooltip. The same protection disables deletion in the tree
context menu. The state responds to changes in directory, selection and
lock settings. Multiple selections also account for selected files from
other directories. Copying out remains available, and locked subdirectories
do not prevent work on files in unlocked neighboring directories.

**Show ZIMA-CAD archive versions (.1, .2, ...)** follows the same convention
as Pro/E revision visibility: checked means visible. Previously saved
`ShowZimaVersions` values remain valid and require no settings migration.

The context menu for a root data source in the tree also provides **Open in
a new tab**. It opens that source's root in a new main tab, like the same
action on individual directories. The action is disabled if the root
directory is unavailable.

The [command panel](command-panel.md) is toggled with the terminal toolbar
button or **Ctrl+Shift+P**. [Built-in tools](integrated-tools.md) are available
from the directory context menu and commands; they have no separate toolbar
icons.
