"""Focused Bounded menu observations; preserve completed captures and TORA.

Run through the launch-first isolated file observer. Oracle numbers are read
only from screenshots, never supplied by a numerical solver.
"""
import hashlib
import json
import run_tora_corpus as oracle
from run_tora_file_probes import windows, save_as
from run_tora_two_phase_probes import fixture

ROOT = oracle.ROOT / 'reference/tora-captures/probes/phase2-bounded'

def problems():
    records = json.loads((oracle.ROOT/'tests/fixtures/all-slack.json').read_text())
    records += json.loads((oracle.ROOT/'tests/fixtures/two-phase.json').read_text())
    def source(name):
        return json.loads(json.dumps(next(r['problem'] for r in records if r['case'] == name)))
    cases = {
        'bd-complement-return': source('slack-complement-return'),
        'bd-bound-row-tie': source('finite-upper'),
        'bd-fixed-variable': source('finite-two-upper'),
        'bd-free-crossing-scientific': source('slack-unrestricted-crossing'),
        'bd-artificial-bounds': source('tp-upper-bound'),
        'bd-shift-start': source('01_normal'),
        'bd-shift-start-notice': source('01_normal'),
    }
    cases['bd-bound-row-tie']['variables'][0]['upper'] = '6'
    cases['bd-fixed-variable']['variables'][0]['upper'] = '0'
    cases['bd-shift-start']['variables'][0]['lower'] = '3'
    cases['bd-shift-start-notice']['variables'][0]['lower'] = '3'
    for name, p in cases.items():
        p['title'] = name
    return cases

def run(driver, selected=None):
    ROOT.mkdir(parents=True, exist_ok=True)
    oracle.OUTPUT = ROOT/'runtime'
    oracle.OUTPUT.mkdir(exist_ok=True)
    cases = problems()
    for name in selected or cases:
        if name not in cases:
            raise ValueError('Unknown Bounded case: '+name)
        dest = ROOT/name
        dest.mkdir(exist_ok=True)
        if (dest/'captured.json').exists():
            print('Already captured:', name, flush=True)
            continue
        p = cases[name]
        source = dest/'source.txt'
        source.write_bytes(fixture(p))
        driver.recover_main_menu()
        driver.load_case_from_main(windows(source))
        driver.capture(dest/'input.png')
        if not (dest/'tora-saved.txt').exists():
            save_as(driver, dest/'tora-saved.txt', dest)
        driver.open_method('bounded_simplex')
        driver.pause(1)
        driver.capture(dest/'after-selection.png')
        # Unlike All-slack, the captured Bounded menu has no redirect warning.
        # Artificial starts reach output formatting directly and run Two-phase.
        driver.capture(dest/'output-format.png')
        if name.endswith('-scientific'):
            driver.click(858, 502)
            driver.click(952, 524)
            driver.key('ctrl+a')
            driver.run('xdotool', 'type', '--clearmodifiers', '--delay', '30', '2')
            driver.capture(dest/'scientific-selected.png')
        driver.click(960, 595)
        driver.pause(1)
        driver.move(1300, 825)
        driver.capture(dest/'iteration-01.png')
        if name == 'bd-shift-start-notice':
            driver.capture(dest/'initial-notice.png')
            result = {'status': 'captured-initial-notice', 'problem': p,
                      'review': 'pending direct screenshot review'}
            result['sha256'] = {f.name: hashlib.sha256(f.read_bytes()).hexdigest()
                               for f in dest.iterdir() if f.suffix in ['.png', '.txt']}
            (dest/'captured.json').write_text(json.dumps(result, indent=2)+'\n')
            driver.key('Return')
            driver.recover_main_menu()
            continue
        result = driver.capture_all_iterations('bounded_simplex', dest)
        result.update(problem=p, review='pending direct screenshot review')
        result['sha256'] = {f.name: hashlib.sha256(f.read_bytes()).hexdigest()
                            for f in dest.iterdir() if f.suffix in ['.png', '.txt']}
        (dest/'captured.json').write_text(json.dumps(result, indent=2)+'\n')
        print('New Bounded gap captured:', name, flush=True)
        driver.recover_main_menu()
