// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "adbscan_core/clusterer.hpp"
#include "../../src/adbscan_ros2/src/dbscan_adaptiveK.h"

namespace
{
adbscan_core::Backend core_backend(const std::string & mode)
{
  if (mode == "oneapi_kdtree") {
    return adbscan_core::Backend::OneApiKdTree;
  }
  if (mode == "oneapi_octree") {
    return adbscan_core::Backend::OneApiOctree;
  }
  return adbscan_core::Backend::PclKdTree;
}

adbscan_core::Result core_cluster(
  const std::vector<LiDAR_data_3D_t> & points, const std::vector<FPDS_t> & k,
  const std::vector<FPDS_t> & radius, const std::string & mode)
{
  std::vector<adbscan_core::Point> converted;
  converted.reserve(points.size());
  for (const auto & point : points) {
    converted.push_back({point.val[0], point.val[1], point.val[2]});
  }
  return adbscan_core::cluster(converted, k, radius, core_backend(mode));
}
}  // namespace

class OneAPINeighborSearch : public ::testing::TestWithParam<const char *>
{
};

TEST_P(OneAPINeighborSearch, small_clusters)
{
  const std::string mode = GetParam();
  std::vector<LiDAR_data_3D_t> points = {{{0.0f, 0.0f, 0.0f}}, {{0.1f, 0.0f, 0.0f}},
                                         {{0.2f, 0.0f, 0.0f}}, {{5.0f, 0.0f, 0.0f}},
                                         {{5.1f, 0.0f, 0.0f}}, {{5.2f, 0.0f, 0.0f}}};
  std::vector<FPDS_t> k(points.size(), 2.0);
  std::vector<FPDS_t> epsilon(points.size(), 0.21);
  std::vector<INT32_t> labels(points.size(), 0);
  std::vector<INT32_t> types(points.size(), 0);

  dbscan_adaptiveK(
    points.data(), k.data(), epsilon.data(), labels.data(), types.data(),
    static_cast<INT32_t>(points.size()), mode);

  EXPECT_EQ(labels, (std::vector<INT32_t>{1, 1, 1, 2, 2, 2})) << mode;
  const auto core = core_cluster(points, k, epsilon, mode);
  EXPECT_EQ(core.labels, labels) << mode;
  EXPECT_EQ(core.types, types) << mode;
}

TEST_P(OneAPINeighborSearch, clusters_separated_by_y_and_z)
{
  const std::string mode = GetParam();
  std::vector<LiDAR_data_3D_t> points = {
    {{0.0f, 1.0f, 2.0f}}, {{0.1f, 1.1f, 2.1f}}, {{0.2f, 1.2f, 2.2f}},
    {{0.0f, 5.0f, 2.0f}}, {{0.1f, 5.1f, 2.1f}}, {{0.2f, 5.2f, 2.2f}},
    {{0.0f, 1.0f, 7.0f}}, {{0.1f, 1.1f, 7.1f}}, {{0.2f, 1.2f, 7.2f}}};
  std::vector<FPDS_t> k(points.size(), 2.0);
  std::vector<FPDS_t> epsilon(points.size(), 0.36);
  std::vector<INT32_t> labels(points.size(), 0);
  std::vector<INT32_t> types(points.size(), 0);

  dbscan_adaptiveK(
    points.data(), k.data(), epsilon.data(), labels.data(), types.data(),
    static_cast<INT32_t>(points.size()), mode);

  EXPECT_EQ(labels, (std::vector<INT32_t>{1, 1, 1, 2, 2, 2, 3, 3, 3})) << mode;
  const auto core = core_cluster(points, k, epsilon, mode);
  EXPECT_EQ(core.labels, labels) << mode;
  EXPECT_EQ(core.types, types) << mode;
}

