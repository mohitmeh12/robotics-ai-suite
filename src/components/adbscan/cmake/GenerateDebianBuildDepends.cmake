# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

foreach(required_variable IN ITEMS ROS_DISTRO ADBSCAN_SOURCE_DIR ADBSCAN_BUILD_DEPENDS_OUTPUT_DIR ADBSCAN_PACKAGE_COMPONENTS)
  if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
    message(FATAL_ERROR "${required_variable} is required")
  endif()
endforeach()

file(MAKE_DIRECTORY "${ADBSCAN_BUILD_DEPENDS_OUTPUT_DIR}")
string(REPLACE "," ";" ADBSCAN_CMAKE_PACKAGES "${ADBSCAN_PACKAGE_COMPONENTS}")
set(local_debian_packages "")
foreach(component IN LISTS ADBSCAN_CMAKE_PACKAGES)
  string(REPLACE "_" "-" debian_component_name "${component}")
  list(APPEND local_debian_packages "ros-${ROS_DISTRO}-${debian_component_name}")
endforeach()

function(read_debian_build_dependencies cmake_file output_variable)
  file(STRINGS "${cmake_file}" cmake_lines)
  set(build_dependencies "")
  set(in_package_metadata FALSE)
  set(dependency_type "")
  foreach(cmake_line IN LISTS cmake_lines)
    if(NOT in_package_metadata)
      if(cmake_line MATCHES "^configure_ros_debian_package_with_metadata\\([ \\t]*$")
        set(in_package_metadata TRUE)
      endif()
    elseif(cmake_line MATCHES "^\\)[ \\t]*$")
      set(in_package_metadata FALSE)
      set(dependency_type "")
    elseif(cmake_line MATCHES "^[ \\t]*BUILD_DEPENDS[ \\t]*$")
      set(dependency_type "BUILD_DEPENDS")
    elseif(cmake_line MATCHES "^[ \\t]*[A-Z_]+[ \\t]*$")
      set(dependency_type "")
    elseif(dependency_type STREQUAL "BUILD_DEPENDS" AND cmake_line MATCHES "^[ \\t]*\"([^\"]+)\"[ \\t]*$")
      list(APPEND build_dependencies "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  if(in_package_metadata OR build_dependencies STREQUAL "")
    message(FATAL_ERROR "Debian build dependencies are missing or incomplete in ${cmake_file}")
  endif()
  set(${output_variable} "${build_dependencies}" PARENT_SCOPE)
endfunction()

foreach(component IN LISTS ADBSCAN_CMAKE_PACKAGES)
  set(package_cmake_file "${ADBSCAN_SOURCE_DIR}/src/${component}/CMakeLists.txt")
  if(NOT EXISTS "${package_cmake_file}")
    message(FATAL_ERROR "Package CMakeLists.txt not found: ${package_cmake_file}")
  endif()
  read_debian_build_dependencies("${package_cmake_file}" build_dependencies)

  if(NOT ADBSCAN_WITH_ONEAPI)
    list(REMOVE_ITEM build_dependencies
      "intel-oneapi-dpcpp-cpp-2026.1"
      "intel-oneapi-compiler-dpcpp-cpp-2026.1"
      "libpcl-oneapi-dev"
      "libflann-oneapi-dev")
  endif()
  list(REMOVE_DUPLICATES build_dependencies)
  string(REPLACE "\${ROS_DISTRO}" "${ROS_DISTRO}" build_dependencies "${build_dependencies}")
  foreach(local_package IN LISTS local_debian_packages)
    list(REMOVE_ITEM build_dependencies "${local_package}")
  endforeach()
  string(JOIN ", " build_dependencies_line ${build_dependencies})
  set(control_directory "${ADBSCAN_BUILD_DEPENDS_OUTPUT_DIR}/${component}/debian")
  string(REPLACE "_" "-" debian_component_name "${component}")
  file(MAKE_DIRECTORY "${control_directory}")
  file(WRITE "${control_directory}/control"
    "Source: adbscan-${debian_component_name}-build-deps\n"
    "Section: misc\n"
    "Priority: optional\n"
    "Maintainer: ECI Maintainer <eci-maintainer@intel.com>\n"
    "Standards-Version: 4.7.0\n"
    "Build-Depends: ${build_dependencies_line}\n"
    "\n"
    "Package: adbscan-${debian_component_name}-build-deps\n"
    "Architecture: all\n"
    "Description: Build dependencies for adbscan ${component}\n"
  )
  message(STATUS "Generated ${control_directory}/control")
endforeach()
