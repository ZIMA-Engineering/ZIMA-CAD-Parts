"""Rebuild the native SVG action/file icons (no raster or theme dependencies)."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
AZURE = '#00D1FF'

def write(name, body):
    path = ROOT / 'gfx' / (name + '.svg')
    path.parent.mkdir(parents=True, exist_ok=True)
    # A narrow light keyline keeps black artwork visible on dark palettes.
    # Keep this in the assets so UI forms and menus share the same appearance.
    halo = re.sub(r'stroke-width="([0-9.]+)"',
                  lambda m: f'stroke-width="{float(m.group(1)) + 1.4:g}"', body)
    halo = re.sub(r'(fill|stroke)="(?!none")[^"]+"',
                  lambda m: m.group(1) + '="#F4F6F8"', halo)
    path.write_text('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">\n'
        '<g fill="none" stroke="#F4F6F8" stroke-width="3.1" stroke-linecap="round" '
        'stroke-linejoin="round">' + halo + '</g>\n'
        '<g fill="none" stroke="#111111" stroke-width="1.7" stroke-linecap="round" '
        'stroke-linejoin="round">' + body.replace('@', AZURE) + '</g>\n</svg>\n', encoding='utf-8')

actions = {
    'folder': '<path d="M3 7V5h6l2 2h10v13H3Z" fill="@"/><path d="M3 10h18"/>',
    'folder-open': '<path d="M3 19V5h6l2 2h9v4H7L3 19Z"/><path d="M7 11h15l-4 9H3Z" fill="@"/>',
    'folder-add': '<path d="M3 7V5h6l2 2h10v13H3Z" fill="@"/><path d="M8 14h8m-4-4v8"/>',
    'copy': '<rect x="3" y="3" width="12" height="14" rx="1"/><rect x="9" y="8" width="12" height="14" rx="1" fill="@"/>',
    'delete': '<path d="M7 7h10l-1 14H8Z" fill="@"/><path d="M4 7h16M9 7V3h6v4M10 11v6M14 11v6"/>',
    'filter': '<path d="M3 4h18l-7 8v7l-4 2v-9Z" fill="@"/>',
    'lock': '<path d="M7 10V7a5 5 0 0 1 10 0v3"/><rect x="4" y="10" width="16" height="12" rx="2" fill="@"/><path d="M12 15v3"/>',
    'move': '<path d="M3 4h10v16H3Z" fill="@"/><path d="M8 12h13m-4-4 4 4-4 4"/>',
    'parts': '<path d="m12 3 9 5v9l-9 5-9-5V8Z" fill="@"/><path d="m3 8 9 5 9-5M12 13v9M7.5 5.5l9 5"/>',
    'refresh': '<path d="M20 10a8 8 0 0 0-14-4L3 9m0-5v5h5"/><path d="M4 14a8 8 0 0 0 14 4l3-3m0 5v-5h-5" stroke="@" stroke-width="2.5"/>',
    'terminal': '<rect x="2" y="4" width="20" height="16" rx="2" fill="@"/><path d="m6 9 3 3-3 3m6 0h5"/>',
    'home': '<path d="m3 11 9-8 9 8h-2v10H5V11Z" fill="@"/><path d="M10 21v-7h4v7"/>',
    'settings': '<path d="m10 2 4 0 1 4 4-1 2 4-3 3 3 3-2 4-4-1-1 4h-4l-1-4-4 1-2-4 3-3-3-3 2-4 4 1Z" fill="@"/><circle cx="12" cy="12" r="3"/>',
    'pin': '<path d="m8 3 8 0-1 7 4 4H5l4-4Z" fill="@"/><path d="M12 14v8"/>',
    'edit': '<path d="M4 3h10v4M4 3v18h16v-9"/><path d="m9 14 10-10 3 3-10 10-5 2Z" fill="@"/>',
    'rename': '<path d="M3 6h12v13H3Z" fill="@"/><path d="M18 3v18m-3-18h6m-6 18h6M6 11h6M6 15h4"/>',
    'add': '<path d="M9 3h6v6h6v6h-6v6H9v-6H3V9h6Z" fill="@"/>',
    'remove': '<path d="M3 9h18v6H3Z" fill="@"/>',
    'tab-new': '<path d="M3 21V5h7l3 3h8v13Z"/><path d="M8 14h9m-4.5-4.5v9" stroke="@" stroke-width="2.5"/>',
    'loading': '<circle cx="12" cy="12" r="8"/><path d="M12 4a8 8 0 0 1 8 8" stroke="@" stroke-width="3"/><path d="M12 7v5l3 2"/>',
    'sync': '<path d="M3 5h7v14H3ZM14 5h7v14h-7Z" fill="@"/><path d="M7 9h10m-2-2 2 2-2 2M17 15H7m2-2-2 2 2 2"/>',
    'clean': '<path d="m5 15 10-10 4 4L9 19Z" fill="@"/><path d="m5 15-3 6 7-2M4 19l3-3M17 3l3-2M21 7h2"/>',
}
for direction, transform in [('right',''), ('left','rotate(180 12 12)'), ('up','rotate(-90 12 12)'), ('down','rotate(90 12 12)')]:
    actions['arrow-'+direction] = f'<g transform="{transform}"><path d="M3 9h10V4l8 8-8 8v-5H3Z" fill="@"/></g>'
for name, body in actions.items():
    write('navigation/' + name, body)

cube = '<path d="m12 3 8 4v9l-8 5-8-5V7Z" fill="@"/><path d="m4 7 8 5 8-5M12 12v9"/>'
assembly = '<path d="m8 3 6 3v7l-6 4-6-4V6Z"/><path d="m16 9 6 3v7l-6 3-6-3v-7Z" fill="@"/><path d="m10 12 6 3 6-3M16 15v7"/>'
sheet = '<path d="M3 3h18v18H3Z"/><path d="M6 6h12v9H6Z" fill="@"/><path d="M12 18h6M15 15v6"/>'
for name, body in [('part',cube), ('assembly',assembly), ('drawing',sheet),
                   ('drawing-format','<path d="M2 4h20v16H2Z"/><path d="M5 7h14v10H5Z" stroke="@"/><path d="M14 14h5v3h-5Z"/>'),
                   ('title-block','<path d="M2 5h20v15H2Z"/><path d="M2 14h20v6H2Z" fill="@"/><path d="M13 5v15M2 10h20M7 14v6"/>')]:
    write('icons/zima-cad/' + name, body)

categories = {
 'part': 'prt_proe prt_nx catpart par ipt sldprt fcstd',
 'assembly': 'asm asm_nx catproduct iam sldasm',
 'mesh': 'step stl iges neu_proe blend psm',
 'text': 'pdf office-document', 'table': 'office-spreadsheet',
 'presentation': 'office-presentation', 'project': 'office-project',
 'drawing': 'drw drw_nx catdrawing dft idw slddrw dwb dwg dxf frm office-drawing',
 'database': 'office-database', 'mail': 'internet-mail mail-mbox',
 'archive': 'zip rar tar 7z', 'image': 'image-x-generic', 'audio': 'audio-x-generic',
 'file': 'undefined',
}
motifs = {
 'part': '<path d="m8 7 4-2 4 2v5l-4 2-4-2Z" fill="@"/><path d="m8 7 4 2 4-2m-4 2v5"/>',
 'assembly': '<path d="M6 6h7v6H6ZM12 10h7v6h-7Z" fill="@"/>',
 'drawing': '<path d="M6 6h12v8H6Z" fill="@"/><path d="M12 11h6m-3 0v3"/>',
 'mesh': '<path d="m6 12 6-7 6 7-6 3Z" fill="@"/><path d="m6 12 6-2 6 2m-6-7v10"/>',
 'text': '<path d="M6 6h12v3H6Z" fill="@"/><path d="M6 12h12M6 15h8"/>',
 'table': '<path d="M6 5h12v10H6Z" fill="@"/><path d="M6 10h12M12 5v10"/>',
 'presentation': '<path d="M6 5h12v8H6Z" fill="@"/><path d="M12 13v3m-3 0h6"/>',
 'project': '<path d="M6 6h7v3H6ZM10 12h8v3h-8Z" fill="@"/><path d="M7 9v4h3"/>',
 'database': '<path d="M7 7v7c0 3 10 3 10 0V7" fill="@"/><ellipse cx="12" cy="7" rx="5" ry="2"/><path d="M7 11c0 3 10 3 10 0"/>',
 'mail': '<path d="M6 6h12v9H6Z" fill="@"/><path d="m6 6 6 5 6-5"/>',
 'archive': '<path d="M10 4h4v12h-4Z" fill="@"/><path d="M10 7h4m-4 4h4m-4 4h4"/>',
 'image': '<circle cx="15" cy="7" r="2" fill="@"/><path d="m5 15 5-7 4 5 3-2 2 4Z" fill="@"/>',
 'audio': '<path d="M10 13V6l7-2v8M10 8l7-2"/><ellipse cx="8" cy="14" rx="2" ry="2" fill="@"/><ellipse cx="15" cy="13" rx="2" ry="2" fill="@"/>',
 'file': '<path d="M7 7h10M7 11h10M7 15h6" stroke="@"/>',
}
labels = {'prt_proe':'PRT','prt_nx':'NX','neu_proe':'NEU','catpart':'CAT','catproduct':'CAT','catdrawing':'CAT',
          'office-document':'DOC','office-spreadsheet':'XLS','office-presentation':'PPT','office-project':'PLAN',
          'office-drawing':'DRAW','office-database':'DB','internet-mail':'MAIL','mail-mbox':'MBOX',
          'image-x-generic':'IMG','audio-x-generic':'SND','undefined':''}
for category, names in categories.items():
    for name in names.split():
        label = labels.get(name, name.upper().replace('_NX',''))
        body = '<rect x="3" y="2" width="18" height="20" rx="1"/>' + motifs[category]
        body += f'<text x="12" y="20.5" fill="#111111" stroke="none" text-anchor="middle" font-family="sans-serif" font-weight="bold" font-size="4.5">{label}</text>'
        write('icons/' + name, body)
