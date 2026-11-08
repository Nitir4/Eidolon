"""Reconstruct authenticated original unit-Transportation files; compare Assignment."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
records=json.loads(Path(sys.argv[2]).read_text())
with tempfile.TemporaryDirectory() as temporary:
    for r in records:
        costs=r['costs'];m,n=len(costs),len(costs[0]);sign=1 if r['mode']=='min' else -1
        grid=[['S/D Name',*['']*(n+1)]]
        grid += [['',*[str(sign*x) for x in row],'1'] for row in costs]
        grid += [['',*['1']*n,'']]
        lines=['4000','5','2','1',json.dumps(r['title']),str(m),str(n)]
        lines += [json.dumps(x) for row in grid for x in row]
        data=('\r\n'.join(lines)+'\r\n').encode('ascii')
        assert hashlib.sha256(data).hexdigest()==r['file_sha256'],(r['case'],'authentic input identity')
        path=Path(temporary)/'input.txt';path.write_bytes(data)
        run=subprocess.run([sys.argv[1],r['mode'],str(path)],text=True,capture_output=True)
        assert run.returncode==0,(r['case'],run.stderr)
        assert json.loads(run.stdout)==r['expected'],(r['case'],run.stdout,r['expected'])
        assert sign*r['expected']['objective']==float(r['expected_tora_objective'])
print(f'{len(records)} authentic original equivalent-model objectives and positive pairs MATCH')
