"""Authenticate original saved Game files and compare reviewed numeric fields."""
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def original_bytes(record):
    matrix = record['payoffs']
    fields = ['9000', *(str(x) for x in record['format']),
              json.dumps(record['title']), str(len(matrix)), str(len(matrix[0]))]
    fields += [json.dumps(str(value)) for row in matrix for value in row]
    return ('\r\n'.join(fields) + '\r\n').encode('ascii')


def flatten(value, path=''):
    if isinstance(value, dict):
        for key, child in value.items():
            yield from flatten(child, path + '/' + key)
    elif isinstance(value, list):
        for key, child in enumerate(value):
            yield from flatten(child, path + '/' + str(key))
    else:
        yield path, value


records = json.loads(Path(sys.argv[2]).read_text())
mode = sys.argv[4:]
comparisons = []
with tempfile.TemporaryDirectory() as directory:
    for record in records:
        data = original_bytes(record)
        if hashlib.sha256(data).hexdigest() != record['file_sha256']:
            raise RuntimeError((record['case'], 'Original file identity mismatch'))
        path = Path(directory) / (record['case'] + '.txt')
        path.write_bytes(data)
        run = subprocess.run([sys.argv[1], str(path), *mode], capture_output=True, text=True, timeout=15)
        if run.returncode:
            raise RuntimeError((record['case'], run.stderr))
        actual = dict(flatten(json.loads(run.stdout)))
        expected = dict(flatten(record['expected']))
        if actual.keys() != expected.keys():
            raise RuntimeError((record['case'], 'Unexpected result shape'))
        for field, value in expected.items():
            comparisons.append([record['case'], field, value, actual[field], value == actual[field]])
        if actual != expected:
            raise RuntimeError((record['case'], actual, expected))
with Path(sys.argv[3]).open('w', newline='') as output:
    writer = csv.writer(output, delimiter='\t')
    writer.writerow(['case', 'field', 'expected', 'actual', 'match'])
    writer.writerows(comparisons)
print(f'{len(records)} original Game examples, {len(comparisons)} reviewed numeric fields MATCH')
