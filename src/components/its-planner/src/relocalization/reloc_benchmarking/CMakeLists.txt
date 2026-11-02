# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# https://docs.ros.org/en/humble/The-ROS2-Project/Contributing/Code-Style-Language-Versions.html#id5
cmake_minimum_required(VERSION 3.14.4)
project(reloc_benchmarking)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
include(ItsDebianPackaging)

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-fno-strict-overflow -fno-delete-null-pointer-checks")

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_core REQUIRED)
find_package(std_srvs REQUIRED)

set(library_name reloc_benchmarking)

set(dependencies
  rclcpp
  geometry_msgs
  nav2_core
  std_srvs
)

add_executable(reloc_benchmarking src/reloc_benchmarking.cpp)

ament_target_dependencies(${library_name}
  ${dependencies}
)

install(TARGETS
  reloc_benchmarking
  DESTINATION lib/${PROJECT_NAME})

its_enable_debian_package(
  PACKAGE_BASENAME "its-reloc-benchmarking"
  PACKAGE_DESCRIPTION "ITS Planner relocalization benchmarking package"
  CHANGELOG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../${ROS_DISTRO}/debian/changelog"
  BUILD_DEPENDS
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-rcl"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-std-srvs"
  PACKAGE_DEPENDS
    "ros-${ROS_DISTRO}-rcl"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-std-srvs"
    "ros-${ROS_DISTRO}-robotics-sdk-license"
)

ament_package()
