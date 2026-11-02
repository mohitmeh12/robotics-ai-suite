# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

foreach(required_variable IN ITEMS ROS_DISTRO ADBSCAN_SOURCE_DIR ADBSCAN_BUILD_DIR ADBSCAN_PACKAGE_OUTPUT_DIR ADBSCAN_PACKAGE_COMPONENTS)
  if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
    message(FATAL_ERROR "${required_variable} is required")
  endif()
endforeach()

file(MAKE_DIRECTORY "${ADBSCAN_PACKAGE_OUTPUT_DIR}")
set(package_stage_dir "${ADBSCAN_BUILD_DIR}/staged-packages")
file(MAKE_DIRECTORY "${package_stage_dir}")
string(REPLACE "," ";" ADBSCAN_CMAKE_PACKAGES "${ADBSCAN_PACKAGE_COMPONENTS}")
if(ADBSCAN_WITH_ONEAPI AND "$ENV{CXX}" STREQUAL "")
  find_program(oneapi_compiler NAMES icpx PATHS /opt/intel/oneapi/compiler/latest/bin)
  if(NOT oneapi_compiler)
    message(FATAL_ERROR "OneAPI packages require icpx; install intel-oneapi-dpcpp-cpp-2026.1 or set CXX")
  endif()
  set(package_compiler_args -DCMAKE_CXX_COMPILER=${oneapi_compiler})
endif()

foreach(package_name IN LISTS ADBSCAN_CMAKE_PACKAGES)
  set(package_source_dir "${ADBSCAN_SOURCE_DIR}/src/${package_name}")
  set(package_build_dir "${ADBSCAN_BUILD_DIR}/${package_name}")
  if(NOT EXISTS "${package_source_dir}/CMakeLists.txt")
    message(FATAL_ERROR "Package source directory not found: ${package_source_dir}")
  endif()

  message(STATUS "Packaging ${package_name}")

  execute_process(
    COMMAND "${CMAKE_COMMAND}" -G Ninja -S "${package_source_dir}" -B "${package_build_dir}"
      -DROS_DISTRO=${ROS_DISTRO}
      -DBUILD_TESTING=OFF
      -DWITH_ONEAPI=${ADBSCAN_WITH_ONEAPI}
      ${package_compiler_args}
      -DCMAKE_PREFIX_PATH=${package_stage_dir}
      -DADBSCAN_PACKAGE_VERSION_SUFFIX=${ADBSCAN_PACKAGE_VERSION_SUFFIX}
    COMMAND_ERROR_IS_FATAL ANY
  )
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${package_build_dir}" --parallel
    COMMAND_ERROR_IS_FATAL ANY
  )
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${package_build_dir}" --prefix "${package_stage_dir}"
    COMMAND_ERROR_IS_FATAL ANY
  )
  execute_process(
    COMMAND cpack --config "${package_build_dir}/CPackConfig.cmake" -G DEB -B "${ADBSCAN_PACKAGE_OUTPUT_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
  )
endforeach()
