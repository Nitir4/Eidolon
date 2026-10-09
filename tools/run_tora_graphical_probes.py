"""Launch-first focused graphical UI observations; never a solver oracle."""
import hashlib,json
from pathlib import Path
import run_tora_corpus as oracle
from run_tora_file_probes import windows,save_as
from run_tora_two_phase_probes import fixture
ROOT=oracle.ROOT/'reference/tora-captures/probes/phase2-graphical'
def problems():
    seed=next(r['problem'] for r in json.loads((oracle.ROOT/'tests/fixtures/all-slack.json').read_text()) if r['case']=='01_normal')
    names=['graph-manual','graph-manual-stable','graph-fraction','graph-tiny','graph-upper','graph-lower','graph-free','graph-one-variable','graph-zero-objective','graph-all-zero','graph-large','graph-scientific','graph-alias','graph-fraction-high','graph-20','graph-100','graph-15','graph-25','graph-50','graph-200','graph-lower-stable']
    calibration=[('graph-19','19'),('graph-30','30'),('graph-40','40'),('graph-60','60'),('graph-80','80'),('graph-fraction-half','3.5')]
    names.extend(name for name,_ in calibration)
    cases={name:json.loads(json.dumps(seed)) for name in names}
    for name,value in [('graph-fraction','2.5'),('graph-tiny','0.2'),('graph-large','1000'),('graph-fraction-high','2.75'),('graph-20','20'),('graph-100','100'),('graph-15','15'),('graph-25','25'),('graph-50','50'),('graph-200','200')]:
        for row in cases[name]['constraints']:row['rhs']=value
    for name,value in calibration:
        for row in cases[name]['constraints']:row['rhs']=value
    cases['graph-upper']['variables'][0]['upper']='1'
    cases['graph-lower']['variables'][0]['lower']='1'
    cases['graph-lower-stable']['variables'][0]['lower']='1'
    cases['graph-free']['variables'][0].update(lower='-inf',upper='inf',unrestricted=True)
    cases['graph-one-variable']['variables']=cases['graph-one-variable']['variables'][:1]
    cases['graph-one-variable']['objective']=cases['graph-one-variable']['objective'][:1]
    cases['graph-one-variable']['constraints']=cases['graph-one-variable']['constraints'][:2]
    for row in cases['graph-one-variable']['constraints']:row['coefficients']=row['coefficients'][:1]
    cases['graph-zero-objective']['objective']=['0','0']
    for row in cases['graph-all-zero']['constraints']:row['rhs']='0'
    cases['graph-alias']['variables'][0]['name']='chairs'
    for name,p in cases.items():p['title']=name
    return cases

def run(driver,selected=None):
    ROOT.mkdir(parents=True,exist_ok=True);oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(exist_ok=True)
    for name in selected or problems():
        p=problems()[name];dest=ROOT/name;dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():print('Already captured:',name,flush=True);continue
        source=dest/'source.txt';source.write_bytes(fixture(p))
        driver.recover_main_menu();driver.load_case_from_main(windows(source));driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists():save_as(driver,dest/'tora-saved.txt',dest)
        driver.click(844,812);driver.click(957,585);driver.click(844,812);driver.hover(946,480);driver.click(1075,479);driver.pause(3)
        driver.capture(dest/'after-selection.png');eligible=driver._is_output_format_screen(dest/'after-selection.png')
        if eligible:
            if name=='graph-scientific':driver.click(858,502)
            driver.capture(dest/'output-format.png');driver.click(960,595);driver.pause(1);driver.capture(dest/'initial.png')
            if name in ['graph-manual','graph-manual-stable']:
                for label,point in [('objective-first',(690,446)),('constraint-1',(690,498)),('constraint-1-repeat',(690,498)),('constraint-2',(690,511)),('constraint-3',(690,525)),('objective-last',(690,446)),('sign-row',(690,550))]:
                    driver.click(*point);driver.move(1300,825);driver.pause(2);driver.capture(dest/(label+'.png'))
                    if driver.ui('dialog')==0:driver.key('Return');driver.capture(dest/(label+'-ack.png'))
            driver.click(704,570);driver.move(1300,825);driver.pause(4);driver.capture(dest/'one-stroke.png')
            driver.click(964,358);driver.move(1300,825);driver.pause(2);driver.capture(dest/'zoom-in.png')
            driver.click(1070,358);driver.move(1300,825);driver.pause(2);driver.capture(dest/'zoom-out.png')
            if name=='graph-manual':
                driver.click(1180,358);driver.capture(dest/'print-dialog.png');driver.key('Escape')
        result={'problem':p,'reached_output_format':eligible,'review':'pending direct visual review','sha256':{f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in dest.iterdir() if f.suffix in ['.png','.txt']}}
        (dest/'captured.json').write_text(json.dumps(result,indent=2)+'\n');print('New graphical capture:',name,'format',eligible,flush=True)
        driver.key('Escape');driver.recover_main_menu()
