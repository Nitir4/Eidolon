"""Check observed printer page boundaries and teaching-content token counts.

PDF readers can traverse geometrically arranged cells in different orders.
Compare token multiplicity on each page; solver/display goldens check cell
identity, while rendered-page review checks layout and clipping.
"""
from collections import Counter
import json
from pathlib import Path
import sys
import unicodedata
from pypdf import PdfReader


def tokens(text):
    return unicodedata.normalize('NFKC', text).split()


def spatial_rows(page):
    chunks = []
    def visit(text, cm, tm, font, size):
        if text.strip():
            x = tm[4] * cm[0] + tm[5] * cm[2] + cm[4]
            y = tm[4] * cm[1] + tm[5] * cm[3] + cm[5]
            chunks.append((y, x, text))
    page.extract_text(visitor_text=visit)
    groups = []
    for y, x, text in sorted(chunks, key=lambda item: (-item[0], item[1])):
        # Font baselines can differ slightly for bold text in the same row.
        if not groups or abs(y - groups[-1][0]) > 1.5:
            groups.append((y, []))
        groups[-1][1].append((x, text))
    return [tokens(' '.join(t for x, t in sorted(group))) for y, group in groups]


records = json.loads(Path(sys.argv[1]).read_text())
folder = Path(sys.argv[2])
pages_checked = words_checked = rows_checked = jobs_checked = 0
for record in records:
    for job_index, job in enumerate(record['jobs'], 1):
        filename = folder / f"{record['case']}-{job_index}.pdf"
        pages = PdfReader(filename).pages
        jobs_checked += 1
        assert len(pages) == len(job['pages']), (filename, 'page count', len(pages))
        for page_index, (page, expected) in enumerate(zip(pages, job['pages']), 1):
            actual = page.extract_text()
            # Input branding and date are native; compare only teaching content.
            if 'ORIGINAL DATA' in expected:
                start = actual.find('TRANSPORTATION')
                assert start >= 0, (filename, page_index, 'missing input heading')
                actual = actual[start:]
            want, got = Counter(tokens(expected)), Counter(tokens(actual))
            assert want == got, (str(filename), page_index,
                                 'missing', dict(want - got), 'extra', dict(got - want))
            rows = spatial_rows(page)
            if 'ORIGINAL DATA' in expected:
                rows = rows[next(i for i, row in enumerate(rows) if row[0] == 'TRANSPORTATION'):]
            wanted_rows = [tokens(row) for row in job['rows'][page_index - 1]]
            assert rows == wanted_rows, (str(filename), page_index, 'spatial rows', rows, wanted_rows)
            rows_checked += len(rows)
            pages_checked += 1
            words_checked += sum(want.values())
print(f'{len(records)} observed workflows, {jobs_checked} jobs, {pages_checked} pages, '
      f'{rows_checked} ordered spatial rows and {words_checked} teaching-content tokens MATCH')
