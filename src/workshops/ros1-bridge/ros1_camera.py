# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""ROS 1 side: publish raw RGB frames on /ros1/image and measure the round trip of /ros1/detections.

Usage: ros1_camera.py <frame.raw> <width> <height> <hz> <seconds>. Exits 1 if fewer than 95% of frames are answered.
"""
import json, sys, time
import rospy
from sensor_msgs.msg import Image
from std_msgs.msg import String

RAW, W, H = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
HZ, DURATION, WARMUP = float(sys.argv[4]), float(sys.argv[5]), 8.0
data = open(RAW, "rb").read()
assert len(data) == W * H * 3, "raw file size does not match WxHx3"

rospy.init_node("ros1_camera")
pub = rospy.Publisher("/ros1/image", Image, queue_size=2)
sent, rtt, labels = {}, [], {}


def on_dets(msg):
    d = json.loads(msg.data)
    if d["frame"] in sent and sent[d["frame"]] > start + WARMUP:
        rtt.append((time.time() - d["stamp"]) * 1000)
        for det in d["dets"]:
            labels[det[0]] = labels.get(det[0], 0) + 1


rospy.Subscriber("/ros1/detections", String, on_dets, queue_size=10)
rate = rospy.Rate(HZ)
start = time.time()
i = 0
while not rospy.is_shutdown() and time.time() < start + WARMUP + DURATION:
    msg = Image(height=H, width=W, encoding="rgb8", step=W * 3, data=data)
    msg.header.stamp = rospy.Time.from_sec(time.time())
    msg.header.frame_id = str(i)
    sent[msg.header.frame_id] = time.time()
    pub.publish(msg)
    i += 1
    rate.sleep()
time.sleep(1)

measured = sum(1 for t in sent.values() if t > start + WARMUP)
rtt.sort()
pct = lambda q: rtt[min(len(rtt) - 1, int(q / 100 * len(rtt)))] if rtt else 0.0
print(f"ros1_camera: {W}x{H} @ {HZ:g} Hz; {len(rtt)}/{measured} frames answered ({100 * len(rtt) / max(measured, 1):.0f}%); "
      f"round trip p50 {pct(50):.1f} ms p90 {pct(90):.1f} ms max {rtt[-1] if rtt else 0:.1f} ms; "
      f"result rate {len(rtt) / DURATION:.1f} Hz; labels {dict(sorted(labels.items(), key=lambda x: -x[1])[:4])}")
sys.exit(0 if measured and len(rtt) >= 0.95 * measured else 1)
