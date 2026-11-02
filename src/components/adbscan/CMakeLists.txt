# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)
project(adbscan_debian_packages NONE)

set(ROS_DISTRO "jazzy" CACHE STRING "ROS distribution to package")
set(ADBSCAN_PACKAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/packages" CACHE PATH "Directory for generated Debian packages")
set(ADBSCAN_PACKAGE_COMPONENTS "" CACHE STRING "Semicolon-separated source directories to package")
set(ADBSCAN_PACKAGE_VERSION_SUFFIX "" CACHE STRING "Optional suffix appended to Debian package versions")
if(ROS_DISTRO STREQUAL "humble")
  set(oneapi_default OFF)
else()
  set(oneapi_default ON)
endif()
option(ADBSCAN_WITH_ONEAPI "Build OneAPI Debian packages" ${oneapi_default})
set(ADBSCAN_BUILD_DEPENDS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/debian-build-deps" CACHE PATH "Directory for generated Debian build dependency control files")

add_custom_target(package
  COMMAND ${CMAKE_COMMAND}
    -DROS_DISTRO=${ROS_DISTRO}
    -DADBSCAN_SOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DADBSCAN_BUILD_DIR=${CMAKE_BINARY_DIR}
    -DADBSCAN_PACKAGE_OUTPUT_DIR=${ADBSCAN_PACKAGE_OUTPUT_DIR}
    "-DADBSCAN_PACKAGE_COMPONENTS=${ADBSCAN_PACKAGE_COMPONENTS}"
    -DADBSCAN_PACKAGE_VERSION_SUFFIX=${ADBSCAN_PACKAGE_VERSION_SUFFIX}
    -DADBSCAN_WITH_ONEAPI=${ADBSCAN_WITH_ONEAPI}
    -P ${CMAKE_SOURCE_DIR}/cmake/PackageAll.cmake
  USES_TERMINAL
  COMMENT "Building native Debian packages for ${ROS_DISTRO}"
)

add_custom_target(debian-build-deps
  COMMAND ${CMAKE_COMMAND}
    -DROS_DISTRO=${ROS_DISTRO}
    -DADBSCAN_WITH_ONEAPI=${ADBSCAN_WITH_ONEAPI}
    -DADBSCAN_SOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DADBSCAN_BUILD_DEPENDS_OUTPUT_DIR=${ADBSCAN_BUILD_DEPENDS_OUTPUT_DIR}
    "-DADBSCAN_PACKAGE_COMPONENTS=${ADBSCAN_PACKAGE_COMPONENTS}"
    -P ${CMAKE_SOURCE_DIR}/cmake/GenerateDebianBuildDepends.cmake
  USES_TERMINAL
  COMMENT "Generating Debian build dependency control files for ${ROS_DISTRO}"
)
