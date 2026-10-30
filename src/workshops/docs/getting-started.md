<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Getting started

This guide takes you from a fresh Ubuntu host to a running workshop with the Intel GPU and NPU passed through. For how the pieces fit together, see [Architecture](architecture.md).

## Prerequisites

- Ubuntu 24.04 host with an Intel GPU and/or NPU, and the `xe` (or `i915`) and `intel_vpu` kernel drivers loaded:

  ```bash
  ls /dev/dri /dev/accel      # card*, renderD*, accel0
  ```

- LXD and the Workshop CLI. Behind an HTTP proxy, give snapd the host's proxy first:

  ```bash
  sudo snap set system proxy.http="$http_proxy" proxy.https="$https_proxy"   # only behind a proxy
  sudo snap install lxd --channel=6/stable
  sudo lxd init --auto
  sudo snap install workshop --classic
  sudo usermod -aG lxd $USER   # then log out and back in
  ```

- Enough space in Workshop's LXD storage pool. Workshop keeps its containers in its own ZFS pool, `workshop` (a loop file, 28.6 GiB on our host), separate from the `default` pool that `lxd init` creates. Each workshop with the full Robotics AI Suite uses about 5 GiB once installed; two or three installs running at once can fill the default size (`no space left on device`). Grow it once it exists (`lxc storage list`):

  ```bash
  lxc storage set workshop size=150GiB
  ```

- A local desktop session (X11 or Wayland) for actions that open RViz or Gazebo. Headless actions don't need one.

- Behind an HTTP proxy, pass it to the workshops before the first launch (see [Proxy](#proxy)).

## Get the repository

```bash
git clone https://github.com/open-edge-platform/robotics-ai-suite.git
cd robotics-ai-suite
```

The workshops need no submodules. Each component's workshop is in `src/components/<component>/workshop/`, and the shared SDKs are in `src/workshops/sdks/`, linked from every `.workshop/` directory. To copy a single workshop elsewhere, use `cp -rL` so the SDKs come along as real files.

## Proxy

Every definition lists the `proxy` SDK first. It reads `src/workshops/sdks/proxy/hooks/proxy.env`, which `src/workshops/scripts/configure-proxy.sh` writes from the host's `http_proxy`, `https_proxy` and `no_proxy`. The file isn't tracked. Without it the SDK changes nothing, so on a direct connection there is nothing to do.

```bash
src/workshops/scripts/configure-proxy.sh       # writes proxy.env, or removes it when no proxy is set
```

To write the file by hand instead, copy [proxy.env.example](../sdks/proxy/hooks/proxy.env.example) to `proxy.env` in the same directory and set your values.

Inside the workshop it configures apt, `/etc/environment`, curl, wget, cargo and dirmngr. Run the script again after the host's proxy changes, then `workshop refresh <name>` to rebuild existing workshops with it.

## First workshop: check the GPU and NPU

[accel-check](../accel-check/README.md) installs only the GPU and NPU drivers, without ROS:

```bash
cd src/workshops/accel-check
workshop launch accel-check
workshop connect accel-check/intel-accel:npu :custom-device   # the NPU never auto-connects
workshop run accel-check check-level-zero                     # lists the GPU and the NPU
```

## Second workshop: the Robotics AI Suite

[object-detection](../../components/object-detection/workshop/README.md) installs the full suite with the official installer, so the first launch takes 10-30 minutes:

```bash
cd ../../components/object-detection/workshop
workshop launch object-detection
workshop connect object-detection/robotics-ai-suite:npu :custom-device
workshop run object-detection benchmark                       # inference latency on CPU, GPU, NPU
workshop connect object-detection/robotics-ai-suite:desktop
workshop run object-detection ros-object-detection-gpu        # RViz window with detections
```

Every ROS 2 component workshop (all except accel-check and ros1-bridge) also has a Humble variant on Ubuntu 22.04: use `<name>-humble` in place of `<name>`. See [Humble and interoperability](humble-and-interop.md).

## Everyday commands

Run these in the workshop's directory (for example `src/components/object-detection/workshop/`); elsewhere they fail with `not a project`.

| Command | What it does |
|---|---|
| `workshop launch <name>` | Creates a workshop from its definition. Only for a new workshop |
| `workshop start <name>` / `stop <name>` | Starts or stops an existing workshop |
| `workshop refresh <name>` | Applies changes to the definition or SDKs. Rebuilds the container if an SDK changed (see [Architecture](architecture.md#what-a-refresh-keeps)) |
| `workshop actions <name>` | Lists the workshop's actions |
| `workshop run <name> <action> [args]` | Runs an action |
| `workshop shell <name>` | Interactive shell in the workshop |
| `workshop exec <name> -- <command>` | Runs one command in the workshop |
| `workshop connections <name>` | Shows which plugs are connected |
| `workshop connect <name>/<sdk>:<plug>` | Connects a plug, for example the NPU or the desktop |
| `workshop remove <name>` | Deletes the workshop |
| `workshop warnings` | Shows Workshop's warnings (`workshop okay` acknowledges them) |

Stop an action with Ctrl+C in its terminal. Killing only the host-side `workshop run` leaves the processes running in the container.

## Next steps

- Pick a workshop from the [index](../README.md#workshops).
- If something doesn't work, see [Troubleshooting](troubleshooting.md).
- To add a workshop for another component, see [Adding a workshop](architecture.md#adding-a-workshop).
