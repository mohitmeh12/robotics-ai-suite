<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ADBSCAN Algorithm with RPLIDAR Input Demo

---

This tutorial describes how to run the ADBSCAN algorithm from Robotics
AI Dev Kit using 2D RPLIDAR input.

It outputs to the `obstacle_array` topic of type
`nav2_dynamic_msgs/ObstacleArray`.

## Prerequisites

- [Prepare the target system](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/)
- [Setup the Robotics AI Dev Kit APT Repositories](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#3-set-up-robotics-ai-suite-oneapi-and-graphics-apt-repositories)
- [Install OpenVINO™ Packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#4-install-openvino-packages)
- [Install Robotics AI Dev Kit Deb packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#6-install-robotics-ai-suite-deb-packages)
- [Install the Intel® NPU Driver on Intel® Core™ Ultra Processors (if applicable)](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#7-install-the-intel-npu-driver-on-intel-core-ultra-processors)

## Install Steps

Install `ros-<distro>-adbscan-ros2` Deb package from the Intel® Robotics AI
Dev Kit APT repository:

> ```bash
> sudo apt update
> sudo apt install ros-<distro>-adbscan-ros2  # e.g., ros-jazzy-adbscan-ros2 or ros-humble-adbscan-ros2
> ```

Connect a live 2D LiDAR that publishes the scan topic configured in
`adbscan_sub_2D.yaml`.

## Run the demo with 2D LIDAR

> ```bash
> ros2 run adbscan_ros2 adbscan_sub --ros-args --params-file "$(ros2 pkg prefix adbscan_ros2)/share/adbscan_ros2/config/adbscan_sub_2D.yaml"
> ```

Expected output: ADBSCAN publishes clusters from the live LiDAR scan.

> ![image](images/adbscan_demo_lidar.jpg)

One can view the list of running ROS 2 nodes by typing `ros2 node list`
in a terminal.

> ![image](images/adbscan_node_list.jpg)
