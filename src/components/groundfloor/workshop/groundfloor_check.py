# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""Subscribe to the groundfloor segmentation outputs and check they look like a real segmentation."""
import sys
import time

import numpy as np
import rclpy
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import PointCloud2
from sensor_msgs_py import point_cloud2 as pc2

DURATION = float(sys.argv[1]) if len(sys.argv) > 1 else 25
# PointLabel enum in pointcloud_groundfloor_segmentation/Monitor.hpp
NAMES = ["UNKNOWN", "UNDEFINED", "INVALID", "GROUND", "ELEVATED", "OBSTACLE", "ABOVE"]

rclpy.init()
node = rclpy.create_node("groundfloor_check")
stats = {k: {"msgs": 0, "points": 0, "t": []} for k in ("labeled", "obstacle")}
labels = np.zeros(len(NAMES), dtype=np.int64)


def cb(name):
    def f(m):
        s = stats[name]
        s["msgs"] += 1
        s["points"] += m.width * m.height
        s["t"].append(time.time())
        if name == "labeled":
            lab = pc2.read_points(m, field_names=["label"], skip_nans=False)["label"].astype(np.int64)
            labels[:] += np.bincount(lab, minlength=len(NAMES))[: len(NAMES)]
    return f


node.create_subscription(PointCloud2, "/segmentation/labeled_points", cb("labeled"), qos_profile_sensor_data)
node.create_subscription(PointCloud2, "/segmentation/obstacle_points", cb("obstacle"), qos_profile_sensor_data)
end = time.time() + DURATION
while time.time() < end:
    rclpy.spin_once(node, timeout_sec=0.1)

for k, s in stats.items():
    rate = (len(s["t"]) - 1) / (s["t"][-1] - s["t"][0]) if len(s["t"]) > 1 else 0.0
    mean = s["points"] // s["msgs"] if s["msgs"] else 0
    print(f"/segmentation/{k}_points: {s['msgs']} msgs, {rate:.1f} Hz, mean {mean} points")
total = labels.sum() or 1
share = {n: 100.0 * labels[i] / total for i, n in enumerate(NAMES) if labels[i]}
print("labels: " + ", ".join(f"{n} {p:.1f}%" for n, p in share.items()))

ok = (stats["labeled"]["msgs"] >= 5 and stats["obstacle"]["msgs"] >= 5
      and share.get("GROUND", 0) >= 5 and share.get("OBSTACLE", 0) >= 5)
print("PASS" if ok else "FAIL: expected both outputs and a ground/obstacle split")
rclpy.shutdown()
sys.exit(0 if ok else 1)
