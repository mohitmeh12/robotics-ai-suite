<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Multi-camera AI Demo Workshop

This tutorial runs the [multi-camera object detection demo](../../../ai_resources/openvino/reference_applications/openvino_multicam_demo.md) (`ros-${ROS_DISTRO}-pyrealsense2-ai-demo`) in a workshop. The demo runs one YOLOv8 pipeline per Intel® RealSense™ camera, in parallel, with OpenVINO™ inference on the Intel® GPU and segmentation post-processing on the GPU through `dpnp`. This workshop runs it with two USB D435i cameras: YOLOv8n detection on the first camera and YOLOv8n segmentation on the second.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- Two USB RealSense™ cameras connected to the host, ideally on separate USB 3 controllers.
- A local desktop session for the demo window.

## Launch the workshop and connect the cameras

1. Launch the workshop. It takes about 11 minutes:

   ```bash
   cd src/components/multicam-demo/workshop
   workshop launch multicam
   ```

2. Connect the cameras. They never connect automatically:

   ```bash
   workshop connect multicam/camera:camera
   ```

3. List the cameras:

   ```bash
   workshop run multicam find-cameras
   ```

   The action prints each camera's index, model, serial number and USB type. The demo configuration refers to cameras by this index.

## Set up the demo

```bash
workshop run multicam install
```

This takes a few minutes. It creates the demo's Python environment (reusing the suite's OpenVINO™, OpenCV, numpy and `pyrealsense2`), installs the demo's Python dependencies, builds its Cython extension, and converts YOLOv8n and YOLOv8n-seg to OpenVINO™ FP16. It is safe to run again.

:::{important}
Run `install` once after every launch and every rebuild. The package installs only the demo's sources, not its Python environment or models, and a rebuild discards them.
:::

## Check the pipelines without a display

```bash
workshop run multicam demo-check
```

The action runs the demo for 30 seconds without a window, and prints PASS if every camera reaches 10 inference frames per second. Pass a duration in seconds to change the run time, for example `demo-check 60`.

With two D435i cameras on Intel® Arc™ B390 graphics, both pipelines keep up with the cameras' 30 fps:

| Camera | Model | Camera fps | Inference fps | GPU time per frame |
|---|---|---|---|---|
| 0 | YOLOv8n | 29.8 | 29.1 | 12-15 ms |
| 1 | YOLOv8n-seg | 29.4 | 29.4 | 13-15 ms |

## Run the demo

```bash
workshop connect multicam/robotics-ai-suite:desktop
workshop run multicam demo
```

A window shows both cameras side by side, with detections on one and segmentation masks on the other. Press `q` or Esc to quit.

## Use more cameras or other models

The workshop's camera configuration is `config/two-cameras.json`. To use up to four cameras, add entries to it, with `source` set to the index from `find-cameras`. `install` converts `yolov8n` and `yolov8n-seg`; to use other models, edit the model list in the `install` action.

## Actions

| Action | Description |
|---|---|
| `find-cameras` | RealSense™ cameras visible in the workshop, with the index that the configuration uses as `source` |
| `install` | Creates the demo's Python environment, installs its dependencies, builds its Cython extension, and converts the models to OpenVINO™ FP16 |
| `demo` | The demo window with both cameras and their detections or masks. `q` or Esc to quit |
| `demo-check [seconds]` | Headless for 30 seconds by default. PASS if every camera in the configuration reaches 10 inference frames per second |

## Humble

Use `multicam-humble` in every command. `demo-check` passes with the same results as on Jazzy, both cameras at about 29 inference frames per second.

Only one workshop can stream from a camera at a time, so run `multicam`, `multicam-humble` and the [RealSense™ Camera](realsense.md) workshop one after another.

## Known issues

The `install` action works around these problems in the package:

- **No setup at install time.** The upstream documentation says installing the package also installs the Python dependencies and models, but the package only sets file permissions.
- **Camera configurations missing.** The configuration files named in the upstream documentation are not in the package. The workshop ships its own.
- **Python version pins.** `ultralytics` is pinned differently in the package's `requirements.txt`, `pyproject.toml` and Dockerfile; `install` uses `>=8.0.20,<9`. `dpnp` 0.16 needs `dpctl` 0.19 and the oneAPI 2025.3 SYCL runtime, so `install` pins both.
- **Library path.** The `dpnp` wheel looks for its SYCL and MKL libraries in a path from its build machine. `install` adds the environment's `lib` directory to `LD_LIBRARY_PATH` in the environment's `activate` script.
- **Cython build on Python 3.10 (Humble).** setuptools 74 and later removed a module that numpy 1.26's build hooks import. On Python older than 3.12, `install` builds with `SETUPTOOLS_USE_DISTUTILS=stdlib`.
- **Size.** `install` uses PyTorch's CPU-only wheel, because PyTorch is only used to convert the models; the default wheel pulls in several GB of CUDA libraries.

One issue remains:

- **Crash or hang on exit.** The demo crashes while exiting, after printing its summary, with any model, camera count or device. On Humble it sometimes hangs there instead. Results are not affected; `demo-check` judges the summary instead of the exit code, and stops the demo 60 seconds after its run time.

## Source

The definitions, package list and camera configuration are in [`src/components/multicam-demo/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/multicam-demo/workshop).
