# adbscan_core

Dependency-light ADBSCAN clustering library for XYZ points. This package does
not parse ROS messages, convert optical frames, or apply TF. The `LegacyCpu`
backend follows the existing direct-scan implementation's clustering and
point-type behavior. CPU-only builds reject other backend selections with
`std::invalid_argument`.

For a complete step-by-step developer guide with CMake configuration examples,
working C++ code, and sensor parameter tuning baselines, see [Developer Guide](DEV_GUIDE.md).

Include `<adbscan_core/clusterer.hpp>` and link `adbscan_core` after finding the
ament package. `cluster()` returns per-point `labels` and `types` (noise `-1`,
border `0`, core `1`), plus one summary per positive cluster label. Summaries
include both the arithmetic mean `centroid` and the axis-aligned `box_center`
and `box_size`; these centers are not interchangeable. Empty input returns an
empty result. Length mismatches throw `std::invalid_argument`.

`cluster_frame(points, options)` derives `k` and radius after applying the
legacy preprocessing for `Planar` (no subsampling or filters), `Lidar3D`
(subsampling, optional Z ground removal), or `Depth` (subsampling, optional Z
ground removal, X and Y filters). Supply depth points in body-frame coordinates;
the ROS adapter remains responsible for optical-frame conversion. Labels and
types index the processed points, not the original input. `subsample_ratio`
must be positive. For input already preprocessed by another application, call
`cluster()` with its own thresholds to avoid filtering twice. Unlike the legacy
frame wrapper, an empty frame yields no obstacle.

The `adbscan_ros2` subscriber uses the frame-level core path by default while
retaining its LaserScan/PointCloud2 conversion and obstacle publication. Set
`use_adbscan_core:=false` to use the previous implementation as a rollback.
In all builds, the core path defaults to `core_backend:=legacy_cpu`. In
`WITH_ONEAPI=ON` builds, set `core_backend:=pcl_kdtree`, `oneapi_kdtree`, or
`oneapi_octree` to use an optional backend. Unsupported or unknown core backend
values fail explicitly. The separate `oneapi_library` parameter continues to
control the subscriber's previous implementation; it does not select the core
backend. The follow-me package calls `cluster()` on its own preprocessed points
and thresholds, retaining its obstacle grouping and target tracking. The main
subscriber maps empty processed frames to the existing zero-sized obstacle;
`cluster_frame()` itself returns an empty result. On the core path its
benchmarking output places the whole core call in the clustering time bucket,
so individual stage timings are not directly comparable with the legacy path.
Applications can select `PclKdTree`, `OneApiKdTree`, or `OneApiOctree` via
`cluster()` or `cluster_frame()` when building this package with
`-DWITH_ONEAPI=ON`. This opt-in build requires IntelLLVM (`icpx`), SYCL, PCL
OneAPI (including kdtree and octree), and FLANN OneAPI headers. The default
build does not require them. For example, in a ROS 2 workspace with these
dependencies installed:

```sh
colcon build --packages-select adbscan_core adbscan_ros2 --cmake-args -DWITH_ONEAPI=ON -DCMAKE_CXX_COMPILER=icpx
```

For Debian packages on a prepared build system, use
`make package ROS_DISTRO=jazzy`. This packages both `adbscan_core` and
`adbscan_ros2` with OneAPI support by default (using `icpx`). Humble packages
default to CPU-only; set `WITH_ONEAPI=OFF` for CPU-only Jazzy packages. The
accelerated core package depends on the PCL OneAPI KD-tree and octree runtime
libraries.

Both packages must be built with the same option. A SYCL runtime and matching
device drivers must be installed; for GPU runs, set
`ONEAPI_DEVICE_SELECTOR=level_zero:gpu` before starting a process that selects
`oneapi_kdtree` or `oneapi_octree`. For SYCL CPU runs use `opencl:cpu`. The
selector does not enable OneAPI backends on its own. The Intel OneAPI UMF library
directory
must also be available to the dynamic linker. The ROS synthetic parity tests
accept `ADBSCAN_TEST_BACKEND=pcl_kdtree`, `oneapi_kdtree`, or `oneapi_octree`
to compare the selected legacy and core subscriber paths on planar, 3D, and
depth point clouds.

The standalone tests under `tests/oneapi` compare all three optional modes
against the existing ROS implementation. Build them with IntelLLVM and run
with `ONEAPI_DEVICE_SELECTOR=opencl:cpu` or `level_zero:gpu`. The octree's
333-neighbor limit is retained for legacy parity; its dense-neighborhood
characterization runs with the standalone OneAPI suite. The package's core
tests check optional backend availability against `WITH_ONEAPI`, using empty
inputs so that this check does not require a SYCL device. Nonempty accelerated
parity cases remain in the standalone suite. Use PCL OneAPI packages with
the corrected octree Y/Z row stride when running these parity tests.
