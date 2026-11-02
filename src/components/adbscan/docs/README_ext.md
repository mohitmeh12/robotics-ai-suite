<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ADBSCAN Follow-me

ADBSCAN (Adaptive Density-Based Spatial Clustering of Applications with Noise) is an Intel® patented unsupervised clustering algorithm designed for robust spatial object detection and localization from 2D LiDAR, 3D LiDAR, and Intel® RealSense™ depth camera point clouds.

Unlike traditional DBSCAN—which relies on static neighborhood search radii ($\epsilon$) and fixed density thresholds ($MinPts$)—ADBSCAN dynamically scales these parameters based on range from the sensor and the field-of-view point distribution. This compensates for optical beam divergence and spatial point cloud sparsity at extended distances, yielding a 20–30% increase in effective object detection range.

The Follow-Me reference application builds on ADBSCAN to continuously track a target human or guide vehicle and command mobile base velocities. Gesture and speech adapters are deferred; a `std_msgs/Bool` activation topic is available for future integrations.

## Architecture & Algorithm

### ADBSCAN Algorithmic Foundation

In LiDAR and structured-light depth sensing, point density decreases non-linearly with distance $r$. Standard spatial clustering with a fixed radius $\epsilon$ causes over-segmentation at close range and cluster fragmentation or omission at distant ranges.

ADBSCAN formulates adaptive clustering parameters:

1. **Dynamic Radius $\epsilon(r)$**: Expands monotonically with range to encapsulate sparsely distributed returns from distant objects.
2. **Dynamic Density $MinPts(r)$**: Scaled using calibration coefficients (`base`, `coeff_1`, `coeff_2`, `scale_factor`) to match expected return point distributions at given range slices.
3. **Hardware Acceleration**: The neighbor search phase can optionally be offloaded to integrated or discrete GPUs via Intel® oneAPI™ SYCL kernels (`oneapi_kdtree` or `oneapi_octree`), delivering substantial latency reductions on dense point clouds.

### System Architecture

The following diagram shows the live sensor-to-follower flow:

```mermaid
graph TD
  Sensor["LiDAR scan or depth cloud"] --> Follower["adbscan_sub: clustering, target tracking, motion"]
  Activation["/follow_enabled: optional Bool activation"] --> Follower
  Follower --> Command["/cmd_vel: Twist"]
  Command --> Robot["TurtleBot3 or physical robot"]
  Wander["Bounded randomized guide motion"] --> Guide["Gazebo guide robot"]
  Guide --> Sensor
```

## Source Code & Workspace

The component packages are consolidated under the `src/` directory:

- `adbscan_core`: Dependency-light, standalone C++ ADBSCAN clustering library (`cluster` and `cluster_frame`) with pluggable neighbor search backends.
- `adbscan_ros2`: Core ADBSCAN clustering perception node publishing `/obstacle_array`.
- `adbscan_ros2_follow_me`: Self-contained follow-me tracker and velocity generator.
- `followme_turtlebot3_gazebo`: Gazebo simulation environments (Harmonic / Fortress).
- `follow_me_interfaces`, `gesture_recognition_pkg`, `speech_recognition_pkg`, and `text_to_speech_pkg`: Experimental interaction packages, not required by the follower.

## Intel®-Optimized ADBSCAN

In this version of ADBSCAN, the algorithm has been optimized for Intel®
architectures by replacing linear neighbor point search with optimized
KD-tree and octree neighbor search using the Intel® oneAPI™ SYCL toolkit,
as well as refactoring the clustering algorithm into the reusable `adbscan_core`
library. It has been tested and validated across Intel® Core™ Ultra and
11th–14th Generation Intel® Core™ processors with integrated Intel® Iris® X^e^
and Intel® Arc™ graphics. This tutorial describes how to build or package
the Intel-optimized ADBSCAN algorithm, select backends at runtime, and compare
execution times against the CPU baseline.

### ADBSCAN Optimization Setup

Hardware acceleration is provided directly through `adbscan_core` when built
with the `WITH_ONEAPI=ON` option. When enabled, a single installation supports
both standard CPU clustering (`legacy_cpu`) and accelerated oneAPI backends
(`oneapi_kdtree`, `oneapi_octree`, `pcl_kdtree`) selectable at runtime via the
`core_backend` parameter. We demonstrate the gain in latency for a ROS 2 bag
file with point cloud data from a RealSense camera. The amount of gain is
prominent when the input is dense or the number of input points is large.
In case of a 2D LIDAR, the point cloud is comparatively sparse and hence,
not showed here.

