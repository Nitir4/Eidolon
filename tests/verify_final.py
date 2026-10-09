"""Exact final summary comparison with independently transcribed TORA output."""
import csv,json,subprocess,sys
from pathlib import Path
records=json.loads(Path(sys.argv[2]).read_text());comparison=[];failures=[]
for r in records:
 p=r['problem'];lines=[f"{len(p['variables'])} {len(p['constraints'])} {p['sense']}"]
 lines += [f"{v['lower']} {v['upper']} {int(v['unrestricted'])}" for v in p['variables']]
 lines += [' '.join(p['objective'])]
 lines += [f"{c['relation']} {c['rhs']} "+' '.join(c['coefficients']) for c in p['constraints']]
 lines += [' '.join(map(str,r['format']))]
 process=subprocess.run([sys.argv[1]],input='\n'.join(lines)+'\n',text=True,capture_output=True)
 if process.returncode:failures.append(r['case']+': '+process.stderr);continue
 actual=json.loads(process.stdout)
 def compare(field,expected,got):
  comparison.append([r['case'],field,str(expected),str(got),expected==got])
  if expected!=got:failures.append(f"{r['case']} {field}: TORA {expected!r}, clone {got!r}")
 for field in ['heading','iteration','objective_label','objective_value','message']:compare(field,r[field],actual[field])
 compare('section-count',len(r['sections']),len(actual['sections']))
 for i,(expected,got) in enumerate(zip(r['sections'],actual['sections'])):
  compare(f'section-{i}-columns',expected['columns'],got['columns'])
  compare(f'section-{i}-rows',len(expected['rows']),len(got['rows']))
  for j,(a,b) in enumerate(zip(expected['rows'],got['rows'])):
   compare(f'section-{i}-row-{j}-label',a['label'],b['label'])
   for k,(x,y) in enumerate(zip(a['values'],b['values'])):compare(f'section-{i}-row-{j}-cell-{k}',x,y)
 target=Path(sys.argv[3]);target.parent.mkdir(parents=True,exist_ok=True)
 with target.open('w',newline='',encoding='utf-8') as f:
  w=csv.writer(f,delimiter='\t');w.writerow(['case','field','expected','actual','match']);w.writerows(comparison)
 if not any(x.startswith(r['case']+' ') or x.startswith(r['case']+':') for x in failures):print(r['case'],'MATCH')
if failures:raise AssertionError('\n'.join(failures))
print(len(records),'final summaries,',len(comparison),'exact fields MATCH')
