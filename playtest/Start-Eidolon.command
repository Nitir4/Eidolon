#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ -d "$root/app/Eidolon.app" ]; then
    open "$root/app/Eidolon.app"
else
    open "$root/app/Eidolon.dmg"
fi
