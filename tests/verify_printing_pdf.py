"""Compare actual native PDF page content with observed original print pages."""
import json
import sys
from pathlib import Path
from pypdf import PdfReader
from collections import Counter


def normalized(text):
    # Qt may encode the same letters as presentation-form ligatures in a PDF.
    text=text.translate(str.maketrans({"ﬀ":"ff","ﬁ":"fi","ﬂ":"fl","ﬃ":"ffi","ﬄ":"ffl","ﬅ":"st","ﬆ":"st"}))
    return " ".join(text.split())


def main():
    records=json.loads(Path(sys.argv[1]).read_text())
    folder=Path(sys.argv[2]);compared=0
    for record in records:
        if not record.get("print_available",True):
            if (folder/(record["case"]+".pdf")).exists():raise AssertionError('Unexpected PDF for unavailable print command')
            continue
        pages=PdfReader(folder/(record["case"]+".pdf")).pages
        expected=record["pages"]
        if len(pages)!=len(expected):
            raise AssertionError(f'{record["case"]}: expected {len(expected)} original pages, got {len(pages)}')
        # Font metrics may move a row across a physical page boundary. Keep
        # exact ordered teaching content and total page count independently.
        groups=[([pages[0]],[expected[0]])]
        if len(pages)>1:groups.append((pages[1:],expected[1:]))
        for index,(group,wanted_pages) in enumerate(groups):
            wanted=wanted_pages[0]
            text=normalized(" ".join(p.extract_text() for p in group))
            # Branding/date belong to the clone; teaching content starts here.
            start=text.find(wanted["start"])
            if start<0:raise AssertionError(f'{record["case"]} page {index+1}: missing heading {wanted["start"]}')
            actual=text[start:]
            expected_text=normalized(" ".join(p["text"] for p in wanted_pages))
            if "plot_labels" in wanted:
                prefix=normalized(wanted["text"])
                if not actual.startswith(prefix):raise AssertionError(f'{record["case"]}: graph summary differs')
                if Counter(actual[len(prefix):].split())!=Counter(wanted["plot_labels"]):raise AssertionError(f'{record["case"]}: graph labels differ')
                compared+=len(prefix.split())+len(wanted["plot_labels"])
                continue
            if actual!=expected_text:
                for i,(a,b) in enumerate(zip(actual.split(),expected_text.split())):
                    if a!=b:break
                else:i=min(len(actual.split()),len(expected_text.split()))
                raise AssertionError(f'{record["case"]} page {index+1}, token {i}: actual={actual.split()[max(0,i-3):i+5]}, expected={expected_text.split()[max(0,i-3):i+5]}')
            compared+=len(expected_text.split())
    print(f'{len(records)} observed print states ({sum(r.get("print_available", True) for r in records)} PDFs) match captured page counts and {compared} content tokens')


if __name__=="__main__":main()
