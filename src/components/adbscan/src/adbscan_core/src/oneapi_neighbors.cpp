#include "adbscan_core/clusterer.hpp"

#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/oneapi/common/sycl_queue.h>
#include <pcl/oneapi/kdtree/kdtree_flann.h>
#include <pcl/oneapi/octree/octree.hpp>
#include <pcl/point_cloud.h>

#include <algorithm>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace adbscan_core
{

std::vector<std::vector<int>> find_neighbors(
  const std::vector<Point> & points, const std::vector<double> & radius, Backend backend)
{
  std::vector<std::vector<int>> neighbors(points.size());
  if (points.empty()) {
    return neighbors;
  }

  if (backend == Backend::OneApiOctree) {
    const int max_answers = 333;  // Preserve the legacy octree's per-query neighbor cap.
    std::vector<pcl::oneapi::Octree::PointType> cloud(points.size());
    std::vector<float> radiuses(points.size());
    for (std::size_t index = 0; index < points.size(); ++index) {
      cloud[index].x = points[index].x;
      cloud[index].y = points[index].y;
      cloud[index].z = points[index].z;
      radiuses[index] = radius[index];
    }

    auto & queue = pcl::oneapi::getDefaultQueue();
    pcl::oneapi::Octree::PointCloud cloud_device(cloud.size(), &queue);
    cloud_device.upload(cloud);
    pcl::oneapi::Octree octree;
    octree.setQueue(queue);
    octree.setCloud(cloud_device);
    octree.build();

    pcl::oneapi::Octree::Queries queries(cloud.size(), &queue);
    pcl::oneapi::Octree::Radiuses search_radiuses(radiuses.size(), &queue);
    queries.upload(cloud);
    search_radiuses.upload(radiuses);
    pcl::oneapi::NeighborIndices results(cloud.size(), max_answers, &queue);
    octree.radiusSearch(queries, search_radiuses, max_answers, results);

    std::vector<int> sizes;
    std::vector<int> indices;
    results.sizes.download(sizes);
    results.data.download(indices);
    for (std::size_t index = 0; index < points.size(); ++index) {
      const auto begin = indices.begin() + index * max_answers;
      neighbors[index].assign(begin, begin + sizes[index]);
    }
    return neighbors;
  }

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  cloud->points.resize(points.size());
  for (std::size_t index = 0; index < points.size(); ++index) {
    (*cloud)[index].x = points[index].x;
    (*cloud)[index].y = points[index].y;
    (*cloud)[index].z = points[index].z;
  }

  if (backend == Backend::PclKdTree) {
    pcl::KdTreeFLANN<pcl::PointXYZ> tree;
    tree.setInputCloud(cloud);
    for (std::size_t index = 0; index < points.size(); ++index) {
      pcl::PointXYZ query(points[index].x, points[index].y, points[index].z);
      std::vector<float> distances;
      tree.radiusSearch(query, static_cast<float>(radius[index]), neighbors[index], distances);
    }
  } else if (backend == Backend::OneApiKdTree) {
    pcl::oneapi::KdTreeFLANN<pcl::PointXYZ> tree(false);
    cloud->width = cloud->size();
    cloud->height = 1;
    tree.setInputCloud(cloud);

    std::unordered_map<float, std::pair<pcl::PointCloud<pcl::PointXYZ>, std::vector<int>>> groups;
    std::set<float> unique_radiuses;
    for (double value : radius) {
      unique_radiuses.insert(static_cast<float>(value));
    }
    if (unique_radiuses.size() > 1) {
      for (std::size_t index = 0; index < points.size(); ++index) {
        auto & group = groups[static_cast<float>(radius[index])];
        group.first.emplace_back(points[index].x, points[index].y, points[index].z);
        group.second.push_back(index);
      }
    } else {
      groups[*unique_radiuses.begin()].first = *cloud;
    }

    for (auto & entry : groups) {
      entry.second.first.width = entry.second.first.size();
      entry.second.first.height = 1;
      pcl::PointCloud<pcl::PointXYZ>::Ptr queries(
        new pcl::PointCloud<pcl::PointXYZ>(entry.second.first));
      std::vector<pcl::Indices> indices;
      std::vector<std::vector<float>> distances;
      tree.radiusSearch(queries, entry.first, indices, distances);
      for (std::size_t index = 0; index < indices.size(); ++index) {
        auto original = unique_radiuses.size() > 1 ? entry.second.second[index] : index;
        neighbors[original].assign(indices[index].begin(), indices[index].end());
      }
    }
  } else {
    throw std::invalid_argument("requested backend is not available in this build");
  }
  return neighbors;
}

int label_neighbors(
  const std::vector<std::vector<int>> & neighbors, const std::vector<double> & k, Result & result)
{
  std::vector<unsigned char> touched(neighbors.size(), 0);
  std::vector<int> active(neighbors.size(), 0);
  int next_cluster = 1;
  for (std::size_t seed = 0; seed < neighbors.size(); ++seed) {
    if (touched[seed]) {
      continue;
    }

    std::fill(active.begin(), active.end(), 0);
    std::queue<int> active_queue;
    for (int index : neighbors[seed]) {
      active[index] = 1;
      active_queue.push(index);
    }

    const auto length = neighbors[seed].size();
    if (length > 1 && length < k[seed] + 1) {
      result.types[seed] = -1;
      result.labels[seed] = -1;
    } else if (length == 1) {
      result.types[seed] = -1;
      result.labels[seed] = -1;
      touched[seed] = 1;
    } else if (length >= k[seed] + 1) {
      result.types[seed] = 1;
      result.labels[seed] = next_cluster;
      for (int index : neighbors[seed]) {
        result.labels[index] = next_cluster;
      }

      while (!active_queue.empty()) {
        const int index = active_queue.front();
        active_queue.pop();
        touched[index] = 1;
        active[index] = 0;

        const auto & nearby = neighbors[index];
        if (nearby.size() > 1) {
          result.types[index] = nearby.size() >= k[seed] + 1 ? 1 : 0;
          for (int neighbor : nearby) {
            result.labels[neighbor] = next_cluster;
            if (!touched[neighbor]) {
              touched[neighbor] = 1;
              if (!active[neighbor]) {
                active[neighbor] = 1;
                active_queue.push(neighbor);
              }
            }
          }
        }
      }
      ++next_cluster;
    }
  }
  return next_cluster;
}

}  // namespace adbscan_core