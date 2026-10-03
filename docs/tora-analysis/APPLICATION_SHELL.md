# Startup and main-menu screens

## Purpose and workflow

TORA Windows-version 1.00, June 2002 displays a blue welcome screen with
`Click Here`, then the centered MAIN popup. Its module order is Linear
Equations, Linear Programming, Transportation model, Integer programming,
Network models, Project Planning, Queuing analysis, Zero-Sum Games, EXIT TORA.
LP opens input mode. Other modules were not studied or implemented.
Evidence: `smoke/basic-lp-800/05-installed-main-menu.png` and the startup/
welcome captures under `methods/lp/corpus/`.

The reference checks its legacy screen size; 800×600 works reliably enough
for LP observations under Wine 11.18. It rejected the user's native high
resolution. This display restriction is not a clone requirement: the native
Qt application must support the stated native resolution/DPI matrix.

## Controls, navigation and file behavior

These screens contain no model fields or file commands. LP input mode chooses
new/open. LP screens offer MAIN Menu and Exit TORA; returning from an input
grid prompts for saving. Screen-specific labels, controls and warnings are
in the adjacent documents. No toolbar has been observed on these screens.

## Unknowns / things I could not verify

Startup keyboard accelerators/default focus, all exit/cancel states, and the
1024×768 path after the final oracle setup. Fonts and artwork are observed
visually only; no TORA icons, binary resources or help text were extracted.
