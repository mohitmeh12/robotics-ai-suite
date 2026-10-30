<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Object Detection Workshop

This tutorial runs the [OpenVINO™ object detection tutorial](../../../ai_resources/openvino/reference_applications/object_detection_tutorial.md) in a workshop, with inference on the CPU, Intel® GPU and Intel® NPU. The workshop installs the full Robotics AI Suite with the official robotics-installer, plus `ros-${ROS_DISTRO}-object-detection-tutorial`.

You will:

1. Launch the workshop and connect the NPU and the desktop.
2. Compare inference latency on the CPU, GPU and NPU.
3. Watch detections in RViz on each device.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- A local desktop session for the RViz steps.

## Launch the workshop

1. Go to the workshop directory and launch it. The first launch takes 10 to 30 minutes:

   ```bash
   cd src/components/object-detection/workshop
   workshop launch object-detection
   ```

2. Connect the NPU and the desktop. Neither connects automatically:

   ```bash
   workshop connect object-detection/robotics-ai-suite:npu :custom-device
   workshop connect object-detection/robotics-ai-suite:desktop
   ```

3. Apply the NPU fix. It takes about 30 seconds and is needed for the NPU to publish detections; see [Known issues](#known-issues):

   ```bash
   workshop run object-detection build-npu-fix
   ```

## Measure inference latency

Run OpenVINO™ `benchmark_app` with the tutorial's model on each device:

```bash
workshop run object-detection benchmark
```

On an Intel® Core™ Ultra X7 358H processor, median latency is about 3.8 ms on the CPU, 0.8 ms on the GPU and 0.9 ms on the NPU. The action also reports NPU busy time.

To measure the rate of the full ROS 2 detection pipeline without opening a window, run:

```bash
workshop run object-detection benchmark-ros
```

It also prints which `openvino_wrapper_lib` is loaded, so you can confirm the NPU fix is in use.

## View detections in RViz

Run the tutorial on one device at a time:

```bash
workshop run object-detection ros-object-detection-gpu
```

RViz opens and shows the detections. Press Ctrl+C in the terminal to stop it, then try `ros-object-detection-cpu` or `ros-object-detection-npu`.

## Actions

| Action | Description |
|---|---|
| `build-npu-fix` | Rebuilds `openvino_wrapper_lib` with the SSD NPU fix as an overlay in `~/ov_ws`. Once built, the other actions use it automatically |
| `ros-object-detection-cpu`, `-gpu`, `-npu` | The tutorial launch with RViz, on the selected device |
| `benchmark` | `benchmark_app` latency on the CPU, GPU and NPU, plus NPU busy time |
| `benchmark-ros` | Headless detection pipeline rate on the CPU, GPU and NPU |

## Performance compared with the host

The same `benchmark_app` run (OpenVINO™ 2025.3, latency hint, 10 s) on the host and in this workshop, alternated three times per device, gave 98% (CPU), 101% (GPU) and 101% (NPU) of the host's throughput. The differences are within run-to-run variation: running in a workshop adds no measurable inference overhead.

## Humble

Use `object-detection-humble` in every command. `benchmark` and `benchmark-ros` work on the CPU and GPU. The NPU does not work on Humble, and `build-npu-fix` does not apply; see [Known issues on Humble](ros-distros.md#known-issues-on-humble).

## Known issues

- **No NPU detections without the fix (Jazzy).** With `ros-jazzy-openvino-wrapper-lib` `2025.3.0-0~0.11.0eci10`, the NPU runs inference but the tutorial publishes no detections. The cause is the SSD preprocessing in `ros2_openvino_toolkit`, fixed upstream in commit [`f6e0f5d`](https://github.com/intel/ros2_openvino_toolkit/commit/f6e0f5d56b8f4720d73f73e9b2379979bf789e83), which is not yet in a released package. `build-npu-fix` applies that commit. CPU and GPU are not affected.
- **A rebuild removes the fix.** `workshop refresh` after an SDK change rebuilds the container and removes `~/ov_ws`. Run `build-npu-fix` again afterwards.

## Source

The definitions, package list and patch are in [`src/components/object-detection/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/object-detection/workshop).
