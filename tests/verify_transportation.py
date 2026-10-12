"""Exact iteration comparison with independent TORA screen transcriptions."""
import csv
import json
from pathlib import Path
import subprocess
import sys
import hashlib
import tempfile

records=json.loads(Path(sys.argv[2]).read_text())
method=sys.argv[4] if len(sys.argv)>4 else 'northwest'
arguments=['--method',method]
differences=[]
fields=tableaus=0
comparison=[]
for record in records:
    p=record['problem']
    lines=[f"{len(p['supply'])} {len(p['demand'])}"]
    lines+=[' '.join(map(str,row)) for row in p['costs']]
    lines+=[' '.join(map(str,p['supply'])),' '.join(map(str,p['demand']))]
    result=subprocess.run([sys.argv[1]]+arguments,input='\n'.join(lines)+'\n',text=True,capture_output=True)
    if result.returncode:
        raise AssertionError(record['case']+': '+result.stderr)
    actual=json.loads(result.stdout)
    grid=[['S/D Name']+['']*(len(p['demand'])+1)]
    grid += [['']+[str(v) for v in row]+[str(p['supply'][i])] for i,row in enumerate(p['costs'])]
    grid += [['']+[str(v) for v in p['demand']]+['']]
    lines=['4000','5','2','1',json.dumps('Transportation '+record['case']),str(len(p['supply'])),str(len(p['demand']))]
    lines += [json.dumps(value) for row in grid for value in row]
    document=('\r\n'.join(lines)+'\r\n').encode('ascii')
    assert hashlib.sha256(document).hexdigest()==record['file_sha256'],record['case']+' actual oracle file hash'
    with tempfile.TemporaryDirectory(prefix='tora-transportation-') as temporary:
        path=Path(temporary)/'independently-generated.txt';path.write_bytes(document)
        encoded=subprocess.run([sys.argv[1],'--file',str(path),'--roundtrip'],capture_output=True)
        assert encoded.returncode==0,record['case']+': '+encoded.stderr.decode()
        assert encoded.stdout==document,record['case']+' exact writer round trip'
        from_file=subprocess.run([sys.argv[1],'--file',str(path)]+arguments,text=True,capture_output=True)
        assert from_file.returncode==0,record['case']+': '+from_file.stderr
        assert json.loads(from_file.stdout)==actual,record['case']+' file-backed trace'
    if actual!=record['tables']:
        for i,(expected,got) in enumerate(zip(record['tables'],actual),1):
            for key in expected:
                if expected[key]!=got.get(key):differences.append((record['case'],i,key,expected[key],got.get(key)))
        if len(actual)!=len(record['tables']):differences.append((record['case'],'count',len(record['tables']),len(actual)))
    def flatten(value,path=''):
        if isinstance(value,dict):
            for k,v in value.items():yield from flatten(v,path+'/'+k)
        elif isinstance(value,list):
            for k,v in enumerate(value):yield from flatten(v,path+'/'+str(k))
        else:yield path,value
    actual_fields=dict(flatten(actual))
    expected_fields=dict(flatten(record['tables']))
    for key,expected in expected_fields.items():
        comparison.append([record['case'],key,expected,actual_fields.get(key)]);fields+=1
    if actual_fields.keys()!=expected_fields.keys():
        differences.append((record['case'],'field paths differ'))
    tableaus+=len(actual)
if len(sys.argv)>3:
    target=Path(sys.argv[3]);target.parent.mkdir(parents=True,exist_ok=True)
    with target.open('w',newline='',encoding='utf-8') as stream:
        writer=csv.writer(stream,delimiter='\t');writer.writerow(['case','field','TORA','Clone']);writer.writerows(comparison)
if differences:raise AssertionError('\n'.join(map(str,differences)))
print(f'{len(records)} {method} cases, {tableaus} tableaus, {fields} exact fields MATCH')
