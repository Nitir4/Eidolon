# Transportation number settings

The observed Transportation profile is implemented in local source, with
Decimal/Scientific notation and N/D stored on the mathematical model.
Three independently reviewed original profiles cover Decimal6/3,
Scientific5/2 and Scientific2/3. Their authentic CRLF files roundtrip byte for
byte;159initial/final display fields match original transcriptions.

Decimal D controls decimal places. Scientific N controls mantissa decimals,
D exponent width. Default is Decimal5/2. Original header marker4000 is
followed by N,D,notation1(decimal)or2(scientific). The codec preserves these
record spellings when opening, editing, structurally changing and saving.
The native precision limit is99, shared with the existing formatter; arbitrary
legacy extrema and crashing settings are not reproduced.

New scientific costs initialize as formatted zero. Committed costs normalize
to selected precision and are reparsed as stored input. Typed supplies/demands
round to ordinary whole numbers. Imported untouched fractions retain their
lexical values. Real keyboard probes confirmed this quantity behavior; direct
WM_CHAR-only probes had dropped-character artifacts and are not specifications.

Objectives, costs, reduced values and normal multipliers use the selected
format. Shipments and supply/demand in tableaus remain ordinary strings. Final
amounts retain their previously observed display rule; cost/contribution use
the selected format. Original printed input formats quantities too; scientific
printed input therefore differs from its tableau quantities.

Execution: Problem.format → TransportationDocument header/cost normalization
→ controller initialization/open/Undo → display/final summary/printing.
Invalid format validation occurs before replacing input. Loaded settings survive
reinitialization. Scientific printing uses95%of printable width so u/v labels
remain visible; this is a native readability policy, not legacy pixel matching.

Local final checks:106/106desktop CTests in104.12s,40/40Qt-free in4.59s.
Printing corpus now14workflows/19jobs/34pages/356orderedrows/1444tokens.
Actual Wayland acceptance passed31views each at100/200%on1920x1080physical.
Smallest physical1366x768at200%was not newly checked in this cycle.
Evidence:build/transportation-settings/. Earlier failed default-only safety
assertion and stale/missing-fixture attempts are retained separately.

REA decompiled the print handler0x4b2a40 with Ghidra12.1.4:
ev_506a6b7af44ab265016f7c3f67325de8fa4fe4dd4c1145d01cff25f9369a6951.
It exposes integer-formatting machinery, but uncertain VB types and an unresolved
terminal-flow boundary prevent treating pseudocode as recovered original source.
Two larger quantity handlers failed; no completed decompilation is claimed for them.
Raw output/binary remain private under reference/tora-native/.

Settings are not packaged/installed or cross-platform CI verified. The installed
Linux launcher remains source dc853b4. The earlier printing Windows gate is
incomplete. GitHub Actions remain disabled and require specific approval.
