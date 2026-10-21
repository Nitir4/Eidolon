#!/usr/bin/env python3
"""Generate classroom examples and independent small-model answer keys.

No TORA outputs are claimed. Uses exact rational vertex enumeration, exhaustive
finite enumeration, Gaussian elimination, path enumeration and queue formulas.
"""
from pathlib import Path
from fractions import Fraction as F
from itertools import combinations, permutations, product
import csv
import json
import math

ROOT = Path(__file__).resolve().parents[1] / 'examples/practice-bank'
records = []

def q(x): return '"' + str(x).replace('"', '""') + '"'
def native(marker, name, extra, rows):
    return '\r\n'.join(map(str, [marker, 5, 4, 1, q(name), *extra, *[q(x) for row in rows for x in row]])) + '\r\n'
def tsv(rows): return '\n'.join('\t'.join(map(str, r)) for r in rows) + '\n'
def add(name, family, workflows, model, expected, rows, file=None, notes=''):
    folder = ROOT / name
    folder.mkdir(parents=True, exist_ok=True)
    (folder/'grid.tsv').write_text(tsv(rows))
    record = dict(id=name, family=family, workflows=workflows, model=model, expected=expected,
                  notes=notes, original_comparison='pending', grid=f'{name}/grid.tsv')
    if file:
        suffix, content = file
        path = folder / ('input' + suffix)
        path.write_bytes(content.encode('ascii'))
        record['input'] = f'{name}/{path.name}'
    (folder/'answer.json').write_text(json.dumps(record, indent=2) + '\n')
    records.append(record)
    return record

def linear_solve(a, b):
    n=len(b); a=[[F(x) for x in row]+[F(y)] for row,y in zip(a,b)]
    for j in range(n):
        pivot=next((i for i in range(j,n) if a[i][j]),None)
        if pivot is None: return None
        a[j],a[pivot]=a[pivot],a[j]; v=a[j][j]; a[j]=[x/v for x in a[j]]
        for i in range(n):
            if i!=j:
                v=a[i][j]; a[i]=[x-v*y for x,y in zip(a[i],a[j])]
    return [row[-1] for row in a]

def lp_answer(p, integral=()):
    # These practice models have exactly two variables and a bounded optimum.
    lines=[(list(map(F,a)),F(b)) for a,rel,b in p['constraints']]
    lower=p.get('lower',[0,0]); upper=p.get('upper',[None,None])
    lines += [([F(j==k) for j in range(2)],F(lower[k])) for k in range(2)]
    lines += [([F(j==k) for j in range(2)],F(upper[k])) for k in range(2) if upper[k] is not None]
    def feasible(x):
        if any(x[j]<lower[j] or (upper[j] is not None and x[j]>upper[j]) for j in range(2)): return False
        for a,rel,b in p['constraints']:
            y=sum(F(v)*z for v,z in zip(a,x))
            if not (y<=b if rel=='<=' else y>=b if rel=='>=' else y==b): return False
        return True
    candidates=[]
    if not integral:
        for first,second in combinations(lines,2):
            x=linear_solve([first[0],second[0]],[first[1],second[1]])
            if x is not None and feasible(x): candidates.append(x)
    elif tuple(integral)==(0,1):
        candidates=[list(map(F,x)) for x in product(range(31),repeat=2) if feasible(x)]
    else:
        k=integral[0]
        for v in range(31):
            for a,b in lines:
                x=linear_solve([a,[int(j==k) for j in range(2)]],[b,v])
                if x is not None and feasible(x): candidates.append(x)
    assert candidates
    objective=lambda x:sum(F(c)*v for c,v in zip(p['objective'],x))
    best=(min if p['sense']=='min' else max)(objective(x) for x in candidates)
    winners=sorted(set(tuple(x) for x in candidates if objective(x)==best))
    return {'status':'optimal','objective':float(best),'objective_exact':str(best),
            'one_solution':list(map(float,winners[0])), 'one_solution_exact':list(map(str,winners[0])),
            'check':'Objective and feasibility are authoritative; tied solutions may differ.'}

