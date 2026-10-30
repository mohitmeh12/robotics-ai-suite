<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# adbscan

[ADBSCAN](../README.md) clusters 2D laser scans or 3D point clouds into obstacles and publishes `/obstacle_array` (`nav2_dynamic_msgs/ObstacleArray`) and `/marker_array`. The workshop runs `adbscan_sub` with the package's configs on `ros-${ROS_DISTRO}-bagfile-laser-pointcloud` (about 18 s; `/scan`, `/camera/depth/color/points`, `/camera/color/image_raw`), so no sensor is needed.

- Definitions: [adbscan.yaml](.workshop/adbscan.yaml) (Jazzy), [adbscan-humble.yaml](.workshop/adbscan-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- Check: [adbscan_check.py](adbscan_check.py)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites) and robotics-installer v2026.3.12 or later.

```bash
cd src/components/adbscan/workshop
workshop launch adbscan                     # or adbscan-humble (see Known issues)
workshop run adbscan bag-replay-check
workshop connect adbscan/robotics-ai-suite:desktop
workshop run adbscan bag-replay             # with RViz; add RS for the point cloud mode
```

## Actions

| Action | Description |
|---|---|
| `bag-replay [2D\|RS]` | `adbscan_sub` with `adbscan_sub_2D.yaml` (default, `/scan`) or `adbscan_sub_RS.yaml` (`/camera/depth/color/points`), the bag on loop, and RViz2 with the packaged `adbscan_RS_config.rviz` (scan, camera image, cluster markers) |
| `bag-replay-check` | Headless, one pass of the bag in 2D mode and then RS mode. PASS if `/obstacle_array` publishes clusters in most messages in both |

The node stamps its markers in the `map` frame, which the bag's TF doesn't contain, so `bag-replay` publishes an identity `map -> laser` transform to draw the clusters over the scan. As with groundfloor, looping the bag makes RViz log `TF_OLD_DATA` warnings.

## Results

| Distro | `bag-replay-check` |
|---|---|
| Jazzy | PASS: 2D at about 2 Hz with 17-24 obstacles per message; RS at about 1.5 Hz with 2-4 |
| Humble | Fails out of the box; PASS (2D and RS) with the PCL workaround below |

## Known issues

- `ros-jazzy-adbscan-ros2` 2.3-3 links PCL 1.15, so it needs robotics-installer v2026.3.12 or later.
- The package has no launch files. The Standalone and Visualization modes were removed upstream; follow-me is out of scope here.
- **Humble:** `ros-humble-adbscan-ros2` 2.3-3 links PCL 1.15 but doesn't depend on it, so the node exits with `libpcl_common.so.1.15: cannot open shared object file`. A packaging fix is in progress. Until then, install the libraries by hand: see [PCL 1.15 libraries missing on Humble](../../../workshops/docs/troubleshooting.md#pcl-115-libraries-missing-on-humble).
