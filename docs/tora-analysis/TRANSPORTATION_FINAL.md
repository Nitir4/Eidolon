# Transportation final summary

Status: seventeen complete independently transcribed final summaries; native
final-summary implementation is deferred. Heading:
TRANSPORTATION MODEL OUTPUT SUMMARY. Title, Final Iteration No. and
Objective Value (minimum cost) precede a shipment table.

The pilot reports iteration 1, objective 150.00. Exact column labels:
From, To, Amt Shipped, Obj Coeff, Obj Contrib. Rows:

| From | To | Amt Shipped | Obj Coeff | Obj Contrib |
| --- | --- | --- | --- | --- |
| S1: | D1: | 5 | 2.00 | 10.00 |
| S1: | D3: | 15 | 1.00 | 15.00 |
| S2: | D1: | 5 | 5.00 | 25.00 |
| S2: | D2: | 25 | 4.00 | 100.00 |

Only positive shipments appear in this pilot, in source/destination order.
Next Iteration and All Iterations are disabled. Write to Printer and the
usual View/Modify Input Data, MAIN Menu, Exit TORA controls are present.
The output grid has a horizontal scroll control.

## Additional observed discrepancies

The default final route reports iteration 1 on the captured inputs. In case
14, alternative optima have different terminal bases: North-west/Least-cost
and Final share one basis, while Vogel reaches another with the same 461.00
objective. Thus a future final presenter cannot blindly reuse Vogel's basis.

Case 08 has fractional terminal shipments 0.75,1.75,0.75,2.75 with objective
20.56; the final summary instead visibly shows amounts 0,2,0,3 while objective
contributions remain 1.69,3.06,4.13,11.69. In case 16, fractional dyadic
shipments all display amount 0 in the final summary, with nonzero contributions
and total 0.42. These are original display discrepancies, not corrected or
invented transcription. A general rounding rule has not been isolated.

## Unknowns / things I could not verify

How the final route chooses its start in general; alias/code-page behavior;
a complete fractional amount-format policy; alternate-optimum notification;
imbalance/invalid models; other scientific/decimal settings; printing and
large-table pagination. This native slice shows iteration output only and
makes no claim to reproduce final-summary display quirks yet.
