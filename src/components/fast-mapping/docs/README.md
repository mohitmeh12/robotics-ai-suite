<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# FastMapping Algorithm

FastMapping is an Intel®-optimized ROS 2 package designed for real-time 3D volumetric occupancy mapping and 2D planar costmap generation from single or multi-camera RGB-D depth streams. FastMapping replaces the pointer-based tree traversal and discrete ray-casting of traditional OctoMap implementations with a Morton-indexed octree structure, pre-allocated chunked memory pools, and continuous B-spline depth fusion.

The component serves as a drop-in 3D spatial perception and 2D obstacle avoidance ingredient for Autonomous Mobile Robots (AMRs) running navigation stacks such as Nav2.

---

## Overview and Architecture

FastMapping ingests synchronized depth images and camera intrinsics from one to four cameras, tracks camera poses across the TF2 coordinate tree, and integrates depth observations into an internal volumetric map. The node simultaneously exposes 3D visual markers for RViz2 and publishes a 2D planar occupancy grid sliced between configurable vertical bounds for immediate consumption by navigation stacks such as Nav2.

```{mermaid}
flowchart TD
    subgraph Inputs [Sensor and Transform Inputs]
        RGBD[RGB-D Cameras / Depth Topics<br/><code>depth_topic_1..4</code>]
        CINFO[Camera Intrinsics<br/><code>depth_info_topic</code>]
        TF[TF2 Coordinate Transforms<br/><code>camera_*_optical_frame &harr; map</code>]
    end

    subgraph FastMappingNode [fast_mapping_node]
        SYNC[CameraSubscribers<br/>ApproximateTime Synchronizer]
        QUEUE[(DataQueue<br/>Thread-safe Frame Queue)]
        WORKER[Worker Thread & TF Lookup]
        FMLIB[fast_mapping_lib<br/>Morton Octree + B-Spline Fusion]
        MAPMGR[MapManager]

        SYNC -->|Enqueue ImageFrame| QUEUE
        QUEUE -->|Dequeue| WORKER
        TF -->|Look up pose at timestamp| WORKER
        WORKER -->|Integrate Depth & Pose| FMLIB
        FMLIB -->|Voxel Grid & Free Space| MAPMGR
    end

    subgraph Outputs [Published ROS 2 Interfaces]
        GRID[<code>world/map</code><br/>nav_msgs/OccupancyGrid<br/>2D Planar Costmap / Nav2]
        FUSED[<code>world/fused_map</code><br/>visualization_msgs/MarkerArray<br/>3D Occupied Voxels]
        OCC[<code>world/occupancy</code><br/>visualization_msgs/MarkerArray<br/>3D Occupied + Free Voxels]
    end

    RGBD --> SYNC
    CINFO --> SYNC
    MAPMGR --> GRID
    MAPMGR --> FUSED
    MAPMGR --> OCC
```

### Key Differences from Standard OctoMap

| Feature | Standard OctoMap (`octomap_server`) | FastMapping (`fast_mapping`) | Architectural Impact |
| :--- | :--- | :--- | :--- |
| **Spatial Indexing** | Pointer-linked tree structure with dynamic per-node heap allocations. | **Morton Code (Z-order curve)** spatial hashing on contiguous arrays. | Eliminates pointer dereferencing and reduces cache misses with $O(1)$ block lookups. |
| **Memory Allocation** | Fine-grained allocations on heap per voxel leaf. | **Pre-allocated Chunked Memory Pools** (`MemoryPool<VoxelBlock>`). | Mitigates heap fragmentation and eliminates allocation spikes during map growth. |
| **Ray Integration** | Bresenham 3D ray-casting per depth pixel ($O(N_{\text{rays}} \cdot N_{\text{steps}})$). | **Projective Forward Mapping**: projects active voxel blocks into the camera frustum plane. | Computations scale with visible voxel volume rather than pixel count along each ray. |
| **Measurement Fusion** | Constant log-odds step increments ($\ell_{\text{hit}}$, $\ell_{\text{miss}}$). | **Continuous B-Spline Fusion (`bfusion`)** with pre-computed sensor noise lookup. | Produces smooth surface boundaries and increases robustness to camera depth noise. |
| **Planar Grid Generation** | Requires auxiliary nodes to project 3D leaf voxels into 2D space. | **Integrated MapManager**: directly generates planar 2D grids during publish cycles. | Provides native `nav_msgs/msg/OccupancyGrid` without secondary conversion nodes. |

---

## Source Code

