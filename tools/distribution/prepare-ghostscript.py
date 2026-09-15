"""Download a pinned Ghostscript runtime and matching source without installing it.

Windows: python tools/distribution/prepare-ghostscript.py --sevenzip C:/path/7z.exe
Requires a trusted, already installed 7-Zip CLI. Output must not exist.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
VERSION = '10.08.0'
BASE = 'https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs10080/'
ASSETS = {
    'gs10080w64.exe': '52a91b8bf09298788d7a57b9206127026c23eacd75405f0a131e26dc381dce50',
    'ghostscript-10.08.0.tar.xz': 'c20492bc8ebb96c87fa2e52a0926e1cda8cde95d66145e018ac713fed5da38cf',
}

def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--sevenzip', required=True, type=Path)
    parser.add_argument('--output', type=Path, default=ROOT / 'tools/ghostscript')
    args = parser.parse_args()
    target = args.output.resolve()
    if target.exists():
        raise RuntimeError('Output must not exist')
    with tempfile.TemporaryDirectory(prefix='parts-ghostscript-') as tmp:
        tmp = Path(tmp)
        for name, expected in ASSETS.items():
            urllib.request.urlretrieve(BASE + name, tmp / name)
            if hashlib.sha256((tmp / name).read_bytes()).hexdigest() != expected:
                raise RuntimeError('Checksum mismatch: ' + name)
        extracted = tmp / 'extracted'
        subprocess.run([str(args.sevenzip.resolve()), 'x', str(tmp / 'gs10080w64.exe'), '-o' + str(extracted), '-y'], check=True)
        if not (extracted / 'bin/gswin64c.exe').is_file() or not (extracted / 'doc/COPYING').is_file():
            raise RuntimeError('Unexpected Ghostscript archive layout')
        target.mkdir(parents=True)
        for name in ('bin', 'lib', 'Resource', 'iccprofiles', 'doc'):
            shutil.copytree(extracted / name, target / name)
        (target / 'source').mkdir()
        shutil.copy2(tmp / 'ghostscript-10.08.0.tar.xz', target / 'source')
    manifest = dict(version=VERSION, url=BASE, installer_sha256=ASSETS['gs10080w64.exe'],
                    source_sha256=ASSETS['ghostscript-10.08.0.tar.xz'])
    manifest['files'] = {p.relative_to(target).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                         for p in sorted(target.rglob('*')) if p.is_file()}
    (target / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(target)

if __name__ == '__main__':
    main()
