# Robotics AI Suite

## Autonomous Mobile Robot

For robots that navigate and operate independently in dynamic environments such as warehouses or factories.

| Algorithm | Documentation | Description |
| --------- | ------------- | ----------- |
| [ADBScan](adbscan) | [ADBScan](https://developer.robotics.intel.com/development-stack/components/optimized_solutions/adbscan-follow-me/index.html) | ADBSCAN (Adaptive DBSCAN) is an Intel-patented algorithm. It is a highly adaptive and scalable object detection and localization (clustering) algorithm, tested successfully to detect objects at all ranges for 2D Lidar, 3D Lidar, and RealSense™ depth camera. |
| [Collaborative-SLAM](collaborative-slam) | [Collaborative-SLAM](https://developer.robotics.intel.com/development-stack/components/optimized_solutions/collaborative-slam/index.html) | A Collaborative Visual SLAM example that is compiled natively for both Intel® Core™ and Intel® Atom® processor-based systems. In addition, GPU acceleration may be enabled on selected Intel® Core™ processor-based system. |
| [Fastmapping](fast-mapping) | [Fastmapping](https://developer.robotics.intel.com/development-stack/components/optimized_solutions/run-fastmapping-algorithm/index.html) | FastMapping application is the Intel® optimized version of octomap. |
| [GroundFloor Segmentation](groundfloor) | [GroundFloor Segmentation](https://developer.robotics.intel.com/development-stack/components/sensors/reference_applications/pointcloud-groundfloor-segmentation/index.html) | Showcases an Intel® algorithm designed for the segmentation of depth sensor data, compatible with 3D LiDAR or a RealSense™ camera inputs |
| [ITS-Planner](its-planner) | [ITS-Planner](https://developer.robotics.intel.com/development-stack/components/navigation/its-path-planner-plugin/index.html) | Intelligent Sampling and Two-Way Search (ITS) global path planner is an Intel-patented algorithm. ITS is a new search approach based on two-way path planning and intelligent sampling, which reduces the compute time by about 20x-30x on a 1000-node map comparing with the A* search algorithm. |
| [Multi-Camera-Demo](multicam-demo) | [Multicam-Demo](https://developer.robotics.intel.com/development-stack/ai_resources/openvino/reference_applications/openvino_multicam_demo/index.html) | Demonstrates the multi-camera use case using an Axiomtek ROBOX500 ROS2 AMR controller and four RealSense™ depth cameras D457 |
| [Object Detection](object-detection) | [Object Detection](https://developer.robotics.intel.com/development-stack/ai_resources/openvino/reference_applications/object_detection_tutorial/index.html) | An example on using the ROS 2 node with OpenVINO toolkit. It outlines the steps for installing the node and executing the object detection model. |
| [Simulations](simulations) | [Simulations](https://developer.robotics.intel.com/development-stack/software_references/amr/simulation/index.html) | Tutorials on using the ROS 2 simulations with the Autonomous Mobile Robot. You can test robot sensing and navigation in these simulated environments. |
| [Wandering](wandering) | [Wandering](https://developer.robotics.intel.com/development-stack/software_references/amr/simulation/wandering_sim/index.html) | Wandering mobile robot application is a ROS 2 sample application. It can be used with different SLAM algorithms in combination with the ROS2 navigation stack, to move the robot around in an unknown environment. The goal is to create a navigational map of the environment. |

## Building and Packaging

### Current Status

Each component currently manages its own build, test, and packaging targets through a dedicated `Makefile` within its directory (for example, `fast-mapping/` or `groundfloor/`). Most components build via Docker containers with ROS 2 and `colcon`, and generate Debian packages (`.deb`) using `dpkg-buildpackage`.

To build, test, or package an individual component, navigate into its folder:

```bash
# Example: Fast Mapping
cd src/components/fast-mapping
make build
make test
make package
```

Refer to each component's README and `Makefile` for specific options and supported `ROS_DISTRO` settings.

## Canonical Workshop Environments

Most components have a `workshop/` directory: a [Canonical Workshop](https://ubuntu.com/workshop/docs/) definition that installs the component with the Robotics AI Suite installer in an LXD container, with the host's Intel GPU, NPU, camera and desktop passed through, in Jazzy and Humble variants side by side. The Robotics System Profiler's workshop is in [ros-kpi/workshop](ros-kpi/workshop/README.md), including a Lyrical variant on Ubuntu 26.04. Shared SDKs, scripts and documentation are in [src/workshops](../workshops/README.md).
