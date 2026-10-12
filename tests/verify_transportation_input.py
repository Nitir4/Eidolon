"""Compare clone-created input bytes to hashes of independently saved TORA files."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

records = json.loads(Path(sys.argv[2]).read_text())
for record in records:
    arguments = [sys.argv[1], '--new', str(record['sources']), str(record['destinations']), record['title'], '--roundtrip']
    if 'edit' in record:
        arguments += ['--edit', *map(str, record['edit'])]
    for edit in record.get('edits', []):
        arguments += ['--edit', *map(str, edit)]
    result = subprocess.run(arguments, capture_output=True)
    assert result.returncode == 0, record['case'] + ': ' + result.stderr.decode()
    actual = hashlib.sha256(result.stdout).hexdigest()
    assert actual == record['file_sha256'], f"{record['case']}: {actual} != {record['file_sha256']}"
print(f'{len(records)} independently observed Transportation input file hashes MATCH')
