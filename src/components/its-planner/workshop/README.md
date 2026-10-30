<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# its-planner

The [ITS path planner](https://developer.robotics.intel.com/development-stack/components/navigation/its-path-planner-plugin/index.html) is a Nav2 global planner plugin. It builds a roadmap from the 2D costmap and does a two-way search on it. The workshop runs it in the TurtleBot3 Gazebo simulation.

- Definitions: [its-planner.yaml](.workshop/its-planner.yaml) (Jazzy), [its-planner-humble.yaml](.workshop/its-planner-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)
- Check: [plan_check.py](plan_check.py)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites).

```bash
cd src/components/its-planner/workshop
workshop launch its-planner                 # or its-planner-humble
workshop run its-planner plan-check         # headless
workshop connect its-planner/robotics-ai-suite:desktop
workshop run its-planner simulation         # with Gazebo and RViz
```

## Actions

| Action | Description |
|---|---|
| `simulation` | Documented launch (`its_differential_launch.py`): TurtleBot3 Waffle in Gazebo, Nav2 with ITS, RViz. Set "2D Pose Estimate" on the robot, then a "Nav2 Goal" |
| `simulation-ackermann` | The same with the Ackermann (Dubins path) variant |
| `plan-check` | Headless: the same Nav2 stack. Sets the spawn pose, requests a path from ITS, and prints its length and endpoints. Returns non-zero if ITS returns no path |

The planner saves its roadmap as `road_map.txt` in the working directory, so the actions run in `~/its_planner`.

## Results

| Distro | `plan-check` |
|---|---|
| Jazzy | 78 poses, 4.07 m, ends on the goal |
| Humble | 81 poses, 4.12 m, ends on the goal |

`its-planner` and `its-planner-humble` were also run side by side on one ROS domain to test Humble <-> Jazzy interoperability. See [Humble and Jazzy on the same domain](../../../workshops/docs/humble-and-interop.md#humble-and-jazzy-on-the-same-domain).
