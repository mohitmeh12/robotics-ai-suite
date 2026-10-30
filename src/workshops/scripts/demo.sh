#!/usr/bin/env bash
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# Team demo: starts each workshop and runs its showcase action. See: src/workshops/scripts/demo.sh help
set -u
cd "$(dirname "$0")/../.." || exit 1   # src/
S=${HUMBLE:+-humble}

DEMOS=(
  "accel-check|check-level-zero|GPU and NPU visible inside the container (Level Zero)"
  "object-detection|ros-object-detection-gpu|OpenVINO object detection on the GPU, in RViz"
  "wandering|gazebo-tutorial|Robot explores and maps a Gazebo world by itself (about 4 min)"
  "its-planner|simulation|ITS planner in Nav2: set a 2D Pose Estimate, then a Nav2 Goal in RViz"
  "fast-mapping|bag-replay|3D voxel map and 2D grid from a recorded depth bag"
  "groundfloor|bag-replay|Ground and obstacle segmentation of depth points (loops)"
  "adbscan|bag-replay|Obstacle clusters over a laser scan (loops)"
  "realsense|tutorial|Live RealSense color and depth (camera must be plugged in)"
  "multicam|demo|Two RealSense cameras with YOLOv8 detection and segmentation on the GPU"
  "collab-slam|tutorial|Collaborative SLAM: two GPU trackers fused into one robot pose, in RViz"
  "benchmark-framework|-|Grafana dashboards of a wandering simulation, live"
  "ros1-bridge|-|ROS 1 camera -> ros1_bridge -> GPU detection on Jazzy, with latency"
)

# Component workshops live in components/<component>/workshop, the others in workshops/<name>.
wsdir() {
  case $1 in
    accel-check | realsense | ros1-bridge) echo "workshops/$1" ;;
    benchmark-framework) echo components/ros-kpi/workshop ;;
    collab-slam) echo components/collaborative-slam/workshop ;;
    multicam) echo components/multicam-demo/workshop ;;
    *) echo "components/$1/workshop" ;;
  esac
}

# Workshop commands only work in the workshop's project directory.
ws() { local dir; dir=$(wsdir "$1"); shift; (cd "$dir" && workshop "$@"); }

# Start the workshop and reconnect its desktop plug, so windows open on the current display.
up() {
  local out
  if ! out=$(ws "$1" start "$2" 2>&1) && [[ $out != *"already running"* ]]; then
    echo "$out" | grep -v "new warnings"
    echo "Cannot start $2; see src/workshops/docs/troubleshooting.md"
    return 1
  fi
  ws "$1" disconnect "$2/robotics-ai-suite:desktop" > /dev/null 2>&1
  ws "$1" connect "$2/robotics-ai-suite:desktop" > /dev/null 2>&1
  return 0
}

demo() {
  local dir=$1 action=$2 w="$1$S"
  case $dir in
    accel-check)
      up accel-check accel-check && ws accel-check run accel-check "$action" ;;
    realsense | multicam)
      up "$dir" "$w" || return
      ws "$dir" connect "$w/camera:camera" > /dev/null 2>&1
      # The multicam demo's Python environment and models come from its install action, not the package.
      if [ "$dir" = multicam ] && ! ws multicam exec "$w" -- bash -c 'ls /opt/ros/*/share/pyrealsense2-ai-demo/models/yolov8/FP16/yolov8n-seg.xml' < /dev/null > /dev/null 2>&1; then
        echo "First run in $w: setting up the demo's Python environment and models (a few minutes)..."
        ws multicam run "$w" install < /dev/null > /tmp/demo-multicam-install.log 2>&1 || { echo "install failed; see /tmp/demo-multicam-install.log"; return 1; }
      fi
      ws "$dir" run "$w" "$action" ;;
    benchmark-framework)
      local wd="wandering$S" ip url
      up wandering "$wd" && up benchmark-framework "$w" || return
      ws benchmark-framework exec "$w" -- test -d /home/workshop/benchmarking < /dev/null 2> /dev/null \
        || ws benchmark-framework run "$w" install
      echo "Pairing $w with $wd..."
      if ! workshops/scripts/authorize-remote.sh "$w" "$wd" > /tmp/demo-pairing.log 2>&1; then
        cat /tmp/demo-pairing.log
        return 1
      fi
      ip=$(ws benchmark-framework exec "$w" -- ip -4 -br addr show eth0 < /dev/null 2>/dev/null | awk '{print $3}' | cut -d/ -f1)
      url="http://$ip:30000/d/ros2-kpi-dashboard/ros2-kpi-dashboard?refresh=5s&from=now-15m&to=now"
      echo "Starting the wandering simulation (Gazebo and RViz windows)..."
      ws wandering run "$wd" gazebo-tutorial > /tmp/demo-wandering.log 2>&1 &
      sleep 20
      cat <<EOF

  Grafana dashboard (live in about 30 s, login admin / admin):

    $url

  Paste the link into a browser, or in VS Code: Ctrl+Shift+P > "Simple Browser: Show".

  Monitoring for 3 minutes (Ctrl+C to stop early). Full output: tail -f /tmp/demo-monitor.log
