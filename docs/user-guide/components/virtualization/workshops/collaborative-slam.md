<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Collaborative SLAM Workshop

This tutorial runs the [Collaborative SLAM](../../optimized_solutions/collaborative-slam.md) multi-camera tutorial in a workshop. Collaborative SLAM is a visual SLAM system in which several trackers, one per camera or robot, build maps and share them through a server. The workshop runs two trackers, one for each camera of a recorded two-camera bag, with ORB feature extraction on the Intel® GPU through Level Zero (SYCL). A UKF (`robot_localization`) fuses both trackers into one robot pose.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- About 20 GiB free in Workshop's storage pool. The GPU ORB extractor pulls in the full Intel® oneAPI toolkit, about 10 GB on disk.
- A local desktop session for the RViz step.

## Launch the workshop

The launch takes about 13 minutes.

```bash
cd src/components/collaborative-slam/workshop
workshop launch collab-slam
```

## Check the GPU

```bash
workshop run collab-slam check-gpu
```

The Intel® Arc™ GPU must appear as a `level_zero:gpu` device in the `sycl-ls` output. If it doesn't, the trackers can't use the GPU.

## Run the trackers without a display

```bash
workshop run collab-slam tracker-check
```

The action replays the bag through both trackers and the UKF. It prints PASS if:

- Both trackers publish `kf_pose`.
- The UKF publishes `/odometry/filtered`.
- Both tracker processes used the GPU, measured as compute-engine cycles from the `xe` driver.
- Nothing crashed.

On Jazzy, the trackers publish 46 and 51 `kf_pose` messages and the UKF 55 fused poses in a 14-second replay.

## View the trackers in RViz

```bash
workshop connect collab-slam/robotics-ai-suite:desktop
workshop run collab-slam tutorial
```

The action runs the packaged `cslam-multi-camera.sh`: the UKF, static transforms, and two GPU trackers, each with its own RViz window, and plays the `multi-camera` bag (about 13 seconds). It runs until you press Ctrl+C.

## Actions

| Action | Description |
|---|---|
| `check-gpu` | `/dev/dri` and `sycl-ls`. The GPU must appear as a `level_zero:gpu` device |
| `tutorial` | The packaged `cslam-multi-camera.sh` with RViz for each tracker. Runs until Ctrl+C |
| `tracker-check` | Headless: the same pipeline without RViz. PASS if both trackers publish, the UKF fuses them, both trackers used the GPU, and nothing crashed |

Both pipeline actions add oneAPI's `umf/<version>/lib` directory to `LD_LIBRARY_PATH`. Without it, the Level Zero adapter can't load `libumf.so.1`, and the trackers fail with `No device of requested type available`.

## Humble

Use `collab-slam-humble` in every command. `tracker-check` passes: the trackers publish 46 and 48 `kf_pose` messages and the UKF 56 fused poses in a 12-second replay.

Ubuntu 22.04 has an older GPU compute runtime, and `sycl-ls` names the GPU `Intel(R) Graphics [0xb080]`, but Level Zero and the GPU tracker work.

## Known issues

- **The `-lze` metapackage installs the CPU tracker.** The generic `ros-<distro>-univloc-tracker`, `-server` and `-slam` packages (2.3-3 and 2.3-4) declare that they provide the `-lze` package names. So `ros-<distro>-collab-slam-lze` is satisfied by the CPU builds, and the tracker never loads the GPU ORB extractor. The workshop's package list names the `-lze` packages explicitly, which replaces the generic ones. With the CPU build installed, the trackers still produce poses, but `tracker-check` reports FAIL because the GPU isn't used.
- **Large install.** `liborb-lze` depends on the full `intel-oneapi-toolkit` (81 packages, a 2.4 GB download and 10.2 GB on disk), although the oneAPI runtime packages would be enough.
- **UKF messages.** The tutorial's UKF node logs `Could not obtain transform from odom->base_link` a few times during the replay, and a NaN error after the bag ends. These come from the tutorial configuration and don't affect the tracker output.

## Source

The definitions, package list and check script are in [`src/components/collaborative-slam/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/collaborative-slam/workshop).
