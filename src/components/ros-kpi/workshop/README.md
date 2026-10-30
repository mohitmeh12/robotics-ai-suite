<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# benchmark-framework

The [Robotics System Profiler](https://developer.robotics.intel.com/development-stack/components/benchmarking/robotics-system-profiler/installation/index.html) (`ros-${ROS_DISTRO}-benchmark-framework`) collects ROS 2 KPIs and exports them to Grafana through Prometheus. This workshop is the monitor: it runs no workload of its own and watches the other workshops through the profiler's remote mode.

- Definitions: [benchmark-framework.yaml](.workshop/benchmark-framework.yaml) (Jazzy), [benchmark-framework-humble.yaml](.workshop/benchmark-framework-humble.yaml) (Humble), [benchmark-framework-lyrical.yaml](.workshop/benchmark-framework-lyrical.yaml) (Lyrical, [Ubuntu 26.04](#lyrical-on-ubuntu-2604))
- Packages: [apt-packages.txt](apt-packages.txt)
- SDKs: `proxy`, `robotics-ai-suite` (`ros2` for Lyrical), `docker` (Docker and Compose inside the workshop)
- Files: [dashboards_check.py](dashboards_check.py), [authorize-remote.sh](../../../workshops/scripts/authorize-remote.sh)

The package installs the profiler read-only in `/opt/ros/<distro>/benchmarking`. The `install` action copies it to `~/benchmarking`, where sessions are written, then sets up `uv` and runs `uv sync` as documented.

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites) and a second workshop to monitor, here [wandering](../../wandering/workshop/README.md) (launched, with the desktop plug connected).

```bash
cd src/components/ros-kpi/workshop
workshop launch benchmark-framework         # or benchmark-framework-humble
workshop run benchmark-framework install
../../../workshops/scripts/authorize-remote.sh benchmark-framework wandering
(cd ../../wandering/workshop && workshop run wandering gazebo-tutorial) &   # workload on the target
workshop run benchmark-framework monitor-remote wandering 100  # Grafana shows it live
```

Open Grafana at `http://<workshop IP>:30000` (login `admin`/`admin`, the profiler's local default). Get the IP with `workshop exec benchmark-framework -- ip -4 -br addr show eth0`; `hostname -I` lists Docker's `docker0` first.

## Actions

| Action | Description |
|---|---|
| `install` | Copies the profiler to `~/benchmarking`, installs `uv`, runs `uv sync` (a venv with ROS 2 system site-packages) |
| `dashboards` | Grafana (port 30000, `ROS2 KPI Dashboard`) and Prometheus in Docker, fed live from the newest session. Ctrl+C stops the exporter |
| `dashboards-stop` | `docker compose down` |
| `monitor-remote <workshop\|ip> [seconds]` | Profiler remote mode against a paired workshop (default 120 s): Grafana and Prometheus up, live exporter, `monitor_stack.py --remote-ip` |
| `dashboards-check` | Headless: stack up, newest session exported, checks that Prometheus scrapes `ros2-kpi` and Grafana has the dashboard, stack down. Returns non-zero if not |

## Remote monitoring across workshops

The profiler's remote mode watches another machine. Process CPU and memory come over passwordless SSH (`ps` and a small psutil probe), and the ROS graph comes over DDS. Any other workshop can be the target, because all workshops share the `workshopbr0` bridge and ROS domain 0.

`src/workshops/scripts/authorize-remote.sh <monitor> <target>` pairs two workshops from the host. It:

1. Creates the monitor's SSH key and adds it to the target's `authorized_keys`.
2. Copies the probe to `~/ros-kpi/src/` on the target and enables SSH there.
3. Tests the login and records the target's address on the monitor, so `monitor-remote` takes the workshop name.

The script is safe to repeat. Pairing lives in the workshops, so rebuilding a target (for example after an SDK hook change) removes it. `monitor-remote` checks SSH first and, if the login fails, prints the pairing command to run again.

Start the workload before `monitor-remote`. Grafana stays up afterwards; stop it with `dashboards-stop`.

Grafana takes about 5 to 30 s to answer after `dashboards` or `monitor-remote` starts it. The `GF_INSTALL_PLUGINS` line in the profiler's `docker-compose.yml` is removed before each start, because Grafana fails to download plugins without internet access and then stops serving port 30000. The provisioned dashboards need no plugins. The demo prints the link only once Grafana answers.

## Results

Monitoring the wandering `gazebo-tutorial`, Grafana showed 552 topic-rate series and 50 process CPU series live. The top processes were Gazebo Sim (about 110% CPU each for server and GUI) and RViz (about 38%). The session recorded 51 topics.

| Distro | Result |
|---|---|
| Jazzy | `install`, `monitor-remote` against `wandering`, `dashboards-check` passes |
| Humble | `install`, `monitor-remote` against `wandering-humble`, `dashboards-check` passes |
| Lyrical (Ubuntu 26.04) | `build-package`, `install`, local `monitor_stack.py` (10 Hz test topic measured at 10.0 Hz), `dashboards-check` passes |

`dashboards-check` needs a recorded session, so run `monitor-remote` against a paired workshop first.

## Lyrical on Ubuntu 26.04

[benchmark-framework-lyrical.yaml](.workshop/benchmark-framework-lyrical.yaml) runs the profiler on `ubuntu@26.04` with ROS 2 Lyrical. The suite installer, the AMR repo and the OpenVINO apt repo have no 26.04 builds yet, so it uses the `ros2` SDK (Lyrical from packages.ros.org, see [Architecture](../../../workshops/docs/architecture.md#sdks)) instead of `robotics-ai-suite`. No repo publishes `ros-lyrical-benchmark-framework` yet, so the SDK skips it and the `build-package` action builds it from the `ros-kpi` source in this repository, streamed in from the host. The profiler needs no OpenVINO or oneAPI.

```bash
cd src/components/ros-kpi/workshop
workshop launch benchmark-framework-lyrical                     # about 3 minutes
git -C "$(git rev-parse --show-toplevel)" archive HEAD:src/components/ros-kpi | workshop exec benchmark-framework-lyrical -- \
  sh -c 'rm -rf ~/benchmark-framework-src && mkdir ~/benchmark-framework-src && tar -x -C ~/benchmark-framework-src'
workshop run benchmark-framework-lyrical build-package          # make package ROS_DISTRO=lyrical, then installs the .deb
workshop run benchmark-framework-lyrical install
```

`build-package` reuses the Jazzy changelog for `lyrical/debian/changelog` when the source has none (`src/components/ros-kpi` has only `humble/` and `jazzy/`). The rest is the same as Jazzy: the 26.04 archive has every runtime dependency, and `uv sync` resolves on Python 3.14. The workshop has Docker 29 and Compose 2.40, and it sees the host GPU through OpenCL.
