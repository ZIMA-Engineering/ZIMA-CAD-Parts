"""Create a fresh versioned Windows distribution; requires Python, Git and MSVC dumpbin.

Run from the VS developer shell. The executable must be built from this checkout.
No existing package is overwritten. Signing/updating are separate future steps.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def run(*args, cwd=ROOT):
    return subprocess.check_output(args, cwd=cwd, text=True, encoding='utf-8', errors='replace')


def version(root=ROOT):
    match = re.search(r'^#define VERSION "(\d{10})"$', (root / 'src/zima-cad-parts.h').read_text(encoding='utf-8'), re.M)
    if not match:
        raise ValueError('Expected VERSION YYYYMMDDNN in src/zima-cad-parts.h')
    value = match[1]
    datetime.datetime.strptime(value[:8], '%Y%m%d')
    if value[8:] == '00':
        raise ValueError('Release sequence starts at 01')
    return value


def source_files(repository=ROOT, prefix=Path()):
    for record in run('git', 'ls-files', '--stage', '-z', cwd=repository).split('\0'):
        if not record:
            continue
        info, name = record.split('\t', 1)
        mode, revision, stage = info.split()
        if stage != '0':
            raise RuntimeError('Resolve Git conflicts before packaging')
        path = repository / name
        if mode == '160000':
            if not (path / '.git').exists():
                raise RuntimeError(f'Uninitialized submodule: {name}')
            yield from source_files(path, prefix / name)
        else:
            yield path, prefix / name


def copy_runtime(executable, destination, qt, occt):
    shutil.copy2(executable, destination / executable.name)
    subprocess.run([str(qt / 'bin/windeployqt.exe'), '--release', '--force',
                    '--compiler-runtime', str(destination / executable.name)], check=True)
    redist = os.environ.get('VCToolsRedistDir')
    crt_dirs = list((Path(redist) / 'x64').glob('Microsoft.VC*.CRT')) if redist else []
    if len(crt_dirs) != 1:
        raise RuntimeError('Run from the MSVC developer shell with one x64 CRT redistributable')
    for dll in crt_dirs[0].glob('*.dll'):
        shutil.copy2(dll, destination / dll.name)
    # Qt has already deployed its own kit. Resolve remaining native imports from
    # OCCT, recursively, instead of copying unrelated DLLs from vcpkg.
    system = Path(os.environ['SystemRoot']) / 'System32'
    candidates = {p.name.lower(): p for p in (occt / 'bin').glob('*.dll')}
    candidates.update({p.name.lower(): p for p in (qt / 'bin').glob('*.dll')})
    inspected = set()
    while True:
        pending = [p for p in destination.rglob('*') if p.suffix.lower() in ('.dll', '.exe') and p not in inspected]
        if not pending:
            break
        for binary in pending:
            inspected.add(binary)
            imports = re.findall(r'^\s+([\w.+-]+\.dll)\s*$', run('dumpbin', '/nologo', '/dependents', str(binary)), re.M | re.I)
            for name in imports:
                if (destination / name).exists() or (binary.parent / name).exists():
                    continue
                if name.lower().startswith(('api-ms-', 'ext-ms-')):
                    continue
                source = candidates.get(name.lower())
                if source is not None:
                    shutil.copy2(source, destination / source.name)
                elif not (system / name).exists():
                    raise RuntimeError(f'Unresolved dependency {name} required by {binary.name}')
    resources = occt / 'share/opencascade/resources'
    if not resources.is_dir():
        raise RuntimeError('Missing OCCT runtime resources')
    for name in ('Shaders', 'SHMessage', 'XSMessage', 'XSTEPResource', 'StdResource', 'UnitsAPI', 'Textures', 'XmlOcafResource'):
        shutil.copytree(resources / name, destination / 'occt' / name)
    (destination / 'qt.conf').write_text('[Paths]\nPrefix=.\nPlugins=.\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--qt', type=Path, required=True)
    parser.add_argument('--occt', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='New output directory, must not exist')
    parser.add_argument('--include', action='append', default=[], help='Explicit untracked project file to include in a development snapshot')
    parser.add_argument('--release', action='store_true', help='Require a clean tagged source tree; does not sign the result')
    args = parser.parse_args()
    build = version()
    commit = run('git', 'rev-parse', 'HEAD').strip()
    dirty = bool(run('git', 'status', '--porcelain', '--untracked-files=normal').strip())
    if args.release:
        if dirty or args.include:
            raise RuntimeError('Release packaging requires a clean checkout without additional files')
        tag = f'ZIMA-CAD-Parts-{build}'
        if run('git', 'rev-parse', f'{tag}^{{commit}}').strip() != commit:
            raise RuntimeError('Release tag does not match HEAD')
    output = args.output.resolve()
    if output.exists():
        raise RuntimeError(f'Refusing to overwrite output: {output}')
    exe = args.exe.resolve()
    if exe.name != 'ZIMA-CAD-Parts.exe' or not exe.is_file():
        raise RuntimeError('Missing ZIMA-CAD-Parts.exe')
    binary_info = json.loads(run(str(exe), '--build-info'))
    if binary_info.get('version') != build:
        raise RuntimeError('Executable version differs from source VERSION; rebuild first')
    files = dict((relative, source) for source, relative in source_files())
    for name in args.include:
        source = (ROOT / name).resolve()
        relative = source.relative_to(ROOT)
        if not source.is_file() or any(x in ('.git', '.local-backups') for x in relative.parts):
            raise ValueError(f'Invalid extra source file: {name}')
        files[relative] = source
    package = output / 'ZIMA-CAD-Parts'
    runtime = package / 'windows' / build
    sources = package / 'source' / build
    runtime.mkdir(parents=True)
    sources.mkdir(parents=True)
    for relative, source in files.items():
        target = sources / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    copy_runtime(exe, runtime, args.qt.resolve(), args.occt.resolve())
    shutil.copytree(ROOT / 'licenses', runtime / 'licenses')
    shutil.copy2(ROOT / 'LICENSE', runtime / 'licenses/Parts-LICENSE')
    launcher_build = output / 'launcher-build'
    launcher_build.mkdir()
    subprocess.run(['cl', '/nologo', '/std:c++17', '/EHsc', '/O2', '/MT',
                    str(ROOT / 'tools/distribution/launcher-windows.cpp'),
                    '/Fe:' + str(package / 'ZIMA-CAD-Parts.exe'),
                    '/link', '/SUBSYSTEM:WINDOWS', 'shell32.lib', 'user32.lib'],
                   cwd=launcher_build, check=True)
    (runtime / 'build.ini').write_text(f'[build]\nversion={build}\nplatform=windows-x64\n', encoding='utf-8')
    for name in ('ZIMA-CAD-Parts.sh',):
        shutil.copy2(ROOT / 'tools/distribution' / name, package / name)
    for name in ('linux', 'custom/windows', 'custom/linux'):
        (package / name).mkdir(parents=True, exist_ok=True)
    (package / 'launcher.ini').write_text(f'[launcher]\nwindows={build}\nwindows_custom=false\nlinux=\n', encoding='utf-8')
    metadata = dict(version=build, name=f'ZIMA-CAD-Parts-{build}', platform='windows-x64',
                    commit=commit, source_modified=dirty, origin='release-candidate' if args.release else 'development',
                    signed=False, qt=run(str(args.qt.resolve() / 'bin/qmake.exe'), '-query', 'QT_VERSION').strip())
    (runtime / 'version.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
    (package / 'README.txt').write_text(
        f'ZIMA-CAD-Parts-{build}\nRun ZIMA-CAD-Parts.exe on Windows.\n'
        'Debian binary is not included in this first Windows package.\n'
        'Select a retained build in launcher.ini or pass -Version YYYYMMDDNN.\n'
        'Custom builds: custom/windows/NAME; launch with -Custom -Version NAME.\n'
        'This package is not signed. Automatic updates/cleanup are not implemented.\n'
        'Source snapshot and build documentation are in source/.\n', encoding='utf-8')
    hashes = {p.relative_to(package).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in sorted(package.rglob('*')) if p.is_file()}
    (package / 'checksums.json').write_text(json.dumps(hashes, indent=2) + '\n', encoding='utf-8')
    print(package)


if __name__ == '__main__':
    main()
