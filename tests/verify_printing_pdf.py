"""Compare actual native PDF page content with observed original print pages."""
import json
import re
import sys
from pathlib import Path
from pypdf import PdfReader
from collections import Counter


def normalized(text):
    # Qt may encode the same letters as presentation-form ligatures in a PDF.
    text=text.translate(str.maketrans({"ﬀ":"ff","ﬁ":"fi","ﬂ":"fl","ﬃ":"ffi","ﬄ":"ffl","ﬅ":"st","ﬆ":"st"}))
    return " ".join(text.split())


def trace_content(text):
    """Read exact tableau cells independently of font-dependent horizontal bands."""
    headings=list(re.finditer(r"\b(?:Iteration \d+|Phase \d+ \(Iter \d+\)?)(?=\s|$)",text))
    if not headings:
        raise AssertionError("Trace has no iteration/phase headings")
    variable=re.compile(r"(?:x\d+|[sSrR]x\d+)\Z")
    iterations=[]
    for index,heading in enumerate(headings):
        body=text[heading.end():headings[index+1].start() if index+1<len(headings) else len(text)]
        bands=re.split(r"\bBasic\b",body)
        if bands[0].strip() or len(bands)<2:
            raise AssertionError("Iteration has content outside its Basic table bands")
        columns=[];row_order=[];rows={}
        for band in bands[1:]:
            tokens=band.split()
            try:first_row=tokens.index("z")
            except ValueError:raise AssertionError("Band has no objective row") from None
            labels=tokens[:first_row]
            if not labels or any(label!="Solution" and not variable.fullmatch(label) for label in labels):
                raise AssertionError("Invalid or absent band column labels")
            if len(set(labels))!=len(labels) or set(labels)&set(columns):
                raise AssertionError("Repeated tableau column")
            columns.extend(labels)
            position=first_row;band_order=[]
            while position<len(tokens):
                token=tokens[position]
                if token=="z":
                    label=" ".join(tokens[position:position+2])
                    if label not in ("z (max)","z (min)"):raise AssertionError("Invalid objective row")
                    position+=2
                elif token in ("Lower","Upper","Unrestr'd"):
                    label=" ".join(tokens[position:position+2])
                    if label not in ("Lower Bound","Upper Bound","Unrestr'd (y/n)?"):
                        raise AssertionError("Invalid variable-bound row")
                    position+=2
                elif variable.fullmatch(token):
                    label=token;position+=1
                else:raise AssertionError(f"Unexpected tableau row token {token!r}")
                if label in band_order:raise AssertionError("Repeated row in one band")
                band_order.append(label)
                start=position
                while position<len(tokens) and tokens[position] not in ("z","Lower","Upper","Unrestr'd") and not variable.fullmatch(tokens[position]):
                    position+=1
                values=tokens[start:position]
                metadata=label in ("Lower Bound","Upper Bound","Unrestr'd (y/n)?")
                if not values or len(values)>len(labels) or (not metadata and len(values)!=len(labels)):
                    raise AssertionError(f"Wrong cell count for row {label!r}")
                if label not in rows:rows[label]={};row_order.append(label)
                rows[label].update(zip(labels,values))
            if band_order!=[label for label in row_order if label in band_order]:
                raise AssertionError("Tableau row order differs across bands")
        iterations.append((heading.group(),columns,[(label,rows[label]) for label in row_order]))
    return text[:headings[0].start()].strip(),iterations


def main():
    records=json.loads(Path(sys.argv[1]).read_text())
    folder=Path(sys.argv[2]);compared=0
    for record in records:
        if not record.get("print_available",True):
            if (folder/(record["case"]+".pdf")).exists():raise AssertionError('Unexpected PDF for unavailable print command')
            continue
        pages=PdfReader(folder/(record["case"]+".pdf")).pages
        expected=record["pages"]
        trace=record["method"] not in ("final","graphical")
        if (not trace and len(pages)!=len(expected)) or (trace and len(pages)<2):
            raise AssertionError(f'{record["case"]}: expected {len(expected)} original pages, got {len(pages)}')
        if any(not normalized(page.extract_text()) for page in pages):
            raise AssertionError(f'{record["case"]}: blank printed page')
        # Keep Original Data separate from output. Trace bands and their page
        # boundaries depend on the installed substitute font; all labelled
        # cells remain exact, while nontrace layouts retain page-count checks.
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
            if trace and index>0:
                # Each cell retains its iteration, row and column identity.
                # Wider substitute fonts can legitimately move Solution or
                # another column into an additional repeated-Basic band.
                if trace_content(actual)!=trace_content(expected_text):
                    raise AssertionError(f'{record["case"]}: tableau headings, columns or formatted cells differ')
                compared+=len(expected_text.split())
                continue
            if actual!=expected_text:
                for i,(a,b) in enumerate(zip(actual.split(),expected_text.split())):
                    if a!=b:break
                else:i=min(len(actual.split()),len(expected_text.split()))
                raise AssertionError(f'{record["case"]} page {index+1}, token {i}: actual={actual.split()[max(0,i-3):i+5]}, expected={expected_text.split()[max(0,i-3):i+5]}')
            compared+=len(expected_text.split())
    print(f'{len(records)} observed print states ({sum(r.get("print_available", True) for r in records)} PDFs) validate {compared} fixture content tokens; nontrace page counts match, trace pagination may vary with fonts')


if __name__=="__main__":main()
