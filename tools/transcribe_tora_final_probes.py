"""Exact final-summary glyph observation; never computes LP expected values.

Two-variable/three-constraint focused layout only. Unknown glyphs remain gaps.
Scientific seed strings were directly reviewed from the reference screenshot.
"""
import json,sys,re,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from read_tora_numeric_cells import image_rgb,glyphs
base=Path('reference/tora-captures/output-studies');focused=Path('reference/tora-captures/probes/phase2-final')
templates={};labels={};provenance=[]
for p in sorted(base.glob('*/final/transcribed-summary.json')):
 d=json.loads(p.read_text())
 if not d.get('sections'):continue
 raw=image_rgb(Path(d['source']))
 for rows in d['sections'].values():
  for row in rows:
   for cell in row['cells'].values():
    text=cell['text'];patterns=glyphs(raw,cell['box'])
    if text in ['infinity','-infinity']:labels['|'.join(patterns)]=text;continue
    assert len(text)==len(patterns),(p,text,len(patterns))
    for pattern,ch in zip(patterns,text):
     assert pattern not in templates or templates[pattern]==ch
     templates[pattern]=ch
raw=image_rgb(focused/'final-scientific/summary.png')
for box,text in [((210,206,335,222),'2.00000e+00'),((335,223,445,239),'3.00000e+00'),((335,411,445,427),'3.66667e+00')]:
 patterns=glyphs(raw,box);assert len(text)==len(patterns),(box,len(patterns),text)
 for pattern,ch in zip(patterns,text):assert pattern not in templates or templates[pattern]==ch;templates[pattern]=ch
 provenance.append({'source':str(focused/'final-scientific/summary.png'),'box':box,'reviewed_text':text})
for dest in sorted(p for p in focused.iterdir() if p.is_dir() and (p/'captured.json').exists()):
 d=json.loads((dest/'captured.json').read_text());p=d['problem'];bounds=sum(float(v['lower'])>0 and not v['unrestricted'] for v in p['variables'])+sum(v['upper']!='inf' for v in p['variables'])
 raw=image_rgb(dest/'summary.png');sections={};issues=[]
 layout=[('variables',2,206,[(210,335),(335,445),(445,575)],['value','objective_coefficient','contribution']),('constraints',3+bounds,257,[(210,335),(335,445)],['rhs','slack_surplus']),('sensitivity_variables',2,360+17*bounds,[(210,335),(335,445),(445,575),(575,700)],['current','minimum','maximum','reduced_cost']),('sensitivity_constraints',3+bounds,411+17*bounds,[(210,335),(335,445),(445,575),(575,700)],['current','minimum','maximum','dual_price'])]
 for key,count,start,xs,fields in layout:
  rows=[]
  for i in range(count):
   y=start+17*i;cells={}
   if y+16>489:continue
   for (x0,x1),field in zip(xs,fields):
    box=(x0,y,x1,y+16);patterns=glyphs(raw,box);text=labels.get('|'.join(patterns))
    if text is None:text=''.join(templates.get(q,'?') for q in patterns)
    if '?' in text or not text:issues.append((key,i,field,text))
    cells[field]={'text':text,'box':box}
   rows.append({'row':i+1,'cells':cells})
  sections[key]=rows
 if (dest/'summary-bottom.png').exists():
  bottom=image_rgb(dest/'summary-bottom.png');bands=[]
  for y in range(188,470):
   if tuple(bottom[(y*808+715)*3:(y*808+715)*3+3])!=(0,255,255):continue
   if not bands or y>bands[-1][-1]+1:bands.append([y])
   else:bands[-1].append(y)
  # Directly reviewed multiple-bound bottom frame starts with constraint 1;
  # its cyan header is above the viewport. Do not invent a band.
  if not bands:
   if dest.name!='sens-both-bounds':raise ValueError('No reviewed bottom layout: '+str(dest))
   first=190
  else:first=bands[-1][-1]+1
  rows=[]
  # Reviewed negative-lower/upper screen omits its sensitivity bound row.
  rhs_count=3 if dest.name=='sens-negative-lower-upper' else 3+bounds
  for i in range(rhs_count):
   cells={}
   for field,(x0,x1) in zip(['current','minimum','maximum','dual_price'],[(210,335),(335,445),(445,575),(575,700)]):
    box=(x0,first+17*i,x1,first+17*i+16);patterns=glyphs(bottom,box);text=labels.get('|'.join(patterns))
    if text is None:text=''.join(templates.get(q,'?') for q in patterns)
    if not text or '?' in text:issues.append(('bottom',i,field,text))
    cells[field]={'text':text,'box':box,'source':str(dest/'summary-bottom.png'),'source_sha256':hashlib.sha256((dest/'summary-bottom.png').read_bytes()).hexdigest()}
   rows.append({'row':i+1,'cells':cells})
  sections['sensitivity_constraints']=rows
 result={'source':str(dest/'summary.png'),'source_sha256':hashlib.sha256((dest/'summary.png').read_bytes()).hexdigest(),'status':'exact-glyph-summary-transcription' if not issues else 'transcription-gaps','sections':sections,'issues':issues}
 (dest/'transcribed-summary.json').write_text(json.dumps(result,indent=2)+'\n')
 print(dest.name,issues,{k:[{c:v['text'] for c,v in r['cells'].items()} for r in a] for k,a in sections.items()})
(focused/'summary-glyph-provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
