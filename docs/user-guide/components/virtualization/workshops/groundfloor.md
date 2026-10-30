<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Groundfloor Segmentation Workshop

This tutorial runs [groundfloor segmentation](../../sensors/reference_applications/pointcloud-groundfloor-segmentation.md) in a workshop. The node labels each depth point (ground, elevated, obstacle and so on) and publishes `/segmentation/labeled_points` and `/segmentation/obstacle_points`. The workshop replays the depth stream from `ros-${ROS_DISTRO}-bagfile-spinning` into the package's RealSense™ launch, so no camera is needed.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md), with robotics-installer v2026.3.12 or later. Earlier installers don't provide PCL 1.15, which the package needs.
- A local desktop session for the RViz step.

## Launch the workshop

```bash
cd src/components/groundfloor/workshop
workshop launch groundfloor
```

## Check segmentation without a display

```bash
workshop run groundfloor bag-replay-check
```

The action replays the bag once and prints PASS if both outputs publish and the points split into ground and obstacles. On Jazzy, `/segmentation/labeled_points` publishes at about 2.3 Hz with about 407,000 points: about 20% ground, 8% elevated and 69% obstacle.

## View the segmentation in RViz

```bash
workshop connect groundfloor/robotics-ai-suite:desktop
workshop run groundfloor bag-replay
```

RViz shows the labeled points coloured by label, and the obstacle points in red. The bag plays in a loop; press Ctrl+C in the terminal to stop.

Each time the bag loops, its timestamps jump back, so `/tmp/groundfloor_launch.log` collects `TF_OLD_DATA` warnings. Segmentation keeps running.

## Actions

| Action | Description |
|---|---|
| `bag-replay` | `realsense_groundfloor_segmentation_launch.py`, the bag on a loop, and RViz with the workshop's `groundfloor.rviz` |
| `bag-replay-check` | Headless, one pass of the bag. PASS if both outputs publish and the labels split into ground and obstacles |

The bag records aligned depth (`/camera/aligned_depth_to_color/*`), so the actions remap it to the `/camera/depth/*` topics that the launch expects.

:::{note}
Don't use the `laser-pointcloud` bag with this node. Its point cloud is unorganized (1 x 897,000 points), and the node needs a depth-image layout, so every point comes out as INVALID.
:::

## Humble

Use `groundfloor-humble` in every command. `ros-humble-pointcloud-groundfloor-segmentation` 2.4-1 links PCL 1.15 but doesn't declare it as a dependency, so the node exits with:

```text
error while loading shared libraries: libpcl_common.so.1.15: cannot open shared object file
```

A packaging fix is in progress. Until it is released, install the libraries after launching the workshop:

```bash
workshop exec groundfloor-humble -- bash -c 'set -a; . /etc/environment; set +a; sudo -E apt-get install -y libpcl-common1.15 libpcl-filters1.15 libpcl-kdtree1.15 libpcl-segmentation1.15'
```

With them, `bag-replay-check` passes at 4.2 Hz. A rebuild removes the libraries; install them again afterwards.

## Source

The definitions, package list, RViz configuration and check script are in [`src/components/groundfloor/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/groundfloor/workshop).
