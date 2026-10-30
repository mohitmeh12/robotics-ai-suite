# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""ROS 2 side: run the tutorial's SSD model with OpenVINO on /ros1/image, publish detections as JSON on /ros1/detections.

Usage: ros1_detector.py [device] [max_seconds]. Prints "ready" once the model is compiled, exits 5 s after the last frame.
"""
import glob, json, sys, time
import cv2
import numpy as np
import openvino as ov
import rclpy
from rclpy.qos import QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Image
from std_msgs.msg import String

DEVICE = sys.argv[1] if len(sys.argv) > 1 else "GPU"
MAX_SECONDS = float(sys.argv[2]) if len(sys.argv) > 2 else 120
IDLE_EXIT = 5.0
MODEL = "/opt/openvino_toolkit/models/convert/public/ssdlite_mobilenet_v2/FP16/ssdlite_mobilenet_v2.xml"
LABELS = open(glob.glob("/opt/ros/*/share/object_detection_tutorial/label/object.labels")[0]).read().split("\n")

core = ov.Core()
# Resize on the CPU: with OpenVINO's embedded resize (PrePostProcessor) the NPU returns no detections.
req = core.compile_model(core.read_model(MODEL), DEVICE).create_infer_request()

rclpy.init()
node = rclpy.create_node("ros1_detector")
pub = node.create_publisher(String, "/ros1/detections", 10)
one_way, infer = [], []
last = None


def on_image(msg):
    global last
    t_recv = last = time.time()
    stamp = msg.header.stamp.sec + msg.header.stamp.nanosec * 1e-9
    frame = np.frombuffer(msg.data, dtype=np.uint8).reshape(msg.height, msg.width, 3)
    t0 = time.time()
    out = req.infer({0: cv2.resize(frame, (300, 300))[None]})[0][0][0]
    infer.append((time.time() - t0) * 1000)
    one_way.append((t_recv - stamp) * 1000)
    dets = [[LABELS[int(d[1])], round(float(d[2]), 2)] + [round(float(v), 3) for v in d[3:7]] for d in out if d[2] > 0.5]
    pub.publish(String(data=json.dumps({"stamp": stamp, "frame": msg.header.frame_id, "infer_ms": infer[-1], "dets": dets})))


node.create_subscription(Image, "/ros1/image", on_image, QoSProfile(depth=5, reliability=ReliabilityPolicy.RELIABLE))
print(f"detector[{DEVICE}]: ready", flush=True)
end = time.time() + MAX_SECONDS
while time.time() < end and (last is None or time.time() < last + IDLE_EXIT):
    rclpy.spin_once(node, timeout_sec=0.1)

p = lambda xs, q: float(np.percentile(xs, q)) if xs else 0.0
print(f"detector[{DEVICE}]: {len(infer)} frames; bridge one-way p50 {p(one_way, 50):.1f} ms p90 {p(one_way, 90):.1f} ms; "
      f"resize+inference p50 {p(infer, 50):.2f} ms")
rclpy.shutdown()
