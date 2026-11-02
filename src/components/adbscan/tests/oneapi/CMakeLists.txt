cmake_minimum_required(VERSION 3.16)
project(adbscan_oneapi_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(PCL REQUIRED COMPONENTS common io kdtree octree oneapi_common oneapi_kdtree oneapi_octree)
find_package(GTest REQUIRED)
find_package(TBB REQUIRED)

set(adbscan_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../src/adbscan_ros2)
set(core_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../src/adbscan_core)
add_executable(adbscan_oneapi_test
  ${adbscan_dir}/test/main.cpp
  ${adbscan_dir}/test/test_adbscan_clusters.cpp
  test_oneapi_neighbors.cpp
  ${adbscan_dir}/src/dbscan_adaptiveK.cpp
  ${adbscan_dir}/src/doDBSCAN.cpp
  ${adbscan_dir}/src/Util.cpp
  ${core_dir}/src/clusterer.cpp
  ${core_dir}/src/oneapi_neighbors.cpp)
target_include_directories(adbscan_oneapi_test PRIVATE
  ${adbscan_dir}/src ${core_dir}/include ${PCL_INCLUDE_DIRS} /usr/include/flann-oneapi)
target_compile_definitions(adbscan_oneapi_test PRIVATE USE_ONEAPI ADBSCAN_CORE_WITH_ONEAPI)
target_compile_options(adbscan_oneapi_test PRIVATE -fsycl -fsycl-unnamed-lambda)
target_link_options(adbscan_oneapi_test PRIVATE -fsycl)
target_link_libraries(adbscan_oneapi_test PRIVATE ${PCL_LIBRARIES} TBB::tbb GTest::gtest)