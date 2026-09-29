"""Package PNGs exported by render-brand-icon from gfx/app-icon.svg."""
from pathlib import Path
import shutil
import struct
import sys

root = Path(__file__).resolve().parents[1]
pngs = Path(sys.argv[1])
sizes = [16, 24, 32, 48, 64, 128, 256]
images = [(size, (pngs / f'{size}.png').read_bytes()) for size in sizes]
header = struct.pack('<HHH', 0, 1, len(images))
offset = 6 + 16 * len(images)
entries = b''
for size, data in images:
    entries += struct.pack('<BBBBHHII', size % 256, size % 256, 0, 0, 1, 32, len(data), offset)
    offset += len(data)
(root / 'gfx/icon.ico').write_bytes(header + entries + b''.join(data for _, data in images))
chunks = b''
for size, kind in [(16,b'icp4'), (32,b'icp5'), (64,b'icp6'), (128,b'ic07'),
                   (256,b'ic08'), (512,b'ic09'), (1024,b'ic10')]:
    data = (pngs / f'{size}.png').read_bytes()
    chunks += kind + struct.pack('>I', len(data) + 8) + data
(root / 'gfx/icon.icns').write_bytes(b'icns' + struct.pack('>I', len(chunks) + 8) + chunks)
shutil.copyfile(pngs / '256.png', root / 'gfx/icon.png')
for size in [16, 22, 24, 32, 48, 64, 128, 256]:
    shutil.copyfile(pngs / f'{size}.png', root / f'dist/icons/hicolor/{size}x{size}/apps/cz.zima_engineering.ZimaCadParts.png')