def lp_file(p, integer=None):
    rows=[['x1','x2','',''],[*p['objective'],'','']]
    rows += [[*a,rel,b] for a,rel,b in p['constraints']]
    rows += [[*p.get('lower',[0,0]),'',''],*[ [*['infinity' if x is None else x for x in p.get('upper',[None,None])],'',''] ],['n','n','','']]
    data=native(5000 if integer else 3000,p['title'],[2,len(p['constraints']),q('Minimize' if p['sense']=='min' else 'Maximize')],rows)
    if integer: data+='\r\n'.join(q(x) for x in [*integer,'',''])+'\r\n'
    return data, rows

# Five different bounded 2-variable feasible regions, shared across applicable methods.
normal=[([3,2],[([1,1],'<=',4),([1,0],'<=',2),([0,1],'<=',3)]),
        ([5,4],[([6,4],'<=',24),([1,2],'<=',6),([-1,1],'<=',1)]),
        ([2,3],[([2,1],'<=',8),([1,2],'<=',8),([1,0],'<=',3)]),
        ([4,7],[([1,2],'<=',10),([3,1],'<=',12),([0,1],'<=',4)]),
        ([7,5],[([2,3],'<=',18),([3,1],'<=',15),([1,0],'<=',4)])]
for i,(c,cons) in enumerate(normal,1):
    name=f'lp-{i:02}'; p={'title':name,'sense':'max','objective':c,'constraints':cons}
    data,rows=lp_file(p)
    add(name,'lp',['LP / Graphical','LP / All-slack','LP / Final solution','LP / Sensitivity'],p,lp_answer(p),rows,('.txt',data),
        'LP TSV contains grid columns x1, x2, relation, RHS; paste names/objective and constraint rows separately to avoid protected blank cells. Bounds remain as shown.')
    bound=dict(p,title=f'bounded-{i:02}',upper=[i+1,2+i/2])
    data,rows=lp_file(bound)
    add(bound['title'],'lp',['LP / Bounded variables'],bound,lp_answer(bound),rows,('.txt',data))
    cover={'title':f'cover-{i:02}','sense':'min','objective':[i+1,2*i+1],
           'constraints':[([1,2],'>=',4+i),([3,1],'>=',6+i),([1,0],'<=',8),([0,1],'<=',8)]}
    data,rows=lp_file(cover)
    add(cover['title'],'lp',['LP / Big-M','LP / Two-phase','LP / Dual simplex'],cover,lp_answer(cover),rows,('.txt',data), 'For Big-M use M=1000. Dual simplex applies directly to this nonnegative-cost minimization.')
    for change in ('cj','rhs'):
        changed=json.loads(json.dumps(p))
        if change=='cj': changed['objective']=[c[0]+3,c[1]-1]
        else: changed['constraints'][0][2]+=2
        case=f'change-{change}-{i:02}'; data,rows=lp_file(p)
        expected={'before':lp_answer(p),'after':lp_answer(changed)}
        workflows=['LP / Change Cj'] if change=='cj' else ['LP / Change RHS','LP / Dual Revised (RHS)']
        add(case,'lp',workflows,dict(baseline=p,changed=changed),expected,rows,('.txt',data),
            'Open baseline input, then Class extensions. Enter changed objective or RHS from answer.json. Compare before/after. Original comparison requires manually changing and re-solving a copy.')
    for mixed in (False,True):
        case=f'integer-{"mixed" if mixed else "pure"}-{i:02}'
        ip=dict(p,title=case); flags=['y','n' if mixed else 'y']; data,rows=lp_file(ip,flags)
        add(case,'integer',['Integer / Mixed'] if mixed else ['Integer / Pure automated','Integer / Guided branch-and-bound'],dict(ip,integral=flags),lp_answer(ip,(0,) if mixed else (0,1)),rows,('.txt',data), 'Integer enumeration bounds 0..30 cover these small bounded feasible regions.')

