#!/usr/bin/env python3
"""Annotate reviewed graph captures, without reading native solver output.

Each emitted coordinate is a matching pixel in a private source screenshot.
Input equations guide searches only; source hashes and pixel hits are checked.
Only small independently transcribed coordinate/color tables enter fixtures.
"""
import argparse,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def annotate(record):
    source=ROOT/record['source'];assert hashlib.sha256(source.read_bytes()).hexdigest()==record['source_sha256']
    width,height=map(int,subprocess.check_output(['magick',str(source),'-format','%w %h','info:'],text=True).split())
    raw=subprocess.check_output(['magick',str(source),'-depth','8','rgb:-'])
    offset=0 if width==808 else -4;left,top,side=325+offset,121+offset,398
    lo,hi=record['expected']['range_min'],record['expected']['range_max'];samples=[];misses=[]
    def pixel(x,y):return list(raw[(y*width+x)*3:(y*width+x)*3+3])
    def seek(x,y,color,label):
        px=left+(x-lo)/(hi-lo)*side;py=top+(hi-y)/(hi-lo)*side
        candidates=[]
        for yy in range(max(top+2,round(py)-3),min(top+side-1,round(py)+4)):
            for xx in range(max(left+2,round(px)-3),min(left+side-1,round(px)+4)):
                if pixel(xx,yy)==color:candidates.append(((xx-px)**2+(yy-py)**2,xx,yy))
        if not candidates:return False
        _,xx,yy=min(candidates);samples.append({'label':label,'x':(xx-left)/side,'y':(yy-top)/side,'rgb':color});return True
    for index,row in enumerate(record['problem']['constraints']):
        a,b=map(float,row['coefficients']);rhs=float(row['rhs']);hit=0
        for fraction in [.2,.4,.6,.8]:
            x=lo+(hi-lo)*fraction
            if b:
                y=(rhs-a*x)/b
                if lo<y<hi:hit+=seek(x,y,[255,0,0],f'constraint-{index+1}')
            y=lo+(hi-lo)*fraction
            if a:
                x=(rhs-b*y)/a
                if lo<x<hi:hit+=seek(x,y,[255,0,0],f'constraint-{index+1}')
        if not hit:misses.append(f'constraint-{index+1}: no isolated red sample (overlap or clipping)')
    # An axis covered by a coincident red constraint is not a blue geometry
    # sample. A nearby status-text glyph must not be mislabeled as that axis.
    rows=record['problem']['constraints']
    covered_vertical=any(float(r['rhs'])==0 and float(r['coefficients'][1])==0 and float(r['coefficients'][0])!=0 for r in rows)
    covered_horizontal=any(float(r['rhs'])==0 and float(r['coefficients'][0])==0 and float(r['coefficients'][1])!=0 for r in rows)
    if not covered_vertical:seek(0,hi*.8,[0,0,255],'vertical-axis')
    if not covered_horizontal:seek(hi*.65,0,[0,0,255],'horizontal-axis')
    expected=record['expected']
    if expected['message']:
        seek(0,0,[0,0,255],'terminal-marker')
    else:
        z=float(expected['objective']);a,b=map(float,record['problem']['objective'])
        for fraction in [.3,.6]:
            x=lo+(hi-lo)*fraction
            if b:
                y=(z-a*x)/b
                if lo<y<hi:seek(x,y,[0,0,255],'objective')
        seek(float(expected['x1']),float(expected['x2']),[0,0,255],'optimum-marker')
    gray=[(x,y) for y in range(top+5,top+side-5) for x in range(left+5,left+side-5) if pixel(x,y)==[128,128,128]]
    if gray:
        for index in [len(gray)//4,len(gray)//2,3*len(gray)//4]:
            x,y=gray[index];samples.append({'label':'feasible-dot','x':(x-left)/side,'y':(y-top)/side,'rgb':[128,128,128]})
    return samples,misses
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--transcribe',action='store_true');args=parser.parse_args()
    path=ROOT/'tests/fixtures/graphical.json';records=json.loads(path.read_text());audit=[]
    for record in records:
        samples,misses=annotate(record);assert samples,record['case'];audit.append({'case':record['case'],'samples':len(samples),'overlaps':misses});record['color_samples']=samples
    destination=ROOT/'build/phase2-graphical/source-geometry-audit.json';destination.parent.mkdir(parents=True,exist_ok=True);destination.write_text(json.dumps(audit,indent=2)+'\n')
    if args.transcribe:path.write_text(json.dumps(records,indent=2)+'\n')
    print(len(records),'source hashes verified;',sum(a['samples'] for a in audit),'source pixel samples annotated')
    for row in audit:
        if row['overlaps']:print(row['case'],row['overlaps'])
if __name__=='__main__':main()
