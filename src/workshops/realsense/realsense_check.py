# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""Measure RealSense image rates and depth validity from the live driver."""
import sys
import time

import numpy as np
import rclpy
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Image

DURATION = float(sys.argv[1]) if len(sys.argv) > 1 else 10
MIN_HZ = 10.0
TOPICS = ["/camera/color/image_raw", "/camera/depth/image_rect_raw", "/camera/aligned_depth_to_color/image_raw"]

rclpy.init()
node = rclpy.create_node("realsense_check")
stats = {t: {"t": [], "size": None, "valid": []} for t in TOPICS}


def make_cb(topic):
    def cb(m):
        s = stats[topic]
        s["t"].append(time.time())
        s["size"] = f"{m.width}x{m.height} {m.encoding}"
        if m.encoding == "16UC1":
            d = np.frombuffer(m.data, dtype=np.uint16)
            s["valid"].append(np.count_nonzero(d) / d.size)
    return cb


for t in TOPICS:
    node.create_subscription(Image, t, make_cb(t), qos_profile_sensor_data)
end = time.time() + DURATION
while time.time() < end:
    rclpy.spin_once(node, timeout_sec=0.1)

ok = True
for t in TOPICS:
    s = stats[t]
    ts = s["t"]
    hz = (len(ts) - 1) / (ts[-1] - ts[0]) if len(ts) > 1 else 0.0
    line = f"{t}: {len(ts)} msgs, {hz:.1f} Hz, {s['size']}"
    if s["valid"]:
        line += f", {100 * np.mean(s['valid']):.0f}% valid depth"
    print(line)
    ok &= hz >= MIN_HZ and (not s["valid"] or np.mean(s["valid"]) > 0.2)
print("PASS" if ok else f"FAIL: expected every stream >= {MIN_HZ:.0f} Hz and >20% valid depth")
rclpy.shutdown()
sys.exit(0 if ok else 1)
