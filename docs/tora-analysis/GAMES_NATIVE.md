# Native Games integration

The native `gamesModule()` registers `Zero-Sum Games`, using the previously
verified finite-matrix solver and independent graphical envelope backend.
Input accepts 1–100 strategies for each player, title and finite payoffs.
LP-based results show both mixtures, game value and the row-player LP
formulation. Graphical is enabled only for two row-player strategies,
matching the observed original eligibility; it draws every payoff line,
the lower envelope and the chosen probability. Zoom changes the payoff
viewport around its center. Original zoom calibration has not been measured.

`GamesDocument` implements the authenticated original 9000 record order:
format N, format D, notation (1 decimal or 2 scientific), quoted title,
row and column counts, then quoted row-major payoffs. A newly opened document
retains its original bytes until edited. Edited documents use CRLF and quoted
fields; unchanged payoff strings remain unchanged. Input is bounded to 4 MiB,
100 strategies per player and finite numeric payoffs. Extra and truncated
records are rejected. The profile is based on retained original examples;
arbitrary undocumented variants are not claimed compatible.

The controller parses before replacing current data, and saves through
`QSaveFile`. Failed open, invalid payoff and failed solve preserve the current
valid document and result. A successful edit invalidates both calculated
results. Read-only open blocks editing and writing the same canonical source;
Save As to another path unlocks the copy. Dirty departure offers save,
discard or cancellation through the shared injectable dialog contract.

Printing uses QPrinter/QPainter and paginates textual output, placing a graph
on a separate page when applicable. UI uses native QWidget layouts, scrollable
matrix/text areas and a resizable splitter. Numeric result strings use the
format retained in the original file. Newly initialized games offer Decimal/Scientific and N/D controls. Header
selection offers row/column copy, paste, delete, insertion and one-level undo.
Single header click inserts after; double click inserts before, following
verified Transportation and fresh Network instructions. This convention has
not been directly re-observed in Games. Games-specific minimum size
was not observed; the native mathematical profile retains at least one
strategy per player. Original Games-specific EditGrid availability remains
a parity question.

Relevant files: `src/desktop/games_document.*`, `games_controller.cpp`,
`games_screens.*`, and `tests/desktop_games.cpp`. Tests cover an original-form
record, exact unchanged bytes, edited roundtrip, invalid numeric/trailing input,
verified mixed-game output, transactional file failure, read-only Save As,
result invalidation, native graph/zoom, 640×480 navigation, and PDF output.
The final focused build succeeded without warnings. All four registered
Games tests passed at 100%, 125%, 150% and 200% scale in 0.76 seconds
(`build/games/native-build.log`, `native-tests.log`). Native 640×480
graph screenshots and PDF files are retained in `build/dev/tests/` as
`desktop-games-native-scale-*.png` and `.pdf`; 100% and 200% screenshots
were visually inspected for readable text, complete controls and plot. No original program capture, decompilation or workflow execution
was performed for this native integration.


Final native gate18October: Games passes all four scales within the22/22 focused
checks(6.98s). Actual200%Wayland review exposed fixed-width tick-label crowding;
labels now use font metrics, single-line text and reduced tick count on small plots.
The accepted200% graph has separated0/.25/.5/.75/1 labels. Underlying envelope and
mixture calculations are unchanged.
