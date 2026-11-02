<!--
Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Robot Re-localization package for ROS2 Navigation

The robot relocalization package enables relocalization in ROS 2 Navigation.
Sensor measurement glitches, environmental disturbances, or scene similarities can cause a robot to lose its
localization. A fast and reliable algorithm is therefore needed to relocalize the robot.
To address this problem, we developed a compute- and memory-efficient relocalization algorithm for mobile robots.

## Getting Started

Robotics AI Suite provides a ROS2 Debian package for the application, supported by the following platforms:

- OS: Ubuntu 22.04 (Humble) or Ubuntu 24.04 (Jazzy)

- ROS version: humble or jazzy

## Install Debian Package

Install the ``ros-${ROS_DISTRO}-its-relocalization-bringup`` Debian package from the Intel Robotics AI Suite APT repo:

```sh
sudo apt install ros-${ROS_DISTRO}-its-relocalization-bringup
```

Run the following script to set environment variables and launch ROS 2 Navigation and TurtleBot3 in Gazebo:

```sh
source /opt/ros/$ROS_DISTRO/setup.bash        # ROS_DISTRO=humble or jazzy
export TURTLEBOT3_MODEL=waffle

# Set Gazebo model path (variable name differs between distributions)
if [ "$ROS_DISTRO" = "jazzy" ]; then
    export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:/opt/ros/$ROS_DISTRO/share/turtlebot3_gazebo/models
else
    export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/$ROS_DISTRO/share/turtlebot3_gazebo/models
fi

ros2 launch nav2_bringup tb3_simulation_launch.py headless:=False
```

Once the ROS2 navigation is running in Gazebo, open a new terminal to bring up the re-localization:

```sh
source /opt/ros/$ROS_DISTRO/setup.bash
ros2 launch relocalization_bringup relocalization.launch.xml 
```

To demonstrate the relocalization package, we created a demo application that simulates a sensor failure.
In this application, the sensor is disabled for a few seconds while the robot travels toward its goal.
Once sensor measurements are restored, the robot automatically relocalizes itself and continues toward the goal.
To run the demo, use:

```sh
ros2 launch relocalization_bringup relocalization_demo.launch.xml mode:=demo
```
