"""Focused all-slack gaps only; run via the launch-first isolated observer.

Never repeats the completed corpus, closes TORA, or derives golden numbers.
Success markers skip completed probes; all images require direct review.
"""
from pathlib import Path
import hashlib,json
import run_tora_corpus as oracle
from run_tora_file_probes import windows,save_as

ROOT=oracle.ROOT/'reference/tora-captures/probes/phase2-all-slack'

def fixture(name,sense,objective,rows,upper='infinity',unrestricted=False):
    lines=['3000','5','2','1','"'+name+'"','2','3','"'+sense+'"']
    cells=['','','','']+list(map(str,objective))+['','']
    for a,b,rhs in rows:cells += [str(a),str(b),'        <=',str(rhs)]
    cells += ['-infinity' if unrestricted else '0','0','','',upper,'infinity','','','y' if unrestricted else 'n','n','','']
    lines += ['"'+x+'"' for x in cells]
    return ('\r\n'.join(lines)+'\r\n').encode('ascii')

def run(driver,selected=None):
    ROOT.mkdir(parents=True,exist_ok=True);oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(exist_ok=True)
    cases={
        'slack-minimum':('Minimize',[-3,-2],[(1,1,4),(1,0,2),(0,1,3)]),
        'slack-unbounded':('Maximize',[1,0],[(-1,1,1),(0,1,3),(0,1,2)]),
        'slack-row-tie':('Maximize',[2,1],[(1,1,2),(1,0,2),(0,1,5)]),
        'slack-near-tie':('Maximize',[3,3.0000000001],[(1,1,4),(1,0,3),(0,1,3)]),
        'slack-complement-return':('Maximize',[4,3],[(2,1,4),(1,0,5),(0,1,6)]),
        'slack-unrestricted':('Maximize',[3,2],[(1,1,4),(1,0,2),(0,1,3)]),
        'slack-unrestricted-negative':('Maximize',[-3,2],[(-1,1,4),(-1,0,2),(0,1,3)]),
        'slack-unrestricted-crossing':('Maximize',[3,2],[(1,.5,2),(-1,1,5),(0,1,10)]),
        'slack-unrestricted-crossing-precision':('Maximize',[3,2],[(1,.5,2),(-1,1,5),(0,1,10)]),
        'slack-unrestricted-crossing-scientific':('Maximize',[3,2],[(1,.5,2),(-1,1,5),(0,1,10)]),
    }
    names=selected or list(cases)
    for name in names:
        if name not in cases:raise ValueError('Unknown focused all-slack case: '+name)
        dest=ROOT/name;dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():print('Already captured:',name,flush=True);continue
        sense,objective,rows=cases[name];source=dest/'source.txt';source.write_bytes(fixture(name,sense,objective,rows,
            upper='1' if name=='slack-complement-return' else 'infinity',unrestricted=name.startswith('slack-unrestricted')))
        driver.recover_main_menu();driver.load_case_from_main(windows(source));driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists():save_as(driver,dest/'tora-saved.txt',dest)
        driver.open_method('all_slack');driver.capture(dest/'output-format.png')
        if name.endswith('-scientific'):
            driver.click(858,502);driver.capture(dest/'format-selected.png')
        if name.endswith('-precision'):
            driver.click(952,541);driver.key('ctrl+a')
            driver.run('xdotool','type','--clearmodifiers','--delay','20','12')
            driver.capture(dest/'format-selected.png')
        driver.click(960,595);driver.pause(1);driver.capture(dest/'iteration-01.png')
        if name=='slack-minimum':
            # Determine precisely when Next appends and when it shows notice.
            for i in range(1,4):
                driver.click(844,432);driver.pause(.5);driver.capture(dest/f'next-{i}.png')
                if driver.ui('dialog')==0:
                    (dest/'next-notice.json').write_text(json.dumps({'click':i,'review':'pending'})+'\n')
                    driver.key('Return');break
        result=driver.capture_all_iterations('all_slack',dest)
        result['review']='pending direct screenshot review'
        result['sha256']={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in dest.iterdir() if p.suffix in ['.png','.txt']}
        (dest/'captured.json').write_text(json.dumps(result,indent=2)+'\n')
        print('New all-slack gap captured:',name,flush=True)
        driver.recover_main_menu()
