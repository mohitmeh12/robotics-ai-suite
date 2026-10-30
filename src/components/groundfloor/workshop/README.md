<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# groundfloor

[Groundfloor segmentation](../README.md) labels each depth point (ground, elevated, obstacle and so on) and publishes `/segmentation/labeled_points` and `/segmentation/obstacle_points`. The workshop replays the depth stream from `ros-${ROS_DISTRO}-bagfile-spinning` into the package's RealSense launch, so no camera is needed.

- Definitions: [groundfloor.yaml](.workshop/groundfloor.yaml) (Jazzy), [groundfloor-humble.yaml](.workshop/groundfloor-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- Files: [groundfloor_check.py](groundfloor_check.py), [groundfloor.rviz](groundfloor.rviz)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites) and robotics-installer v2026.3.12 or later.

```bash
cd src/components/groundfloor/workshop
workshop launch groundfloor                 # or groundfloor-humble (see Known issues)
workshop run groundfloor bag-replay-check
workshop connect groundfloor/robotics-ai-suite:desktop
workshop run groundfloor bag-replay         # with RViz
```

## Actions

| Action | Description |
|---|---|
| `bag-replay` | `realsense_groundfloor_segmentation_launch.py`, the bag on loop, and RViz2 with `groundfloor.rviz` (labeled points coloured by label, obstacle points in red) |
| `bag-replay-check` | Headless, one pass of the bag. PASS if both outputs publish and the labels split into ground and obstacles |

The bag records aligned depth (`/camera/aligned_depth_to_color/*`), so the actions remap it to the `/camera/depth/*` topics the launch expects. Don't use the `laser-pointcloud` bag: its cloud is unorganized (1 x 897k points) and the node needs a depth-image layout, so every point comes out INVALID.

Each time `bag-replay` loops, the bag's timestamps jump back, so `/tmp/groundfloor_launch.log` collects `TF_OLD_DATA` warnings. Segmentation keeps running.

## Results

| Distro | `bag-replay-check` |
|---|---|
| Jazzy | PASS: `/segmentation/labeled_points` at about 2.3 Hz (about 407k points), about 20% ground, 8% elevated, 69% obstacle |
| Humble | Fails out of the box; PASS (4.2 Hz) with the PCL workaround below |

## Known issues

- The package links PCL 1.15, which installers before v2026.3.12 (oneAPI 2025.3, PCL 1.14) don't provide.
- **Humble:** `ros-humble-pointcloud-groundfloor-segmentation` 2.4-1 links PCL 1.15 but doesn't depend on it, so the node exits with `libpcl_common.so.1.15: cannot open shared object file`. A packaging fix is in progress. Until then, install the libraries by hand: see [PCL 1.15 libraries missing on Humble](../../../workshops/docs/troubleshooting.md#pcl-115-libraries-missing-on-humble).
