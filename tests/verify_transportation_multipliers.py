"""Exact anchor display comparison with independent original screenshots."""
import csv
import json
from pathlib import Path
import subprocess
import sys

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
for record in records:
    problem = record['problem']
    lines = [f"{len(problem['supply'])} {len(problem['demand'])}"]
    lines += [' '.join(map(str, row)) for row in problem['costs']]
    lines += [' '.join(map(str, problem['supply'])), ' '.join(map(str, problem['demand']))]
    command = [sys.argv[1], '--method', record['method'], '--anchor', *map(str, record['anchor'])]
    result = subprocess.run(command, input='\n'.join(lines) + '\n', text=True, capture_output=True)
    assert result.returncode == 0, (record['case'], result.stderr)
    actual = json.loads(result.stdout)
    assert len(actual) == record['trace_length'], record['case'] + ': trace length changed'
    for expected in record['expected']:
        index = expected['iteration'] - 1
        wanted = dict(flatten(expected['table']))
        got = dict(flatten(actual[index]))
        assert wanted.keys() == got.keys(), record['case'] + ': field paths differ'
        for field, value in wanted.items():
            comparisons.append([record['case'], str(index + 1) + field, value, got[field], value == got[field]])
        assert wanted == got, (record['case'], index + 1, [(k, v, got[k]) for k, v in wanted.items() if v != got[k]])
with Path(sys.argv[3]).open('w', newline='') as output:
    writer = csv.writer(output, delimiter='\t')
    writer.writerow(['case', 'field', 'expected', 'actual', 'match'])
    writer.writerows(comparisons)
print(f'{len(records)} anchor scenarios, {len(comparisons)} exact display fields MATCH')
