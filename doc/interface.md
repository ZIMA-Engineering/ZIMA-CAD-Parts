# Vzhled a ovládání

Aktivní hlavní záložka má zelené pozadí ve stylu ZIMA-CAD (`#4DD811`)
a tmavý text. Stejné zvýraznění používají záložky **Adresář** a **Díly**.
Neaktivní záložky respektují systémovou paletu; při přejetí myší mají
světle zelené zvýraznění. Hlavní záložky při nedostatku místa používají
posouvací tlačítka.

**Adresář** a **Díly** jsou širší a před názvem mají ikonu složky nebo dílu.
Tlačítka **Filtry**, **Obnovit**, **Smazat**, **Přesunout** a **Kopírovat**
používají společnou sadu SVG ikon. Ikony a styl jsou součástí prostředků
aplikace v `gfx/navigation`; nevyžadují další instalaci tématu ikon.

Původní položka **Upravit** v kontextovém menu adresáře se nyní jmenuje
**Vlastnosti adresáře**. Odpovídající položka u datového zdroje je
**Vlastnosti datového zdroje**.

Ve Vlastnostech adresáře je zámeček **Chránit před smazáním a přesunutím**.
Napravo je **Aplikovat na podadresáře**. Toto tlačítko působí okamžitě,
nezávisle na následném potvrzení či zrušení okna. Podrobnosti, omezení
lokálního zámečku a jeho uložení popisuje [dokumentace adresářů a filtrů](filters.md).

Zamčené soubory mají **Smazat** a **Přesunout** zašedlé a neaktivní.
Bublinová nápověda ukazuje důvod. Stejná ochrana vypíná mazání v kontextovém
menu stromu. Stav reaguje na změnu adresáře, výběru a zámečku; při hromadném
výběru se zohledňují i vybrané soubory z jiných adresářů. Kopírování zůstává
povolené a zamčené podadresáře neblokují práci se soubory v nezamčených sousedech.

Volba **Zobrazit archivní verze ZIMA-CAD** má stejný smysl jako volba verzí
Pro/E: zaškrtnuto znamená zobrazit. Dříve uložené hodnoty `ShowZimaVersions`
zůstávají platné a nevyžadují přepis nastavení.
