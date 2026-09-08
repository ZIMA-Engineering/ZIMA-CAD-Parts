# Jazyky rozhraní

Parts podporuje češtinu (cs_CZ), němčinu (de_DE), angličtinu (en_US),
francouzštinu (fr_FR) a ruštinu (ru_RU). V horní liště jsou vlajky v pořadí
angličtina, čeština, němčina, francouzština, ruština. Ruská vlajka je vpravo.

Ruština je nyní přidaná do Parts; její doplnění do ZIMA-CAD je samostatný
budoucí krok. Zdrojové soubory ani nastavení ZIMA-CAD tato změna neupravuje.

Vlajka přepne rozhraní i jazyk metadat. Stejnou volbu nabízí Nastavení.
Změna se uloží a projeví bez restartu. Automatická detekce používá jazyk
systému; regionální varianty se sjednotí (například fr_CA na fr_FR).
Nepodporovaný jazyk přejde na angličtinu. Regionální varianta ru_BY se
sjednotí na ru_RU; dříve uložená ruština je znovu plně podporovaná.

Přeloženy jsou texty aplikace, dialogy, chyby, vyhledávání, nastavení,
operace se soubory a vestavěná stránka O programu. Uživatelské názvy
adresářů, metadata, skripty a externí webové stránky program nepřekládá.
Metadata používají vlastní jazykové hodnoty z projektových souborů.

## Údržba překladů

Angličtina je jazyk zdrojových textů. Úplné katalogy jsou v locale:
zima-cad-parts_cs_CZ.ts, zima-cad-parts_de_DE.ts, zima-cad-parts_fr_FR.ts
a zima-cad-parts_ru_RU.ts.
Po změně textů spusťte lupdate zima-cad-parts.pro -no-obsolete,
doplňte překlady a spusťte python tests/check_translations.py.

qmake pomocí CONFIG += lrelease embed_translations kompiluje katalogy
a vkládá je do prostředků aplikace pod /i18n. Samostatné aplikační
soubory QM tedy nejsou pro běh potřebné. Standardní dialogy Qt používají
sloučené katalogy qt_cs.qm, qt_de.qm, qt_fr.qm a qt_ru.qm ze složky translations,
které na Windows připraví windeployqt. Na Linuxu se mohou načíst také
systémové katalogy qtbase_cs.qm, qtbase_de.qm, qtbase_fr.qm a qtbase_ru.qm.

Integrační test ověřuje všech pět vlajek, změnu skutečných textů
hlavní lišty, francouzský a ruský dialog filtrů, regionální variantu,
nepodporovaný jazyk, uložení volby a opakovaný návrat do angličtiny.

Aktuální katalogy obsahují 402 textů včetně správy hesel, automatických indexů
a CAD náhledů. Kontrola zahrnuje také české, německé, francouzské a ruské množné tvary.
Ruština se za běhu ověřuje pro počty 1, 2, 5 a 21.
