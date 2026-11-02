# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.8)
project(follow_me_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME} "msg/GestureCategory.msg" "msg/AudioCommand.msg")
# if(BUILD_TESTING)
# find_package(ament_lint_auto REQUIRED) the following line skips the linter
# which checks for copyrights comment the line when a copyright and license is
# added to all source files set(ament_cmake_copyright_FOUND TRUE) the following
# line skips cpplint (only works in a git repo) comment the line when this
# package is in a git repo and when a copyright and license is added to all
# source files set(ament_cmake_cpplint_FOUND TRUE)
ament_package()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/RosDebianPackage.cmake)
configure_ros_debian_package_with_metadata(
  PACKAGE_NAME "ros-${ROS_DISTRO}-follow-me-interfaces"
  DESCRIPTION "Follow-me interfaces ROS2 package"
  CONTACT "ECI Maintainer <eci-maintainer@intel.com>"
  BUILD_DEPENDS
    "build-essential"
    "cmake"
    "ninja-build"
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-rosidl-default-generators"
    "ros-${ROS_DISTRO}-builtin-interfaces"
  DEPENDS
    "ros-${ROS_DISTRO}-rosidl-default-runtime"
    "ros-${ROS_DISTRO}-builtin-interfaces"
)
