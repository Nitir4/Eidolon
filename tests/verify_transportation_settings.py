"""Authentic nondefault-format roundtrips and independent original displays."""
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def flatten(value, path=''):
    if isinstance(value, dict):
        for key, item in value.items():
            yield from flatten(item, path + '/' + key)
    elif isinstance(value, list):
        for key, item in enumerate(value):
            yield from flatten(item, path + '/' + str(key))
    else:
        yield path, value


comparisons = []
records = json.loads(Path(sys.argv[2]).read_text())
with tempfile.TemporaryDirectory() as temporary:
    for r in records:
        p = r['problem']
        m, n = len(p['supply']), len(p['demand'])
        grid = [['S/D Name', *p.get('destination_names', [''] * n), '']]
        grid += [[p.get('source_names', [''] * m)[i], *map(str, row), str(p['supply'][i])]
                 for i, row in enumerate(p['costs'])]
        grid += [['', *map(str, p['demand']), '']]
        lines = ['4000', *map(str, r['format']), json.dumps(r['title']), str(m), str(n)]
        lines += [json.dumps(value) for row in grid for value in row]
        data = ('\r\n'.join(lines) + '\r\n').encode('ascii')
        assert hashlib.sha256(data).hexdigest() == r['file_sha256'], (r['case'], 'authentic input identity')
        path = Path(temporary) / 'input.txt'
        path.write_bytes(data)
        run = subprocess.run([sys.argv[1], '--file', str(path), '--roundtrip'], capture_output=True)
        assert run.returncode == 0, (r['case'], run.stderr.decode())
        assert run.stdout == data, (r['case'], 'roundtrip bytes')
        for suffix, options, expected in [
            ('initial', [], r['expected_initial']),
            ('final', ['--final'], {'title': r['title'], **r['expected_final']}),
        ]:
            run = subprocess.run([sys.argv[1], '--file', str(path), *options], capture_output=True, text=True)
            assert run.returncode == 0, (r['case'], suffix, run.stderr)
            result = json.loads(run.stdout)
            actual = dict(flatten(result[0] if suffix == 'initial' else result))
            wanted = dict(flatten(expected))
            assert actual.keys() == wanted.keys()
            for field, value in wanted.items():
                comparisons.append([r['case'], suffix + field, value, actual[field], value == actual[field]])
            assert actual == wanted, (r['case'], suffix, [(k, v, actual[k]) for k, v in wanted.items() if v != actual[k]])
with Path(sys.argv[3]).open('w', newline='') as output:
    writer = csv.writer(output, delimiter='\t')
    writer.writerow(['case', 'field', 'expected', 'actual', 'match'])
    writer.writerows(comparisons)
print(f'{len(records)} authentic format roundtrips, {len(comparisons)} initial/final fields MATCH')
