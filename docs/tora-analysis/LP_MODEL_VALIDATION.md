# LP model and validation foundation

This defines the clone's domain model, based on existing Phase 0 input captures
and focused observations of the running TORA reference. It does not describe
TORA internals. The 15-case corpus was not rerun. Raw screenshots and saved
files remain private/gitignored; only original numerical transcriptions use
the committed test grammar in `tests/fixtures/lp-models.txt`.

## Purpose and data contract

`tora::lp` is a Qt-free C++17 static library. `LpProblem` owns its title,
Maximize/Minimize direction, objective vector, ordered variables and ordered
constraints. A constraint holds one coefficient per variable, <=/>=/= and
RHS. A variable holds its optional user alias, lower bound, upper bound and
unrestricted flag. Generated x1/x2 headers remain independent of aliases;
constraint labels are Constr 1 onward. Checked label lookups and value copies
protect consumers from invalid indices and accidental snapshot changes.

`blank(n,m)` checks counts before allocation. Defaults are empty title/aliases,
Maximize, zero coefficients/RHS, <= rows, lower bound zero, upper bound
positive infinity and unrestricted false. Counts must be 1..100 on each axis
for compatibility with this observed Student edition. The old 100/100 grid
and 101/101 warning are reused; no large-model solving claim is made.

Arithmetic values are binary64, using Foundation 2's Scalar. Validation never
formats, rounds, snaps near zero, flips a negative RHS, removes constraints or
expands unrestricted variables. Original aliases/bounds/flags are retained.
Aliases are opaque strings here: encoding, length limits, duplicate-name UI
behavior and parsing belong to their data-entry/file gates. Display settings,
source lexemes, filename and dirty/read-only state belong to ProblemDocument
and the Foundation 4 codec, which are not implemented at this gate.

## Directly observed validation

| Input | TORA observation | Foundation behavior |
| --- | --- | --- |
| Zero variables; zero variables and constraints | Problem Size / Invalid problem size | Located dimension error, factory refuses allocation |
| Two variables, zero constraints | Same Problem Size / Invalid problem size | NoConstraints error with observed wording |
| 100 variables / 100 constraints | Input grid initializes | Accepted dimensions |
| 101 variables / 101 constraints | Problem size / Student version limits maximum size to 100 | Student edition cap applied to both dimensions |
| Lower=upper=1, unrestricted n | Output-format screen reached | Equal bounds accepted |
| Lower=3, upper=1, unrestricted n | Input Data Error | InconsistentBounds error |
| Lower=-1, upper=infinity, unrestricted n | Output-format screen reached | Signed lower bound retained |
| Lower=-3, upper=-1, unrestricted n | Saved by TORA; Solve/Modify menu reached | Signed finite pair retained |
| Lower=0, upper=infinity, unrestricted y | Output-format screen reached | Flag retained without expansion |
| Lower=3, upper=1, unrestricted y | Same Input Data Error | Flag does not bypass consistency |

The exact two-line reversed-bound body is:

```text
Lower bound exceeds upper bound for variable 1.
Reenter correct bounds
```

The clone uses the variable's one-based index in that diagnostic. Its issue
location is zero-based for callers. Arbitrary later-variable wording and a
near-equal numeric threshold are not independently captured: index substitution
and strict binary64 comparison are explicit engineering generalizations of
this observed rule. No solver tolerance is applied to model bounds.

An earlier one-variable/zero-constraint frame was blank without a usable grid;
it was inconclusive. A settled two-variable/zero-constraint follow-up directly
captured the warning above. The negative-upper algorithm-format frame was
incomplete; saved input and the Solve/Modify menu establish input acceptance,
not a solver transform or result. GUI driving needed one Save As retry and
explicit Wine-window positioning. The existing TORA instance was reused and
left running. Completed Phase 0 traces were not repeated or overwritten.

## Validation results and engineering checks

`LpValidator::validate(const LpProblem&)` returns all issues in stable order:
dimensions, objective direction/shape/values, variables/bounds, then each
constraint's shape/relation/values/RHS. Each issue has a code, severity, origin,
optional variable/constraint location, dialog title and message. A report is
valid when it has no errors. ObservedTora identifies directly captured wording;
Engineering identifies new safety diagnostics, not claimed legacy text.

Finite objective coefficients, constraint coefficients and RHS are required.
Lower bounds must be finite; upper bounds may be finite or positive infinity.
NaN, negative-infinite upper bounds, infinite lower bounds, malformed enum
values and vector-width mismatches receive engineering diagnostics. Values
inside ragged vectors are checked safely without indexing another vector.
These checks do not establish TORA's text parser, overflow paths or malformed
file behavior. Positive infinity is a bound endpoint, not a matrix cell.

