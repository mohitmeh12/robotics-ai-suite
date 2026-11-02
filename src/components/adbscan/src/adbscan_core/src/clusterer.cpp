#include "adbscan_core/clusterer.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace adbscan_core
{

#ifdef ADBSCAN_CORE_WITH_ONEAPI
std::vector<std::vector<int>> find_neighbors(
  const std::vector<Point> & points, const std::vector<double> & radius, Backend backend);
int label_neighbors(
  const std::vector<std::vector<int>> & neighbors, const std::vector<double> & k, Result & result);
#endif

static Result summarize(const std::vector<Point> & points, Result result, int next_cluster);

Result cluster(
  const std::vector<Point> & points, const std::vector<double> & k,
  const std::vector<double> & radius, Backend backend)
{
  if (points.size() != k.size() || points.size() != radius.size()) {
    throw std::invalid_argument("points, k and radius must have equal lengths");
  }
  if (backend != Backend::LegacyCpu) {
#ifdef ADBSCAN_CORE_WITH_ONEAPI
    Result result{std::vector<int>(points.size(), 0), std::vector<int>(points.size(), 0), {}};
    const int next_cluster = label_neighbors(find_neighbors(points, radius, backend), k, result);
    return summarize(points, std::move(result), next_cluster);
#else
    throw std::invalid_argument("requested backend is not available in this build");
#endif
  }

  const auto count = points.size();
  Result result{std::vector<int>(count, 0), std::vector<int>(count, 0), {}};
  std::vector<unsigned char> touched(count, 0);
  std::vector<int> active(count, 0);
  std::vector<int> active_next(count, 0);
  int next_cluster = 1;

  for (std::size_t seed = 0; seed < count; ++seed) {
    if (touched[seed] != 0) {
      continue;
    }

    int active_count = 0;
    for (std::size_t index = 0; index < count; ++index) {
      const double delta_x = points[seed].x - points[index].x;
      const double delta_y = points[seed].y - points[index].y;
      const double delta_z = points[seed].z - points[index].z;
      const double distance = std::sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
      active[index] = distance <= radius[seed] ? 1 : 0;
      active_count += active[index];
    }

    if (active_count > 1 && active_count < k[seed] + 1) {
      result.types[seed] = -1;
      result.labels[seed] = -1;
    }
    if (active_count == 1) {
      result.types[seed] = -1;
      result.labels[seed] = -1;
      touched[seed] = 1;
    }
    if (active_count < k[seed] + 1) {
      continue;
    }

    result.types[seed] = 1;
    result.labels[seed] = next_cluster;
    for (std::size_t index = 0; index < count; ++index) {
      if (active[index]) {
        result.labels[index] = next_cluster;
      }
    }

    while (active_count > 0) {
      std::size_t index = 0;
      while (index < count && !active[index]) {
        ++index;
      }
      if (index == count) {
        break;
      }
      touched[index] = 1;
      active[index] = 0;
      --active_count;

      int neighbor_count = 0;
      for (std::size_t neighbor = 0; neighbor < count; ++neighbor) {
        const double delta_x = points[index].x - points[neighbor].x;
        const double delta_y = points[index].y - points[neighbor].y;
        const double delta_z = points[index].z - points[neighbor].z;
        const double distance =
          std::sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
        active_next[neighbor] = distance <= radius[seed] ? 1 : 0;
        neighbor_count += active_next[neighbor];
      }
      if (neighbor_count > 1) {
        result.types[index] = neighbor_count >= k[seed] + 1 ? 1 : 0;
        for (std::size_t neighbor = 0; neighbor < count; ++neighbor) {
          if (!active_next[neighbor]) {
            continue;
          }
          result.labels[neighbor] = next_cluster;
          if (!touched[neighbor]) {
            touched[neighbor] = 1;
            if (!active[neighbor]) {
              active[neighbor] = 1;
              ++active_count;
            }
          }
        }
      }
    }
    ++next_cluster;
  }

  return summarize(points, std::move(result), next_cluster);
}

static Result summarize(const std::vector<Point> & points, Result result, int next_cluster)
{
  const auto count = points.size();
  std::vector<unsigned char> cluster_seen(next_cluster, 0);
  std::vector<int> cluster_order;
  for (int label : result.labels) {
    if (label > 0 && !cluster_seen[label]) {
      cluster_seen[label] = 1;
      cluster_order.push_back(label);
    }
  }

  for (int id : cluster_order) {
    Cluster output{id, 0, {}, {}, {}};
    Point minimum{
      std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max()};
    Point maximum{
      std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest()};
    double sum_x = 0, sum_y = 0, sum_z = 0;
    for (std::size_t index = 0; index < count; ++index) {
      if (result.labels[index] != id) {
        continue;
      }
      ++output.point_count;
      const auto & point = points[index];
      sum_x += point.x;
      sum_y += point.y;
      sum_z += point.z;
      minimum.x = std::min(minimum.x, point.x);
      minimum.y = std::min(minimum.y, point.y);
      minimum.z = std::min(minimum.z, point.z);
      maximum.x = std::max(maximum.x, point.x);
      maximum.y = std::max(maximum.y, point.y);
      maximum.z = std::max(maximum.z, point.z);
    }
    if (output.point_count == 0) {
      continue;
    }
    output.centroid = {
      static_cast<float>(sum_x / output.point_count),
      static_cast<float>(sum_y / output.point_count),
      static_cast<float>(sum_z / output.point_count)};
    output.box_center = {
      (minimum.x + maximum.x) / 2, (minimum.y + maximum.y) / 2, (minimum.z + maximum.z) / 2};
    output.box_size = {maximum.x - minimum.x, maximum.y - minimum.y, maximum.z - minimum.z};
    result.clusters.push_back(output);
  }
  return result;
}

Result cluster_frame(
  const std::vector<Point> & points, const FrameOptions & options, Backend backend)
{
  if (options.subsample_ratio == 0) {
    throw std::invalid_argument("subsample_ratio must be positive");
  }
#ifndef ADBSCAN_CORE_WITH_ONEAPI
  if (backend != Backend::LegacyCpu) {
    throw std::invalid_argument("requested backend is not available in this build");
  }
#endif

  std::vector<Point> selected;
  if (options.mode == FrameMode::Planar) {
    selected = points;
  } else {
    for (std::size_t index = 0; index < points.size(); index += options.subsample_ratio) {
      const auto & point = points[index];
      if (options.ground_removal && point.z < options.z_filter) {
        continue;
      }
      if (
        options.mode == FrameMode::Depth &&
        (point.x > options.x_filter_back || point.y < options.y_filter_right ||
         point.y > options.y_filter_left)) {
        continue;
      }
      selected.push_back(point);
    }
  }

  if (selected.empty()) {
    return cluster({}, {}, {}, backend);
  }

  Point minimum = selected.front();
  Point maximum = selected.front();
  float minimum_x = 0;
  float maximum_x = 0;
  for (const auto & point : selected) {
    minimum.x = std::min(minimum.x, point.x);
    minimum.y = std::min(minimum.y, point.y);
    minimum.z = std::min(minimum.z, point.z);
    maximum.x = std::max(maximum.x, point.x);
    maximum.y = std::max(maximum.y, point.y);
    maximum.z = std::max(maximum.z, point.z);
    minimum_x = std::min(minimum_x, point.x);
    maximum_x = std::max(maximum_x, point.x);
  }

  const double product =
    static_cast<double>(maximum.x - minimum.x) * (maximum.y - minimum.y) * (maximum.z - minimum.z);
  const double denominator = selected.size() * std::sqrt(std::pow(3.1416, 3.0));
  std::vector<double> k(selected.size());
  std::vector<double> radius(selected.size());
  for (std::size_t index = 0; index < selected.size(); ++index) {
    const double x = selected[index].x;
    const double threshold = x * x * options.coeff_2 + x * options.coeff_1 + options.base;
    k[index] = std::floor(std::max(1.0, threshold));
    radius[index] = std::pow(product * k[index] * 1.3293 / denominator, 1.0 / 3.0);
    if (options.mode == FrameMode::Planar || options.mode == FrameMode::Depth) {
      radius[index] *= options.scale_factor;
      if (radius[index] <= 0) {
        radius[index] = 0.02 * (maximum_x - minimum_x);
      }
    } else {
      radius[index] *= options.epsilon_scale_factor;
      if (radius[index] < options.min_3d_epsilon) {
        radius[index] = options.min_3d_epsilon;
      }
    }
  }
  return cluster(selected, k, radius, backend);
}

}  // namespace adbscan_core