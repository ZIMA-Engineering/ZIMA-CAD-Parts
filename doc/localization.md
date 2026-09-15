# Interface languages

Parts supports English (`en_US`), Czech (`cs_CZ`), German (`de_DE`), French
(`fr_FR`) and Russian (`ru_RU`). The toolbar flags appear in that order, with
Russian on the right.

Russian support in ZIMA-CAD is a separate future task. Adding it to Parts
does not modify ZIMA-CAD source files or settings.

A flag switches both the interface and metadata language. The same choice
is available in Settings. The change is saved and takes effect without a
restart. Automatic detection uses the system language and normalizes
regional variants, for example `fr_CA` to `fr_FR` and `ru_BY` to `ru_RU`.
Unsupported languages fall back to English. Previously saved Russian
preferences are supported again.

Application UI text is translated, including dialogs, search, settings,
file operations and the built-in About page. Technical CLI/core diagnostics
and command help remain in English. User directory names, metadata, scripts
and external web pages are not translated automatically. Metadata uses the
language-specific values stored in project files.

## Maintaining translations

English is the source language. Complete translation catalogs are in `locale/`:

- `zima-cad-parts_cs_CZ.ts`
- `zima-cad-parts_de_DE.ts`
- `zima-cad-parts_fr_FR.ts`
- `zima-cad-parts_ru_RU.ts`

After changing UI strings, update the catalogs, complete the translations
and run the check:

```sh
lupdate zima-cad-parts.pro -no-obsolete
python tests/check_translations.py
```

qmake uses `CONFIG += lrelease embed_translations` to compile the catalogs
and embed them in application resources under `/i18n`. Separate application
QM files are not required at runtime. Standard Qt dialogs use merged
`qt_cs.qm`, `qt_de.qm`, `qt_fr.qm` and `qt_ru.qm` catalogs in `translations/`,
prepared by `windeployqt` on Windows. Linux can also load the system catalogs
`qtbase_cs.qm`, `qtbase_de.qm`, `qtbase_fr.qm` and `qtbase_ru.qm`.

Integration tests cover all five flags, actual toolbar text changes, French
and Russian filter dialogs, regional variants, unsupported language fallback,
persistence and switching back to English.

The current catalogs contain 454 messages each, covering password management,
automatic indexes, CAD previews, directory properties, protection locks,
recursive lock application, the command panel and built-in tools. Checks
include Czech, German, French and Russian plural forms. Russian plurals are
also tested at runtime with counts 1, 2, 5 and 21.

Project documentation is maintained in English, as required by
[AGENTS.md](../AGENTS.md). This does not change the languages of UI catalogs
or examples demonstrating localized metadata.