### ADBSCAN Optimization Prerequisites

Complete the [Getting Started](../../platform_foundation/getting_started.md) guide before continuing.

### Install and run the ROS 2 bag file Deb package

Install the following package with ROS 2 bag files in order to publish point
cloud data from LIDAR and RealSense camera:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
sudo apt install ros-jazzy-bagfile-laser-pointcloud
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
sudo apt install ros-humble-bagfile-laser-pointcloud
```

:::
::::

Run the following commands in a terminal:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
source /opt/ros/jazzy/setup.bash
ros2 bag play --loop /opt/ros/jazzy/share/bagfiles/laser-pointcloud
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
source /opt/ros/humble/setup.bash
ros2 bag play --loop /opt/ros/humble/share/bagfiles/laser-pointcloud
```

:::
::::

This command will launch the ROS 2 bag file and publish the recorded point
cloud data to respective topics. You will view the following screen output:

![rosbag_play_screen](images/rosbag_play_screen.png)

`ros2 topic list` command will show a list of the published topics which
include `/scan` (point cloud from 2D LIDAR) and `/camera/depth/color/points`
(point cloud from RealSense camera).

### Build or Install oneAPI-Enabled Packages

To build locally with oneAPI support:

```bash
source /opt/ros/<distro>/setup.bash
colcon build --packages-select adbscan_core adbscan_ros2 \
  --cmake-args -DWITH_ONEAPI=ON -DCMAKE_CXX_COMPILER=icpx
```

To build and install Debian packages with oneAPI support:

```bash
CXX=icpx make package ROS_DISTRO=<distro> WITH_ONEAPI=ON
sudo apt update
sudo apt install ./build/debian-packages/packages/*.deb
```

### Run Intel-Optimized ADBSCAN on GPU

Run the node with the `oneapi_kdtree` backend offloaded to GPU:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
source /opt/ros/jazzy/setup.bash
ONEAPI_DEVICE_SELECTOR=level_zero:gpu ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file /opt/ros/jazzy/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
  -p core_backend:=oneapi_kdtree
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
source /opt/ros/humble/setup.bash
ONEAPI_DEVICE_SELECTOR=level_zero:gpu ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file /opt/ros/humble/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
  -p core_backend:=oneapi_kdtree
```

:::
::::

This will print tables with the benchmarking data as showed below:

![benchmarking_picture_adbscan](images/benchmarking_picture_adbscan.png)

The table shows a breakdown between pre-processing, ADBSCAN execution and
post-processing time. The caption at the bottom of the table will print
which PCL library is being used.

### Run Standard (CPU Baseline) Execution

Run the node with the CPU baseline backend (`legacy_cpu`):

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
source /opt/ros/jazzy/setup.bash
ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file /opt/ros/jazzy/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
  -p core_backend:=legacy_cpu
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
source /opt/ros/humble/setup.bash
ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file /opt/ros/humble/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
  -p core_backend:=legacy_cpu
```

:::
::::

This will print a similar table with the benchmarking data.

![benchmark_table_unoptimized](images/benchmark_table_unoptimized.png)

You will see that the ADBSCAN execution time is much smaller for the optimized
version compared to the standard one. The pre-processing and post-processing time
should be more or less of the same range in both versions, since the input bag
file is identical. The amount of gain in execution time will depend on the
system configuration, the size of the point cloud data in the input frames etc.
See [ADBSCAN Behavior Baseline](adbscan-behavior-baseline.md) for full benchmark
metrics across CPU and GPU devices.

### Re-configurable parameters

The clustering backend is configured via `core_backend` (`legacy_cpu`,
`pcl_kdtree`, `oneapi_kdtree`, `oneapi_octree`).
Moreover, one can run both optimized and unoptimized packages with a parameter
called `benchmark_number_of_frames`. It will take an integer (greater or equal
to 1) as input and the benchmarking table will produce the average execution
time of `benchmark_number_of_frames` frames, instead of a single frame
(default value).

