"""Authenticate original changed LP files; compare reviewed final-summary fields."""
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
    for row in p['constraints']:
        grid+=row['coefficients']+['        '+row['relation'],row['rhs']]
    for key in ('lower','upper'):
        grid += [('infinity' if v[key]=='inf' else '-infinity' if v[key]=='-inf' else v[key]) for v in p['variables']]+['','']
    grid += [('y' if v['unrestricted'] else 'n') for v in p['variables']]+['','']
    lines=['3000','5','2','1',json.dumps(p['title']),str(n),str(m),json.dumps('Maximize' if p['sense']=='max' else 'Minimize')]
    return ('\r\n'.join(lines+[json.dumps(x) for x in grid])+'\r\n').encode('ascii')


def flatten(value,path=''):
    if isinstance(value,dict):
        for key,child in value.items():
            yield from flatten(child,path+'/'+key)
    elif isinstance(value,list):
        for key,child in enumerate(value):
            yield from flatten(child,path+'/'+str(key))
    else:
        yield path,value


rows=[]
records=json.loads(Path(sys.argv[2]).read_text())
with tempfile.TemporaryDirectory() as temp:
    for record in records:
        blob=data(record['changed'])
        assert hashlib.sha256(blob).hexdigest()==record['provenance']['file_sha256'],(record['case'],'authentic file identity')
        path=Path(temp)/(record['case']+'.txt');path.write_bytes(blob)
        run=subprocess.run([sys.argv[1],str(path),*record['baseline_rhs']],capture_output=True,text=True)
        assert run.returncode==0,(record['case'],run.stderr)
        actual=dict(flatten(json.loads(run.stdout)));expected=dict(flatten(record['expected']))
        assert actual.keys()==expected.keys()
        for field,value in expected.items():
            rows.append([record['case'],field,value,actual[field],value==actual[field]])
        assert actual==expected,(record['case'],actual,expected)
with Path(sys.argv[3]).open('w',newline='') as out:
    writer=csv.writer(out,delimiter='\t');writer.writerow(['case','field','expected','actual','match']);writer.writerows(rows)
print(f'{len(records)} REA-captured equivalent LPs, {len(rows)} original summary fields MATCH')
