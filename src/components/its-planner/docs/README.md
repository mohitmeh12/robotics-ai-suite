<!--
Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ITS Path Planner ROS 2 Navigation Plugin

---

Intelligent Sampling and Two-Way Search (ITS) global path planner is an
Intel® patented algorithm.

The ITS Plugin for the ROS 2 Navigation 2 application plugin is a global
path planner module that is based on Intelligent sampling and Two-way
Search (ITS).

ITS is a search approach based on two-way path planning and intelligent
sampling. It reduces compute time by about 20x-30x on a 1,000-node map,
compared with the A\* search algorithm. The inputs are
the 2D occupancy grid map, the robot position, and the goal position.
It does not support continuous replanning.

**Prerequisites:** Use a simple behavior tree with a compute path to pose
and a follow path.

*ITS planner inputs:*

- global 2D costmap (`nav2_costmap_2d::Costmap2D`)
- start and goal pose (`geometry_msgs::msg::PoseStamped`)

*ITS planner outputs:*

- 2D waypoints of the path

**Path planning steps summary:**

1. The ITS planner converts the 2D costmap to either a Probabilistic
    Road Map (PRM) or a Deterministic Road Map (DRM).
2. The generated roadmap is saved as a text file that can be reused for
    multiple inquiries.
3. The ITS planner conducts a two-way search to find a path from the
    source to the destination. Either a smoothing filter or Catmull spline
    interpolation can be used to create a smooth, continuous
    path. The generated smooth path is in the form of a ROS 2 navigation
    message type (`nav_msgs::msg`).

For customization options, see [ITS Path Planner Plugin Customization](https://developer.robotics.intel.com/development-stack/components/navigation/its-path-planner-plugin/#its-path-planner-plugin-customization)

## Source Code

The source code of this component can be found here:
[ITS-Planner](https://github.com/open-edge-platform/edge-ai-suites/tree/main/robotics-ai-suite/components/its-planner)

## Getting Started

Robotics AI Dev Kit provides a ROS 2 Debian package for the application,
supported by the following platforms:

- ROS 2 version: humble or jazzy

## Prerequisites

- [Prepare the target system](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/)
- [Setup the Robotics AI Dev Kit APT Repositories](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#3-set-up-robotics-ai-suite-oneapi-and-graphics-apt-repositories)
- [Install OpenVINO™ Packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#4-install-openvino-packages)
- [Install Robotics AI Dev Kit Deb packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#6-install-robotics-ai-suite-deb-packages)
- [Install the Intel® NPU Driver on Intel® Core™ Ultra Processors (if applicable)](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#7-install-the-intel-npu-driver-on-intel-core-ultra-processors)

## Install Debian package

Install the `ros-${ROS_DISTRO}-its-planner` Debian package from the Intel®
Robotics AI Dev Kit APT repository:

> ``` bash
> sudo apt install ros-${ROS_DISTRO}-its-planner
> ```

Run the following script to set environment variables:

> ``` bash
> source /opt/ros/$ROS_DISTRO/setup.bash        # ROS_DISTRO=humble or jazzy
> export TURTLEBOT3_MODEL=waffle
>
> # Set Gazebo model path (variable name differs between distributions)
> if [ "$ROS_DISTRO" = "jazzy" ]; then
>     export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:/opt/ros/$ROS_DISTRO/share/turtlebot3_gazebo/models
> else
>     export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/$ROS_DISTRO/share/turtlebot3_gazebo/models
> fi
> ```

To launch the default ITS planner which is based on differential drive
robot, run:

> ``` bash
> ros2 launch nav2_bringup tb3_simulation_launch.py \
>   headless:=False \
>   params_file:=/opt/ros/$ROS_DISTRO/share/its_planner/nav2_params_${ROS_DISTRO}.yaml \
>   default_bt_xml_filename:=/opt/ros/$ROS_DISTRO/share/its_planner/navigate_w_recovery_${ROS_DISTRO}.xml
> ```

ITS Planner also supports Ackermann steering; to launch the Ackermann
ITS planner run:

> ``` bash
> ros2 launch nav2_bringup tb3_simulation_launch.py \
>   headless:=False \
>   params_file:=/opt/ros/$ROS_DISTRO/share/its_planner/nav2_params_dubins_${ROS_DISTRO}.yaml \
>   default_bt_xml_filename:=/opt/ros/$ROS_DISTRO/share/its_planner/navigate_w_recovery_${ROS_DISTRO}.xml
> ```

[!NOTE]
> The command above opens the Gazebo\* and RViz applications. Gazebo\*
> can take up to a minute to open, depending on the host's
> capabilities. Both applications contain the simulated waffle map, and
> a simulated robot. Initially, the applications are opened in the
> background, but you can bring them into the foreground, side-by-side,
> for a better visual.

a.  Set the robot **2D Pose Estimate** in rviz2:
    1.  Set the initial robot pose by pressing **2D Pose Estimate** in
        rviz2.
    2.  Click the 2D map at the robot's estimated location.
        For reference, use the robot pose as it appears in Gazebo\*.
    3.  Set the orientation by dragging forward from the down-click.
        This also enables ROS 2 navigation.
    ![image](images/2d_pose_estimate.png)

b.  In rviz2, press **Navigation2 Goal**, and choose a destination for
    the robot. This calls the behavioral tree navigator to go to that
    goal through an action server.
    ![image](images/set_navigation_goal.png)
    ![image](images/path_created.png)
    **Expected result:** The robot moves along the path generated to its
    new destination.

c.  Set new destinations for the robot, one at a time.
    ![image](images/goal_achived_gazebo_rviz.png)

d.  To close this, do the following:
    - Press `Ctrl-C` in the terminal where you launched the command.
