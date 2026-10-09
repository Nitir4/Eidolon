"""Launch-first final-summary gap observations; original corpus stays intact."""
import argparse
import hashlib
import json
from pathlib import Path
import run_tora_corpus as oracle
from run_tora_file_probes import IsolatedFileDriver, windows, save_as
from run_tora_two_phase_probes import fixture

ROOT = oracle.ROOT / 'reference/tora-captures/probes/phase2-final'

def problems():
    records = json.loads((oracle.ROOT/'tests/fixtures/bounded.json').read_text())
    def source(case):
        return json.loads(json.dumps(next(r['problem'] for r in records if r['case']==case)))
    cases = {name:source('01_normal') for name in
             ['final-upper', 'final-lower', 'final-free', 'final-fixed', 'final-zero-objective', 'final-alias']}
    cases['final-upper']['variables'][0]['upper']='1'
    cases['final-lower']['variables'][0]['lower']='1'
    cases['final-free']['variables'][0].update(lower='-inf', upper='inf', unrestricted=True)
    cases['final-fixed']['variables'][0].update(lower='1',upper='1')
    cases['final-zero-objective']['objective']=['0','0']
    cases['final-alias']['variables'][0]['name']='chairs'
    cases['final-redundant']=source('06_equality')
    cases['final-redundant']['constraints'][1]={'relation':'=', 'rhs':'8', 'coefficients':['2','2']}
    cases['final-free-negative']=source('13_mixed_sign')
    cases['final-free-negative']['variables'][1].update(lower='-inf',upper='inf',unrestricted=True)
    cases['final-scientific']=source('10_dual')
    cases['final-decimal-three']=source('13_mixed_sign')
    # Additional sensitivity-only gaps; resumable markers preserve prior work.
    for name in ['sens-lower-active','sens-negative-lower','sens-basic-upper','sens-both-bounds','sens-negative-free-optimum','sens-negative-rhs-active']:
        cases[name]=source('01_normal')
    cases['sens-lower-active']['objective']=['1','3']
    cases['sens-lower-active']['variables'][0]['lower']='1'
    cases['sens-negative-lower']['objective']=['-3','2']
    cases['sens-negative-lower']['variables'][0]['lower']='-1'
    cases['sens-basic-upper']['variables'][0]['upper']='3'
    for v in cases['sens-both-bounds']['variables']:v.update(lower='1',upper='2')
    cases['sens-negative-free-optimum']['objective']=['-3','2']
    cases['sens-negative-free-optimum']['variables'][0].update(lower='-inf',upper='inf',unrestricted=True)
    cases['sens-negative-free-optimum']['constraints'][1].update(relation='>=',rhs='-2')
    cases['sens-negative-rhs-active']['objective']=['-3','2']
    cases['sens-negative-rhs-active']['constraints'][1].update(relation='<=',rhs='-1',coefficients=['-1','0'])
    cases['sens-negative-lower-positive-cost']=source('01_normal')
    cases['sens-negative-lower-positive-cost']['variables'][0]['lower']='-1'
    cases['sens-min-reduced']=source('01_normal')
    cases['sens-min-reduced'].update(sense='min',objective=['1','3'])
    cases['sens-min-reduced']['constraints'][0].update(relation='>=',rhs='1',coefficients=['1','0'])
    cases['sens-min-upper']=json.loads(json.dumps(cases['sens-min-reduced']))
    cases['sens-min-upper']['objective']=['1','-3']
    cases['sens-min-upper']['variables'][1]['upper']='1'
    cases['sens-negative-lower-upper']=source('01_normal')
    cases['sens-negative-lower-upper']['variables'][0].update(lower='-1',upper='1')
    for name,p in cases.items():p['title']=name
    return cases

def run(driver, selected):
    for name in selected or problems():
        dest=ROOT/name;dest.mkdir(parents=True,exist_ok=True)
        if (dest/'captured.json').exists():
            print('Already captured:',name,flush=True);continue
        p=problems()[name];source=dest/'source.txt';source.write_bytes(fixture(p))
        driver.recover_main_menu();driver.load_case_from_main(windows(source));driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists():save_as(driver,dest/'tora-saved.txt',dest)
        driver.click(844,812);driver.click(957,585);driver.click(844,812)
        driver.hover(946,480);driver.hover(1075,496);driver.click(1130,498)
        driver.capture(dest/'output-format.png')
        if not driver._is_output_format_screen(dest/'output-format.png'):
            raise RuntimeError('Final solution did not reach output format: '+name)
        fmt=[0,5,2]
        if name=='final-scientific':driver.click(858,502);fmt[0]=1
        if name=='final-decimal-three':
            driver.click(952,541);driver.key('ctrl+a');driver.run('xdotool','type','--clearmodifiers','3');fmt[2]=3
        driver.capture(dest/'format-selected.png');driver.click(960,595);driver.pause(1)
        driver.move(1300,825);driver.capture(dest/'summary.png')
        if driver.ui('dialog')==0:driver.key('Return');driver.capture(dest/'summary-ack.png')
        (dest/'captured.json').write_text(json.dumps({'problem':p,'format':fmt,'review':'pending visual/glyph review',
            'sha256':{f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in dest.iterdir() if f.suffix in ['.png','.txt']}},indent=2)+'\n')
        print('Captured final summary:',name,flush=True);driver.recover_main_menu()

def scroll(driver, selected):
    for name in selected or problems():
        dest=ROOT/name
        if (dest/'summary-bottom.png').exists():continue
        p=problems()[name]
        if not any((not v['unrestricted']) and (float(v['lower'])>0 or v['upper']!='inf') for v in p['variables']):continue
        driver.recover_main_menu();driver.load_case_from_main(windows(dest/'tora-saved.txt'))
        driver.click(844,812);driver.click(957,585);driver.click(844,812)
        driver.hover(946,480);driver.hover(1075,496);driver.click(1130,498);driver.click(960,595)
        driver.pause(.7);driver.move(1291,729)
        driver.run('xdotool','click','--repeat','35','--delay','35','1');driver.move(1300,825)
        driver.capture(dest/'summary-bottom.png');driver.recover_main_menu()
        print('Captured clipped bound rows:',name,flush=True)

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execute',action='store_true',required=True)
    parser.add_argument('--cases',nargs='*')
    parser.add_argument('--scroll',action='store_true')
    args=parser.parse_args()
    oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(parents=True,exist_ok=True)
    driver=IsolatedFileDriver(.12,.7)
    driver.launch_tora()
    run(driver,args.cases)
    if args.scroll:scroll(driver,args.cases)
