"""Compare actual solver tables with independent TORA transcriptions."""
import csv,json,subprocess,sys
from pathlib import Path

records=json.loads(Path(sys.argv[2]).read_text())
tables=cells=redirects=0
comparison=[]
for record in records:
    p=record['problem'];lines=[f"{len(p['variables'])} {len(p['constraints'])} {p['sense']}"]
    lines += [f"{v['lower']} {v['upper']} {int(v['unrestricted'])}" for v in p['variables']]
    lines += [' '.join(p['objective'])]
    lines += [f"{r['relation']} {r['rhs']} "+' '.join(r['coefficients']) for r in p['constraints']]
    lines += ['0 5 2',record.get('operation','all')]
    process=subprocess.run([sys.argv[1]],input='\n'.join(lines)+'\n',text=True,capture_output=True)
    if process.returncode:raise AssertionError(f"{record['case']}: {process.stderr}")
    actual=json.loads(process.stdout)
    assert actual['requires_artificial']==record['requires_artificial'],record['case']+' redirect'
    comparison.append([record['case'],'','classification','','',str(record['requires_artificial']),str(actual['requires_artificial'])])
    if record['requires_artificial']:redirects+=1;continue
    assert len(actual['tables'])==len(record['tables']),record['case']+' iteration count'
    assert actual['message']==record['message'],record['case']+' terminal message: '+actual['message']
    comparison.append([record['case'],'','terminal','','',record['message'],actual['message']])
    for expected,got in zip(record['tables'],actual['tables']):
        for key in ['heading','columns','rows','entering','leaving']:
            # Some older frames contain no captured selection; they provide
            # numeric/label goldens, not an invented entering/leaving oracle.
            if key in ['entering','leaving'] and expected[key] is None:continue
            if expected[key]!=got[key]:
                raise AssertionError(f"{record['case']} {expected['heading']} {key}\nTORA: {expected[key]}\nClone: {got[key]}")
        comparison.append([record['case'],expected['heading'],'heading','','',expected['heading'],got['heading']])
        for col,a,b in zip(range(len(expected['columns'])),expected['columns'],got['columns']):comparison.append([record['case'],expected['heading'],'column','',str(col),a,b])
        for a,b in zip(expected['rows'],got['rows']):
            comparison.append([record['case'],expected['heading'],'row',a['label'],'',a['label'],b['label']])
            for col,x,y in zip(expected['columns'],a['values'],b['values']):comparison.append([record['case'],expected['heading'],'cell',a['label'],col,x,y])
        tables+=1;cells+=sum(len(r['values']) for r in expected['rows'])
    for sample in record.get('supplementary_displays',[]):
        probe_lines=lines[:-2]+[' '.join(map(str,sample['format'])),record.get('operation','all')]
        result=subprocess.run([sys.argv[1]],input='\n'.join(probe_lines)+'\n',text=True,capture_output=True,check=True)
        got=json.loads(result.stdout)['tables'][sample['table']]['rows'][sample['row']]['values'][sample['column']]
        assert got==sample['value'],record['case']+' supplementary notation '+got
        comparison.append([record['case'],'Iteration 4','notation','x1-','x2',sample['value'],got])
        cells+=1
    print(record['case'],len(actual['tables']),'tables MATCH')
print(f'{len(records)} input classifications, {redirects} artificial-method redirects, {tables} complete tables, {cells} exact cell strings MATCH')

if len(sys.argv)>3:
    target=Path(sys.argv[3]);target.parent.mkdir(parents=True,exist_ok=True)
    with target.open("w",newline="",encoding="utf-8") as f:
        writer=csv.writer(f,delimiter="\t");writer.writerow(["case","iteration","kind","row","column","TORA","Clone"]);writer.writerows(comparison)