# Exact elimination for equations and matrix inverse.
matrices=[[[2,1],[1,-1]],[[3,2],[1,4]],[[1,2,1],[0,1,3],[2,1,1]],[[4,1,0],[1,3,1],[0,1,2]],[[2,-1,1],[1,2,-1],[3,1,2]]]
for i,a in enumerate(matrices,1):
    n=len(a); x=list(range(1,n+1)); b=[sum(v*w for v,w in zip(row,x)) for row in a]
    inverse=[linear_solve(a,[int(j==k) for j in range(n)]) for k in range(n)]
    assert all(v is not None for v in inverse)
    inverse=[list(row) for row in zip(*inverse)]
    for matrix_only in (False,True):
        case=f'{"inverse" if matrix_only else "equations"}-{i:02}'; rows=a if matrix_only else [row+[y] for row,y in zip(a,b)]
        add(case,'matrix' if matrix_only else 'equations',['Equations / Matrix inverse' if matrix_only else 'Equations / Solve linear equations'],{'A':a,'b':b},
            {'solution':x,'inverse_exact':[[str(v) for v in row] for row in inverse]},rows,
            ('.txt',native(1000 if matrix_only else 2000,case,[n],rows)))

# Exhaustive small transportation allocations, with integral supplies/demands.
for i,costs in enumerate([[[2,3,1],[5,4,8]],[[4,1,7],[2,6,3]],[[8,6,2],[3,5,4]],[[1,4,6],[7,2,5]],[[3,8,5],[4,1,7]]],1):
    supply=[3,4]; demand=[2,2,3]; choices=[]
    for row in product(*(range(d+1) for d in demand)):
        if sum(row)!=supply[0]: continue
        flow=[list(row),[d-x for d,x in zip(demand,row)]]
        cost=sum(flow[r][c]*costs[r][c] for r in range(2) for c in range(3)); choices.append((cost,flow))
    objective,flow=min(choices); case=f'transport-{i:02}'; rows=[['S/D Name','D1','D2','D3','Supply'],['S1',*costs[0],3],['S2',*costs[1],4],['Demand',*demand,'']]
    add(case,'transportation',['Transportation / North-west','Transportation / Least cost','Transportation / Vogel','Transportation / MODI improvement','Transportation / Final solution'],dict(costs=costs,supply=supply,demand=demand),dict(optimal_cost=objective,one_optimal_flow=flow),rows,('.txt',native(4000,case,[2,3],rows)),
        'Final optimum key. Initial heuristic costs can differ. Compare each initial allocation separately, then improve to the stated optimum. Paste numeric costs starting at the first cost cell, or open input.txt.')

# Square min/max assignments plus rectangular practice in both senses.
for sense in ('min','max'):
    for i,costs in enumerate([[[9,2,7],[6,4,3],[5,8,1]],[[4,8,6],[7,3,5],[9,2,1]],[[5,1,4],[2,6,3]],[[3,8],[7,2],[4,6]],[[1,4,7],[5,2,8],[6,9,3]]],1):
        m,n=len(costs),len(costs[0]); candidates=[]
        if m<=n:
            for cols in permutations(range(n),m): candidates.append((sum(costs[r][c] for r,c in enumerate(cols)),list(enumerate(cols))))
        else:
            for rs in permutations(range(m),n): candidates.append((sum(costs[r][c] for c,r in enumerate(rs)),[(r,c) for c,r in enumerate(rs)]))
        objective,pairs=(min if sense=='min' else max)(candidates); case=f'assignment-{sense}-{i:02}'
        obj=dict(format='ToraCloneAssignment',version=1,title=case,sense=sense,workers=[f'W{r+1}' for r in range(m)],jobs=[f'J{c+1}' for c in range(n)],numberFormat=[0,5,4],costs=costs)
        add(case,'assignment',[f'Assignment / {sense}']+(['Assignment / Rectangular'] if m!=n else []),obj,dict(objective=objective,one_pairing_zero_based=pairs),costs,('.assignment.json',json.dumps(obj,indent=2)), 'Original equivalent: Transportation with unit supply/demand; negate profits for max. Rectangular cases need a zero-cost dummy row/column.')
