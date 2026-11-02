#include <gtest/gtest.h>

#include <stdexcept>

#include "adbscan_core/clusterer.hpp"

TEST(ADBSCANCoreTest, separated_clusters_and_noise)
{
  const auto result = adbscan_core::cluster(
    {{0.0f, 0, 0},
     {0.1f, 0, 0},
     {0.2f, 0, 0},
     {5.0f, 0, 0},
     {5.1f, 0, 0},
     {5.2f, 0, 0},
     {10.0f, 0, 0}},
    std::vector<double>(7, 2.0), std::vector<double>(7, 0.21));

  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 1, 2, 2, 2, -1}));
  EXPECT_EQ(result.types, (std::vector<int>{1, 1, 1, 1, 1, 1, -1}));
  ASSERT_EQ(result.clusters.size(), 2u);
  EXPECT_EQ(result.clusters[0].id, 1);
  EXPECT_EQ(result.clusters[1].id, 2);
}

TEST(ADBSCANCoreTest, border_points_join_core_cluster)
{
  const auto result = adbscan_core::cluster(
    {{0.0f, 0, 0}, {0.2f, 0, 0}, {0.4f, 0, 0}}, {2, 2, 2}, {0.21, 0.21, 0.21});

  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 1}));
  EXPECT_EQ(result.types, (std::vector<int>{0, 1, 0}));
}

TEST(ADBSCANCoreCharacterization, shared_border_expands_and_merges_clusters)
{
  const std::vector<adbscan_core::Point> points = {{0.0f, 0, 0},  {0.05f, 0, 0}, {0.10f, 0, 0},
                                                   {0.15f, 0, 0}, {0.6f, 0, 0},  {1.05f, 0, 0},
                                                   {1.10f, 0, 0}, {1.15f, 0, 0}, {1.20f, 0, 0}};
  const auto result = adbscan_core::cluster(
    points, std::vector<double>(points.size(), 3.0), std::vector<double>(points.size(), 0.46));

  EXPECT_EQ(result.labels, std::vector<int>(points.size(), 1));
  EXPECT_EQ(result.types[4], 0);
  ASSERT_EQ(result.clusters.size(), 1u);
  EXPECT_EQ(result.clusters[0].point_count, points.size());
  EXPECT_NEAR(result.clusters[0].centroid.x, 0.6, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_center.x, 0.6, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_size.x, 1.2, 1e-6);
}

TEST(ADBSCANCoreCharacterization, radius_boundary_is_inclusive)
{
  const std::vector<adbscan_core::Point> points = {{0.0f, 0, 0}, {0.5f, 0, 0}, {1.0f, 0, 0}};
  const auto on_boundary = adbscan_core::cluster(points, {2, 2, 2}, {0.5, 0.5, 0.5});
  const auto outside_boundary = adbscan_core::cluster(points, {2, 2, 2}, {0.49, 0.49, 0.49});

  EXPECT_EQ(on_boundary.labels, (std::vector<int>{1, 1, 1}));
  EXPECT_EQ(outside_boundary.labels, (std::vector<int>{-1, -1, -1}));
}

TEST(ADBSCANCoreCharacterization, seed_radius_reclassifies_earlier_noise_as_core)
{
  const auto result =
    adbscan_core::cluster({{0.0f, 0, 0}, {0.4f, 0, 0}, {0.8f, 0, 0}}, {1, 1, 1}, {0.1, 0.5, 0.1});

  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 1}));
  EXPECT_EQ(result.types[0], 1);
  EXPECT_EQ(result.types[1], 1);
}

TEST(ADBSCANCoreCharacterization, seed_k_reclassifies_higher_threshold_neighbors_as_core)
{
  const auto result = adbscan_core::cluster(
    {{0.0f, 0, 0}, {0.1f, 0, 0}, {0.2f, 0, 0}}, {4, 2, 4}, {0.21, 0.21, 0.21});

  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 1}));
  EXPECT_EQ(result.types, (std::vector<int>{1, 1, 1}));
}

TEST(ADBSCANCoreTest, returns_mean_centroid_and_box_center_separately)
{
  const auto result =
    adbscan_core::cluster({{0.0f, 0, 0}, {0.1f, 0, 0}, {0.4f, 0, 0}}, {1, 1, 1}, {0.5, 0.5, 0.5});

  ASSERT_EQ(result.clusters.size(), 1u);
  EXPECT_NEAR(result.clusters[0].centroid.x, 1.0 / 6.0, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_center.x, 0.2, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_size.x, 0.4, 1e-6);
}

TEST(ADBSCANCoreTest, empty_frame_and_invalid_lengths)
{
  const auto empty = adbscan_core::cluster({}, {}, {});
  EXPECT_TRUE(empty.labels.empty());
  EXPECT_TRUE(empty.clusters.empty());
  EXPECT_THROW(adbscan_core::cluster({{0, 0, 0}}, {}, {0.5}), std::invalid_argument);
}

TEST(ADBSCANCoreTest, optional_backend_availability_matches_build)
{
  for (const auto backend :
       {adbscan_core::Backend::PclKdTree, adbscan_core::Backend::OneApiKdTree,
        adbscan_core::Backend::OneApiOctree}) {
#ifdef ADBSCAN_CORE_WITH_ONEAPI
    // Empty inputs check compiled support without requiring a SYCL device.
    EXPECT_NO_THROW(adbscan_core::cluster({}, {}, {}, backend));
    EXPECT_NO_THROW(adbscan_core::cluster_frame({}, {}, backend));
#else
    EXPECT_THROW(adbscan_core::cluster({{0, 0, 0}}, {1}, {0.5}, backend), std::invalid_argument);
    EXPECT_THROW(adbscan_core::cluster({}, {}, {}, backend), std::invalid_argument);
    EXPECT_THROW(adbscan_core::cluster_frame({}, {}, backend), std::invalid_argument);
#endif
  }
}

