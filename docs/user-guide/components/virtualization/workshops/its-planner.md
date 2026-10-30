<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# ITS Path Planner Workshop

This tutorial runs the [ITS path planner](../../navigation/its-path-planner-plugin.md) in a workshop. ITS is a Nav2 global planner plugin: it builds a roadmap from the 2D costmap and searches it from both ends. The workshop runs it in the TurtleBot3 Gazebo simulation, so no robot is needed.

You will:

1. Request a path from ITS without a display, to check the stack.
2. Plan and drive to goals interactively in Gazebo and RViz.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- A local desktop session for the interactive simulation.

## Launch the workshop

```bash
cd src/components/its-planner/workshop
workshop launch its-planner
```

## Check the planner

Run the headless check:

```bash
workshop run its-planner plan-check
```

It starts the Nav2 stack with ITS, sets the robot's spawn pose, requests a path, and prints its length and end points. On Jazzy, ITS returns a path of 78 poses and 4.07 m that ends on the goal. The action returns non-zero if ITS returns no path.

## Plan interactively

1. Connect the desktop and start the simulation:

   ```bash
   workshop connect its-planner/robotics-ai-suite:desktop
   workshop run its-planner simulation
   ```

   Gazebo and RViz open with a TurtleBot3 Waffle.

2. In RViz, click **2D Pose Estimate** and set the robot's pose on the map.
3. Click **Nav2 Goal** and set a goal. ITS plans a path, and the robot drives to the goal.

Press Ctrl+C in the terminal to stop the simulation.

To try the Ackermann variant, which plans with Dubins paths, run `simulation-ackermann` instead.

## Actions

| Action | Description |
|---|---|
| `simulation` | The documented `its_differential_launch.py`: TurtleBot3 Waffle in Gazebo, Nav2 with ITS, and RViz |
| `simulation-ackermann` | The same, with the Ackermann (Dubins path) variant |
| `plan-check` | Headless: the same Nav2 stack. Requests one path and prints it. Returns non-zero if ITS returns no path |

The planner saves its roadmap as `road_map.txt` in the working directory, so the actions run in `~/its_planner`.

## Humble

Use `its-planner-humble` in every command. `plan-check` returns 81 poses and 4.12 m, ending on the goal.

`its-planner` and `its-planner-humble` were also run side by side on one ROS domain to test Humble and Jazzy interoperability. See [Humble and Jazzy on the same domain](ros-distros.md#humble-and-jazzy-on-the-same-domain).

## Source

The definitions, package list and check script are in [`src/components/its-planner/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/its-planner/workshop).
