#!/usr/bin/env python3
# Copyright (C) 2026 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

"""Installed-package E2E coverage for the ITS global planner Nav2 plugin.

The test drives a headless ``planner_server`` configured with the
``its_planner::ITSPlanner`` plugin against a synthetic free-space costmap and
asserts that a ``ComputePathToPose`` request returns a valid path. It therefore
exercises the installed ``its_planner`` package exactly as Nav2 would load it.
"""

import os
import shutil
import tempfile
import time
import unittest

from geometry_msgs.msg import PoseStamped
from launch import LaunchDescription
from launch_ros.actions import Node
import launch_testing
import launch_testing.actions
import launch_testing.util
from nav2_msgs.action import ComputePathToPose
import rclpy
from rclpy.action import ActionClient


MAP_CELLS = 200
MAP_RESOLUTION = 0.05
MAP_ORIGIN = -5.0
PLANNER_ID = "GridBased"
STARTUP_TIMEOUT_SECONDS = 120.0
PLAN_TIMEOUT_SECONDS = 30.0
PLAN_ATTEMPTS = 3

_temp_dirs = []


def _write_free_map(directory):
    """Write a fully free-space PGM map and its Nav2 metadata YAML."""
    image_path = os.path.join(directory, "e2e_map.pgm")
    with open(image_path, "wb") as image_file:
        image_file.write(f"P5\n{MAP_CELLS} {MAP_CELLS}\n255\n".encode("ascii"))
        image_file.write(bytes([254]) * (MAP_CELLS * MAP_CELLS))
    yaml_path = os.path.join(directory, "e2e_map.yaml")
    with open(yaml_path, "w", encoding="utf-8") as yaml_file:
        yaml_file.write(
            "image: e2e_map.pgm\n"
            "mode: trinary\n"
            f"resolution: {MAP_RESOLUTION}\n"
            f"origin: [{MAP_ORIGIN}, {MAP_ORIGIN}, 0.0]\n"
            "negate: 0\n"
            "occupied_thresh: 0.65\n"
            "free_thresh: 0.25\n"
        )
    return yaml_path


def _write_params(directory, map_yaml):
    """Write a minimal Nav2 parameter file loading the ITS planner plugin."""
    params_path = os.path.join(directory, "e2e_params.yaml")
    with open(params_path, "w", encoding="utf-8") as params_file:
        params_file.write(
            "map_server:\n"
            "  ros__parameters:\n"
            "    use_sim_time: false\n"
            f'    yaml_filename: "{map_yaml}"\n'
            "\n"
            "planner_server:\n"
            "  ros__parameters:\n"
            "    use_sim_time: false\n"
            "    expected_planner_frequency: 20.0\n"
            '    planner_plugins: ["GridBased"]\n'
            "    GridBased:\n"
            '      plugin: "its_planner::ITSPlanner"\n'
            "      interpolation_resolution: 0.05\n"
            "      catmull_spline: false\n"
            "      smoothing_window: 15\n"
            "      buffer_size: 10\n"
            "      build_road_map_once: true\n"
            "      enable_k: false\n"
            "      min_samples: 250\n"
            '      roadmap: "PROBABILISTIC"\n'
            "      w: 32\n"
            "      h: 32\n"
            "      n: 2\n"
            "\n"
            "global_costmap:\n"
            "  global_costmap:\n"
            "    ros__parameters:\n"
            "      use_sim_time: false\n"
            "      update_frequency: 1.0\n"
            "      publish_frequency: 1.0\n"
            "      global_frame: map\n"
            "      robot_base_frame: base_link\n"
            "      robot_radius: 0.22\n"
            "      resolution: 0.05\n"
            "      track_unknown_space: false\n"
            '      plugins: ["static_layer", "inflation_layer"]\n'
            "      static_layer:\n"
            '        plugin: "nav2_costmap_2d::StaticLayer"\n'
            "        map_subscribe_transient_local: true\n"
            "      inflation_layer:\n"
            '        plugin: "nav2_costmap_2d::InflationLayer"\n'
            "        cost_scaling_factor: 3.0\n"
            "        inflation_radius: 0.7\n"
            "      always_send_full_costmap: true\n"
            "\n"
            "lifecycle_manager:\n"
            "  ros__parameters:\n"
            "    use_sim_time: false\n"
            "    autostart: true\n"
            '    node_names: ["map_server", "planner_server"]\n'
        )
    return params_path


