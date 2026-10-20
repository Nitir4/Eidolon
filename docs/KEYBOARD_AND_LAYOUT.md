# Keyboard entry and responsive tables

Press Tab to focus a control, then Enter to activate it. Enter also opens a focused
noneditable dropdown or a button's menu; use arrows and Enter to choose an item.

For editable table cells:

| Action | Key |
|---|---|
| Select a cell | Single click or arrow keys |
| Replace its contents | Start typing |
| Edit the existing contents | F2 or double click |
| Move down; save first when editing | Enter |
| Move up; save first when editing | Shift+Enter |
| Move to next editable cell; save first when editing | Tab |
| Move to previous editable cell | Shift+Tab |
| Cancel the current edit | Escape |
| Copy selected rectangle to the clipboard | Ctrl+C (Command+C on macOS) |
| Paste a tab-separated rectangle at the current cell | Ctrl+V (Command+V on macOS) |

F2 keeps arrow keys inside the editor for moving the caret. Navigation skips labels,
protected cells and hidden rows/columns. At a table boundary the cell stays selected;
it does not wrap around or open another editor. Shift+arrow extends the native table
selection. Menus retain their module-specific row/column commands and shortcuts.

Clipboard input must be rectangular and fit within editable cells. A block crossing
labels, protected cells or a table boundary is rejected before writing any values.
Model-rejected values roll back preceding accepted writes where the model supports
rejection. Some modules retain invalid text as a draft and validate it when solving;
correct any reported draft errors before solving or saving. The clipboard contains
values, not formulas, styles or a spreadsheet workbook.

Small LP input tables end at their actual last row. Larger tables use the available
space and scroll internally. LP tableau/final-summary screens use the full content
width and remaining height. The tableau's iteration instructions can be expanded
when needed. The first Basic column stays visible while scrolling horizontally.
Very small windows retain the outer page scrollbar to keep controls reachable.

[Practice bank](../examples/practice-bank/README.md): at least five exercises per
workflow, with input files and independent answer keys. Original TORA comparisons
are pending; use the included comparison sheet when starting that session.
