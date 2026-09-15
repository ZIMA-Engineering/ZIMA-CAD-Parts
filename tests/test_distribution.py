"""Packaging contracts that do not require Qt or a running GUI."""
import importlib.util
import json
import os
import runpy
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('package_windows', ROOT / 'tools/distribution/package-windows.py')
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


class PackagingTests(unittest.TestCase):
    def test_version_rejects_invalid_dates_and_sequence(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'src').mkdir()
            for value in ('2026023001', '2026091500', '9.0'):
                (root / 'src/zima-cad-parts.h').write_text(f'#define VERSION "{value}"\n')
                with self.assertRaises(ValueError):
                    package.version(root)
            (root / 'src/zima-cad-parts.h').write_text('#define VERSION "2026091501"\n')
            self.assertEqual(package.version(root), '2026091501')

    def test_export_includes_submodule_contents(self):
        files = {relative.as_posix() for _, relative in package.source_files()}
        self.assertIn('3rdparty/qtkeychain/qtkeychain.pri', files)
        self.assertTrue(any(x.startswith('libqdxf/src/') for x in files))
        self.assertFalse(any('.git/' in x or '.local-backups/' in x for x in files))

    def test_debian_host_library_boundary(self):
        module = runpy.run_path(str(ROOT / 'tools/distribution/package-debian.py'))
        host = module['SYSTEM']
        for name in ('libc.so.6', 'libm.so.6', 'libGLX.so.0', 'libdrm_amdgpu.so.1'):
            self.assertIsNotNone(host.fullmatch(name), name)
        for name in ('libQt6Core.so.6', 'libstdc++.so.6', 'libcrypto.so.3', 'libTKernel.so.7'):
            self.assertIsNone(host.fullmatch(name), name)

    @unittest.skipUnless(shutil.which('powershell.exe'), 'Windows launcher test')
    def test_launcher_selection_and_invalid_manifests(self):
        with tempfile.TemporaryDirectory(prefix='Parts launcher ') as tmp:
            root = Path(tmp)
            script = root / 'launch-windows.ps1'
            shutil.copy2(ROOT / 'tools/distribution/launch-windows.ps1', script)
            native = os.environ.get('PARTS_LAUNCHER')
            if native:
                shutil.copy2(native, root / 'ZIMA-CAD-Parts.exe')
            for build in ('2026091501', '2026091601'):
                folder = root / 'windows' / build
                folder.mkdir(parents=True)
                (folder / 'ZIMA-CAD-Parts.exe').touch()
                (folder / 'build.ini').write_text(f'[build]\nversion={build}\nplatform=windows-x64\n')
                (folder / 'version.json').write_text(json.dumps(dict(version=build, platform='windows-x64')))
            (root / 'launcher.ini').write_text('[launcher]\nwindows=2026091601\nwindows_custom=false\n')
            def check(*args):
                command = [str(root / 'ZIMA-CAD-Parts.exe')] if native else ['powershell.exe', '-NoProfile', '-File', str(script)]
                return subprocess.run([*command, '-Check', *args], capture_output=True, text=True)
            result = check()
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('2026091601', result.stdout)
            self.assertIn('2026091501', check('-Version', '2026091501').stdout)
            self.assertNotEqual(check('-Version', '../outside').returncode, 0)
            self.assertNotEqual(check('-Custom', '-Version', '../outside').returncode, 0)
            self.assertNotEqual(check('-Version', '2026091701').returncode, 0)
            custom = root / 'custom/windows/my-build'
            custom.mkdir(parents=True)
            (custom / 'ZIMA-CAD-Parts.exe').touch()
            self.assertEqual(check('-Custom', '-Version', 'my-build').returncode, 0)
            (root / 'windows/2026091601/version.json').write_text('{"version":"wrong","platform":"windows-x64"}')
            (root / 'windows/2026091601/build.ini').write_text('[build]\nversion=wrong\nplatform=windows-x64\n')
            self.assertNotEqual(check().returncode, 0)


if __name__ == '__main__':
    unittest.main()

