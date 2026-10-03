#!/usr/bin/env python3
"""Check observed colored selections against policies using displayed cells.

This audits reference observations, not a clone solver. It never creates or
replaces a numeric transcription. Rounded-cell agreement cannot establish a
hidden tolerance or precision policy.
"""
from fractions import Fraction
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / 'reference/tora-captures/methods/lp/corpus'


def main():
    checks = []
    counts = {'primal_entering': 0, 'primal_leaving': 0, 'dual': 0}
    for path in sorted(CORPUS.glob('*/*/transcribed-trace.json')):
        data = json.loads(path.read_text())
        for table in data['tables']:
            entering = table.get('selected_entering_column')
            leaving = table.get('selected_leaving_row')
            if not entering:
                continue
            rows = [r for r in table['rows']
                    if not r['label'].startswith(('Lower', 'Upper', 'Unrestr'))]
            z, basis = rows[0], rows[1:]
            costs = {c: Fraction(v) for c, v in z['values'].items()
                     if c != 'Solution' and v != 'blocked'}
            observed = {'trace': str(path.relative_to(ROOT)),
                        'trace_sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                        'iteration': table['iteration'], 'phase': table['phase']}
            if any(Fraction(r['values']['Solution']) < 0 for r in basis):
                if not leaving:
                    continue
                row = min(basis, key=lambda r: Fraction(r['values']['Solution']))
                ratios = {c: costs[c] / Fraction(v) for c, v in row['values'].items()
                          if c != 'Solution' and Fraction(v) < 0}
                expected = min(ratios, key=ratios.get)
                checks.append(dict(observed, rule='dual', observed=[entering, leaving],
                                   expected=[expected, row['label']],
                                   passed=entering == expected and leaving == row['label']))
                counts['dual'] += 1
                continue
            select = min if z['label'] == 'z (max)' else max
            expected = select(costs, key=costs.get)
            checks.append(dict(observed, rule='primal_entering', observed=entering,
                               expected=expected, passed=entering == expected))
            counts['primal_entering'] += 1
            if not leaving:
                continue  # A bound can win before a row; do not invent a row.
            ratios = {r['label']: Fraction(r['values']['Solution']) / Fraction(r['values'][entering])
                      for r in basis if Fraction(r['values'][entering]) > 0
                      and Fraction(r['values']['Solution']) >= 0}
            expected = min(ratios, key=ratios.get)
            checks.append(dict(observed, rule='primal_leaving', observed=leaving,
                               expected=expected, passed=leaving == expected))
            counts['primal_leaving'] += 1
    failed = [c for c in checks if not c['passed']]
    report = {'status': 'passed' if not failed else 'failed', 'counts': counts,
              'limitation': 'Policies agree with these displayed cells; hidden numerical thresholds are unverified.',
              'checks': checks}
    (CORPUS / 'pivot-policy-audit.json').write_text(json.dumps(report, indent=2) + '\n')
    print({'status': report['status'], 'counts': counts, 'disagreements': len(failed)})
    if failed:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