For example, you can use the following command to run the optimized ADBSCAN
with `oneapi_octree` backend on GPU and display the benchmarking data for
an average of 5 frames:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
ONEAPI_DEVICE_SELECTOR=level_zero:gpu ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file /opt/ros/jazzy/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
  -p benchmark_number_of_frames:=5 -p core_backend:=oneapi_octree
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
ONEAPI_DEVICE_SELECTOR=level_zero:gpu ros2 run adbscan_ros2 adbscan_sub --ros-args \
  --params-file /opt/ros/humble/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
  -p benchmark_number_of_frames:=5 -p core_backend:=oneapi_octree
```

:::
::::

A complete list of the reconfigurable parameters is given below:

- `use_adbscan_core`

  Routes clustering through the shared `adbscan_core` library (default: `true`).

- `core_backend`

  Available options: `legacy_cpu`, `pcl_kdtree`, `oneapi_kdtree`, `oneapi_octree`.
  Accelerated backends require `WITH_ONEAPI=ON`.

- `Lidar_type`

  Type of the point cloud sensor. For RealSense camera and LIDAR inputs,
  the default value is set to `RS` and `2D`, respectively.

- `Lidar_topic`

  Name of the topic publishing point cloud data.

- `Verbose`

  If this flag is set to `True`, the locations of the detected target objects
  will be printed as the screen log.

- `subsample_ratio`

  This is the downsampling rate of the original point cloud data. Default value
  = 15 (i.e., every 15-th data in the original point cloud is sampled and passed
  to the core ADBSCAN algorithm).

- `x_filter_back`

  Point cloud data with x-coordinate `x_filter_back` are filtered out (positive
  x direction lies in front of the robot).

- `y_filter_left`, `y_filter_right`

  Point cloud data with y-coordinate `y_filter_left` and `y-coordinate` \<
  `y_filter_right` are filtered out (positive `y-direction` is to the left of
  robot and vice versa)

- `z_filter`

  Point cloud data with z-coordinate \< `z_filter` will be filtered out.
  This option will be ignored in case of 2D Lidar.

- `Z_based_ground_removal`

  Filtering in the z-direction will be applied only if this value is non-zero.
  This option will be ignored in case of 2D Lidar.

- `base`, `coeff_1`, `coeff_2`, `scale_factor`

  These are the coefficients used to calculate the adaptive parameters of the
  ADBSCAN algorithm. These values are pre-computed and recommended to keep unchanged.

- `oneapi_library`

  Legacy parameter used when `use_adbscan_core:=false`.

- `benchmark_number_of_frames`

  Any integer greater or equal to 1. This is the number of frames over which
  the average execution time is executed and printed in the benchmarking table.

### ADBSCAN Optimization Troubleshooting

- Failed to install Deb package: Please make sure to run `sudo apt update`
  before installing the necessary Deb packages.

- You can stop the demo anytime by pressing `ctrl-C`.

- The screen log will show `number of points after subsampling` and
  `number of points after filtering`. If these values are zero, please make
  sure to adjust the following parameters to make sure these values are
  greater than zero.

  - Decrease `subsample_ratio`.
  - Increase the absolute values of `x_filter_back`, `y_filter_right`,
    `y_filter_left`.

    Please see the description of these parameters in the table and adjust
    according to your environment.

- IA-optimized ADBSCAN offloads the neighbor search to GPUs when using
  `oneapi_kdtree` and `oneapi_octree` backend.

  Please make sure that your system is equipped with working GPU and Level Zero
  runtime if using these backends. Set `ONEAPI_DEVICE_SELECTOR=level_zero:gpu`
  (or `opencl:cpu` for SYCL CPU). You can use `lspci` and `sycl-ls` in a Linux
  terminal to view GPU and SYCL device info.

- You may experience lower performance if the Linux kernel schedules the `adbscan_ros2`
process to an efficient-core (E-core). To achieve better performance, you can
utilize the `taskset` command to set the process's CPU affinity. For example,
you can direct `adbscan_ros2` to run on CPU core 0 which is
a performance-core (P-core).

  ::::{tab-set}
  :::{tab-item} **Jazzy**
  :sync: jazzy

  ```bash
  source /opt/ros/jazzy/setup.bash
  taskset -c 0 ros2 run adbscan_ros2 adbscan_sub --ros-args \
    --params-file /opt/ros/jazzy/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
    -p core_backend:=oneapi_kdtree
  ```

  :::
  :::{tab-item}  **Humble**
  :sync: humble

  ```bash
  source /opt/ros/humble/setup.bash
  taskset -c 0 ros2 run adbscan_ros2 adbscan_sub --ros-args \
    --params-file /opt/ros/humble/share/adbscan_ros2/config/adbscan_sub_RS.yaml \
    -p core_backend:=oneapi_kdtree
  ```

  :::
  ::::

## Simulate Follow-me in Gazebo

This simulation follows a moving guide robot using LiDAR or depth data. Gesture
and speech integrations are deferred; the follower tracks by default and
accepts optional activation on `/follow_enabled` (`std_msgs/msg/Bool`).
This tutorial describes how to launch the demo in `Gazebo` simulator (Gazebo Harmonic on ROS 2 Jazzy, Gazebo Fortress on ROS 2 Humble).

### Simulation Setup

#### Simulation Prerequisites

Complete the [Getting Started](../../platform_foundation/getting_started.md) guide
before continuing.

#### Install the Simulation Deb Package

Install `ros-jazzy-followme-turtlebot3-gazebo` Deb package from Intel®
Autonomous Mobile Robot APT repository. This is the wrapper package which will
launch all of the dependencies in the backend.

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
sudo apt update
sudo apt install ros-jazzy-followme-turtlebot3-gazebo
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
sudo apt update
sudo apt install ros-humble-followme-turtlebot3-gazebo
```

