# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""Subscribe to ADBSCAN outputs and report obstacle counts per message."""
import sys
import time

import rclpy
from nav2_dynamic_msgs.msg import ObstacleArray
from visualization_msgs.msg import MarkerArray

DURATION = float(sys.argv[1]) if len(sys.argv) > 1 else 25
rclpy.init()
node = rclpy.create_node("adbscan_check")
obst = {"msgs": 0, "counts": [], "t": []}
markers = {"msgs": 0}


def on_obstacles(m):
    obst["msgs"] += 1
    obst["counts"].append(len(m.obstacles))
    obst["t"].append(time.time())


node.create_subscription(ObstacleArray, "/obstacle_array", on_obstacles, 10)
node.create_subscription(MarkerArray, "/marker_array", lambda m: markers.__setitem__("msgs", markers["msgs"] + 1), 10)
end = time.time() + DURATION
while time.time() < end:
    rclpy.spin_once(node, timeout_sec=0.1)

c, t = obst["counts"], obst["t"]
rate = (len(t) - 1) / (t[-1] - t[0]) if len(t) > 1 else 0.0
nonzero = sum(1 for x in c if x)
print(f"/obstacle_array: {obst['msgs']} msgs, {rate:.1f} Hz, obstacles per msg min/mean/max "
      f"{min(c, default=0)}/{(sum(c) / len(c)) if c else 0:.1f}/{max(c, default=0)}, {nonzero} msgs with obstacles")
print(f"/marker_array: {markers['msgs']} msgs")
ok = obst["msgs"] >= 5 and nonzero >= 0.5 * obst["msgs"]
print("PASS" if ok else "FAIL: expected /obstacle_array with clusters in most messages")
rclpy.shutdown()
sys.exit(0 if ok else 1)
