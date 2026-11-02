#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "adbscan_core/clusterer.hpp"

struct Scenario
{
  const char * name;
  std::vector<adbscan_core::Point> points;
  adbscan_core::FrameOptions options;
};

static Scenario planar_frame()
{
  std::vector<adbscan_core::Point> points;
  points.reserve(720);
  for (int index = 0; index < 720; ++index) {
    const double angle = -1.5707963267948966 + 3.141592653589793 * index / 719;
    points.push_back(
      {static_cast<float>(3.0 * std::cos(angle)), static_cast<float>(3.0 * std::sin(angle)), 0});
  }
  adbscan_core::FrameOptions options;
  options.mode = adbscan_core::FrameMode::Planar;
  options.base = 5;
  options.scale_factor = 1;
  return {"planar", std::move(points), options};
}

static Scenario lidar3d_frame()
{
  std::vector<adbscan_core::Point> points;
  points.reserve(2048);
  for (int index = 0; index < 2048; ++index) {
    const int group = index / 128;
    const int member = index % 128;
    points.push_back(
      {1.0f + 3.0f * (group % 4) + 0.01f * (member % 8),
       3.0f * (group / 4) + 0.01f * ((member / 8) % 8), 0.5f + 0.01f * (member / 64)});
  }
  adbscan_core::FrameOptions options;
  options.base = 5;
  options.min_3d_epsilon = 0.3f;
  return {"lidar3d", std::move(points), options};
}

static Scenario depth_frame()
{
  std::vector<adbscan_core::Point> points;
  points.reserve(640 * 480);
  for (int index = 0; index < 640 * 480; ++index) {
    if (index % 150 != 0) {
      points.push_back({50.0f, 0.0f, -1.0f});
      continue;
    }
    const int group = (index / 150) / 128;
    const int member = (index / 150) % 128;
    points.push_back(
      {1.0f + 0.6f * (group % 4) + 0.01f * (member % 8),
       -0.9f + 0.6f * (group / 4) + 0.01f * ((member / 8) % 8), 0.5f + 0.02f * (member / 64)});
  }
  adbscan_core::FrameOptions options;
  options.mode = adbscan_core::FrameMode::Depth;
  options.subsample_ratio = 150;
  options.ground_removal = true;
  options.z_filter = 0;
  options.x_filter_back = 3;
  options.y_filter_right = -2;
  options.y_filter_left = 2;
  options.base = 5;
  options.scale_factor = 1;
  return {"depth", std::move(points), options};
}

int main(int argc, char ** argv)
{
  const int samples = argc > 1 ? std::stoi(argv[1]) : 20;
  const int warmups = argc > 2 ? std::stoi(argv[2]) : 3;
  if (samples < 1 || warmups < 0) {
    throw std::invalid_argument("samples must be positive and warmups nonnegative");
  }

  const std::vector<Scenario> scenarios = {planar_frame(), lidar3d_frame(), depth_frame()};
  const std::vector<std::pair<const char *, adbscan_core::Backend>> backends = {
    {"legacy_cpu", adbscan_core::Backend::LegacyCpu},
    {"pcl_kdtree", adbscan_core::Backend::PclKdTree},
    {"oneapi_kdtree", adbscan_core::Backend::OneApiKdTree},
    {"oneapi_octree", adbscan_core::Backend::OneApiOctree}};

  std::cout << "# warmups=" << warmups << " samples=" << samples << '\n';
  std::cout << "scenario,backend,input_points,processed_points,clusters,cold_ms,median_ms,p95_ms,"
               "frames_per_s\n";
  std::cout << std::fixed << std::setprecision(3);
  for (const auto & scenario : scenarios) {
    for (const auto & backend : backends) {
      std::size_t processed_points = 0;
      std::size_t clusters = 0;
      const auto run_frame = [&]() {
        const auto start = std::chrono::steady_clock::now();
        const auto result =
          adbscan_core::cluster_frame(scenario.points, scenario.options, backend.second);
        const auto end = std::chrono::steady_clock::now();
        processed_points = result.labels.size();
        clusters = result.clusters.size();
        if (processed_points == 0 || clusters == 0) {
          throw std::runtime_error("benchmark frame produced no clusters");
        }
        return std::chrono::duration<double, std::milli>(end - start).count();
      };

      const double cold_ms = run_frame();
      for (int index = 0; index < warmups; ++index) {
        run_frame();
      }
      std::vector<double> durations;
      durations.reserve(samples);
      double total_ms = 0;
      for (int index = 0; index < samples; ++index) {
        const double duration = run_frame();
        durations.push_back(duration);
        total_ms += duration;
      }
      std::sort(durations.begin(), durations.end());
      const double median_ms = samples % 2 == 0
                                 ? (durations[samples / 2 - 1] + durations[samples / 2]) / 2
                                 : durations[samples / 2];
      const double p95_ms = durations[static_cast<int>(std::ceil(samples * 0.95)) - 1];
      std::cout << scenario.name << ',' << backend.first << ',' << scenario.points.size() << ','
                << processed_points << ',' << clusters << ',' << cold_ms << ',' << median_ms << ','
                << p95_ms << ',' << samples * 1000.0 / total_ms << '\n';
    }
  }

  return 0;
}