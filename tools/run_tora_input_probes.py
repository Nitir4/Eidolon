"""Focused Phase 2 input observations, launched by the approved file driver.

Reuses the actual reference, existing fixtures and isolated 800x600 display.
Never closes TORA or repeats the solver corpus. Captures require direct review.
"""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import run_tora_corpus as oracle
from run_tora_file_probes import type_text, windows, save_as

ROOT=oracle.ROOT/'reference/tora-captures/probes/phase2-input'

def run(driver, selected=None):
    driver.delay=.15
    ROOT.mkdir(parents=True,exist_ok=True)
    oracle.OUTPUT=ROOT/'runtime';oracle.OUTPUT.mkdir(exist_ok=True)
    source=ROOT/'source-normal.txt'
    shutil.copyfile(oracle.ROOT/'reference/tora-captures/roundtrips/01_normal/tora-saved.txt',source)
    cases=selected or ['numeric-replace-keyboard','objective-toggle','alias-keyboard','quoted-alias-keyboard',
        'relation-replace-keyboard','flag-replace-keyboard','flag-restore','numeric-invalid-replace','numeric-prefix',
        'column-copy-paste','column-delete-second','column-delete-three','column-insert-menu','undo-menu-state',
        'count-return','count-tab','format-n11','format-d11','format-d11-value','format-scientific-n11','format-n99','format-d99']
    def click(x,y,count=1):driver.click(x+556,y+252,count)
    def edit(x,y,text,commit='Tab'):
        click(x,y,2);driver.capture(dest/'editor.png')
        # VB's alphabetic cell handlers depend on real keyboard events.
        # Native text injection remains appropriate for the file picker only.
        driver.key('Home');driver.key('shift+End');driver.key('BackSpace')
        driver.run('xdotool','type','--clearmodifiers','--delay','35','--',text)
        driver.capture(dest/'typed.png')
        dump=subprocess.run([str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'dump'],env=driver.env,stdout=subprocess.PIPE)
        (dest/'editor-text.txt').write_bytes(dump.stdout)
        driver.key(commit);driver.pause(.5);driver.capture(dest/'after-commit.png')
    if selected in [['inspect'],['inspect-title'],['inspect-solve']]:
        driver.key('Escape')
        if selected==['inspect-title']:click(250,80)
        if selected==['inspect-solve']:click(286,563)
        else:click(22,42)
        driver.capture(ROOT/('runtime/'+selected[0]+'.png'))
        return
    if selected==['reopen-quoted-alias']:
        dest=ROOT/'reopen-quoted-alias';dest.mkdir(exist_ok=True)
        if (dest/'observations.json').exists():return
        driver.recover_main_menu();driver.load_case_from_main(windows(ROOT/'quoted-alias-keyboard/tora-saved.txt'))
        driver.capture(dest/'reopened.png')
        (dest/'observations.json').write_text(json.dumps({'case':'reopen-quoted-alias','review':'pending direct review','sha256':hashlib.sha256((dest/'reopened.png').read_bytes()).hexdigest()},indent=2)+'\n')
        return
    for case in cases:
        dest=ROOT/case;dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():
            print('Already captured:',case,flush=True);continue
        driver.recover_main_menu()
        if case.startswith('format-'):
            driver.click(946,534);driver.click(858,426)
            if case=='format-scientific-n11':click(302,263)
            n='99' if case=='format-n99' else ('999' if case=='format-n-limit' else ('11' if case in ['format-n11','format-scientific-n11'] else '5'))
            d='99' if case=='format-d99' else ('999' if case=='format-d-limit' else ('11' if case in ['format-d11','format-d11-value'] else '2'))
            for x,y,value in [(383,297,n),(383,314,d)]:
                click(x,y);driver.key('Home');driver.key('shift+End');driver.key('BackSpace')
                driver.run('xdotool','type','--clearmodifiers','--delay','35','--',value)
                dump=subprocess.run([str(oracle.ROOT/'build/foundation4/tora-ui.exe'),'dump'],env=driver.env,stdout=subprocess.PIPE)
                (dest/('format-'+str(y)+'-text.txt')).write_bytes(dump.stdout)
            driver.capture(dest/'selected-format.png');click(399,383);driver.capture(dest/'after-go.png')
            for x,y,value in [(250,80,'Format probe'),(187,104,'2'),(187,128,'3')]:
                click(x,y);driver.key('Home');driver.key('shift+End');driver.key('BackSpace')
                driver.run('xdotool','type','--clearmodifiers','--delay','35','--',value)
            driver.key('Return');driver.capture(dest/'initialized.png')
        else:driver.load_case_from_main(windows(source))
        driver.capture(dest/'input.png')
        if case=='numeric-commit':edit(320,227,'1.2345678901234567')
        elif case=='numeric-keyboard':edit(320,232,'1.2345678901234567')
        elif case=='numeric-replace-keyboard':edit(320,232,'1.2345678901234567')
        elif case=='objective':edit(228,227,'Minimize')
        elif case=='objective-toggle':click(230,230)
        elif case=='alias':edit(320,210,'decision')
        elif case=='alias-return':edit(320,215,'decision','Return')
        elif case=='alias-keyboard':edit(320,215,'decision','Return')
        elif case=='quoted-alias':edit(320,210,'named, "x"')
        elif case=='quoted-alias-return':edit(320,215,'named, "x"','Return')
        elif case=='quoted-alias-keyboard':edit(320,215,'named, "x"','Return')
        elif case=='relation':edit(495,245,'>')
        elif case=='relation-keyboard':edit(495,250,'>')
        elif case=='relation-replace-keyboard':edit(495,250,'>')
        elif case=='flag':edit(320,328,'Y')
        elif case=='flag-keyboard':edit(320,334,'Y')
        elif case=='flag-replace-keyboard':edit(320,334,'Y')
        elif case=='numeric-invalid':edit(320,227,'bogus')
        elif case=='numeric-invalid-replace':edit(320,232,'bogus')
        elif case=='numeric-prefix':edit(320,232,'3junk')
        elif case=='format-d11-value':edit(320,232,'1.2345678901234567')
        elif case.startswith('format-'):pass
        elif case=='count-return':click(187,128);driver.key('Return')
        elif case=='count-tab':click(187,128);driver.key('Tab')
        elif case=='column-copy-paste':
            click(320,195);driver.key('ctrl+c');click(390,195);driver.key('ctrl+p')
        elif case=='column-delete':
            click(320,195);driver.key('ctrl+d')
        elif case=='column-delete-menu':
            click(320,198);click(65,42);driver.capture(dest/'command-menu.png');click(115,111)
        elif case in ['column-delete-target','row-delete-target','column-delete-upper','row-delete-upper']:
            target=(320,198) if case.startswith('column') else (230,266)
            click(*target);click(65,42);driver.capture(dest/'command-menu.png')
            click(115,107 if case.endswith('upper') else 111)
            driver.capture(dest/'delete-mode.png');click(*target)
        elif case=='column-insert-before':
            click(320,198);click(65,42);click(115,128)
            driver.capture(dest/'insert-mode.png');click(320,198,2)
        elif case=='column-delete-three':
            click(390,198);driver.key('ctrl+i');driver.capture(dest/'inserted.png')
            click(430,198);click(65,42);driver.capture(dest/'command-menu.png');click(115,107)
        elif case=='column-delete-second':
            click(390,198);click(65,42);driver.capture(dest/'command-menu.png');click(115,107)
        elif case=='flag-restore':
            edit(320,334,'Y');edit(320,334,'N')
        elif case=='column-insert':
            click(390,195);driver.key('ctrl+i');driver.capture(dest/'insert-mode.png');click(390,195)
        elif case=='column-insert-menu':
            click(390,198);click(65,42);driver.capture(dest/'command-menu.png');click(115,128)
            driver.capture(dest/'insert-mode.png');click(390,198)
        elif case=='delete-shortcut-menu-open':
            click(320,198);click(65,42);driver.capture(dest/'command-menu.png');driver.key('ctrl+d')
        elif case=='undo-menu-state':
            click(230,266);click(65,42);click(115,128);click(230,266)
            driver.capture(dest/'inserted.png');click(65,42);driver.capture(dest/'undo-menu.png');click(115,61)
        elif case=='row-shortcuts':
            click(230,245);driver.key('ctrl+c');click(230,261);driver.key('ctrl+p')
            driver.capture(dest/'pasted.png');driver.key('ctrl+d')
        elif case=='undo':
            click(230,261);driver.key('ctrl+i');driver.capture(dest/'insert-mode.png');click(230,261)
            driver.capture(dest/'inserted.png');driver.key('ctrl+u')
        else:raise ValueError('Unknown input probe: '+case)
        driver.pause(.5);driver.capture(dest/'after-edit.png')
        if driver.ui('dialog')==0:
            driver.capture(dest/'validation-dialog.png')
            if case.startswith('format-'):
                (dest/'oracle-failure.json').write_text(json.dumps({'case':case,'status':'diagnostic requires direct review','capture':'validation-dialog.png'},indent=2)+'\n')
                print('Stopped at reference diagnostic:',case,flush=True);return
            driver.key('Return');driver.pause(.5)
            driver.capture(dest/'after-acknowledgement.png')
        try:
            # The legacy File save commands can be gray after grid edits.
            # MAIN invokes Save? while permitting the captured all-zero row.
            click(407,565);driver.pause(.5)
            driver.capture(dest/'leave-save-prompt.png')
            if driver.ui('button','Yes')!=0:raise RuntimeError('Input leave did not offer Save?')
            driver.pause(.5)
            if driver.ui('dialog')!=0:raise RuntimeError('Save? Yes did not open its native picker')
            driver.capture(dest/'save-dialog.png')
            click(195,225);type_text(driver,windows(dest/'tora-saved.txt'))
            driver.capture(dest/'save-filename.png')
            if driver.ui('button','Save')!=0:raise RuntimeError('Native Save button absent')
            driver.pause(.6)
            if driver.ui('dialog')==0:
                driver.capture(dest/'save-confirmation.png')
                if driver.ui('button','Yes')!=0:raise RuntimeError('Unexpected save confirmation')
                driver.pause(.5)
            driver.capture(dest/'after-save.png')
            if not (dest/'tora-saved.txt').is_file():raise RuntimeError('Reference did not save input')
        except RuntimeError as error:
            driver.capture(dest/'save-unavailable.png')
            (dest/'error.txt').write_text(str(error)+'\n')
            print('Needs review:',case,error,flush=True)
            return
        (dest/'captured.json').write_text(json.dumps({'case':case,'review':'pending direct review','sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in dest.iterdir() if p.suffix in ['.txt','.png']}},indent=2)+'\n')
        print('Captured:',case,flush=True)
