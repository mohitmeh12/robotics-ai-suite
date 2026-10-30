<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ADBSCAN Workshop

This tutorial runs [ADBSCAN](../../optimized_solutions/adbscan-follow-me.md) obstacle clustering in a workshop. ADBSCAN clusters 2D laser scans or 3D point clouds into obstacles, and publishes them on `/obstacle_array` (`nav2_dynamic_msgs/ObstacleArray`) and `/marker_array`. The workshop runs `adbscan_sub` on the recorded bag from `ros-${ROS_DISTRO}-bagfile-laser-pointcloud` (about 18 seconds of `/scan`, `/camera/depth/color/points` and `/camera/color/image_raw`), so no sensor is needed.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md), with robotics-installer v2026.3.12 or later. Earlier installers don't provide PCL 1.15, which the package needs.
- A local desktop session for the RViz step.

## Launch the workshop

```bash
cd src/components/adbscan/workshop
workshop launch adbscan
```

## Check clustering without a display

```bash
workshop run adbscan bag-replay-check
```

The action replays the bag once in 2D mode (laser scan) and once in RS mode (RealSense™ point cloud), and prints PASS if `/obstacle_array` publishes clusters in most messages in both. On Jazzy, 2D mode publishes at about 2 Hz with 17 to 24 obstacles per message, and RS mode at about 1.5 Hz with 2 to 4.

## View the clusters in RViz

1. Connect the desktop and replay the bag in 2D mode:

   ```bash
   workshop connect adbscan/robotics-ai-suite:desktop
   workshop run adbscan bag-replay
   ```

   RViz shows the laser scan, the camera image and the cluster markers. The bag plays in a loop; press Ctrl+C in the terminal to stop.

2. Replay it in RS mode, which clusters the depth camera's point cloud:

   ```bash
   workshop run adbscan bag-replay RS
   ```

The node stamps its markers in the `map` frame, which the bag's transforms don't contain, so `bag-replay` publishes an identity `map` to `laser` transform to draw the clusters over the scan. Each time the bag loops, RViz logs `TF_OLD_DATA` warnings.

## Actions

| Action | Description |
|---|---|
| `bag-replay [2D\|RS]` | `adbscan_sub` with `adbscan_sub_2D.yaml` (default, `/scan`) or `adbscan_sub_RS.yaml` (`/camera/depth/color/points`), the bag on a loop, and RViz with the packaged `adbscan_RS_config.rviz` |
| `bag-replay-check` | Headless, one pass of the bag in 2D mode and then RS mode. PASS if `/obstacle_array` publishes clusters in most messages in both |

The package has no launch files, so the actions start `adbscan_sub` directly with the packaged configuration files. The follow-me application is not part of this workshop.

## Humble

Use `adbscan-humble` in every command. `ros-humble-adbscan-ros2` 2.3-3 links PCL 1.15 but doesn't declare it as a dependency, so the node exits with:

```text
error while loading shared libraries: libpcl_common.so.1.15: cannot open shared object file
```

A packaging fix is in progress. Until it is released, install the libraries after launching the workshop:

```bash
workshop exec adbscan-humble -- bash -c 'set -a; . /etc/environment; set +a; sudo -E apt-get install -y libpcl-common1.15 libpcl-filters1.15 libpcl-kdtree1.15 libpcl-segmentation1.15'
```

With them, `bag-replay-check` passes in both modes. A rebuild removes the libraries; install them again afterwards.

## Source

The definitions, package list and check script are in [`src/components/adbscan/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/adbscan/workshop).
