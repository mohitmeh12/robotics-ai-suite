#!/bin/bash
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# Headless multi-camera tutorial: two GPU trackers + UKF on the bundled bag. PASS if both trackers
# publish poses, the fused odometry is produced, the trackers use the GPU, and nothing crashes.
source /opt/ros/*/setup.bash
# libur_adapter_level_zero needs oneAPI's libumf, which isn't on the linker path.
export LD_LIBRARY_PATH=$(ls -d /opt/intel/oneapi/umf/*/lib | tail -1)${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
T=/opt/ros/$ROS_DISTRO/share/collab-slam/tutorial-multi-camera
BAG=/opt/ros/$ROS_DISTRO/share/bagfiles/multi-camera
L=/tmp/cslam-check
rm -rf $L && mkdir -p $L

cleanup() {
  pkill -INT -f "[r]os2 (launch|run|topic)" 2>/dev/null; sleep 2
  pkill -9 -f "[/]opt/ros/$ROS_DISTRO/lib/(univloc|robot_localization|tf2_ros)|[p]lay_rosbag_ros2|[r]os2 (launch|run|topic)" 2>/dev/null
}
trap cleanup EXIT

ros2 run robot_localization ukf_node --ros-args --params-file $T/ukf_config_two_camera.yaml > $L/ukf.log 2>&1 &
ros2 run tf2_ros static_transform_publisher -0.283 0.028 0.368 -0.509 -0.503 0.485 0.502 base_link d455_front_color_optical_frame --ros-args -p use_sim_time:=true > /dev/null 2>&1 &
ros2 run tf2_ros static_transform_publisher 0.213 -0.038 0.354 -0.495 0.509 -0.508 0.488 base_link d455_rear_color_optical_frame --ros-args -p use_sim_time:=true > /dev/null 2>&1 &
for id in 0 2; do
  cam=$([ $id = 0 ] && echo d455_front || echo d455_rear)
  ros2 launch univloc_tracker tracker.launch.py ID:=$id queue_size:=0 gui:=false publish_tf:=false camera:=$cam rviz:=false need_covariance:=true > $L/tracker$id.log 2>&1 &
done
sleep 5
ros2 topic echo /univloc_tracker_0/kf_pose > $L/pose0.txt 2>/dev/null &
ros2 topic echo /univloc_tracker_2/kf_pose > $L/pose2.txt 2>/dev/null &
ros2 topic echo /odometry/filtered > $L/odom.txt 2>/dev/null &
sleep 2

S0=$(date +%s)
python3 $T/play_rosbag_ros2.py $BAG use_sim_time=0 keep_time=0 > $L/bag.log 2>&1
S1=$(date +%s)

# GPU compute-engine cycles per tracker process, from the xe driver's fdinfo.
gpu=0
for p in $(pgrep -f "[u]nivloc_tracker_ros"); do
  c=$(awk '/^drm-cycles-ccs:/{s+=$2} END{print s+0}' /proc/$p/fdinfo/* 2>/dev/null)
  echo "tracker pid $p GPU compute cycles: $c"
  [ "$c" -gt 0 ] && gpu=$((gpu + 1))
done
sleep 3

p0=$(grep -c '^header:' $L/pose0.txt); p2=$(grep -c '^header:' $L/pose2.txt); od=$(grep -c '^header:' $L/odom.txt)
deaths=$(cat $L/tracker0.log $L/tracker2.log $L/ukf.log | grep -c "process has died")
echo "bag replay $((S1 - S0)) s; kf_pose tracker0 $p0, tracker2 $p2; /odometry/filtered $od; trackers on GPU $gpu/2; crashes $deaths"
echo "last fused position: $(grep -A3 'position:' $L/odom.txt | tail -3 | tr -s ' \n' ' ')"

if [ "$p0" -gt 10 ] && [ "$p2" -gt 10 ] && [ "$od" -gt 10 ] && [ "$gpu" -eq 2 ] && [ "$deaths" -eq 0 ]; then
  echo "PASS"
else
  echo "FAIL (logs in $L)"
  exit 1
fi
