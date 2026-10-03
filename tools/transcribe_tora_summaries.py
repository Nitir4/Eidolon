#!/usr/bin/env python3
"""Read the observed two-variable/three-constraint summary layout by glyphs.

This is reference transcription tooling, not an LP implementation. Values are
read from pixels, never computed from the model. Unknown glyphs block approval.
"""
from pathlib import Path
import hashlib
import json
import re
from read_tora_numeric_cells import CAPTURES, ROOT, glyphs, image_rgb

STUDIES=CAPTURES/'output-studies'

def main():
    seeds=[
        ('01_normal',(250,206,335,222),'2.00'),
        ('01_normal',(370,206,445,222),'3.00'),
        ('01_normal',(480,206,560,222),'6.00'),
        ('01_normal',(480,223,560,239),'4.00'),
        ('01_normal',(480,411,560,427),'5.00'),
        ('01_normal',(610,411,680,427),'2.00'),
        ('01_normal',(610,428,680,444),'1.00'),
        ('09_bounded',(250,291,335,307),'7.00'),
        ('12_negative_rhs',(370,411,445,427),'-8.00'),
        ('15_minimization',(194,137,250,151),'=9.00'),
        ('06_equality',(480,206,560,222),'9.00'),
        ('15_minimization',(370,274,445,290),'2.00+'),
    ]
    templates={};labels={};provenance=[];images={}
    def raw(case):
        if case not in images: images[case]=image_rgb(STUDIES/case/'final/final-output.png')
        return images[case]
    for case,box,text in seeds:
        patterns=glyphs(raw(case),box)
        if len(patterns)!=len(text):raise ValueError(f'Review seed: {case} {box} {text}')
        for p,c in zip(patterns,text):
            if p in templates and templates[p]!=c:raise ValueError('Conflicting summary font glyph')
            templates[p]=c
        source=STUDIES/case/'final/final-output.png'
        provenance.append({'source':str(source.relative_to(ROOT)),
            'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'box':box,'reviewed_text':text})
    for case,box,text in [('01_normal',(480,360,560,376),'infinity'),
                          ('15_minimization',(370,428,445,444),'-infinity')]:
        labels['|'.join(glyphs(raw(case),box))]=text
    (STUDIES/'summary-glyph-provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
    audit={}
    for directory in sorted(p for p in STUDIES.iterdir() if p.is_dir()):
        source=directory/'final/final-output.png'
        if not source.exists():continue
        case=directory.name
        if case in ('03_infeasible','04_unbounded'):
            message='Problem has no feasible solution' if case=='03_infeasible' else 'Optimum solution is unbounded'
            result={'source':str(source.relative_to(ROOT)), 'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
                    'status':'directly-reviewed-terminal-summary','message':message,'sections':{}}
        else:
            sections={};issues=[]
            layout=[('variables',[206,223],[(250,335),(370,445),(480,560)],['value','objective_coefficient','contribution']),
                    ('constraints',[257,274,291],[(250,335),(370,445)],['rhs','slack_surplus']),
                    ('sensitivity_variables',[360,377],[(250,335),(370,445),(480,560),(610,680)],['current','minimum','maximum','reduced_cost']),
                    ('sensitivity_constraints',[411,428,445],[(250,335),(370,445),(480,560),(610,680)],['current','minimum','maximum','dual_price'])]
            for section,ys,xs,fields in layout:
                rows=[]
                for index,y in enumerate(ys,1):
                    cells={}
                    for field,(x0,x1) in zip(fields,xs):
                        box=(x0,y,x1,y+16);patterns=glyphs(raw(case),box)
                        text=labels.get('|'.join(patterns))
                        if text is None:text=''.join(templates.get(p,'?') for p in patterns)
                        valid=text in ('infinity','-infinity') or re.fullmatch(r'-?\d+\.\d+[+-]?',text)
                        if not valid:issues.append(f'{section} {index} {field}: {text!r}')
                        cells[field]={'text':text if valid else None,'box':box}
                    rows.append({'row':index,'cells':cells})
                sections[section]=rows
            result={'source':str(source.relative_to(ROOT)), 'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
                    'status':'exact-glyph-summary-transcription' if not issues else 'summary-transcription-gaps',
                    'issues':issues,'sections':sections}
        (directory/'final/transcribed-summary.json').write_text(json.dumps(result,indent=2)+'\n')
        audit[case]={'status':result['status'],'issues':result.get('issues',[])}
    (STUDIES/'summary-transcription-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
    from collections import Counter
    print(Counter(v['status'] for v in audit.values()))
    for k,v in audit.items():
        if v['issues']:print(k,v['issues'])

if __name__=='__main__':main()
