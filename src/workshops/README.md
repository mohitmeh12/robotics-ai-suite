<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Robotics AI Suite: Canonical Workshop environments

Proof-of-concept [Canonical Workshop](https://ubuntu.com/workshop/docs/) environments for the Intel [Robotics AI Suite](https://developer.robotics.intel.com/). Each workshop is a container with the suite and one component installed, with the host's Intel GPU, NPU, camera and desktop passed through. Jazzy (Ubuntu 24.04) and Humble (Ubuntu 22.04) variants run side by side on one host, and a ROS 1 Noetic application can reach the suite through `ros1_bridge`.

## Quick start

On an Ubuntu 24.04 host with LXD and Workshop installed (see [Getting started](docs/getting-started.md#prerequisites)). Behind an HTTP proxy, run `src/workshops/scripts/configure-proxy.sh` once first; it passes the host's `http_proxy`, `https_proxy` and `no_proxy` to the workshops.

```bash
git clone https://github.com/open-edge-platform/robotics-ai-suite.git
cd robotics-ai-suite/src/components/object-detection/workshop
workshop launch object-detection                                  # 10-30 min the first time
workshop connect object-detection/robotics-ai-suite:npu :custom-device
workshop run object-detection benchmark                           # CPU, GPU, NPU latency
```

## Workshops

Tested on Panther Lake (Core Ultra X7 358H, Arc B390) with robotics-installer v2026.3.12.

| Workshop | What it runs | Jazzy | Humble |
|---|---|---|---|
| [accel-check](accel-check/README.md) | GPU and NPU passthrough check, no ROS | Pass | - |
| [object-detection](../components/object-detection/workshop/README.md) | OpenVINO object detection tutorial on CPU, GPU, NPU | Pass (NPU with `build-npu-fix`) | CPU and GPU; NPU fails |
| [wandering](../components/wandering/workshop/README.md) | Autonomous exploration and mapping in simulation | Pass | Pass |
| [its-planner](../components/its-planner/workshop/README.md) | ITS global planner for Nav2 in simulation | Pass | Pass |
| [fast-mapping](../components/fast-mapping/workshop/README.md) | 3D/2D mapping from a recorded depth bag | Pass | Pass |
| [groundfloor](../components/groundfloor/workshop/README.md) | Ground/obstacle segmentation from a recorded depth bag | Pass | Pass with PCL workaround |
| [adbscan](../components/adbscan/workshop/README.md) | Obstacle clustering from a recorded laser and point cloud bag | Pass | Pass with PCL workaround |
| [realsense](realsense/README.md) | Live RealSense camera through the camera plug | Pass | Pass |
| [multicam](../components/multicam-demo/workshop/README.md) | Multi-camera AI demo: YOLOv8 on two RealSense cameras at once, on the GPU | Pass | Pass |
| [collab-slam](../components/collaborative-slam/workshop/README.md) | Collaborative SLAM multi-camera tutorial, GPU feature extraction | Pass | Pass |
| [benchmark-framework](../components/ros-kpi/workshop/README.md) | Robotics System Profiler and Grafana, monitoring other workshops | Pass | Pass |
| [ros1-bridge](ros1-bridge/README.md) | ROS 1 Noetic camera -> `ros1_bridge` -> detection on Jazzy, and back | Pass | - |

Ubuntu 26.04 (ROS 2 Lyrical): the suite installer, the AMR repo and the OpenVINO apt repo have no 26.04 builds yet, so only [benchmark-framework](../components/ros-kpi/workshop/README.md#lyrical-on-ubuntu-2604) has a Lyrical variant (`benchmark-framework-lyrical`, ROS 2 from packages.ros.org through the `ros2` SDK). It passes.

## Documentation

Step-by-step tutorials for every workshop are in the user guide: [Canonical Workshop Environments](https://developer.robotics.intel.com/development-stack/components/virtualization/workshops/) (source in [`docs/user-guide/components/virtualization/workshops/`](../../docs/user-guide/components/virtualization/workshops/index.md)). The pages below are the reference for working on the workshops themselves.

- [Getting started](docs/getting-started.md): prerequisites, first workshops, everyday commands
- [Architecture](docs/architecture.md): SDKs, plugs, Jazzy/Humble pairs, refresh behaviour, adding a workshop
- [Humble and interoperability](docs/humble-and-interop.md): Humble results and issues, mixing distros, ROS 2 from the host
- [Troubleshooting](docs/troubleshooting.md): GPU after reboot, display, NPU, hooks, proxy, disk and more

## Layout

Each component's workshop lives next to its source, in `src/components/<component>/workshop/`; this directory holds what the workshops share and the workshops that have no component of their own.

```
src/workshops/
  sdks/          Shared Workshop SDKs (proxy, intel-accel, robotics-ai-suite, ros2, docker, camera, ros1-bridge)
  scripts/       Host-side scripts: proxy setup, pairing for remote monitoring, the demo tour, the ros1-bridge check
  docs/          Documentation
  accel-check/   GPU and NPU check, no ROS
  realsense/     Live RealSense camera
  ros1-bridge/   ROS 1 Noetic through ros1_bridge
src/components/<component>/workshop/
  .workshop/<name>.yaml (Jazzy), <name>-humble.yaml, SDK symlinks to src/workshops/sdks
  apt-packages.txt, helpers used by the actions, README.md
```

| Workshop | Directory |
|---|---|
| adbscan, fast-mapping, groundfloor, its-planner, object-detection, wandering | `src/components/<same name>/workshop` |
| collab-slam | `src/components/collaborative-slam/workshop` |
| multicam | `src/components/multicam-demo/workshop` |
| benchmark-framework | `src/components/ros-kpi/workshop` |
| accel-check, realsense, ros1-bridge | `src/workshops/<same name>` |