# A fifth rectangular case, giving five rectangular exercises rather than four.
costs=[[8,2,7,5],[4,6,1,3]]; obj=dict(format='ToraCloneAssignment',version=1,title='assignment-rectangle-05',sense='min',workers=['W1','W2'],jobs=['J1','J2','J3','J4'],numberFormat=[0,5,4],costs=costs)
add(obj['title'],'assignment',['Assignment / Rectangular'],obj,dict(objective=3,one_pairing_zero_based=[[0,1],[1,2]]),costs,('.assignment.json',json.dumps(obj,indent=2)))

# Connected undirected graphs: enumerate all spanning trees and all simple paths.
for i,weights in enumerate(([2,5,1,4,3],[4,2,6,3,1],[1,7,3,2,5],[6,3,2,5,4],[3,4,5,1,2]),1):
    edges=[[u,v,w] for (u,v),w in zip([(0,1),(0,2),(1,2),(1,3),(2,3)],weights)]
    trees=[]
    for es in combinations(edges,3):
        seen={0}
        for _ in range(4):
            for u,v,w in es:
                if u in seen or v in seen: seen.update((u,v))
        if len(seen)==4: trees.append(sum(e[2] for e in es))
    def paths(u,visited):
        if u==3: yield 0
        for a,b,w in edges:
            v=b if a==u else a if b==u else None
            if v is not None and v not in visited:
                yield from (w+x for x in paths(v,visited|{v}))
    distance=min(paths(0,{0}))
    for tree in (True,False):
        case=f'{"mst" if tree else "route"}-{i:02}'; rows=[['Node Name','A','B','C','D']]+[[chr(65+r)]+['' if r==c else 'infinity' for c in range(4)] for r in range(4)]
        for u,v,w in edges: rows[u+1][v+1]=rows[v+1][u+1]=w
        data=native(7001 if tree else 0,case,[4],rows)+'1\r\n'
        add(case,'mst' if tree else 'route',['Networks / Minimum spanning tree'] if tree else ['Networks / Shortest route','Networks / Dijkstra','Networks / Floyd','Networks / Bellman-Ford extension'],dict(nodes=4,edges_zero_based=edges,source=0,sink=3),dict(total_weight=min(trees)) if tree else dict(distance=distance),rows,('.txt',data), 'Route files use marker 0: select Shortest Route before opening. Source A, destination D. Positive undirected costs are valid for all listed methods.')
    case=f'flow-{i:02}'; capacities=[w+i for w in weights]; edges=[[u,v,w] for (u,v),w in zip([(0,1),(0,2),(1,2),(1,3),(2,3)],capacities)]
    cuts=[sum(w for u,v,w in edges if u in s and v not in s) for mask in range(4) for s in [{0}|{j+1 for j in range(2) if mask>>j&1}]]
    rows=[['Node Name','A','B','C','D']]+[[chr(65+r)]+['' if r==c else 0 for c in range(4)] for r in range(4)]
    for u,v,w in edges: rows[u+1][v+1]=w
    add(case,'flow',['Networks / Maximal flow'],dict(nodes=4,edges_zero_based=edges,source=0,sink=3),dict(max_flow=min(cuts),min_cut=min(cuts)),rows,('.txt',native(7004,case,[4],rows)+'0\r\n'), 'Source A, sink D. Expected value independently enumerates all four source/sink cuts.')

# CPM and PERT: different branch durations and a join. PERT means equal CPM durations.
for i,durations in enumerate(([3,2,4,3,2],[2,5,6,2,3],[4,3,2,5,1],[5,2,3,4,2],[2,4,5,3,4]),1):
    endpoints=[(1,2),(1,3),(2,4),(3,4),(4,5)]; early={1:0}
    for node in range(2,6): early[node]=max(early[u]+d for (u,v),d in zip(endpoints,durations) if v==node)
    duration=early[5]; late={5:duration}
    for node in range(4,0,-1): late[node]=min(late[v]-d for (u,v),d in zip(endpoints,durations) if u==node)
    floats=[late[v]-early[u]-d for (u,v),d in zip(endpoints,durations)]
    for pert in (False,True):
        case=f'{"pert" if pert else "cpm"}-{i:02}'
        rows=[[u,v,chr(65+j),d-1,d,d+1] if pert else [u,v,chr(65+j),d] for j,((u,v),d) in enumerate(zip(endpoints,durations))]
        expected=dict(project_duration=duration,total_float=floats,critical_activities=[chr(65+j) for j,f in enumerate(floats) if f==0])
        if pert: expected['activity_variances_exact']=['1/9']*5; expected['project_variance_exact']='1/3'; expected['note']='Do not sum variance over both branches; compare selected critical path and tie behaviour separately.'
        add(case,'planning',['Planning / PERT'] if pert else ['Planning / CPM','Planning / Schedule bar chart'],dict(method='PERT' if pert else 'CPM',activities=rows),expected,rows,('.txt',native(8002 if pert else 8001,case,[5,len(rows)+1],rows)), 'CPM TSV: from, to, symbol, duration. PERT TSV: from, to, symbol, optimistic, most likely, pessimistic; start at the first estimate cell for the last three fields.')

