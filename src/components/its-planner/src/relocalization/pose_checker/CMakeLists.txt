# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# https://docs.ros.org/en/humble/The-ROS2-Project/Contributing/Code-Style-Language-Versions.html#id5
cmake_minimum_required(VERSION 3.14.4)
project(pose_checker)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
include(ItsDebianPackaging)

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-fno-strict-overflow -fno-delete-null-pointer-checks")

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(nav2_core REQUIRED)
find_package(its_planner REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_amcl REQUIRED)
find_package(std_srvs REQUIRED)

set(library_name pose_checker)

set(dependencies
  rclcpp
  nav2_msgs
  nav2_core
  its_planner
  nav2_costmap_2d
  geometry_msgs
  nav2_amcl
  std_srvs
)

add_executable(pose_checker src/pose_checker.cpp)

ament_target_dependencies(${library_name}
  ${dependencies}
)

install(TARGETS
  pose_checker
  DESTINATION lib/${PROJECT_NAME})

its_enable_debian_package(
  PACKAGE_BASENAME "its-pose-checker"
  PACKAGE_DESCRIPTION "Pose-checking package for the relocalization module"
  CHANGELOG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../${ROS_DISTRO}/debian/changelog"
  BUILD_DEPENDS
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-rcl"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-nav2-msgs"
    "ros-${ROS_DISTRO}-nav2-costmap-2d"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-nav2-amcl"
    "ros-${ROS_DISTRO}-std-srvs"
    "ros-${ROS_DISTRO}-tf2-ros"
    "ros-${ROS_DISTRO}-nav2-core"
  PACKAGE_DEPENDS
    "ros-${ROS_DISTRO}-rcl"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-nav2-msgs"
    "ros-${ROS_DISTRO}-its-planner"
    "ros-${ROS_DISTRO}-nav2-costmap-2d"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-nav2-amcl"
    "ros-${ROS_DISTRO}-std-srvs"
    "ros-${ROS_DISTRO}-tf2-ros"
    "ros-${ROS_DISTRO}-nav2-core"
    "ros-${ROS_DISTRO}-robotics-sdk-license"
)

ament_package()
