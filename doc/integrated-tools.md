# Built-in tools and CLI

PS2PDF, PTC-Cleaner and STEP-Edit are Parts functions available from a
directory's context menu in the tree, the CLI and the bottom command panel.
They have no separate toolbar icons, About dialogs or external ZIMA tool
executables. Default recursion and Cleaner rules are in Parts Settings,
on the **Tools and programs** page. ZIMA-CAD-Sync is no longer offered.
The original projects, installations and their saved settings are preserved.

## Common workflow

1. Select a file or directory, options and whether to include subdirectories.
2. Review the affected files and skipped items, with reasons. PTC-Cleaner and
   PS2PDF load their lists automatically; STEP-Edit uses **Preview**.
3. Select items and use **Clean**, **Create PDF**, or **Apply selected**.
   Confirmation shows the item count.

Changing options invalidates the preview. PTC-Cleaner and PS2PDF automatically
refresh it after a short typing pause; STEP-Edit requires another **Preview**. Before applying changes, the file
contents are checked again using SHA-256. The system directory `0000-index`,
symlinks and junctions are excluded from traversal. Reading and execution
run outside the UI thread. **Cancel** stops further work and terminates an
ongoing Ghostscript conversion. Completed changes remain in place. Results
report completed items, failures, skipped files and cancellation.

This is not a transaction protecting against concurrent writes from another
application. In particular, a STEP file being edited should not also be open
for writing in a CAD application.

## Commands

The same Qt Core implementation serves the GUI, panel and standalone CLI:

```text
ZIMA-CAD-Parts-cli ps2pdf "C:/project/drawing.ps"
ZIMA-CAD-Parts-cli ps2pdf "C:/project" --recursive --apply
ZIMA-CAD-Parts-cli ps2pdf "C:/project" --output-dir pdf --delete-source --apply
ZIMA-CAD-Parts-cli ptc-clean "C:/project"
ZIMA-CAD-Parts-cli ptc-clean "C:/project" --mask "trail.txt.*" --apply
ZIMA-CAD-Parts-cli ptc-clean "C:/project" --patterns-only --mask "*.log"
ZIMA-CAD-Parts-cli step-edit "C:/project/model.step"
ZIMA-CAD-Parts-cli step-edit "C:/project/model.step" --set "author=Vladimír" --apply
```

On Windows, the standalone executable has the `.exe` extension. In the
panel, omit the executable name. An omitted path uses the active tab;
the standalone CLI requires a path. Nothing changes without `--apply`.
The CLI returns UTF-8 JSON. Exit code 0 means success, 2 means invalid
arguments, and 3 means a data error or at least one failed or skipped item during
`--apply`. With code 3, JSON goes to stderr and may include items already
completed. Automation must check both the exit code and the `completed`,
`failed` and `skipped` lists. Code 4 indicates an output write failure.
The CLI does not load default masks or recursion from GUI settings:
the command explicitly defines its scope.

## PS2PDF

Converts PS, EPS and PostScript PLT files to PDF 1.7 using Ghostscript
`pdfwrite`. PLT files containing HPGL or PCL are not converted. The original
file remains unchanged unless source deletion is explicitly selected. The GUI
uses the relative output directory `pdf` by default. Relative output paths are
resolved from the selected source directory and are created when conversion
starts. The CLI creates PDFs beside their inputs unless `--output-dir` is used.
Existing PDFs are atomically replaced only after Ghostscript has produced and
validated the new PDF, so a failed conversion preserves the previous drawing.
Multiple inputs targeting the same output are reported as collisions in the
preview. **Delete PS source files after creating PDF** and CLI
`--delete-source` remove each PS, EPS or PostScript PLT source only after its
own PDF has been saved successfully. Ghostscript options are fixed, including
`-dSAFER`, and no shell is used.

The PS2PDF window follows the same automatic workflow as PTC-Cleaner. It uses
the directory from which it was opened, has no redundant source selector or
Preview button, keeps its options below the candidate list and runs modelessly
so the rest of Parts remains usable.

