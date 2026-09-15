"""Bundle an already built Debian 13 amd64 executable. Must run on Debian 13.

This packager remains experimental until the resulting bundle passes the Debian
CI and KDE/Wayland tests. It never disables the WebEngine sandbox.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import runpy
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
shared = runpy.run_path(str(ROOT / 'tools/distribution/package-windows.py'))
run, version, source_files = (shared[key] for key in ('run', 'version', 'source_files'))

# Host ABI, display server and GPU-driver libraries remain supplied by Debian.
SYSTEM = re.compile(r'^(ld-linux.*|lib(c|m|pthread|dl|rt|resolv|util)\.so\..*|libnss_.*|lib(GL|GLX|GLdispatch|EGL|OpenGL|GLESv[12]|drm|gbm|vulkan)([_.-].*)?\.so.*)$')


def dependencies(path):
    output = run('ldd', str(path))
    if 'not found' in output:
        raise RuntimeError(f'Unresolved dependencies for {path}:\n{output}')
    result = []
    for line in output.splitlines():
        match = re.search(r'=>\s+(/\S+)', line)
        if match:
            result.append(Path(match[1]))
    return result


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--exe', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--qmake', default='qmake6')
    args = parser.parse_args()
    distro = platform.freedesktop_os_release()
    if distro.get('ID') != 'debian' or distro.get('VERSION_ID') != '13' or platform.machine() != 'x86_64':
        raise RuntimeError('Build/package only on Debian 13 amd64')
    build = version()
    exe = args.exe.resolve()
    if json.loads(run(str(exe), '--build-info')).get('version') != build:
        raise RuntimeError('Rebuild executable: version differs from source')
    output = args.output.resolve()
    if output.exists():
        raise RuntimeError('Output must not exist')
    package = output / 'ZIMA-CAD-Parts'
    runtime = package / 'linux' / build
    binary = runtime / 'bin'
    libraries = runtime / 'lib'
    binary.mkdir(parents=True)
    libraries.mkdir()
    query = lambda key: Path(run(args.qmake, '-query', key).strip())
    shutil.copy2(exe, binary / 'ZIMA-CAD-Parts')
    process = query('QT_INSTALL_LIBEXECS') / 'QtWebEngineProcess'
    shutil.copy2(process, binary / 'QtWebEngineProcess')
    plugin_root = query('QT_INSTALL_PLUGINS')
    for group in ('platforms', 'imageformats', 'iconengines', 'networkinformation', 'tls', 'platformthemes', 'styles', 'xcbglintegrations', 'wayland-shell-integration', 'wayland-graphics-integration-client'):
        if (plugin_root / group).is_dir():
            shutil.copytree(plugin_root / group, runtime / 'plugins' / group, symlinks=False)
    if not list((runtime / 'plugins/platforms').glob('*wayland*.so')):
        raise RuntimeError('Install qt6-wayland before packaging')
    qtdata = query('QT_INSTALL_DATA')
    shutil.copytree(qtdata / 'resources', runtime / 'resources', symlinks=False)
    shutil.copytree(query('QT_INSTALL_TRANSLATIONS'), runtime / 'translations', symlinks=False)
    resource_candidates = list(Path('/usr/share').glob('opencascade*/resources')) + [Path('/usr/share/opencascade/resources')]
    occt = next((p for p in resource_candidates if (p / 'Shaders').is_dir()), None)
    if occt is None:
        raise RuntimeError('OCCT resources with Shaders not found')
    shutil.copytree(occt, runtime / 'occt', symlinks=False)
    seeds = [exe, process] + list((runtime / 'plugins').rglob('*.so'))
    inspected = set()
    host = set()
    while seeds:
        path = seeds.pop()
        if path in inspected:
            continue
        inspected.add(path)
        for dependency in dependencies(path):
            if SYSTEM.fullmatch(dependency.name):
                host.add(dependency.name)
                continue
            target = libraries / dependency.name
            if not target.exists():
                shutil.copy2(dependency, target)
                seeds.append(dependency)
    for path in list(binary.iterdir()) + list(libraries.iterdir()) + list((runtime / 'plugins').rglob('*.so')):
        relative = os.path.relpath(libraries, path.parent)
        subprocess.run(['patchelf', '--set-rpath', '$ORIGIN/' + relative, str(path)], check=True)
    (binary / 'qt.conf').write_text('[Paths]\nPrefix=..\nPlugins=plugins\nLibraries=lib\nLibraryExecutables=bin\nData=.\nTranslations=translations\n')
    shutil.copy2(ROOT / 'tools/distribution/run-debian.sh', runtime / 'ZIMA-CAD-Parts')
    (runtime / 'ZIMA-CAD-Parts').chmod(0o755)
    shutil.copy2(ROOT / 'tools/distribution/ZIMA-CAD-Parts.sh', package / 'ZIMA-CAD-Parts.sh')
    (package / 'ZIMA-CAD-Parts.sh').chmod(0o755)
    for folder in ('windows', 'custom/windows', 'custom/linux'):
        (package / folder).mkdir(parents=True, exist_ok=True)
    (package / 'launcher.ini').write_text(f'[launcher]\nwindows=\nlinux={build}\nlinux_custom=false\n')
    for source, relative in source_files():
        target = package / 'source' / build / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    shutil.copytree(ROOT / 'licenses', runtime / 'licenses')
    shutil.copy2(ROOT / 'LICENSE', runtime / 'licenses/Parts-LICENSE')
    (runtime / 'build.ini').write_text(f'[build]\nversion={build}\nplatform=debian-13-x86_64\n')
    metadata = dict(version=build, platform='debian-13-x86_64', commit=run('git', 'rev-parse', 'HEAD').strip(),
                    origin='experimental', signed=False, host_libraries=sorted(host),
                    qt=run(args.qmake, '-query', 'QT_VERSION').strip())
    (runtime / 'version.json').write_text(json.dumps(metadata, indent=2) + '\n')
    checksums = {p.relative_to(package).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                 for p in sorted(package.rglob('*')) if p.is_file()}
    (package / 'checksums.json').write_text(json.dumps(checksums, indent=2) + '\n')
    print(package)


if __name__ == '__main__':
    main()
