"""Resumable, launch-first observations of gaps in the Dual corpus.

Only UI and saved-file observations; no numerical solver supplies oracle data.
"""
import hashlib
import json
import run_tora_corpus as oracle
from run_tora_file_probes import windows, save_as
from run_tora_two_phase_probes import fixture

ROOT = oracle.ROOT / 'reference/tora-captures/probes/phase2-dual'

def problems():
    records = json.loads((oracle.ROOT/'tests/fixtures/two-phase.json').read_text())
    seed = next(r['problem'] for r in records if r['case'] == '10_dual')
    names = ['du-seed-scientific', 'du-rhs-tie', 'du-ratio-tie', 'du-infeasible',
             'du-upper-bound', 'du-lower-bound', 'du-free', 'du-max-negative',
             'du-initial-optimal', 'du-zero-cost', 'du-redundant', 'du-fixed',
             'du-negative-rhs', 'du-decimal', 'du-zero-rhs']
    cases = {name: json.loads(json.dumps(seed)) for name in names}
    for r in cases['du-rhs-tie']['constraints']: r['rhs'] = '4'
    cases['du-ratio-tie']['objective'] = ['1', '2']
    cases['du-infeasible']['constraints'][0].update(coefficients=['-1', '-1'], rhs='1')
    cases['du-upper-bound']['variables'][0]['upper'] = '1'
    cases['du-lower-bound']['variables'][0]['lower'] = '2'
    cases['du-free']['variables'][0].update(lower='-inf', upper='inf', unrestricted=True)
    cases['du-max-negative'].update(sense='max', objective=['-2', '-3'])
    for r in cases['du-initial-optimal']['constraints']: r['relation'] = '<='
    cases['du-zero-cost']['objective'] = ['0', '3']
    cases['du-redundant']['constraints'][2] = dict(cases['du-redundant']['constraints'][1])
    cases['du-fixed']['variables'][0]['upper'] = '0'
    cases['du-negative-rhs']['constraints'][0].update(relation='<=', rhs='-4', coefficients=['-1', '-1'])
    cases['du-decimal'].update(objective=['0.7', '1.1'])
    cases['du-decimal']['constraints'][0].update(coefficients=['0.3', '0.7'], rhs='1.9')
    for r in cases['du-zero-rhs']['constraints']: r['rhs'] = '0'
    cases['du-ratio-manual'] = json.loads(json.dumps(cases['du-ratio-tie']))
    cases['du-bound-cancel'] = json.loads(json.dumps(cases['du-upper-bound']))
    cases['du-alternative-next'] = json.loads(json.dumps(cases['du-ratio-tie']))
    cases['du-alternative-next-scientific'] = json.loads(json.dumps(cases['du-ratio-tie']))
    for name, p in cases.items(): p['title'] = name
    return cases

def finish_alternative(driver,dest):
    from transcribe_tora_captures import normalize_table
    from read_tora_numeric_cells import learn, learn_labels, learn_caption_characters, transcribe
    driver.capture(dest/'alternative-ready.png')
    templates,_=learn();labels=learn_labels();captions=learn_caption_characters(templates)
    transcribe(dest/'alternative-ready.png',templates,labels,captions)
    observed=json.loads((dest/'alternative-ready.cells.json').read_text())
    header=max(row['y'] for row in observed['rows'] if any(c.get('label')=='Basic' for c in row['cells']))
    xs,ys,_=normalize_table(dest/'alternative-ready.png', dest/'alternative-grid.pgm')
    driver.click(oracle.LEGACY_ORIGIN[0]+(xs[2]+xs[3])//2-4,
                 oracle.LEGACY_ORIGIN[1]+header+8-4)
    driver.capture(dest/'alternative-selected.png');driver.click(865,432)
    driver.capture(dest/'alternative-next.png');driver.key('Return')
    left,right,bottom=driver._table_bounds(dest/'alternative-ready.png')
    driver.move(oracle.LEGACY_ORIGIN[0]+right+8,oracle.LEGACY_ORIGIN[1]+194+8)
    driver.run('xdotool','click','--repeat','40','--delay','20','1')
    result=driver.capture_all_iterations('dual_simplex',dest)
    return result

def run(driver, selected=None):
    ROOT.mkdir(parents=True, exist_ok=True)
    oracle.OUTPUT = ROOT/'runtime'; oracle.OUTPUT.mkdir(exist_ok=True)
    cases = problems()
    for name in selected or [n for n in cases if n not in ['du-ratio-manual','du-bound-cancel','du-alternative-next','du-alternative-next-scientific']]:
        if name not in cases: raise ValueError('Unknown Dual case: '+name)
        dest = ROOT/name; dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():
            print('Already captured:', name, flush=True); continue
        p = cases[name]
        source = dest/'source.txt'; source.write_bytes(fixture(p))
        driver.recover_main_menu(); driver.load_case_from_main(windows(source))
        driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists(): save_as(driver, dest/'tora-saved.txt', dest)
        driver.open_method('dual_simplex'); driver.pause(1); driver.capture(dest/'after-selection.png')
        if not driver._is_output_format_screen(dest/'after-selection.png'):
            driver.capture(dest/'method-selection-warning.png')
            if name=='du-bound-cancel':
                if driver.ui('button','Cancel')!=0: raise RuntimeError('Expected Dual Cancel button')
                driver.pause(1); driver.capture(dest/'m-value.png'); driver.click(955,534)
            else: driver.key('Return')
            driver.pause(2)
        driver.capture(dest/'output-format.png')
        if not driver._is_output_format_screen(dest/'output-format.png'):
            raise RuntimeError('Dual did not reach output formatting: '+name)
        if name.endswith('-scientific'):
            driver.click(858,502); driver.click(952,524); driver.key('ctrl+a')
            driver.run('xdotool','type','--clearmodifiers','--delay','30','2')
            driver.capture(dest/'scientific-selected.png')
        driver.click(960,595); driver.pause(1); driver.move(1300,825)
        driver.capture(dest/'iteration-01.png')
        if name=='du-ratio-manual':
            from transcribe_tora_captures import normalize_table
            xs,ys,_=normalize_table(dest/'iteration-01.png', dest/'manual-grid.pgm')
            def cell(row,column):
                driver.click(oracle.LEGACY_ORIGIN[0]+(xs[column]+xs[column+1])//2-4,
                             oracle.LEGACY_ORIGIN[1]+(ys[row]+ys[row+1])//2-4)
            cell(1,2); driver.capture(dest/'wrong-entering.png'); driver.key('Return')
            cell(3,0); driver.capture(dest/'wrong-leaving.png'); driver.key('Return')
            cell(4,0); driver.capture(dest/'leaving-before-entering.png')
            cell(1,1); driver.capture(dest/'correct-entering.png')
        result = driver.capture_all_iterations('dual_simplex', dest, return_to_input=not name.startswith('du-alternative-next'))
        if name.startswith('du-alternative-next'):
            result=finish_alternative(driver,dest)
        result.update(problem=p, review='pending direct screenshot review')
        result['sha256'] = {f.name: hashlib.sha256(f.read_bytes()).hexdigest()
                            for f in dest.iterdir() if f.suffix in ['.png','.txt']}
        (dest/'captured.json').write_text(json.dumps(result,indent=2)+'\n')
        print('New Dual gap captured:', name, flush=True)
        driver.recover_main_menu()
