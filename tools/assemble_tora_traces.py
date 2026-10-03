#!/usr/bin/env python3
"""Assemble observed glyph transcriptions; report gaps instead of guessing.

Rows are aligned using screenshot glyph fingerprints. No LP arithmetic is
used. Every resulting table retains its contributing screenshot paths.
"""
from pathlib import Path
import argparse
import json
import re

ROOT=Path(__file__).resolve().parents[1]

def row_value(cell):
    return cell.get('text') if cell.get('text') is not None else cell.get('label')

def read_frame(path):
    left=json.loads(path.read_text())
    right_path=path.with_name(path.name.replace('.cells.json','-right.cells.json'))
    right=json.loads(right_path.read_text()) if right_path.exists() else None
    by_y={r['y']:r for r in right['rows']} if right else {}
    rows=[]
    counts={side:1+max((c['column'] for row in data['rows'] for c in row['cells']),default=-1)
            for side,data in [('left',left),('right',right)] if data}
    for row in left['rows']:
        r=by_y.get(row['y'])
        rows.append({'left':row,'right':r,'source':left['source'],
                     'right_source':right['source'] if right else None,
                     'column_counts':counts,
                     'fingerprint':(row['glyph_fingerprint'],r['glyph_fingerprint'] if r else None)})
    return rows

def stitch(frames):
    rows=[];issues=[];start=0
    def signature(row):
        return {(side,c['column']): (c.get('text'),c.get('label'),c.get('heading'))
                for side in ['left','right'] if row.get(side)
                for c in row[side]['cells']
                if c.get('text') is not None or c.get('label') or c.get('heading')}
    def compatible(a,b):
        sa,sb=signature(a),signature(b)
        if not sa and not sb:
            return all(not r[side]['cells'] for r in [a,b] for side in ['left','right'] if r.get(side))
        shared=sa.keys() & sb.keys()
        return bool(shared) and all(sa[k]==sb[k] for k in shared)
    def merge(a,b):
        # Only merge directly read cells from overlapping screenshot rows.
        # Unknown pixels (for example a captured cursor) never supply a value.
        for side in ['left','right']:
            if not b.get(side):continue
            if not a.get(side):a[side]=b[side];continue
            ca={c['column']:c for c in a[side]['cells']}
            present={c['column'] for c in b[side]['cells']}
            for column,old in list(ca.items()):
                if column not in present and column<b['column_counts'].get(side,0) and row_value(old) is None and not old.get('heading'):
                    del ca[column]
            for cb in b[side]['cells']:
                old=ca.get(cb['column'])
                if old is None or (row_value(old) is None and not old.get('heading')):
                    ca[cb['column']]=cb
            a[side]['cells']=list(ca.values())
        a.setdefault('overlap_sources',[]).extend([b['source'],b.get('right_source')])
    for frame in frames:
        if not rows:
            rows.extend(frame);continue
        options=[]
        for offset in range(start,min(len(rows),start+20)+1):
            overlap=min(len(frame),len(rows)-offset)
            if overlap<3:continue
            matches=sum(rows[offset+i]['fingerprint']==frame[i]['fingerprint'] or compatible(rows[offset+i],frame[i]) for i in range(overlap))
            if matches/overlap>=0.85:
                options.append((matches,overlap,offset))
        if not options:
            issues.append('Cannot align a scroll frame by observed row fingerprints')
            continue
        _,overlap,start=max(options)
        for i in range(overlap):
            if rows[start+i]['fingerprint']!=frame[i]['fingerprint'] and not compatible(rows[start+i],frame[i]):
                issues.append('Overlapping row differs; visual review required')
            else:merge(rows[start+i],frame[i])
        rows.extend(frame[overlap:])
    return rows,issues

