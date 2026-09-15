# Filtry a náhledy v Parts

Parts zobrazuje všechny typy souborů, včetně skrytých a systémových.
Adresář `0000-index` zůstává vyhrazený pro metadata a nezobrazuje se.
Pomocný soubor `.directory` se také vždy skrývá na Windows i Linuxu,
bez zápisu výjimky do lokálních filtrů; na disku zůstává zachovaný.
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
Původní seznam povolených typů v 0000-index/files.ini se pro zobrazení souborů
už nepoužívá. Soubor zůstává zachovaný, automaticky se nepřepisuje ani nemaže.
Aktuální pravidla ukládejte do filters.ini.

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
Na tomto Windows je sloučená aplikace sestavena pomocí Qt 6.10.1 MSVC
a Open CASCADE 8.0.0. Integrační testy přebírají stejné OCCT_ROOT jako aplikace.

## ZIMA-CAD

Typy dokumentů: .prtz díl, .asmz sestava, .drwz výkres, .frmz rámeček, .tblz razítko.
Aktuální dokument nemá číselnou příponu. Soubory .1, .2, ... jsou archivní kopie
předchozích uložení. ShowZimaVersions=false skryje všechny takové archivy, i když
aktuální dokument chybí; nikdy je nevydává za aktuální verzi. Výchozí hodnota je true: v dialogu je volba „Zobrazit archivní verze ZIMA-CAD“ zaškrtnutá a číslované verze zůstávají viditelné.
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

## Sdílené parametry dílu

Soubory stejného dílu (například `xxx.pdf`, `xxx.prt.1`, `xxx.prt.10`
a `xxx.prtz`) používají společné parametry pod názvem `xxx`. Editace
v tabulce i dialogu pracuje se stejným záznamem a aktualizuje všechny
odpovídající řádky. Názvy obsahující tečky zachovávají společný základ.

Běžné obnovení seznamu nemaže záznamy v metadata.ini. Dříve chybně uložené
hodnoty pod celým názvem existujícího souboru zůstávají zachované a slouží
jako náhradní hodnoty, pokud společný záznam ještě není vyplněný.

Parametry Pro/E se načítají pouze z nejvyšší číselné verze každého CAD
souboru, nezávisle na nastavení viditelnosti verzí. Prázdná načtená hodnota
nepřepisuje vyplněný parametr.

## Ochrana knihovny před smazáním a přesunutím

Ve **Vlastnostech adresáře** lze zaškrtnout **Chránit před smazáním a
přesunutím**. Volba se ukládá do místního `0000-index/metadata.ini`:

```ini
[Directory]
PreventRemoval=true
```

Výchozí hodnota je `false`. Zámeček se nedědí: chrání pouze soubory přímo
v tomto adresáři. Podadresáře lze zamknout samostatně. Odemčené podadresáře
zůstávají použitelné i uvnitř zamčené knihovny.

Parts odmítne také odstranění nebo přesun celé složky obsahující zamčený
adresář. Kopírování z knihovny je povolené; přepsání zamčeného souboru
kopírováním nebo přesunem je zablokované. Parametry lze nadále upravovat.
Jde o ochranu před omylem uvnitř Parts, nikoliv o oprávnění operačního systému.

Zamčené soubory mají tlačítka **Smazat** a **Přesunout** neaktivní a zašedlá.
Vysvětlení je dostupné v bublinové nápovědě. Stav se mění při přepnutí adresáře,
změně výběru i po zamčení nebo odemčení ve Vlastnostech adresáře.

Tlačítko **Aplikovat na podadresáře** napravo od zámečku okamžitě zapíše
aktuální stav zaškrtávátka do všech existujících podadresářů, i v dalších
úrovních. Zaškrtnuto zamyká, nezaškrtnuto odemyká. Ostatní metadata zůstávají
zachována. Systémové adresáře `0000-index` a odkazy na adresáře se neprocházejí.
Nově vytvořené podadresáře nastavení automaticky nepřebírají.

Operaci lze zastavit; již provedené změny zůstávají uložené. Výsledek uvádí
počet aktualizovaných adresářů a případné chyby. Zrušení okna Vlastností
nevrací hromadnou změnu zpět. Nastavení samotného otevřeného adresáře se
nadále potvrzuje tlačítkem OK.
