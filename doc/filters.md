# Filtry a náhledy v Parts

Parts zobrazuje všechny typy souborů, včetně skrytých a systémových.
Adresář `0000-index` zůstává vyhrazený pro metadata a nezobrazuje se.
Zobrazení podadresářů jako dílů dál řídí `Directory/SubdirectoriesAsParts`.

## Lokální výjimky

Tlačítko **Filters...** upravuje pouze aktuální adresář. Nastavení se ukládá do
`0000-index/filters.ini`; do podadresářů ani přes metadata includes se nedědí.

Příklad (výchozí seznam výjimek je prázdný):

```ini
[Filters]
Hide=*.bak, *.tmp, Thumbs.db
ShowVersions=false
ShowZimaVersions=false
```

V dialogu se zadává jeden název nebo vzor na řádek. `*` znamená libovolný počet
znaků, `?` jeden znak. Vzory se porovnávají s celým názvem souboru bez cesty,
bez rozlišování velikosti písmen, stejně na Windows i Linuxu.
Po ruční úpravě INI obnovte seznam v Parts.

`ShowVersions=false` zobrazí pouze nejvyšší číselnou verzi souborů
`*.prt.N`, `*.asm.N`, `*.drw.N`, `*.frm.N`, `*.neu.N` (tedy `.10` před `.9`).
Pokud výjimka skryje nejnovější verzi, starší verze se místo ní nezobrazí.
Bez lokálního nastavení verzí se použije dosavadní preference aplikace.
Původní seznam povolených typů se pro zobrazení souborů už nepoužívá.

Hledání v hlavičce Parts prohledává zobrazenou hodnotu, včetně celého názvu
souboru a přípony. Nerozlišuje velikost písmen. Při psaní čeká 120 ms na další
znak a pak filtruje; výjimky a nejnovější verze se nepřepočítávají při každém
stisku klávesy.

## Náhledy a ikony

Obrázky se dekódují v pracovním vlákně, až když si tabulka vyžádá náhled.
Hotové náhledy se doplňují do jednotlivých buněk bez resetu seznamu.
Paměťová cache náhledů má limit 32 MiB. Při změně složky se fronta vyprázdní
a opožděné výsledky staré složky se ignorují; při opuštění Parts se zruší
čekající požadavky. Již probíhající čtení jednoho souboru může doběhnout,
ale přechod do jiné složky na ně nečeká.

Samotný výpis souborů a metadata zůstávají synchronní: na pomalém síťovém disku
mohou stále chvíli trvat. Dekódování všech obrázků už není podmínkou pro práci
se seznamem. Automatické měření šířek sloupců je omezeno na 50 položek.

Pro/E a ZIMA-CAD mají vlastní ikony, ostatní soubory používají systémového poskytovatele Qt.
Náhledy zachovávají přednost obrázků vedle dílu, poté `0000-index/thumbnails`
a nakonec `IncludeThumbnails`. Cyklické odkazy se bezpečně ukončí.

## Ověření

Samostatné testy nepotřebují WebEngine:

```sh
mkdir .build-tests
cd .build-tests
qmake ../tests/parts-performance.pro
make
./parts-performance-tests
```

Na Windows použijte odpovídající Qt kit a `mingw32-make` nebo `nmake`;
výsledný program může být v podadresáři `release`.
Pro testy bez oken nastavte `QT_QPA_PLATFORM=offscreen`.

Integrační test skutečného modelu a proxy používá objekty již sestavené aplikace:

```sh
qmake ../tests/parts-integration.pro PARTS_OBJECTS_DIR=/absolutni/cesta/k/objektum
make
```

Je nutné použít stejný Qt kit a release konfiguraci jako u aplikace.
Na tomto Windows je aplikace sestavena pomocí Qt 6.10.1 MSVC v `.build-msvc/release`.

## ZIMA-CAD

Typy dokumentů: .prtz díl, .asmz sestava, .drwz výkres, .frmz rámeček, .tblz razítko.
Aktuální dokument nemá číselnou příponu. Soubory .1, .2, ... jsou archivní kopie
předchozích uložení. ShowZimaVersions=false skryje všechny takové archivy, i když
aktuální dokument chybí; nikdy je nevydává za aktuální verzi. Výchozí hodnota je true.
Přepínač je nezávislý na Pro/E a platí pouze pro aktuální složku.
Ikony jsou kopie původních SVG ze ZIMA-CAD/resources/icons, uložené přímo v Parts.

## Spuštění Windows sestavy

Spustitelný release program je v kořeni projektu: ZIMA-CAD-Parts.exe.
Potřebuje přiložené DLL a adresáře Qt. Před zkoušením nové sestavy zavřete staré instance,
aby nedocházelo k záměně verzí a souběžnému ukládání společných preferencí.

Neaktivní záložky načítají obsah při prvním zobrazení. Úvodní obrazovka zachovává
checkbox a nastavenou dobu zobrazení, ale používá časovač a neblokuje hlavní vlákno.
Po úspěšném smazání složky strom přejde na jejího rodiče; QFileSystemModel aktualizuje
jen změněnou větev. Ostatní rozbalené větve se neresetují.
