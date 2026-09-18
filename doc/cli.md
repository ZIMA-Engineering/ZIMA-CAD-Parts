# CLI: reading parts and using built-in tools

`ZIMA-CAD-Parts-cli` is a separate console application built on Qt Core. It
opens no windows and does not start WebEngine or the thumbnail manager. On
Windows, the executable has the `.exe` extension. The [bottom command
panel](command-panel.md) uses the same command processor.
The `list` and `params` commands are read-only. The `ps2pdf`, `ptc-clean`
and `step-edit` functions can apply changes with `--apply`; see
[built-in tools](integrated-tools.md). AI is not implemented yet.

## Usage

```powershell
./ZIMA-CAD-Parts-cli.exe list "C:/CAD/Project" --language en
./ZIMA-CAD-Parts-cli.exe list "C:/CAD/Project" --name screw --json
./ZIMA-CAD-Parts-cli.exe params "C:/CAD/Project/xxx.prt.10" --language en
./ZIMA-CAD-Parts-cli.exe --help
```

On Debian, use the same arguments without `.exe`. Command results are UTF-8
JSON (`--json` is optional); help and version output are plain text.
`list` returns `schemaVersion`, directory, language, columns and parts.
Each part has `name`, `path`, `baseName`, `directory` and `parameters`.
`params` returns one explicitly selected existing file or directory;
visibility filters do not restrict this direct lookup.

Parameters are **saved values from metadata.ini**. The CLI does not parse
CAD files again or synchronize their contents into metadata. Automatic Pro/E
import in the GUI may subsequently change saved values; it is not part of
these read commands. Displayed revisions follow the same rules as the GUI.

Filters are loaded only from the local `0000-index/filters.ini` and are not
inherited. `0000-index` and `.directory` remain hidden. Subdirectories appear
in the listing only when `Directory/SubdirectoriesAsParts` is enabled.
Pro/E revision visibility follows local settings, using the application's
saved global preference as the default. `--default-proe-versions all|latest`
changes only that default; it does not override the local `filters.ini`.
`--name` matches the full file name without case sensitivity, like the
GUI's name column filter.

Without `--language`, the language comes from the application's
`LanguageMetadata` setting (default: `en`). Use metadata language codes
`cs`, `en`, `de`, `fr` or `ru`. Existing language fallback and support for
legacy groups named after the full file name are preserved. PDF, Pro/E and
ZIMA-CAD files with the same base name share parameters.

## Reading without changing project files

The `list` and `params` commands do not save user settings or create indexes,
thumbnails or backups in the project. Version 1 metadata is converted using
the existing migration in a temporary copy, which is removed afterwards.
Project files remain unchanged. `IncludeParameters` supports relative and
absolute paths; a cycle, excessive nesting or unreadable metadata produces
an error.

Exit codes: 0 for success, 2 for invalid arguments, 3 for input or read
errors, and 4 for an output write failure. Errors are JSON on stderr;
results go to stdout. Public JSON uses `schemaVersion: 1`, and field names
are not localized.

## Shared logic

`src/core/partsread.*` is shared by the GUI and CLI: item enumeration, part
base names, local parameter lookup with legacy fallback, and include path
resolution. `LocalFilters` supplies the shared filtering rules. File,
PartCache and Metadata delegate the corresponding operations to this layer.
`partsquery.*` builds the read snapshot and JSON without widget dependencies.

This is the first stage of separating the core. The GUI still has its own
caches, migrations and write operations; the models and dialogs have not all
been rewritten.

## Build and distribution

```sh
mkdir .build-cli
cd .build-cli
qmake ../zima-cad-parts-cli.pro
make
```

On Windows, use a Qt MSVC developer shell and `nmake release` instead of
`make`. This target needs only Qt Core 6.8+ and C++17. Build the GUI as usual.
Both packaging scripts require `--cli` pointing to the matching CLI binary
and check its version number. Windows places it beside the versioned GUI
executable; Debian places it in the versioned `bin/`. CI also builds the CLI.

## Verification

`tests/test_cli.py` starts the real executable selected by `PARTS_CLI_EXE`.
It checks Unicode, revisions, local filters, parameters, legacy metadata,
explicit empty values, includes, cycles, exit codes and unchanged project
files. On Windows, the process was tested with only Qt Core and its runtime
dependencies, without GUI plugins and with an invalid Qt platform setting.

The integration test `cliMatchesGuiReadResults` compares actual JSON with
the GUI model and filter. Set `PARTS_CLI_EXE` when running the integration
suite; otherwise, only this process test is skipped. These changes still
need Linux verification in Debian CI. See [verification](verification.md)
for the latest recorded results.

## Built-in tools

`ps2pdf`, `ptc-clean` and `step-edit` share their implementation with the GUI.
Their previews, `--apply` execution and file modification rules are described
in the [tools documentation](integrated-tools.md). The read-only guarantees
above apply to `list` and `params`, not to tool execution.

For `ps2pdf`, `--output-dir` accepts an absolute path or a path relative to the
selected source directory. `--delete-source` removes a source only after its
new or replaced PDF has been saved successfully.

## Updates

`update check`, `update download --target YYYYMMDDNN`, `update status`,
`update install --target YYYYMMDDNN --apply` and `update rollback --apply`
use the same helper as Settings. Installation and rollback are asynchronous
and restart Parts after its instances close. Without `--apply`, they return
`confirmation-required`. See [GitHub updates](github-updates.md) for prerequisites
and result files. The active project path never selects the installation root.
