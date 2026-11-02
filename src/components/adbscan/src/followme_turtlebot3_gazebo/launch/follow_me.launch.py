#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    OpaqueFunction,
    TimerAction,
)
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def launch_follower(context):
    sensor = LaunchConfiguration("sensor").perform(context)
    device = LaunchConfiguration("device").perform(context)
    launch_dir = os.path.join(get_package_share_directory("followme_turtlebot3_gazebo"), "launch")
    config_dir = os.path.join(get_package_share_directory("adbscan_ros2_follow_me"), "config")
    is_depth = sensor == "realsense"
    shared = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                launch_dir, "rs_shared.launch.py" if is_depth else "lidar_shared.launch.py"
            )
        )
    )
    follower = Node(
        package="adbscan_ros2_follow_me",
        executable="adbscan_sub",
        parameters=[
            os.path.join(config_dir, "adbscan_sub_RS.yaml" if is_depth else "adbscan_sub_2D.yaml"),
            {
                "use_sim_time": True,
                "start_enabled": ParameterValue(
                    LaunchConfiguration("start_enabled"), value_type=bool
                ),
                "core_backend": ParameterValue(LaunchConfiguration("backend"), value_type=str),
                **(
                    {"optical_frame": False, "min_dist": 0.3, "init_tgt_loc": 0.5}
                    if is_depth
                    else {}
                ),
            },
        ],
        remappings=[("cmd_vel", "tb3/cmd_vel")],
        additional_env={"ONEAPI_DEVICE_SELECTOR": device} if device else {},
        output="screen",
    )
    guide = Node(
        package="followme_turtlebot3_gazebo",
        executable="guide_wander.py",
        parameters=[{"use_sim_time": True}],
        condition=IfCondition(
            PythonExpression(["'", LaunchConfiguration("guide_motion"), "' == 'wander'"])
        ),
        output="screen",
    )
    return [shared, TimerAction(period=15.0, actions=[follower, guide])]


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument("sensor", default_value="lidar", choices=["lidar", "realsense"]),
            DeclareLaunchArgument(
                "start_enabled", default_value="true", choices=["true", "false"]
            ),
            DeclareLaunchArgument(
                "backend",
                default_value="legacy_cpu",
                choices=["legacy_cpu", "pcl_kdtree", "oneapi_kdtree", "oneapi_octree"],
            ),
            DeclareLaunchArgument("device", default_value=""),
            DeclareLaunchArgument(
                "guide_motion", default_value="wander", choices=["wander", "manual"]
            ),
            OpaqueFunction(function=launch_follower),
        ]
    )
