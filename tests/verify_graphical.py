"""Compare independently transcribed graphical results, never generate goldens."""
import json,subprocess,sys,math
from pathlib import Path
exe,fixture,report=sys.argv[1:]
rows=['case\tfield\texpected\tactual\tmatch'];failures=[]
for r in json.loads(Path(fixture).read_text()):
    p=r['problem'];parts=[f"{len(p['variables'])} {len(p['constraints'])} {p['sense']}"]
    parts += [f"{v['lower']} {v['upper']} {int(v['unrestricted'])}" for v in p['variables']]
    parts += [' '.join(p['objective'])]
    parts += [f"{c['relation']} {c['rhs']} "+' '.join(c['coefficients']) for c in p['constraints']]
    parts += [' '.join(map(str,r['format']))]
    proc=subprocess.run([exe],input='\n'.join(parts)+'\n',text=True,capture_output=True)
    if proc.returncode:failures.append(f"{r['case']}: {proc.stderr}");continue
    actual=json.loads(proc.stdout)
    for field,expected in r['expected'].items():
        found=actual.get(field);match=math.isclose(expected,found,rel_tol=1e-12,abs_tol=1e-12) if isinstance(expected,(int,float)) and isinstance(found,(int,float)) else expected==found
        rows.append('\t'.join([r['case'],field,str(expected),str(found),str(match).lower()]))
        if not match:failures.append(f"{r['case']}.{field}: {expected!r} != {found!r}")
Path(report).write_text('\n'.join(rows)+'\n')
if failures:raise SystemExit('\n'.join(failures))
print(f'{len(rows)-1} graphical oracle fields match')
