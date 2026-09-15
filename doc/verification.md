# Ověření sestavy

## Windows — ověřeno 15. září 2026

Aplikace byla sestavena z kompletního projektu pomocí Qt 6.10.1 MSVC x64
s Open CASCADE 8.0.0. Finální integrační sada dokončila **22 testů bez chyby**.
Kontrola všech čtyř překladových katalogů ověřila **412 úplných textů** v každém
jazyce, včetně parametrů, nových dialogů a množných tvarů.

Nasazený `ZIMA-CAD-Parts.exe` v kořeni vytvořil reagující hlavní okno a korektně
se ukončil s návratovým kódem 0. Vzhled zelených záložek a SVG ikon byl
zkontrolován v běžícím programu na Windows. Běh na Linuxu/KDE v této změně
nebyl samostatně ověřen.

## Co ověřují integrační testy

- Přepínání jazyků, SVG ikony a import STEP/IGES/STL.
- Lazy načítání záložek, náhledy, filtry a neblokující úvodní obrazovku.
- Zachování ostatních větví stromu při smazání a práci se zobrazenou složkou.
- Sdílené parametry PDF, Pro/E a ZIMA-CAD, editaci v tabulce i dialogu,
  zachování při Refresh a čtení starších záznamů metadat.
- Výběr nejvyšší číselné verze Pro/E pro načítání parametrů.
- Uložení a odemčení lokálního zámečku, zákaz smazání a přesunu chráněných
  souborů či celé nadřazené složky a povolené kopírování z knihovny.
- Nezávislé podadresáře, blokování přepsání chráněného souboru a reakci
  tlačítek na výběr, zamčení a odemčení.
- Rekurzivní hromadné zamčení i odemčení, zachování jiných metadat,
  vynechání `0000-index`, hlášení chyb a zastavení operace.
- Okamžitý účinek tlačítka Aplikovat na podadresáře i při pozdějším zrušení
  okna Vlastností. Nové podadresáře zámeček automaticky nepřebírají.

## Opakování kontrol

Překlady:

```sh
python tests/check_translations.py
```

Sestavujte mimo adresář zdrojů. Pro integrační testy nejprve dokončete
release aplikace, poté spusťte qmake nad `tests/parts-integration.pro`.
Předejte `PARTS_OBJECTS_DIR` s absolutní cestou k release objektům aplikace,
stejné `OCCT_ROOT` a `CONFIG+=release`. Použijte stejný Qt kit a kompilátor;
na Windows MSVC sestavte pomocí `nmake release`.

Testovací EXE potřebuje stejné běhové knihovny jako aplikace a Qt Test.
Na Windows lze dočasně umístit testovací EXE vedle nasazené aplikace.
Pro testy bez obrazovky nastavte `QT_QPA_PLATFORM=offscreen` a
`QT_QPA_PLATFORM_PLUGIN_PATH` na `plugins/platforms` použitého Qt kitu.
Standardní distribuční složka nemusí obsahovat plugin `qoffscreen.dll`.
Po dokončení odstraňte pouze dočasný testovací EXE.

Samostatná sada `tests/parts-performance.pro` nevyžaduje WebEngine;
postup je uveden v [dokumentaci filtrů](filters.md).
