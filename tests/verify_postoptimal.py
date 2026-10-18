"""Authenticate original files, then compare reviewed solves and independent basis algebra."""
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

def data(p):
    n,m=len(p['variables']),len(p['constraints'])
    grid=['']*(n+2)+p['objective']+['','']
    for row in p['constraints']:grid+=row['coefficients']+['        '+row['relation'],row['rhs']]
    for key in ['lower','upper']:
        grid += [('infinity' if v[key]=='inf' else '-infinity' if v[key]=='-inf' else v[key]) for v in p['variables']]+['','']
    grid += [('y' if v['unrestricted'] else 'n') for v in p['variables']]+['','']
    lines=['3000','5','2','1',json.dumps(p['title']),str(n),str(m),json.dumps('Maximize' if p['sense']=='max' else 'Minimize')]
    return ('\r\n'.join(lines+[json.dumps(x) for x in grid])+'\r\n').encode('ascii')
def flatten(v,path=''):
    if isinstance(v,dict):
        for k,x in v.items():yield from flatten(x,path+'/'+k)
    elif isinstance(v,list):
        for k,x in enumerate(v):yield from flatten(x,path+'/'+str(k))
    else:yield path,v
rows=[]
records=json.loads(Path(sys.argv[2]).read_text())
with tempfile.TemporaryDirectory() as temp:
    for r in records:
        files=[]
        for field,prov in [('baseline','baseline_provenance'),('changed','provenance')]:
            blob=data(r[field]);assert hashlib.sha256(blob).hexdigest()==r[prov]['file_sha256'],(r['case'],field,'authentic identity')
            path=Path(temp)/(field+'.txt');path.write_bytes(blob);files.append(str(path))
        run=subprocess.run([sys.argv[1],*files],capture_output=True,text=True)
        assert run.returncode==0,(r['case'],run.stderr)
        actual=dict(flatten(json.loads(run.stdout)));expected=dict(flatten(r['expected']))
        assert actual.keys()==expected.keys()
        for key,value in expected.items():rows.append([r['case'],key,value,actual[key],value==actual[key]])
        assert actual==expected,(r['case'],actual,expected)
with Path(sys.argv[3]).open('w',newline='') as out:
    w=csv.writer(out,delimiter='\t');w.writerow(['case','field','expected','actual','match']);w.writerows(rows)
print(f'{len(records)} original changed-model comparisons, {len(rows)} outcome/basis fields MATCH')
