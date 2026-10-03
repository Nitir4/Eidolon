#!/usr/bin/env python3
"""Audit saved Phase 0 evidence without launching TORA or repeating cases.

A pass means the declared capture/transcription coverage is present and
consistent. It does not approve Gate 0 or remove documented unknowns.
"""
from pathlib import Path
import hashlib
import json
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
REF = ROOT / 'reference/tora-captures'
CORPUS = REF / 'methods/lp/corpus'


def main():
    for script in ['audit_tora_transcriptions.py', 'audit_tora_pivots.py']:
        subprocess.run([sys.executable, str(ROOT / 'tools' / script)], check=True)
    issues = []
    counts = {'graphical': 0, 'final_summary': 0, 'roundtrip': 0}
    def require(condition, message):
        if not condition:
            issues.append(message)
    def check_png(path):
        if not path.exists():
            issues.append(f'Missing {path.relative_to(ROOT)}')
            return
        raw = path.read_bytes()
        require(raw.startswith(b'\x89PNG\r\n\x1a\n') and b'IEND' in raw[-20:],
                f'Invalid/interrupted PNG {path.relative_to(ROOT)}')
        if len(raw) >= 24:
            require(struct.unpack('>II', raw[16:24]) == (808, 608),
                    f'Unexpected oracle dimensions {path.relative_to(ROOT)}')
    def check_hash(data):
        path = ROOT / data['source']
        require(path.exists() and hashlib.sha256(path.read_bytes()).hexdigest() == data['source_sha256'],
                f'Stale source hash {data["source"]}')
    for name in ['remaining-capture-plan.json', 'transcription-capture-plan.json']:
        require(not json.loads((CORPUS / name).read_text())['pairs'], f'Repair plan is not empty: {name}')
    manifest = json.loads((REF / 'corpus/lp/manifest.json').read_text())
    for entry in manifest:
        case = entry['case']
        directory = REF / 'output-studies' / case
        graph = directory / 'graphical'
        for name in ['input', 'output-format', 'graph-initial', 'graph-one-stroke', 'graph-zoom-in', 'graph-zoom-out']:
            check_png(graph / (name + '.png'))
        data = json.loads((graph / 'transcribed-graph-result.json').read_text())
        require(data['status'] == 'visually-reviewed-graph-result', f'Unreviewed graph result: {case}')
        check_hash(data); counts['graphical'] += 1
        final = directory / 'final'
        for name in ['input', 'output-format', 'final-output']:
            check_png(final / (name + '.png'))
        data = json.loads((final / 'transcribed-summary.json').read_text())
        require(not data.get('issues'), f'Summary transcription gaps: {case}')
        check_hash(data); counts['final_summary'] += 1
    for directory in sorted((REF / 'roundtrips').iterdir()):
        if not directory.is_dir() or not (directory / 'observations.json').exists():
            continue
        data = json.loads((directory / 'observations.json').read_text())
        require(data['byte_identical'] and data['reopened_in_tora'], f'Failed roundtrip: {directory.name}')
        require((directory / 'tora-saved.txt').is_file(), f'Missing TORA-saved bytes: {directory.name}')
        counts['roundtrip'] += 1
    require(counts == {'graphical': 15, 'final_summary': 15, 'roundtrip': 19}, 'Declared evidence counts differ')
    for document in ['ARCHITECTURE.md', 'FILE_FORMAT.md', 'UI_SPEC.md', 'TRACEABILITY.md',
                     'OPEN_QUESTIONS.md', 'PHASE0_REPORT.md']:
        require((ROOT / 'docs' / document).is_file(), f'Missing {document}')
    for document in (ROOT / 'docs/tora-analysis').glob('*.md'):
        if document.name == 'README.md':
            continue
        require('Unknowns / things I could not verify' in document.read_text(),
                f'Missing unknowns section: {document.name}')
    report = {'status': 'passed' if not issues else 'failed', 'counts': counts,
              'issues': issues, 'gate0': json.loads((ROOT / 'docs/STATUS.json').read_text())['phase0']['status'],
              'limitation': 'Evidence coverage/provenance audit only; see docs/OPEN_QUESTIONS.md.'}
    (REF / 'phase0-evidence-audit.json').write_text(json.dumps(report, indent=2) + '\n')
    print(report)
    if issues:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
