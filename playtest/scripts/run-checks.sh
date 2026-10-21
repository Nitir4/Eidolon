#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
results="$root/results"
mkdir -p "$results"
cd "$root"
shasum -a 256 -c SHA256SUMS.txt > "$results/integrity.txt" 2>&1 || {
    echo 'Integrity check failed. See results/integrity.txt.' >&2
    exit 1
}
if [ ! -f "$results/comparison.csv" ]; then
    cp comparison-template.csv "$results/comparison.csv"
fi
{
    date -u '+UTC=%Y-%m-%dT%H:%M:%SZ'
    sw_vers
    uname -m
} > "$results/environment.txt"
mountpoint=$(mktemp -d "${TMPDIR:-/tmp}/eidolon-playtest.XXXXXX")
mounted=0
cleanup() {
    if [ "$mounted" -eq 1 ]; then hdiutil detach "$mountpoint" > "$results/detach.txt" 2>&1 || true; fi
    rmdir "$mountpoint" 2>/dev/null || true
}
trap cleanup EXIT
trap 'exit 130' HUP INT TERM
hdiutil attach -readonly -nobrowse -mountpoint "$mountpoint" "$root/app/Eidolon.dmg" > "$results/mount.txt"
mounted=1
app="$mountpoint/Eidolon.app/Contents/MacOS/Eidolon"
[ -x "$app" ] || { echo 'Eidolon executable missing from mounted image.' >&2; exit 1; }
printf 'screen,exit_code\n' > "$results/startup-checks.csv"
index=0
for screen in welcome main lp/input-mode transportation/input-mode assignment/input equations/input games/input integer/input networks/input planning/input queuing/input; do
    # A watchdog belongs only to this new smoke-test process.
    "$app" --smoke-test --screen "$screen" > "$results/startup-$index.log" 2>&1 &
    app_pid=$!
    (
        sleep 30 & timer_pid=$!
        trap 'kill "$timer_pid" 2>/dev/null || true; exit 0' TERM INT HUP
        wait "$timer_pid" || exit 0
        kill -TERM "$app_pid" 2>/dev/null || true
    ) &
    watch_pid=$!
    code=0
    wait "$app_pid" || code=$?
    kill "$watch_pid" 2>/dev/null || true
    wait "$watch_pid" 2>/dev/null || true
    printf '%s,%s\n' "$screen" "$code" >> "$results/startup-checks.csv"
    [ "$code" -eq 0 ] || { echo "Startup failed: $screen. See results/startup-$index.log." >&2; exit 1; }
    index=$((index + 1))
done
echo 'Integrity and 11 screen startups passed. Continue with testing.md for numerical and TORA checks.'
