#!/usr/bin/env python3
"""Check manually entered scalar observations; blank values never pass."""
import argparse
import csv
import math
from pathlib import Path


def check(row, application):
    value = (row.get(application) or '').strip()
    if not value:
        return 'UNTESTED'
    if value.lower() in ('unsupported', 'skipped', 'not available'):
        return 'SKIPPED'
    try:
        observed = float(value)
        expected = float(row['expected_' + application])
        tolerance = float(row['absolute_tolerance'])
    except ValueError:
        return 'INVALID'
    if not math.isfinite(observed):
        return 'FAIL'
    return 'PASS' if abs(observed-expected) <= tolerance else 'FAIL'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('csv', type=Path)
    args = parser.parse_args()
    with args.csv.open(encoding='utf-8-sig', newline='') as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
        required = {'case', 'workflow', 'metric', 'eidolon', 'tora',
                    'expected_eidolon', 'expected_tora', 'absolute_tolerance', 'notes'}
        if not required.issubset(reader.fieldnames or []):
            parser.error('CSV header differs from comparison-template.csv')
    counts = {}
    for row in rows:
        for app in ['eidolon', 'tora']:
            status = check(row, app)
            row[app + '_check'] = status
            key = app + ':' + status
            counts[key] = counts.get(key, 0)+1
    output = args.csv.with_name('comparison-reviewed.csv')
    fields = list(reader.fieldnames or []) + ['eidolon_check', 'tora_check']
    with output.open('w', encoding='utf-8', newline='') as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    for key, count in sorted(counts.items()):
        print(f'{key}: {count}')
    print(f'Reviewed scalar observations: {output}')
    print('Manual feasibility, trace, UI, file and PDF checks are still required.')
    return 1 if any(k.endswith((':FAIL', ':INVALID')) for k in counts) else 0


if __name__ == '__main__':
    raise SystemExit(main())
