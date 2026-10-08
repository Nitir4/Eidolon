"""Missing finite-M terminal/bounds/free traces. Launch-first driver owns TORA.

Skip completed markers; preserve the running reference and original captures.
No calculated numbers are used to transcribe oracle output.
"""
from pathlib import Path
import hashlib,json
import run_tora_corpus as oracle
from run_tora_file_probes import windows,save_as

ROOT=oracle.ROOT/'reference/tora-captures/probes/phase2-m-method'

def run(driver,selected=None):
    ROOT.mkdir(parents=True,exist_ok=True);oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(exist_ok=True)
    # Reuse a saved model, changing only the focused dimensions of the test.
    models=json.loads((oracle.ROOT/'tests/fixtures/m-method.json').read_text())
    eq=next(r['problem'] for r in models if r['case']=='06_equality')
    cases={name:(value,None) for name,value in [('m-zero','0'),('m-negative','-1'),('m-fraction','0.5'),('m-exponent','1e2'),('m-nonnumeric','bogus')]}
    cases.update({'m-dual-scientific':('100','scientific'),'m-dual-scientific-short':('100','scientific'),'m-slack-cancel-button':('100','cancel'),'m-slack-cancel-solve':('100','cancel'),'m-slack-cancel':('100','cancel'),'m-upper-bound':('100','upper'),'m-lower-bound':('100','lower'),'m-free-variable':('100','free')})
    for name in selected or list(cases):
        if name not in cases:raise ValueError('Unknown M case: '+name)
        dest=ROOT/name;dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():print('Already captured:',name,flush=True);continue
        value,variant=cases[name];base=next(r['problem'] for r in models if r['case']==('10_dual' if variant=='scientific' else '01_normal')) if variant in ['scientific','cancel'] else eq
        p=json.loads(json.dumps(base));p['title']=name
        if variant=='upper':p['variables'][0]['upper']='2'
        if variant=='lower':p['variables'][0]['lower']='1'
        if variant=='free':p['variables'][0].update(lower='-inf',upper='inf',unrestricted=True)
        cells=['','','','']+p['objective']+['','']
        for row in p['constraints']:cells += row['coefficients']+['        '+row['relation'],row['rhs']]
        for key in ['lower','upper']:
            cells += [('-infinity' if v[key]=='-inf' else 'infinity' if v[key]=='inf' else v[key]) for v in p['variables']]+['','']
        cells += [('y' if v['unrestricted'] else 'n') for v in p['variables']]+['','']
        lines=['3000','5','2','1','"'+name+'"','2','3',('"Maximize"' if p['sense']=='max' else '"Minimize"')]+['"'+c+'"' for c in cells]
        source=dest/'source.txt';source.write_bytes(('\r\n'.join(lines)+'\r\n').encode('ascii'))
        driver.recover_main_menu();driver.load_case_from_main(windows(source));driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists():save_as(driver,dest/'tora-saved.txt',dest)
        driver.open_method('m_method',stop_at_m=True);driver.capture(dest/'m-default.png')
        if variant=='cancel':
            if name.endswith(('-button','-solve')):driver.click(1006,585)
            else:driver.key('Escape')
            driver.capture(dest/'after-cancel.png')
            if name.endswith('-solve'):
                driver.click(960,595);driver.pause(1);driver.capture(dest/'iteration-01.png');driver.capture_all_iterations('all_slack',dest)
            (dest/'captured.json').write_text(json.dumps({'problem':p,'review':'pending cancel route review'})+'\n')
            driver.recover_main_menu();continue
        driver.click(1010,485);driver.key('Home');driver.key('shift+End');driver.key('BackSpace')
        driver.run('xdotool','type','--clearmodifiers','--delay','30','--',value)
        driver.capture(dest/'m-entered.png');driver.click(955,534);driver.capture(dest/'output-format.png')
        if variant=='scientific':
            driver.click(858,502)
            if name.endswith('-short'):
                driver.click(952,524);driver.key('ctrl+a');driver.run('xdotool','type','--clearmodifiers','--delay','30','2')
            driver.capture(dest/'scientific-selected.png')
        driver.click(960,595);driver.pause(1);driver.move(1300,825);driver.capture(dest/'iteration-01.png')
        result=driver.capture_all_iterations('m_method',dest);result.update(problem=p,m=value,review='pending direct screenshot review')
        result['sha256']={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in dest.iterdir() if f.suffix in ['.png','.txt']}
        (dest/'captured.json').write_text(json.dumps(result,indent=2)+'\n')
        print('New M gap captured:',name,flush=True);driver.recover_main_menu()
