#!/usr/bin/env bash
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# ROS 1 -> ROS 2 detection loop across the ros1-bridge and ros1-detector workshops; prints PASS or FAIL.
# Usage: scripts/ros1-bridge-check.sh [device] [width] [height] [hz] [seconds] [rmw]
set -uo pipefail
DEV=${1:-GPU} W=${2:-640} H=${3:-480} HZ=${4:-10} SECS=${5:-30} RMW=${6:-rmw_cyclonedds_cpp}
cd "$(dirname "$0")/../ros1-bridge" || exit 1

if [ ! -f "frames/${W}x${H}.raw" ]; then
  workshop run ros1-detector frames "$W" "$H" < /dev/null 2>&1 | grep -v "new warnings"
  [ "${PIPESTATUS[0]}" -eq 0 ] || exit 1
fi

LOG=$(mktemp)
workshop run ros1-detector detect "$DEV" $((SECS + 120)) "$RMW" > "$LOG" 2>&1 < /dev/null &
DET=$!
# Killing a host-side `workshop run` leaves its processes running in the container.
cleanup() {
  kill $DET 2>/dev/null
  workshop exec ros1-detector -- pkill -f "[r]os1_detector.py" < /dev/null > /dev/null 2>&1
  workshop exec ros1-bridge -- pkill -9 -f "[r]os1_camera|[p]arameter_bridge|[r]osmaster|[r]osout" < /dev/null > /dev/null 2>&1
  rm -f "$LOG"
}
trap cleanup EXIT

# Compiling the model takes a few seconds (longer on first NPU use).
for _ in $(seq 120); do
  grep -q "ready" "$LOG" && break
  kill -0 $DET 2>/dev/null || break
  sleep 1
done
if ! grep -q "ready" "$LOG"; then
  grep -v "new warnings" "$LOG"
  echo "FAIL: detector on $DEV did not start"
  exit 1
fi

workshop run ros1-bridge bridge-detect "$W" "$H" "$HZ" "$SECS" "$RMW" < /dev/null 2>&1 | grep -v "new warnings"
RC=${PIPESTATUS[0]}
wait $DET
grep "detector\[.*frames" "$LOG"

if [ "$RC" -eq 0 ]; then
  echo "PASS: ${W}x${H} @ ${HZ} Hz on $DEV ($RMW)"
else
  echo "FAIL: ${W}x${H} @ ${HZ} Hz on $DEV ($RMW)"
fi
exit "$RC"
