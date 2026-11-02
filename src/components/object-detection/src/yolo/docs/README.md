<!--
Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# OpenVINO™ YOLOv8 Tutorial

---

This tutorial explains how to install and run the ROS 2 OpenVINO™ YOLOv8
node. It supports detection, segmentation, and pose models, and can
process images from configurable ROS 2 topics on a CPU or GPU.

## Prerequisites

- [Prepare the target system](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/)
- [Setup the Robotics AI Dev Kit APT Repositories](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#3-set-up-robotics-ai-suite-oneapi-and-graphics-apt-repositories)
- [Install OpenVINO™ Packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#4-install-openvino-packages)
- [Install Robotics AI Dev Kit Deb packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#6-install-robotics-ai-suite-deb-packages)
- [Install the Intel® NPU Driver on Intel® Core™ Ultra Processors (if applicable)](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#7-install-the-intel-npu-driver-on-intel-core-ultra-processors)

## Install the OpenVINO™ package

Follow the instructions on [Install OpenVINO™ Packages](https://developer.robotics.intel.com/development-stack/platform_foundation/getting_started/step_by_step/#4-install-openvino-packages) to install OpenVINO™.

## Install the Python packages (optional)

The following Python packages are needed to download a model
automatically and convert it to OpenVINO IR format. If you already have
a model, you can provide its path in the configuration instead.

> ```bash
> pip3 install numpy pandas openvino-dev ultralytics nncf onnx
> ```

## Install the Debian packages

> ```bash
> sudo apt install ros-humble-openvino-yolov8 ros-humble-openvino-yolov8-msgs
> ```

For ROS 2 Jazzy, replace `humble` in both package names with `jazzy`.

## Run the Demo with Intel® RealSense™ Image Topics

Create a configuration file named `pipeline.toml` in your current
directory. If the file is missing or invalid, running `ros2 run yolo
yolo` prints an example configuration to the terminal.

> ```bash
> title = "Yolo Ros Node"
>
> [main]
> pipelines = ["pipeline1"] # names will be used to name output topics
> models = ["model1"]
>
> [model1]
> model_path = "" # if empty, model will be downloaded from ultralytics, otherwise provide path to .xml file or other format supported by openvino
> model_path_bin = "" # use only if you want to provide your own model in openvino format, and there you need to provide path to .bin file
> # following options are only used if model_path is empty
> task = "segmentation" # options: detection, segmentation, pose
> # w,h of internal resolution, input frames are resized
> # consider using smaller resolution for faster inference
> width = 640
> height = 480
> model_size = "n" # options: n, s, m, l, x (refer to ultralytics docs)
> half = true # use half precision
>
>
> [pipeline1]
> model = "model1" # model name from [main] section
> device = "CPU"  # options CPU, GPU (this is directly passed to openvino)
> performance_mode = 1 # Performance mode 1=Latency, 2=Throughput, 3=Cumulative throughput
> priority = 1 # 0=Low, 1=Normal, 2=High
> precision = 1 #  0 = FP32, 1 = FP16 , this is passed as hint to openvino
> num_requests = 1 # Number of inference requests (hint for openvino) recommended 1 per input stream
> map_frame = "" # setting is ignored if single topic is used, otherwise it will be used to synchronize camera location
> queue_size = 10
> workers = 2 # Aim for 2 workers per input stream
> max_fps = -1 # -1 for unlimited
> publish_video = true # publish video with detections
> publish_detections = true # publish detections (special message type), can be used to generate video with detections
> # use this if you don't have or don't need synchronized depth data
> rgb_topic = []
> rgb_topic_max_fps = [] # same length as rgb_topic, -1 for unlimited will assume -1 for all topics if not provided
> # depth is not used by yolo, but if provided this node will synchronize rgb and depth and transform to camera frame
> # this can be later used to place detections in 3D space
> # topics need to be provided in pairs
> rgbd_topic_rgb = ["/camera/color/image_raw"]
> rgbd_topic_depth = ["/camera/depth/image_raw"]
> rgbd_topic_max_fps = []
> ```
>
> Start the YOLO node in the first terminal:
>
> ```bash
> ros2 run yolo yolo --toml pipeline.toml
> ```

In a second terminal, start the RealSense camera:

> ```bash
> ros2 run realsense2_camera realsense2_camera_node --ros-args \
>             -p depth_module.profile:=640x480x60 \
>             -p rgb_camera.profile:=640x480x60 \
>             -p align_depth.enable:=True \
>             -r /camera/camera/color/image_raw:=/camera/color/image_raw \
>             -r /camera/camera/color/camera_info:=/camera/color/camera_info \
>             -r /camera/camera/aligned_depth_to_color/image_raw:=/camera/depth/image_raw \
>             -r /camera/camera/aligned_depth_to_color/camera_info:=/camera/depth/camera_info
> ```

Once both processes are running, open RViz:

> ```bash
> rviz2
> ```

In RViz, add an Image display subscribed to
`/pipeline1/color/image_raw/yolo_video` to view the result. Press
`Ctrl+C` in each terminal to stop the processes.

## Advanced usage

In addition to publishing video, the node publishes post-processed
detections on the `<pipeline_name>/<rgb_topic>/yolo_frame` topic. With
the example configuration above, the topic is
`pipeline1/color/image_raw/yolo_frame`.

The messages have the following structure:

> ```bash
> std_msgs/Header header # Header timestamp should be acquisition time of image
>
> sensor_msgs/Image rgb_image # Original image
> sensor_msgs/Image depth_image # only if topic depth is provided
>
> string task # "Detection", "Segmentation", or "Pose"
>
> geometry_msgs/TransformStamped camera_transform # Camera transform captured at the time of image arrival
>
> YoloDetection[] detections # Array of detections
> ```

The `YoloDetection` message has the following structure:

> ```bash
> float32 confidence
>
> # Coordinates of bounding box
> uint32 x
> uint32 y
> uint32 height
> uint32 width
>
> string class_name
>
> # Only used for Pose task
> float32[] pose_xy #  X,Y of joints in image coordinates (17 total)
> float32[] pose_visible # Probability of joint being visible
>
> # Used only for segmentation; this is a flattened mask array the same size as the bounding box
> float32[] mask
> ```

The same message structure is used for all three tasks (detection,
segmentation, and pose). Fields that do not apply to a task are empty.

For pose estimation, see this [diagram of COCO keypoints and their
connections](https://www.researchgate.net/figure/Key-points-for-human-poses-according-to-the-COCO-output-format-R-L-right-left_fig3_353746430).

## Other considerations

YOLOv8 models require a commercial license from Ultralytics. This
package provides a way to run the model with OpenVINO™ and ROS 2. Models
and weights are downloaded from Ultralytics and converted to IR format.

The model must have a fixed input shape and 80 classes for detection or
segmentation. Keep this in mind when providing fine-tuned models.

Automatic downloading of INT8 models is supported only for square input
shapes and the detection task. This is a limitation of the
Ultralytics/nncf libraries. You can still use a quantized model for
another task or resolution by providing the model yourself.

The input image resolution from the ROS 2 topic does not need to match
the model input resolution. If the sizes differ, the image is resized
using bicubic interpolation, and the model output is scaled back to the
original image size.

Experiment with `performance_mode` and `num_requests` to find the right
balance between latency and throughput for your workload. Throughput
mode may increase throughput, but compare the results with your
application's latency requirements.
