"""File-backed final summaries against independently reviewed original rows."""
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

records = json.loads(Path(sys.argv[2]).read_text())
comparisons = []
with tempfile.TemporaryDirectory() as temporary:
    for record in records:
        problem = record['problem']
        m, n = len(problem['supply']), len(problem['demand'])
        sources = problem.get('source_names', [''] * m)
        destinations = problem.get('destination_names', [''] * n)
        grid = [['S/D Name', *destinations, '']]
        grid += [[sources[i], *map(str, row), str(problem['supply'][i])] for i, row in enumerate(problem['costs'])]
        grid += [['', *map(str, problem['demand']), '']]
        lines = ['4000', '5', '2', '1', json.dumps(record['title']), str(m), str(n)]
        lines += [json.dumps(value) for row in grid for value in row]
        data = ('\r\n'.join(lines) + '\r\n').encode('ascii')
        assert hashlib.sha256(data).hexdigest() == record['file_sha256'], record['case'] + ': authentic file hash'
        file = Path(temporary) / 'observed-input.txt'
        file.write_bytes(data)
        run = subprocess.run([sys.argv[1], '--file', str(file), '--final'], capture_output=True, text=True)
        assert run.returncode == 0, (record['case'], run.stderr)
        actual = dict(flatten(json.loads(run.stdout)))
        expected = dict(flatten({'title': record['title'], **record['expected']}))
        assert actual.keys() == expected.keys(), record['case'] + ': field paths differ'
        for field, value in expected.items():
            comparisons.append([record['case'], field, value, actual[field], value == actual[field]])
        assert actual == expected, (record['case'], [(k, v, actual[k]) for k, v in expected.items() if v != actual[k]])
with Path(sys.argv[3]).open('w', newline='') as output:
    writer = csv.writer(output, delimiter='\t')
    writer.writerow(['case', 'field', 'expected', 'actual', 'match'])
    writer.writerows(comparisons)
print(f'{len(records)} independently reviewed final summaries, {len(comparisons)} exact fields MATCH')