The Windows runtime includes Ghostscript 10.08.0 in `tools/ghostscript`,
its AGPL license and the corresponding source archive. Prepare it before
building a distribution:

```text
python tools/distribution/prepare-ghostscript.py --sevenzip "C:/Program Files/7-Zip/7z.exe"
```

The script checks pinned SHA-256 hashes of the official binary and source
archives and does not install Ghostscript system-wide. The packager checks
the manifest and includes the entire runtime and source archive. This binary
dependency is generated locally and is not committed to Git. See the
[Ghostscript releases](https://ghostscript.com/releases/) and upstream
[AGPL download information](https://ghostscript.com/releases/gsdnld.html).
The Parts license is separate: see [LICENSE](../LICENSE) and the
[Ghostscript notice](../licenses/Ghostscript-NOTICE.txt).

Debian 13 currently uses the system `ghostscript` package
(`sudo apt install ghostscript`), which must also be present on the target
machine. It is listed in the distribution manifest and installed in CI.
Bundling Linux Ghostscript is a separate step; this integration's Linux
build has not been verified on the Windows workstation.

## PTC-Cleaner

The dialog uses the directory from which it was opened in Parts. It has no
source-path field, file/directory browse buttons or Preview button. The initial
scan and rescans after changing recursion, revision rules or masks are automatic
and read-only. Recursion, revision rules and masks are below the file list.
**Clean** is enabled only for a completed list; it asks for confirmation before
moving the checked files to the trash. Successfully removed files disappear
from the list, leaving it empty when all candidates were cleaned. Unchecked
files and failures remain available. Changing options immediately retires the
old list. Each distinct file is hashed only once per scan, including a latest
revision shared by many older revisions; apply still rechecks file contents. Prepared AI approvals keep their captured list
and never trigger an automatic rescan.

The number after the last dot is the revision. Groups are identified by the
entire preceding name and directory: `part.v2.prt.2` and `part.v2.prt.10`
belong together, while `part.v2.asm.3` has a separate group. The highest
number is retained. This rule applies to numeric extensions, not only CAD
file types. Optional masks provide additional removal candidates, so they
can also select the latest revision. `--patterns-only` disables revision
comparison. Masks are case-sensitive, as in the original Cleaner.

On Windows, Cleaner queues the checked files into one native Shell recycle
operation on the worker thread. Each file is revalidated immediately before its
move. A validation failure can stop the remaining batch; unprocessed files stay
in the list. The Cleaner window is modeless, so the rest of Parts remains usable
while it is open. Directory-cache and auto-index notifications for the affected
tree are suspended during the batch and reconciled once when it ends; unrelated
directories continue updating. Closing a busy Cleaner requests cancellation.

Cleaner moves files to the system trash. If trash is unavailable, it reports
an error and does not fall back to permanent deletion. The
`Directory/PreventRemoval` lock applies only to files directly in that
directory and is checked again when applying changes. Before removing an
older revision, the retained latest revision is also checked.

## STEP-Edit

Reads the seven `FILE_NAME` fields in the HEADER section. `--set` supports
`name`, `date`, `author`, `organization`, `preprocessor`, `system` and
`authorization`. Unselected fields are preserved exactly. Author and
organization are lists when read; setting either replaces that list with
one supplied string. Commas and apostrophes are handled correctly, and
Unicode is written using STEP escape sequences. Without selected fields,
the command only reads; `--apply` requires at least one field. Name changes
are not implicitly applied to a batch.

Before writing, the original is copied to
`0000-index/tool-backups/<unique-id>-<filename>`. The file is replaced using
QSaveFile, and geometric DATA remains unchanged. Backups are not removed
automatically. This tool edits the STEP header, not geometry.

## Verification

`tests/test_tools.py` runs the actual CLI on temporary data, including
conversion with real Ghostscript, Unicode STEP content and backup checks.
Integration tests also cover changed previews and revalidation of source
files, retained revisions and locks. PDF and STEP dialogs were checked in
the running Windows application.

See [verification](verification.md) for the current build and test results.
The system trash test needs an environment that permits access to the trash;
a restrictive sandbox can block it. Linux runtime verification is pending.
