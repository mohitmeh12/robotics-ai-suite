<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Robotics System Profiler Workshop

This tutorial runs the [Robotics System Profiler](../../benchmarking/robotics-system-profiler/index.md) (`ros-${ROS_DISTRO}-benchmark-framework`) in a workshop. The profiler collects ROS 2 KPIs and shows them in Grafana through Prometheus. This workshop is a monitor: it runs no workload of its own, and watches another workshop with the profiler's remote mode.

You will pair the profiler workshop with the [Wandering](wandering.md) workshop, run the Wandering simulation, and watch its topic rates and process CPU usage live in Grafana.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- The [Wandering](wandering.md) workshop, launched and with the desktop connected. Any other workshop works as the target as well.

## Launch and set up the workshop

1. Launch the workshop:

   ```bash
   cd src/components/ros-kpi/workshop
   workshop launch benchmark-framework
   ```

2. Set up the profiler:

   ```bash
   workshop run benchmark-framework install
   ```

   The package installs the profiler read-only in `/opt/ros/<distro>/benchmarking`. `install` copies it to `~/benchmarking`, where sessions are written, installs `uv`, and runs `uv sync` as the profiler's documentation describes.

## Pair the workshops

The profiler's remote mode reads process CPU and memory usage from the target over passwordless SSH, and the ROS graph over DDS. Pair the profiler workshop with the target from the host:

```bash
../../../workshops/scripts/authorize-remote.sh benchmark-framework wandering
```

The script:

1. Creates the monitor's SSH key and adds it to the target's `authorized_keys`.
2. Copies the profiler's process probe to `~/ros-kpi/src/` on the target and enables SSH there.
3. Tests the login and records the target's address on the monitor, so that `monitor-remote` accepts the workshop name.

It is safe to run again. Pairing lives inside the workshops, so a rebuild of either workshop removes it; run the script again afterwards.

## Monitor the workload

1. Start the workload on the target, in the background:

   ```bash
   (cd ../../wandering/workshop && workshop run wandering gazebo-tutorial) &
   ```

2. Monitor it for 100 seconds:

   ```bash
   workshop run benchmark-framework monitor-remote wandering 100
   ```

   The action starts Grafana and Prometheus, checks the SSH login, and runs the profiler against the target. If the login fails, it prints the pairing command to run again.

3. Find the workshop's IP address:

   ```bash
   workshop exec benchmark-framework -- ip -4 -br addr show eth0
   ```

   Use this address; `hostname -I` lists Docker's `docker0` address first.

4. Open Grafana at `http://<workshop IP>:30000` and log in with `admin` / `admin`, the profiler's local default. Open the **ROS2 KPI Dashboard**. Grafana takes 5 to 30 seconds to answer after it starts.

While monitoring the Wandering simulation, Grafana showed 552 topic-rate series and 50 process CPU series live. The top processes were the Gazebo Sim server and GUI at about 110% CPU each, and RViz at about 38%.

Grafana keeps running after `monitor-remote` ends. Stop it with:

```bash
workshop run benchmark-framework dashboards-stop
```

## Actions

| Action | Description |
|---|---|
| `install` | Copies the profiler to `~/benchmarking`, installs `uv`, and runs `uv sync` (a virtual environment with the ROS 2 system site-packages) |
| `monitor-remote <workshop\|ip> [seconds]` | Remote mode against a paired workshop, for 120 seconds by default: Grafana and Prometheus up, live exporter, `monitor_stack.py --remote-ip` |
| `dashboards` | Grafana (port 30000) and Prometheus in Docker, fed live from the newest session. Ctrl+C stops the exporter |
| `dashboards-stop` | Stops Grafana and Prometheus (`docker compose down`) |
| `dashboards-check` | Headless: starts the stack, exports the newest session, checks that Prometheus scrapes `ros2-kpi` and Grafana has the dashboard, and stops the stack. Returns non-zero on failure |

`dashboards-check` needs a recorded session, so run `monitor-remote` first.

Grafana and Prometheus run in Docker inside the workshop. Before each start, the actions remove the `GF_INSTALL_PLUGINS` line from the profiler's `docker-compose.yml`, because Grafana stops serving when it can't download plugins without internet access. The provided dashboards need no plugins.

## Humble

Use `benchmark-framework-humble` in every command, and pair it with `wandering-humble`. `install`, `monitor-remote` and `dashboards-check` work as on Jazzy.

## Ubuntu 26.04 and ROS 2 Lyrical

`benchmark-framework-lyrical` runs the profiler on Ubuntu 26.04 with ROS 2 Lyrical. robotics-installer and Intel®'s apt repositories don't support Ubuntu 26.04 yet, so this workshop installs ROS 2 from packages.ros.org, and builds the profiler package from the `ros-kpi` source in the repository. The profiler needs neither OpenVINO™ nor oneAPI.

1. Launch the workshop. It takes about 3 minutes:

   ```bash
   cd src/components/ros-kpi/workshop
   workshop launch benchmark-framework-lyrical
   ```

2. Copy the profiler source from your clone into the workshop:

   ```bash
   git -C "$(git rev-parse --show-toplevel)" archive HEAD:src/components/ros-kpi | \
     workshop exec benchmark-framework-lyrical -- \
     sh -c 'rm -rf ~/benchmark-framework-src && mkdir ~/benchmark-framework-src && tar -x -C ~/benchmark-framework-src'
   ```

3. Build and install the package, then set up the profiler:

   ```bash
   workshop run benchmark-framework-lyrical build-package
   workshop run benchmark-framework-lyrical install
   ```

The other actions work as on Jazzy. In testing, a local `monitor_stack.py` measured a 10 Hz test topic at 10.0 Hz, and `dashboards-check` passed.

## Source

The definitions, package list and check script are in [`src/components/ros-kpi/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/ros-kpi/workshop). The pairing script is [`src/workshops/scripts/authorize-remote.sh`](https://github.com/open-edge-platform/robotics-ai-suite/blob/main/src/workshops/scripts/authorize-remote.sh).