EOF
      ws benchmark-framework run "$w" monitor-remote "$wd" 180 > /tmp/demo-monitor.log 2>&1
      grep -aE "Session complete|Visualizations:" /tmp/demo-monitor.log | sed "s|monitoring_sessions|$w:~/benchmarking/monitoring_sessions|"
      echo "Waiting for the wandering simulation to finish (Ctrl+C to skip)..."
      wait
      grep -aE "^(PASS|FAIL)" /tmp/demo-wandering.log
      echo "Grafana stays up at the link above. Stop it with: (cd src/components/ros-kpi/workshop && workshop run $w dashboards-stop)" ;;
    ros1-bridge)
      up ros1-bridge ros1-bridge && up ros1-bridge ros1-detector && workshops/scripts/ros1-bridge-check.sh GPU 1280 720 30 30 ;;
    *)
      up "$dir" "$w" && ws "$dir" run "$w" "$action" ;;
  esac
}

# Ctrl+C stops the running action but not this script.
trap 'echo' INT

list() {
  for d in "${DEMOS[@]}"; do IFS='|' read -r name _ what <<< "$d"; printf "  %-20s %s\n" "$name" "$what"; done
}

usage() {
  cat <<'EOF'
Run the Robotics AI Suite workshops one after another for a demo.

Usage:
  src/workshops/scripts/demo.sh                 Tour of all workshops. Before each one: Enter to run, s to skip, q to quit
  src/workshops/scripts/demo.sh <workshop>...   Run only these, in tour order, without prompts
  src/workshops/scripts/demo.sh list            List the workshops and what each demo shows
  src/workshops/scripts/demo.sh status          Show which workshops are launched and running, and their plug connections
  src/workshops/scripts/demo.sh help            Show this help
  HUMBLE=1 src/workshops/scripts/demo.sh ...    Use the Humble (Ubuntu 22.04) variants; accel-check and ros1-bridge have none

Examples:
  src/workshops/scripts/demo.sh wandering adbscan
  HUMBLE=1 src/workshops/scripts/demo.sh its-planner

During a demo:
  Ctrl+C ends the current demo; in the tour it moves on to the next workshop.
  Most demos keep running until Ctrl+C. These finish by themselves: accel-check and ros1-bridge (under a minute),
  wandering (about 4 minutes), benchmark-framework (3 minutes of monitoring, then waits for wandering).

Before the demo:
  - Launch each workshop once (workshop launch <name> in its directory); this script only starts them.
  - Run from a terminal on the host's desktop session; windows open on that display.
  - realsense: plug in the camera. its-planner: set a 2D Pose Estimate, then a Nav2 Goal in RViz.
  - multicam: plug in two RealSense cameras. The first run sets up the demo's Python environment and models
    (workshop run multicam install, a few minutes); run it once before the demo to skip the wait.
  - benchmark-framework: prints a Grafana URL (login admin/admin); open it in a browser while wandering runs.
  - If a workshop won't start after a reboot, see src/workshops/docs/troubleshooting.md.
  - Afterwards, stop workshops you don't need: workshop stop <name> (in its directory).

Workshops:
EOF
  list
}

# yes / no for a plug ending in :<name> in `workshop connections` output, - if the workshop has none.
plug() { awk -v p=":$2\$" '$2 ~ p {print ($3 == "-" ? "no" : "yes"); f=1} END {if (!f) print "-"}' <<< "$1"; }

status() {
  local fmt="%-28s %-9s %-4s %-4s %-8s %s\n" d dir w st c
  printf "$fmt" WORKSHOP STATUS GPU NPU DESKTOP CAMERA
  for d in "${DEMOS[@]}"; do
    dir=${d%%|*}
    while read -r w st; do
      c=$(ws "$dir" connections "$w" 2>/dev/null)
      printf "$fmt" "$w" "$st" "$(plug "$c" gpu)" "$(plug "$c" npu)" "$(plug "$c" desktop)" "$(plug "$c" camera)"
    done < <(ws "$dir" list 2>/dev/null | awk 'NR > 1 {print $1, $2}')
  done
  echo
  echo "Workshops not listed have not been launched. Each demo starts its workshop and connects the desktop (and camera)."
  echo "NPU, if needed: workshop connect <workshop>/<sdk>:npu :custom-device (in the workshop's directory)"
}

case ${1:-} in
  help | -h | --help) usage; exit 0 ;;
  list) list; exit 0 ;;
  status) status; exit 0 ;;
esac

n=0
for d in "${DEMOS[@]}"; do
  IFS='|' read -r name action what <<< "$d"
  if [ $# -gt 0 ] && [[ " $* " != *" $name "* ]]; then continue; fi
  n=$((n + 1))
  echo
  echo "=== $name$( [[ $name != accel-check && $name != ros1-bridge ]] && echo "$S"): $what"
  if [ $# -eq 0 ]; then
    read -r -p "Enter to run, s to skip, q to quit: " ans < /dev/tty || ans=s
    case $ans in q*) break ;; s*) continue ;; esac
  fi
  demo "$name" "$action"
done
[ $n -gt 0 ] || { echo "Unknown workshop: $*. See: src/workshops/scripts/demo.sh help"; exit 1; }
