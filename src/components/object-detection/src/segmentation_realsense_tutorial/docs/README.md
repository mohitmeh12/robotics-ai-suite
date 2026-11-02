<!--
Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# OpenVINO™ Tutorial with Segmentation

---

This tutorial demonstrates semantic segmentation with the ROS 2
OpenVINO™ node. It explains how to install the node and run a model using
images from an Intel® RealSense™ camera. Inference can run on a CPU or
GPU.

## Prerequisites

- [Prepare the target system](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/)
- [Setup the Robotics AI Dev Kit APT Repositories](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#3-set-up-robotics-ai-suite-oneapi-and-graphics-apt-repositories)
- [Install OpenVINO™ Packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#4-install-openvino-packages)
- [Install Robotics AI Dev Kit Deb packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#6-install-robotics-ai-suite-deb-packages)
- [Install the Intel® NPU Driver on Intel® Core™ Ultra Processors (if applicable)](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#7-install-the-intel-npu-driver-on-intel-core-ultra-processors)

## Install the tutorial package

```bash
sudo apt install ros-humble-segmentation-realsense-tutorial
```

For ROS 2 Jazzy, replace `humble` in the package name with `jazzy`.

## Run the Demo with Intel® RealSense™ Image Topics

Run one of the following commands to launch the segmentation tutorial
with a specific inference engine:

- GPU inference engine

  ```bash
  ros2 launch segmentation_realsense_tutorial openvino_segmentation.launch.py device:=GPU
  ```

- CPU inference engine

  ```bash
  ros2 launch segmentation_realsense_tutorial openvino_segmentation.launch.py device:=CPU
  ```

> [!NOTE]
> If you do not specify a device, the node uses the GPU by default.

When launched, the node downloads the `deeplabv3` model, converts it to
OpenVINO IR format, and begins inference on images from the Intel®
RealSense™ camera.

To stop the application, press `Ctrl+C` in the terminal running the
launch command.
