# Private TORA oracle evidence

Screenshots and TORA-saved files are under tora-captures/ and gitignored.
TORA binaries/runtime are excluded from source and packaging. Publishing any
TORA screenshot/file requires the user's explicit approval.

Gate 0 was approved on 2026-10-04. The 37-pair and 17-pair repair batches
are complete and must not be repeated. The audited corpus has 79 traces,
273 tables and 7,711 numeric cells, plus two expected equality warnings.
Graphical/final studies are 15/15; byte-identical Save As/reopen round trips
are 19/19. See docs/PHASE0_REPORT.md and docs/OPEN_QUESTIONS.md.

Run python3 tools/audit_phase0.py to check saved evidence without launching
TORA. Raw OCR is draft data. Reviewed glyphs and source-hashed transcriptions
are the numerical references; independent golden review remains a module-gate
obligation. Only a specific unresolved question justifies another targeted run.

The oracle driver starts TORA if absent and reuses a pinned, opaque 800×600
Wine desktop. Do not close it between cases or rerun completed capture plans.
