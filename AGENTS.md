# Repository Guidelines

## Project Structure & Module Organization
- Core Qt/C++ sources and UI forms live in `src/` (widgets, dialogs, models, product view extensions under `src/extensions/`).
- DXF/PROE parsers are vendored in `libqdxf/` and `libproe/`.
- UI assets are under `gfx/`; translation catalogs under `locale/`; sample landing pages in `data/`.
- Additional docs for datasource rules sit in `doc/`; build metadata is defined by `zima-cad-parts.pro` and the generated `Makefile`.

## Build, Test, and Development Commands
- Prereqs: Qt 6.4+ with `core`, `gui`, `widgets`, `network`, `webenginewidgets` plus `qmake`, `g++`.
- Standard build: `qmake && make -j$(nproc)` from the repo root; run the app with `./ZIMA-CAD-Parts`.
- Always verify changes with a full build before submission, using the standard command above.
- Clean artifacts: `make clean`. Use `shell.nix` or `nix-build` for a pinned toolchain if you have Nix.

## Coding Style & Naming Conventions
- C++17 with Qt patterns; 4-space indentation; braces on new lines; keep Qt connects/signals grouped.
- Class and method names are PascalCase/camelCase (`MainWindow`, `showSettings`); member pointers typically prefixed with `m_`.
- Prefer Qt types (`QString`, `QVector`) and Qt logging (`qDebug`) over STL/stdio in UI code.
- Keep UI text in translation-aware strings; when touching `*.ui`, ensure identifiers stay descriptive.
- Keep the Czech translation (`locale/zima-cad-parts_cs_CZ.ts`) in sync with code/UI text changes; run `lupdate` and translate new entries when strings change.

## Testing Guidelines
- No automated test suite is present; run the built binary and exercise core flows (load a datasource, browse parts, open tech specs, download files).
- For datasource-related changes, validate against the sample structure described in `doc/datasource.md` and ensure metadata/thumbnails render.
- File critical regressions as issues before merging when manual gaps exist.

## Commit & Pull Request Guidelines
- Commit messages follow short, imperative summaries (e.g., “Add Copy as... to directory context menu”).
- Scope commits narrowly; include rationale in the body if behavior changes or migrations are involved.
- PRs should describe the change, steps to verify, and risks; attach screenshots/GIFs for UI adjustments and note impacted dialogs/widgets.
- Link related issues or TODOs; mention platform nuances (Linux/Windows) when relevant.