TEST(ADBSCANCoreFrameTest, lidar3d_produces_two_obstacles)
{
  adbscan_core::FrameOptions options;
  options.base = 2;
  options.min_3d_epsilon = 0.5f;
  const auto result = adbscan_core::cluster_frame(
    {{1.0f, 0.0f, 0.0f},
     {1.1f, 0.1f, 0.1f},
     {1.2f, 0.2f, 0.2f},
     {5.0f, 1.0f, 1.0f},
     {5.1f, 1.1f, 1.1f},
     {5.2f, 1.2f, 1.2f}},
    options);

  ASSERT_EQ(result.clusters.size(), 2u);
  EXPECT_NEAR(result.clusters[0].box_center.x, 1.1, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_center.y, 0.1, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_center.z, 0.1, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_size.x, 0.2, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.x, 5.1, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.y, 1.1, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.z, 1.1, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_size.z, 0.2, 1e-6);
}

TEST(ADBSCANCoreFrameTest, planar_scan_produces_obstacles_in_scan_frame)
{
  adbscan_core::FrameOptions options;
  options.mode = adbscan_core::FrameMode::Planar;
  options.base = 2;
  options.scale_factor = 1;
  const auto result = adbscan_core::cluster_frame(
    {{1.0f, 0, 0}, {1.04f, 0, 0}, {1.08f, 0, 0}, {5.0f, 0, 0}, {5.04f, 0, 0}, {5.08f, 0, 0}},
    options);

  ASSERT_EQ(result.clusters.size(), 2u);
  EXPECT_NEAR(result.clusters[0].box_center.x, 1.04, 1e-6);
  EXPECT_FLOAT_EQ(result.clusters[0].box_center.y, 0);
  EXPECT_FLOAT_EQ(result.clusters[0].box_center.z, 0);
  EXPECT_NEAR(result.clusters[1].box_center.x, 5.04, 1e-6);
}

TEST(ADBSCANCoreFrameTest, depth_points_in_body_frame_produce_obstacles)
{
  adbscan_core::FrameOptions options;
  options.mode = adbscan_core::FrameMode::Depth;
  options.x_filter_back = 3;
  options.y_filter_left = 2;
  options.y_filter_right = -2;
  options.scale_factor = 1;
  options.base = 2;
  const auto result = adbscan_core::cluster_frame(
    {{1.0f, 0.0f, 0.5f},
     {1.1f, 0.1f, 0.6f},
     {1.2f, 0.2f, 0.7f},
     {2.0f, 1.0f, 0.5f},
     {2.1f, 1.1f, 0.6f},
     {2.2f, 1.2f, 0.7f}},
    options);

  ASSERT_EQ(result.clusters.size(), 2u);
  EXPECT_NEAR(result.clusters[0].box_center.x, 1.1, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_center.y, 0.1, 1e-6);
  EXPECT_NEAR(result.clusters[0].box_center.z, 0.6, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.x, 2.1, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.y, 1.1, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.z, 0.6, 1e-6);
}

TEST(ADBSCANCoreFrameTest, lidar3d_subsamples_before_optional_ground_filter)
{
  adbscan_core::FrameOptions options;
  options.subsample_ratio = 2;
  options.ground_removal = true;
  options.z_filter = 0;
  options.x_filter_back = -1;
  options.base = 1;
  options.min_3d_epsilon = 0.25f;
  const auto result = adbscan_core::cluster_frame(
    {{0.0f, 0, 1},
     {50, 0, 1},
     {0.1f, 0, -1},
     {50, 0, 1},
     {0.2f, 0, 1},
     {50, 0, 1},
     {5.0f, 0, 1},
     {50, 0, 1},
     {5.1f, 0, 1}},
    options);

  EXPECT_EQ(result.labels, (std::vector<int>{1, 1, 2, 2}));
  ASSERT_EQ(result.clusters.size(), 2u);
  EXPECT_NEAR(result.clusters[0].box_center.x, 0.1, 1e-6);
  EXPECT_NEAR(result.clusters[1].box_center.x, 5.05, 1e-6);
}

TEST(ADBSCANCoreFrameTest, rejects_zero_subsample_ratio)
{
  adbscan_core::FrameOptions options;
  options.subsample_ratio = 0;
  EXPECT_THROW(adbscan_core::cluster_frame({{0, 0, 0}}, options), std::invalid_argument);
}

TEST(ADBSCANCoreCharacterization, summaries_follow_first_appearance_after_noise_relabeling)
{
  const auto result = adbscan_core::cluster(
    {{0.0f, 0, 0},
     {10.0f, 0, 0},
     {10.1f, 0, 0},
     {10.2f, 0, 0},
     {0.1f, 0, 0},
     {0.2f, 0, 0},
     {0.3f, 0, 0}},
    std::vector<double>(7, 2), {0.01, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21});

  EXPECT_EQ(result.labels, (std::vector<int>{2, 1, 1, 1, 2, 2, 2}));
  ASSERT_EQ(result.clusters.size(), 2u);
  EXPECT_EQ(result.clusters[0].id, 2);
  EXPECT_EQ(result.clusters[1].id, 1);
}