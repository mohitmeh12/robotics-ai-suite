<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# accel-check

A minimal workshop that checks GPU and NPU passthrough into the container, without ROS or OpenVINO. Run it first on a new host.

- Definition: [accel-check/workshop.yaml](workshop.yaml) (Ubuntu 24.04 only)
- SDKs: `proxy`, `intel-accel` (gpu and npu plugs; GPU user-space drivers from the kobuk-team PPA, NPU driver from the AMR repo; `clinfo`, `vainfo`, `qmassa`)

## Get started with Workshop

Requires the [prerequisites](../docs/getting-started.md#prerequisites).

```bash
cd src/workshops/accel-check
workshop launch accel-check
workshop connect accel-check/intel-accel:npu :custom-device   # the NPU never auto-connects
workshop run accel-check check-level-zero
```

## Actions

| Action | Description |
|---|---|
| `check-devices` | `/dev/dri`, `/dev/accel`, and group membership |
| `check-open` | Opens each device node and reads its kernel driver |
| `check-level-zero` | Lists the GPU and NPU through Level Zero |
| `check-gpu` | `clinfo`, `vainfo` |
| `check-qmassa` | GPU stats for the `xe` driver (replaces `intel_gpu_top`) |

## Results

On Panther Lake (Core Ultra X7 358H, Arc B390, `xe` driver), all actions succeed, and Level Zero lists both the GPU and the NPU. For inference performance inside a workshop compared with the host, see [object-detection](../../components/object-detection/workshop/README.md#results).

If the workshop fails to start after a host reboot, or the NPU is missing, see [Troubleshooting](../docs/troubleshooting.md).
