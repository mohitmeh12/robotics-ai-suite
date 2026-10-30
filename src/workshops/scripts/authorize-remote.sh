#!/bin/bash
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# Let <monitor> workshop monitor <target> workshop with the Robotics System Profiler's remote mode:
# passwordless SSH (monitor key -> target authorized_keys), the remote psutil probe, and SSH listening.
# Runs on the host because each workshop only mounts its own project directory.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)   # src/
MONITOR=${1:?usage: $0 <monitor-workshop> <target-workshop>}
TARGET=${2:?usage: $0 <monitor-workshop> <target-workshop>}

project_dir() {
  local f
  f=$(grep -lxE "name: $1" "$ROOT"/workshops/*/workshop.yaml "$ROOT"/workshops/*/.workshop/*.yaml \
    "$ROOT"/components/*/workshop/.workshop/*.yaml 2>/dev/null | head -1)
  [ -n "$f" ] || { echo "no workshop definition named $1" >&2; exit 1; }
  case $f in */.workshop/*) dirname "$(dirname "$f")" ;; *) dirname "$f" ;; esac
}
wx() { local w=$1; shift; (cd "$(project_dir "$w")" && workshop exec "$w" -- "$@"); }

KEY=$(wx "$MONITOR" bash -c '[ -f ~/.ssh/id_ed25519 ] || ssh-keygen -q -t ed25519 -N "" -f ~/.ssh/id_ed25519 -C "$(hostname)"; cat ~/.ssh/id_ed25519.pub')
PROBE=$(wx "$MONITOR" bash -c 'cat /opt/ros/*/benchmarking/src/_psutil_probe.py')

wx "$TARGET" bash -c '
  set -e
  if ! command -v sshd >/dev/null; then
    set -a; . /etc/environment; set +a
    sudo -E apt-get install -y -q openssh-server python3-psutil >/dev/null
  fi
  sudo systemctl enable --now ssh.socket >/dev/null 2>&1 || sudo systemctl enable --now ssh >/dev/null
  install -d -m 700 ~/.ssh
  touch ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys
  grep -qxF "$1" ~/.ssh/authorized_keys || echo "$1" >> ~/.ssh/authorized_keys
  # monitor_resources.py expects the probe at ~/ros-kpi/src/_psutil_probe.py on the remote.
  mkdir -p ~/ros-kpi/src && printf "%s\n" "$2" > ~/ros-kpi/src/_psutil_probe.py
' _ "$KEY" "$PROBE"

IP=$(wx "$TARGET" bash -c "ip -4 -o addr show eth0 | awk '{print \$4}' | cut -d/ -f1")
# A rebuilt target has a new SSH host key; drop the stale one so the check below can accept it.
wx "$MONITOR" bash -c 'ssh-keygen -q -R "$1" >/dev/null 2>&1 || true' _ "$IP"
wx "$MONITOR" ssh -o BatchMode=yes -o StrictHostKeyChecking=accept-new -o ConnectTimeout=5 "workshop@$IP" \
  'echo "SSH ok: $(hostname), psutil $(python3 -c "import psutil; print(psutil.__version__)")"'
# monitor-remote resolves workshop names through this file.
wx "$MONITOR" bash -c 'mkdir -p ~/.config/remote-targets && echo "$2" > ~/.config/remote-targets/"$1"' _ "$TARGET" "$IP"
echo "Monitor it with: workshop run $MONITOR monitor-remote $TARGET [seconds]"
