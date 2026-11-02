<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Intel-optimized ADBSCAN Algorithm

---

In this version of ADBSCAN, the algorithm has been optimized for Intel®
architectures by replacing linear neighbor point search with optimized
KD-tree and octree neighbor search using the Intel® oneAPI™ SYCL toolkit,
as well as refactoring the clustering algorithm into the reusable `adbscan_core`
library. It has been tested and validated across Intel® Core™ Ultra and
11th–14th Generation Intel® Core™ processors with integrated Intel® Iris® X^e^
and Intel® Arc™ graphics. This tutorial describes how to build or package
the Intel-optimized ADBSCAN algorithm, select backends at runtime, and compare
execution times against the CPU baseline.

## Getting Started

Hardware acceleration is provided directly through `adbscan_core` when built
with the `WITH_ONEAPI=ON` option. When enabled, a single installation supports
both standard CPU clustering (`legacy_cpu`) and accelerated oneAPI backends
(`oneapi_kdtree`, `oneapi_octree`, `pcl_kdtree`) selectable at runtime via the
`core_backend` parameter. We demonstrate the gain in latency for a ROS 2 bag
file with point cloud data from an Intel® RealSense™ camera. The gain is most
prominent when the input point cloud is dense or contains many points.

## Prerequisites

- [Prepare the target system](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/)
- [Setup the Robotics AI Dev Kit APT Repositories](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#3-set-up-robotics-ai-suite-oneapi-and-graphics-apt-repositories)
- [Install Robotics AI Dev Kit Deb packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#6-install-robotics-ai-suite-deb-packages)
- Intel® oneAPI Base Toolkit (specifically the DPC++/C++ Compiler `icpx`) and PCL OneAPI libraries (`libpcl-oneapi-kdtree`, `libpcl-oneapi-octree`).
- Working Intel GPU driver (Level Zero runtime). Verify GPU availability with `lspci` and `sycl-ls`.

## Building or Installing with oneAPI Acceleration

### Building from Source

In a ROS 2 workspace, build with the Intel oneAPI compiler and flag:

```bash
source /opt/ros/<distro>/setup.bash
colcon build --packages-select adbscan_core adbscan_ros2 \
  --cmake-args -DWITH_ONEAPI=ON -DCMAKE_CXX_COMPILER=icpx
```

### Packaging and Installing Debian Packages

To produce Debian packages with oneAPI acceleration:

```bash
CXX=icpx make package ROS_DISTRO=<distro> WITH_ONEAPI=ON
sudo apt update
sudo apt install ./build/debian-packages/packages/*.deb
```

## Running the Demo with Recorded Data

### Run the ROS 2 Bag File

Install the recorded bag file package if available, or use any recorded
point cloud bag:

```bash
# Ubuntu 24.04 (Jazzy)
sudo apt install ros-jazzy-bagfile-laser-pointcloud
source /opt/ros/jazzy/setup.bash
ros2 bag play --loop /opt/ros/jazzy/share/bagfiles/laser-pointcloud

# Ubuntu 22.04 (Humble)
sudo apt install ros-humble-bagfile-laser-pointcloud
source /opt/ros/humble/setup.bash
ros2 bag play --loop /opt/ros/humble/share/bagfiles/laser-pointcloud
```

This launches the bag playback and publishes point cloud data to
`/camera/depth/color/points` and `/scan`.

### Run Intel-Optimized ADBSCAN on GPU

Run the subscriber node with the oneAPI KD-tree or octree backend offloaded
to the GPU:

```bash
source /opt/ros/<distro>/setup.bash
ONEAPI_DEVICE_SELECTOR=level_zero:gpu ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file "$(ros2 pkg prefix adbscan_ros2)/share/adbscan_ros2/config/adbscan_sub_RS.yaml" \
  -p core_backend:=oneapi_kdtree
```

To run with the oneAPI octree backend:

```bash
ONEAPI_DEVICE_SELECTOR=level_zero:gpu ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file "$(ros2 pkg prefix adbscan_ros2)/share/adbscan_ros2/config/adbscan_sub_RS.yaml" \
  -p core_backend:=oneapi_octree
```

### Run Baseline CPU ADBSCAN

Run the same node executable with the CPU baseline backend:

```bash
ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file "$(ros2 pkg prefix adbscan_ros2)/share/adbscan_ros2/config/adbscan_sub_RS.yaml" \
  -p core_backend:=legacy_cpu
```

You will observe that ADBSCAN clustering execution time is significantly faster
when hardware-accelerated on GPU compared to the unaccelerated CPU path. See
[ADBSCAN Behavior Baseline](adbscan-behavior-baseline.md) for detailed latency,
p95, and throughput benchmarks measured across devices.

## Re-configurable Parameters

| Parameter | Type | Default | Description |
| --- | --- | --- | --- |
| `use_adbscan_core` | bool | `true` | When true, invokes the shared `adbscan_core` library. |
| `core_backend` | string | `legacy_cpu` | Neighbor search backend: `legacy_cpu`, `pcl_kdtree`, `oneapi_kdtree`, `oneapi_octree`. Accelerated backends require `WITH_ONEAPI=ON`. |
| `Lidar_type` | string | `RS` | Type of sensor (`RS`, `2D`, or `3D`). |
| `Lidar_topic` | string | `/camera/depth/color/points` | Topic publishing sensor data. |
| `Verbose` | bool | `true` | Prints cluster count and execution timing. |
| `subsample_ratio` | float | `15.0` | Downsampling ratio applied to point cloud data. |
| `x_filter_back`, `y_filter_left`, `y_filter_right`, `z_filter` | float | varies | Spatial bounding box limits for point filtering. |
| `Z_based_ground_removal` | float | `1.0` | Applies Z ground plane filtering if non-zero. |
| `base`, `coeff_1`, `coeff_2`, `scale_factor` | float | varies | Pre-computed polynomial coefficients for range-adaptive clustering. |
| `oneapi_library` | string | `oneapi_kdtree` | Legacy backend selector used when `use_adbscan_core:=false`. |
| `benchmark_number_of_frames` | int | `1` | Number of frames over which to average timing measurements. |

## Troubleshooting

- **Target Device Selection**: Ensure `ONEAPI_DEVICE_SELECTOR=level_zero:gpu`
  (for GPU) or `ONEAPI_DEVICE_SELECTOR=opencl:cpu` (for SYCL CPU) is exported in
  the running terminal. Run `sycl-ls` to list available SYCL devices.
- **Empty Output**: If the log indicates zero points after subsampling/filtering,
  check `subsample_ratio` or expand `x_filter_back`, `y_filter_right`, and
  `y_filter_left`.
- **CPU Affinity (E-cores vs P-cores)**: On hybrid Intel® Core™ processors (12th Gen+,
  Intel® Core™ Ultra), the Linux kernel may schedule worker threads onto E-cores. Use
  `taskset` to bind the process to P-cores for peak performance:

  ```bash
  taskset -c 0-3 ros2 run adbscan_ros2 adbscan_sub --ros-args \
    --params-file "$(ros2 pkg prefix adbscan_ros2)/share/adbscan_ros2/config/adbscan_sub_RS.yaml" \
    -p core_backend:=oneapi_kdtree
  ```
