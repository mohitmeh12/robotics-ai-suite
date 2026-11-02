cmake_minimum_required(VERSION 3.8)
project(adbscan_core)

find_package(ament_cmake REQUIRED)
option(WITH_ONEAPI "Enable PCL and OneAPI neighbor-search backends" OFF)

add_library(adbscan_core SHARED src/clusterer.cpp)
target_compile_features(adbscan_core PUBLIC cxx_std_17)
target_include_directories(adbscan_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(WITH_ONEAPI)
  find_package(PCL REQUIRED COMPONENTS common io kdtree octree oneapi_common oneapi_kdtree oneapi_octree)
  set(oneapi_runtime_dependencies
    "intel-oneapi-runtime-dpcpp-cpp"
    "libpcl-oneapi-kdtree${PCL_VERSION_MAJOR}.${PCL_VERSION_MINOR}"
    "libpcl-oneapi-octree${PCL_VERSION_MAJOR}.${PCL_VERSION_MINOR}")
  target_sources(adbscan_core PRIVATE src/oneapi_neighbors.cpp)
  target_include_directories(adbscan_core PRIVATE ${PCL_INCLUDE_DIRS} /usr/include/flann-oneapi)
  target_compile_definitions(adbscan_core PRIVATE ADBSCAN_CORE_WITH_ONEAPI)
  target_compile_options(adbscan_core PRIVATE -fsycl -fsycl-unnamed-lambda)
  target_link_options(adbscan_core PRIVATE -fsycl)
  target_link_libraries(adbscan_core PRIVATE ${PCL_LIBRARIES})
endif()

install(DIRECTORY include/ DESTINATION include)
install(TARGETS adbscan_core EXPORT export_adbscan_core
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
ament_export_targets(export_adbscan_core HAS_LIBRARY_TARGET)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_clusterer test/test_clusterer.cpp)
  target_link_libraries(test_clusterer adbscan_core)
  if(WITH_ONEAPI)
    target_compile_definitions(test_clusterer PRIVATE ADBSCAN_CORE_WITH_ONEAPI)
  endif()
endif()

ament_package()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/RosDebianPackage.cmake)
configure_ros_debian_package_with_metadata(
  PACKAGE_NAME "ros-${ROS_DISTRO}-adbscan-core"
  DESCRIPTION "Reusable ADBSCAN clustering library"
  CONTACT "ECI Maintainer <eci-maintainer@intel.com>"
  BUILD_DEPENDS
    "build-essential"
    "cmake"
    "ninja-build"
    "ros-${ROS_DISTRO}-ament-cmake"
    "ros-${ROS_DISTRO}-ament-cmake-gtest"
    "intel-oneapi-dpcpp-cpp-2026.1"
    "intel-oneapi-compiler-dpcpp-cpp-2026.1"
    "libpcl-dev"
    "libpcl-oneapi-dev"
    "libflann-oneapi-dev"
  DEPENDS
    ${oneapi_runtime_dependencies}
)