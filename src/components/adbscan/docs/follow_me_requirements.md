<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Follow-Me Requirements

Install ROS 2 Jazzy (Gazebo Harmonic) or Humble (Gazebo Fortress) with
`ros_gz_sim`, `ros_gz_bridge`, `turtlebot3_gazebo`, `rclpy`, and the ROS message
dependencies declared in the package manifests. The full simulation cannot
start without `turtlebot3_gazebo`; verify availability with:

```bash
ros2 pkg prefix turtlebot3_gazebo
```

For a source workspace, install the ROS dependencies and build the packages:

```bash
source /opt/ros/<distro>/setup.bash
rosdep install --from-paths src --ignore-src -r -y
make build
source install/setup.bash
```

No gesture or audio Python environment is required for live following.
Those interaction adapters are deferred; see the [application guide](follow_me_readme.md)
for the current activation hook and launch options.
