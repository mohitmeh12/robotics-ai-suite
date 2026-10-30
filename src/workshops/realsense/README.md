<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# realsense

A RealSense camera attached to the host, used from inside the container through Workshop's `camera` interface. The workshop runs the `realsense2_tutorial` package (`ros-${ROS_DISTRO}-realsense2-tutorial-demo`): the `realsense2_camera` driver with aligned depth, plus RViz2.

- Definitions: [realsense.yaml](.workshop/realsense.yaml) (Jazzy), [realsense-humble.yaml](.workshop/realsense-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- SDKs: `proxy`, `robotics-ai-suite`, `camera` (camera plug only)
- Check: [realsense_check.py](realsense_check.py)

## Get started with Workshop

Requires the [prerequisites](../docs/getting-started.md#prerequisites) and a USB RealSense camera on the host.

```bash
cd src/workshops/realsense
workshop launch realsense                   # or realsense-humble
workshop connect realsense/camera:camera    # never auto-connects
workshop run realsense find-cameras
workshop run realsense tutorial-check
workshop connect realsense/robotics-ai-suite:desktop
workshop run realsense tutorial             # live images in RViz; Ctrl+C to stop
```

## Actions

| Action | Description |
|---|---|
| `find-cameras` | Device nodes in the container, the package's `find_cameras.sh` (prints the `camera_type` to use), and `rs-enumerate-devices -s` |
| `tutorial [usb\|gmsl]` | Documented launch `realsense2_tutorial.launch.py camera_type:=usb` (default): live color and aligned depth images in RViz2 |
| `tutorial-check` | Headless: the same driver arguments without RViz. PASS if color, depth and aligned depth each stream at 10 Hz or more and depth is more than 20% valid |

The `camera` plug alone is enough for a USB RealSense. It passes `/dev/video*` and `/dev/media*`, and the packaged librealsense uses the kernel V4L2 backend, so no USB bus or custom-device plug is needed.

## Results

With a D435i (firmware 5.17.3.10) on Panther Lake:

| Distro | `tutorial-check` |
|---|---|
| Jazzy | PASS: color 1280x720 at about 29 Hz, depth 848x480 at about 30 Hz, aligned depth at about 27 Hz |
| Humble | PASS: color about 26 Hz, depth about 30 Hz |

`camera_type:=gmsl` (D457 over GMSL) has not been tested.

## Hot-plugging

With the plug connected:

| Case | Result |
|---|---|
| Unplug and replug while no driver is running | The device nodes leave and return in the container. The next `tutorial-check` passes without restarting the workshop |
| Unplug and replug while the driver is streaming | The driver logs `The device has been disconnected!` and stays up, but never reattaches; restart the action. Natively on the host, the same driver reconnects by itself |

The driver waits for udev `add` events to find the camera again. On replug the host sees `add` for all six video nodes, but the container only receives three (`video1/3/5`, not the `video0/2/4` stream nodes), and each `remove` arrives twice.
