// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Intel Corporation

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <sys/resource.h>
#include <vector>

#include "../src/dbscan_adaptiveK.h"
#include "../src/doDBSCAN.hpp"
#include "../src/Util.hpp"

namespace
{
struct Labels
{
  std::vector<INT32_t> cluster;
  std::vector<INT32_t> type;
};

Labels cluster_points(
  std::vector<LiDAR_data_3D_t> points, std::vector<FPDS_t> k, std::vector<FPDS_t> epsilon)
{
  Labels result{std::vector<INT32_t>(points.size(), 0), std::vector<INT32_t>(points.size(), 0)};
  dbscan_adaptiveK(
    points.data(), k.data(), epsilon.data(), result.cluster.data(), result.type.data(),
    static_cast<INT32_t>(points.size()), "pcl_kdtree");
  return result;
}

void record_frame_baseline(const char * frame, int points, const std::vector<double> & timings)
{
  rusage usage{};
  ASSERT_EQ(getrusage(RUSAGE_SELF, &usage), 0);
  std::cout << "baseline_frame=" << frame << " raw_points=" << points << " stage_seconds=";
  for (double seconds : timings) {
    std::cout << seconds << ' ';
  }
  std::cout << "peak_rss_kib=" << usage.ru_maxrss << std::endl;
}
}  // namespace

TEST(ADBSCANTest, separated_clusters_and_noise)
{
  auto labels = cluster_points(
    {{{0.0f, 0.0f, 0.0f}},
     {{0.1f, 0.0f, 0.0f}},
     {{0.2f, 0.0f, 0.0f}},
     {{5.0f, 0.0f, 0.0f}},
     {{5.1f, 0.0f, 0.0f}},
     {{5.2f, 0.0f, 0.0f}},
     {{10.0f, 0.0f, 0.0f}}},
    std::vector<FPDS_t>(7, 2.0), std::vector<FPDS_t>(7, 0.21));

  EXPECT_EQ(labels.cluster, (std::vector<INT32_t>{1, 1, 1, 2, 2, 2, -1}));
  EXPECT_EQ(labels.type, (std::vector<INT32_t>{1, 1, 1, 1, 1, 1, -1}));
}

TEST(ADBSCANTest, border_points_join_core_cluster)
{
  auto labels = cluster_points(
    {{{0.0f, 0.0f, 0.0f}}, {{0.2f, 0.0f, 0.0f}}, {{0.4f, 0.0f, 0.0f}}}, {2.0, 2.0, 2.0},
    {0.21, 0.21, 0.21});

  EXPECT_EQ(labels.cluster, (std::vector<INT32_t>{1, 1, 1}));
  EXPECT_EQ(labels.type, (std::vector<INT32_t>{0, 1, 0}));
}

TEST(ADBSCANCharacterization, shared_border_expands_and_merges_clusters)
{
  auto labels = cluster_points(
    {{{0.0f, 0.0f, 0.0f}},
     {{0.05f, 0.0f, 0.0f}},
     {{0.10f, 0.0f, 0.0f}},
     {{0.15f, 0.0f, 0.0f}},
     {{0.6f, 0.0f, 0.0f}},
     {{1.05f, 0.0f, 0.0f}},
     {{1.10f, 0.0f, 0.0f}},
     {{1.15f, 0.0f, 0.0f}},
     {{1.20f, 0.0f, 0.0f}}},
    std::vector<FPDS_t>(9, 3.0), std::vector<FPDS_t>(9, 0.46));

  EXPECT_EQ(labels.cluster, (std::vector<INT32_t>{1, 1, 1, 1, 1, 1, 1, 1, 1}));
  EXPECT_EQ(labels.type[4], 0);
}

