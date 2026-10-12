"""Compare structural edits with independently transcribed original grid records.

Original structural saves can have corrupt dimension headers. Only the observed
cells are compatibility expectations; consistent native dimensions are a
separate, explicit safety requirement.
"""
import csv
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def encode(title, grid):
    records = ['4000', '5', '2', '1', json.dumps(title),
               str(len(grid) - 2), str(len(grid[0]) - 2)]
    records += [json.dumps(cell) for row in grid for cell in row]
    return ('\r\n'.join(records) + '\r\n').encode('ascii')


records = json.loads(Path(sys.argv[2]).read_text())
comparisons = []
with tempfile.TemporaryDirectory() as folder:
    for case in records:
        path = Path(folder) / 'input.txt'
        path.write_bytes(encode(case['title'], case['input']))
        arguments = [sys.argv[1], '--file', str(path), '--roundtrip']
        for operation in case['operations']:
            arguments += ['--' + operation[0], *map(str, operation[1:])]
        result = subprocess.run(arguments, capture_output=True)
        assert result.returncode == 0, (case['case'], result.stderr.decode())
        actual = result.stdout.decode('ascii').split('\r\n')
        assert actual.pop() == '', case['case'] + ': missing trailing CRLF'
        expected = encode(case['title'], case['expected']).decode('ascii').split('\r\n')[:-1]
        assert len(actual) == len(expected), case['case'] + ': inconsistent record count'
        for index, (wanted, got) in enumerate(zip(expected, actual)):
            comparisons.append([case['case'], str(index), wanted, got, wanted == got])
        assert actual == expected, case['case'] + ': observed grid/native header mismatch'
with Path(sys.argv[3]).open('w', newline='') as output:
    writer = csv.writer(output, delimiter='\t')
    writer.writerow(['case', 'record', 'expected', 'actual', 'match'])
    writer.writerows(comparisons)
print(f'{len(records)} observed EditGrid scenarios, {len(comparisons)} records MATCH')
