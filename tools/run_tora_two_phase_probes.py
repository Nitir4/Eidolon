"""Launch-first, resumable observations of missing Two-phase cases.

Only new bounds/free/cleanup/transition cases are run. Original captures and
the running TORA process are retained. No solver supplies oracle values.
"""
import hashlib
import json
import run_tora_corpus as oracle
from run_tora_file_probes import windows, save_as

ROOT=oracle.ROOT/'reference/tora-captures/probes/phase2-two-phase'

def problems():
    records=json.loads((oracle.ROOT/'tests/fixtures/m-method.json').read_text())
    def source(name):return json.loads(json.dumps(next(r['problem'] for r in records if r['case']==name)))
    cases={name:source('06_equality') for name in ['tp-upper-bound','tp-lower-bound','tp-free-variable','tp-redundant-equality','tp-zero-equality','tp-phase2-pivot','tp-bound-infeasible','tp-mixed-slack','tp-manual-transition','tp-manual-ack','tp-transition-bottom']}
    cases['tp-upper-bound']['variables'][0]['upper']='2'
    cases['tp-lower-bound']['variables'][0]['lower']='1'
    cases['tp-free-variable']['variables'][0].update(lower='-inf',upper='inf',unrestricted=True)
    cases['tp-redundant-equality']['constraints'][1]={'relation':'=','rhs':'8','coefficients':['2','2']}
    p=cases['tp-zero-equality'];p['constraints'][0]['rhs']='0';p['constraints'][1]['rhs']='0';p['constraints'][2]['rhs']='0'
    cases['tp-phase2-pivot']['objective']=['2','3']
    cases['tp-bound-infeasible']['variables'][0]['upper']='0'
    cases['tp-mixed-slack']['constraints'][2].update(relation='<=',rhs='3')
    cases['tp-dual-scientific']=source('10_dual')
    cases['tp-slack-cancel']=source('01_normal')
    for name,p in cases.items():p['title']=name
    return cases

def fixture(p):
    n=len(p['variables']);m=len(p['constraints'])
    cells=['']*(n+2)+p['objective']+['','']
    for row in p['constraints']:cells+=row['coefficients']+['        '+row['relation'],row['rhs']]
    for key in ['lower','upper']:
        cells += [('infinity' if v[key]=='inf' else '-infinity' if v[key]=='-inf' else v[key]) for v in p['variables']]+['','']
    cells += [('y' if v['unrestricted'] else 'n') for v in p['variables']]+['','']
    lines=['3000','5','2','1','"'+p['title']+'"',str(n),str(m),'"Maximize"' if p['sense']=='max' else '"Minimize"']+['"'+c+'"' for c in cells]
    return ('\r\n'.join(lines)+'\r\n').encode('ascii')

def run(driver,selected=None):
    ROOT.mkdir(parents=True,exist_ok=True);oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(exist_ok=True)
    cases=problems()
    for name in selected or cases:
        if name not in cases:raise ValueError('Unknown Two-phase case: '+name)
        dest=ROOT/name;dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():print('Already captured:',name,flush=True);continue
        p=cases[name];source=dest/'source.txt';source.write_bytes(fixture(p))
        driver.recover_main_menu();driver.load_case_from_main(windows(source));driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists():save_as(driver,dest/'tora-saved.txt',dest)
        driver.open_method('two_phase');driver.capture(dest/'after-selection.png')
        if name=='tp-slack-cancel':driver.click(1006,585);driver.capture(dest/'after-cancel.png')
        if name=='tp-dual-scientific':
            driver.click(858,502);driver.click(952,524);driver.key('ctrl+a');driver.run('xdotool','type','--clearmodifiers','--delay','30','2')
        driver.capture(dest/'output-format.png');driver.click(960,595);driver.pause(1);driver.move(1300,825);driver.capture(dest/'iteration-01.png')
        if name=='tp-manual-transition':
            for i in range(1,5):
                driver.click(865,432);driver.capture(dest/f'after-next-{i:02d}.png')
            driver.key('Return')
        if name=='tp-manual-ack':
            for i in range(1,4):
                driver.click(865,432);driver.capture(dest/f'after-next-{i:02d}.png')
            driver.key('Return');driver.capture(dest/'after-phase1-ack.png')
            driver.key('Return');driver.capture(dest/'after-optimal-ack.png')
        if name=='tp-transition-bottom':
            for i in range(1,4):driver.click(865,432)
            driver.capture(dest/'phase1-notice.png');driver.key('Return');driver.capture(dest/'after-phase1-ack.png')
            left,right,bottom=driver._table_bounds(dest/'after-phase1-ack.png')
            driver.move(oracle.LEGACY_ORIGIN[0]+right+8,oracle.LEGACY_ORIGIN[1]+bottom-7)
            driver.run('xdotool','click','--repeat','40','--delay','35','1');driver.move(1300,825);driver.capture(dest/'after-phase1-ack-bottom.png')
            if bottom<495:driver._capture_right_columns(dest/'after-phase1-ack-bottom-right.png',left,right,bottom)
            driver.click(865,432);driver.capture(dest/'after-next-on-phase2.png');driver.key('Return');driver.capture(dest/'after-final-ack.png')
            result={'status':'observed-manual-phase-boundary','problem':p,'review':'pending direct screenshot review','sha256':{f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in dest.iterdir() if f.suffix in ['.png','.txt']}}
            (dest/'captured.json').write_text(json.dumps(result,indent=2)+'\n');driver.recover_main_menu();continue
        result=driver.capture_all_iterations('two_phase',dest);result.update(problem=p,review='pending direct screenshot review')
        result['sha256']={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in dest.iterdir() if f.suffix in ['.png','.txt']}
        (dest/'captured.json').write_text(json.dumps(result,indent=2)+'\n')
        print('New Two-phase gap captured:',name,flush=True);driver.recover_main_menu()
