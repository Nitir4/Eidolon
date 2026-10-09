#!/usr/bin/env python3
"""Resumable Foundation 4 file observations; launch/reuse TORA first."""
from __future__ import annotations
import argparse
import hashlib
import json
import shutil
import subprocess
import os
import signal
import time
import sys
from pathlib import Path
import run_tora_corpus as oracle
from run_tora_numeric_probes import launch_visible
from run_tora_model_probes import ModelProbeDriver

ROOT = oracle.ROOT / 'reference/tora-captures/probes/foundation4'

class FileProbeDriver(ModelProbeDriver):
    def ui(self,*command):
        return subprocess.run([str(oracle.ROOT/'build/foundation4/tora-ui.exe'),*command],env=self.env,stdout=subprocess.DEVNULL).returncode
    def recover_main_menu(self):
        if not self._attach_to_running_window():raise RuntimeError('No reference window')
        if self.ui('dialog')==0:
            warning=subprocess.run(['xdotool','search','--onlyvisible','--name',r'^Input Data Error$|^Problem [Ss]ize$'],env=self.env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            if warning.returncode==0 and self.ui('button','OK')==0:self.pause(.6)
        if self.ui('button','Cancel')==0:self.pause(.6)
        if self.ui('button','No')==0:self.pause(.6)
        # VB command buttons need real pointer input. BM_CLICK is suitable for
        # the native picker but did not reliably return from VB form handlers.
        super().recover_main_menu()
    @staticmethod
    def _png_pixel(path,px,py):
        # Read UI pixels natively; avoid decoding the entire PNG in Python for
        # every individual recovery sentinel.
        fmt=','.join(f'%[fx:round(255*p{{{px},{py}}}.{channel})]' for channel in 'rgb')
        value=subprocess.check_output(['magick',str(path),'-format',fmt,'info:'],text=True)
        return tuple(map(int,value.split(',')))
    def __init__(self,*args):
        super().__init__(*args)
        self.env['LIBGL_ALWAYS_SOFTWARE']='1'
    def _arrange_wine_desktop(self):
        super()._arrange_wine_desktop()
        self.run(str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'arrange')
    def load_case_from_main(self,basename):
        self.click(946,534);self.pause(1)
        self.capture(oracle.OUTPUT/'input-mode.png')
        self.click(858,448);self.pause(.4)
        if self.ui('dialog')!=0:self.click(955,635)
        self.pause(1)
        self.capture(oracle.OUTPUT/'open-dialog.png')
        self.click(750,472);type_text(self,basename)
        self.run(str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'readonly','false')
        self.capture(oracle.OUTPUT/'filename-entered.png')
        if self.ui('button','Open')!=0:raise RuntimeError('Native Open button not found')
        self.pause(1)
        if not self._attach_to_running_window(focus=False):raise RuntimeError('TORA disappeared on open')
        title=subprocess.check_output(['xdotool','getwindowname',self.window_id],env=self.env,text=True)
        if basename.lower() not in title.lower():raise RuntimeError('TORA did not open the requested path: '+title)
    def _attach_to_running_window(self,focus=True):
        if self.ui('present')!=0:return False
        attached=super()._attach_to_running_window(focus)
        clients=json.loads(subprocess.check_output(['hyprctl','clients','-j'],env=self.env,text=True))
        parent=next((c for c in clients if c.get('class')=='explorer.exe' and 'ToraOracle800' in c.get('title','')),None)
        if parent is None:return False
        self.window_origin=tuple(parent['at'])
        if attached:return True
        # TORA's MAIN form can maximize to the host X11 screen after a reboot,
        # while rendering inside Wine's 800x600 desktop. Validate that form
        # explicitly, then capture the visible Wine viewport rather than an
        # unrelated app or an offscreen 1920-wide image.
        ids=subprocess.run(['xdotool','search','--onlyvisible','--name','^TORA'],env=self.env,text=True,stdout=subprocess.PIPE).stdout.splitlines()
        if not ids:return False
        # Newly created MAIN forms can inherit host-screen geometry outside
        # the virtual desktop. Restore the actual TORA form before capture.
        self.ui('arrange')
        if super()._attach_to_running_window(focus):
            self.window_origin=tuple(parent['at']);return True
        if parent['size']!=[808,608]:raise RuntimeError('Unexpected Wine viewport')
        desktop=subprocess.check_output(['xdotool','search','--onlyvisible','--name','ToraOracle800 - Wine Desktop'],env=self.env,text=True).splitlines()[-1]
        self.window_id=desktop;self.window_origin=tuple(parent['at'])
        if focus:self.run('hyprctl','dispatch','focuswindow','address:'+parent['address'])
        return True
    def capture(self,path):
        if not self._attach_to_running_window(focus=False):raise RuntimeError('No TORA reference window')
        clients=json.loads(subprocess.check_output(['hyprctl','clients','-j'],env=self.env,text=True))
        parent=next(c for c in clients if c.get('class')=='explorer.exe' and 'ToraOracle800' in c.get('title',''))
        x,y=parent['at'];w,h=parent['size']
        if [w,h]!=[808,608]:raise RuntimeError('Unexpected reference viewport')
        path.parent.mkdir(parents=True,exist_ok=True)
        self.run('grim','-g',f'{x},{y} {w}x{h}',str(path))
    def move(self,x,y):
        root_x=self.window_origin[0]+x-oracle.LEGACY_ORIGIN[0]
        root_y=self.window_origin[1]+y-oracle.LEGACY_ORIGIN[1]
        self.run('hyprctl','dispatch','movecursor',str(root_x),str(root_y))
        self.run('xdotool','mousemove','--sync',str(root_x),str(root_y))
        self.pause(.15)
    def click(self,x,y,count=1):
        # Send normal pointer input on Wine's desktop after checking its target.
        # The helper refuses other apps and Exit.
        result=self.ui('click',str(x-oracle.LEGACY_ORIGIN[0]),str(y-oracle.LEGACY_ORIGIN[1]),str(count))
        if result!=0:raise RuntimeError('Reference pointer target was not a TORA control: '+str(result))
        self.pause()
    def key(self,key):
        if self.ui('key',key)!=0:raise RuntimeError('Reference key event failed')
        self.pause()

class IsolatedFileDriver(FileProbeDriver):
    """Dedicated 800x600 X screen: no host focus/cursor/capture dependency."""
    capture_frame_offset=0
    def __init__(self,*args):
        super().__init__(*args)
        self.env['DISPLAY']=':97'
        prefix=oracle.ROOT/'.tora-file-wine'
        if not prefix.exists():
            print('Preparing private reference runtime copy',flush=True)
            shutil.copytree(oracle.ROOT/'.tora-oracle-wine',prefix,symlinks=True)
        self.env['WINEPREFIX']=str(prefix)
        if subprocess.run(['xdotool','getdisplaygeometry'],env=self.env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode:
            log=(oracle.OUTPUT/'isolated-display.log').open('a')
            server=subprocess.Popen(['Xwayland',':97','-geometry','800x600','-dpi','96','-nolisten','tcp','-noreset','-shm'],stdout=log,stderr=log,start_new_session=True)
            for _ in range(40):
                time.sleep(.25)
                clients=json.loads(subprocess.check_output(['hyprctl','clients','-j'],text=True))
                parent=next((c for c in clients if c.get('pid')==server.pid),None)
                if parent:
                    target='address:'+parent['address']
                    for args in [('setfloating',target),('resizewindowpixel','exact 800 600,'+target),('movewindowpixel','exact 556 252,'+target)]:
                        subprocess.run(['hyprctl','dispatch',*args],check=True,stdout=subprocess.DEVNULL)
                    break
            else:raise RuntimeError('Isolated X display did not appear')
        info=subprocess.check_output(['xdotool','getdisplaygeometry'],env=self.env,text=True).strip()
        if info!='800 600':raise RuntimeError('Reference X screen is not 800x600: '+info)
    def _arrange_wine_desktop(self):
        self.ui('arrange');self.pause(.5)
    def run(self,*args):
        if args[:2]==('xdotool','windowactivate'):
            args=('xdotool','windowfocus',args[-1])
        super().run(*args)
    def _attach_to_running_window(self,focus=True):
        return oracle.ToraDriver._attach_to_running_window(self,focus)
    def capture(self,path):
        if not self._attach_to_running_window(False):raise RuntimeError('No TORA reference form')
        path.parent.mkdir(parents=True,exist_ok=True)
        self.run('magick','import','-window','root',str(path))
    def move(self,x,y):
        oracle.ToraDriver.move(self,x,y)
    def click(self,x,y,count=1):
        oracle.ToraDriver.click(self,x,y,count)
    def key(self,key):
        oracle.ToraDriver.key(self,key)

def windows(path): return 'Z:' + str(path.resolve()).replace('/', '\\')
def type_text(driver, text):
    helper=oracle.ROOT/'build/foundation4/tora-ui.exe'
    if not helper.is_file():raise RuntimeError('Build the reference UI helper first')
    driver.run(str(helper),'text',text)

def replace_text(driver, text):
    driver.key('Home');driver.key('shift+End');driver.key('BackSpace');type_text(driver,text)

def save_as(driver, path, dest):
    driver.key('Escape');driver.click(1300,740);driver.pause(.3)
    if driver.ui('menu','Save As')!=0:
        # This VB form draws its menu itself (GetMenu is null). Use the
        # directly captured File -> Save As positions, then verify the picker.
        driver.click(571,290);driver.pause(.3);driver.click(590,326)
    driver.pause(.4)
    if driver.ui('dialog')!=0:
        driver.key('Return');driver.pause(.6)
    if driver.ui('dialog')!=0:raise RuntimeError('Save As did not open its native picker; no text entered')
    driver.capture(dest/'save-dialog.png')
    driver.click(750,472);driver.key('ctrl+a');type_text(driver,windows(path))
    driver.capture(dest/'save-filename.png')
    if driver.ui('button','Save')!=0:raise RuntimeError('Native Save button not found')
    driver.pause(1)
    if driver.ui('dialog')==0:
        driver.capture(dest/'overwrite-confirmation.png')
        if driver.ui('button','Yes')!=0:raise RuntimeError('Save picker did not close and no overwrite confirmation was available')
        driver.pause(.8)
    if driver.ui('dialog')==0:raise RuntimeError('Save did not finish; reference dialog remains open')
    driver.capture(dest/'after-save.png')
    if not path.is_file():
        suffixed=path.with_name(path.name+'.txt')
        if suffixed.is_file():
            print('Native picker added .txt:',suffixed.name,flush=True)
        else:raise RuntimeError('Save As did not write '+str(path))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execute',action='store_true')
    parser.add_argument('--kind',choices=['text','comma-title','quoted-title','unicode-title','quoted-name','extension','overwrite','readonly','readonly-save','save-target','clone','input','simplex','m-method','two-phase','bounded','dual'],required=True)
    parser.add_argument('--input-probes',nargs='*')
    parser.add_argument('--capture-current',action='store_true')
    parser.add_argument('--capture-desktop',action='store_true')
    parser.add_argument('--stop-at-open',action='store_true')
    parser.add_argument('--repair-editor',action='store_true')
    parser.add_argument('--menu-recovery',action='store_true')
    parser.add_argument('--resume-open',action='store_true')
    parser.add_argument('--focus-dump',action='store_true')
    parser.add_argument('--files',nargs='*',type=Path)
    parser.add_argument('--stop-automation',action='store_true')
    parser.add_argument('--isolated',action='store_true')
    parser.add_argument('--batch',action='store_true',help='Run the remaining focused text/picker probes sequentially')
    parser.add_argument('--finish-save-target',action='store_true')
    args=parser.parse_args()
    if not args.execute: print('Observation:',args.kind);return
    if args.batch:
        for kind in ['comma-title','quoted-title','unicode-title','quoted-name','extension','overwrite']:
            command=[sys.executable,'-u',str(Path(__file__).resolve()),'--execute','--kind',kind]
            if args.isolated:command.append('--isolated')
            subprocess.run(command,check=True)
        return
    if args.stop_automation:
        # Stop only this observer, never Wine/TORA. Unlike terminal Ctrl+C,
        # SIGTERM to the Python PID does not broadcast a Windows console event.
        for entry in Path('/proc').iterdir():
            if not entry.name.isdigit() or int(entry.name)==os.getpid():continue
            try:command=(entry/'cmdline').read_bytes().split(b'\0')
            except (FileNotFoundError,PermissionError):continue
            if len(command)>2 and command[0].endswith(b'python3') and command[1:3]==[b'-u',b'tools/run_tora_file_probes.py'] and b'--stop-automation' not in command:
                os.kill(int(entry.name),signal.SIGTERM);print('Stopped observer PID',entry.name)
        return
    oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(parents=True,exist_ok=True)
    driver=(IsolatedFileDriver if args.isolated else FileProbeDriver)(.25,1)
    if args.focus_dump:
        subprocess.run([str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'windows'],env=driver.env)
        subprocess.run([str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'menus'],env=driver.env)
        subprocess.run([str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'dump'],env=driver.env);return
    if args.capture_desktop:
        clients=json.loads(subprocess.check_output(['hyprctl','clients','-j'],env=driver.env,text=True))
        parent=next(c for c in clients if c.get('class')=='explorer.exe' and 'ToraOracle800' in c.get('title',''))
        x,y=parent['at'];w,h=parent['size']
        ids=subprocess.run(['xdotool','search','--onlyvisible','--name','TORA'],env=driver.env,text=True,stdout=subprocess.PIPE).stdout.splitlines()
        windows_found=[]
        for wid in ids:
            windows_found.append({'id':wid,'name':subprocess.check_output(['xdotool','getwindowname',wid],env=driver.env,text=True),'geometry':subprocess.check_output(['xdotool','getwindowgeometry','--shell',wid],env=driver.env,text=True)})
        (oracle.OUTPUT/'desktop-windows.json').write_text(json.dumps(windows_found,indent=2)+'\n')
        driver.run('grim','-g',f'{x},{y} {w}x{h}',str(oracle.OUTPUT/'wine-desktop.png'));return
    if args.menu_recovery:
        if not driver._attach_to_running_window():raise RuntimeError('No reference window')
        driver._arrange_wine_desktop();driver.key('Escape')
        driver.click(571,290);driver.click(590,342);driver.pause(1)
        driver.capture(oracle.OUTPUT/'menu-recovery.png')
        driver.click(957,585);driver.pause(1)
        driver.capture(oracle.OUTPUT/'menu-recovery-after-no.png');return
    if args.repair_editor:
        if not driver._attach_to_running_window():raise RuntimeError('No reference window')
        driver._arrange_wine_desktop()
        driver.key('Escape')
        driver.click(800,328);replace_text(driver,'F4 safe');driver.key('Tab')
        driver.click(875,467,count=2);replace_text(driver,'decision');driver.key('Tab')
        driver.click(1300,740);driver.key('Escape')
        driver.capture(oracle.OUTPUT/'editor-repaired.png')
        driver.click(571,290);driver.capture(oracle.OUTPUT/'editor-repaired-menu.png');driver.key('Escape')
        return
    if args.capture_current:
        if not driver._attach_to_running_window():raise RuntimeError('No reference window')
        diagnostics={}
        for name,cmd in [('monitors',['hyprctl','monitors','-j']),('clients',['hyprctl','clients','-j']),('pointer',['xdotool','getmouselocation','--shell']),('native-pointer',['hyprctl','cursorpos','-j'])]:
            diagnostics[name]=subprocess.check_output(cmd,env=driver.env,text=True)
        (oracle.OUTPUT/'diagnostics.json').write_text(json.dumps(diagnostics,indent=2)+'\n')
        driver.capture(oracle.OUTPUT/'current-window.png');return
    if args.resume_open:
        if not driver._attach_to_running_window():raise RuntimeError('No reference window')
        driver._arrange_wine_desktop()
    else:launch_visible(driver)
    if args.kind=='dual':
        from run_tora_dual_probes import run
        run(driver,args.input_probes)
        return
    if args.kind=='bounded':
        from run_tora_bounded_probes import run
        run(driver,args.input_probes)
        return
    if args.kind=='two-phase':
        from run_tora_two_phase_probes import run
        run(driver,args.input_probes)
        return
    if args.kind=='m-method':
        from run_tora_m_probes import run
        run(driver,args.input_probes)
        return
    if args.kind=='simplex':
        from run_tora_simplex_probes import run
        run(driver,args.input_probes)
        return
    if args.kind=='input':
        from run_tora_input_probes import run
        run(driver,args.input_probes)
        return
    if args.kind=='clone':
        if not args.files:parser.error('Provide clone-written files')
        cases=[(p.stem,p.resolve()) for p in args.files]
    else:
        source=ROOT/'source-normal.txt';source.parent.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(oracle.ROOT/'reference/tora-captures/roundtrips/01_normal/tora-saved.txt',source)
        cases=[(args.kind,source)]
    for name,source in cases:
        dest=ROOT/args.kind/name;dest.mkdir(parents=True,exist_ok=True)
        if (dest/'captured.json').exists() and not args.resume_open and not args.finish_save_target:print('Already captured:',name,flush=True);continue
        if not args.resume_open:driver.recover_main_menu()
        if args.kind in ['readonly','readonly-save']:
            if not args.resume_open:driver.click(946,534);driver.click(858,448);driver.click(955,635)
            driver.capture(dest/'open-dialog.png')
            if args.stop_at_open:return
            driver.click(750,472);driver.key('ctrl+a');type_text(driver,windows(source))
            driver.run(str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'readonly','true');driver.capture(dest/'readonly-selected.png')
            driver.click(932,471);driver.pause(2)
        else:driver.load_case_from_main(windows(source))
        driver.capture(dest/'input.png')
        if args.kind=='text':
            driver.click(800,328);replace_text(driver,'F4, "quoted" cafe');driver.key('Tab')
            driver.click(740,376);driver.key('Return');driver.pause(1)
            driver.capture(dest/'title-reinitialized.png')
            driver.click(875,467,count=2);replace_text(driver,'named, "x"');driver.key('Return')
            driver.click(740,376);driver.key('Return');driver.pause(1)
            driver.capture(dest/'text-entered.png')
            save_as(driver,dest/'tora-saved.txt',dest)
            driver.recover_main_menu();driver.load_case_from_main(windows(dest/'tora-saved.txt'))
            driver.capture(dest/'reopened.png')
        elif args.kind in ['comma-title','quoted-title','unicode-title','quoted-name']:
            if args.kind=='quoted-name':
                driver.click(875,467,count=2);driver.key('ctrl+a')
                driver.run('xdotool','type','--clearmodifiers','--delay','80','--','named, "x"')
                driver.capture(dest/'alias-before-commit.png');driver.key('Return');driver.pause(.6)
                driver.click(800,328);driver.key('Tab')
            else:
                text={'comma-title':'F4, comma','quoted-title':'F4 "quoted"','unicode-title':'F4 cafe é'}[args.kind]
                driver.click(800,328);replace_text(driver,text);driver.key('Tab')
                driver.click(740,376);driver.key('Return');driver.pause(1)
            driver.capture(dest/'text-entered.png')
            save_as(driver,dest/'tora-saved.txt',dest)
            driver.recover_main_menu();driver.load_case_from_main(windows(dest/'tora-saved.txt'))
            driver.capture(dest/'reopened.png')
        elif args.kind=='extension':
            save_as(driver,dest/'without-suffix',dest)
        elif args.kind in ['readonly','readonly-save']:
            driver.click(571,290);driver.capture(dest/'file-menu.png');driver.key('Escape')
            driver.click(800,328);driver.key('ctrl+a');type_text(driver,'Readonly edit probe');driver.key('Tab')
            driver.capture(dest/'after-edit.png')
            if args.kind=='readonly-save':
                before=source.read_bytes()
                driver.click(571,290);driver.click(590,309);driver.pause(2)
                driver.capture(dest/'after-save-command.png')
                (dest/'source-after-save.txt').write_bytes(source.read_bytes())
                print('Readonly Save changed source:',source.read_bytes()!=before,flush=True)
                save_as(driver,dest/'tora-saved.txt',dest)
        elif args.kind=='overwrite':
            target=dest/'tora-saved.txt';target.write_bytes(source.read_bytes())
            driver.click(800,328);driver.key('ctrl+a');type_text(driver,'F4 overwrite changed');driver.key('Tab')
            driver.click(571,290);driver.click(590,326);driver.pause(.8)
            driver.click(750,472);driver.key('ctrl+a');type_text(driver,windows(target));driver.click(932,471);driver.pause(1)
            driver.capture(dest/'overwrite-prompt.png');driver.key('n');driver.pause(1);driver.capture(dest/'after-no.png')
            driver.key('Escape');driver.pause(.8)
            save_as(driver,dest/'accepted-new.txt',dest)
        elif args.kind=='save-target':
            before=source.read_bytes();target=dest/'tora-saved.txt'
            save_as(driver,target,dest)
            driver.click(800,328);type_text(driver,'F4 after Save As');driver.key('Tab')
            driver.click(571,290);driver.click(590,309);driver.pause(1)
            driver.capture(dest/'after-save-command.png')
            if args.finish_save_target:
                if driver.ui('button','Save')!=0:raise RuntimeError('Subsequent Save picker absent')
                driver.pause(.8);driver.capture(dest/'subsequent-overwrite.png')
                if driver.ui('button','Yes')!=0:raise RuntimeError('Subsequent overwrite prompt absent')
                driver.pause(.8);driver.capture(dest/'after-confirmed-save.png')
            (dest/'source-after-save.txt').write_bytes(source.read_bytes())
            print('Subsequent Save changed original:',source.read_bytes()!=before,flush=True)
            print('Subsequent Save changed Save As target:',target.read_bytes()!=before,flush=True)
        elif args.kind=='clone':
            save_as(driver,dest/'tora-saved.txt',dest)
            if (dest/'tora-saved.txt').read_bytes()!=source.read_bytes():
                print('Byte differences need review:',name,flush=True)
            driver.recover_main_menu();driver.load_case_from_main(windows(dest/'tora-saved.txt'))
            driver.capture(dest/'reopened.png')
        (dest/'captured.json').write_text(json.dumps({'kind':args.kind,'case':name,'review':'pending direct review',
            'source':str(source.relative_to(oracle.ROOT)),
            'sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in dest.iterdir() if p.is_file() and p.suffix in ['.png','.txt']}},indent=2)+'\n')
        print('Captured:',name,flush=True);driver.recover_main_menu()

if __name__=='__main__':main()
