"""Generate a fresh comparison sheet and original-workflow equivalents."""
import csv
import json
import math
from pathlib import Path


def original_death_mean(population, rate):
    """Arithmetic recovered from the reference's unit-time native routine."""
    count = min(population, 500)
    term, total, before_peak = math.exp(-rate), 0.0, True
    for k in range(1, count):
        term = rate*term/k
        total += (count-k)*term
        if term >= .00001:
            before_peak = False
        elif not before_peak:
            break
    return total


def metrics(record):
    family, expected = record['family'], record['expected']
    if 'after' in expected:
        expected = expected['after']
    if family in ('lp', 'integer', 'assignment'):
        return {'objective': expected['objective']}
    if family == 'transportation':
        return {'optimal_cost': expected['optimal_cost']}
    if family == 'mst':
        return {'total_weight': expected['total_weight']}
    if family == 'route':
        return {'distance': expected['distance']}
    if family == 'flow':
        return {k: expected[k] for k in ['max_flow', 'min_cut']}
    if family == 'planning':
        values = {'project_duration': expected['project_duration']}
        values.update({f'activity_{i+1}_float': x for i, x in enumerate(expected['total_float'])})
        return values
    if family in ('equations', 'matrix'):
        if family == 'equations':
            return {f'x{i+1}': x for i, x in enumerate(expected['solution'])}
        from fractions import Fraction
        return {f'inverse_{i+1}_{j+1}': float(Fraction(x))
                for i, row in enumerate(expected['inverse_exact']) for j, x in enumerate(row)}
    if family == 'game':
        return {'value': expected['value'], **{f'row_probability_{i+1}': x for i, x in enumerate(expected['row_probabilities'])},
                **{f'column_probability_{i+1}': x for i, x in enumerate(expected['column_probabilities'])}}
    if family == 'queue':
        keys = ['mean', 'P_no_events'] if 'mean' in expected else ['P0', 'L', 'Lq', 'W', 'Wq', 'lambda_effective']
        return {k: expected[k] for k in keys}
    raise ValueError('Unsupported practice family: ' + family)


def make_material(bank_root, destination):
    bank = json.loads((bank_root/'bank.json').read_text())
    equivalents = destination/'tora-equivalents'
    equivalents.mkdir()
    quote = lambda x: '"' + str(x).replace('"', '""') + '"'
    rows, equivalent_index = [], {}
    for record in bank['cases']:
        model, family, name = record['model'], record['family'], record['id']
        note = ''
        if 'changed' in model:
            # Native LP examples have 2 variables and columns [label,x1,x2,relation,RHS]
            # encoded at a four-column numeric grid width in these fixtures.
            lines = (bank_root/record['input']).read_bytes().decode('ascii').splitlines()
            changed = model['changed']
            lines[4] = quote(name+' changed')
            for j, value in enumerate(changed['objective']):
                lines[12+j] = quote(value)
            for i, (_, _, rhs) in enumerate(changed['constraints']):
                lines[8+(i+2)*4+3] = quote(rhs)
            path = equivalents/(name+'.txt')
            path.write_bytes(('\r\n'.join(lines)+'\r\n').encode('ascii'))
            note = 'TORA: solve the supplied changed-model equivalent; revised trace is not available there.'
            equivalent_index[name] = path.name
        elif family == 'assignment':
            costs = model['costs']
            count = max(len(costs), len(costs[0]))
            sign = -1 if model['sense'] == 'max' else 1
            square = [[sign*costs[i][j] if i < len(costs) and j < len(costs[0]) else 0
                       for j in range(count)] for i in range(count)]
            grid = [['S/D Name', *[f'J{j+1}' for j in range(count)], 'Supply']]
            grid += [[f'W{i+1}', *row, 1] for i, row in enumerate(square)]
            grid += [['Demand', *[1]*count, '']]
            lines = ['4000', '5', '4', '1', quote(name+' equivalent'), str(count), str(count)]
            lines += [quote(x) for row in grid for x in row]
            path = equivalents/(name+'.txt')
            path.write_bytes(('\r\n'.join(lines)+'\r\n').encode('ascii'))
            note = 'TORA: unit Transportation equivalent; record raw minimum cost (negated profits for maximization).'
            equivalent_index[name] = path.name
        for workflow in record['workflows']:
            for metric, expected in metrics(record).items():
                original, reason = expected, note
                if family == 'assignment' and model['sense'] == 'max':
                    original = -expected
                if family == 'queue' and name.startswith('queue-finite-kn'):
                    attempt = model['arrival']*(model['N']-record['expected']['L'])
                    if metric == 'lambda_effective':
                        original = attempt
                    elif metric in ('W', 'Wq'):
                        original = record['expected']['L' if metric == 'W' else 'Lq']/attempt
                    if metric in ('lambda_effective', 'W', 'Wq'):
                        reason = 'Known original attempted-arrival denominator; Eidolon uses admitted arrivals.'
                if family == 'queue' and name.startswith('queue-pure-death') and metric == 'mean':
                    original = original_death_mean(model['initial_population'], model['rate'])
                    reason = 'Known original omitted initial state plus 1e-5 cutoff; compare each app to its own key.'
                rows.append(dict(workflow=workflow, case=name, metric=metric,
                                 expected_eidolon=format(expected, '.15g'), expected_tora=format(original, '.15g'),
                                 absolute_tolerance='0.000051', eidolon='', tora='', notes=reason))
    fields = ['workflow', 'case', 'metric', 'expected_eidolon', 'expected_tora',
              'absolute_tolerance', 'eidolon', 'tora', 'notes']
    with (destination/'comparison-template.csv').open('w', encoding='utf-8', newline='') as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    (equivalents/'index.json').write_text(json.dumps(equivalent_index, indent=2)+'\n')
    return dict(cases=len(bank['cases']), workflows=len(bank['workflows']),
                workflow_cases=sum(len(r['workflows']) for r in bank['cases']), scalar_rows=len(rows))