Mathematically infeasible, unbounded, degenerate and alternative-optimum models
remain valid inputs. Their solver status, method eligibility, warnings and
termination are not input-validation errors. There is no feasibility solver,
preprocessor, codec, document controller or GUI binding at this foundation.

## Tests and unknowns

Tests were written before the implementations: configuring with the new static
target initially failed on the deliberately absent problem.cpp. `lp.model`
compares 22 original model transcriptions, preserving ordered input fields and
defaults, labels and independent copies. `lp.validation` covers exact observed
messages, before-allocation count guards, nonfinite/structural failures,
aggregate locations/order, bounds/signs and non-mutating binary64 values.
Both run without Qt and in the existing three-OS workflow.

Unrestricted expansion, negative bound complements, fixed-variable elimination,
near-equal consistency thresholds, method eligibility and bound-related
sensitivity need their preprocessing/solver gates (Q-06/Q-09). Name/text
validation and keyboard/edit behavior need the data-entry gate (Q-03). File
encoding/parsing/round trips need Foundation 4 (Q-04). None is guessed here.

## Capture provenance

Paths below are relative to `reference/tora-captures/`. SHA-256 identifies the
locally reviewed source. The three probe-fixtures inputs were observed in
`methods/lp/corpus/<case>/all_slack/input.png`; they are not falsely described
as TORA Save As outputs. Other source rows are actual saved round-trip files.

| Original fixture case | Source | SHA-256 |
| --- | --- | --- |
| 01_normal | roundtrips/01_normal/tora-saved.txt | `3bbf4f898538072795fd666c1618ea44c6dde6c04bcbebf313c5aed66ee76328` |
| 02_degenerate | roundtrips/02_degenerate/tora-saved.txt | `735e386af0557ada3faf38d619b1f5f4d81d85291ccab95ad645b81468b4fb38` |
| 03_infeasible | roundtrips/03_infeasible/tora-saved.txt | `28a59710760bbbfd459f753dad0875d214885c312a486188493dcff765838a45` |
| 04_unbounded | roundtrips/04_unbounded/tora-saved.txt | `3eeb362a16139539e97df3aab206cd2a6d516f72fd19021a28aaf88418237932` |
| 05_alternative | roundtrips/05_alternative/tora-saved.txt | `1069b43fa6ed6ba726fdbf29d56cba402aba664f0d9d3990cf0d5f3cbe13f96b` |
| 06_equality | roundtrips/06_equality/tora-saved.txt | `b62e261d3d0932eee48250fc014ab6aef01fea27b91c89340b5d3c423311118f` |
| 07_big_m | roundtrips/07_big_m/tora-saved.txt | `f95b9c249861b19f288d7d055b37625999486986da9e428c12abf593e7f1ef63` |
| 08_two_phase | roundtrips/08_two_phase/tora-saved.txt | `b88e03af5d01f2af582e05857a32381fdc009a270273b3f023b3497baa20070f` |
| 09_bounded | roundtrips/09_bounded/tora-saved.txt | `0fd43bf03c872da24d224eb78ac6591f345ef81897841181f3e8ca8efe35ac81` |
| 10_dual | roundtrips/10_dual/tora-saved.txt | `7b1101ec5feac17f11d69904ddb77a24419af5a190a3026b760454c8723cfa7f` |
| 11_zero_rhs | roundtrips/11_zero_rhs/tora-saved.txt | `b3b3e7c6f6e0b141b5facb637d61ee7bbf926ed15b0a889c11624880c95c12ec` |
| 12_negative_rhs | roundtrips/12_negative_rhs/tora-saved.txt | `09404a38bf49155bee8581a16a456884e563d2eafba3e9c349e289ecbb5719af` |
| 13_mixed_sign | roundtrips/13_mixed_sign/tora-saved.txt | `cd216384018dcdc627fd888d9f4abd8822aec61ed47e7ac8f961fa9d6d748cb2` |
| 14_tight_bounds | roundtrips/14_tight_bounds/tora-saved.txt | `60bae5d8c8f748de90d4ae8872430103fedc2aa5724b7f6c1f884a10c034fba3` |
| 15_minimization | roundtrips/15_minimization/tora-saved.txt | `0abb3efdf85d2b33fad5bcbdf24162e96a6b04a134e896f6a53028e79088854d` |
| finite-upper | probe-fixtures/finite-upper.txt | `7384b0b42fe783f73b97949ac86b3522ca8a35f5de6bf9d4f82ddd45b55c4260` |
| finite-two-upper | probe-fixtures/finite-two-upper.txt | `29cc2e3c4d2cfc392e122fded02ecb8de3dcb0ea41b4f09cde038c3b9b9a8bd6` |
| finite-lower | probe-fixtures/finite-lower.txt | `e50985aba6253d8864335fc335dbb252922ea83ca896f1bd4f0b4430f1ca49b1` |
| probe-1var-1con | roundtrips/probe-1var-1con/tora-saved.txt | `8e0f06aed579442dd172969af7cc2f371c3a0bcc307036ea0730703f75cf4395` |
| probe-3var-2con | roundtrips/probe-3var-2con/tora-saved.txt | `dce41af6d3e9dab994a4313ae0c30f22d923f57d650b03a0f7c54b4d66c0c4a8` |
| probe-scientific-flag2 | roundtrips/probe-scientific-flag2/tora-saved.txt | `c629b6425f1a0451e279ee26d38bf64c3ef063d2670b0f3c1e0210985941ac41` |
| probe-numeric-digits | roundtrips/probe-numeric-digits/tora-saved.txt | `563803102c144811696cfc5e8ebb01bb11ddaf941b57ba767a3e3e5b0b5848d1` |