TEST(ADBSCANCharacterization, radius_boundary_differs_by_build)
{
  std::vector<LiDAR_data_3D_t> points = {
    {{0.0f, 0.0f, 0.0f}}, {{0.5f, 0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}};
  auto on_boundary = cluster_points(points, {2.0, 2.0, 2.0}, {0.5, 0.5, 0.5});
  auto outside_boundary = cluster_points(points, {2.0, 2.0, 2.0}, {0.49, 0.49, 0.49});

#ifdef USE_ONEAPI
  EXPECT_EQ(on_boundary.cluster, (std::vector<INT32_t>{-1, -1, -1}));
#else
  EXPECT_EQ(on_boundary.cluster, (std::vector<INT32_t>{1, 1, 1}));
#endif
  EXPECT_EQ(outside_boundary.cluster, (std::vector<INT32_t>{-1, -1, -1}));
}

TEST(ADBSCANCharacterization, seed_radius_can_reclassify_earlier_noise_as_core)
{
  auto labels = cluster_points(
    {{{0.0f, 0.0f, 0.0f}}, {{0.4f, 0.0f, 0.0f}}, {{0.8f, 0.0f, 0.0f}}}, {1.0, 1.0, 1.0},
    {0.1, 0.5, 0.1});

  EXPECT_EQ(labels.cluster, (std::vector<INT32_t>{1, 1, 1}));
#ifdef USE_ONEAPI
  EXPECT_EQ(labels.type[0], -1);
#else
  EXPECT_EQ(labels.type[0], 1);
#endif
  EXPECT_EQ(labels.type[1], 1);
}

TEST(ADBSCANCharacterization, seed_k_can_reclassify_higher_threshold_neighbors_as_core)
{
  auto labels = cluster_points(
    {{{0.0f, 0.0f, 0.0f}}, {{0.1f, 0.0f, 0.0f}}, {{0.2f, 0.0f, 0.0f}}}, {4.0, 2.0, 4.0},
    {0.21, 0.21, 0.21});

  EXPECT_EQ(labels.cluster, (std::vector<INT32_t>{1, 1, 1}));
  EXPECT_EQ(labels.type, (std::vector<INT32_t>{1, 1, 1}));
}

TEST(ADBSCANTest, obstacle_geometry_preserves_interleaved_cluster_membership)
{
  auto clusters = get_clusters(
    {Point(1, 2, 3, 7), Point(20, 20, 20, -1), Point(10, 4, 6, 2), Point(1.5, 3, 4, 7),
     Point(3, 6, 7, 7), Point(14, 8, 10, 2)});

  ASSERT_EQ(clusters.size(), 2u);
  EXPECT_EQ(clusters[0].cluster_id, 7);
  EXPECT_EQ(clusters[1].cluster_id, 2);
  EXPECT_EQ(clusters[0].size, 3);
  EXPECT_EQ(clusters[1].size, 2);
  auto first = get_obstacle(clusters[0].point_list);
  EXPECT_DOUBLE_EQ(first.c_x, 2);
  EXPECT_DOUBLE_EQ(first.c_y, 4);
  EXPECT_DOUBLE_EQ(first.c_z, 5);
  EXPECT_DOUBLE_EQ(first.x, 2);
  EXPECT_DOUBLE_EQ(first.y, 4);
  EXPECT_DOUBLE_EQ(first.z, 4);
  auto second = get_obstacle(clusters[1].point_list);
  EXPECT_DOUBLE_EQ(second.c_x, 12);
  EXPECT_DOUBLE_EQ(second.c_y, 6);
  EXPECT_DOUBLE_EQ(second.c_z, 8);
  EXPECT_DOUBLE_EQ(second.x, 4);
  EXPECT_DOUBLE_EQ(second.y, 4);
  EXPECT_DOUBLE_EQ(second.z, 4);
  EXPECT_TRUE(get_clusters({}).empty());
}

TEST(ADBSCANTest, full_frame_produces_two_obstacles)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.subsample_ratio = 1;
  CONFIG_PARAMS.base = 2;
  CONFIG_PARAMS.Lidar_type = "3D";
  std::vector<LiDAR_data_4D_t> points = {{{1.0f, 0.0f, 0.0f, 0.0f}}, {{1.1f, 0.1f, 0.1f, 0.0f}},
                                         {{1.2f, 0.2f, 0.2f, 0.0f}}, {{5.0f, 1.0f, 1.0f, 0.0f}},
                                         {{5.1f, 1.1f, 1.1f, 0.0f}}, {{5.2f, 1.2f, 1.2f, 0.0f}}};
  std::vector<double> timings(5, 0.0);

  auto obstacles = doDBSCAN(
    points.data(), static_cast<int>(points.size() * sizeof(LiDAR_data_4D_t)), 3, &timings, 0.9f,
    0.5f);

  ASSERT_EQ(obstacles.size(), 2u);
  EXPECT_NEAR(obstacles[0].c_x, 1.1, 1e-6);
  EXPECT_NEAR(obstacles[0].c_y, 0.1, 1e-6);
  EXPECT_NEAR(obstacles[0].c_z, 0.1, 1e-6);
  EXPECT_NEAR(obstacles[0].x, 0.2, 1e-6);
  EXPECT_NEAR(obstacles[1].c_x, 5.1, 1e-6);
  EXPECT_NEAR(obstacles[1].c_y, 1.1, 1e-6);
  EXPECT_NEAR(obstacles[1].c_z, 1.1, 1e-6);
  EXPECT_NEAR(obstacles[1].z, 0.2, 1e-6);
}

