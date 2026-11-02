<!--
Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Enable Collaborative Visual SLAM Framework on ROS2 Navigation

The Collaborative Visual SLAM Framework is available from the [CSLAM repository](https://github.com/open-edge-platform/edge-ai-suites).

Follow these instructions to enable the Collaborative Visual SLAM Framework with the ROS 2 Navigation package.

1. Get the Collaborative Visual SLAM code

    `git clone --recursive https://github.com/open-edge-platform/edge-ai-suites -b main`

2. Build collaborative VSLAM, `nav2_bringup`, and the ITS planner.

    `source /opt/ros/humble/setup.bash`

    `colcon build`

    `source install/setup.bash`

3. Run collaborative SLAM with ROS 2 Navigation.

    To run in the localization mode: `./run_collab.sh localization`

    To run in the mapping mode: `./run_collab.sh mapping`
