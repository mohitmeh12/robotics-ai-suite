<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# FastMapping Workshop

This tutorial runs [FastMapping](../../optimized_solutions/run-fastmapping-algorithm.md) in a workshop. FastMapping builds a 3D voxel map and a 2D occupancy grid from depth images. The workshop replays the recorded depth bag from `ros-${ROS_DISTRO}-bagfile-spinning` (about 12 seconds), so no camera is needed.

## Prerequisites

- A host set up as described in [Getting Started](getting-started.md).
- A local desktop session for the RViz step.

## Launch the workshop

```bash
cd src/components/fast-mapping/workshop
workshop launch fast-mapping
```

## Build a map without a display

```bash
workshop run fast-mapping bag-replay-headless
```

The action runs `fast_mapping_node` on one pass of the bag and prints the number of map updates, the voxel count and the occupancy grid size. On Jazzy it reports 172 map updates (about 30,000 voxels) and a 512 x 512 occupancy grid, with all 46 frames processed.

## View the map in RViz

```bash
workshop connect fast-mapping/robotics-ai-suite:desktop
workshop run fast-mapping bag-replay
```

RViz shows the 3D map on `/world/fused_map` and the 2D occupancy grid on `/world/map` as the bag plays. Press Ctrl+C in the terminal to stop.

## Actions

| Action | Description |
|---|---|
| `bag-replay` | The documented tutorial launch: `fast_mapping_node`, RViz (`/world/fused_map`, `/world/map`) and the bag |
| `bag-replay-headless` | The same node and bag without RViz. Prints map update counts, voxel count and occupancy grid size |

## Humble

Use `fast-mapping-humble` in every command. `bag-replay-headless` reports 165 map updates, with all 46 frames processed.

## Source

The definitions and package list are in [`src/components/fast-mapping/workshop/`](https://github.com/open-edge-platform/robotics-ai-suite/tree/main/src/components/fast-mapping/workshop).
