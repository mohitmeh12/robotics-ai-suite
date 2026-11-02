# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

import math
import os
import struct
import subprocess
import tempfile
import time
import unittest
from ament_index_python.packages import get_package_prefix
import launch
import launch_ros.actions
import launch_testing
import launch_testing.actions
from nav2_dynamic_msgs.msg import ObstacleArray
import pytest
import rclpy
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import LaserScan, PointCloud2, PointField
from sensor_msgs_py import point_cloud2
from std_msgs.msg import Header


@pytest.mark.launch_test
def generate_test_description():
    adbscan_pub_node = launch_ros.actions.Node(
        package="adbscan_ros2",
        executable="adbscan_pub",
        name="adbscan_pub_test_node",
        output="screen",
    )

    return launch.LaunchDescription(
        [
            adbscan_pub_node,
            launch_testing.actions.ReadyToTest(),
        ]
    )


class TestAdbscanTopicPublication(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.node = rclpy.create_node("test_adbscan_subscriber")

    def tearDown(self):
        self.node.destroy_node()

    def test_nodes_and_topics(self):
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            topics = dict(self.node.get_topic_names_and_types())
            if "/Obstacle_Array" in topics:
                break
            time.sleep(0.1)

        self.assertIn("/Obstacle_Array", topics)
        self.assertIn("nav2_dynamic_msgs/msg/ObstacleArray", topics["/Obstacle_Array"])

    def assert_subscriber_parity(
        self, message, lidar_type, parameters, expected_x, expected_boxes=None
    ):
        topic = f"/adbscan_core_test_{lidar_type.lower()}"
        publisher = self.node.create_publisher(type(message), topic, qos_profile_sensor_data)
        executable = os.path.join(
            get_package_prefix("adbscan_ros2"), "lib", "adbscan_ros2", "adbscan_sub"
        )
        backend = os.environ.get("ADBSCAN_TEST_BACKEND")
        observed = {}

        try:
            for enabled in ("false", "true"):
                received = []
                subscription = self.node.create_subscription(
                    ObstacleArray,
                    f"/adbscan_{lidar_type.lower()}_{enabled}/obstacle_array",
                    received.append,
                    10,
                )
                process = subprocess.Popen(
                    [
                        executable,
                        "--ros-args",
                        "-r",
                        f"__ns:=/adbscan_{lidar_type.lower()}_{enabled}",
                        "-p",
                        f"Lidar_type:={lidar_type}",
                        "-p",
                        f"Lidar_topic:={topic}",
                        "-p",
                        "Verbose:=false",
                        "-p",
                        f"use_adbscan_core:={enabled}",
                        *(["-p", f"oneapi_library:={backend}"] if backend else []),
                        *(
                            ["-p", f"core_backend:={backend}"]
                            if backend and enabled == "true"
                            else []
                        ),
                        *[
                            argument
                            for name, value in parameters.items()
                            for argument in ("-p", f"{name}:={value}")
                        ],
                    ],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                )
                try:
                    deadline = time.monotonic() + 8
                    while not received and time.monotonic() < deadline:
                        if process.poll() is not None:
                            self.fail(
                                f"subscriber exited: {enabled}: "
                                f"{process.stderr.read().decode(errors='replace')}"
                            )
                        message.header.stamp = self.node.get_clock().now().to_msg()
                        publisher.publish(message)
                        rclpy.spin_once(self.node, timeout_sec=0.1)
                    self.assertTrue(received, f"no obstacle message: {enabled}")
                    obstacles = received[-1].obstacles
                    self.assertEqual(len(obstacles), len(expected_x), enabled)
                    for item, center_x in zip(obstacles, expected_x):
                        self.assertTrue(math.isclose(item.position.x, center_x, abs_tol=1e-4))
                    observed[enabled] = [
                        (
                            item.position.x,
                            item.position.y,
                            item.position.z,
                            item.size.x,
                            item.size.y,
                            item.size.z,
                        )
                        for item in obstacles
                    ]
                finally:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                    process.stderr.close()
                    self.node.destroy_subscription(subscription)
            for legacy, core in zip(observed["false"], observed["true"]):
                for expected, actual in zip(legacy, core):
                    self.assertTrue(math.isclose(expected, actual, abs_tol=1e-4))
            if expected_boxes is not None:
                for actual, expected in zip(observed["true"], expected_boxes):
                    for value, target in zip(actual, expected):
                        self.assertTrue(math.isclose(value, target, abs_tol=1e-4))
        finally:
            self.node.destroy_publisher(publisher)

    def test_subscriber_opt_in_preserves_planar_obstacles(self):
        scan = LaserScan()
        scan.header.frame_id = "laser"
        scan.angle_min = 0.0
        scan.angle_increment = 0.00001
        scan.angle_max = 0.00005
        scan.range_min = 0.01
        scan.range_max = 10.0
        scan.ranges = [1.0, 1.04, 1.08, 5.0, 5.04, 5.08]
        self.assert_subscriber_parity(scan, "2D", {"base": 2.0, "scale_factor": 1.0}, [1.04, 5.04])

    def test_core_defaults_to_cpu_with_legacy_oneapi_parameter(self):
        scan = LaserScan()
        scan.header.frame_id = "laser"
        scan.angle_min = 0.0
        scan.angle_increment = 0.00001
        scan.angle_max = 0.00005
        scan.range_min = 0.01
        scan.range_max = 10.0
        scan.ranges = [1.0, 1.04, 1.08, 5.0, 5.04, 5.08]
        topic = "/adbscan_core_default_input"
        publisher = self.node.create_publisher(LaserScan, topic, qos_profile_sensor_data)
        received = []
        subscription = self.node.create_subscription(
            ObstacleArray, "/adbscan_core_default/obstacle_array", received.append, 10
        )
        executable = os.path.join(
            get_package_prefix("adbscan_ros2"), "lib", "adbscan_ros2", "adbscan_sub"
        )
        try:
            with tempfile.TemporaryFile() as output:
                process = subprocess.Popen(
                    [
                        executable,
                        "--ros-args",
                        "-r",
                        "__ns:=/adbscan_core_default",
                        "-p",
                        "Lidar_type:=2D",
                        "-p",
                        f"Lidar_topic:={topic}",
                        "-p",
                        "Verbose:=false",
                        "-p",
                        "oneapi_library:=oneapi_kdtree",
                        "-p",
                        "benchmark_number_of_frames:=1",
                    ],
                    stdout=output,
                    stderr=output,
                )
                try:
                    deadline = time.monotonic() + 8
                    while len(received) < 3 and time.monotonic() < deadline:
                        if process.poll() is not None:
                            output.seek(0)
                            self.fail(output.read().decode(errors="replace"))
                        scan.header.stamp = self.node.get_clock().now().to_msg()
                        publisher.publish(scan)
                        rclpy.spin_once(self.node, timeout_sec=0.1)
                    self.assertGreaterEqual(len(received), 3)
                finally:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                output.seek(0)
                self.assertIn(b"Core CPU ADBSCAN", output.read())
        finally:
            self.node.destroy_subscription(subscription)
            self.node.destroy_publisher(publisher)

    def test_subscriber_opt_in_preserves_lidar3d_obstacles(self):
        cloud = point_cloud2.create_cloud_xyz32(
            Header(frame_id="lidar"),
            [
                (0.0, 0.0, 1.0),
                (50.0, 0.0, 1.0),
                (0.1, 0.0, -1.0),
                (50.0, 0.0, 1.0),
                (0.2, 0.0, 1.0),
                (50.0, 0.0, 1.0),
                (5.0, 0.0, 1.0),
                (50.0, 0.0, 1.0),
                (5.1, 0.0, 1.0),
            ],
        )
        self.assert_subscriber_parity(
            cloud,
            "3D",
            {
                "subsample_ratio": 2.0,
                "Z_based_ground_removal": 1.0,
                "z_filter": 0.0,
                "x_filter_back": -1.0,
                "base": 1.0,
                "min_3d_epsilon": 0.25,
            },
            [0.1, 5.05],
            [(0.1, 0.0, 1.0, 0.2, 0.0, 0.0), (5.05, 0.0, 1.0, 0.1, 0.0, 0.0)],
        )

    def test_subscriber_opt_in_preserves_realsense_obstacles(self):
        body_points = [
            (1.0, 0.0, 0.5),
            (1.1, 0.1, 0.6),
            (1.2, 0.2, 0.7),
            (2.0, 1.0, 0.5),
            (2.1, 1.1, 0.6),
            (2.2, 1.2, 0.7),
            (4.0, 0.0, 0.5),
            (1.0, 3.0, 0.5),
            (1.0, -3.0, 0.5),
            (1.0, 0.0, -0.5),
        ]
        cloud = point_cloud2.create_cloud_xyz32(
            Header(frame_id="camera_depth_optical_frame"),
            [(-y, -z, x) for x, y, z in body_points],
        )
        self.assert_subscriber_parity(
            cloud,
            "RS",
            {
                "subsample_ratio": 1.0,
                "Z_based_ground_removal": 1.0,
                "z_filter": 0.0,
                "x_filter_back": 3.0,
                "y_filter_left": 2.0,
                "y_filter_right": -2.0,
                "base": 2.0,
                "scale_factor": 1.0,
            },
            [1.1, 2.1],
            [(1.1, 0.1, 0.6, 0.2, 0.2, 0.2), (2.1, 1.1, 0.6, 0.2, 0.2, 0.2)],
        )

    @unittest.skipUnless(os.environ.get("ADBSCAN_STREAM_TEST") == "1", "opt-in 30 Hz stream")
    def test_depth_stream_at_30hz(self):
        width, height, subsample = 640, 480, 150
        frame_count, period = int(os.environ.get("ADBSCAN_STREAM_FRAMES", "60")), 1 / 30
        self.assertGreater(frame_count, 0)
        self.assertLessEqual(frame_count, 90)
        cloud_data = bytearray(struct.pack("<fff", 0, 1, 50) * (width * height))
        for index in range(0, width * height, subsample):
            group, member = divmod(index // subsample, 128)
            body_x = 1.0 + 0.6 * (group % 4) + 0.01 * (member % 8)
            body_y = -0.9 + 0.6 * (group // 4) + 0.01 * ((member // 8) % 8)
            body_z = 0.5 + 0.02 * (member // 64)
            struct.pack_into("<fff", cloud_data, index * 12, -body_y, -body_z, body_x)
        cloud = PointCloud2(
            header=Header(frame_id="camera_depth_optical_frame"),
            height=height,
            width=width,
            fields=[
                PointField(name=name, offset=offset, datatype=PointField.FLOAT32, count=1)
                for name, offset in (("x", 0), ("y", 4), ("z", 8))
            ],
            is_bigendian=False,
            point_step=12,
            row_step=width * 12,
            data=bytes(cloud_data),
            is_dense=True,
        )
        publisher = self.node.create_publisher(
            PointCloud2, "/adbscan_stream_input", qos_profile_sensor_data
        )
        executable = os.path.join(
            get_package_prefix("adbscan_ros2"), "lib", "adbscan_ros2", "adbscan_sub"
        )
        observed = {}

        try:
            modes = os.environ.get("ADBSCAN_STREAM_MODES", "true").split(",")
            backend = os.environ.get("ADBSCAN_TEST_BACKEND", "oneapi_kdtree")
            self.assertTrue(set(modes) <= {"false", "true"})
            for enabled in modes:
                sent_at = {}
                received = {}
                unmatched_stamps = []
                late = 0

                def on_obstacles(message):
                    stamp = (message.header.stamp.sec, message.header.stamp.nanosec)
                    if stamp in sent_at and stamp not in received:
                        boxes = sorted(
                            (
                                item.position.x,
                                item.position.y,
                                item.position.z,
                                item.size.x,
                                item.size.y,
                                item.size.z,
                            )
                            for item in message.obstacles
                        )
                        received[stamp] = (time.monotonic() - sent_at[stamp], boxes)
                    elif stamp not in sent_at:
                        unmatched_stamps.append(stamp)

                subscription = self.node.create_subscription(
                    ObstacleArray, f"/adbscan_stream_{enabled}/obstacle_array", on_obstacles, 10
                )
                with tempfile.TemporaryFile() as stderr:
                    process = subprocess.Popen(
                        [
                            executable,
                            "--ros-args",
                            "-r",
                            f"__ns:=/adbscan_stream_{enabled}",
                            "-p",
                            "Lidar_type:=RS",
                            "-p",
                            "Lidar_topic:=/adbscan_stream_input",
                            "-p",
                            "Verbose:=false",
                            "-p",
                            f"use_adbscan_core:={enabled}",
                            "-p",
                            f"oneapi_library:={backend}",
                            *(["-p", f"core_backend:={backend}"] if enabled == "true" else []),
                            "-p",
                            "subsample_ratio:=150.0",
                            "-p",
                            "Z_based_ground_removal:=1.0",
                            "-p",
                            "z_filter:=0.0",
                            "-p",
                            "x_filter_back:=3.0",
                            "-p",
                            "y_filter_left:=2.0",
                            "-p",
                            "y_filter_right:=-2.0",
                            "-p",
                            "base:=5.0",
                            "-p",
                            "scale_factor:=1.0",
                        ],
                        stdout=subprocess.DEVNULL,
                        stderr=stderr,
                    )
                    try:
                        print(f"30hz {enabled}: waiting for subscriber", flush=True)
                        deadline = time.monotonic() + 8
                        while (
                            publisher.get_subscription_count() == 0 and time.monotonic() < deadline
                        ):
                            rclpy.spin_once(self.node, timeout_sec=0.05)
                        self.assertGreater(publisher.get_subscription_count(), 0)

                        def publish_frame(message):
                            message.header.stamp = self.node.get_clock().now().to_msg()
                            stamp = (message.header.stamp.sec, message.header.stamp.nanosec)
                            sent_at[stamp] = time.monotonic()
                            publisher.publish(message)
                            return stamp

                        probe = PointCloud2(
                            header=Header(frame_id=cloud.header.frame_id),
                            height=1,
                            width=1,
                            fields=cloud.fields,
                            point_step=12,
                            row_step=12,
                            data=cloud.data[:12],
                            is_dense=True,
                        )
                        deadline = time.monotonic() + 15
                        while not received and time.monotonic() < deadline:
                            publish_frame(probe)
                            rclpy.spin_once(self.node, timeout_sec=0.1)
                            if process.poll() is not None:
                                stderr.seek(0)
                                self.fail(stderr.read().decode(errors="replace")[-4096:])
                        if not received:
                            stderr.seek(0)
                            self.fail(
                                f"warmup timed out: {enabled}; "
                                f"other stamps={unmatched_stamps[:4]}; "
                                f"subscriber log={stderr.read().decode(errors='replace')[-4096:]}"
                            )
                        sent_at.clear()
                        received.clear()

                        print(f"30hz {enabled}: warming full depth frame", flush=True)
                        warmup_stamps = set()
                        next_send = 0.0
                        deadline = time.monotonic() + 15
                        while (
                            not warmup_stamps.intersection(received)
                            and time.monotonic() < deadline
                        ):
                            now = time.monotonic()
                            if now >= next_send:
                                warmup_stamps.add(publish_frame(cloud))
                                next_send = now + 1.0
                            rclpy.spin_once(self.node, timeout_sec=0.05)
                            if process.poll() is not None:
                                stderr.seek(0)
                                self.fail(stderr.read().decode(errors="replace")[-4096:])
                        self.assertTrue(
                            warmup_stamps.intersection(received),
                            f"full-frame warmup timed out: {enabled}",
                        )
                        sent_at.clear()
                        received.clear()

                        print(f"30hz {enabled}: streaming {frame_count} frames", flush=True)
                        start = time.monotonic()
                        for frame in range(frame_count):
                            scheduled = start + frame * period
                            while time.monotonic() < scheduled:
                                rclpy.spin_once(
                                    self.node,
                                    timeout_sec=min(scheduled - time.monotonic(), 0.01),
                                )
                            late += time.monotonic() - scheduled > period
                            publish_frame(cloud)
                            rclpy.spin_once(self.node, timeout_sec=0)
                        deadline = time.monotonic() + 5
                        while len(received) < frame_count and time.monotonic() < deadline:
                            rclpy.spin_once(self.node, timeout_sec=0.05)
                        durations = sorted(latency * 1000 for latency, _ in received.values())
                        self.assertTrue(durations, f"no streamed output: {enabled}")
                        p95 = durations[math.ceil(len(durations) * 0.95) - 1]
                        missed = frame_count - len(received)
                        print(
                            f"30hz {enabled}: received={len(received)}/{frame_count} "
                            f"missed={missed} late_sends={late} "
                            f"median_ms={durations[len(durations) // 2]:.2f} p95_ms={p95:.2f}"
                        )
                        observed[enabled] = next(iter(received.values()))[1]
                        self.assertEqual(len(observed[enabled]), 16)
                        for _, boxes in received.values():
                            self.assertEqual(len(boxes), 16)
                            for reference, current in zip(observed[enabled], boxes):
                                for expected, actual in zip(reference, current):
                                    self.assertTrue(math.isclose(expected, actual, abs_tol=1e-3))
                    finally:
                        process.terminate()
                        try:
                            process.wait(timeout=3)
                        except subprocess.TimeoutExpired:
                            process.kill()
                            process.wait()
                        self.node.destroy_subscription(subscription)

            if "false" in observed and "true" in observed:
                for legacy, core in zip(observed["false"], observed["true"]):
                    for expected, actual in zip(legacy, core):
                        self.assertTrue(math.isclose(expected, actual, abs_tol=1e-3))
        finally:
            self.node.destroy_publisher(publisher)