# Stationary queues: exact birth-death balance and closed geometric-tail sums.
for kind in ('mm1','mmc','finite-k','finite-n','finite-kn'):
    for i in range(1,6):
        lam=F(i,2); mu=F(i+2); c=1 if kind=='mm1' else 2
        k=4+i if kind in ('finite-k','finite-kn') else (5+i if kind=='finite-n' else None)
        n=5+i if kind in ('finite-n','finite-kn') else None
        weights=[F(1)]; last=k if k else c
        for j in range(1,last+1): weights.append(weights[-1]*lam*(n-j+1 if n else 1)/(min(j,c)*mu))
        rho=lam/(c*mu); norm=sum(weights) if k else sum(weights[:-1])+weights[-1]/(1-rho)
        ps=[w/norm for w in weights]
        if k:
            L=sum(j*p for j,p in enumerate(ps)); Lq=sum(max(0,j-c)*p for j,p in enumerate(ps)); accepted=mu*sum(min(j,c)*p for j,p in enumerate(ps))
        else:
            Lq=ps[-1]*rho/(1-rho)**2; L=Lq+lam/mu; accepted=lam
        expected={key:float(v) for key,v in dict(P0=ps[0],L=L,Lq=Lq,W=L/accepted,Wq=Lq/accepted,lambda_effective=accepted).items()}
        expected['exact']={key:str(v) for key,v in dict(P0=ps[0],L=L,Lq=Lq,W=L/accepted,Wq=Lq/accepted).items()}
        case=f'queue-{kind}-{i:02}'; row=[str(float(lam)),str(float(mu)),c,k or 'infinity',n or 'infinity']
        labels={'mm1':'M/M/1','mmc':'M/M/c','finite-k':'Finite capacity K','finite-n':'Finite source N','finite-kn':'Finite K and N'}
        add(case,'queue',[f'Queuing / {labels[kind]}'],dict(arrival=float(lam),service=float(mu),servers=c,K=k,N=n),expected,[row],('.txt',native(6000,case,[1],[row])), 'Finite source lambda is per idle source. W/Wq use accepted throughput. Common time units throughout.')
for death in (False,True):
    for i in range(1,6):
        case=f'queue-pure-{"death" if death else "birth"}-{i:02}'; row=[0 if death else i,i if death else 0,1,500 if death else 'infinity',500 if death else 'infinity']
        # Constant-rate deaths stop at zero, unlike independent-customer deaths.
        initial=500
        expected={'time':1,'rate':i,'P_no_events':math.exp(-i),'mean':initial-i if death else i}
        if death: expected.update(initial_population=500,mean_note='500-rate to displayed precision; exact mean has a positive Poisson tail correction below 1e-700.')
        add(case,'queue',[f'Queuing / Pure {"death" if death else "birth"}'],dict(rate=i,time=1,initial_population=500 if death else 0),expected,[row],('.txt',native(6000,case,[1],[row])), 'Transient constant-rate unit-time profile. Pure death uses N=500 to match the captured original effective population; record probability of no deaths at state 500. Pure birth records state 0.')

