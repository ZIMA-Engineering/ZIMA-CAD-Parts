#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
version=
custom=false
check=false
while [ "$#" -gt 0 ]; do
    case "$1" in
        -Version) [ "$#" -ge 2 ] || exit 1; version=$2; shift 2;;
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
    actual=$(sed -n 's/^version=//p' "$directory/build.ini" | tr -d '\r')
    target=$(sed -n 's/^platform=//p' "$directory/build.ini" | tr -d '\r')
    [ "$actual" = "$version" ] && [ "$target" = debian-13-x86_64 ] || { echo 'Build manifest mismatch.' >&2; exit 1; }
fi
launcher="$directory/ZIMA-CAD-Parts"
[ -x "$launcher" ] || { echo "Debian build not installed: $version" >&2; exit 1; }
if [ "$check" = true ]; then printf '%s\n' "$launcher"; exit 0; fi
exec "$launcher"