TEST(ADBSCANTest, planar_scan_produces_obstacles_in_scan_frame)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.Lidar_type = "2D";
  CONFIG_PARAMS.scale_factor = 1;
  CONFIG_PARAMS.base = 2;
  std::vector<LiDAR_data_4D_t> points = {{{1.0f, 0.0f, 0.0f, 0.0f}},  {{1.04f, 0.0f, 0.0f, 0.0f}},
                                         {{1.08f, 0.0f, 0.0f, 0.0f}}, {{5.0f, 0.0f, 0.0f, 0.0f}},
                                         {{5.04f, 0.0f, 0.0f, 0.0f}}, {{5.08f, 0.0f, 0.0f, 0.0f}}};
  std::vector<double> timings(5, 0.0);

  auto obstacles =
    doDBSCAN(points.data(), static_cast<int>(points.size() * sizeof(LiDAR_data_4D_t)), 2, &timings);

  ASSERT_EQ(obstacles.size(), 2u);
  EXPECT_NEAR(obstacles[0].c_x, 1.04, 1e-6);
  EXPECT_DOUBLE_EQ(obstacles[0].c_y, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].c_z, 0);
  EXPECT_NEAR(obstacles[1].c_x, 5.04, 1e-6);
  EXPECT_DOUBLE_EQ(obstacles[1].c_y, 0);
  EXPECT_DOUBLE_EQ(obstacles[1].c_z, 0);
}

TEST(ADBSCANTest, depth_points_in_body_frame_produce_obstacles)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.subsample_ratio = 1;
  CONFIG_PARAMS.x_filter_back = 3;
  CONFIG_PARAMS.y_filter_left = 2;
  CONFIG_PARAMS.y_filter_right = -2;
  CONFIG_PARAMS.Lidar_type = "RS";
  CONFIG_PARAMS.scale_factor = 1;
  CONFIG_PARAMS.base = 2;
  std::vector<LiDAR_data_4D_t> points = {{{1.0f, 0.0f, 0.5f, 0.0f}}, {{1.1f, 0.1f, 0.6f, 0.0f}},
                                         {{1.2f, 0.2f, 0.7f, 0.0f}}, {{2.0f, 1.0f, 0.5f, 0.0f}},
                                         {{2.1f, 1.1f, 0.6f, 0.0f}}, {{2.2f, 1.2f, 0.7f, 0.0f}}};
  std::vector<double> timings(5, 0.0);

  auto obstacles =
    doDBSCAN(points.data(), static_cast<int>(points.size() * sizeof(LiDAR_data_4D_t)), 4, &timings);

  ASSERT_EQ(obstacles.size(), 2u);
  EXPECT_NEAR(obstacles[0].c_x, 1.1, 1e-6);
  EXPECT_NEAR(obstacles[0].c_y, 0.1, 1e-6);
  EXPECT_NEAR(obstacles[0].c_z, 0.6, 1e-6);
  EXPECT_NEAR(obstacles[1].c_x, 2.1, 1e-6);
  EXPECT_NEAR(obstacles[1].c_y, 1.1, 1e-6);
  EXPECT_NEAR(obstacles[1].c_z, 0.6, 1e-6);
}

