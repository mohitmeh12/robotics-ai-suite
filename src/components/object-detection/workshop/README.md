<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# object-detection

The full Robotics AI Suite, installed with the official installer, plus the OpenVINO object detection tutorial on CPU, GPU and NPU.

- Definitions: [object-detection.yaml](.workshop/object-detection.yaml) (Jazzy), [object-detection-humble.yaml](.workshop/object-detection-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt) (`ros-${ROS_DISTRO}-object-detection-tutorial`)
- Patch: [patches/ssd-npu-fix.patch](patches/ssd-npu-fix.patch)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites).

```bash
cd src/components/object-detection/workshop
workshop launch object-detection            # Jazzy; first launch 10-30 min
workshop connect object-detection/robotics-ai-suite:npu :custom-device
workshop connect object-detection/robotics-ai-suite:desktop   # for RViz; needs a local X/Wayland session
workshop run object-detection build-npu-fix                   # Jazzy only, ~30 s; see Known issues
workshop run object-detection ros-object-detection-gpu
```

For Humble, use `object-detection-humble` in every command.

## Actions

| Action | Description |
|---|---|
| `build-npu-fix` | Rebuilds `openvino_wrapper_lib` with the SSD NPU fix as an overlay in `~/ov_ws`. Once built, the actions below use it automatically |
| `ros-object-detection-cpu` / `-gpu` / `-npu` | Tutorial launch with RViz on the selected device |
| `benchmark` | `benchmark_app` latency for the tutorial model on CPU, GPU and NPU, plus NPU busy time |
| `benchmark-ros` | Headless detection pipeline rate on CPU, GPU and NPU (prints which wrapper library is loaded) |

## Results

On Panther Lake (Core Ultra X7 358H), median inference latency is 3.83 ms on CPU, 0.78 ms on GPU and 0.86 ms on NPU.

**Container overhead:** the same `benchmark_app` run (OpenVINO 2025.3, latency hint, 10 s) on the host and in this workshop, alternating three times per device, gave 98% (CPU), 101% (GPU) and 101% (NPU) of the host's throughput. The differences are within run-to-run variation.

**Humble:** `benchmark` and `benchmark-ros` work on CPU and GPU. The NPU fails, and `build-npu-fix` doesn't apply. See [Humble variants](../../../workshops/docs/humble-and-interop.md#humble-variants).

## Known issues

- **No NPU detections (Jazzy):** with `ros-jazzy-openvino-wrapper-lib` `2025.3.0-0~0.11.0eci10`, the NPU runs inference but the tutorial publishes no detections. The cause is the SSD preprocessing in `ros2_openvino_toolkit`'s `ObjectDetectionSSDModel::updateLayerProperty()`, fixed upstream in [`f6e0f5d`](https://github.com/intel/ros2_openvino_toolkit/commit/f6e0f5d56b8f4720d73f73e9b2379979bf789e83) on branch `mm/ros2-fix-ssd-shape-npu`, which is not yet merged into `ros2` or released in a package. CPU and GPU are not affected. Run `build-npu-fix` to apply the fix (`patches/ssd-npu-fix.patch` is that commit); after that the NPU publishes the same detections as CPU and GPU.
- `workshop refresh` rebuilds the container and removes `~/ov_ws`. Run `build-npu-fix` again afterwards.
