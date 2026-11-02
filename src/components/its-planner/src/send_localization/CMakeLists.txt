# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# https://docs.ros.org/en/humble/The-ROS2-Project/Contributing/Code-Style-Language-Versions.html#id5
cmake_minimum_required(VERSION 3.14.4)
project(send_localization)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(ItsDebianPackaging)

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-fno-strict-overflow -fno-delete-null-pointer-checks")

# Detect ROS2 distribution
if(DEFINED ENV{ROS_DISTRO})
    set(ROS_DISTRO $ENV{ROS_DISTRO})
else()
    execute_process(COMMAND bash -c "source /opt/ros/*/setup.bash && echo $ROS_DISTRO" OUTPUT_VARIABLE ROS_DISTRO OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

message(STATUS "Building for ROS2 distribution: ${ROS_DISTRO}")

# Set compile definitions based on ROS distribution
if(ROS_DISTRO STREQUAL "jazzy")
    add_compile_definitions(ROS2_JAZZY)
elseif(ROS_DISTRO STREQUAL "humble")
    add_compile_definitions(ROS2_HUMBLE)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(nav2_core REQUIRED)
find_package(its_planner REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_amcl REQUIRED)
find_package(std_srvs REQUIRED)

set(library_name send_localization_core)
set(executable_name send_localization)

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

# Library
add_library(${library_name} SHARED
  src/send_localization.cpp
)

ament_target_dependencies(${library_name}
  ${dependencies}
)

# Executable
add_executable(${executable_name}
  src/main.cpp
)

target_link_libraries(${executable_name} ${library_name})

ament_target_dependencies(${executable_name}
  ${dependencies}
)

install(TARGETS ${library_name}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS
  ${executable_name}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  add_subdirectory(test)
endif()

ament_export_dependencies(${dependencies})

its_enable_debian_package(
  PACKAGE_BASENAME "its-send-localization"
  PACKAGE_DESCRIPTION "Package for sending localization data in the relocalization module"
  BUILD_DEPENDS
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-ament-cmake-gtest"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-nav2-amcl"
    "ros-${ROS_DISTRO}-nav2-core"
    "ros-${ROS_DISTRO}-nav2-msgs"
    "ros-${ROS_DISTRO}-rcl"
    "ros-${ROS_DISTRO}-rclcpp-lifecycle"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-std-srvs"
  PACKAGE_DEPENDS
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-its-planner"
    "ros-${ROS_DISTRO}-nav2-amcl"
    "ros-${ROS_DISTRO}-nav2-core"
    "ros-${ROS_DISTRO}-nav2-msgs"
    "ros-${ROS_DISTRO}-rcl"
    "ros-${ROS_DISTRO}-rclcpp-lifecycle"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-robotics-sdk-license"
    "ros-${ROS_DISTRO}-std-srvs"
)

ament_package()
