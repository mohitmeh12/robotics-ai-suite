<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ADBSCAN Algorithm with Intel® RealSense™ Camera Input Demo

---

This tutorial describes how to run the ADBSCAN algorithm from Intel®
RealSense™ camera input.

It outputs to the `obstacle_array` topic of type
`nav2_dynamic_msgs/ObstacleArray`.

## Prerequisites

- [Prepare the target system](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/)
- [Setup the Robotics AI Dev Kit APT Repositories](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#3-set-up-robotics-ai-suite-oneapi-and-graphics-apt-repositories)
- [Install OpenVINO™ Packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#4-install-openvino-packages)
- [Install Robotics AI Dev Kit Deb packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#6-install-robotics-ai-suite-deb-packages)
- [Install the Intel® NPU Driver on Intel® Core™ Ultra Processors (if applicable)](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#7-install-the-intel-npu-driver-on-intel-core-ultra-processors)

## Install

Install `ros-<distro>-adbscan-ros2` Deb package from the Intel® Robotics AI
Dev Kit APT repository:

> ```sh
> sudo apt update
> sudo apt install ros-<distro>-adbscan-ros2  # e.g., ros-jazzy-adbscan-ros2 or ros-humble-adbscan-ros2
> ```

Connect a live Intel RealSense depth camera that publishes the point cloud
topic configured in `adbscan_sub_RS.yaml`.

## Run the demo with Intel® RealSense™

> ```sh
> ros2 run adbscan_ros2 adbscan_sub --ros-args --params-file "$(ros2 pkg prefix adbscan_ros2)/share/adbscan_ros2/config/adbscan_sub_RS.yaml"
> ```
>
> The node publishes clusters from the live RealSense point cloud. Start
> `rviz2` separately to visualize the output. The archived example below
> illustrates the expected detections:
>
> > ::: video
> > ../../../../videos/adbscan_demo_RS.mp4
> > :::