:::
::::

The follower does not require a Python virtual environment or optional
gesture/audio Python packages.

### Run Demo with 2D Lidar

Launch the complete Gazebo simulation with 2D LiDAR:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
ros2 launch followme_turtlebot3_gazebo follow_me.launch.py
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
ros2 launch followme_turtlebot3_gazebo follow_me.launch.py
```

:::
::::

The guide robot changes direction and speed at random intervals within a
bounded area; there is no prerecorded route. The TurtleBot3 tracks it as soon
as a target is detected.

![screenshot_followme_w_gesture_demo](images/follow_me_demo_gazebo_rviz.png)

- The green square robot is the moving guide robot (the target).
- The gray circular robot is a
  [TurtleBot3](https://emanual.robotis.com/docs/en/platform/turtlebot3/simulation/#gazebo-simulation)
  robot, which will follow the guide robot. TurtleBot3 robot is equipped with a
  2D Lidar and a RealSense Depth Camera. In this demo, the 2D Lidar is
  used as the input topic.

The following condition must be fulfilled to start the TurtleBot3 robot:

- The target (guide robot) will be within the tracking radius of the
  TurtleBot3 robot. Radius is a reconfigurable parameter in:
  `/opt/ros/jazzy/share/adbscan_ros2_follow_me/config/adbscan_sub_2D.yaml`.

  In ROS 2 Humble, the file is located at the similar directory path.

The TurtleBot3 stops when the target is lost or when an optional activation
client publishes `false` on `/follow_enabled`.

- The target (guide robot) moves to a distance of more than the tracking radius
  of the TurtleBot3 robot. Radius is a reconfigurable parameter in:
  `/opt/ros/jazzy/share/adbscan_ros2_follow_me/config/adbscan_sub_2D.yaml`.

  In ROS 2 Humble, the file is located at the similar directory path.

### Run Demo with RealSense Camera

Launch the same simulation with simulated RealSense depth sensing:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
ros2 launch followme_turtlebot3_gazebo follow_me.launch.py sensor:=realsense
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
ros2 launch followme_turtlebot3_gazebo follow_me.launch.py sensor:=realsense
```

:::
::::