The source repository for this component is hosted at:
[FastMapping on GitHub](https://github.com/open-edge-platform/edge-ai-suites/tree/main/robotics-ai-suite/components/fast-mapping)

---

## Prerequisites and Installation

### Supported Systems

Complete target platform configuration following the [Platform Foundation Getting Started Guide](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/).

### Install via APT

Install the pre-built Debian packages from the Robotics AI Suite repository:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
sudo apt update
sudo apt install -y ros-jazzy-fast-mapping
```

:::
:::{tab-item} **Humble**
:sync: humble

```bash
sudo apt update
sudo apt install -y ros-humble-fast-mapping
```

:::
::::

:::{note}
The `ros-${ROS_DISTRO}-fast-mapping` package includes a sample ROS 2 bag file for testing and validation. After installation, the bag is located at `/opt/ros/${ROS_DISTRO}/share/bagfiles/spinning/`.
:::

### Environment Setup

Source the target ROS 2 environment in every terminal:

::::{tab-set}
:::{tab-item} **Jazzy**
:sync: jazzy

```bash
source /opt/ros/jazzy/setup.bash
```

:::
:::{tab-item} **Humble**
:sync: humble

```bash
source /opt/ros/humble/setup.bash
```

:::
::::

---

## ROS 2 Integration Interfaces

### Subscribed Topics

| Topic | Message Type | Encoding / Format | Description |
| :--- | :--- | :--- | :--- |
| `depth_info_topic` | `sensor_msgs/msg/CameraInfo` | N/A | Camera intrinsic parameters (focal length $f_x, f_y$ and principal point $c_x, c_y$). Default: `camera/aligned_depth_to_color/camera_info`. |
| `depth_topic_1` | `sensor_msgs/msg/Image` | `16UC1` (depth in mm) or `32FC1` (depth in m) | Primary depth image stream. Default: `camera/aligned_depth_to_color/image_raw`. |
| `depth_topic_2` | `sensor_msgs/msg/Image` | `16UC1` or `32FC1` | Secondary depth stream (active when `depth_cameras >= 2`). Default: `camera_left/aligned_depth_to_color/image_raw`. |
| `depth_topic_3` | `sensor_msgs/msg/Image` | `16UC1` or `32FC1` | Third depth stream (active when `depth_cameras >= 3`). Default: `camera_right/aligned_depth_to_color/image_raw`. |
| `depth_topic_4` | `sensor_msgs/msg/Image` | `16UC1` or `32FC1` | Fourth depth stream (active when `depth_cameras >= 4`). Default: `camera_rear/aligned_depth_to_color/image_raw`. |

:::{important}
In multi-camera configurations, all depth cameras must share identical optical resolution and intrinsic models when subscribing to a shared `depth_info_topic`.
:::

### Published Topics

| Topic | Message Type | QoS Durability | Description |
| :--- | :--- | :--- | :--- |
| `world/map` | `nav_msgs/msg/OccupancyGrid` | **Transient Local** (Reliable, KeepLast(1)) | 2D occupancy grid projected between `projection_min_z` and `projection_max_z`. Fully compatible with Nav2 costmap layers. |
| `world/fused_map` | `visualization_msgs/msg/MarkerArray` | **Transient Local** (Reliable, KeepLast(1)) | 3D visual cube markers representing occupied voxels in the world. |
| `world/occupancy` | `visualization_msgs/msg/MarkerArray` | **Transient Local** (Reliable, KeepLast(1)) | 3D visual markers for both occupied and observed free voxels in the world. |

:::{note}
FastMapping publishers use **Transient Local durability** with **Reliable reliability**. Subscribers (including Nav2 costmap layers and custom listener nodes) must configure their QoS profile to `transient_local` durability to ensure historical map messages are received when connecting after node initialization.
:::

### Transform (TF) Requirements

`fast_mapping_node` requires continuous coordinate frame transformations in the TF2 tree connecting the target global map frame (`map_frame`, default: `"map"`) to the optical frame indicated in the `header.frame_id` of each depth image (e.g., `camera_color_optical_frame`).

```text
map ──► odom ──► base_footprint ──► base_link ──► camera_link ──► camera_color_optical_frame
```

The node uses asynchronous TF lookups with linear interpolation:

- The `tf_delay` parameter (default: `0.7` seconds) defines the permissible temporal tolerance when querying poses for incoming frames.
- If the TF tree fails to provide a transformation within the `tf_delay` window, the frame is dropped to prevent unaligned map distortion.

For tabletop testing without an active SLAM module or robot base, publish a static transform between `map` and your camera optical frame:

```bash
ros2 run tf2_ros static_transform_publisher 0 0 1 0 0 0 map camera_color_optical_frame
```

---

## Configuration Parameters

All parameters can be supplied via a ROS 2 YAML parameter file or overridden from the command line:

| Parameter | Type | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- |
| `map_frame` | `string` | `"map"` | - | Target global coordinate frame ID. |
| `voxel_size` | `float` | `0.04` | meters | Resolution (leaf size) of each 3D voxel ($0.04 = 4\text{ cm}$). Smaller values increase spatial resolution but consume more memory and compute. |
| `max_depth_range` | `float` | `3.0` | meters | Maximum depth ray distance integrated into the map. Depth pixels exceeding this distance are ignored to avoid far-field sensor noise. |
| `projection_min_z` | `float` | `0.1` | meters | Minimum vertical height (z-axis) relative to `map_frame` included in the 2D planar grid projection. |
| `projection_max_z` | `float` | `1.0` | meters | Maximum vertical height (z-axis) relative to `map_frame` included in the 2D planar grid projection. |
| `robot_radius` | `float` | `0.2` | meters | Clearance radius around the robot base cleared as free space in the planar grid. |
| `noise_factor` | `float` | `0.02` | - | Sensor noise factor scaling the B-spline uncertainty model around surface boundaries. |
| `tf_delay` | `float` | `0.7` | seconds | Permissible buffer delay for TF transform lookup synchronization. |
| `zmin` | `float` | `-inf` | meters | Lower vertical bound for 3D volumetric voxel integration. |
| `zmax` | `float` | `inf` | meters | Upper vertical bound for 3D volumetric voxel integration. |
| `depth_cameras` | `int` | `1` | count | Total count of synchronized depth camera streams (`1` to `4`). |
| `depth_info_topic` | `string` | `camera/aligned_depth_to_color/camera_info` | - | Topic publishing camera intrinsics. |
| `depth_topic_1` | `string` | `camera/aligned_depth_to_color/image_raw` | - | Topic for primary camera depth stream. |
| `depth_topic_2` | `string` | `camera_left/aligned_depth_to_color/image_raw` | - | Topic for secondary camera depth stream. |
| `depth_topic_3` | `string` | `camera_right/aligned_depth_to_color/image_raw` | - | Topic for third camera depth stream. |
| `depth_topic_4` | `string` | `camera_rear/aligned_depth_to_color/image_raw` | - | Topic for fourth camera depth stream. |

### Parameter Tuning Guide for Common Robotics Scenarios

- **Indoor Mobile Robot Navigation (AMR with Nav2):**
  - Set `voxel_size:=0.05` ($5\text{ cm}$) to match typical Nav2 costmap resolutions.
  - Set `projection_min_z:=0.05` (just above ground plane to prevent floor reflections from appearing as obstacles).
  - Set `projection_max_z:=1.20` (matching the highest point on the robot chassis or payload).
  - Set `robot_radius:=0.35` (matching robot footprint).
- **Compute-Constrained Platforms (e.g., low-power Intel Atom® or Edge SBCs):**
  - Set `voxel_size:=0.08` or `0.10` ($8\text{--}10\text{ cm}$).
  - Set `max_depth_range:=2.5` to limit ray-casting volume.
- **Tabletop or Manipulator 3D Reconstruction:**
  - Set `voxel_size:=0.01` or `0.02` ($1\text{--}2\text{ cm}$) for fine geometric detail.
  - Set `zmin:=0.0` and `zmax:=1.0` to constrain map generation to the workspace.

---

## Integration Workflows

### 1. Integrating FastMapping with Nav2 Costmaps

To use FastMapping as a real-time obstacle detection and local/global costmap layer in Nav2:

1. Launch `fast_mapping_node` mapped to the robot's depth camera and TF tree:

   ```bash
   ros2 run fast_mapping fast_mapping_node --ros-args \
       -p map_frame:=map \
       -p voxel_size:=0.05 \
       -p projection_min_z:=0.05 \
       -p projection_max_z:=1.20 \
       -p robot_radius:=0.30 \
       -p depth_topic_1:=/camera/depth/image_rect_raw \
       -p depth_info_topic:=/camera/depth/camera_info
   ```

2. In your Nav2 costmap configuration (`nav2_params.yaml`), configure a static or obstacle layer subscribing to `world/map`. Note the requirement for `map_subscribe_transient_local: True`:

   ```yaml
   global_costmap:
     global_costmap:
       ros__parameters:
         update_frequency: 5.0
         publish_frequency: 2.0
         global_frame: map
         robot_base_frame: base_link
         use_sim_time: True
         plugins: ["static_layer", "obstacle_layer", "inflation_layer"]
         obstacle_layer:
           plugin: "nav2_costmap_2d::StaticLayer"
           enabled: True
           map_subscribe_transient_local: True
           topic: "world/map"
         inflation_layer:
           plugin: "nav2_costmap_2d::InflationLayer"
           cost_scaling_factor: 3.0
           inflation_radius: 0.55
   ```

### 2. Multi-Camera 360-Degree Surround Mapping

For robots equipped with multiple depth cameras (e.g., front- and side-facing cameras):

```bash
ros2 run fast_mapping fast_mapping_node --ros-args \
    -p depth_cameras:=2 \
    -p depth_topic_1:=/camera_front/depth/image_rect_raw \
    -p depth_topic_2:=/camera_left/depth/image_rect_raw \
    -p depth_info_topic:=/camera_front/depth/camera_info
```

FastMapping uses an `ApproximateTime` synchronizer with a 30-frame message buffer to align timestamps between cameras before integrating depth observations into the shared 3D volume.

### 3. Live SLAM with Intel® RealSense™ and RTAB-Map

FastMapping includes a reference launch pipeline combining an Intel® RealSense™ D400 series camera, RTAB-Map SLAM for visual odometry, and RViz2:

1. Install RTAB-Map ROS dependencies:

   ::::{tab-set}
   :::{tab-item} **Jazzy**
   :sync: jazzy

   ```bash
   sudo apt install -y ros-jazzy-rtabmap-ros
   ```

   :::
   :::{tab-item} **Humble**
   :sync: humble

   ```bash
   sudo apt install -y ros-humble-rtabmap-ros
   ```

   :::
   ::::

2. Launch the integrated pipeline:

   ```bash
   ros2 launch fast_mapping fast_mapping_rtabmap.launch.py
   ```

The launch file starts:

- RViz2 with pre-configured display profiles.
- RTAB-Map SLAM after a 10-second delay for TF stabilization.
- `fast_mapping_node` after 12 seconds.
- RealSense camera node with depth alignment enabled after 15 seconds.

### 4. Standalone Sample Execution with Recorded ROS 2 Bag Data

FastMapping installs a pre-recorded dataset of a mobile robot rotating in an office environment:

```bash
ros2 launch fast_mapping fast_mapping.launch.py
```

Expected video demonstration:
[FastMapping Sample Video](https://github.com/open-edge-platform/edge-ai-suites/blob/main/robotics-ai-suite/docs/robotics/videos/fast_mapping.mp4)

---

## RViz2 Visualization Setup

To manually visualize FastMapping outputs in an existing RViz2 session:

1. Set the **Fixed Frame** to `map`.
2. Add a **Map** display:
   - Topic: `/world/map`
   - Durability Policy: `Transient Local`
   - Color Scheme: `costmap` or `map`
3. Add a **MarkerArray** display for 3D occupied voxels:
   - Topic: `/world/fused_map`
4. Add a **MarkerArray** display for complete occupancy (free and occupied):
   - Topic: `/world/occupancy`
5. Add a **TF** display to observe the camera and robot coordinate axes in real time.

---

## Runtime Diagnostics and Diagnostics Interpretation

`fast_mapping_node` periodically logs operational statistics (every 3 seconds by default, configured via `p_stat_log_interval_`):

```text
[INFO] [fast_mapping]: fast_mapping got 90 images in 3.0s. Aligned 90. Processed 90 (30.00 Hz). 0 left in queue
```

- **`images`**: Number of raw depth images received by the subscriber.
- **`Aligned`**: Number of frames with valid depth data synchronized across all cameras.
- **`Processed`**: Number of frames successfully matched with a TF pose and integrated into the octree.
- **`Hz`**: Effective mapping throughput.
- **`left in queue`**: Number of pending frames in the buffer. If this number continually grows, the compute platform is saturated; increase `voxel_size` or decrease input camera frame rate.

---

## Troubleshooting

- **`[INFO] [fast_mapping]: waiting for camera depth info from ...`**
  - **Cause:** `fast_mapping_node` has not received a `sensor_msgs/msg/CameraInfo` message.
  - **Resolution:** Verify that camera intrinsics are published using `ros2 topic echo <depth_info_topic> --once`. Ensure the topic name passed to `depth_info_topic` matches your camera driver.
- **TF Transform Lookup Failure / Missing Transforms**
  - **Cause:** No active TF link connects `map_frame` to the optical frame indicated in depth image headers.
  - **Resolution:** Inspect the TF tree with `ros2 run tf2_tools view_frames`. Ensure your SLAM or odometry module publishes `map -> odom` and your robot state publisher publishes `base_link -> camera_color_optical_frame`. If network latency delays TF frames, increase `tf_delay:=1.0`.
- **Map Not Displaying in RViz2 or Nav2 Costmaps**
  - **Cause:** QoS durability mismatch. FastMapping publishes on `transient_local`.
  - **Resolution:** In RViz2, expand the Map display topic settings and change **Durability Policy** from `Volatile` to `Transient Local`. In Nav2, set `map_subscribe_transient_local: True` in `nav2_params.yaml`.
- **`[fast_mapping] Image has distortion. Not yet supported!`**
  - **Cause:** Camera provides raw unrectified depth with significant distortion coefficients.
  - **Resolution:** Use rectified depth topics (e.g., `aligned_depth_to_color/image_raw` or `image_rect_raw`).
- **For general platform assistance**, refer to the [Robotics AI Suite Troubleshooting Guide](https://developer.robotics.intel.com/development-stack/resources/troubleshooting/).
