#!/bin/sh
# Launch by the executable's own path so Qt resolves its adjacent qt.conf.
# The AppImage runtime supplies APPDIR; direct AppDir use works as well.
app_dir=${APPDIR:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}
exec "$app_dir/usr/bin/Eidolon" "$@"
