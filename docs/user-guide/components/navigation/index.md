# Dynamic and Autonomous Navigation

Dynamic and autonomous navigation is the core capability that enables mobile robots—ranging from industrial Autonomous Mobile Robots (AMRs) and automated guided vehicles (AGVs) to bipedal humanoids—to understand their surroundings, establish robust localization, safely maneuver around moving obstacles, and execute missions in unpredictable environments.

The Robotics AI Suite provides an end-to-end, hardware-accelerated navigation architecture based on **ROS 2 Nav2** (supporting ROS 2 Jazzy and Humble). This foundation is augmented by Intel-patented global path planners, adaptive spatial clustering algorithms, 3D volumetric voxel mapping, direct LiDAR-inertial-visual odometry pipelines, multi-robot collaborative SLAM, and automated re-localization.


## Navigation Architecture

The navigation stack integrates sensor feeds, high-rate state estimation, multi-layer costmaps, global trajectory planners, dynamic local controllers, and autonomous recovery mechanisms:

```{mermaid}
flowchart TD
    subgraph Sensors["Sensor Acquisition"]
        RS["RealSense Depth Cameras\n(D435i / D455 / D415)"]
        LiDAR["2D / 3D LiDAR Scanners\n(Livox Mid-360 / Velodyne / Ouster)"]
        IMU["Inertial Measurement Units\n(High-Rate 6-Axis / 9-Axis IMU)"]
    end

    subgraph State_Estimation["State Estimation & SLAM"]
        LIO["LIO / LIVO Odometry Engines\n(FAST-LIO2 / FAST-LIVO2 / Point-LIO)"]
        CollabSLAM["Collaborative Visual SLAM\n(Multi-Robot SSE/AVX2/LZE Mapping)"]
        Reloc["Robot Re-localization\n(Kidnapped Robot Recovery)"]
    end

    subgraph Costmaps["3D Perception & Dynamic Costmaps"]
        GF["3D Groundfloor Segmentation\n(Traversability & Slope Classification)"]
        FM["FastMapping Algorithm\n(3D OctoMap Voxel Grid)"]
        ADB["Adaptive DBSCAN (ADBScan)\n(Dynamic Obstacle Clustering & Tracking)"]
        Costmap2D["Nav2 Costmap 2D Servers\n(Static + Voxel + ADBScan + Inflation)"]
    end

    subgraph Planning_Control["Nav2 Navigation & Motion Planning"]
        BT["Nav2 Behavior Tree Navigator\n(bt_navigator & Action Execution)"]
        ITS["ITS Global Path Planner\n(20-30x Acceleration over A*)"]
        Controller["Nav2 Controller Server\n(DWB / MPPI Dynamic Tracking)"]
        Wandering["Autonomous Frontier Exploration\n(wandering_app Pipeline)"]
    end

    subgraph Actuation["Actuator Execution"]
        CmdVel["Motor Controller / Base\n(/cmd_vel Velocity Commands)"]
    end

    RS --> State_Estimation
    LiDAR --> State_Estimation
    IMU --> State_Estimation

    RS --> Costmaps
    LiDAR --> Costmaps

    State_Estimation -->|/odom, TF odom -> base_link| Planning_Control
    State_Estimation -->|/map, TF map -> odom| Planning_Control
    Reloc -->|Pose Recovery Trigger| BT

    Costmaps --> Costmap2D
    Costmap2D --> ITS
    Costmap2D --> Controller

    BT --> ITS
    BT --> Controller
    Wandering --> BT
    Controller --> CmdVel
```


## Core Navigation Pillars

Explore the guides below to learn how each navigation component is configured, optimized, and deployed across the Robotics AI Suite:

::::{grid} 2

:::{grid-item-card} **ROS 2 Nav2 Core Integration**
:link: nav2-integration
:link-type: doc
:link-alt: clickable cards

Architecture, lifecycle management, action servers, behavior trees, and standard parameters for Nav2 on ROS 2 Jazzy.
:::

:::{grid-item-card} **ITS Global Path Planner**
:link: its-path-planner-plugin
:link-type: doc
:link-alt: clickable cards

Patented two-way search and intelligent sampling global planner providing 20–30x speedups over $A^*$.
:::

:::{grid-item-card} **Robot Re-localization**
:link: navigation-relocalization
:link-type: doc
:link-alt: clickable cards

