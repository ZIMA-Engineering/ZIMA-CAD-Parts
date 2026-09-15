# Debian distribuce – připraveno k ověření

Cílové prostředí je Debian 13 (trixie), amd64. Aktuální stabilní řadu uvádí
[Debian Releases](https://www.debian.org/releases/). Qt Wayland a obrazové
pluginy dodává při sestavení správce balíčků. Podporované balíčky jsou popsány
v [qt6-wayland](https://packages.debian.org/trixie/qt6-wayland),
[qt6-svg-plugins](https://packages.debian.org/trixie/qt6-svg-plugins)
a [qt6-image-formats-plugins](https://packages.debian.org/trixie/qt6-image-formats-plugins).

**Stav:** Windows pracoviště nemá WSL ani Docker. Skript ani sestavení
nebyly spuštěny na Debianu. Není zatím k dispozici ověřená Linux binárka.
Nový CI workflow je omezen na Debian 13; ověří sestavení a připraví
experimentální archiv, ale sám nenahrazuje testy KDE/Wayland.

## Sestavení

Z inicializovaného Git checkoutu s obsahem submodulů:

```sh
docker build --pull --build-arg BASE_IMAGE=debian:13 --build-arg DISTRO=debian -f .github/ci/linux-build.Dockerfile -t parts-debian-build .
docker run --rm --user "$(id -u):$(id -g)" --volume "$PWD:/workspace" --workdir /workspace --env HOME=/tmp parts-debian-build bash .github/ci/build-debian.sh
```

Výstup je v `.dist-output/debian/ZIMA-CAD-Parts/` a v odpovídajícím tar.gz.
Skript `package-debian.py` kontroluje Debian 13, architekturu a číslo EXE,
exportuje sledované zdroje včetně submodulů, přibaluje Qt, WebEngine,
pluginy Wayland/X11, OCCT a rekurzivně dohledané knihovny. Přenosné cesty
ELF souborů nastavuje přes patchelf. Existující výstup nepřepisuje.

Glibc, grafické rozhraní a ovladače zůstávají ze systému. Manifest uvádí
konkrétní hostitelské knihovny. Sandbox WebEngine se nevypíná; aplikace
se má spouštět jako běžný uživatel. Dostupnost knihovny podle ldd sama
neprokazuje úplnost dynamicky načítaných pluginů nebo témat KDE.

## Spuštění

```sh
./ZIMA-CAD-Parts.sh
./ZIMA-CAD-Parts.sh -Version 2026091501
./ZIMA-CAD-Parts.sh -Custom -Version moje-sestaveni
./ZIMA-CAD-Parts.sh -Check
```

`launcher.ini` používá klíče `linux` a `linux_custom`. Každé oficiální
adresářové sestavení má `build.ini` s verzí a platformou debian-13-x86_64.
Manifest není kryptografický podpis. Vlastní sestavení leží v custom/linux.
Verzovaný spouštěč nastaví cesty knihoven a prostředků pouze svému procesu.

Před veřejným vydáním je nutné ověřit balík na čistém Debianu s KDE/Wayland:
spuštění, webové stránky/PDF, CAD import a vykreslení, systémové ikony,
hesla přes systémovou službu, přepínání verzí a kopírování celého balíku
na jinou cestu. Je také nutné dokončit distribuční licenční soupis všech
přibalených systémových balíků a podpisy. Výsledky zatím nejsou potvrzené.
