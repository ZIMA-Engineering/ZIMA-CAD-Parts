#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
version=
custom=false
check=false
explicit=false
while [ "$#" -gt 0 ]; do
    case "$1" in
        -Version) [ "$#" -ge 2 ] || exit 1; version=$2; explicit=true; shift 2;;
        -Custom) custom=true; shift;;
        -Check) check=true; shift;;
        *) echo 'Usage: ZIMA-CAD-Parts.sh [-Version NAME] [-Custom] [-Check]' >&2; exit 1;;
    esac
done
if [ -z "$version" ]; then
    version=$(sed -n 's/^linux=//p' "$root/launcher.ini" | tr -d '\r')
    custom=$(sed -n 's/^linux_custom=//p' "$root/launcher.ini" | tr -d '\r')
fi
if [ "$custom" = true ]; then
    case "$version" in
        ''|[!a-zA-Z0-9]*|*[!a-zA-Z0-9._-]*|*.) echo 'Invalid custom build name.' >&2; exit 1;;
    esac
    directory="$root/custom/linux/$version"
else
    case "$version" in
        ''|*[!0-9]*) echo 'No Debian build selected.' >&2; exit 1;;
    esac
    [ "${#version}" -eq 10 ] || exit 1
    directory="$root/linux/$version"
    if [ "$check" != true ]; then
        if [ "$custom" != true ] && [ "$explicit" != true ] && [ -f "$root/installation.json" ] &&
            { [ -f "$root/.updates/engine.ini" ] || [ -f "$root/release-info/debian-13-x86_64-$version.json" ] || [ -f "$root/.updates/installed/debian-13-x86_64-$version.json" ]; }; then
            engine=
            if [ -f "$root/.updates/engine.ini" ]; then
                engine=$(sed -n 's/^linux=//p' "$root/.updates/engine.ini" | tr -d '\r')
            fi
            [ -n "$engine" ] || engine=$version
            case "$engine" in ''|*[!0-9]*) echo 'Invalid updater engine version.' >&2; exit 1;; esac
            [ "${#engine}" -eq 10 ] || exit 1
            unset QT_PLUGIN_PATH QT_QPA_PLATFORM_PLUGIN_PATH QML_IMPORT_PATH QML2_IMPORT_PATH
            export LD_LIBRARY_PATH="$root/linux/$engine/lib"
            exec "$root/linux/$engine/bin/ZIMA-CAD-Parts-update" launch --root "$root"
        fi
    fi
    actual=$(sed -n 's/^version=//p' "$directory/build.ini" | tr -d '\r')
    target=$(sed -n 's/^platform=//p' "$directory/build.ini" | tr -d '\r')
    [ "$actual" = "$version" ] && [ "$target" = debian-13-x86_64 ] || { echo 'Build manifest mismatch.' >&2; exit 1; }
fi
launcher="$directory/ZIMA-CAD-Parts"
[ -x "$launcher" ] || { echo "Debian build not installed: $version" >&2; exit 1; }
if [ "$check" = true ]; then printf '%s\n' "$launcher"; exit 0; fi

exec "$launcher"
