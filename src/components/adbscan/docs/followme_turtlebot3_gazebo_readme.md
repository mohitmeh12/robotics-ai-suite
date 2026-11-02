<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Follow-Me TurtleBot3 Gazebo Simulation (`followme_turtlebot3_gazebo`)

This package provides the Gazebo simulation environments, robot models, and launch configurations for testing the ADBSCAN-based person-following application on a TurtleBot3 Waffle robot in simulation.

## Overview

The simulation environment spawns two robots:

1. **Guide Robot (`guide_robot`)**: An airport-style lead vehicle featuring a black-and-yellow checkerboard visual pattern. It wanders within a bounded area with randomized motion by default, or accepts manual velocity commands.
2. **Follower Robot (`turtlebot3_waffle_depth`)**: A simulated TurtleBot3 Waffle equipped with a 2D LiDAR and an Intel® RealSense™ depth camera sensor plugin. It runs the `adbscan_ros2_follow_me` node to detect the guide robot and autonomously follow it.

## Supported Distributions and Simulators

- **ROS 2 Jazzy**: Uses **Gazebo Harmonic** (`ros_gz_sim`).
- **ROS 2 Humble**: Uses **Gazebo Fortress** / Gazebo Classic.

## Simulation Models

The simulation assets are located in `src/followme_turtlebot3_gazebo/models/`:

- `models/guide_robot`: SDF model and textures for the lead guide vehicle.
- `models/turtlebot3_waffle_depth`: SDF and URDF descriptions for the follower robot with simulated depth sensor plugins.

## Launch

With the ROS 2 workspace sourced, start the complete simulation:

```bash
ros2 launch followme_turtlebot3_gazebo follow_me.launch.py
```

The launch includes the matching Gazebo world and bridges, then starts the
`adbscan_sub` follower and guide controller after robot spawning. The follower
owns tracking and motion commands. The guide controller uses odometry to stay
near the start position, changing speed and heading at random intervals.
For depth sensing, pass `sensor:=realsense`. To control the guide externally,
pass `guide_motion:=manual` and publish a `geometry_msgs/msg/Twist` to
`/guide_robot/cmd_vel`. The follower's activation hook is `/follow_enabled`
(`std_msgs/msg/Bool`); no gesture or speech process is required.

See the [Follow-Me Application Guide](follow_me_readme.md) for commands and
startup backend selection.