Rapid, memory-efficient pose recovery for kidnapped robot events and sensor dropouts in Nav2.
:::

:::{grid-item-card} **Dynamic Obstacle Avoidance & 3D Costmaps**
:link: dynamic-obstacle-avoidance
:link-type: doc
:link-alt: clickable cards

Real-time spatial clustering with ADBScan, 3D voxel representation with FastMapping, and traversability ground plane segmentation.
:::

:::{grid-item-card} **LiDAR & Visual Odometry (LIO & LIVO)**
:link: lio-livo-pipelines
:link-type: doc
:link-alt: clickable cards

Direct 6-DoF state estimation using FAST-LIO2, FAST-LIVO2, and Point-LIO on Intel platforms with CPU core pinning.
:::

:::{grid-item-card} **Collaborative Visual SLAM**
:link: ../optimized_solutions/collaborative-slam
:link-type: doc
:link-alt: clickable cards

Multi-robot mapping and map merging accelerated with AVX2 and Level-Zero compute kernels for Intel CPUs and GPUs.
:::

:::{grid-item-card} **FastMapping Algorithm**
:link: ../optimized_solutions/run-fastmapping-algorithm
:link-type: doc
:link-alt: clickable cards

Real-time 3D OctoMap voxel mapping from RealSense depth cameras for multi-level spatial awareness.
:::

:::{grid-item-card} **Autonomous Frontier Exploration**
:link: ../../software_references/amr/simulation/wandering_sim
:link-type: doc
:link-alt: clickable cards

Simulate and deploy the autonomous Wandering pipeline combining RTAB-Map SLAM, ADBScan, and Nav2.
:::

::::


## Dynamic & Autonomous Navigation Capabilities

### 1. High-Performance Global Path Planning: ITS Planner
In large warehouse layouts or crowded factory floors with thousands of navigation nodes, traditional grid-search algorithms such as $A^*$ or Dijkstra can exhibit severe calculation latency when planning complex routes. The **Intelligent Sampling and Two-Way Search (ITS)** global path planner accelerates route calculation by **20–30x** over $A^*$ on 1,000-node maps. It builds reusable Probabilistic Road Maps (PRM) or Deterministic Road Maps (DRM) and applies smoothing filters or Catmull-Rom spline interpolation to output dynamically feasible paths.

### 2. Rapid Re-localization & Kidnapped Robot Recovery
Robots in industrial facilities occasionally lose localization due to rapid dynamic occlusion, symmetrical corridors, wheel slip over spills, or temporary sensor dropouts. The **Robot Re-localization Package** provides an algorithm specifically tuned for mobile robots that swiftly searches candidate poses and restores accurate orientation and position without requiring full manual re-initialization in RViz2.

### 3. Dynamic Obstacle Avoidance & Adaptive Clustering
Static occupancy grids assume a static environment. In real-world environments, moving humans and equipment enter the robot's immediate corridor. Intel's **Adaptive DBSCAN (ADBScan)** algorithm solves the point-density degradation problem of LiDAR and depth cameras by dynamically modulating clustering radii as a function of range. Clustered obstacle bounding boxes feed directly into the **`nav2_adbscan_layer`**, allowing local controllers like DWB and MPPI to execute proactive evasion maneuvers without waiting for slow grid cell decays.

### 4. 3D Volumetric Mapping & Traversability Analysis
To navigate safely through complex 3D environments, robots must identify overhanging obstacles, suspended conveyor belts, low tables, and floor drop-offs. **FastMapping** constructs dynamic 3D voxel representations in real time from RealSense RGB-D feeds, while the **3D Pointcloud Groundfloor Segmentation** package separates traversable ramps and sloped ground from actual physical obstacles.

### 5. Robust State Estimation with LIO & LIVO Pipelines
Wheel encoders slip on slick floors, and 2D scan matchers fail in featureless hallways. The suite integrates direct LiDAR-Inertial Odometry (**FAST-LIO2**, **Point-LIO**) and direct LiDAR-Inertial-Visual Odometry (**FAST-LIVO2**). These frameworks compute drift-free 6-DoF odometry by fusing solid-state or spinning LiDARs, 6-axis IMUs, and direct photometric visual alignment without compute-intensive feature extractors.

### 6. Multi-Robot Collaborative Mapping
When fleets of robots map an expansive facility simultaneously, **Collaborative Visual SLAM** allows each robot to stream local keyframes and landmarks to a central agent. Intel-optimized SSE, AVX2, and Level-Zero instruction pipelines execute real-time loop closure, global bundle adjustment, and map merging across the entire fleet.

