<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Humble and interoperability

The Robotics AI Suite supports ROS 2 Jazzy (Ubuntu 24.04) and Humble (Ubuntu 22.04). With Workshop, both run side by side on one Ubuntu 24.04 host, each in its own container. This page covers the Humble variants, mixing distros on one ROS domain, and reaching workshops from the host. For ROS 1 Noetic, see [ros1-bridge](../ros1-bridge/README.md).

## Humble variants

Each `<name>-humble` workshop runs the same actions on Ubuntu 22.04 with ROS 2 Humble (installer profile `full`, robotics-installer v2026.3.12):

```bash
cd src/components/its-planner/workshop
workshop launch its-planner-humble
workshop run its-planner-humble plan-check
```

Results on Panther Lake, 2026-10-06:

| Workshop | Result |
|---|---|
| [fast-mapping-humble](../../components/fast-mapping/workshop/README.md) | `bag-replay-headless`: 165 map updates, all 46 frames |
| [wandering-humble](../../components/wandering/workshop/README.md) | `gazebo-tutorial` PASS in 183 s (unexplored 64% -> 38%) |
| [its-planner-humble](../../components/its-planner/workshop/README.md) | `plan-check`: 81 poses, 4.12 m |
| [realsense-humble](../realsense/README.md) | `tutorial-check` PASS: color about 26 Hz, depth about 30 Hz |
| [collab-slam-humble](../../components/collaborative-slam/workshop/README.md) | `tracker-check` PASS on the GPU (2026-10-09) |
| [multicam-humble](../../components/multicam-demo/workshop/README.md) | `demo-check` PASS, both cameras about 29 inference fps on the GPU (2026-10-12) |
| [benchmark-framework-humble](../../components/ros-kpi/workshop/README.md) | `install`, `monitor-remote` against `wandering-humble`, `dashboards-check` PASS |
| [groundfloor-humble](../../components/groundfloor/workshop/README.md) | Fails out of the box; PASS (4.2 Hz) with the PCL workaround |
| [adbscan-humble](../../components/adbscan/workshop/README.md) | Fails out of the box; PASS (2D and RS) with the PCL workaround |
| [object-detection-humble](../../components/object-detection/workshop/README.md) | `benchmark` and `benchmark-ros` on CPU and GPU; NPU fails, `build-npu-fix` not applicable |

### Known issues on Humble

- **groundfloor, adbscan:** `ros-humble-pointcloud-groundfloor-segmentation` 2.4-1 and `ros-humble-adbscan-ros2` 2.3-3 link PCL 1.15 but don't depend on it. A packaging fix is in progress; see [PCL 1.15 libraries missing on Humble](troubleshooting.md#pcl-115-libraries-missing-on-humble) for the workaround.
- **object-detection NPU:** the newest NPU user-space driver for 22.04 in the AMR/ECI repos is 1.24 (24.04 has 1.28), and OpenVINO fails with `NPU Compiler Adapter must be used with LEVEL0 backend`. CPU and GPU work. `build-npu-fix` targets Jazzy (`cv_bridge.hpp`) and doesn't build on Humble.
- **Rendering:** Ubuntu 22.04's Mesa 23.2 has no driver for the Arc B390 (`Driver does not support the 0xb080 PCI ID`), so RViz and Gazebo render with `llvmpipe` on the CPU. The simulations still run (Gazebo at about 1.3x real time). Compute (OpenVINO on the GPU) is not affected.
- **wandering:** the Humble launch uses Gazebo Classic and replaces `GAZEBO_MODEL_PATH`, so Gazebo fetches `ground_plane` and `sun` from the online model database through the proxy. Without that (for example with `GAZEBO_MODEL_DATABASE_URI=` set empty), the robot falls through the world.
- **benchmark-framework:** `dashboards-check` needs a recorded session, so run `monitor-remote` against a paired workshop first. `src/workshops/scripts/authorize-remote.sh` works with Humble workshops.

## Humble and Jazzy on the same domain

`its-planner` (Jazzy) and `its-planner-humble` (Humble) were run side by side on domain 0, with the default Fast DDS middleware and with Cyclone DDS. Mixing distros is not officially supported. What we saw:

| Test | Result |
|---|---|
| `std_msgs/String`, `Int32` pub/sub, both directions, Fast DDS and Cyclone | Data arrives. Humble logs one decode error (`sequence size exceeds remaining buffer` with Fast DDS, `string data is not null-terminated` with Cyclone), then receives normally |
| `ros2 node list` / `ros2 lifecycle get` | Each side lists only its own nodes. With Jazzy nodes present, Humble's `ros2 lifecycle get` fails with `empty node name returned by the RMW layer`. Topic endpoint counts are correct |
| Lifecycle service and `/initialpose`, Jazzy client -> Humble Nav2 | Works |
| `ComputePathToPose` action, Humble client -> Jazzy Nav2 (ITS) | Works: 80-pose path |
| `ComputePathToPose` action, Jazzy client -> Humble Nav2 (ITS) | Hangs: the Jazzy result type adds `error_code` and `error_msg`, which Humble's reply lacks |

Messages with identical definitions interoperate. Where a type grew fields between distros, an older sender to a newer receiver fails (Nav2 action results, for example). Keep Humble and Jazzy workshops on different `ROS_DOMAIN_ID`s unless you are testing this.

## ROS 2 from the host

Workshops share the `workshopbr0` LXD bridge with the host, and ROS 2 discovery (Fast DDS, `ROS_AUTOMATIC_DISCOVERY_RANGE=SUBNET`) reaches across it. A host with ROS 2 Jazzy installed (for example with the same `robotics-installer --profile complete`) can list, echo and visualize topics from a running workshop action.

The ROS domain ID must match on both sides:

| Where | `ROS_DOMAIN_ID` |
|---|---|
| `workshop run <name> <action>` | Unset, so **0** (actions don't read `~/.bashrc`) |
| `workshop shell <name>` | Random 1-100, written to the container's `~/.bashrc` by the installer |
| Host after `robotics-installer` | Random 1-100, written to the host's `~/.bashrc` |

To attach to a running action from the host, override the host's domain ID:

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=0
ros2 topic list
ros2 topic echo --once --field info /map
rviz2    # add /map, /tf, RobotModel, ...
```

Check the values with `grep ROS_DOMAIN_ID ~/.bashrc` on the host, or `workshop exec <name> -- grep ROS_DOMAIN_ID /home/workshop/.bashrc` for a workshop. To reach an interactive `workshop shell`, set the host to that shell's value instead. All workshops on the same domain also see each other's topics.
