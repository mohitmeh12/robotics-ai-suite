<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ROS Distributions and Interoperability

The Robotics AI Suite supports ROS 2 Jazzy on Ubuntu 24.04 and ROS 2 Humble on Ubuntu 22.04. With Workshop, both run side by side on one Ubuntu 24.04 host, each in its own container. This page covers the Humble workshops, mixing distributions on one ROS domain, and reaching workshops from the host. For ROS 1 Noetic, see the [ROS 1 Bridge](ros1-bridge.md) workshop.

## Humble workshops

Every ROS 2 workshop has a Humble variant, `<name>-humble`, with the same actions. It installs the suite with robotics-installer's `full` profile. For example:

```bash
cd src/components/its-planner/workshop
workshop launch its-planner-humble
workshop run its-planner-humble plan-check
```

| Workshop | Humble result |
|---|---|
| [Object Detection](object-detection.md) | `benchmark` and `benchmark-ros` work on the CPU and GPU. The NPU doesn't work |
| [Wandering](wandering.md) | `gazebo-tutorial` passes in about 3 minutes |
| [ITS Path Planner](its-planner.md) | `plan-check`: 81 poses, 4.12 m |
| [FastMapping](fast-mapping.md) | `bag-replay-headless`: 165 map updates, all 46 frames |
| [Groundfloor Segmentation](groundfloor.md) | Passes at 4.2 Hz with the PCL workaround |
| [ADBSCAN](adbscan.md) | Passes in 2D and RS mode with the PCL workaround |
| [Collaborative SLAM](collaborative-slam.md) | `tracker-check` passes on the GPU |
| [RealSense™ Camera](realsense.md) | `tutorial-check` passes: color at about 26 Hz, depth at about 30 Hz |
| [Multi-camera AI Demo](multicam.md) | `demo-check` passes: both cameras at about 29 inference frames per second on the GPU |
| [Robotics System Profiler](benchmark-framework.md) | `install`, `monitor-remote` against `wandering-humble`, and `dashboards-check` work |

### Known issues on Humble

- **PCL 1.15 (Groundfloor Segmentation, ADBSCAN).** `ros-humble-pointcloud-groundfloor-segmentation` 2.4-1 and `ros-humble-adbscan-ros2` 2.3-3 link PCL 1.15 but don't declare it as a dependency. A packaging fix is in progress; until then, see [PCL 1.15 libraries missing on Humble](troubleshooting.md#pcl-115-libraries-missing-on-humble).
- **NPU (Object Detection).** The newest NPU user-space driver for Ubuntu 22.04 in Intel®'s repositories is 1.24 (Ubuntu 24.04 has 1.28), and OpenVINO™ fails with `NPU Compiler Adapter must be used with LEVEL0 backend`. The CPU and GPU work.
- **Rendering.** Mesa 23.2 in Ubuntu 22.04 doesn't support Intel® Arc™ B390 graphics (`Driver does not support the 0xb080 PCI ID`), so RViz and Gazebo render on the CPU with `llvmpipe`. Simulations still run, Gazebo at about 1.3 times real time. Compute on the GPU, such as OpenVINO™ inference, is not affected.
- **Gazebo models (Wandering).** The Humble launch uses Gazebo Classic and replaces `GAZEBO_MODEL_PATH`, so Gazebo downloads `ground_plane` and `sun` from the online model database. Without internet access, the robot falls through the world.

## Humble and Jazzy on the same domain

Mixing ROS 2 distributions on one ROS domain is not officially supported. To see what works, `its-planner` (Jazzy) and `its-planner-humble` (Humble) were run side by side on domain 0, with the default Fast DDS middleware and with Cyclone DDS:

| Test | Result |
|---|---|
| `std_msgs/String` and `Int32` publish and subscribe, both directions, Fast DDS and Cyclone DDS | Data arrives. Humble logs one decode error, then receives normally |
| `ros2 node list`, `ros2 lifecycle get` | Each side lists only its own nodes. With Jazzy nodes present, Humble's `ros2 lifecycle get` fails with `empty node name returned by the RMW layer`. Topic endpoint counts are correct |
| Lifecycle service and `/initialpose`, Jazzy client to Humble Nav2 | Works |
| `ComputePathToPose` action, Humble client to Jazzy Nav2 | Works: an 80-pose path |
| `ComputePathToPose` action, Jazzy client to Humble Nav2 | Hangs: the Jazzy result type adds `error_code` and `error_msg` fields, which Humble's reply lacks |

Messages with identical definitions interoperate. Where a type gained fields between distributions, as some Nav2 action results did, a sender on the older distribution can't serve a receiver on the newer one. Keep Humble and Jazzy workshops on different `ROS_DOMAIN_ID` values unless you are testing this.

## ROS 2 from the host

Workshops share the `workshopbr0` LXD bridge with the host, and ROS 2 discovery reaches across it. A host with ROS 2 Jazzy installed, for example with robotics-installer, can list, echo and visualize topics from a running workshop.

The ROS domain ID must match on both sides:

| Where | `ROS_DOMAIN_ID` |
|---|---|
| `workshop run <name> <action>` | Unset, so **0**. Actions don't read `~/.bashrc` |
| `workshop shell <name>` | A random value from 1 to 100, written to the workshop's `~/.bashrc` by robotics-installer |
| Host with robotics-installer | A random value from 1 to 100, written to the host's `~/.bashrc` |

To attach to a running action from the host, set the host's domain ID to 0:

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=0
ros2 topic list
ros2 topic echo --once --field info /map
rviz2
```

Check the values with `grep ROS_DOMAIN_ID ~/.bashrc` on the host, or `workshop exec <name> -- grep ROS_DOMAIN_ID /home/workshop/.bashrc` for a workshop. To reach a node started in a `workshop shell`, set the host to that shell's value instead. All workshops on the same domain also see each other's topics.