TEST(OneAPICoreCharacterization, pcl_boundary_and_earlier_noise_types)
{
  std::vector<LiDAR_data_3D_t> boundary = {{{0.0f, 0, 0}}, {{0.5f, 0, 0}}, {{1.0f, 0, 0}}};
  auto result = core_cluster(boundary, {2, 2, 2}, {0.5, 0.5, 0.5}, "pcl_kdtree");
  EXPECT_EQ(result.labels, (std::vector<int>{-1, -1, -1}));

  std::vector<LiDAR_data_3D_t> noise = {{{0.0f, 0, 0}}, {{0.4f, 0, 0}}, {{0.8f, 0, 0}}};
  result = core_cluster(noise, {1, 1, 1}, {0.1, 0.5, 0.1}, "pcl_kdtree");
  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 1}));
  EXPECT_EQ(result.types[0], -1);
}

TEST_P(OneAPINeighborSearch, heterogeneous_radii_preserve_noise_relabeling)
{
  const std::string mode = GetParam();
  std::vector<LiDAR_data_3D_t> points = {{{0.0f, 0, 0}}, {{0.4f, 0, 0}}, {{0.8f, 0, 0}}};
  std::vector<FPDS_t> k = {1, 1, 1};
  std::vector<FPDS_t> radius = {0.1, 0.5, 0.1};
  std::vector<INT32_t> labels(points.size(), 0);
  std::vector<INT32_t> types(points.size(), 0);

  dbscan_adaptiveK(
    points.data(), k.data(), radius.data(), labels.data(), types.data(), points.size(), mode);
  const auto core = core_cluster(points, k, radius, mode);
  EXPECT_EQ(core.labels, labels) << mode;
  EXPECT_EQ(core.types, types) << mode;
}

TEST_P(OneAPINeighborSearch, frame_api_uses_selected_backend)
{
  const std::vector<adbscan_core::Point> points = {{0.0f, 0, 0}, {0.1f, 0, 0}, {0.2f, 0, 0}};
  adbscan_core::FrameOptions options;
  options.base = 2;

  const auto result = adbscan_core::cluster_frame(points, options, core_backend(GetParam()));
  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 1}));
  ASSERT_EQ(result.clusters.size(), 1U);
  EXPECT_EQ(result.clusters[0].point_count, points.size());
}

TEST(OneAPICharacterization, octree_preserves_legacy_dense_neighborhood_cap)
{
  constexpr int point_count = 334;
  std::vector<LiDAR_data_3D_t> points(point_count);
  for (int index = 0; index < point_count; ++index) {
    points[index] = {
      {static_cast<float>(index % 2) * 0.001f, static_cast<float>(index % 3) * 0.001f,
       static_cast<float>(index % 5) * 0.001f}};
  }
  std::vector<FPDS_t> k(point_count, 333.0);
  std::vector<FPDS_t> epsilon(point_count, 0.1);
  std::vector<INT32_t> types(point_count, 0);
  std::vector<INT32_t> labels(point_count, 0);

  dbscan_adaptiveK(
    points.data(), k.data(), epsilon.data(), labels.data(), types.data(), point_count,
    "pcl_kdtree");
  EXPECT_EQ(labels, std::vector<INT32_t>(point_count, 1));

  std::fill(labels.begin(), labels.end(), 0);
  std::fill(types.begin(), types.end(), 0);
  dbscan_adaptiveK(
    points.data(), k.data(), epsilon.data(), labels.data(), types.data(), point_count,
    "oneapi_octree");
  EXPECT_EQ(labels, std::vector<INT32_t>(point_count, -1));
  const auto core = core_cluster(points, k, epsilon, "oneapi_octree");
  EXPECT_EQ(core.labels, labels);
  EXPECT_EQ(core.types, types);
  EXPECT_TRUE(core.clusters.empty());
}

INSTANTIATE_TEST_SUITE_P(
  Modes, OneAPINeighborSearch, ::testing::Values("pcl_kdtree", "oneapi_kdtree", "oneapi_octree"));
