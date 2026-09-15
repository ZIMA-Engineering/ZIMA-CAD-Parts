#!/bin/bash
set -euo pipefail
# Called inside the Debian build container as a non-root user.
git config --global --add safe.directory /workspace
mkdir -p .build-debian
cd .build-debian
qmake6 "OCCT_REQUIRED=1" ../zima-cad-parts.pro
make -j"$(nproc)"
cd ..
python3 tests/check_translations.py
python3 tests/test_distribution.py
python3 tools/distribution/package-debian.py --exe .build-debian/ZIMA-CAD-Parts --output .dist-output/debian
package="$PWD/.dist-output/debian/ZIMA-CAD-Parts"
"$package/ZIMA-CAD-Parts.sh" -Check
runtime=$(find "$package/linux" -mindepth 1 -maxdepth 1 -type d)
"$runtime/ZIMA-CAD-Parts" --build-info
version=$(sed -n 's/^version=//p' "$runtime/build.ini")
tar -C .dist-output/debian -czf ".dist-output/ZIMA-CAD-Parts-$version-debian-13-x86_64.tar.gz" ZIMA-CAD-Parts
