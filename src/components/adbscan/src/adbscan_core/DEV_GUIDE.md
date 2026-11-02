<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Developer Guide: Integrating `adbscan_core` in Your Codebase

This guide walks third-party and internal C++ developers through integrating the `adbscan_core` clustering library into their own software stacks.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Prerequisites & Build Requirements](#2-prerequisites--build-requirements)
3. [Installing or Building the Library](#3-installing-or-building-the-library)
4. [CMake Integration](#4-cmake-integration)
5. [Complete Working Code Example](#5-complete-working-code-example)
6. [API Architecture & Reference](#6-api-architecture--reference)
   - [Coordinate Frame and Units](#coordinate-frame-and-units)
   - [Core Data Structures](#core-data-structures)
   - [Functions & Signatures](#functions--signatures)
   - [Search Backends & Hardware Acceleration](#search-backends--hardware-acceleration)
   - [Error Handling & Edge Cases](#error-handling--edge-cases)
7. [Parameter Tuning & Sensor Baselines](#7-parameter-tuning--sensor-baselines)

---

## 1. Overview

`adbscan_core` is a lightweight, standalone C++17 library implementing **Adaptive Density-Based Spatial Clustering of Applications with Noise (ADBSCAN)**.

### Why ADBSCAN?

Standard DBSCAN requires fixed clustering radius ($\epsilon$) and minimum neighbor count ($MinPts$). However, real-world spatial sensors (such as LiDAR and structured-light or Time-of-Flight depth cameras) suffer from beam divergence and angular dispersion: points naturally become sparser as distance from the sensor increases.

- A fixed $\epsilon$ that works at close range fragments distant objects into noise.
- A fixed $\epsilon$ tuned for long range bridges distinct objects at close range into single massive clusters.

ADBSCAN resolves this by dynamically scaling $\epsilon(x)$ and $MinPts(x)$ as a function of range $x$, boosting effective object detection range by 20% to 30% without requiring neural network inference.

### Library Boundaries

- **No ROS Middleware Dependency**: The library operates strictly on standard C++ types (`std::vector<adbscan_core::Point>`) and has no dependency on ROS 2 messages, node handles, optical transforms, or TF.
- **Pluggable Backends**: Out of the box, `adbscan_core` provides a fast direct-scan CPU backend (`LegacyCpu`). When built with oneAPI support, it also provides PCL KD-tree and SYCL-accelerated GPU/CPU KD-tree and octree search backends.

---

## 2. Prerequisites & Build Requirements

- **C++ Compiler**: C++17 compliant (GCC 9+, Clang 10+, or Intel oneAPI DPC++/C++ Compiler `icpx` 2024+).
- **CMake**: Version 3.16 or higher.
- **Build System**:
  - `adbscan_core` uses `ament_cmake` as its packaging buildtool.
  - To build from source, an active ROS 2 environment (Humble or Jazzy) or an `ament_cmake` workspace is required.
  - Once built and installed, downstream consumers can link `adbscan_core` in standard CMake projects using `find_package(adbscan_core REQUIRED)` as long as the install prefix is included in `CMAKE_PREFIX_PATH`.
- **Optional (Hardware Acceleration)**:
  - Intel® oneAPI Base Toolkit (DPC++ compiler, SYCL runtime).
  - OpenCL CPU runtime or Level Zero GPU driver.
  - PCL OneAPI development headers (`libpcl-oneapi-dev`, `libflann-oneapi-dev`).

---

## 3. Installing or Building the Library

### Option A: Install from Debian Packages (Recommended on Ubuntu)

If you are on Ubuntu 22.04 (Humble) or Ubuntu 24.04 (Jazzy) with the appropriate repositories configured:

```bash
sudo apt update
sudo apt install ros-${ROS_DISTRO}-adbscan-core
```

This installs the headers into `/opt/ros/${ROS_DISTRO}/include` and the library into `/opt/ros/${ROS_DISTRO}/lib`.

### Option B: Build from Source in a Colcon Workspace

1. Clone the repository into a workspace `src/` directory:

   ```bash
   mkdir -p ~/ws/src && cd ~/ws/src
   git clone <repository_url> .
   ```

2. Build with default CPU support:

   ```bash
   cd ~/ws
   source /opt/ros/${ROS_DISTRO}/setup.bash
   colcon build --packages-select adbscan_core
   ```

3. (Optional) Build with Intel oneAPI SYCL acceleration:

   ```bash
   colcon build --packages-select adbscan_core \
     --cmake-args -DWITH_ONEAPI=ON -DCMAKE_CXX_COMPILER=icpx
   ```

---

## 4. CMake Integration

Downstream CMake projects can find and link `adbscan_core` using modern CMake imported targets.

### Example `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_clustering_app LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find adbscan_core
find_package(adbscan_core REQUIRED)

# Define your executable or library
add_executable(my_clustering_app src/main.cpp)

# Link against the adbscan_core imported target
target_link_libraries(my_clustering_app PRIVATE adbscan_core::adbscan_core)
```

### Building Your Project

Ensure your shell has sourced the workspace or prefix where `adbscan_core` was installed so CMake can locate its configuration file:

```bash
# If installed via ROS 2 Debian packages:
source /opt/ros/${ROS_DISTRO}/setup.bash

# Or if installed from a custom local colcon build:
source ~/ws/install/setup.bash

# Configure and build
mkdir build && cd build
cmake ..
cmake --build .
```

*Note: If building outside of a sourced environment, explicitly provide the prefix to CMake:*

```bash
cmake -DCMAKE_PREFIX_PATH="/path/to/installed/adbscan_core" ..
```

---

## 5. Complete Working Code Example

Below is a complete, self-contained C++ application demonstrating both `cluster_frame()` (high-level, sensor-aware frame clustering) and `cluster()` (low-level, custom-threshold clustering).

```cpp
#include <iostream>
#include <vector>
#include <adbscan_core/clusterer.hpp>

int main()
{
  // 1. Prepare raw 3D point cloud data (e.g. from LiDAR or depth sensor)
  // Coordinates are Cartesian XYZ in meters (sensor/body frame).
  std::vector<adbscan_core::Point> raw_points = {
    // Cluster A (~1 meter ahead)
    {1.00f, 0.00f, 0.00f},
    {1.05f, 0.02f, 0.01f},
    {1.02f, -0.01f, 0.02f},
    {1.08f, 0.01f, -0.01f},

    // Cluster B (~3 meters ahead, to the left)
    {3.00f, 1.50f, 0.10f},
    {3.05f, 1.52f, 0.11f},
    {3.02f, 1.48f, 0.09f},
    {3.07f, 1.51f, 0.12f},

    // Isolated noise point (~5 meters away)
    {5.00f, -2.00f, 0.50f}
  };

  // --------------------------------------------------------------------------
  // Method 1: cluster_frame() - Automatic preprocessing & adaptive thresholds
  // --------------------------------------------------------------------------
  std::cout << "=== Running cluster_frame() ===" << std::endl;

  adbscan_core::FrameOptions options;
  options.mode = adbscan_core::FrameMode::Lidar3D;
  options.subsample_ratio = 1;       // Process all points (no downsampling)
  options.ground_removal = false;    // Set true if ground filtering is desired
  options.base = 2.5f;               // Density intercept
  options.coeff_1 = -0.03f;          // Linear range density coefficient
  options.coeff_2 = 0.00015f;        // Quadratic range density coefficient
  options.epsilon_scale_factor = 0.9f;
  options.min_3d_epsilon = 0.20f;    // Minimum search radius in meters

  // Run clustering with the default CPU backend
  adbscan_core::Result result = adbscan_core::cluster_frame(
    raw_points, options, adbscan_core::Backend::LegacyCpu);

  // Inspect the detected clusters
  std::cout << "Detected " << result.clusters.size() << " clusters." << std::endl;
  for (const auto & cluster : result.clusters) {
    std::cout << "\nCluster ID: " << cluster.id << std::endl;
    std::cout << "  Point count: " << cluster.point_count << std::endl;
    std::cout << "  Centroid (mean): ("
          << cluster.centroid.x << ", "
          << cluster.centroid.y << ", "
              << cluster.centroid.z << ") m" << std::endl;
    std::cout << "  Box Center:      ("
          << cluster.box_center.x << ", "
          << cluster.box_center.y << ", "
              << cluster.box_center.z << ") m" << std::endl;
    std::cout << "  Box Size (XYZ):  ("
          << cluster.box_size.x << ", "
          << cluster.box_size.y << ", "
              << cluster.box_size.z << ") m" << std::endl;
  }

  // Inspect per-point classifications
  // Types: -1 = Noise, 0 = Border point, 1 = Core point
  std::cout << "\nPer-point classifications:" << std::endl;
  for (std::size_t i = 0; i < result.labels.size(); ++i) {
    std::cout << "  Point " << i << ": Label=" << result.labels[i]
              << " (" << (result.labels[i] == -1 ? "Noise" : "In Cluster") << ")"
              << ", Type=" << result.types[i]
              << " (" << (result.types[i] == 1 ? "Core" : (result.types[i] == 0 ? "Border" : "Noise")) << ")"
              << std::endl;
  }

  // --------------------------------------------------------------------------
  // Method 2: cluster() - Direct invocation with caller-defined k and radius
  // --------------------------------------------------------------------------
  std::cout << "\n=== Running cluster() directly ===" << std::endl;

  // Supply explicit per-point min neighbor count (k) and search radius (meters)
  std::vector<double> k(raw_points.size(), 2.0);          // At least 2 points
  std::vector<double> radius(raw_points.size(), 0.30);    // 30 cm radius

  adbscan_core::Result direct_result = adbscan_core::cluster(
    raw_points, k, radius, adbscan_core::Backend::LegacyCpu);

  std::cout << "Direct cluster() found " << direct_result.clusters.size() << " clusters." << std::endl;

  return 0;
}
```

---

## 6. API Architecture & Reference

The public API is declared in `<adbscan_core/clusterer.hpp>`.

### Coordinate Frame and Units

- **Units**: All coordinates (`x`, `y`, `z`), bounding box sizes, search radii, and filter thresholds are expressed in **meters**.
- **Frame Convention**: Points must be supplied in a standard robot or sensor body Cartesian coordinate frame:
  - $+X$: Forward (sensor viewing direction)
  - $+Y$: Left
  - $+Z$: Up
- **Important Note for Depth Cameras**: Many cameras (such as Intel® RealSense™) output point clouds in an optical frame ($+Z$ forward, $+X$ right, $+Y$ down). You **must** rotate the points to the robot body frame ($+X$ forward) before passing them to `cluster_frame()`, as the internal range scaling calculations rely on the $+X$ coordinate.

### Core Data Structures

#### `adbscan_core::Point`

Represents an individual 3D point:

```cpp
struct Point {
  float x;
  float y;
  float z;
};
```

#### `adbscan_core::Cluster`

Summary metrics computed for each discovered cluster:

```cpp
struct Cluster {
  int id;                     // Positive label ID (1, 2, ...)
  std::size_t point_count;    // Total member points in the cluster
  Point centroid;             // Arithmetic mean of member coordinates
  Point box_center;           // Geometric center of the axis-aligned bounding box
  Point box_size;             // Extents [width_x, width_y, width_z] of the box in meters
};
```

> **Warning**: `centroid` and `box_center` are **not interchangeable**. The centroid represents point mass center and is sensitive to point density skew, whereas `box_center` is the exact geometric midpoint between minimum and maximum bounds.

#### `adbscan_core::Result`

Output returned by `cluster()` and `cluster_frame()`:

```cpp
struct Result {
  std::vector<int> labels;        // Label per point: -1 for noise, positive integer >= 1 for cluster
  std::vector<int> types;         // Type per point: -1 (noise), 0 (border), 1 (core)
  std::vector<Cluster> clusters;  // Summaries of positive clusters
};
```

#### `adbscan_core::FrameOptions`

Parameters configuring sensor frame preprocessing and dynamic threshold generation:

```cpp
struct FrameOptions {
  FrameMode mode = FrameMode::Lidar3D; // Planar, Lidar3D, or Depth
  unsigned int subsample_ratio = 1;    // Subsampling stride (must be >= 1)
  bool ground_removal = false;         // Enable Z-based ground plane clipping
  float z_filter = 0;                  // Remove points with z < z_filter when ground_removal=true
  float x_filter_back = 0;             // Depth mode: max X range in meters
  float y_filter_right = 0;            // Depth mode: min Y bound in meters (usually negative)
  float y_filter_left = 0;             // Depth mode: max Y bound in meters (usually positive)
  float base = 0;                      // Polynomial constant term for k(x)
  float coeff_1 = 0;                   // Polynomial linear coefficient for k(x)
  float coeff_2 = 0;                   // Polynomial quadratic coefficient for k(x)
  float scale_factor = 0;              // Radius multiplier for Planar & Depth modes
  float epsilon_scale_factor = 0.9f;   // Radius multiplier for Lidar3D mode
  float min_3d_epsilon = 0.9f;         // Minimum radius floor in meters for Lidar3D mode
};
```

### Functions & Signatures

#### `cluster_frame()`

```cpp
Result cluster_frame(
  const std::vector<Point> & points,
  const FrameOptions & options,
  Backend backend = Backend::LegacyCpu);
```

- Preprocesses points based on `options.mode`:
  - `Planar`: No downsampling or filtering.
  - `Lidar3D`: Subsamples by `subsample_ratio` and optionally removes ground points below `z_filter`.
  - `Depth`: Subsamples, optionally removes ground, and clips to the FOV bounding box (`x_filter_back`, `y_filter_right`, `y_filter_left`).
- Calculates adaptive $k(x)$ and radius $\epsilon(x)$ per point based on forward range $+X$ and overall point cloud volume.
- **Indices Note**: `result.labels` and `result.types` correspond to the **filtered/subsampled** points, not the raw input array.

#### `cluster()`

```cpp
Result cluster(
  const std::vector<Point> & points,
  const std::vector<double> & k,
  const std::vector<double> & radius,
  Backend backend = Backend::LegacyCpu);
```

- Performs core ADBSCAN clustering on pre-filtered points using user-supplied $k$ and $radius$ vectors.
- Requires `points.size() == k.size()` and `points.size() == radius.size()`.

### Search Backends & Hardware Acceleration

| Backend | Identifier | Build Requirement | Target Device | Notes |
| --- | --- | --- | --- | --- |
| **Legacy CPU** | `Backend::LegacyCpu` | Default (Standard C++) | CPU | Direct neighbor search; portable and zero external runtime dependencies. |
| **PCL KD-Tree** | `Backend::PclKdTree` | `-DWITH_ONEAPI=ON` | CPU | FLANN / PCL KD-tree CPU search. |
| **OneAPI KD-Tree**| `Backend::OneApiKdTree` | `-DWITH_ONEAPI=ON` | GPU or CPU | SYCL-accelerated parallel KD-tree search via Intel oneAPI. |
| **OneAPI Octree** | `Backend::OneApiOctree` | `-DWITH_ONEAPI=ON` | GPU or CPU | SYCL-accelerated parallel octree search. Note: has a 333-neighbor limit per query query. |

When using `OneApiKdTree` or `OneApiOctree`, select the runtime hardware device before launching your process via the `ONEAPI_DEVICE_SELECTOR` environment variable:

```bash
# Offload search to Intel integrated or discrete GPU
export ONEAPI_DEVICE_SELECTOR=level_zero:gpu
./my_clustering_app

# Run SYCL kernels on host CPU
export ONEAPI_DEVICE_SELECTOR=opencl:cpu
./my_clustering_app
```

### Error Handling & Edge Cases

- **Zero Subsample Ratio**: If `options.subsample_ratio == 0`, `cluster_frame()` throws `std::invalid_argument("subsample_ratio must be positive")`.
- **Vector Size Mismatch**: In `cluster()`, if `points.size() != k.size()` or `points.size() != radius.size()`, it throws `std::invalid_argument`.
- **Unavailable Backend**: Requesting a non-CPU backend (`PclKdTree`, `OneApiKdTree`, `OneApiOctree`) when built without `-DWITH_ONEAPI=ON` throws `std::invalid_argument("requested backend is not available in this build")`.
- **Empty Point Clouds**: An empty input vector returns an empty `Result` (empty `labels`, empty `types`, empty `clusters`). It does not throw.

---

## 7. Parameter Tuning & Sensor Baselines

`cluster_frame()` generates adaptive clustering parameters using:
$$k(x) = \left\lfloor \max\left(1.0, \text{coeff\_2} \cdot x^2 + \text{coeff\_1} \cdot x + \text{base}\right) \right\rfloor$$

$$\text{radius}(x) = \left(\frac{V \cdot k(x) \cdot 1.3293}{N \cdot \sqrt{\pi^3}}\right)^{1/3}$$

Where $V$ is point cloud bounding volume and $N$ is the number of points. Radius is then scaled by `scale_factor` (for `Planar` and `Depth`) or `epsilon_scale_factor` (for `Lidar3D`).

### Baseline Configurations

Because default-constructed `FrameOptions` leaves coefficients at `0`, use the verified baseline configurations below for common robotics sensor setups:

| Parameter | 2D LiDAR (e.g. RPLidar) | 3D LiDAR (e.g. Velodyne/Ouster) | Depth Camera (e.g. RealSense D435/D455) |
| --- | --- | --- | --- |
| `mode` | `FrameMode::Planar` | `FrameMode::Lidar3D` | `FrameMode::Depth` |
| `subsample_ratio` | `1` to `5` | `10` to `20` | `100` to `150` |
| `ground_removal` | `false` | `true` | `true` |
| `z_filter` | `-1.3` m (if enabled) | `-1.3` m | `-0.1` m |
| `x_filter_back` | N/A | N/A | `3.0` m |
| `y_filter_right`| N/A | N/A | `-2.0` m |
| `y_filter_left` | N/A | N/A | `2.0` m |
| `base` | `2.2458` | `3.4694` | `5.2954` |
| `coeff_1` | `-0.3501` | `-0.0335` | `-0.1648` |
| `coeff_2` | `-0.2243` | `0.00015` | `-0.0180` |
| `scale_factor` | `0.07` | N/A | `0.90` |
| `epsilon_scale_factor` | N/A | `0.70` to `0.90` | N/A |
| `min_3d_epsilon` | N/A | `0.30` to `0.90` m | N/A |

### Practical Tuning Tips

1. **Objects Split or Fragmented**: If single physical objects are broken into multiple clusters, increase `scale_factor` (or `epsilon_scale_factor`) to expand the search radius, or lower `base` to decrease minimum required density.
2. **Adjacent Objects Merged**: If separate nearby objects (e.g., legs of a person and a chair) merge into one cluster, decrease `scale_factor` or increase `base`.
3. **High Latency on Dense Clouds**: If clustering latency exceeds your real-time budget, increase `subsample_ratio` (e.g., from `50` to `150` for depth cameras) or build with `-DWITH_ONEAPI=ON` to utilize the GPU backends.