def generate_test_description():
    """Launch map_server, the ITS planner_server and lifecycle management."""
    temp_dir = tempfile.mkdtemp(prefix="its_planner_e2e_")
    _temp_dirs.append(temp_dir)
    map_yaml = _write_free_map(temp_dir)
    params = _write_params(temp_dir, map_yaml)
    return LaunchDescription(
        [
            Node(
                package="nav2_map_server",
                executable="map_server",
                name="map_server",
                output="screen",
                parameters=[params],
            ),
            Node(
                package="nav2_planner",
                executable="planner_server",
                name="planner_server",
                output="screen",
                parameters=[params],
            ),
            Node(
                package="nav2_lifecycle_manager",
                executable="lifecycle_manager",
                name="lifecycle_manager",
                output="screen",
                parameters=[params],
            ),
            Node(
                package="tf2_ros",
                executable="static_transform_publisher",
                name="map_to_base_link",
                output="screen",
                arguments=["0", "0", "0", "0", "0", "0", "map", "base_link"],
            ),
            launch_testing.util.KeepAliveProc(),
            launch_testing.actions.ReadyToTest(),
        ]
    )


class TestItsPlannerE2E(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        for temp_dir in _temp_dirs:
            if os.path.isdir(temp_dir):
                shutil.rmtree(temp_dir, ignore_errors=True)
        _temp_dirs.clear()

    def setUp(self):
        rclpy.init()
        self.node = rclpy.create_node("its_planner_e2e_test")
        self.client = ActionClient(self.node, ComputePathToPose, "compute_path_to_pose")

    def tearDown(self):
        self.client.destroy()
        self.node.destroy_node()
        rclpy.shutdown()

    def _make_pose(self, x_position, y_position):
        pose = PoseStamped()
        pose.header.frame_id = "map"
        pose.pose.position.x = x_position
        pose.pose.position.y = y_position
        pose.pose.orientation.w = 1.0
        return pose

    def _request_path(self):
        goal = ComputePathToPose.Goal()
        goal.start = self._make_pose(-1.0, -1.0)
        goal.goal = self._make_pose(1.0, 1.0)
        goal.planner_id = PLANNER_ID
        goal.use_start = True

        send_future = self.client.send_goal_async(goal)
        rclpy.spin_until_future_complete(self.node, send_future, timeout_sec=PLAN_TIMEOUT_SECONDS)
        goal_handle = send_future.result()
        if goal_handle is None or not goal_handle.accepted:
            return None

        result_future = goal_handle.get_result_async()
        rclpy.spin_until_future_complete(self.node, result_future,
                                         timeout_sec=PLAN_TIMEOUT_SECONDS)
        wrapped_result = result_future.result()
        if wrapped_result is None:
            return None
        return wrapped_result.result.path

    def test_its_planner_plans_path(self):
        """The installed ITS planner must load and return a valid global path."""
        server_ready = self.client.wait_for_server(timeout_sec=STARTUP_TIMEOUT_SECONDS)
        self.assertTrue(
            server_ready,
            "planner_server ComputePathToPose action was not available; "
            "the its_planner::ITSPlanner plugin failed to load",
        )
        for _ in range(PLAN_ATTEMPTS):
            path = self._request_path()
            if path is not None and len(path.poses) > 0:
                return
            time.sleep(2.0)
        self.fail("its_planner::ITSPlanner did not return a valid path")