TEST(ADBSCANCharacterization, empty_2d_frame_returns_zero_sized_obstacle)
{
  std::vector<double> timings(5, 0.0);
  auto obstacles = doDBSCAN(nullptr, 0, 2, &timings);

  ASSERT_EQ(obstacles.size(), 1u);
  EXPECT_DOUBLE_EQ(obstacles[0].c_x, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].x, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].y, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].z, 0);
}

TEST(ADBSCANCharacterization, empty_3d_frame_returns_zero_sized_obstacle)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.subsample_ratio = 1;
  std::vector<double> timings(5, 0.0);
  auto obstacles = doDBSCAN(nullptr, 0, 3, &timings);

  ASSERT_EQ(obstacles.size(), 1u);
  EXPECT_DOUBLE_EQ(obstacles[0].c_x, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].x, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].y, 0);
  EXPECT_DOUBLE_EQ(obstacles[0].z, 0);
}

TEST(ADBSCANBenchmark, DISABLED_dense_cloud_baseline)
{
  for (int point_count : {512, 2048, 8192}) {
    std::vector<LiDAR_data_3D_t> points(point_count);
    for (int index = 0; index < point_count; ++index) {
      points[index] = {{static_cast<float>(index % 2), 0.0f, 0.0f}};
    }
    auto start = std::chrono::steady_clock::now();
    auto labels = cluster_points(
      points, std::vector<FPDS_t>(point_count, 2.0), std::vector<FPDS_t>(point_count, 0.1));
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - start);
    EXPECT_EQ(labels.cluster.front(), 1);
    EXPECT_EQ(labels.cluster.back(), 2);
    std::cout << "baseline_points=" << point_count << " elapsed_us=" << elapsed.count()
              << " clusters=2" << std::endl;
  }
}

TEST(ADBSCANBenchmark, DISABLED_planar_frame_baseline)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.Lidar_type = "2D";
#ifdef USE_ONEAPI
  CONFIG_PARAMS.oneapi_library = "oneapi_octree";
#endif
  CONFIG_PARAMS.scale_factor = 1;
  CONFIG_PARAMS.base = 2;
  std::vector<LiDAR_data_4D_t> points(720);
  for (size_t index = 0; index < points.size(); ++index) {
    double angle = -1.57079632679 + 3.14159265359 * index / (points.size() - 1);
    points[index] = {
      {static_cast<float>(4 * std::cos(angle)), static_cast<float>(4 * std::sin(angle)), 0.0f,
       0.0f}};
  }
  std::vector<double> timings(5, 0.0);

  auto obstacles =
    doDBSCAN(points.data(), static_cast<int>(points.size() * sizeof(LiDAR_data_4D_t)), 2, &timings);

  ASSERT_EQ(obstacles.size(), 1u);
  record_frame_baseline("planar", static_cast<int>(points.size()), timings);
}

TEST(ADBSCANBenchmark, DISABLED_depth_frame_baseline)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.subsample_ratio = 150;
  CONFIG_PARAMS.x_filter_back = 3;
  CONFIG_PARAMS.y_filter_left = 2;
  CONFIG_PARAMS.y_filter_right = -2;
  CONFIG_PARAMS.Lidar_type = "RS";
#ifdef USE_ONEAPI
  CONFIG_PARAMS.oneapi_library = "oneapi_octree";
