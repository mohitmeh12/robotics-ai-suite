<!--
Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# OpenVINO Vision Applications

## Documentation

Comprehensive documentation on this component is available here: [dev guide](https://developer.robotics.intel.com/development-stack/ai_resources/openvino/reference_applications/).

## Overview

This repository contains multiple vision applications that leverage OpenVINO for computer vision tasks in robotics, including object detection, semantic segmentation, and YOLOv8-based detection. These applications provide ROS 2 integration for real-time perception capabilities using Intel RealSense cameras and OpenVINO inference engine.

## Get Started

### System Requirements

Prepare the target system following the [official documentation](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/).

### Build

Packages are built natively with CMake and CPack. Export the `ROS_DISTRO` env
variable for the desired platform and run `make package`. The generated Debian
packages are written to `build/debian-packages/packages/`. The following command
is an example for the `Jazzy` distribution.

```bash
ROS_DISTRO=jazzy make package
```

The build reads each package's declared build dependencies from its
`CMakeLists.txt`. To install them before building (for example in a fresh
container), run:

```bash
ROS_DISTRO=jazzy make install-debian-build-deps
```

You can list all built packages:

```bash
ls build/debian-packages/packages/*.deb
```

```text
ros-jazzy-object-detection-tutorial_2.3-1_amd64.deb
ros-jazzy-segmentation-realsense-tutorial_2.3-1_amd64.deb
ros-jazzy-openvino-yolov8-msgs_2.3-1_amd64.deb
ros-jazzy-openvino-yolov8_2.3-1_amd64.deb
```

To build inside a ROS container on the host (installs build dependencies
automatically):

```bash
ROS_DISTRO=jazzy make container-package
```

To clean all build artifacts:

```bash
make clean
```

### Install

If Ubuntu 22.04 with Humble is used, then run

```bash
source /opt/ros/humble/setup.bash
```

If Ubuntu 24.04 with Jazzy is used, then run

```bash
source /opt/ros/jazzy/setup.bash
```

Finally, install the Debian packages that were built via `make package`:

```bash
sudo apt update
sudo apt install ./build/debian-packages/packages/ros-$(ROS_DISTRO)-object-detection-tutorial_*_amd64.deb
sudo apt install ./build/debian-packages/packages/ros-$(ROS_DISTRO)-segmentation-realsense-tutorial_*_amd64.deb
sudo apt install ./build/debian-packages/packages/ros-$(ROS_DISTRO)-openvino-yolov8-msgs_*_amd64.deb
sudo apt install ./build/debian-packages/packages/ros-$(ROS_DISTRO)-openvino-yolov8_*_amd64.deb
```

### Test

To run unit tests (implemented with `colcon`) execute the below command with target `ROS_DISTRO` (example for Jazzy):

```bash
ROS_DISTRO=jazzy make test
```

Tests can also be run inside a ROS container on the host:

```bash
ROS_DISTRO=jazzy make container-test
```

### Development

There is a set of prepared Makefile targets to speed up the development.

In particular, use the following Makefile target to run code linters.

```bash
make lint
```

To run license compliance validation:

```bash
make license-check
```

To see a full list of available Makefile targets:

```bash
make
```

```text
Target                   Description
------                   -----------
build                    Build selected ROS packages with Colcon for local development
clean                    Remove local build, install, log, and package artifacts
container-package        Build Debian packages inside a ROS container on the host
container-test           Build & test with Colcon inside a ROS container on the host
debian-build-deps        Generate Debian Build-Depends control files without configuring components
install-debian-build-deps Install generated Debian Build-Depends with APT
license-check            Perform a REUSE license check using docker container https://hub.docker.com/r/fsfe/reuse
lint                     Run all sub-linters using super-linter (using linters defined for this repo only)
lint-all                 Run super-linter over entire repository (auto-detects code to lint)
lint-bash                Run Bash linter using super-linter
lint-clang               Run clang-format linter using super-linter
lint-python              Run Python linters (pylint, flake8) using super-linter
package                  Build native Debian packages using CMake + CPack
source-package           Create source package tarball
test                     Build & test with Colcon
```

## Usage

This repository contains three main applications:

### Object Detection Application

The Object Detection application provides real-time object detection capabilities using OpenVINO and Intel RealSense cameras. It supports various pre-trained models and can be customized for specific detection tasks.

For detailed usage instructions, see the [Object Detection Tutorial documentation](https://developer.robotics.intel.com/development-stack/ai_resources/openvino/reference_applications/object_detection_tutorial/).

### Segmentation Realsense Tutorial

The Segmentation application demonstrates semantic segmentation using OpenVINO with Intel RealSense depth cameras. It provides pixel-level classification for scene understanding.

For detailed usage instructions, see the [Segmentation Realsense Tutorial documentation](https://developer.robotics.intel.com/development-stack/ai_resources/openvino/reference_applications/segmentation_realsense_tutorial/).

### YOLOv8

The YOLOv8 application provides state-of-the-art object detection using the YOLOv8 model optimized with OpenVINO. It offers high-performance real-time detection for robotics applications.

For detailed usage instructions, see the [YOLOv8 OpenVINO Tutorial documentation](https://developer.robotics.intel.com/development-stack/ai_resources/openvino/reference_applications/yolov8_openvino_tutorial/).

## Try It in a Canonical Workshop

[workshop/](workshop/README.md) runs this component in a [Canonical Workshop](https://ubuntu.com/workshop/docs/) container (`object-detection` on Jazzy, `object-detection-humble` on Humble) with the host's Intel GPU and NPU passed through, installed with the Robotics AI Suite installer. See [Canonical Workshop environments](../../workshops/README.md) for the prerequisites and the other workshops.

## License

OpenVINO Vision Applications is licensed under [Apache 2.0 License](./LICENSES/Apache-2.0.txt).
