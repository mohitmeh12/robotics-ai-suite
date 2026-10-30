<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Wandering Workshop

This tutorial runs the [Wandering](../../../software_references/amr/simulation/wandering_sim.md) application in a workshop. Wandering explores and maps its surroundings autonomously with Nav2, SLAM and ADBSCAN obstacle detection. Here it runs in simulation, so no robot is needed.

You will launch the workshop, then watch a simulated TurtleBot3 explore and map an arena in Gazebo and RViz. A monitor stops the run when the arena is mapped.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- A local desktop session.

## Launch the workshop

```bash
cd src/components/wandering/workshop
workshop launch wandering
workshop connect wandering/robotics-ai-suite:desktop
```

## Explore and map in Gazebo

Run the Gazebo tutorial:

```bash
workshop run wandering gazebo-tutorial
```

Gazebo Sim and RViz open. A TurtleBot3 Waffle explores the arena with RTAB-Map and Nav2, and the map grows in RViz.

The action stops on its own. A monitor follows the explorer's `Unexplored: N%` log line:

- **PASS** (exit code 0): unexplored space shrank and then made no new low for 2 minutes, which means the arena is mapped. Small changes up and down don't reset the timer.
- **FAIL** (exit code 1): a launched process died, unexplored space never shrank, or 20 minutes passed.

On Jazzy, the run passes in about 4 minutes, with unexplored space going from 63% to 37%.

The launch output is written to `/tmp/wandering_launch.log` in the workshop. To stop early, press Ctrl+C in the terminal.

## Actions

| Action | Description |
|---|---|
| `gazebo-tutorial` | `wandering_gazebo_tutorial wandering_gazebo.launch.py`: TurtleBot3 Waffle in Gazebo Sim, RTAB-Map, Nav2 and RViz |
| `simulation` | `wandering_bringup wandering_sim.launch.py`: a simulated RGB-D robot with Nav2, SLAM, ADBSCAN and two RViz windows. See [Known issues](#known-issues) |

## Humble

Use `wandering-humble` in every command. `gazebo-tutorial` passes in about 3 minutes, with unexplored space going from 64% to 38%.

On Humble the launch uses Gazebo Classic, and RViz and Gazebo render on the CPU with `llvmpipe`; see [Known issues on Humble](ros-distros.md#known-issues-on-humble).

## Monitor the run

To see the KPIs of the run in Grafana, monitor this workshop from the [Robotics System Profiler](benchmark-framework.md) workshop.

## Known issues

- **`simulation` stops early (Jazzy).** `wandering_sim.launch.py` in `ros-jazzy-wandering-bringup` 0.2.0-1 waits for the SLAM map with `ros2 topic echo /map --once`, without a message type or transient-local QoS, so it can miss the map and never start Nav2. The `simulation` action launches a corrected copy. With it, Nav2 starts and the robot explores, but in our runs it still stopped within a few minutes: either the planner found no path, or `rtabmap` (`ros-jazzy-rtabmap-slam` 0.23.7) crashed and the robot lost its pose. Use `gazebo-tutorial`, which is not affected.
- **Gazebo models on Humble.** The Humble launch replaces `GAZEBO_MODEL_PATH`, so Gazebo downloads `ground_plane` and `sun` from the online model database. The workshop needs internet access, through the proxy if there is one. Without the models, the robot falls through the world.

## Source

The definitions, package list and monitor are in [`src/components/wandering/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/wandering/workshop).
