<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# collab-slam

Collaborative SLAM ([`src/components/collaborative-slam`](../README.md)) is a visual SLAM system in which several trackers (one per camera or robot) build maps and share them through a server. The workshop runs the multi-camera tutorial: two trackers, one per camera of a recorded two-camera bag, with ORB feature extraction on the Intel GPU through Level Zero (SYCL). A UKF (`robot_localization`) fuses both trackers into one robot pose.

- Definitions: [collab-slam.yaml](.workshop/collab-slam.yaml) (Jazzy), [collab-slam-humble.yaml](.workshop/collab-slam-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- Check: [tracker_check.sh](tracker_check.sh)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites) and about 20 GiB in the storage pool: the GPU ORB package pulls in the full oneAPI toolkit (about 10 GB).

```bash
cd src/components/collaborative-slam/workshop
workshop launch collab-slam                 # or collab-slam-humble; about 13 min
workshop run collab-slam check-gpu          # the Arc GPU must be listed under level_zero
workshop run collab-slam tracker-check
workshop connect collab-slam/robotics-ai-suite:desktop
workshop run collab-slam tutorial           # RViz for each tracker; Ctrl+C to stop
```

## Actions

| Action | Description |
|---|---|
| `check-gpu` | `/dev/dri` and `sycl-ls`: the GPU must appear as a `level_zero:gpu` device |
| `tutorial` | The packaged `cslam-multi-camera.sh`: UKF, static transforms, two GPU trackers with RViz, and the `multi-camera` bag (about 13 s). Runs until Ctrl+C |
| `tracker-check` | Headless: the same pipeline without RViz. PASS if both trackers publish `kf_pose`, `/odometry/filtered` is produced, both tracker processes used the GPU (compute-engine cycles from the `xe` driver), and nothing crashed |

Both pipeline actions add oneAPI's `umf/<version>/lib` directory to `LD_LIBRARY_PATH`. Without it, the Level Zero adapter can't load `libumf.so.1` and the trackers fail with `No device of requested type available`.

## Results

On Panther Lake (Arc B390), `tracker-check`:

| Distro | Result |
|---|---|
| Jazzy | PASS: 46 and 51 `kf_pose` messages, 55 fused poses in a 14 s replay; tracker GPU cycles 8.3M and 7.8M |
| Humble | PASS: 46 and 48 `kf_pose` messages, 56 fused poses in a 12 s replay; tracker GPU cycles 6.3M and 6.1M |

On Humble, the Ubuntu 22.04 GPU runtime is older (compute runtime 25.31 vs 26.31) and `sycl-ls` names the GPU `Intel(R) Graphics [0xb080]`, but Level Zero and the GPU tracker work.

With the CPU build of the tracker installed (see Known issues), the same run produces poses but no GPU cycles, so `tracker-check` reports FAIL.

## Known issues

- **The `-lze` metapackage installs the CPU tracker.** The generic `ros-<distro>-univloc-tracker`, `-server` and `-slam` packages (2.3-3 and 2.3-4) declare that they provide the `-lze` names. So `ros-<distro>-collab-slam-lze` is satisfied by the CPU builds, and the tracker never loads `libgpu_orb`. `apt-packages.txt` names the `-lze` packages explicitly, which replaces the generic ones.
- **Large install.** `liborb-lze` depends on the full `intel-oneapi-toolkit` (81 packages, 2.4 GB download, 10.2 GB on disk on Jazzy), where the oneAPI runtime packages would be enough to run it.
- **UKF messages.** The tutorial's UKF node logs `Could not obtain transform from odom->base_link` a few times during the replay, and a NaN error after the bag ends. The tracker output is not affected; these come from the tutorial configuration.
- The packages need no build step, so the issue's `make safe-package` build action doesn't apply.
