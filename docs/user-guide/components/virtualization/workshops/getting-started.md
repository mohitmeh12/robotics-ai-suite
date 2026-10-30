<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Getting Started with Workshops

This guide takes you from an Ubuntu 24.04 host to a running workshop with the Robotics AI Suite, with the Intel® GPU and Intel® NPU passed through. It takes about 30 minutes, most of it is the first suite installation.

You will:

1. Install LXD and the Workshop CLI.
2. Check that the GPU and NPU work in a container with the `accel-check` workshop.
3. Launch the `object-detection` workshop and run inference on the CPU, GPU and NPU.

## Prerequisites

- An Ubuntu 24.04 host with an Intel® GPU and/or Intel® NPU. The `xe` (or `i915`) and `intel_vpu` kernel drivers must be loaded. Check that the device nodes exist:

  ```bash
  ls /dev/dri /dev/accel
  ```

  You should see `card*` and `renderD*` in `/dev/dri`, and `accel0` in `/dev/accel`.

- A local desktop session (X11 or Wayland) for the steps that open RViz or Gazebo. Headless steps don't need one.
- About 5 GiB of disk space per workshop with the full suite, more while it installs.

## Install LXD and Workshop

1. If the host is behind an HTTP proxy, give snapd the proxy first:

   ```bash
   sudo snap set system proxy.http="$http_proxy" proxy.https="$https_proxy"
   ```

2. Install and initialize LXD, then install Workshop:

   ```bash
   sudo snap install lxd --channel=6/stable
   sudo lxd init --auto
   sudo snap install workshop --classic
   ```

3. Add your user to the `lxd` group, then log out and back in:

   ```bash
   sudo usermod -aG lxd $USER
   ```

4. Make room in Workshop's storage pool. Workshop keeps its containers in its own LXD storage pool, `workshop`, separate from the `default` pool that `lxd init` creates. Its default size (28.6 GiB on our test host) fills up with two or three suite workshops installing at once, and launches then fail with `no space left on device`. Once the pool exists (check with `lxc storage list`), grow it:

   ```bash
   lxc storage set workshop size=150GiB
   ```

## Get the workshop definitions

Clone the Robotics AI Suite repository. No submodules are needed.

```bash
git clone https://github.com/open-edge-platform/robotics-ai-suite.git
cd robotics-ai-suite
```

Each component's workshop is in `src/components/<component>/workshop/`. Run `workshop` commands from that directory; anywhere else they fail with `not a project`.

## Configure the proxy

Skip this step on a direct internet connection.

Behind an HTTP proxy, pass the host's `http_proxy`, `https_proxy` and `no_proxy` to the workshops before the first launch:

```bash
src/workshops/scripts/configure-proxy.sh
```

The script writes `src/workshops/sdks/proxy/hooks/proxy.env`, which every workshop reads while it is built to configure apt, `/etc/environment`, curl, wget, cargo and dirmngr. The file is not tracked by git. To write it by hand, copy `proxy.env.example` from the same directory and edit it.

If the host's proxy changes later, run the script again, then rebuild each workshop with `workshop refresh <name>`.

## Check the GPU and NPU

The `accel-check` workshop installs only the GPU and NPU user-space drivers, without ROS, so it builds in a few minutes. Use it to confirm that devices reach a container on a new host.

1. Launch the workshop:

   ```bash
   cd src/workshops/accel-check
   workshop launch accel-check
   ```

2. Connect the NPU. The GPU connects automatically; the NPU never does:

   ```bash
   workshop connect accel-check/intel-accel:npu :custom-device
   ```

3. List the devices through Level Zero:

   ```bash
   workshop run accel-check check-level-zero
   ```

   The output lists both the GPU and the NPU.

Other checks in this workshop:

| Action | What it checks |
|---|---|
| `check-devices` | `/dev/dri`, `/dev/accel`, and group membership |
| `check-open` | Opens each device node and reads its kernel driver |
| `check-level-zero` | Lists the GPU and NPU through Level Zero |
| `check-gpu` | `clinfo` and `vainfo` |
| `check-qmassa` | GPU statistics for the `xe` driver (replaces `intel_gpu_top`) |

## Launch a Robotics AI Suite workshop

The `object-detection` workshop installs the full suite with the official robotics-installer, so the first launch takes 10 to 30 minutes.

1. Launch the workshop and connect the NPU:

   ```bash
   cd ../../components/object-detection/workshop
   workshop launch object-detection
   workshop connect object-detection/robotics-ai-suite:npu :custom-device
   ```

2. Measure inference latency on each device:

   ```bash
   workshop run object-detection benchmark
   ```

3. Connect the desktop and open the tutorial in RViz:

   ```bash
   workshop connect object-detection/robotics-ai-suite:desktop
   workshop run object-detection ros-object-detection-gpu
   ```

   Press Ctrl+C in the terminal to stop it.

Continue with the [Object Detection](object-detection.md) tutorial, which also covers the NPU.

## Everyday commands

| Command | What it does |
|---|---|
| `workshop launch <name>` | Creates a workshop from its definition. Use it only for a new workshop |
| `workshop start <name>` / `workshop stop <name>` | Starts or stops an existing workshop |
| `workshop refresh <name>` | Applies changes to the definition or its SDKs. Rebuilds the container if an SDK changed; see [What a refresh keeps](how-it-works.md#what-a-refresh-keeps) |
| `workshop actions <name>` | Lists the workshop's actions |
| `workshop run <name> <action> [args]` | Runs an action |
| `workshop shell <name>` | Opens an interactive shell in the workshop |
| `workshop exec <name> -- <command>` | Runs one command in the workshop |
| `workshop connections <name>` | Shows which interfaces are connected |
| `workshop connect <name>/<sdk>:<plug>` | Connects an interface, such as the NPU or the desktop |
| `workshop remove <name>` | Deletes the workshop |
| `workshop warnings` | Shows Workshop's warnings; `workshop okay` acknowledges them |

:::{note}
Stop an action with Ctrl+C in its terminal. Closing the terminal or killing only the host-side `workshop run` leaves the processes running in the container; see [Troubleshooting](troubleshooting.md#ros-processes-keep-running-after-workshop-run-is-killed).
:::

## Use Humble instead of Jazzy

Every ROS 2 workshop also has a Humble variant on Ubuntu 22.04. Use `<name>-humble` in place of `<name>` in every command, for example `workshop launch object-detection-humble`. Jazzy and Humble workshops can run at the same time. See [ROS Distributions and Interoperability](ros-distros.md).

## Next steps

- Pick a tutorial from the [list of workshops](index.md#available-workshops).
- Learn how the definitions are put together in [How Workshops Work](how-it-works.md).
- If something doesn't work, see [Troubleshooting](troubleshooting.md).