### 7. Autonomous Frontier Exploration (`wandering`)
For unknown environment exploration, the suite provides the **`wandering`** reference application. The robot autonomously discovers frontiers, builds occupancy maps using SLAM (e.g. RTAB-Map or Collaborative SLAM), marks dynamic objects with ADBScan, and navigates toward unexplored regions until full map coverage is achieved.


## Navigation Solutions and Reference Implementations

The following table indexes navigation packages, tutorials, and end-to-end reference applications available in the suite:

| Solution / Ingredient | Category | Key Hardware & Algorithms | Documentation |
| --- | --- | --- | --- |
| **Nav2 Core Integration** | Middleware / Planning | ROS 2 Jazzy/Humble, BT Navigator, Costmap 2D, DWB, MPPI | [ROS 2 Nav2 Core Integration](nav2-integration.md) |
| **ITS Path Planner** | Global Planning | Patented two-way search, PRM/DRM roadmaps, spline smoothing | [ITS Path Planner ROS 2 Navigation Plugin](its-path-planner-plugin.md) |
| **Robot Re-localization** | Localization Recovery | Low-memory candidate pose scoring, recovery behavior | [Robot Re-localization Package for ROS 2 Navigation](navigation-relocalization.md) |
| **Dynamic Obstacle Avoidance** | Costmaps / Perception | Adaptive DBSCAN (`nav2_adbscan_layer`), RealSense, LiDAR | [Dynamic Obstacle Avoidance and Costmap Layers](dynamic-obstacle-avoidance.md) |
| **LIO & LIVO Pipelines** | Odometry / State Estimation | FAST-LIO2, FAST-LIVO2, Point-LIO, Livox Mid-360, IMU | [LiDAR and Visual Odometry Pipelines (LIO & LIVO)](lio-livo-pipelines.md) |
| **Collaborative Visual SLAM** | Mapping / SLAM | Multi-robot map merging, SSE/AVX2/Level-Zero optimization | [Collaborative Visual SLAM](../optimized_solutions/collaborative-slam.md) |
| **FastMapping Algorithm** | 3D Voxel Mapping | OctoMap 3D voxel generation from RealSense depth cameras | [FastMapping Algorithm](../optimized_solutions/run-fastmapping-algorithm.md) |
| **ADBSCAN Follow-me** | Tracking / Perception | Adaptive point cloud clustering, person tracking and following | [ADBSCAN Follow-me](../optimized_solutions/adbscan-follow-me.md) |
| **3D Groundfloor Segmentation** | Traversability Analysis | Normal estimation and plane fitting for RealSense/LiDAR | [3D Pointcloud Groundfloor Segmentation for RealSense Camera and 3D LiDAR](../sensors/reference_applications/pointcloud-groundfloor-segmentation.md) |
| **FAST-LIO2 Sample Pipeline** | LIO Reference | Colcon build, patch suite, Livox Mid-360, profiling | [LIO SLAM: FAST-LIO2](../../software_references/humanoid/sample_pipelines/fast_lio2_demo.md) |
| **FAST-LIVO2 Sample Pipeline** | LIVO Reference | Direct visual-inertial-LiDAR fusion, NTU VIRAL validation | [LIVO SLAM: FAST-LIVO2](../../software_references/humanoid/sample_pipelines/fast_livo2_demo.md) |
| **Point-LIO Sample Pipeline** | LIO Reference | Point-by-point high-frequency estimation for agile platforms | [LIO SLAM: Point-LIO](../../software_references/humanoid/sample_pipelines/point_lio_demo.md) |
| **Simulating `wandering`** | AMR Simulation | Gazebo Harmonic digital twin, RTAB-Map, Nav2, ADBScan | [Simulating `wandering` in Gazebo](../../software_references/amr/simulation/wandering_sim.md) |
| **Deploying `wandering`** | Physical Deployment | AMR bringup on Intel® Core™ Ultra Series 3 Processors, RealSense, Nav2 | [Deploying `wandering`](../../software_references/amr/deployment/wandering_deploy.md) |


:::{toctree}
:maxdepth: 1
:hidden:

nav2-integration
its-path-planner-plugin
navigation-relocalization
dynamic-obstacle-avoidance
lio-livo-pipelines
:::
