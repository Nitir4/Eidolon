# Resumption checkpoint — 2026-10-04

State: Gate 0 approved by the user on 2026-10-04 (yes please go ahead).
Foundation 1 is complete. The user changed the sequence to 1 → 5 → 2 → 3 → 4;
Foundation 5 is active and must stop for review before Foundation 2 begins.
See STATUS.json and FOUNDATION5_REPORT.md for current build/CI status.
This checkpoint preserves the completed reference work;
it is not an instruction to restart Phase 0.

Completed corpus work must not be repeated. The 37-pair and 17-pair plans both
have pairs=[] and explicit completed status. All 79 traces pass coverage;
273 tables, 7,711 numeric cells, plus two equality warnings. Graph/final
studies are 15/15; TORA Save As/reopen 19/19 byte-identical. Manual alternate,
format, M, grid, file framing, graph eligibility/zoom and print probes are
saved. No GUI batch is left running by this checkpoint.

To check saved evidence without GUI input, run:

    python3 tools/audit_phase0.py

This is an evidence audit, not user approval or a product test. Remaining
unknowns are tracked by IDs Q-01…Q-10 in OPEN_QUESTIONS.md; they are not a
request to rerun the main corpus. Only targeted new observations justified by
a concrete unresolved question should launch the oracle again.

The observation driver starts TORA if missing, otherwise reuses it. Its desktop
is 800×600 under Wine and pinned/opaque; do not kill it after each case.
The accepted GUI prefix is python3 tools/run_tora_corpus.py. Keep all original
TORA assets/captures private. The original unfocused grid and appended-M probes
are marked invalid/limited; use closing-20261004/grid-focused and m-validated-*
for reviewed results. Successful Undo and successful printing are unverified.
