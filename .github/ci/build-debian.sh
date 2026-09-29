#!/bin/bash
set -euo pipefail
# Called inside the Debian build container as a non-root user.
git config --global --add safe.directory /workspace
mkdir -p .build-debian
cd .build-debian
qmake6 "OCCT_REQUIRED=1" ../zima-cad-parts.pro
make -j"$(nproc)"
cd ..
mkdir -p .build-cli
(cd .build-cli && qmake6 ../zima-cad-parts-cli.pro && make -j"$(nproc)")
mkdir -p .build-updater
(cd .build-updater && qmake6 ../zima-cad-parts-update.pro && make -j"$(nproc)")
export PARTS_CLI_EXE="$PWD/.build-cli/ZIMA-Parts-cli"
python3 tests/test_cli.py
python3 tests/test_tools.py
python3 tests/check_translations.py
python3 tests/test_distribution.py
python3 tools/distribution/package-debian.py --exe .build-debian/ZIMA-Parts --cli "$PARTS_CLI_EXE" --updater .build-updater/ZIMA-Parts-update --output .dist-output/debian
package="$PWD/.dist-output/debian/ZIMA-Parts"
mkdir -p .build-updater-tests .build-update-fixture
(cd .build-updater-tests && qmake6 ../zima-cad-parts-update.pro CONFIG+=update_tests && make -j"$(nproc)")
(cd .build-update-fixture && qmake6 ../tests/update-fixture.pro && make -j"$(nproc)")
export PARTS_UPDATER_TEST_EXE="$PWD/.build-updater-tests/ZIMA-Parts-update"
export PARTS_UPDATE_FIXTURE_EXE="$PWD/.build-update-fixture/update-fixture"
python3 tests/test_updates.py
"$package/ZIMA-Parts.sh" -Check
runtime=$(find "$package/linux" -mindepth 1 -maxdepth 1 -type d)
"$runtime/ZIMA-Parts" --build-info
version=$(sed -n 's/^version=//p' "$runtime/build.ini")
tar -C .dist-output/debian -czf ".dist-output/ZIMA-Parts-$version-debian-13-x86_64.tar.gz" ZIMA-Parts
