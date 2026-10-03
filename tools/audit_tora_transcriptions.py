#!/usr/bin/env python3
"""Validate reference transcription coverage and screenshot provenance."""
from pathlib import Path
from collections import Counter
import hashlib
import json
import re

ROOT=Path(__file__).resolve().parents[1]
CORPUS=ROOT/'reference/tora-captures/methods/lp/corpus'

def main():
    progress=json.loads((CORPUS/'progress.json').read_text())['runs']
    cases={};issues=[];tables_total=0;numeric_total=0;checked=set()
    for key,run in progress.items():
        if run['status']=='directly-observed-input-warning':
            cases[key]={'status':'observed-precondition-warning'};continue
        directory=CORPUS/key
        data=json.loads((directory/'transcribed-trace.json').read_text());tables=data['tables']
        problems=list(data['issues'])
        if not tables:problems.append('No tables')
        sequence=[t['iteration'] for t in tables]
        if sequence!=list(range(1,len(tables)+1)):problems.append(f'Noncontiguous iteration sequence {sequence}')
        if not run.get('tableau_heading_verified'):problems.append('Capture manifest lacks first-heading validation')
        for table in tables:
            columns=table['columns']
            if len(set(columns))!=len(columns):problems.append('Repeated column label')
            if not columns or columns[-1]!='Solution':problems.append('Solution is not the final column')
            numeric_rows=[r for r in table['rows'] if not r['label'].startswith(('Lower','Upper','Unrestr'))]
            if len(numeric_rows)!=4:problems.append(f"Iteration {table['iteration']}: objective plus three basis rows missing")
            for row in numeric_rows:
                if set(row['values'])!=set(columns):problems.append('Numeric row has missing or extra columns')
                for column,value in row['values'].items():
                    if value=='blocked':
                        if not (table['phase']==2 and row['label'].startswith('z (') and column.startswith('Rx')):
                            problems.append('Blocked value outside Phase 2 artificial objective cell')
                    elif not re.fullmatch(r'-?\d+\.\d+',value):problems.append(f'Unresolved numeric value {value!r}')
                    else:numeric_total+=1
            for name in ['Lower Bound','Upper Bound',"Unrestr'd (y/n)?"]:
                if sum(r['label']==name for r in table['rows'])!=1:problems.append(f'Missing or repeated {name}')
            for source in table['sources']:
                if source in checked:continue
                path=ROOT/source;cell_path=path.with_suffix('.cells.json')
                observed=json.loads(cell_path.read_text())
                if observed['source_sha256']!=hashlib.sha256(path.read_bytes()).hexdigest():problems.append(f'Stale source hash {source}')
                checked.add(source)
        tables_total+=len(tables)
        status='complete-observed-table-transcription' if not problems else 'audit-failed'
        cases[key]={'status':status,'tables':len(tables),'last_phase':tables[-1]['phase'] if tables else None,
                    'issues':sorted(set(problems))}
        issues.extend((key,x) for x in problems)
    report={'status':'passed' if not issues else 'failed','runs':len(cases),'tables':tables_total,
            'numeric_cells':numeric_total,'source_screenshots_checked':len(checked),
            'counts':dict(Counter(v['status'] for v in cases.values())),'cases':cases}
    (CORPUS/'coverage-audit.json').write_text(json.dumps(report,indent=2)+'\n')
    print({k:v for k,v in report.items() if k!='cases'})
    for issue in issues:print(issue)
    if issues:raise SystemExit(1)

if __name__=='__main__':main()
