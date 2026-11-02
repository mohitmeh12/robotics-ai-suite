<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ADBSCAN Documentation

---

ADBSCAN (Adaptive DBSCAN) is an Intel® patented algorithm. It is a
highly adaptive and scalable object detection and localization
(clustering) algorithm, tested successfully to detect objects at all
ranges for 2D Lidar, 3D Lidar, and Intel® RealSense™ depth camera. This
method automatically computes clustering parameters (radius and minimum
number of points that define a cluster) based on the distance from the
sensor and the data density in its field of view, thus alleviating the
guesswork from parameter selection and enabling efficient hierarchical
clustering. ADBSCAN increases detection range by 30%-40% and detects
20%-30% more objects, compared to the state-of-the-art methods. It has
been gainfully used in multiple applications such as 2D/3D Lidar or
Intel® RealSense™ based object tracking, multi-modal object
classification (Camera + Lidar), surface segmentation, Lidar-based
object classification, occupancy grid generation etc.

## Documentation Index

### Core Package & Algorithm Documentation

- [ADBSCAN Core Integration Guide](../src/adbscan_core/DEV_GUIDE.md) - End-to-end developer guide for integrating `adbscan_core` into external codebases, including CMake examples and parameter baselines.
- [ADBSCAN Core Library Overview](../src/adbscan_core/README.md) - Standalone C++ clustering library API (`cluster`, `cluster_frame`), neighbor backends, and oneAPI build options.
- [Dynamic Obstacle Messages (`nav2_dynamic_msgs`)](../src/nav2_dynamic_msgs/README.md) - Custom dynamic obstacle ROS 2 interfaces (`Obstacle`, `ObstacleArray`) used by `adbscan_ros2` and Nav2.
- [ADBSCAN ROS 2 Node Guide](../src/adbscan_ros2/Readme.md) - Configuration parameters, topic schemas, core backend selection, and RViz visualization.
- [Intel Architecture Optimized ADBSCAN](IA-optimized-adbscan-algorithm.md) - oneAPI GPU/CPU acceleration, parallel search benchmarks, and parameters.
- [ADBSCAN Behavior Baseline](adbscan-behavior-baseline.md) - Baseline characterization fixtures, test invariants, and timing benchmarks across backends.

### Follow-Me Application & Simulation Packages

- [Follow-Me Node (`adbscan_ros2_follow_me`)](../src/adbscan_ros2_follow_me/README.md) - Target follower tracking cluster centroids, velocity control, and topics.
- [Gazebo Simulation (`followme_turtlebot3_gazebo`)](../src/followme_turtlebot3_gazebo/README.md) - Gazebo Harmonic / Fortress simulation worlds, robot models, and launch configurations.
- [Follow-Me Application Guide](follow_me_readme.md) - High-level guide for running the target tracking follower in simulation and physical robots.
- [Follow-Me Requirements](follow_me_requirements.md) - System packages and ROS 2 dependencies for Humble and Jazzy.
- [Follow-Me TurtleBot3 Gazebo Simulation Overview](followme_turtlebot3_gazebo_readme.md) - Simulation world configurations, wandering guide robot model, and launch sequences.

### Multi-Modal Interaction Packages (Experimental)

- [Follow-Me Interfaces (`follow_me_interfaces`)](../src/follow_me_interfaces/README.md) - Custom ROS 2 message definitions (`AudioCommand`, `GestureCategory`).
- [Gesture Recognition (`gesture_recognition_pkg`)](../src/gesture_recognition_pkg/README.md) - MediaPipe hand gesture recognition from camera streams.
- [Speech Recognition (`speech_recognition_pkg`)](../src/speech_recognition_pkg/README.md) - OpenVINO QuartzNet speech command recognition.
- [Text-to-Speech (`text_to_speech_pkg`)](../src/text_to_speech_pkg/README.md) - OpenVINO Forward Tacotron and MelGAN neural voice feedback.

### Sensor & Platform Tutorials

- [ADBSCAN AAEON Robot Tutorial](adbscan_aaeon_robot.md) - Running ADBSCAN on a physical AAEON AMR platform.
- [ADBSCAN RealSense Demo](adbscan-realsense.md) - Running ADBSCAN from Intel® RealSense™ depth camera bag data.
- [ADBSCAN RPLidar Demo](adbscan-rplidar.md) - Running ADBSCAN from 2D RPLidar bag data.

## Troubleshooting

- Failed to install Deb package: Please make sure to run `sudo apt update` before installing Debian packages.
- You can stop running nodes or simulations anytime by pressing `Ctrl-C`.
