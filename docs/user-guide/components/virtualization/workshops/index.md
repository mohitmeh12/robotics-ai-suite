<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Canonical Workshop Environments

[Canonical Workshop](https://ubuntu.com/workshop/docs/) creates reproducible development environments as LXD system containers. The Robotics AI Suite provides a Workshop definition for each component: one command creates a container with the suite and that component installed, and passes the host's Intel® GPU, Intel® NPU, cameras and desktop through to it.

Use workshops to:

- Try a component without installing the suite on the host.
- Run ROS 2 Jazzy (Ubuntu 24.04) and ROS 2 Humble (Ubuntu 22.04) side by side on one Ubuntu 24.04 host.
- Connect a ROS 1 Noetic application to the suite through `ros1_bridge`.
- Keep each component's dependencies separate, and rebuild an environment from scratch at any time.

Inference in a workshop runs at the same speed as on the host: in the [Object Detection](object-detection.md) tutorial, OpenVINO™ throughput in the workshop was within run-to-run variation of the host's on the CPU, GPU and NPU.

## Available workshops

Every ROS 2 workshop has a Jazzy definition, `<name>`, and a Humble definition, `<name>-humble`.

| Tutorial | What it runs | Extra hardware | Jazzy | Humble |
|---|---|---|---|---|
| [Object Detection](object-detection.md) | OpenVINO™ object detection on CPU, GPU and NPU | - | Yes | CPU and GPU |
| [Wandering](wandering.md) | Autonomous exploration and mapping in simulation | - | Yes | Yes |
| [ITS Path Planner](its-planner.md) | ITS global planner for Nav2 in simulation | - | Yes | Yes |
| [FastMapping](fast-mapping.md) | 3D and 2D mapping from a recorded depth bag | - | Yes | Yes |
| [Groundfloor Segmentation](groundfloor.md) | Ground and obstacle segmentation from a recorded depth bag | - | Yes | With a workaround |
| [ADBSCAN](adbscan.md) | Obstacle clustering from a recorded laser scan and point cloud | - | Yes | With a workaround |
| [Collaborative SLAM](collaborative-slam.md) | Two visual SLAM trackers with GPU feature extraction | - | Yes | Yes |
| [RealSense™ Camera](realsense.md) | A live RealSense™ camera through the camera interface | One RealSense™ camera | Yes | Yes |
| [Multi-camera AI Demo](multicam.md) | YOLOv8 on two RealSense™ cameras at once, on the GPU | Two RealSense™ cameras | Yes | Yes |
| [Robotics System Profiler](benchmark-framework.md) | ROS 2 KPIs in Grafana, monitoring another workshop | - | Yes | Yes |
| [ROS 1 Bridge](ros1-bridge.md) | A ROS 1 Noetic camera, `ros1_bridge`, and detection on Jazzy | - | Yes | - |

All workshops were tested on an Intel® Core™ Ultra X7 358H processor with Intel® Arc™ B390 graphics and robotics-installer v2026.3.12. The results in these tutorials are examples from that system.

:::{note}
Individual results may vary.
:::

## Where to start

::::{grid} 2

:::{grid-item-card} **Getting Started**
:link: getting-started
:link-type: doc
:link-alt: clickable cards

Install LXD and Workshop, check the GPU and NPU in a container, and launch your first suite workshop.
:::

:::{grid-item-card} **How Workshops Work**
:link: how-it-works
:link-type: doc
:link-alt: clickable cards

SDKs, device interfaces, Jazzy and Humble pairs, and how to add a workshop for another component.
:::

:::{grid-item-card} **ROS Distributions and Interoperability**
:link: ros-distros
:link-type: doc
:link-alt: clickable cards

Humble workshops, mixing Humble and Jazzy on one ROS domain, and reaching workshops from the host.
:::

:::{grid-item-card} **Troubleshooting**
:link: troubleshooting
:link-type: doc
:link-alt: clickable cards

Fixes for device, display, proxy, storage and ROS problems in workshops.
:::
::::

The workshop definitions are in the repository next to each component's source, in `src/components/<component>/workshop/`. The shared parts, and the workshops without a component of their own, are in [`src/workshops/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/workshops).

:::{toctree}
:hidden:

Getting Started <getting-started>
How Workshops Work <how-it-works>
Object Detection <object-detection>
Wandering <wandering>
ITS Path Planner <its-planner>
FastMapping <fast-mapping>
Groundfloor Segmentation <groundfloor>
ADBSCAN <adbscan>
Collaborative SLAM <collaborative-slam>
RealSense Camera <realsense>
Multi-camera AI Demo <multicam>
Robotics System Profiler <benchmark-framework>
ROS 1 Bridge <ros1-bridge>
ROS Distributions and Interoperability <ros-distros>
Troubleshooting <troubleshooting>
:::
