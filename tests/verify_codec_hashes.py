"""Verify clone output against hashes of private, actual TORA Save As bytes."""
import hashlib
import pathlib
import sys

metadata, output = map(pathlib.Path, sys.argv[1:])
count = 0
for line in metadata.read_text().splitlines():
    if not line or line.startswith('#'):
        continue
    case, n, d, mode, length, expected, source = line.split('\t')
    actual = (output / (case + '.txt')).read_bytes()
    assert len(actual) == int(length) and hashlib.sha256(actual).hexdigest() == expected, case
    count += 1
assert count == 27
print(f'{count} clone-written files match actual TORA byte counts and SHA-256 hashes')
