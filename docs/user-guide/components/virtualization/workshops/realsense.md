<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# RealSense™ Camera Workshop

This tutorial uses an Intel® RealSense™ camera attached to the host from inside a workshop, through Workshop's `camera` interface. The workshop runs the [RealSense™ ROS 2 tutorial](../../sensors/reference_applications/realsense-ros2.md) (`ros-${ROS_DISTRO}-realsense2-tutorial-demo`): the `realsense2_camera` driver with aligned depth, plus RViz.

You will connect the camera, check that it streams, and view color and aligned depth images in RViz.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- A USB RealSense™ camera connected to the host. The workshop was tested with a D435i.
- A local desktop session for the RViz step.

## Launch the workshop and connect the camera

1. Launch the workshop:

   ```bash
   cd src/workshops/realsense
   workshop launch realsense
   ```

2. Connect the camera. It never connects automatically:

   ```bash
   workshop connect realsense/camera:camera
   ```

3. Check that the workshop sees the camera:

   ```bash
   workshop run realsense find-cameras
   ```

   The action lists the video device nodes, the `camera_type` to use, and the output of `rs-enumerate-devices -s`.

The `camera` interface alone is enough for a USB RealSense™ camera. It passes `/dev/video*` and `/dev/media*` to the workshop, and the packaged librealsense uses the kernel V4L2 backend, so no USB bus access is needed.

## Check the streams without a display

```bash
workshop run realsense tutorial-check
```

The action starts the driver with the tutorial's arguments, without RViz. It prints PASS if color, depth and aligned depth each stream at 10 Hz or more, and more than 20% of depth pixels are valid. With a D435i on Jazzy, color streams at 1280 x 720 at about 29 Hz, depth at 848 x 480 at about 30 Hz, and aligned depth at about 27 Hz.

## View the camera in RViz

```bash
workshop connect realsense/robotics-ai-suite:desktop
workshop run realsense tutorial
```

RViz shows live color and aligned depth images. Press Ctrl+C in the terminal to stop.

## Actions

| Action | Description |
|---|---|
| `find-cameras` | Device nodes in the workshop, the package's `find_cameras.sh` (prints the `camera_type` to use), and `rs-enumerate-devices -s` |
| `tutorial [usb\|gmsl]` | The documented `realsense2_tutorial.launch.py` with `camera_type:=usb` (default): live color and aligned depth images in RViz |
| `tutorial-check` | Headless: the same driver arguments without RViz. PASS if color, depth and aligned depth each stream at 10 Hz or more and depth is more than 20% valid |

`camera_type:=gmsl`, for a D457 over GMSL, has not been tested in a workshop.

## Unplugging and replugging the camera

With the camera interface connected:

- **No driver running:** the device nodes leave and return in the workshop. The next action works without restarting the workshop.
- **Driver streaming:** the driver logs `The device has been disconnected!` and stays up, but doesn't reconnect when the camera returns. Stop the action and run it again. (On the host, the same driver reconnects by itself: when the camera returns, the workshop receives only some of the device events the driver waits for.)

## Humble

Use `realsense-humble` in every command. `tutorial-check` passes, with color at about 26 Hz and depth at about 30 Hz.

Only one workshop can stream from a camera at a time. Stop the camera in one workshop before using it in another, such as the [Multi-camera AI Demo](multicam.md).

## Source

The definitions, package list and check script are in [`src/workshops/realsense/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/workshops/realsense).
