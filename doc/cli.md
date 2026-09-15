# CLI: první čtecí etapa

`ZIMA-CAD-Parts-cli` je samostatný konzolový program nad Qt Core. Neotevírá
okna, nespouští WebEngine ani správce náhledů. Na Windows má příponu `.exe`.
Spodní příkazový panel, zápis dat a AI nejsou součástí této etapy.

## Použití

```powershell
./ZIMA-CAD-Parts-cli.exe list "C:/CAD/Projekt" --language cs
./ZIMA-CAD-Parts-cli.exe list "C:/CAD/Projekt" --name sroub --json
./ZIMA-CAD-Parts-cli.exe params "C:/CAD/Projekt/xxx.prt.10" --language cs
./ZIMA-CAD-Parts-cli.exe --help
```

Na Debianu se používají stejné argumenty bez `.exe`. Výstup je vždy UTF-8
JSON (`--json` je volitelný). `list` vrací schemaVersion, adresář, jazyk,
sloupce a díly; každý díl má name, path, baseName, directory a parameters.
`params` vrací jeden explicitně vybraný existující soubor nebo adresář;
viditelnost podle filtrů jeho přímé načtení neomezuje.

Parametry jsou **uložené hodnoty z metadata.ini**. CLI znovu neparsuje CAD
soubory a nesynchronizuje jejich obsah do metadat. Automatický Pro/E import
v GUI tedy může následně změnit uložené hodnoty; v této etapě není součástí
čtecího příkazu. Výběr zobrazených revizí používá stejná pravidla jako GUI.

Filtry se načítají výhradně z místního `0000-index/filters.ini` a nedědí se.
`0000-index` a `.directory` zůstávají skryté. Podadresáře jsou součástí výpisu
jen při Directory/SubdirectoriesAsParts. Volba verzí Pro/E respektuje
místní nastavení a jako výchozí hodnotu používá uložené globální nastavení
aplikace. `--default-proe-versions all|latest` změní jen tuto výchozí hodnotu,
nepřebije místní filters.ini. `--name` filtruje celé jméno bez rozlišení
velikosti písmen, stejně jako filtr sloupce názvu v GUI.

Jazyk je bez `--language` převzat z LanguageMetadata aplikace (výchozí en).
Používejte kódy metadat cs, en, de, fr nebo ru. Zachovává se dosavadní
jazykový fallback i čtení starších skupin pojmenovaných podle celého souboru.
PDF, Pro/E a ZIMA-CAD se stejným základním názvem sdílejí stejné parametry.

## Čtení bez změn v projektu

CLI neukládá uživatelské nastavení a nevytváří indexy, náhledy nebo zálohy
v projektu. Formát metadat v1 převádí existující migrací pouze v dočasné
kopii, kterou po skončení odstraní. Na disku projektu se nic nemění.
Podporuje IncludeParameters včetně relativních a absolutních cest;
cyklus, příliš hluboké vnoření nebo nečitelná metadata vrátí chybu.

Návratové kódy: 0 úspěch, 2 chybné argumenty, 3 chyba vstupních dat nebo
čtení, 4 chyba zápisu výstupu. Chyba je JSON na stderr, výsledky na stdout.
Ve veřejném JSON je `schemaVersion: 1`; názvy polí nejsou lokalizované.

## Společná logika

`src/core/partsread.*` sdílí GUI i CLI: seznam položek, základní názvy dílů,
čtení místních hodnot včetně legacy fallbacku a řešení cest includes.
`LocalFilters` zůstává společným pravidlem filtrování. File, PartCache a
Metadata delegují odpovídající operace na tuto vrstvu. `partsquery.*`
sestavuje čtecí snímek a JSON bez závislosti na widgetech.

Jde o začátek oddělení jádra: GUI nadále má své cache, migrace a zápisové
operace. Nejedná se o kompletní přepis všech modelů a dialogů.

## Sestavení a distribuce

```sh
mkdir .build-cli
cd .build-cli
qmake ../zima-cad-parts-cli.pro
make
```

Na Windows použijte Qt MSVC developer shell a `nmake release` místo make.
Cílový projekt potřebuje pouze Qt Core 6.8+ a C++17. GUI se sestavuje jako
dosud. Oba balicí skripty vyžadují `--cli` s cestou k odpovídající CLI
binárce a ověřují její číslo verze. Windows ji ukládá vedle verzovaného
GUI EXE; Debian do verzovaného bin/. Sestavení v CI zahrnuje také CLI.

## Ověření

`tests/test_cli.py` spouští skutečný proces podle PARTS_CLI_EXE. Ověřuje
Unicode, revize, místní filtry, parametry, starý formát, explicitně prázdné
hodnoty, includes, cykly, chybové kódy a nezměněné projektové soubory.
Na Windows byl proces testován pouze s Qt Core a jeho běhovými závislostmi,
bez GUI pluginů, s neplatně nastavenou platformou Qt.

Integrační test cliMatchesGuiReadResults porovnává skutečný JSON s GUI
modelem a filtrem. Při spuštění integrační sady nastavte PARTS_CLI_EXE;
bez něj se pouze tento procesový test přeskočí. Linuxové provedení těchto
změn zatím vyžaduje ověření v Debian CI.
