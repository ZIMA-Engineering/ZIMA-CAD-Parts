"""Real CLI coverage for integrated tools; all files are temporary fixtures."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

CLI = os.environ.get('PARTS_CLI_EXE')
STEP = b"ISO-10303-21;\nHEADER;\nFILE_DESCRIPTION(('sample'),'2;1');\nFILE_NAME('part,name','2026-09-15',('O''Brien','Second'),('Org'),'prep','sys','');\nENDSEC;\nDATA;\n#1=PRODUCT('unchanged','DATA', '',());\nENDSEC;\nEND-ISO-10303-21;\n"

@unittest.skipUnless(CLI, 'Set PARTS_CLI_EXE')
class ToolTests(unittest.TestCase):
    def invoke(self, *args, code=0):
        result = subprocess.run([CLI, *map(str, args)], capture_output=True, timeout=30)
        self.assertEqual(result.returncode, code, result.stderr.decode('utf-8', 'replace'))
        return json.loads((result.stderr if code else result.stdout).decode('utf-8'))

    def test_cleaner_numeric_groups_and_preview_no_writes(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)
            for name in ('bolt.v2.prt.1','bolt.v2.prt.9','bolt.v2.prt.10','bolt.v2.pdf','other.asm.2','trail.txt.1'):
                (p/name).write_text(name)
            before = {f.name: f.read_bytes() for f in p.iterdir()}
            result = self.invoke('ptc-clean', p)
            names = {Path(i['path']).name for i in result['items']}
            self.assertEqual(names, {'bolt.v2.prt.1','bolt.v2.prt.9'})
            self.assertTrue(all(Path(i['keep']).name == 'bolt.v2.prt.10' for i in result['items']))
            masks = self.invoke('ptc-clean',p,'--patterns-only','--mask','trail.txt.*')
            self.assertEqual(len(masks['items']),1)
            self.assertEqual({f.name:f.read_bytes() for f in p.iterdir()}, before)

    def test_local_lock_recursion_and_system_exclusion(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp); (p/'child').mkdir(); (p/'0000-index').mkdir()
            (p/'0000-index/metadata.ini').write_text('[Directory]\nPreventRemoval=true\n')
            for d in (p, p/'child', p/'0000-index'):
                (d/'part.prt.1').write_text('old'); (d/'part.prt.2').write_text('new')
            result = self.invoke('ptc-clean',p,'--recursive')
            self.assertEqual([Path(i['path']).parent for i in result['items']], [p/'child'])
            self.assertEqual(len(result['skipped']),1)
            self.invoke('ptc-clean',p/'0000-index',code=3)

    def test_step_edit_changes_only_header_with_exact_backup(self):
        with tempfile.TemporaryDirectory(prefix='Parts české ') as tmp:
            p = Path(tmp)/'model.step'; p.write_bytes(STEP)
            preview = self.invoke('step-edit',p,'--set',"author=Žluťoučký O'Brien",'--set','organization=ZIMA, s.r.o.')
            self.assertEqual(preview['items'][0]['fields']['author'], ["O'Brien", 'Second'])
            self.assertEqual(p.read_bytes(), STEP)
            result = self.invoke('step-edit',p,'--set',"author=Žluťoučký O'Brien",'--set','organization=ZIMA, s.r.o.','--apply')
            self.assertEqual(Path(result['completed'][0]['backup']).read_bytes(),STEP)
            self.assertEqual(p.read_bytes().split(b'ENDSEC;',1)[1],STEP.split(b'ENDSEC;',1)[1])
            fields = self.invoke('step-edit',p)['items'][0]['fields']
            self.assertEqual(fields['author'],["Žluťoučký O'Brien"])
            self.assertEqual(fields['name'],'part,name')
            self.assertEqual(fields['organization'],['ZIMA, s.r.o.'])

    def test_multiline_header_comments_and_empty_field(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)/'model.stp'
            p.write_bytes(STEP.replace(b"FILE_NAME('", b"/* FILE_NAME('fake'); */\n FILE_NAME(\n'"))
            self.invoke('step-edit',p,'--set','authorization=','--apply')
            self.assertEqual(self.invoke('step-edit',p)['items'][0]['fields']['authorization'],'')

    def test_invalid_step_and_options_leave_original(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)/'broken.step'; p.write_text('DATA; no header')
            result = self.invoke('step-edit',p,'--set','author=Name','--apply',code=3)
            self.assertEqual(len(result['skipped']),1)
            self.assertEqual(p.read_text(),'DATA; no header')
            self.invoke('step-edit',p,'--set','unsupported=x',code=3)
            self.invoke('list',tmp,'--apply',code=2)
            self.invoke('ps2pdf',tmp,'--mask','*',code=2)

    def test_pdf_conversion_and_existing_destination(self):
        with tempfile.TemporaryDirectory(prefix='PDF české ') as tmp:
            p = Path(tmp)/'výkres.ps'
            p.write_bytes(b'%!PS-Adobe-3.0\n/Helvetica findfont 20 scalefont setfont\n72 700 moveto (Parts conversion test) show\nshowpage\n')
            result = self.invoke('ps2pdf',p)
            self.assertFalse(p.with_suffix('.pdf').exists())
            self.assertEqual(len(result['items']),1)
            self.invoke('ps2pdf',p,'--apply')
            pdf = p.with_suffix('.pdf'); original = pdf.read_bytes()
            self.assertTrue(original.startswith(b'%PDF-'))
            result = self.invoke('ps2pdf',p,'--apply',code=3)
            self.assertEqual(len(result['skipped']),1)
            self.assertEqual(pdf.read_bytes(),original)

    def test_cleaner_apply_moves_only_old_fixture_to_trash(self):
        with tempfile.TemporaryDirectory(prefix='Parts cleaner test ') as tmp:
            p = Path(tmp)
            old = p/'parts-cleaner-test.prt.1'; latest = p/'parts-cleaner-test.prt.10'
            old.write_text('old test fixture'); latest.write_text('latest test fixture')
            result = self.invoke('ptc-clean',p,'--apply')
            self.assertEqual(len(result['completed']),1)
            self.assertFalse(old.exists())
            self.assertEqual(latest.read_text(),'latest test fixture')

    def test_failed_conversion_leaves_no_pdf(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'bad.ps'
            p.write_bytes(b'%!PS\nThisIsNotAPostScriptOperator\n')
            result=self.invoke('ps2pdf',p,'--apply',code=3)
            self.assertEqual(len(result['failed']),1)
            self.assertFalse(p.with_suffix('.pdf').exists())
            self.assertEqual(list(Path(tmp).iterdir()),[p])

    def test_pdf_collision_and_non_postscript_plt(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)
            (p/'one.ps').write_bytes(b'%!PS\nshowpage\n')
            (p/'one.eps').write_bytes(b'%!PS\nshowpage\n')
            (p/'plot.plt').write_bytes(b'IN;SP1;PU0,0;')
            result=self.invoke('ps2pdf',p)
            self.assertEqual(result['items'],[])
            self.assertEqual(len(result['skipped']),3)

if __name__ == '__main__':
    unittest.main()
