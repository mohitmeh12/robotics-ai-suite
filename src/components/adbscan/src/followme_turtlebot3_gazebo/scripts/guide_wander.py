#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0

import math
import random
import time

import rclpy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.node import Node


class GuideWander(Node):
    def __init__(self):
        super().__init__("guide_wander")
        self.publisher = self.create_publisher(Twist, "/guide_robot/cmd_vel", 10)
        self.create_subscription(Odometry, "/guide_robot/odom", self.update_odom, 10)
        self.create_timer(0.1, self.drive)
        self.position = None
        self.heading = 0.0
        self.next_change = 0.0
        self.speed = 0.0
        self.turn = 0.0

    def update_odom(self, odom):
        self.position = odom.pose.pose.position
        orientation = odom.pose.pose.orientation
        self.heading = math.atan2(
            2 * (orientation.w * orientation.z + orientation.x * orientation.y),
            1 - 2 * (orientation.y**2 + orientation.z**2),
        )

    def drive(self):
        command = Twist()
        if self.position is None:
            self.publisher.publish(command)
            return

        distance = math.hypot(self.position.x - 0.8, self.position.y)
        if distance > 1.7:
            direction = math.atan2(-self.position.y, 0.8 - self.position.x)
            error = math.atan2(
                math.sin(direction - self.heading), math.cos(direction - self.heading)
            )
            command.angular.z = max(-0.6, min(0.6, 1.5 * error))
            command.linear.x = 0.1 if abs(error) < 0.8 else 0.0
        else:
            if time.monotonic() >= self.next_change:
                self.next_change = time.monotonic() + random.uniform(1.0, 3.0)
                self.speed = random.choice([0.0, random.uniform(0.08, 0.17)])
                self.turn = random.uniform(-0.55, 0.55)
            command.linear.x = self.speed
            command.angular.z = self.turn
        self.publisher.publish(command)


def main():
    rclpy.init()
    node = GuideWander()
    try:
        rclpy.spin(node)
    finally:
        node.publisher.publish(Twist())
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
