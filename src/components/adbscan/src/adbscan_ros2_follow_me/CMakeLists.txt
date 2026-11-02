# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.5)
project(adbscan_ros2_follow_me)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
set(CMAKE_CXX_STANDARD 17)

if("$ENV{ROS_DISTRO}" STRLESS "humble")
    add_compile_definitions(PRE_ROS_HUMBLE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wformat-overflow=1)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(adbscan_core REQUIRED)
add_compile_options(-std=c++17)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav2_dynamic_msgs REQUIRED)
set(PCL_DIR /usr/lib/x86_64-linux-gnu/cmake/pcl-1.12)
find_package(pcl_conversions REQUIRED)
find_package(PCL 1.12 REQUIRED COMPONENTS common io kdtree)
find_package(Eigen3 REQUIRED)

include_directories(include)

set(HEADER_FILES src/ADBScan.h src/dbscan_adaptiveK.h src/doDBSCAN.hpp src/print_info.h src/Util.hpp)
add_library(adbscan_lib src/dbscan_adaptiveK.cpp src/doDBSCAN.cpp src/print_info.cpp src/Util.cpp ${HEADER_FILES})
ament_target_dependencies(adbscan_lib adbscan_core)

set(dependencies
  "geometry_msgs"
  "nav2_dynamic_msgs"
  "pcl_conversions"
  "PCL"
  "rclcpp"
  "sensor_msgs"
  "std_msgs"
  "visualization_msgs"
)
add_executable(adbscan_pub src/adbscan_pub.cpp)
ament_target_dependencies(adbscan_pub ${dependencies})

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(adbscan_sub
  src/adbscan_sub.cpp
  src/dbscan_adaptiveK.cpp
  src/doDBSCAN.cpp
  src/print_info.cpp
  src/Util.cpp
)
ament_target_dependencies(adbscan_sub ${dependencies} adbscan_core)
target_link_libraries(adbscan_sub ${PCL_LIBRARIES})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  set(TESTFILES
    test/main.cpp
    test/test_adbscan_clusters.cpp)
  ament_add_gtest(${PROJECT_NAME}_test ${TESTFILES})
  target_link_libraries(${PROJECT_NAME}_test adbscan_lib ${PCL_LIBRARIES})

  install(TARGETS
    ${PROJECT_NAME}_test
    DESTINATION lib/${PROJECT_NAME})
endif()

install(TARGETS
  adbscan_pub
  adbscan_sub
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}/
)
ament_package()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/RosDebianPackage.cmake)
configure_ros_debian_package_with_metadata(
  PACKAGE_NAME "ros-${ROS_DISTRO}-adbscan-ros2-follow-me"
  DESCRIPTION "An AMR application for target tracking and motion control with Adaptive DBScan clustering"
  CONTACT "ECI Maintainer <eci-maintainer@intel.com>"
  BUILD_DEPENDS
    "build-essential"
    "cmake"
    "ninja-build"
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-adbscan-core"
    "ros-${ROS_DISTRO}-ament-cmake-gtest"
    "ros-${ROS_DISTRO}-ament-lint-auto"
    "ros-${ROS_DISTRO}-ament-lint-common"
    "ros-${ROS_DISTRO}-builtin-interfaces"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-nav2-dynamic-msgs"
    "ros-${ROS_DISTRO}-pcl-conversions"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-sensor-msgs"
    "ros-${ROS_DISTRO}-std-msgs"
    "ros-${ROS_DISTRO}-visualization-msgs"
    "libpcl-dev"
    "libeigen3-dev"
  DEPENDS
    "ros-${ROS_DISTRO}-adbscan-core"
    "ros-${ROS_DISTRO}-builtin-interfaces"
    "ros-${ROS_DISTRO}-geometry-msgs"
    "ros-${ROS_DISTRO}-nav2-dynamic-msgs (>= 0.0.1-1)"
    "ros-${ROS_DISTRO}-pcl-conversions"
    "ros-${ROS_DISTRO}-rclcpp"
    "ros-${ROS_DISTRO}-ros2run"
    "ros-${ROS_DISTRO}-sensor-msgs"
    "ros-${ROS_DISTRO}-std-msgs"
    "ros-${ROS_DISTRO}-visualization-msgs"
)
