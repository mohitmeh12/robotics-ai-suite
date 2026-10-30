<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# multicam

The multi-camera AI demo (`ros-${ROS_DISTRO}-pyrealsense2-ai-demo`, from [`src/components/multicam-demo`](../README.md)) runs one YOLOv8 pipeline per RealSense camera, in parallel, with OpenVINO inference on the GPU and segmentation post-processing on the GPU through `dpnp`. Upstream targets four GMSL D457 cameras; this workshop runs it with two USB D435i cameras through the `camera` plug.

- Definitions: [multicam.yaml](.workshop/multicam.yaml) (Jazzy), [multicam-humble.yaml](.workshop/multicam-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- Camera config: [config/two-cameras.json](config/two-cameras.json): YOLOv8n detection on camera 0, YOLOv8n segmentation on camera 1, both on the GPU
- SDKs: `proxy`, `robotics-ai-suite`, `camera`

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites) and two USB RealSense cameras on the host, ideally on separate USB 3 controllers.

```bash
cd src/components/multicam-demo/workshop
workshop launch multicam                    # or multicam-humble; about 11 min
workshop connect multicam/camera:camera     # never auto-connects
workshop run multicam find-cameras          # index, model, serial, USB type
workshop run multicam install               # Python environment and models, a few minutes
workshop run multicam demo-check
workshop connect multicam/robotics-ai-suite:desktop
workshop run multicam demo                  # both cameras side by side; q or Esc to quit
```

## Actions

| Action | Description |
|---|---|
| `find-cameras` | RealSense cameras visible in the workshop, with the index the config uses as `source` |
| `install` | Creates the demo's venv (reusing the suite's OpenVINO, OpenCV, numpy and `pyrealsense2`), installs the Python dependencies from the package's `requirements.txt`, builds its Cython extension, and converts YOLOv8n and YOLOv8n-seg to OpenVINO FP16. Safe to run again |
| `demo` | The demo window with both cameras and their detections or masks. `q` or Esc to quit |
| `demo-check [seconds]` | Headless for 30 s by default. PASS if every camera in the config reaches 10 inference frames/s |

To use more cameras or other models, add entries to `config/two-cameras.json` (up to four, `source` is the index from `find-cameras`). `install` converts `yolov8n` and `yolov8n-seg`; edit the model list in the action for others.

**Run `install` once after every launch or rebuild.** The package doesn't set up the demo's Python environment or models, and a rebuild discards them. `src/workshops/scripts/demo.sh multicam` runs `install` itself when the models are missing, but that adds a few minutes to the first demo; run it beforehand to avoid the wait.

Only one workshop can stream from a camera at a time, so run `multicam` and `multicam-humble` (or `realsense`) one after another.

## Results

Two D435i (firmware 5.17.3.10), each on its own USB 3 bus, on Panther Lake (Arc B390), `demo-check` over 30 s:

| Workshop | Camera | Model | Camera fps | Inference fps | GPU time per frame |
|---|---|---|---|---|---|
| multicam (Jazzy) | 0 | YOLOv8n | 29.8 | 29.1 (880 frames) | 12-15 ms |
| multicam (Jazzy) | 1 | YOLOv8n-seg | 29.4 | 29.4 (877 frames) | 13-15 ms |
| multicam-humble | 0 | YOLOv8n | 29.8 | 29.3 (891 frames) | 12-15 ms |
| multicam-humble | 1 | YOLOv8n-seg | 29.4 | 29.5 (887 frames) | 13-15 ms |

Both pipelines keep up with the cameras' 30 fps.

## Known issues

The package installs only the demo's sources. Everything below is handled by the `install` action.

- **No setup at install time.** The upstream README says installing the package also installs the Python dependencies and models; the package's postinst (Jazzy and Humble) only changes permissions. `install` does those steps.
- **Camera configs missing.** The README names `config_ros2_v4l2_rs-color-0_*.js` files that are in neither the package nor the repository. This workshop ships its own config.
- **Python version pins.** `ultralytics` is pinned to 8.0.0 in `requirements.txt`, `>=8.0.20` in `pyproject.toml` and 8.0.43 in the Dockerfile; `install` uses `>=8.0.20,<9`. `dpctl` is unpinned, but `dpnp` 0.16 needs `dpctl` 0.19 (later versions dropped `dpctl.tensor`) and the oneAPI 2025.3 SYCL runtime (`libsycl.so.8`); without pins, pip installs the 2026.1 runtime and the import fails.
- **Library path.** The `dpnp` wheel's RPATH points at its build machine, so its SYCL and MKL libraries in the venv aren't found. `install` adds the venv's `lib` directory to `LD_LIBRARY_PATH` in the venv's `activate` script.
- **Cython build on Python 3.10 (Humble).** The venv's numpy 1.26 hooks the compiler through `numpy.distutils`, which imports `distutils.msvccompiler`; setuptools 74+ removed it, and `requirements.txt` requires setuptools 78.1.1+. On Python < 3.12, `install` builds with `SETUPTOOLS_USE_DISTUTILS=stdlib`.
- **Crash or hang on exit.** The launcher segfaults while exiting, after printing its summary, with any model, camera count or device (CPU included); on Humble it sometimes hangs there instead, with one thread blocked writing to an internal pipe. Results aren't affected; `demo-check` judges the summary instead of the exit code and kills the launcher 60 s after its run time.
- **Large Python environment.** `install` uses PyTorch's CPU-only wheel (torch is only used to convert models); the default wheel pulls several GB of CUDA libraries.
