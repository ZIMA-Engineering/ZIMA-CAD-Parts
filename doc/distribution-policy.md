# Závazný návrh distribuce a verzování

Schváleno vlastníkem projektu 15. září 2026. Tento dokument je závazným
zadáním pro budoucí implementaci, nikoli popisem již hotového aktualizátoru.
Později se stejná pravidla použijí pro ZIMA-CAD a další programy
ZIMA-Engineering. Jejich repozitáře se tímto zápisem nemění.

## Identita vydání

- Vydání, distribuční balíky a Git tagy se označují například
  `ZIMA-CAD-Parts-2026091501`.
- Interní číslo má formát `YYYYMMDDNN`: datum sestavení/vydání a dvoumístné
  pořadí vydání v daném dni. Stejné číslo označuje zdroje i odpovídající
  oficiální sestavení pro jednotlivé platformy.
- „ZIMA-CAD-Parts 9“ je pouze marketingový název v sekci O programu.
  Devítka se nepoužívá pro technické verzování, názvy balíků, tagy ani
  rozhodování o aktualizacích. O programu uvádí také skutečné číslo sestavení.
- Oficiální uvedení je plánováno na narozeniny vlastníka; konkrétní datum
  zatím nebylo stanoveno v tomto zadání.

## Distribuční struktura

Výsledkem je jeden společný archiv `ZIMA-CAD-Parts-YYYYMMDDNN.zip`, bez
platformy v názvu. Obsahuje složku ZIMA-CAD-Parts se zdroji a sestaveními
obou platforem. Linuxové sestavení lze doplnit později a stejnou složku
znovu zabalit. Při spojování se zachová nastavení již přítomné platformy;
oba běhové balíky musí odpovídat příslušné verzi zdrojů. Kontrolní součty
se po doplnění souborů musí znovu vytvořit.

Místní `.dist-output/` obsahuje přímo `ZIMA-CAD-Parts/` a výsledný ZIP,
bez trvalé mezisložky windows-native-final. Platformní výstupy CI jsou
mezivýsledky pro sestavení společného vydání, nikoli odlišné veřejné edice.

Stálá hlavní složka obsahuje dva spouštěče. Verze jsou uvnitř podsložek
jednotlivých platforem, nikoli v názvu této stálé instalační složky.

```text
ZIMA-CAD-Parts/
    ZIMA-CAD-Parts.exe
    ZIMA-CAD-Parts.sh
    windows/
        2026091501/
        2026091601/
    linux/
        2026091501/
        2026091601/
    source/
        2026091501/
        2026091601/
    custom/
        windows/
        linux/
    launcher.ini
```

Spouštěče vybírají nastavenou verzi pro svůj systém. Přesné schéma
`launcher.ini` se určí při implementaci. Každá oficiální verze obsahuje
vlastní potřebné běhové knihovny, pluginy a prostředky; nesmějí se míchat
knihovny různých verzí nebo platforem.

Zdrojová složka obsahuje kompletní projekt, prostředky, překlady, předpisy
sestavení a skutečný obsah submodulů v odpovídajících revizích. Musí umožnit
vlastní sestavení po dodání vývojových závislostí. Zdrojové archivy nesmějí
obsahovat pracovní zálohy, osobní data nebo dočasné sestavy.

## Platformy

- Windows: oficiální připravené sestavení s potřebnými závislostmi.
- Linux: nejprve pouze nejnovější stabilní Debian, x86-64, s ověřením
  KDE/Wayland. Každé vydání uvede přesnou podporovanou verzi Debianu.
- Linuxový balík přibaluje Qt, WebEngine, OCCT a potřebné přenositelné
  závislosti. Základní systémové knihovny a grafické ovladače dodává Debian.
  Přesný seznam přibalených souborů se stanoví a ověří při implementaci.
- Běžný uživatel rozbalí a spustí; kompilace není podmínkou používání.
- Nový stabilní Debian se začne podporovat po ověření sestavení a běhu.
  Ostatní distribuce zatím nejsou cílem podpory.

## Aktualizace a uchování verzí

Oficiální balíky se zveřejňují u vydání na GitHubu a odpovídají Git tagu.
Aktualizace stahuje hotový balík; binární sestavy se neukládají do běžné
historie zdrojového repozitáře.

Nová verze se připraví do samostatné složky, ověří a aktivuje po ukončení
programu. Běžící instalace se nepřepisuje. Předchozí verze umožňuje návrat.
Pro každou platformu se uchovávají maximálně dvě oficiální verze: aktuální
a předchozí. Dočasně rozpracovaná aktualizace nesmí způsobit předčasné
smazání funkční verze; starší verze se odstraní až po úspěšném ověření nové.
Při úklidu zdrojů je nutné zachovat zdroje všech ponechaných oficiálních
verzí na obou platformách. Uživatelská a projektová data zůstávají oddělená.

## Vlastní sestavení a původ balíků

Vlastní sestavení patří do `custom/`, lze je vybrat ke spuštění a aktualizátor
je nepřepisuje ani automaticky nemaže. Limit dvou verzí se na ně nevztahuje.

Oficiální sestavení ZIMA-Engineering se od vlastních sestavení rozlišují
ve verzi, diagnostice a sekci O programu. Uvádí se číslo vydání, commit
a původ sestavení. Pravost oficiálního balíku se ověřuje podpisem; samotný
text v upravitelných zdrojích není důkazem původu. Technologie podpisu
a správa klíčů budou určeny při implementaci.

Testování a podpora se vztahují na oficiální nezměněné balíky. Vlastní
sestavení nejsou vydavatelem ověřena. Toto pravidlo podpory nezavádí
nová licenční omezení ani právní příslib záruky.
