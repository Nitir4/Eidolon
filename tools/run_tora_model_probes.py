#!/usr/bin/env python3
"""Focused Foundation 3 observations; reuse TORA and skip completed probes."""
from __future__ import annotations
import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path
import run_tora_corpus as oracle
from run_tora_numeric_probes import launch_visible

ROOT = oracle.ROOT / 'reference/tora-captures/probes/foundation3'

class ModelProbeDriver(oracle.ToraDriver):
    def _arrange_wine_desktop(self):
        # Address each operation to Wine itself. A concurrently opening app
        # must not receive an implicit resize/move of the active window.
        clients=json.loads(subprocess.check_output(['hyprctl','clients','-j'],env=self.env,text=True))
        desktop=next(c for c in clients if c.get('class')=='explorer.exe' and 'ToraOracle800' in c.get('title',''))
        target='address:'+desktop['address']
        for prop in ['alpha','alphainactive']:
            self.run('hyprctl','dispatch','setprop',target,prop,'1','override')
        self.run('hyprctl','dispatch','setprop',target,'opaque','on')
        self.run('hyprctl','dispatch','setfloating',target)
        if not desktop.get('pinned'):self.run('hyprctl','dispatch','pin',target)
        self.run('hyprctl','dispatch','resizewindowpixel','exact 808 608,'+target)
        self.run('hyprctl','dispatch','movewindowpixel','exact 556 252,'+target)
        self.run('hyprctl','dispatch','focuswindow',target)
        self.pause(1)
DIMENSIONS = {'zero-variables':('0','1'), 'zero-constraints':('1','0'),
              'zero-constraints-two':('2','0'),
              'zero-both':('0','0'), 'negative-variables':('-1','1'),
              'fractional-variables':('1.5','1'), 'nonnumeric-variables':('bogus','1')}
MODELS = {'equal-bounds':(['1','0'],['1','infinity'],['n','n']),
          'reversed-bounds':(['3','0'],['1','infinity'],['n','n']),
          'negative-lower':(['-1','0'],['infinity','infinity'],['n','n']),
          'negative-upper':(['-3','0'],['-1','infinity'],['n','n']),
          'free-default-bounds':(['0','0'],['infinity','infinity'],['y','n']),
          'free-finite-bounds':(['3','0'],['1','infinity'],['y','n'])}

def model_file(name, lower, upper, flags):
    q=lambda x:'"'+x+'"'
    lines=['3000','5','2','1',q('Foundation 3 '+name),'2','3',q('Maximize')]
    lines += list(map(q,['','','','', '3','2','','']))
    for row in [['1','1','        <=','4'],['1','0','        <=','2'],['0','1','        <=','3'],lower+['',''],upper+['',''],flags+['','']]: lines += list(map(q,row))
    return ('\r\n'.join(lines)+'\r\n').encode('ascii')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execute',action='store_true')
    parser.add_argument('--kind',choices=['dimensions','bounds'],required=True)
    parser.add_argument('--cases',nargs='*')
    parser.add_argument('--capture-current',action='store_true')
    args=parser.parse_args()
    cases=DIMENSIONS if args.kind=='dimensions' else MODELS
    if args.cases and set(args.cases)-set(cases): parser.error('Unknown probe')
    selected=[(k,v) for k,v in cases.items() if not args.cases or k in args.cases]
    if not args.execute: print('Focused probes:',[k for k,_ in selected]);return
    oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(parents=True,exist_ok=True)
    driver=ModelProbeDriver(.2,.8)
    if args.capture_current:
        if not driver._attach_to_running_window(): raise RuntimeError('No reference window')
        driver.capture(oracle.OUTPUT/'current-window.png');return
    launch_visible(driver)
    for name,values in selected:
        dest=ROOT/args.kind/name
        if (dest/'captured.json').exists(): print('Already captured:',name,flush=True);continue
        dest.mkdir(parents=True,exist_ok=True)
        driver.recover_main_menu()
        if args.kind=='dimensions':
            driver.click(946,534);driver.click(858,426);driver.click(955,635)
            driver.capture(dest/'blank-input.png')
            for x,y,value in [(800,328,'F3 '+name),(740,352,values[0]),(740,376,values[1])]:
                driver.click(x,y);driver.key('ctrl+a');driver.run('xdotool','type','--clearmodifiers','--delay','20','--',value)
            driver.capture(dest/'entered.png');driver.key('Return');driver.pause(2)
            driver.capture(dest/'after-initialize.png')
            # Capture any modal without guessing its text; acknowledge a normal
            # validation warning and preserve the resulting input screen.
            screen=dest/'after-initialize.png'
            if driver._looks_gray(driver._png_pixel(screen,400,300)):
                driver.key('Return');driver.pause(.8);driver.capture(dest/'after-acknowledge.png')
            elif values[1]=='0':
                # TORA advertises both Tab and Return as initialization keys.
                # Observe both before classifying an apparently blank form.
                driver.click(740,376);driver.key('Tab');driver.pause(2)
                driver.capture(dest/'after-tab-initialize.png')
        else:
            source=dest/'source.txt';source.write_bytes(model_file(name,*values))
            installed=oracle.INSTALL/('f3-'+name+'.txt');shutil.copyfile(source,installed)
            driver.load_case_from_main('Z:'+str(installed).replace('/','\\'))
            driver.capture(dest/'input.png')
            saved=dest/'tora-saved.txt'
            if not saved.exists():
                driver.click(571,290);driver.click(590,326)
                driver.capture(dest/'save-as-dialog.png')
                driver.click(750,472);driver.key('ctrl+a')
                driver.run('xdotool','type','--clearmodifiers','--delay','5','Z:'+str(saved).replace('/','\\'))
                driver.click(932,471);driver.pause(2)
                driver.capture(dest/'input-after-save.png')
                if not saved.exists():raise RuntimeError('Native Save As failed; inspect '+name)
            driver.click(844,812);driver.pause(.8);driver.capture(dest/'after-solve-menu.png')
            # The existing save prompt is dismissed through its No button.
            driver.click(957,585);driver.pause(.8);driver.capture(dest/'after-save-no.png')
            driver.click(844,812);driver.pause(.8);driver.capture(dest/'solve-selection.png')
            driver.hover(946,480);driver.hover(1075,496);driver.hover(1140,515);driver.click(1000,515)
            driver.pause(2);driver.capture(dest/'method-validation.png')
            if driver._looks_gray(driver._png_pixel(dest/'method-validation.png',400,300)):
                driver.key('Return');driver.pause(.8);driver.capture(dest/'method-after-acknowledge.png')
        (dest/'captured.json').write_text(json.dumps({'case':name,'kind':args.kind,'entered':values,
            'review':'pending direct screenshot review',
            'sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in dest.iterdir() if p.suffix in ['.png','.txt']}},indent=2)+'\n')
        print('Captured:',name,flush=True)
        if args.kind=='dimensions':
            # Clear invalid/zero counts so recovery can initialize a scratch
            # grid without reusing an unverified size on the next probe.
            driver.key('Escape')
            for x,y,value in [(740,352,'2'),(740,376,'3')]:
                driver.click(x,y);driver.key('ctrl+a');driver.run('xdotool','type','--clearmodifiers','--delay','20',value)
            driver.key('Return');driver.pause(.8)
        driver.recover_main_menu()

if __name__=='__main__':main()
