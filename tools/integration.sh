#!/usr/bin/env bash
# Integration coverage: drive the real (coverage-instrumented) binary headless,
# without logging in to WhatsApp Web. It exercises the CLI, app bootstrap, the
# main window, the Qt WebEngine setup (which loads the QR/link page), the tray,
# the lock screen and the dialogs reachable over the single-instance IPC. To
# reach more startup branches it launches several sessions with different
# pre-seeded settings (tray style, theme, zoom, minimized, no-tray).
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
mkdir -p "$XDG_CONFIG_HOME/shakaran" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"
export QT_QPA_PLATFORM=offscreen
export QTWEBENGINE_CHROMIUM_FLAGS="--no-sandbox --disable-gpu"

CONF="$XDG_CONFIG_HOME/shakaran/whatly.conf"

seed() { printf '[General]\n%b\n' "$1" > "$CONF"; }

run() { # args, timeout
  timeout "${2:-20}" "$BIN" $1 >>"$TMP/out.log" 2>&1
  echo "  [exit $?] whatly $1"
}

session() { # label, seconds, extra-ipc-commands...
  local label=$1 secs=$2; shift 2
  echo "== session: $label =="
  timeout -s TERM $((secs + 20)) "$BIN" >>"$TMP/out.log" 2>&1 &
  local app=$!
  sleep "$secs"
  for a in "$@"; do run "$a" 12; sleep 1; done
  sleep 1
  kill -TERM "$app" 2>/dev/null
  wait "$app" 2>/dev/null
  echo "  [quit] $label"
}

echo "== CLI (each returns from main and flushes coverage) =="
rm -f "$CONF"
run "--version"; run "--build-info"; run "--help"; run "--unread"
run "--migrate-from=whatsie --dry-run"

# Default session, driven over IPC through every reachable command.
rm -f "$CONF"
session "default+ipc" 12 "-w" "-s" "-i" "--open-scheduled" "-t" "-r" "-l" "-w"

# Monochrome tray, dark theme, zoomed out, follows the system theme.
seed "monochromeTrayIcon=true\nwindowTheme=dark\nfollowSystemTheme=true\nzoomFactor=0.75\nmuteAudio=true\nnotificationCombo=1\nminimizeOnTrayIconClick=true"
session "themed+tray" 10 "-t" "-s"

# No tray icon, a different widget style, started minimized.
seed "hideTrayIcon=true\nwidgetStyle=Fusion\nstartMinimized=true\ndisableNotificationPopups=true\ninterfaceFontSize=11"
session "notray+minimized" 10 "-w"

# Locked at startup: a password is set and lockscreen=true, so the lock screen
# (and the More-Apps widget it hosts) is shown on launch.
seed "asdfg=MTIzNDU=\nlockscreen=true\nappAutoLocking=true\nautoLockDuration=1"
session "locked" 10

# Rich feature settings + a dictionary fixture, so opening Settings (-s)
# populates the spell-check list, theme, automatic-theme and other controls.
DICT="$TMP/dicts"; mkdir -p "$DICT"
for d in en_US es_ES fr_FR; do printf 'BDIC' > "$DICT/$d.bdic"; done
export QTWEBENGINE_DICTIONARIES_PATH="$DICT"
seed "spellCheckEnabled=true\nspellCheckLanguages=en_US, es_ES\nautomaticTheme=true\nsmoothScrolling=true\nfollowSystemTheme=true\nmonochromeTrayIcon=true"
session "richsettings" 10 "-s" "-i"
unset QTWEBENGINE_DICTIONARIES_PATH

rm -rf "$TMP"
echo "== done =="
