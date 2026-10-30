# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""Ask Nav2's planner (ITS plugin) for a path in the TurtleBot3 simulation and print it."""
import math
import time

import rclpy
from geometry_msgs.msg import PoseWithCovarianceStamped
from lifecycle_msgs.srv import GetState
from nav2_msgs.action import ComputePathToPose
from rclpy.action import ActionClient

START, GOAL = (-2.0, -0.5), (2.0, 0.5)  # START is the spawn pose in tb3_simulation_launch.py

rclpy.init()
node = rclpy.create_node("its_plan_check")

# The global costmap only activates once AMCL has an initial pose (map->odom TF).
init = PoseWithCovarianceStamped()
init.header.frame_id = "map"
init.pose.pose.position.x, init.pose.pose.position.y = START
init.pose.pose.orientation.w = 1.0
pub = node.create_publisher(PoseWithCovarianceStamped, "/initialpose", 1)
state = node.create_client(GetState, "/planner_server/get_state")
deadline = time.time() + 180
while time.time() < deadline:
    pub.publish(init)
    if state.wait_for_service(timeout_sec=2.0):
        f = state.call_async(GetState.Request())
        rclpy.spin_until_future_complete(node, f, timeout_sec=2.0)
        if f.result() and f.result().current_state.label == "active":
            break
else:
    raise SystemExit("planner_server did not become active")

client = ActionClient(node, ComputePathToPose, "/compute_path_to_pose")
client.wait_for_server(timeout_sec=30.0)
goal = ComputePathToPose.Goal()
goal.planner_id = "GridBased"
goal.use_start = True
goal.start.header.frame_id = goal.goal.header.frame_id = "map"
goal.start.pose.position.x, goal.start.pose.position.y = START
goal.goal.pose.position.x, goal.goal.pose.position.y = GOAL
goal.start.pose.orientation.w = goal.goal.pose.orientation.w = 1.0
path = []
for attempt in range(10):
    f = client.send_goal_async(goal)
    rclpy.spin_until_future_complete(node, f, timeout_sec=10.0)
    r = f.result().get_result_async()
    rclpy.spin_until_future_complete(node, r, timeout_sec=60.0)
    path = r.result().result.path.poses if r.result() else []
    if path:
        break
    time.sleep(3.0)
if not path:
    raise SystemExit("ITS planner returned no path")

p0, p1 = path[0].pose.position, path[-1].pose.position
length = sum(math.dist((a.pose.position.x, a.pose.position.y), (b.pose.position.x, b.pose.position.y))
             for a, b in zip(path, path[1:]))
print(f"ITS path: {len(path)} poses, {length:.2f} m, ({p0.x:.2f},{p0.y:.2f}) -> ({p1.x:.2f},{p1.y:.2f}),"
      f" requested {START} -> {GOAL}, attempt {attempt + 1}")
rclpy.shutdown()