def parse_tables(rows):
    tables=[];table=None;issues=[];maps={}
    for observed in rows:
        left=observed['left'];right=observed['right']
        heading=next((c['heading'] for c in left['cells'] if c.get('heading')),None)
        if heading:
            table={'phase':heading['phase'],'iteration':heading['iteration'],
                   'heading':heading['display'],'columns':[],'rows':[],'sources':[],
                   'selected_entering_column':None,'selected_leaving_row':None}
            tables.append(table);maps={}
            continue
        if table is None:continue
        if not left['cells'] and (right is None or not right['cells']):continue
        header=next((c for c in left['cells'] if c.get('label')=='Basic'),None)
        if header:
            for c in left['cells']:
                if c.get('background_rgb')==[0,255,0] and c.get('label'):
                    table['selected_entering_column']=c['label']
            for side,data in [('left',left),('right',right)]:
                if not data:continue
                maps[side]={c['column']:c['label'] for c in data['cells'] if c.get('label') and c['column']>0}
            names=list(maps.get('left',{}).values())
            for name in maps.get('right',{}).values():
                if name not in names:names.append(name)
            table['columns']=names
            if 'Solution' not in names:issues.append(f"Iteration {table['iteration']}: Solution column unresolved")
            continue
        label_cell=next((c for c in left['cells'] if c['column']==0),None)
        if label_cell is None:continue
        label=label_cell.get('label')
        if label_cell.get('background_rgb')==[255,0,0]:
            table['selected_leaving_row']=label
        if not label:
            issues.append(f"Iteration {table['iteration']}: row label unresolved")
            continue
        if label=='Basic' or label.startswith('Iteration '):continue
        values={};unresolved=[]
        for side,data in [('left',left),('right',right)]:
            if not data:continue
            cells={c['column']:c for c in data['cells']}
            for column,name in maps.get(side,{}).items():
                cell=cells.get(column)
                if cell is None:
                    values.setdefault(name,'')
                elif cell.get('clipped_at_viewport_edge'):
                    continue
                elif row_value(cell) is None:
                    unresolved.append(name)
                else:
                    value=row_value(cell)
                    if name in values and values[name] not in ('',value):
                        issues.append(f"Iteration {table['iteration']} {label}/{name}: horizontal views disagree")
                    values[name]=value
        unresolved=[name for name in unresolved if name not in values or not values[name]]
        if unresolved:issues.append(f"Iteration {table['iteration']} {label}: unresolved {', '.join(unresolved)}")
        table['rows'].append({'label':label,'values':values,'unresolved':unresolved})
        for source in [observed['source'],observed['right_source']]+observed.get('overlap_sources',[]):
            if source and source not in table['sources']:table['sources'].append(source)
    for table in tables:
        for label in ['Lower Bound','Upper Bound',"Unrestr'd (y/n)?"]:
            if not any(r['label']==label for r in table['rows']):
                issues.append(f"Iteration {table['iteration']}: {label} row missing")
        if not any(r['label'].startswith('z (') for r in table['rows']):
            issues.append(f"Iteration {table['iteration']}: objective row missing")
    return tables,issues

def assemble(directory,run):
    top=directory/'tableau-top.cells.json'
    if not top.exists():top=directory/'iteration-01.cells.json'
    if not top.exists():return {'status':'missing-cell-transcriptions','issues':['First tableau not transcribed']}
    frames=[read_frame(top)]
    maximum=run.get('scroll_pages',8)
    for i in range(1,maximum+1):
        p=directory/f'tableau-scroll-{i:02d}.cells.json'
        if not p.exists():return {'status':'missing-cell-transcriptions','issues':[str(p)]}
        frames.append(read_frame(p))
    rows,alignment=stitch(frames)
    tables,issues=parse_tables(rows)
    issues=sorted(set(alignment+issues))
    return {'status':'assembled-transcription-needs-review' if not issues else 'transcription-gaps',
            'tables':tables,'issues':issues,'observed_row_count':len(rows)}

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root',type=Path,default=ROOT/'reference/tora-captures/methods/lp/corpus')
    args=ap.parse_args();root=args.root.resolve()
    progress=json.loads((root/'progress.json').read_text())['runs'];audit={}
    for key,run in progress.items():
        if run['status']=='directly-observed-input-warning':
            audit[key]={'status':'expected-input-warning'};continue
        result=assemble(root/key,run)
        (root/key/'transcribed-trace.json').write_text(json.dumps(result,indent=2)+'\n')
        audit[key]={'status':result['status'],'tables':len(result.get('tables',[])),'issues':result['issues']}
    (root/'transcription-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
    from collections import Counter
    print(Counter(v['status'] for v in audit.values()))

if __name__=='__main__':main()