In this demo, RealSense camera of the TurtleBot3 robot is selected as
the input point cloud sensor. After running all of the above commands,
you will observe similar behavior of the TurtleBot3 robot and guide robot in the
`Gazebo` GUI as in [Run Demo with 2D Lidar](#run-demo-with-2d-lidar).

There are reconfigurable parameters in
`/opt/ros/humble/share/adbscan_ros2_follow_me/config/` directory for both LIDAR
(`adbscan_sub_2D.yaml`) and RealSense camera (`adbscan_sub_RS.yaml`).

The user can modify parameters depending on the respective robot, sensor
configuration and environments (if required) before running the tutorial.
Find a brief description of the parameters in the following list:

- ``Lidar_type``

  Type of the point cloud sensor. For RealSense camera and LIDAR inputs,
  the default value is set to ``RS`` and ``2D``, respectively.

- ``Lidar_topic``

  Name of the topic publishing point cloud data.

- ``Verbose``

  If this flag is set to ``True``, the locations of the detected target objects
  will be printed as the screen log.

- ``subsample_ratio``

  This is the downsampling rate of the original point cloud data. Default value
  = 15 (i.e. every 15-th data in the original point cloud is sampled and passed
  to the core ADBSCAN algorithm).

- ``x_filter_back``

  Point cloud data with x-coordinate > ``x_filter_back`` are filtered out
  (positive x direction lies in front of the robot).

- ``y_filter_left``, ``y_filter_right``

  Point cloud data with y-coordinate > ``y_filter_left`` and y-coordinate <
  ``y_filter_right`` are filtered out (positive y-direction is to the left of
  robot and vice versa).

- ``z_filter``

  Point cloud data with z-coordinate < ``z_filter`` will be filtered out. This
  option will be ignored in case of 2D Lidar.

- ``Z_based_ground_removal``

  Filtering in the z-direction will be applied only if this value is non-zero.
  This option will be ignored in case of 2D Lidar.

- ``base``, ``coeff_1``, ``coeff_2``, ``scale_factor``

  These are the coefficients used to calculate adaptive parameters of the
  ADBSCAN algorithm. These values are pre-computed and recommended
  to keep unchanged.

- ``init_tgt_loc``

  This value describes the initial target location. The person needs to be at a
  distance of ``init_tgt_loc`` in front of the robot to initiate the motor.

- ``max_dist``

  This is the maximum distance that the robot can follow. If the person moves at
  a distance > ``max_dist``, the robot will stop following.

- ``min_dist``

  This value describes the safe distance the robot will always maintain with the
  target person. If the person moves closer than ``min_dist``,
  the robot stops following.

- ``max_linear``

  Maximum linear velocity of the robot.

- ``max_angular``

  Maximum angular velocity of the robot.

- ``max_frame_blocked``

  The robot will keep following the target for ``max_frame_blocked`` number of
  frames in the event of a temporary occlusion.

- ``tracking_radius``

  The robot will keep following the target as long as the current target
  location = previous location +/- ``tracking_radius``

### Simulation Troubleshooting

- Failed to install Deb package: Please make sure to run `sudo apt update`
  before installing the necessary Deb packages.

- You can stop the demo anytime by pressing `ctrl-C`. If the `Gazebo` simulator
  freezes or does not stop, please use the following commands in a terminal:

  ::::{tab-set}
  :::{tab-item} **Jazzy (Gazebo Harmonic)**
  :sync: jazzy

  ```bash
  sudo killall -9 gz ruby
  ```

  :::
  :::{tab-item}  **Humble (Gazebo Fortress)**
  :sync: humble

  ```bash
  sudo killall -9 gazebo gzserver gzclient
  ```

  :::
  ::::

## Deploy Follow-me on a Physical Mobile Robot (Clearpath Jackal / AAEON AMR)

This section provides instructions for running the ADBSCAN-based Follow-me
pipeline on physical Autonomous Mobile Robots using RealSense depth camera or
LiDAR inputs.

The sensor publishes point clouds to `/camera/depth/color/points` (RealSense)
or laser scans to `/scan` (LiDAR). The `adbscan_sub` follower
subscribes to the corresponding topic, segments clusters using range-adaptive
parameters, determines the target person's centroid, computes linear and angular
velocity adjustments, and publishes differential-drive commands to `/cmd_vel` of
type `geometry_msgs/msg/Twist`. This message is subsequently consumed by the
chassis base controller.

### Deployment Setup

#### Deployment Prerequisites

Complete the [Getting Started](../../platform_foundation/getting_started.md) guide
before continuing.

#### Install the Deployment Packages

Deployments can run from the prebuilt Debian packages installed from the
Robotics AI Dev Kit APT repository:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
sudo apt update
sudo apt install ros-jazzy-adbscan-ros2 ros-jazzy-adbscan-ros2-follow-me
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
sudo apt update
sudo apt install ros-humble-adbscan-ros2 ros-humble-adbscan-ros2-follow-me
```

:::
::::

Alternatively, if building directly from source in a cloned workspace:

```bash
make build
source install/setup.bash
```

### Run Demo

To launch the follow-me application on a robot platform using Intel RealSense depth sensing, run:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
source /opt/ros/jazzy/setup.bash
# If running from a built workspace:
# source install/setup.bash

ros2 run adbscan_ros2_follow_me adbscan_sub --ros-args --params-file /opt/ros/jazzy/share/adbscan_ros2_follow_me/config/adbscan_sub_RS.yaml
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
source /opt/ros/humble/setup.bash
# If running from a built workspace:
# source install/setup.bash

ros2 run adbscan_ros2_follow_me adbscan_sub --ros-args --params-file /opt/ros/humble/share/adbscan_ros2_follow_me/config/adbscan_sub_RS.yaml
```

:::
::::

For 2D LiDAR based tracking:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
source /opt/ros/jazzy/setup.bash
ros2 run adbscan_ros2_follow_me adbscan_sub --ros-args --params-file /opt/ros/jazzy/share/adbscan_ros2_follow_me/config/adbscan_sub_2D.yaml
```

:::
:::{tab-item}  **Humble**
:sync: humble

```bash
source /opt/ros/humble/setup.bash
ros2 run adbscan_ros2_follow_me adbscan_sub --ros-args --params-file /opt/ros/humble/share/adbscan_ros2_follow_me/config/adbscan_sub_2D.yaml
```

:::
::::

After starting the node, the robot begins searching for trackable clusters in its initial detection radius (`init_tgt_loc`, default 0.5m–0.8m) and tracks the centroid of the acquired target as it moves.

Configuration parameters can be inspected and modified in:
`/opt/ros/<distro>/share/adbscan_ros2_follow_me/config/adbscan_sub_RS.yaml` (or `src/adbscan_ros2_follow_me/config/adbscan_sub_RS.yaml` when developing locally).
Find a brief description of the parameters in the following list:

- ``Lidar_type``

  Type of the point cloud sensor. For RealSense camera and LIDAR inputs,
  the default value is set to ``RS`` and ``2D``, respectively.

- ``Lidar_topic``

  Name of the topic publishing point cloud data.

- ``Verbose``

  If this flag is set to ``True``, the locations of the detected target objects
  will be printed as the screen log.

- ``subsample_ratio``

  This is the downsampling rate of the original point cloud data. Default value
  = 15 (i.e. every 15-th data in the original point cloud is sampled and passed
  to the core BSCAN algorithm).

- ``x_filter_back``

  Point cloud data with x-coordinate > ``x_filter_back`` are filtered out
  (positive x direction lies in front of the robot).

- ``y_filter_left``, ``y_filter_right``

  Point cloud data with y-coordinate > ``y_filter_left`` and y-coordinate <
  ``y_filter_right`` are filtered out (positive y-direction is to the left of
  robot and vice versa).

- ``z_filter``

  Point cloud data with z-coordinate < ``z_filter`` will be filtered out. This
  option will be ignored in case of 2D Lidar.

- ``Z_based_ground_removal``

  Filtering in the z-direction will be applied only if this value is non-zero.
  This option will be ignored in case of 2D Lidar.

- ``base``, ``coeff_1``, ``coeff_2``, ``scale_factor``

  These are the coefficients used to calculate adaptive parameters of the
  ADBSCAN algorithm. These values are pre-computed and recommended
  to keep unchanged.

- ``init_tgt_loc``

  This value describes the initial target location. The person needs to be at a
  distance of ``init_tgt_loc`` in front of the robot to initiate the motor.

- ``max_dist``

  This is the maximum distance that the robot can follow. If the person moves
  at a distance > ``max_dist``, the robot will stop following.

- ``min_dist``

  This value describes the safe distance the robot will always maintain with
  the target person. If the person moves closer than ``min_dist``,
  the robot stops following.

- ``max_linear``

  Maximum linear velocity of the robot.

- ``max_angular``

  Maximum angular velocity of the robot.

- ``max_frame_blocked``

  The robot will keep following the target for ``max_frame_blocked`` number of
  frames in the event of a temporary occlusion.

- ``tracking_radius``

  The robot will keep following the target as long as the current target
  location = previous location +/- ``tracking_radius``

### Deployment Troubleshooting

- Failed to install Deb package: Please make sure to run `sudo apt update`
  before installing the necessary Deb packages.
- You may stop the demo anytime by pressing `ctrl-C`.
- If the robot rotates more than intended at each step, try reducing the
  parameter `max_angular` in the parameter file.
- If the motor controller board does not start, restart the robot.
- For general robot issues, refer to
  the [troubleshooting guide](../../resources/troubleshooting).
