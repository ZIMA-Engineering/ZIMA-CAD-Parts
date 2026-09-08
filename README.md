# ZIMA-CAD-Parts

Prohlížeč CAD projektů pro Windows a Linux. Zobrazuje všechny soubory,
lokální výjimky řídí filters.ini. Náhledy se doplňují průběžně.

- [Filtry, ZIMA-CAD, náhledy a práce se stromem](doc/filters.md)
- [Jazyky a údržba překladů](doc/localization.md)

## Připravená Windows sestava

Spusťte ZIMA-CAD-Parts.exe přímo z kořene projektu. Vedle programu musí
zůstat přiložené DLL a složky Qt (zejména platforms, resources,
translations a QtWebEngineProcess.exe). Překlady aplikace jsou vložené
do EXE. Binární soubory nejsou verzované v Gitu.

## Sestavení

Vyžaduje Qt 6.4 nebo novější s moduly Core, Gui, Widgets, Network,
OpenGL, Core5Compat a WebEngineWidgets, qmake a nástroje Linguist.
Zdrojové závislosti se načítají pomocí git submodule update --init --recursive.

Na Windows použijte Qt MSVC a odpovídající vývojářský příkazový řádek
Visual Studia. Aktuální sestava byla ověřena s Qt 6.10.1 MSVC x64.
Qt WebEngine vyžaduje MSVC kit.

    mkdir .build-release
    cd .build-release
    qmake ../zima-cad-parts.pro -spec win32-msvc CONFIG+=release CONFIG-=debug
    nmake
    windeployqt --release release/ZIMA-CAD-Parts.exe

Pro nasazení do kořene zkopírujte výsledné EXE do kořene a spusťte
windeployqt --release nad touto cílovou cestou, aby knihovny skončily
vedle programu. Při aktualizaci musí být cílová instance zavřená.

Na Linuxu s KDE použijte Qt z distribuce včetně WebEngine, Qt5Compat
a nástrojů Linguist:

    mkdir .build-release
    cd .build-release
    qmake ../zima-cad-parts.pro CONFIG+=release
    make -j4

Některé distribuce nazývají příkaz qmake6. Qt standardní ikony a dialogy
používají integraci systému; vlastní CAD ikony jsou součástí aplikace.

## Ověření

Samostatné testy filtrů a náhledů sestavíte z tests/parts-performance.pro.
Integrační testy z tests/parts-integration.pro potřebují proměnnou
PARTS_OBJECTS_DIR s absolutní cestou k objektům sestavené aplikace
(na Windows typicky .build-release/release). Použijte stejný Qt kit.

Katalogy ověřuje python tests/check_translations.py. Pro testy bez
zobrazení oken nastavte QT_QPA_PLATFORM=offscreen.
Linuxový běh této aktualizace nebyl na Windows pracovním počítači ověřen.
