# Jazyky rozhraní

Parts používá stejnou sadu jazyků jako ZIMA-CAD: češtinu (cs_CZ),
němčinu (de_DE), angličtinu (en_US) a francouzštinu (fr_FR).
V horní liště jsou vlajky v pořadí angličtina, čeština, němčina,
francouzština. Poslední vlajka vpravo tedy patří francouzštině.

Vlajka přepne rozhraní i jazyk metadat. Stejnou volbu nabízí Nastavení.
Změna se uloží a projeví bez restartu. Automatická detekce používá jazyk
systému; regionální varianty se sjednotí (například fr_CA na fr_FR).
Nepodporovaný jazyk, včetně dříve uložené ruštiny, přejde na angličtinu.

Přeloženy jsou texty aplikace, dialogy, chyby, vyhledávání, nastavení,
operace se soubory a vestavěná stránka O programu. Uživatelské názvy
adresářů, metadata, skripty a externí webové stránky program nepřekládá.
Metadata používají vlastní jazykové hodnoty z projektových souborů.

## Údržba překladů

Angličtina je jazyk zdrojových textů. Úplné katalogy jsou v locale:
zima-cad-parts_cs_CZ.ts, zima-cad-parts_de_DE.ts, zima-cad-parts_fr_FR.ts.
Po změně textů spusťte lupdate zima-cad-parts.pro -no-obsolete,
doplňte překlady a spusťte python tests/check_translations.py.

qmake pomocí CONFIG += lrelease embed_translations kompiluje katalogy
a vkládá je do prostředků aplikace pod /i18n. Samostatné aplikační
soubory QM tedy nejsou pro běh potřebné. Standardní dialogy Qt používají
sloučené katalogy qt_cs.qm, qt_de.qm a qt_fr.qm ze složky translations,
které na Windows připraví windeployqt. Na Linuxu se mohou načíst také
systémové katalogy qtbase_cs.qm, qtbase_de.qm a qtbase_fr.qm.

Integrační test ověřuje všechny čtyři vlajky, změnu skutečných textů
hlavní lišty, francouzský dialog filtrů, regionální variantu,
nepodporovaný jazyk, uložení volby a opakovaný návrat do angličtiny.
