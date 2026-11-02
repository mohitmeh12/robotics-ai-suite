// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Intel Corporation

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>

#include "adbscan_core/clusterer.hpp"
#include "../src/doDBSCAN.hpp"

Config_params_t CONFIG_PARAMS;

ADBSCAN_Result_t adaptive_parameters(
  LiDAR_data_3D_t * points, FPDS_t * k, FPDS_t * k_floor, FPDS_t * radius, FPDS_t * radius_cont,
  Adaptive_params_t * params, INT32_t count);

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

  for (int i = 0; i < points.size(); i++) {
    points_in_frame.push_back(Point(points[i][0], points[i][1], points[i][2], (int)points[i][3]));
  }

  std::vector<Cluster> clusters = get_clusters(points_in_frame);
  EXPECT_EQ(clusters.size(), 7);
}

TEST(ADBSCANTest, legacy_cpu_matches_core_for_clusters_and_noise)
{
  std::vector<LiDAR_data_3D_t> legacy_points = {
    {{0.0f, 0.0f, 0.0f}}, {{0.1f, 0.0f, 0.0f}}, {{0.2f, 0.1f, 0.0f}}, {{0.5f, 0.0f, 0.0f}},
    {{3.0f, 1.0f, 1.0f}}, {{3.1f, 1.1f, 1.0f}}, {{3.2f, 1.0f, 1.0f}}, {{8.0f, 0.0f, 0.0f}},
  };
  std::vector<adbscan_core::Point> core_points;
  for (const auto & point : legacy_points) {
    core_points.push_back({point.val[0], point.val[1], point.val[2]});
  }
  std::vector<double> k(legacy_points.size(), 2.0);
  std::vector<double> radius(legacy_points.size(), 0.35);
  std::vector<INT32_t> legacy_labels(legacy_points.size(), 0);
  std::vector<INT32_t> legacy_types(legacy_points.size(), 0);

  dbscan_adaptiveK(
    legacy_points.data(), k.data(), radius.data(), legacy_labels.data(), legacy_types.data(),
    legacy_points.size());
  const auto core = adbscan_core::cluster(core_points, k, radius);

  EXPECT_EQ(core.labels, legacy_labels);
  EXPECT_EQ(core.types, legacy_types);
  EXPECT_EQ(core.clusters.size(), 2u);
  EXPECT_EQ(core.labels.back(), -1);
}

TEST(ADBSCANTest, legacy_cpu_matches_core_with_adaptive_parameters)
{
  std::vector<LiDAR_data_3D_t> legacy_points = {
    {{0.0f, 0.0f, 0.0f}}, {{0.1f, 0.0f, 0.0f}}, {{0.2f, 0.0f, 0.0f}}, {{0.3f, 0.0f, 0.0f}},
    {{2.0f, 2.0f, 0.1f}}, {{2.1f, 2.0f, 0.1f}}, {{2.2f, 2.0f, 0.1f}}, {{5.0f, 5.0f, 1.0f}},
  };
  std::vector<adbscan_core::Point> core_points;
  for (const auto & point : legacy_points) {
    core_points.push_back({point.val[0], point.val[1], point.val[2]});
  }
  std::vector<double> k = {2, 2, 3, 2, 2, 2, 2, 3};
  std::vector<double> radius = {0.18, 0.18, 0.25, 0.18, 0.20, 0.20, 0.20, 0.10};
  std::vector<INT32_t> legacy_labels(legacy_points.size(), 0);
  std::vector<INT32_t> legacy_types(legacy_points.size(), 0);

  dbscan_adaptiveK(
    legacy_points.data(), k.data(), radius.data(), legacy_labels.data(), legacy_types.data(),
    legacy_points.size());
  const auto core = adbscan_core::cluster(core_points, k, radius);

  EXPECT_EQ(core.labels, legacy_labels);
  EXPECT_EQ(core.types, legacy_types);
  EXPECT_EQ(core.clusters.size(), 2u);
  EXPECT_EQ(core.labels.back(), -1);
}

TEST(ADBSCANTest, recorded_realsense_frame_matches_core)
{
  const char * frame_path = std::getenv("ADBSCAN_PARITY_XYZ");
  if (!frame_path) {
    GTEST_SKIP() << "Set ADBSCAN_PARITY_XYZ to a sampled XYZ frame";
  }
  std::ifstream frame(frame_path);
  ASSERT_TRUE(frame.is_open());
  std::vector<LiDAR_data_3D_t> legacy_points;
  std::vector<adbscan_core::Point> core_points;
  float x, y, z;
  while (frame >> x >> y >> z) {
    legacy_points.push_back({{x, y, z}});
    core_points.push_back({x, y, z});
  }
  ASSERT_TRUE(frame.eof());
  ASSERT_GE(legacy_points.size(), 3u);

  CONFIG_PARAMS.Lidar_type = "RS";
  CONFIG_PARAMS.scale_factor = 1;
  Adaptive_params_t adaptive{2, 0, 0};
  std::vector<double> k(legacy_points.size()), k_floor(legacy_points.size());
  std::vector<double> radius(legacy_points.size()), radius_cont(legacy_points.size());
  ASSERT_EQ(
    adaptive_parameters(
      legacy_points.data(), k.data(), k_floor.data(), radius.data(), radius_cont.data(), &adaptive,
      legacy_points.size()),
    ADBSCAN_SUCCESS);
  std::vector<INT32_t> legacy_labels(legacy_points.size(), 0);
  std::vector<INT32_t> legacy_types(legacy_points.size(), 0);

  dbscan_adaptiveK(
    legacy_points.data(), k_floor.data(), radius.data(), legacy_labels.data(), legacy_types.data(),
    legacy_points.size());
  const auto core = adbscan_core::cluster(core_points, k_floor, radius);

  EXPECT_EQ(core.labels, legacy_labels);
  EXPECT_EQ(core.types, legacy_types);
  EXPECT_FALSE(core.clusters.empty());
}

TEST(ADBSCANTest, core_clustering_preserves_follow_me_target_tracking)
{
  CONFIG_PARAMS = {};
  CONFIG_PARAMS.subsample_ratio = 1;
  CONFIG_PARAMS.x_filter_back = 0;
  CONFIG_PARAMS.y_filter_right = -2;
  CONFIG_PARAMS.y_filter_left = 2;
  CONFIG_PARAMS.z_filter = -1;
  CONFIG_PARAMS.Z_based_ground_removal = 1;
  CONFIG_PARAMS.base = 1;
  CONFIG_PARAMS.Lidar_type = "3D";
  CONFIG_PARAMS.tracking_radius = 0.5;
  CONFIG_PARAMS.max_frame_blocked = 2;

  const LiDAR_data_4D_t points[] = {
    {{1.0f, 0.0f, 0.0f, 0.0f}},
    {{1.1f, 0.1f, 0.1f, 0.0f}},
    {{1.2f, 0.0f, 0.0f, 0.0f}},
    {{4.0f, 1.0f, 1.0f, 0.0f}},
  };
  void * data = malloc(sizeof(points));
  ASSERT_NE(data, nullptr);
  memcpy(data, points, sizeof(points));
  int blocked_frames = 0;
  std::vector<Obstacle> obstacles;

  const auto target =
    doDBSCAN(data, sizeof(points), 3, {1.0f, 0.0f, 0.0f}, blocked_frames, obstacles);

  ASSERT_EQ(obstacles.size(), 1u);
  EXPECT_NEAR(target.x, 1.1f, 1e-5);
  EXPECT_NEAR(target.y, 0.03333f, 1e-4);
  EXPECT_EQ(blocked_frames, 2);
}
