#!/usr/bin/env python3
"""Create reviewable OCR drafts from oracle screenshots, never golden results.

Requires ImageMagick and a separately installed Tesseract. Every draft records
the source SHA256. Colors are normalized only in a temporary derived image;
the original observation is preserved. Numeric and label review is required.
"""
from __future__ import annotations
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def grouped(values):
    groups = []
    for value in values:
        if not groups or value > groups[-1][-1] + 1:
            groups.append([value])
        else:
            groups[-1].append(value)
    return [sum(g) // len(g) for g in groups]

def normalize_table(source, destination):
    width, height = 808, 608
    raw = subprocess.check_output(['magick', str(source), '-depth', '8', 'rgb:-'])
    if len(raw) != width * height * 3:
        raise ValueError(f'Unexpected image dimensions: {source}')
    def rgb(x, y):
        at = (y * width + x) * 3
        return tuple(raw[at:at + 3])
    def line_pixel(p):
        return min(p) > 170 and p[0] > 200
    def white(p):
        return min(p) > 190 and max(p) - min(p) < 50
    surface = [(x,y) for y in range(194,508) for x in range(40,765)
               if rgb(x,y)[0] < 100 and rgb(x,y)[1] > 150 and rgb(x,y)[2] > 150]
    left = min(x for x,y in surface)
    right = max(x for x,y in surface)
    # Cyan arrow glyphs belong to the scrollbar, not the last grid cell.
    gray = [x for x in range(left+1,765)
            if sum(min(rgb(x,y))>100 and max(rgb(x,y))-min(rgb(x,y))<30
                   for y in range(240,480))>180]
    strips=[]
    for x in gray:
        if not strips or x>strips[-1][-1]+1: strips.append([x])
        else: strips[-1].append(x)
    scrollbars=[s for s in strips if len(s)>=8]
    if scrollbars: right=scrollbars[-1][0]-1
    bottom=max(y for x,y in surface if x<=right)
    xs = grouped([x for x in range(left+1, right)
                  if sum(line_pixel(rgb(x,y)) for y in range(195,330)) > 100])
    ys = grouped([y for y in range(194, bottom+1)
                  if sum(line_pixel(rgb(x,y)) for x in range(left+1,right)) > (right-left)*0.85])
    xs = [left-1] + [x for x in xs if left < x < right] + [right+1]
    # Do not interpret the cropped last row or horizontal scroll track.
    ys = [193] + [y for y in ys if 193 < y <= bottom]
    pixels = bytearray([255]) * (width * height)
    for y0, y1 in zip(ys, ys[1:]):
        for x0, x1 in zip(xs, xs[1:]):
            bg = Counter(tuple((c//32)*32 for c in rgb(x,y))
                         for y in range(y0+1,y1) for x in range(x0+1,x1)).most_common(1)[0][0]
            light_ink = bg[2] > 128 and bg[1] < 96 and bg[0] < 96
            green_bg = bg[1] > 128 and bg[2] < 96 and bg[0] < 96
            red_bg = bg[0] > 128 and bg[1] < 96 and bg[2] < 96
            for y in range(y0+1, y1):
                for x in range(x0+1, x1):
                    r,g,b = rgb(x,y)
                    ink = white((r,g,b)) if light_ink else ((red_bg or green_bg) and white((r,g,b))) or (b > 150 and r < 100 and g < 100)
                    if ink:
                        pixels[y*width+x] = 0
    destination.write_bytes(f'P5\n{width} {height}\n255\n'.encode() + pixels)
    return xs, ys, (left, right)

def transcribe(source, executable, env, force=False):
    output = source.with_suffix('.ocr.json')
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    if output.exists() and not force:
        prior = json.loads(output.read_text())
        if prior.get('source_sha256') == digest and prior.get('transcriber_version') == 2:
            return False
    is_table = source.name.startswith(('iteration-01', 'tableau-scroll', 'tableau-top'))
    with tempfile.TemporaryDirectory(prefix='tora-ocr-') as directory:
        tmp = Path(directory)
        if is_table:
            xs, ys, (left, right) = normalize_table(source, tmp/'normalized.pgm')
            conversion = [str(tmp/'normalized.pgm'), '-crop', f'{right-left-1}x311+{left}+194']
        else:
            xs, ys = [], []
            conversion = [str(source)]
        subprocess.run(['magick', *conversion, '-filter', 'triangle', '-resize', '400%', str(tmp/'read.png')], check=True)
        result = subprocess.run([executable, str(tmp/'read.png'), 'stdout', '--psm', '6' if is_table else '11'],
                                env=env, text=True, capture_output=True, check=True)
        output.write_text(json.dumps({
            'source': str(source.relative_to(ROOT)), 'source_sha256': digest,
            'status': 'ocr-draft-needs-visual-review', 'engine': 'Tesseract',
            'transcriber_version': 2,
            'table_column_boundaries': xs, 'table_row_boundaries': ys,
            'text': result.stdout, 'diagnostics': result.stderr.strip(),
        }, indent=2) + '\n')
        source.with_suffix('.ocr.txt').write_text(result.stdout)
    return True

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--tesseract', default='tesseract')
    ap.add_argument('--runtime-root', type=Path)
    ap.add_argument('--workers', type=int, default=3)
    ap.add_argument('--force', action='store_true')
    ap.add_argument('--root', type=Path, default=ROOT/'reference/tora-captures/methods/lp/corpus')
    args = ap.parse_args()
    env = os.environ.copy()
    env['OMP_THREAD_LIMIT'] = '1'
    if args.runtime_root:
        env['LD_LIBRARY_PATH'] = str(args.runtime_root/'usr/lib')
        env['TESSDATA_PREFIX'] = str(args.runtime_root/'usr/share/tessdata')
    sources = sorted(p for p in args.root.resolve().rglob('*.png')
                     if p.name.startswith(('iteration-01', 'tableau-scroll', 'tableau-top', 'terminal-message', 'method-selection-warning', 'input-requirement-warning', 'final-output')))
    def work(source):
        return transcribe(source, args.tesseract, env, args.force)
    count = 0
    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        for i, changed in enumerate(pool.map(work, sources), 1):
            count += changed
            if i % 50 == 0:
                print(f'Processed {i}/{len(sources)} screenshots; {count} updated drafts', flush=True)
    print(f'Processed {len(sources)} screenshots; {count} updated drafts; visual review required.')

if __name__ == '__main__':
    main()