| Targeted observation | Evidence frame/file | SHA-256 |
| --- | --- | --- |
| zero-variables | probes/foundation3/dimensions/zero-variables/after-initialize.png | `ae81c2898ca57184e0988075c2f0537fe8d85b5eb9d2d2239597b918e9f72c01` |
| zero-both | probes/foundation3/dimensions/zero-both/after-initialize.png | `170552390e86de501a88cfce2b97a9de17a494c33fc2afc8a55997a6eded4b69` |
| zero-constraints confirmed | probes/foundation3/dimensions/zero-constraints-two/after-initialize.png | `d20e81517e3599be09a915081399f05fd3d779f70f13c0972fa25e518c6f7846` |
| equal-bounds | probes/foundation3/bounds/equal-bounds/method-validation.png | `a76f2302dbb258f95b883fe853a5e02f033e3c22259a5f643a112cee3a3d982d` |
| reversed-bounds | probes/foundation3/bounds/reversed-bounds/method-validation.png | `a60502ee9acab05998f6ed765aff290e969d7b4c250a4644b5452674627c2533` |
| negative-lower | probes/foundation3/bounds/negative-lower/method-validation.png | `ba39412f36fea12a3a378b1d031e144628c212a4c0246c56d7281ab2e639f7f0` |
| free-default-bounds | probes/foundation3/bounds/free-default-bounds/method-validation.png | `81701d1d3914c18a821f841710508b5bd9834d60f685dddeb7c52b534b6d8e47` |
| free-finite-bounds | probes/foundation3/bounds/free-finite-bounds/method-validation.png | `ec5416d47c9d7a1348cc8d7ef0863755efb0f66d5c09b8bbdca6f32fd9442279` |
| negative-upper input acceptance | probes/foundation3/bounds/negative-upper/solve-selection.png | `bf19168815866ca76dd58a184f03542ba66addd35a0c2425baf2669766b9b53a` |
| equal-bounds saved input | probes/foundation3/bounds/equal-bounds/tora-saved.txt | `5299ae17a730edfadb24d779a9c75fb45e2013edfee42d9338f943639b820298` |
| reversed-bounds saved input | probes/foundation3/bounds/reversed-bounds/tora-saved.txt | `2419021157a377234d9deb20685497ec2196857dccefb9af49159ff080a2e344` |
| negative-lower saved input | probes/foundation3/bounds/negative-lower/tora-saved.txt | `b16c8e72c60c408d5c7b9bb63ef954187e2132cfd63a9978d2865e464063bfe5` |
| negative-upper saved input | probes/foundation3/bounds/negative-upper/tora-saved.txt | `071c6c5e4ee9957a4459d57d429b4dbd6c2c533329c29d7da58455976681267a` |
| free-default-bounds saved input | probes/foundation3/bounds/free-default-bounds/tora-saved.txt | `e534cbd74dec07d8f80b6f763f7ec3587406b48dde935a535647f196c193490b` |
| free-finite-bounds saved input | probes/foundation3/bounds/free-finite-bounds/tora-saved.txt | `d79bcf48878f30cbe4343e7c396566071841b61429a7e31119842774fa1401e5` |
