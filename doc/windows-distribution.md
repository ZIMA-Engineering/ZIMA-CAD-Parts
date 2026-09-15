# První implementace distribučního balíku pro Windows

Tato etapa realizuje strukturu a spouštění podle [závazného návrhu](distribution-policy.md).
Automatické aktualizace, podpisy, mazání starých verzí a Debian runtime zatím
nejsou implementované. Vzniklý vývojový balík není podepsané oficiální vydání.

## Verze a sestavení

Jediným zdrojem čísla je `VERSION` v `src/zima-cad-parts.h` (YYYYMMDDNN).
Program ho uvádí v O programu a v `QCoreApplication::applicationVersion()`.
Volání `ZIMA-CAD-Parts.exe --build-info` vrací JSON bez spuštění GUI.
Marketingová devítka se zobrazuje pouze na stránce O programu.

Sestavte celý projekt pomocí qmake a nmake z MSVC developer shellu,
mimo zdrojový adresář, se stejným Qt kitem a OCCT jako pro distribuci.
Balení vyžaduje Python 3, Git, MSVC `dumpbin` a `windeployqt` tohoto kitu.

```powershell
python tools/distribution/package-windows.py --exe .build-release/release/ZIMA-CAD-Parts.exe --qt C:/Qt/6.10.1/msvc2022_64 --occt C:/zb/i/x64-windows --output .dist-output/2026091501
```

EXE musí pocházet z tohoto checkoutu. Pokud není schopné najít DLL v build
adresáři, přidejte pro balení do PATH `bin` příslušného Qt a OCCT. Skript
ověřuje číslo EXE proti zdrojům, nikoli úplnou reprodukovatelnost kompilace.
Výstup musí být nový adresář; existující balík se nepřepisuje.

Zdroje se kopírují podle Git indexu, včetně skutečného obsahu inicializovaných
submodulů. Ve vývojovém balíku jde o aktuální obsah pracovních souborů;
manifest označí změněný checkout. Nové dosud nesledované soubory lze přidat
jednotlivě pomocí `--include cesta`. Žádné ostatní nesledované soubory
se automaticky nepřibalují. Úplnost exportu je nutné před vydáním ověřit
sestavením přímo z exportovaných zdrojů.

Přepínač `--release` vyžaduje čistý checkout a tag `ZIMA-CAD-Parts-<VERSION>`
na HEAD. Vytváří kandidáta vydání; sám nezajišťuje podpis ani oficiální
ověřenou distribuci. GitHub workflow používá tento skript; datumový tag vytváří koncept vydání
s archivem. Dokud není implementovaný podpis, nejde o automaticky
publikované podepsané vydání.

Qt nasazuje windeployqt; další DLL se dohledávají rekurzivně podle importů
přes dumpbin. Qt knihovny mají přednost z vybraného Qt kitu. Nevyřešená
závislost balení zastaví. Dynamicky načítané komponenty je nutné ověřit
běhovými testy, samotný seznam importů jejich úplnost nedokazuje.

## Spouštění a přepínání

V kořeni vytvořené složky ZIMA-CAD-Parts spusťte `ZIMA-CAD-Parts.exe`.
Jde o malý nativní Win32 spouštěč se staticky připojeným MSVC runtime;
nepotřebuje Qt ani povolení PowerShell skriptů. Samotná aplikace je ve
verzované složce `windows/<verze>/`.

`launcher.ini` vybírá Windows verzi; `windows_custom=false` znamená
oficiální adresářovou větev. Vývojový balík ve stejné struktuře je stále
v manifestu označen jako development, nikoli jako ověřené vydání.

```powershell
./ZIMA-CAD-Parts.exe -Version 2026091501
./ZIMA-CAD-Parts.exe -Custom -Version moje-sestaveni
./ZIMA-CAD-Parts.exe -Check
```

Vlastní sestavení leží v `custom/windows/moje-sestaveni/`, včetně svého
EXE a knihoven. Přepínač `-Check` ověří výběr a vypíše cestu bez spuštění.
Spouštěč kontroluje formát názvu a u vydání shodu `build.ini` (číslo a platforma). Neověřuje
kryptografický podpis. Pro nový proces odstraní vývojové Qt cesty a omezí
PATH na složku programu a systém Windows.

Linuxový kořenový spouštěč vybírá standardní nebo vlastní sestavení
a kontroluje build.ini. Bez Debian balíku skončí s chybou. Linux zde
zatím nebyl spuštěn; postup popisuje dokumentace Debian distribuce.
Do složek lze ručně umístit aktuální a předchozí sestavení; tato etapa sama
žádnou verzi nestahuje ani neodstraňuje.

`checksums.json` obsahuje SHA-256 zabalených souborů. Slouží ke kontrole
integrity, ne jako podpis původu. `version.json` uvádí verzi, commit,
platformu, Qt kit a vývojový stav.

## Ověření první etapy

Kontroly spouštěče a exportu: `python tests/test_distribution.py`.
Testy ověřují výběr aktuální i předchozí verze, vlastní sestavení,
odmítnutí neplatných cest a manifestů, datumové číslování a přítomnost
obsahu submodulů v exportu. Překlady kontroluje `tests/check_translations.py`.

Před předáním balíku ověřte spuštění přes kořenový spouštěč bez vývojového
Qt/OCCT v PATH a sestavení z přibalených zdrojů. Běh na počítači vývojáře
nenahrazuje ověření v čisté instalaci Windows. Zdrojový export a první
balík se testují místně; změny GitHub workflow vyžadují následný běh v CI.

## Výsledný archiv

Dohodnutý výsledný název je `ZIMA-CAD-Parts-YYYYMMDDNN.zip`, bez platformy.
Místní balík leží přímo v `.dist-output/ZIMA-CAD-Parts/`. Po doplnění
Linuxového sestavení je nutné zachovat Windows část launcher.ini a
přegenerovat checksums.json. Automatické spojení dvou platformních výstupů
a přebalení zatím skripty neprovádějí.