#endif
  CONFIG_PARAMS.scale_factor = 1;
  CONFIG_PARAMS.base = 2;
  std::vector<LiDAR_data_4D_t> points(640 * 480);
  for (size_t index = 0; index < points.size(); ++index) {
    size_t sample = index / 150;
    points[index] = {
      {static_cast<float>(1 + sample % 2 + sample % 7 * 0.001),
       static_cast<float>(sample % 5 * 0.001), static_cast<float>(0.5 + sample % 3 * 0.001), 0.0f}};
  }
  std::vector<double> timings(5, 0.0);

  auto obstacles =
    doDBSCAN(points.data(), static_cast<int>(points.size() * sizeof(LiDAR_data_4D_t)), 4, &timings);

  ASSERT_EQ(obstacles.size(), 2u);
  record_frame_baseline("depth", static_cast<int>(points.size()), timings);
}

TEST(ADBSCANTest, adbscan_cluster_test)
{
  std::vector<Point> points_in_frame;
  std::vector<std::vector<double>> points;
  points = {{22.719000, 0.031000, 0.977000, 1},   {23.257999, 0.361000, 0.996000, 1},
            {23.771000, 0.743000, 1.013000, 1},   {24.194000, 1.100000, 1.028000, 1},
            {24.634001, 1.469000, 1.044000, 1},   {24.934999, 1.881000, 1.055000, 1},
            {25.016001, 2.243000, 1.058000, 1},   {25.379000, 2.638000, 1.072000, 1},
            {25.714001, 3.042000, 1.085000, 1},   {26.063000, 3.499000, 1.098000, 1},
            {26.707001, 3.972000, 1.122000, 1},   {22.101999, 3.602000, 0.966000, 2},
            {22.033001, 3.947000, 0.966000, 2},   {27.686001, 5.372000, 1.163000, 3},
            {27.688999, 5.780000, 1.166000, 3},   {27.888000, 6.234000, 1.175000, 3},
            {28.917000, 6.944000, 1.215000, 3},   {28.797001, 7.347000, 1.214000, 3},
            {28.591999, 7.727000, 1.211000, 3},   {28.847000, 8.285000, 1.224000, 3},
            {7.669000, 4.863000, 0.516000, 4},    {7.514000, 4.898000, 0.512000, 4},
            {7.684000, 5.166000, 0.522000, 4},    {7.502000, 5.199000, 0.518000, 4},
            {7.385000, 5.291000, 0.516000, 4},    {7.372000, 5.442000, 0.519000, 4},
            {19.695000, -19.572001, 1.148000, 5}, {19.343000, -18.687000, 1.119000, 5},
            {19.792000, -18.586000, 1.127000, 5}, {20.225000, -18.403999, 1.134000, 5},
            {18.683001, -16.527000, 1.053000, 6}, {18.575001, -15.970000, 1.037000, 6},
            {19.106001, -15.962000, 1.051000, 6}, {19.643999, -15.893000, 1.064000, 6},
            {23.756001, -18.664000, 1.231000, 1}, {23.280001, -17.420000, 1.192000, 1},
            {23.159000, -16.825001, 1.177000, 1}, {22.841000, -16.106001, 1.154000, 1},
            {22.785000, -15.537000, 1.142000, 1}, {22.841000, -15.107000, 1.135000, 1},
            {22.527000, -14.446000, 1.114000, 1}, {22.632000, -14.066000, 1.110000, 1},
            {22.318001, -13.391000, 1.089000, 1}, {22.232000, -12.916000, 1.079000, 1},
            {22.329000, -12.553000, 1.075000, 1}, {22.242001, -12.049000, 1.064000, 1},
            {22.343000, -11.698000, 1.062000, 1}, {15.028000, -7.609000, 0.779000, 7},
            {14.822000, -7.129000, 0.765000, 7},  {15.184000, -7.011000, 0.775000, 7}};

  for (uint64_t i = 0; i < points.size(); i++) {
    points_in_frame.push_back(
      Point(points[i][0], points[i][1], points[i][2], static_cast<int>(points[i][3])));
  }

  std::vector<Cluster> clusters = get_clusters(points_in_frame);

  EXPECT_EQ(clusters.size(), 7);
}
