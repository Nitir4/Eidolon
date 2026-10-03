#!/usr/bin/env python3
"""Transcribe numeric screenshot cells by exact observed glyph matching.

This observation tool recognizes only glyphs learned from explicitly reviewed
cells. It never fills missing digits using arithmetic. Unrecognized/clipped
cells remain unresolved. The resulting cell data still needs table/label review.
"""
from __future__ import annotations
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile
from transcribe_tora_captures import normalize_table

ROOT = Path(__file__).resolve().parents[1]
CAPTURES = ROOT/'reference/tora-captures'

def glyphs(raw, box):
    x0,y0,x1,y1 = box
    def pixel(x,y):
        at = (y*808+x)*3
        return tuple(raw[at:at+3])
    bg = Counter(tuple((c//32)*32 for c in pixel(x,y))
                 for y in range(y0,y1) for x in range(x0,x1)).most_common(1)[0][0]
    light = bg[2]>128 and bg[1]<96 and bg[0]<96
    green = bg[1]>128 and bg[2]<96 and bg[0]<96
    red = bg[0]>128 and bg[1]<96 and bg[2]<96
    ink = set()
    for y in range(y0,y1):
        for x in range(x0,x1):
            r,g,b = pixel(x,y)
            white = min(r,g,b)>190 and max(r,g,b)-min(r,g,b)<50
            if (white if light else ((red or green) and white) or (b>150 and r<100 and g<100)):
                ink.add((x,y))
    occupied = sorted({x for x,y in ink})
    groups = []
    for x in occupied:
        if not groups or x>groups[-1][-1]+1:
            groups.append([x])
        else:
            groups[-1].append(x)
    result = []
    for group in groups:
        left,right = group[0],group[-1]
        ys = [y for x,y in ink if left<=x<=right]
        top,bottom = min(ys),max(ys)
        pattern = '/'.join(''.join('1' if (x,y) in ink else '0' for x in range(left,right+1))
                           for y in range(top,bottom+1))
        result.append(pattern)
    return result

def image_rgb(path):
    return subprocess.check_output(['magick',str(path),'-depth','8','rgb:-'])

def learn():
    frozen = CAPTURES/'reviewed-glyph-seeds'
    normal = frozen/'01_normal-all_slack-iteration-01.png'
    digits = frozen/'roundtrips-probe-numeric-digits-input-reopened.png'
    bounded = frozen/'09_bounded-all_slack-iteration-01.png'
    dual = frozen/'10_dual-dual_simplex-iteration-01.png'
    dual_final = frozen/'15_minimization-dual_simplex-tableau-scroll-01.png'
    seeds = [
        (normal,(211,227,287,241),'-3.00'),
        (normal,(289,227,365,241),'-2.00'),
        (normal,(211,243,287,257),'1.00'),
        (normal,(601,243,677,257),'4.00'),
        (digits,(290,224,360,240),'8.00'),
        (digits,(362,224,430,240),'7.00'),
        (digits,(533,242,602,257),'9.00'),
        (digits,(533,259,602,274),'6.00'),
        (digits,(533,276,602,291),'5.00'),
        (bounded,(601,259,677,273),'6.00'),
        (bounded,(601,275,677,289),'7.00'),
        (dual,(601,275,677,289),'-5.00'),
        (dual_final,(601,227,677,241),'8.00'),
        (dual_final,(601,386,677,401),'9.00'),
    ]
    templates = {}
    provenance = []
    raw_by_path = {}
    for source,box,text in seeds:
        raw = raw_by_path.setdefault(source,image_rgb(source))
        patterns = glyphs(raw,box)
        if len(patterns)!=len(text):
            raise ValueError(f'Review seed geometry: {source}, {box}, {text}, glyph count {len(patterns)}')
        for pattern,char in zip(patterns,text):
            if pattern in templates and templates[pattern]!=char:
                raise ValueError('Conflicting reviewed glyph templates')
            templates[pattern] = char
        provenance.append({'source':str(source.relative_to(ROOT)), 'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
                           'box':box,'reviewed_text':text})
    if set('0123456789.-')-set(templates.values()):
        raise ValueError('Reviewed digit inventory is incomplete')
    return templates,provenance

def learn_labels():
    normal = CAPTURES/'methods/lp/corpus/01_normal/all_slack/iteration-01.png'
    dual = CAPTURES/'methods/lp/corpus/10_dual/dual_simplex/iteration-01.png'
    final = CAPTURES/'reviewed-glyph-seeds/15_minimization-dual_simplex-tableau-scroll-01.png'
    wide = CAPTURES/'methods/lp/corpus/06_equality/m_method/iteration-01.png'
    unbounded = CAPTURES/'methods/lp/corpus/04_unbounded/m_method/iteration-01.png'
    seeds=[]
    for i,label in enumerate(['Basic','z (max)','sx3','sx4','sx5','Lower Bound','Upper Bound',"Unrestr'd (y/n)?"]):
        seeds.append((normal,(114,211+16*i,210,225+16*i),label))
    for i,label in enumerate(['x1','x2','sx3','sx4','sx5','Solution']):
        seeds.append((normal,(211+78*i,211,288+78*i,225),label))
    for i,label in enumerate(['z (min)','Sx3','Sx4','Sx5']):
        seeds.append((dual,(114,227+16*i,210,241+16*i),label))
    for i,label in enumerate(['x1','x2','Sx3','Sx4','Sx5','Solution']):
        seeds.append((dual,(211+78*i,211,288+78*i,225),label))
    seeds.extend([(final,(114,243,210,257),'x1'),(final,(114,435,210,449),'x2')])
    seeds.append((wide,(76,322,172,337),"Unrestr'd (y/n)?"))
    seeds.append((wide.with_name('tableau-top.png'),(76,322,172,337),"Unrestr'd (y/n)?"))
    seeds.append((CAPTURES/'methods/lp/corpus/06_equality/two_phase/tableau-scroll-05-right.png',
                  (329,354,406,369),'blocked'))
    cursor=CAPTURES/'methods/lp/corpus/finite-lower/bounded_simplex/tableau-top.png'
    seeds.extend([(cursor,(367,306,444,321),'__observed_cursor__'),
                  (cursor,(367,322,444,337),'__observed_cursor__')])
    seeds.extend([(normal,(114,195,210,209),'Iteration 1'),
                  (final,(114,195,210,209),'Iteration 2'),
                  (final,(114,355,210,369),'Iteration 3'),
                  (normal,(289,307,365,321),'infinity'),
                  (normal,(289,323,365,337),'n'),
                  (unbounded,(76,275,172,289),'Rx8')])
    for i,label in enumerate(['Rx5','Rx6','Rx7']):
        seeds.append((wide,(76,243+16*i,172,257+16*i),label))
    templates={}
    for source,box,label in seeds:
        raw=image_rgb(source)
        key='|'.join(glyphs(raw,box))
        if key and key in templates and templates[key]!=label:
            raise ValueError(f'Conflicting label templates: {label} vs {templates[key]}')
        if key:templates[key]=label
    return templates

def learn_caption_characters(digits):
    templates=dict(digits)
    seeds=[(CAPTURES/'methods/lp/corpus/01_normal/all_slack/iteration-01.png',
            (114,195,210,209),'Iteration1'),
           (CAPTURES/'methods/lp/corpus/03_infeasible/two_phase/iteration-01.png',
            (76,195,172,209),'Phase1(Iter1')]
    normal=CAPTURES/'methods/lp/corpus/01_normal/all_slack/iteration-01.png'
    dual=CAPTURES/'methods/lp/corpus/10_dual/dual_simplex/iteration-01.png'
    wide=CAPTURES/'methods/lp/corpus/06_equality/m_method/iteration-01.png'
    for i,text in enumerate(['sx3','sx4','sx5']):
        seeds.append((normal,(114,243+16*i,210,257+16*i),text))
    seeds.append((dual,(114,243,210,257),'Sx3'))
    for i,text in enumerate(['Rx5','Rx6','Rx7']):
        seeds.append((wide,(76,243+16*i,172,257+16*i),text))
    seeds.extend([
        (CAPTURES/'methods/lp/corpus/finite-upper/bounded_simplex/tableau-scroll-02.png',(211,194,288,209),'Ux1'),
        (CAPTURES/'methods/lp/corpus/finite-lower/bounded_simplex/tableau-top.png',(211,210,288,225),'Lx1'),
    ])
    for source,box,text in seeds:
        patterns=glyphs(image_rgb(source),box)
        if len(patterns)!=len(text):
            raise ValueError('Caption seed must be reviewed before decoding')
        for pattern,char in zip(patterns,text):
            if pattern in templates and templates[pattern]!=char:
                raise ValueError('Conflicting caption glyph')
            templates[pattern]=char
    return templates

def transcribe(source,templates,labels,captions):
    output = source.with_suffix('.cells.json')
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    template_digest = hashlib.sha256(json.dumps([templates,labels,captions],sort_keys=True).encode()).hexdigest()
    if output.exists() and json.loads(output.read_text()).get('reader_version')==3 and json.loads(output.read_text()).get('source_sha256')==digest and json.loads(output.read_text()).get('template_sha256')==template_digest:
        return False
    raw = image_rgb(source)
    with tempfile.TemporaryDirectory(prefix='tora-cells-') as directory:
        xs,ys,_ = normalize_table(source,Path(directory)/'normalized.pgm')
    rows = []
    for y0,y1 in zip(ys,ys[1:]):
        if y1-y0>18 or y1-y0<12:
            continue
        cells = []
        fingerprints=[]
        for column,(x0,x1) in enumerate(zip(xs,xs[1:])):
            box = (x0+1,y0+1,x1,y1)
            patterns = glyphs(raw,box)
            fingerprints.append(patterns)
            if not patterns:
                continue
            text = ''.join(templates.get(p,'?') for p in patterns)
            label = labels.get('|'.join(patterns))
            if label=='__observed_cursor__':
                fingerprints[-1]=[]
                continue
            caption=''.join(captions.get(p,'?') for p in patterns)
            heading=None
            match=re.fullmatch(r'Iteration(\d+)',caption)
            if match:heading={'phase':None,'iteration':int(match[1]),'display':f'Iteration {match[1]}'}
            match=re.fullmatch(r'Phase(\d+)\(Iter(\d+)\)?',caption)
            if match:heading={'phase':int(match[1]),'iteration':int(match[2]),'display':f'Phase {match[1]} (Iter {match[2]}'}
            if not label and re.fullmatch(r'(?:x|sx|Sx|Rx|Ux|Lx|ULx)\d+',caption):label=caption
            exact = '?' not in text and re.fullmatch(r'-?\d+\.\d+',text) is not None
            clipped = x1 == xs[-1] and (x1-x0) < 76
            exact = exact and not clipped
            cells.append({'box':box,'text':text if exact else None,
                          'column':column,'heading':heading,
                          'label':label,
                          'clipped_at_viewport_edge':clipped,
                          'status':'exact-observed-glyph-match' if exact else 'exact-observed-label-match' if label else 'unresolved-label-or-clipped-cell'})
        while fingerprints and not fingerprints[-1]: fingerprints.pop()
        rows.append({'y':y0,'cells':cells,'glyph_fingerprint':hashlib.sha256(json.dumps(fingerprints).encode()).hexdigest()})
    output.write_text(json.dumps({'source':str(source.relative_to(ROOT)), 'source_sha256':digest,
        'reader_version':3,
        'template_sha256':template_digest,
        'status':'numeric-cells-transcribed-table-and-label-review-pending',
        'rows':rows},indent=2)+'\n')
    return True

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root',type=Path,default=CAPTURES/'methods/lp/corpus')
    ap.add_argument('--workers',type=int,default=3)
    ap.add_argument('--refresh-labels',action='store_true',help='refresh unresolved labels using existing verified cell geometry')
    args=ap.parse_args()
    templates,provenance=learn()
    labels=learn_labels()
    captions=learn_caption_characters(templates)
    (CAPTURES/'numeric-glyph-provenance.json').write_text(json.dumps({'reviewed_seeds':provenance,
         'recognized_characters':sorted(set(templates.values()))},indent=2)+'\n')
    sources=sorted(p for p in args.root.resolve().rglob('*.png')
                   if p.name.startswith(('iteration-01','tableau-top','tableau-scroll')))
    def work(source):
        if not args.refresh_labels:return transcribe(source,templates,labels,captions)
        target=source.with_suffix('.cells.json')
        data=json.loads(target.read_text())
        if data.get('reader_version')!=3 or data['source_sha256']!=hashlib.sha256(source.read_bytes()).hexdigest():
            raise ValueError(f'Cell geometry requires a full refresh: {source}')
        raw=None
        for row in data['rows']:
            kept=[]
            for cell in row['cells']:
                if raw is None:raw=image_rgb(source)
                x0,y0,_,_=cell['box'];at=((y0+2)*808+x0+2)*3
                cell['background_rgb']=list(raw[at:at+3])
                if cell.get('text') is not None or cell.get('label') or cell.get('heading'):
                    kept.append(cell);continue
                label=labels.get('|'.join(glyphs(raw,cell['box'])))
                caption=''.join(captions.get(p,'?') for p in glyphs(raw,cell['box']))
                if not label and re.fullmatch(r'(?:x|sx|Sx|Rx|Ux|Lx|ULx)\d+',caption):label=caption
                if label=='__observed_cursor__':continue
                if label:
                    cell['label']=label;cell['status']='exact-observed-label-match'
                kept.append(cell)
            row['cells']=kept
        target.write_text(json.dumps(data,indent=2)+'\n')
        return True
    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        for i,_ in enumerate(pool.map(work,sources),1):
            if i%50==0:print(f'Numeric cells processed {i}/{len(sources)}',flush=True)
    print(f'Numeric cells processed {len(sources)}; labels/table coverage still need review.')

if __name__=='__main__':
    main()
