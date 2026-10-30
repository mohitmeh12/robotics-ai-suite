<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# wandering

The [Wandering](https://developer.robotics.intel.com/development-stack/software_references/amr/simulation/wandering_sim/index.html) app explores and maps its surroundings autonomously with Nav2, SLAM and ADBScan obstacle detection, in simulation.

- Definitions: [wandering.yaml](.workshop/wandering.yaml) (Jazzy), [wandering-humble.yaml](.workshop/wandering-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- Monitor: [explore_monitor.py](explore_monitor.py)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites) and a local desktop session.

```bash
cd src/components/wandering/workshop
workshop launch wandering                   # or wandering-humble
workshop connect wandering/robotics-ai-suite:desktop
workshop run wandering gazebo-tutorial
```

## Actions

| Action | Description |
|---|---|
| `simulation` | `wandering_bringup wandering_sim.launch.py`: simulated RGB-D robot, Nav2, SLAM, ADBScan, two RViz windows |
| `gazebo-tutorial` | `wandering_gazebo_tutorial wandering_gazebo.launch.py`: TurtleBot3 Waffle in Gazebo Sim, RTAB-Map, Nav2, RViz |

Both actions stop on their own. `explore_monitor.py` follows the explorer's `Unexplored: N%` log line:

- **PASS (exit 0):** unexplored space shrank and then made no new low for 2 minutes, which means the arena is mapped. A point of jitter up and down doesn't reset the timer.
- **FAIL (exit 1):** a launched process died, unexplored space never shrank, or 20 minutes passed.

The launch output goes to `/tmp/wandering_launch.log` in the workshop. To stop an action, press Ctrl+C in its terminal; killing only the host-side `workshop run` leaves the ROS processes running (see [Troubleshooting](../../../workshops/docs/troubleshooting.md#ros-processes-keep-running-after-workshop-run-is-killed)).

## Results

| Distro | `gazebo-tutorial` |
|---|---|
| Jazzy | PASS in about 4 minutes (unexplored 63% -> 37%) |
| Humble | PASS in 183 s (unexplored 64% -> 38%) |

## Known issues

- **`simulation` (Jazzy):** `wandering_sim.launch.py` (`ros-jazzy-wandering-bringup` 0.2.0-1) waits for the SLAM map with `ros2 topic echo /map --once`, without a message type or transient-local QoS. The probe exits before RTAB-Map advertises `/map`, or misses the latched map, so Nav2 and the explorer never start. The `simulation` action launches a copy with the type and QoS added. With that, Nav2 starts and the robot explores. In our runs it still stopped within a few minutes: either the planner found no path from its position, or `rtabmap` (`ros-jazzy-rtabmap-slam` 0.23.7) segfaulted and the robot lost its pose. `gazebo-tutorial` is not affected.
- **Humble:** the launch uses Gazebo Classic and replaces `GAZEBO_MODEL_PATH`, so Gazebo fetches `ground_plane` and `sun` from the online model database through the proxy. Without that (for example with `GAZEBO_MODEL_DATABASE_URI=` set empty), the robot falls through the world. Rendering uses `llvmpipe` on Humble; see [Humble variants](../../../workshops/docs/humble-and-interop.md#known-issues-on-humble).
