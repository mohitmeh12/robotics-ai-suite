# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.8)
project(text_to_speech_pkg)

# Default to C++17
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(follow_me_interfaces REQUIRED)

# Install Python modules
ament_python_install_package(${PROJECT_NAME})
install(DIRECTORY text_to_speech_pkg/models text_to_speech_pkg/utils
        DESTINATION lib/${PROJECT_NAME})
# Install Python executables
install(PROGRAMS text_to_speech_pkg/text_to_speech_node.py
        DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME}/)
ament_package()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/RosDebianPackage.cmake)
configure_ros_debian_package_with_metadata(
  PACKAGE_NAME "ros-${ROS_DISTRO}-text-to-speech-pkg"
  DESCRIPTION "Text to speech ROS2 package"
  CONTACT "ECI Maintainer <eci-maintainer@intel.com>"
  BUILD_DEPENDS
    "build-essential"
    "cmake"
    "ninja-build"
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-ament-cmake-python"
    "ros-${ROS_DISTRO}-follow-me-interfaces"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-rclpy"
    "ros-${ROS_DISTRO}-std-msgs"
    "ros-${ROS_DISTRO}-geometry-msgs"
  DEPENDS
    "ros-${ROS_DISTRO}-follow-me-interfaces"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-rclpy"
    "ros-${ROS_DISTRO}-std-msgs"
    "ros-${ROS_DISTRO}-geometry-msgs"
)
