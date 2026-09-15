"""Real, read-only CLI process tests. Set PARTS_CLI_EXE to the built executable."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

CLI = os.environ.get('PARTS_CLI_EXE')

@unittest.skipUnless(CLI, 'Set PARTS_CLI_EXE')
class CliTests(unittest.TestCase):
    def invoke(self, *args, code=0):
        env = dict(os.environ, QT_QPA_PLATFORM='intentionally-nonexistent')
        env.pop('QT_PLUGIN_PATH', None)
        env.pop('QT_QPA_PLATFORM_PLUGIN_PATH', None)
        if os.name == 'nt':
            env['PATH'] = os.environ['SystemRoot'] + '/System32'
        result = subprocess.run([CLI, *map(str, args)], capture_output=True, env=env, timeout=10)
        self.assertEqual(result.returncode, code, result.stderr.decode('utf-8', 'replace'))
        return json.loads((result.stdout if code == 0 else result.stderr).decode('utf-8'))

    def write(self, root, content, filename='metadata.ini'):
        index = root / '0000-index'
        index.mkdir(exist_ok=True)
        (index / filename).write_text(content, encoding='utf-8')

    def snapshot(self, root):
        return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in root.rglob('*') if p.is_file()}

    def test_filters_versions_shared_unicode_and_no_writes(self):
        with tempfile.TemporaryDirectory(prefix='Parts české ') as tmp:
            root = Path(tmp)
            for name in ('xxx.pdf','xxx.prt.9','xxx.prt.10','xxx.prtz','xxx.prtz.2','hidden.log','.directory','poznámka.txt'):
                (root / name).touch()
            self.write(root, '[Directory]\nVersion=2\nParameters=description\n[Parts]\nxxx\\description\\cs=Společný díl\n')
            self.write(root, '[Filters]\nShowVersions=false\nShowZimaVersions=false\nHide=*.log\n', 'filters.ini')
            before = self.snapshot(root)
            result = self.invoke('list', root, '--language', 'cs')
            names = {p['name'] for p in result['parts']}
            self.assertEqual(names, {'xxx.pdf','xxx.prt.10','xxx.prtz','poznámka.txt'})
            for part in result['parts']:
                if part['baseName'] == 'xxx':
                    self.assertEqual(part['parameters']['description'], 'Společný díl')
            self.assertEqual(self.invoke('params', root / 'xxx.prt.9','--language','cs')['part']['parameters']['description'], 'Společný díl')
            self.assertEqual(len(self.invoke('list',root,'--name','PRT.10')['parts']),1)
            self.assertEqual(self.snapshot(root), before)

    def test_legacy_conversion_and_explicit_empty(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'xxx.pdf').touch()
            self.write(root, '[params]\nen\\1=Description\n[xxx]\n1=Legacy\n')
            before = self.snapshot(root)
            self.assertEqual(self.invoke('params', root/'xxx.pdf')['part']['parameters']['param01'],'Legacy')
            self.assertEqual(self.snapshot(root),before)
            self.write(root, '[Directory]\nVersion=2\nParameters=description\n[Parts]\nxxx\\description\\en=\nxxx.pdf\\description\\en=Old\n')
            self.assertEqual(self.invoke('params',root/'xxx.pdf','--language','en')['part']['parameters']['description'],'')

    def test_includes_and_cycles(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            library = root/'library'; library.mkdir()
            (root/'xxx.pdf').touch()
            self.write(root, '[Directory]\nVersion=2\nIncludeParameters=library\n')
            self.write(library, '[Directory]\nVersion=2\nParameters=description\n[Parts]\nxxx\\description\\en=Included\n')
            self.assertEqual(self.invoke('params',root/'xxx.pdf')['part']['parameters']['description'],'Included')
            self.write(library, '[Directory]\nVersion=2\nIncludeParameters=..\n')
            before = self.snapshot(root)
            self.assertIn('Cyclic', self.invoke('list',root,code=3)['error'])
            self.assertEqual(self.snapshot(root), before)

    def test_no_metadata_created_and_filters_do_not_inherit(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.write(root,'[Filters]\nHide=*.pdf\n','filters.ini')
            child = root/'child'; child.mkdir(); (child/'a.pdf').touch()
            self.assertEqual(len(self.invoke('list',child)['parts']),1)
            self.assertFalse((child/'0000-index').exists())
            self.invoke('list',root/'missing',code=3)
            self.invoke('bad-command',root,code=2)
            self.invoke('list',root,'--default-proe-versions','bad',code=2)

if __name__ == '__main__':
    unittest.main()