# Five crossing 2x2 zero-sum games; all valid for graphical and LP-based solve.
for i,m in enumerate(([[1,-1],[-1,1]],[[4,1],[0,3]],[[3,-2],[-1,2]],[[-1,-4],[-5,-2]],[[5,0],[1,4]]),1):
    a,b=m[0]; c,d=m[1]; den=F(a-b-c+d); p=F(d-c)/den; r=F(d-b)/den; value=F(a*d-b*c)/den
    assert 0<p<1 and 0<r<1
    case=f'game-{i:02}'; expected=dict(value=float(value),row_probabilities=[float(p),float(1-p)],column_probabilities=[float(r),float(1-r)],exact=dict(value=str(value),p=str(p),q=str(r)))
    add(case,'game',['Games / LP-based','Games / Graphical'],dict(payoffs=m),expected,m,('.txt',native(9000,case,[2,2],m)), 'Payoffs are to the maximizing row player. Graphical route requires two rows.')

workflow={}
for r in records:
    for name in r['workflows']: workflow.setdefault(name,[]).append(r['id'])
assert all(len(ids)>=5 for ids in workflow.values()), workflow
ROOT.mkdir(parents=True,exist_ok=True)
# Keep the generated index compact; each case's answer.json remains fully indented.
index_lines = ['{', '  "version": 1,', '  "original_comparison": "pending",', '  "workflows": {']
index_lines += ['    '+json.dumps(k)+': '+json.dumps(v)+(',' if i+1<len(workflow) else '') for i,(k,v) in enumerate(workflow.items())]
index_lines += ['  },', '  "cases": [']
index_lines += ['    '+json.dumps(r,separators=(',',':'))+(',' if i+1<len(records) else '') for i,r in enumerate(records)]
index_lines += ['  ]', '}']
(ROOT/'bank.json').write_text('\n'.join(index_lines)+'\n')
with (ROOT/'comparison.csv').open('w',newline='') as f:
    writer=csv.writer(f); writer.writerow(['workflow','example','clone_status','clone_value','original_status','original_value','feasible','notes'])
    for name,ids in workflow.items():
        for case in ids: writer.writerow([name,case,'','','','','','Pending comparison'])
intro='''# Practice problems for the comparison session

Every listed workflow has at least five exercises. These are newly prepared
practice problems with independent mathematical answer keys. **Original TORA
comparison is pending.** A shared exercise is deliberately reused across methods
so we can compare their final answers and intermediate behaviour on the same data.

Open a case's `input.txt` through its module's Open Existing File action; Assignment
uses `input.assignment.json`. Read `answer.json` for the problem, expected answer
and any method settings. `grid.tsv` can be copied from a text editor or spreadsheet.
For LP, paste only editable blocks (objective coefficients, constraint rows, bounds)
to avoid the protected blanks. Planning hides unused CPM/PERT columns; paste each
visible block separately. Network files include names and diagonal placeholders;
open them as files, or paste editable off-diagonal blocks only.

Use `comparison.csv` to record both applications' results, constraints/flows and
notes. Exact fractions are included where useful. Compare objective and feasibility
before variable/pairing identity: equally optimal solutions and tie order can differ.
For Transportation compare the initial heuristic separately, then the final optimum.
Cj/RHS extensions compare to a manually changed copy re-solved in original TORA.
Assignment uses the unit-supply/demand Transportation equivalent in original TORA.
The negative-edge Bellman-Ford extension uses positive graphs here for a common baseline.

Answer keys use exact rational 2D vertex enumeration, bounded integer enumeration,
Gaussian elimination, exhaustive assignments/transportation allocations/spanning
trees/cuts/simple paths, DAG path lengths, birth-death balance with analytic tails,
and 2x2 mixed-game formulas. No recorded original result is invented. Decimal
outputs should be compared at the same notation/precision (files use four places).
Pure-death means are specified to displayed precision; its boundary is absorbing zero.
The generator is `tools/generate_practice_bank.py` and uses only Python's standard library.

These are baseline problems. We will separately test invalid input, infeasibility,
unboundedness, degeneracy/ties, singular matrices, disconnected/negative-cycle
networks, cyclic planning, unstable queues, numerical extremes and large tables
once the UX work is accepted. Native Windows/macOS and physical mixed-DPI checks
remain separate from this comparison session.

| Workflow | Exercises |
|---|---|
'''
for name,ids in workflow.items(): intro+='| '+name+' | '+', '.join(f'[{case}]({case}/README.md)' for case in ids)+' |\n'
intro+=f'\n{len(records)} distinct cases; {len(workflow)} workflows; {sum(map(len,workflow.values()))} comparison rows.\n'
(ROOT/'README.md').write_text(intro)
print(f'{len(records)} cases, {len(workflow)} workflows, {sum(map(len,workflow.values()))} comparison rows; minimum {min(map(len,workflow.values()))} examples per workflow')

