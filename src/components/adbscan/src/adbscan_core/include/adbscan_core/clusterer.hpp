#ifndef ADBSCAN_CORE__CLUSTERER_HPP_
#define ADBSCAN_CORE__CLUSTERER_HPP_

#include <cstddef>
#include <vector>

namespace adbscan_core
{

/// A 3D Cartesian point in the sensor/body reference frame, with coordinates in meters.
struct Point
{
  /// X coordinate in meters.
  float x;
  /// Y coordinate in meters.
  float y;
  /// Z coordinate in meters.
  float z;
};

/// Geometric summary of a cluster with a positive label.
struct Cluster
{
  /// Positive cluster label assigned to the member points.
  int id;
  /// Number of points assigned to this cluster.
  std::size_t point_count;
  /// Arithmetic mean of the member point coordinates; not the bounding-box center.
  Point centroid;
  /// Center of the axis-aligned bounding box; not necessarily the centroid.
  Point box_center;
  /// X, Y, and Z extents of the axis-aligned bounding box, in meters.
  Point box_size;
};

/// Neighbor-search backend used for clustering.
enum class Backend
{
  /// Direct-scan CPU implementation; available in the default build.
  LegacyCpu,
  /// PCL KD-tree implementation; requires -DWITH_ONEAPI=ON and a SYCL runtime.
  PclKdTree,
  /// OneAPI KD-tree implementation; requires -DWITH_ONEAPI=ON and a SYCL runtime.
  OneApiKdTree,
  /// OneAPI octree implementation; requires -DWITH_ONEAPI=ON and a SYCL runtime.
  OneApiOctree
};

/// Sensor-specific preprocessing performed by cluster_frame().
enum class FrameMode
{
  /// 2D LiDAR scan; no subsampling or filtering.
  Planar,
  /// 3D LiDAR; subsampling and optional Z-based ground removal.
  Lidar3D,
  /// Depth points; subsampling, optional ground removal, and X/Y FOV filtering.
  Depth
};

/// Preprocessing and per-point threshold settings for cluster_frame().
struct FrameOptions
{
  /// Sensor frame processing mode.
  FrameMode mode = FrameMode::Lidar3D;
  /// Integer subsampling step for non-Planar modes; must be > 0 even in Planar mode.
  /// cluster_frame() throws std::invalid_argument when this is zero.
  unsigned int subsample_ratio = 1;
  /// Enable Z-based ground plane clipping in Lidar3D and Depth modes.
  bool ground_removal = false;
  /// Remove points with Z below this threshold when ground_removal is true.
  float z_filter = 0;
  /// Depth-mode upper X bound; points with X greater than this are removed.
  float x_filter_back = 0;
  /// Depth-mode lower Y bound; points with Y less than this are removed.
  float y_filter_right = 0;
  /// Depth-mode upper Y bound; points with Y greater than this are removed.
  float y_filter_left = 0;
  /// Constant term of k(x) = floor(max(1.0, coeff_2 * x^2 + coeff_1 * x + base)).
  float base = 0;
  /// Linear coefficient of the range-dependent minimum-points threshold k(x).
  float coeff_1 = 0;
  /// Quadratic coefficient of the range-dependent minimum-points threshold k(x).
  float coeff_2 = 0;
  /// Search-radius multiplier for Planar and Depth modes.
  float scale_factor = 0;
  /// Search-radius multiplier for Lidar3D mode.
  float epsilon_scale_factor = 0.9f;
  /// Minimum search radius in meters for Lidar3D mode.
  float min_3d_epsilon = 0.9f;
};

/// Per-point assignments and summaries produced by clustering.
struct Result
{
  /// Cluster label per processed point: -1 for noise, or a positive cluster ID (>= 1).
  std::vector<int> labels;
  /// Classification per processed point: -1 for noise, 0 for border, 1 for core.
  std::vector<int> types;
  /// One Cluster summary for each positive cluster label.
  std::vector<Cluster> clusters;
};

/**
 * @brief Cluster points using caller-supplied per-point thresholds.
 * @param points Input points in sensor/body coordinates, in meters.
 * @param k Per-point minimum neighbor-count thresholds; same length as points.
 * @param radius Per-point search radii in meters; same length as points.
 * @param backend Neighbor-search implementation to use.
 * @return Result with per-point labels and types and summaries of positive clusters.
 * @throws std::invalid_argument If points, k, and radius have different lengths, or the
 * requested backend is unsupported by this build.
 */
Result cluster(
  const std::vector<Point> & points, const std::vector<double> & k,
  const std::vector<double> & radius, Backend backend = Backend::LegacyCpu);

/**
 * @brief Preprocess a sensor frame, derive per-point thresholds, and cluster it.
 * @param points Input points in sensor/body coordinates, in meters.
 * @param options Frame preprocessing and threshold settings.
 * @param backend Neighbor-search implementation to use.
 * @return Result whose labels and types index the preprocessed points, not the original
 * points if subsampling or filtering was applied. Empty input returns an empty Result.
 * @throws std::invalid_argument If options.subsample_ratio is zero or the requested
 * backend is unsupported by this build.
 */
Result cluster_frame(
  const std::vector<Point> & points, const FrameOptions & options,
  Backend backend = Backend::LegacyCpu);

}  // namespace adbscan_core

#endif