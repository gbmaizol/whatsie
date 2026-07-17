#!/usr/bin/env bash
#
# Build the Whatly snap and (optionally) upload it to the store.
#
#   tools/release-snap.sh                 # build only -> whatly_<ver>_amd64.snap
#   tools/release-snap.sh --release stable   # build, then upload to a channel
#   tools/release-snap.sh --release edge
#
# Requires: snapcraft (classic snap) and an LXD build backend. If LXD is not
# set up, this installs and initialises it. Uploading needs a logged-in
# snapcraft session (`snapcraft login`) — the script does not log you in.
#
# Note for Docker users: Docker's iptables rules block LXD container networking,
# which makes the build fail with "no network access". This script inserts the
# needed DOCKER-USER ACCEPT rules for lxdbr0 when it detects the conflict.
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO"

CHANNEL=""
if [ "${1:-}" = "--release" ]; then CHANNEL="${2:?channel required, e.g. stable}"; fi

need() { command -v "$1" >/dev/null 2>&1; }

# --- LXD backend ------------------------------------------------------------
if ! need lxd; then
  echo "==> Installing LXD"
  sudo snap install lxd
  sudo lxd init --auto
  sudo usermod -aG lxd "$USER"
fi
SG=(sg lxd -c)   # run with the lxd group active without needing a re-login

# --- Docker/LXD iptables conflict -------------------------------------------
if need docker && sudo iptables -L DOCKER-USER -n >/dev/null 2>&1; then
  if ! sudo iptables -C DOCKER-USER -i lxdbr0 -j ACCEPT 2>/dev/null; then
    echo "==> Allowing lxdbr0 forwarding past Docker's firewall"
    sudo iptables -I DOCKER-USER -i lxdbr0 -j ACCEPT
    sudo iptables -I DOCKER-USER -o lxdbr0 -j ACCEPT
  fi
fi

# --- Build ------------------------------------------------------------------
echo "==> Building the snap"
rm -f whatly_*.snap
"${SG[@]}" "cd '$REPO' && snapcraft pack"
SNAP=$(ls -1 whatly_*.snap | head -1)
echo "Built: $SNAP"

# --- Upload -----------------------------------------------------------------
if [ -n "$CHANNEL" ]; then
  echo "==> Uploading to '$CHANNEL'"
  snapcraft whoami >/dev/null || { echo "Run 'snapcraft login' first."; exit 1; }
  snapcraft upload "$SNAP" --release="$CHANNEL"
  echo "Released $SNAP to $CHANNEL."
fi
