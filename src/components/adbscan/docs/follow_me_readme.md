<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Follow-Me Application

The `adbscan_sub` follower tracks a nearby target from a 2D LiDAR scan or
RealSense depth cloud and publishes `geometry_msgs/msg/Twist` commands. It
starts tracking on its own; gesture and speech recognition are not required.

## Gazebo

Build and source the workspace with ROS 2 Humble or Jazzy and the Gazebo
dependencies installed:

```bash
source /opt/ros/<distro>/setup.bash
make build
source install/setup.bash
ros2 launch followme_turtlebot3_gazebo follow_me.launch.py
```

The launch starts Gazebo, both robots, the ROS/Gazebo bridges, and the follower.
The guide robot changes speed and direction at random within a bounded area;
there is no recorded bag or predetermined path. Use `sensor:=realsense` for
the simulated depth camera instead of the default 2D LiDAR.

To drive the guide yourself, launch with `guide_motion:=manual` and publish
commands in another sourced terminal, for example:

```bash
ros2 topic pub -r 5 /guide_robot/cmd_vel geometry_msgs/msg/Twist '{linear: {x: 0.12}, angular: {z: 0.2}}'
```

Stop the publisher with Ctrl-C. The guide will not receive a canned trajectory.

## Controls

The follower starts enabled. Set `start_enabled:=false` at launch to require
an external activation command. A `std_msgs/msg/Bool` on `/follow_enabled`
changes the active state; disabling publishes a zero velocity command.
Adapters for future gesture and speech recognition can use this same topic.
Speech-specific commands beyond activation are not implemented yet.

```bash
ros2 topic pub --once /follow_enabled std_msgs/msg/Bool '{data: false}'
ros2 topic pub --once /follow_enabled std_msgs/msg/Bool '{data: true}'
```

The `backend` launch argument selects the clustering backend at startup:
`legacy_cpu` (default), `pcl_kdtree`, `oneapi_kdtree`, or `oneapi_octree`.
For oneAPI, set `device:=cpu` or `device:=gpu` as appropriate for the installed
runtime and hardware. Run `ros2 launch followme_turtlebot3_gazebo
follow_me.launch.py --show-args` for launch options.

The 2D and RealSense follower profiles live in
`src/adbscan_ros2_follow_me/config/adbscan_sub_2D.yaml` and
`src/adbscan_ros2_follow_me/config/adbscan_sub_RS.yaml`. The launch applies
simulation-only RealSense frame and distance overrides automatically.

## Robot Deployment

Run the same follower executable with the profile for your sensor. Connect
its `cmd_vel` output to the robot base controller, and verify the robot's
emergency stop and velocity limits before enabling physical motion.

```bash
ros2 run adbscan_ros2_follow_me adbscan_sub --ros-args \
  --params-file src/adbscan_ros2_follow_me/config/adbscan_sub_2D.yaml
```

For a RealSense sensor, use `adbscan_sub_RS.yaml` instead. The follower
subscribes to the sensor topic configured in that file. Use the
`start_enabled` and `core_backend` ROS parameters to select activation and
clustering at startup. No separate gesture or audio follower binary is needed.
