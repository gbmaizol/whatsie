#!/usr/bin/env bash
# Integration coverage: drive the real (coverage-instrumented) binary headless,
# without logging in to WhatsApp Web. It exercises the CLI, app bootstrap, the
# main window, the Qt WebEngine setup (which loads the QR/link page) and the
# dialogs reachable over the single-instance IPC (settings, about, scheduled…).
#
# Build first:  cmake -B build -DWHATLY_COVERAGE=ON && cmake --build build --target whatly
# Then:         tools/integration.sh [build-dir]
#
# Everything runs under a throwaway HOME so it never touches your real data or
# WhatsApp session. Login-gated features (sending, chats) are out of scope.
set -uo pipefail
cd "$(dirname "$0")/.."
BUILD=${1:-build}
BIN="$(pwd)/$BUILD/whatly"
[ -x "$BIN" ] || { echo "Build the instrumented binary first: $BIN"; exit 1; }

TMP=$(mktemp -d)
export HOME="$TMP"
export XDG_CONFIG_HOME="$TMP/.config" XDG_DATA_HOME="$TMP/.local/share" \
       XDG_CACHE_HOME="$TMP/.cache" XDG_RUNTIME_DIR="$TMP/run"
mkdir -p "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"
export QT_QPA_PLATFORM=offscreen
export QTWEBENGINE_CHROMIUM_FLAGS="--no-sandbox --disable-gpu"

run() { # args, timeout
  timeout "${2:-20}" "$BIN" $1 >>"$TMP/out.log" 2>&1
  echo "  [exit $?] whatly $1"
}

echo "== CLI (each returns from main and flushes coverage) =="
run "--version"
run "--build-info"
run "--help"
run "--unread"
run "--migrate-from=whatsie --dry-run"

echo "== Driven GUI session (loads WhatsApp Web QR page; no login) =="
timeout -s TERM 50 "$BIN" >>"$TMP/out.log" 2>&1 &
APP=$!
sleep 12   # let the window build and WhatsApp Web load

# Drive the running instance over its single-instance IPC. Each of these is a
# short-lived secondary process that sends a command and exits.
for a in "-w" "-s" "-i" "--open-scheduled" "-t" "-r"; do
  run "$a" 12
  sleep 1
done

sleep 2
kill -TERM "$APP" 2>/dev/null
wait "$APP" 2>/dev/null
echo "  [main instance quit gracefully]"

rm -rf "$TMP"
echo "== done =="
