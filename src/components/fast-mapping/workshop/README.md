<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# fast-mapping

[FastMapping](https://developer.robotics.intel.com/development-stack/components/optimized_solutions/run-fastmapping-algorithm/index.html) builds a 3D voxel map and a 2D occupancy grid from depth images. The workshop replays the recorded bag shipped in `ros-${ROS_DISTRO}-bagfile-spinning` (about 12 s), so no camera is needed.

- Definitions: [fast-mapping.yaml](.workshop/fast-mapping.yaml) (Jazzy), [fast-mapping-humble.yaml](.workshop/fast-mapping-humble.yaml) (Humble)
- Packages: [apt-packages.txt](apt-packages.txt)

## Get started with Workshop

Requires the [prerequisites](../../../workshops/docs/getting-started.md#prerequisites).

```bash
cd src/components/fast-mapping/workshop
workshop launch fast-mapping                # or fast-mapping-humble
workshop run fast-mapping bag-replay-headless
workshop connect fast-mapping/robotics-ai-suite:desktop
workshop run fast-mapping bag-replay        # with RViz
```

## Actions

| Action | Description |
|---|---|
| `bag-replay` | Documented tutorial launch: `fast_mapping_node`, RViz2 (`/world/fused_map`, `/world/map`) and the bag |
| `bag-replay-headless` | The same node and bag without RViz. Prints map update counts, voxel count and occupancy grid size |

## Results

| Distro | `bag-replay-headless` |
|---|---|
| Jazzy | 172 map updates (about 30k voxels), 512x512 occupancy grid, all 46 frames |
| Humble | 165 map updates, all 46 frames |