# Readable exercise sheets alongside machine-readable inputs/answers.
def markdown_table(headers, rows):
    return '| ' + ' | '.join(headers) + ' |\n|' + '|'.join(['---']*len(headers)) + '|\n' + ''.join('| ' + ' | '.join(str(x) for x in row) + ' |\n' for row in rows)
def expression(coefficients):
    return ' + '.join(f'{value}*x{j+1}' for j,value in enumerate(coefficients))
for r in records:
    m=r['model']; family=r['family']
    sheet='# '+r['id']+'\n\nMethods: '+', '.join(r['workflows'])+'.\n\n'
    sheet+='Open ['+Path(r['input']).name+']('+Path(r['input']).name+') in the matching module.\n\n'
    if family in ('lp','integer'):
        p=m.get('baseline',m)
        sheet+=('Maximize' if p['sense']=='max' else 'Minimize')+' `z = '+expression(p['objective'])+'`.\n\nSubject to:\n\n'
        sheet+=''.join('- `'+expression(a)+' '+rel+' '+str(b)+'`\n' for a,rel,b in p['constraints'])+'\n'
        sheet+='Bounds: '+', '.join(f"{p.get('lower',[0,0])[j]} <= x{j+1} <= {p.get('upper',[None,None])[j] if p.get('upper',[None,None])[j] is not None else 'infinity'}" for j in range(2))+'.\n\n'
        if family=='integer': sheet+='Integer variables: '+', '.join(f'x{j+1}' for j,v in enumerate(m['integral']) if v=='y')+'. Other variables are continuous.\n\n'
        if 'changed' in m:
            change=m['changed']; sheet+='Changed objective: `'+expression(change['objective'])+'`. Changed RHS: `'+str([row[2] for row in change['constraints']])+'`.\n\n'
    elif family in ('matrix','equations'):
        rows=m['A'] if family=='matrix' else [a+[b] for a,b in zip(m['A'],m['b'])]
        sheet+=markdown_table([f'x{j+1}' for j in range(len(m['A']))]+([] if family=='matrix' else ['RHS']),rows)+'\n'
    elif family in ('mst','route','flow'):
        sheet+=markdown_table(['From (1-based)','To (1-based)','Capacity' if family=='flow' else 'Weight'],[[u+1,v+1,w] for u,v,w in m['edges_zero_based']])+'\n'
        sheet+='Start/source: node A (1). Destination/sink: node D (4).\n\n'
    elif family=='planning':
        sheet+=markdown_table(['From','To','Activity']+(['a','m','b'] if m['method']=='PERT' else ['Duration']),m['activities'])+'\n'
    elif family=='queue':
        sheet+=markdown_table(['Parameter','Value'],list(m.items()))+'\n'
    elif family in ('assignment','game','transportation'):
        matrix=m.get('costs',m.get('payoffs'))
        sheet+=markdown_table(['Row']+[str(j+1) for j in range(len(matrix[0]))],[[i+1,*row] for i,row in enumerate(matrix)])+'\n'
        if family=='transportation': sheet+='Supply: `'+str(m['supply'])+'`. Demand: `'+str(m['demand'])+'`.\n\n'
        if family=='assignment': sheet+='Objective: '+m['sense']+'.\n\n'
    sheet+='Expected answer (independent mathematics; original comparison pending):\n\n```json\n'+json.dumps(r['expected'],indent=2)+'\n```\n\n'+r['notes']+'\n\n[Paste block](grid.tsv) · [Structured problem and answer](answer.json) · [All exercises](../README.md)\n'
    (ROOT/r['id']/'README.md').write_text(sheet)
