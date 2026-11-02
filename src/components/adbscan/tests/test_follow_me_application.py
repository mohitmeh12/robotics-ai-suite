# SPDX-License-Identifier: Apache-2.0

import os
import time
import unittest

from ament_index_python.packages import get_package_share_directory
from geometry_msgs.msg import Twist
import launch
import launch_ros.actions
import launch_testing.actions
import pytest
import rclpy
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import LaserScan
from std_msgs.msg import Bool


@pytest.mark.launch_test
def generate_test_description():
    config = get_package_share_directory("adbscan_ros2_follow_me") + "/config/adbscan_sub_2D.yaml"
    backend = os.environ.get("ADBSCAN_TEST_BACKEND")
    follower = launch_ros.actions.Node(
        package="adbscan_ros2_follow_me",
        executable="adbscan_sub",
        parameters=[
            config,
            {"Verbose": False},
            *([{"core_backend": backend}] if backend else []),
        ],
        remappings=[
            ("scan", "/follower_test/scan"),
            ("cmd_vel", "/follower_test/cmd_vel"),
            ("follow_enabled", "/follower_test/follow_enabled"),
        ],
        output="screen",
    )
    return launch.LaunchDescription([follower, launch_testing.actions.ReadyToTest()])


class TestFollowerActivation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.node = rclpy.create_node("follower_activation_test")

    def tearDown(self):
        self.node.destroy_node()

    def test_default_tracking_and_activation(self):
        scan_publisher = self.node.create_publisher(
            LaserScan, "/follower_test/scan", qos_profile_sensor_data
        )
        enabled_publisher = self.node.create_publisher(Bool, "/follower_test/follow_enabled", 10)
        velocities = []
        subscription = self.node.create_subscription(
            Twist, "/follower_test/cmd_vel", velocities.append, 10
        )
        scan = LaserScan()
        scan.range_min = 0.01
        scan.range_max = 10.0
        scan.angle_increment = 0.00001
        scan.ranges = [1.0, 1.04, 1.08, 5.0, 5.04, 5.08]

        def await_velocity(predicate):
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline:
                scan.header.stamp = self.node.get_clock().now().to_msg()
                scan_publisher.publish(scan)
                rclpy.spin_once(self.node, timeout_sec=0.1)
                if any(predicate(velocity) for velocity in velocities):
                    velocities.clear()
                    return
            self.fail("follower did not publish the expected velocity")

        try:
            await_velocity(lambda velocity: velocity.linear.x > 0)
            enabled_publisher.publish(Bool(data=False))
            await_velocity(lambda velocity: velocity.linear.x == 0 and velocity.angular.z == 0)
            enabled_publisher.publish(Bool(data=True))
            await_velocity(lambda velocity: velocity.linear.x > 0)
        finally:
            self.node.destroy_subscription(subscription)
            self.node.destroy_publisher(enabled_publisher)
            self.node.destroy_publisher(scan_publisher)
