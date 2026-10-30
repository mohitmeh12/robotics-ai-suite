<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Troubleshooting Workshops

Problems seen while running the Robotics AI Suite workshops, and their fixes. In the commands below, `<w>` is the workshop name, and `<sdk>` is the SDK that declares the plug: `robotics-ai-suite` for the ROS 2 workshops, `intel-accel` for `accel-check`.

## Workshop won't start after a host reboot

```text
error: ... Start "<w>" workshop (Failed starting device "robotics-ai-suite_gpu_intel_1": Failed detecting requested GPU device)
```

Workshop records the GPU by its DRM card number when the `gpu` plug is connected. The card number can change between boots: after a reboot, the same GPU can come up as `card0` instead of `card1`, and every workshop with a `gpu` plug then fails to start. Check with `ls /dev/dri`. Reconnect the plug, then start the workshop:

```bash
workshop disconnect <w>/<sdk>:gpu
workshop connect <w>/<sdk>:gpu
workshop start <w>
```

Do this for each workshop. It is safe on stopped workshops.

## GUI action fails with "Cannot open X display"

A `desktop` connection keeps the host's X display from when it was connected. If the display changes, for example from `:0` to `:1` after logging out and back in, windows in the workshop fail to open. Actions that open a window check the display first and print the commands to reconnect:

```bash
workshop disconnect <w>/robotics-ai-suite:desktop
workshop connect <w>/robotics-ai-suite:desktop
```

The `desktop` plug needs a local desktop session on the host. It doesn't work over SSH without one.

## NPU missing in the workshop

`/dev/accel` is empty in the workshop, or OpenVINO™ lists only `CPU` and `GPU`. The `npu` plug is a `custom-device` plug, which never connects automatically:

```bash
workshop connect <w>/<sdk>:npu :custom-device
workshop connections <w>
```

`workshop connections` should show `npu` connected to `system:custom-device`. Reconnect after `workshop restore`, which resets connections. On Humble the NPU is visible, but OpenVINO™ can't use it; see [Known issues on Humble](ros-distros.md#known-issues-on-humble).

## Launch or refresh fails in a hook

```text
error: cannot perform the following tasks:
- Run hook "setup-project" for "robotics-ai-suite" SDK (command exit code 100)
```

Find the failing command in the change log:

```bash
workshop changes
workshop tasks <id> | grep -B2 -A2 -E "^E:|ERROR"
```

Use the change ID with status `Error` from `workshop changes`. Exit code 100 comes from apt. A `502 Bad Gateway` or `Failed to fetch` is usually a temporary proxy error; launch again. To keep the container and fix the problem in place, launch with `workshop launch --wait-on-error <w>`, then resume or abort the launch.

## Downloads fail or time out behind a proxy

The `proxy` SDK sets the host's proxy for apt, curl, wget, cargo and dirmngr. It must be first in the definition's `sdks:` list, so that later hooks use it.

- Behind a proxy, run `src/workshops/scripts/configure-proxy.sh` on the host with `http_proxy` and `https_proxy` set, then run `workshop refresh <w>`. When the proxy file is missing, the launch log shows `proxy: no proxy.env, using a direct connection`.
- On a direct connection, run the script with no proxy set; it removes the proxy file. Then run `workshop refresh <w>`.
- Hooks don't load `/etc/environment`. Hook commands that need the proxy start with `set -a; . /etc/environment; set +a`. `workshop run` and `workshop exec` sessions already have the proxy variables.
- `sudo` drops the proxy variables; use `sudo -E`.

## Storage pool full

Launches fail with `no space left on device`. Each workshop with the full suite uses about 5 GiB once installed, and more while installing. Check the free space in Workshop's pool and grow it:

```bash
lxc storage info workshop --project workshop.intel | grep space
lxc storage set workshop size=150GiB
```

Remove workshops you don't use with `workshop remove <w>`.

## Something built in the workshop is gone

`workshop refresh` rebuilds the container when an SDK changed, which discards anything built or configured inside it. Examples are `~/ov_ws` from the Object Detection workshop's `build-npu-fix`, the Multi-camera AI Demo's Python environment and models, and SSH pairing from `authorize-remote.sh`. Run those steps again. See [What a refresh keeps](how-it-works.md#what-a-refresh-keeps).

## ROS processes keep running after `workshop run` is killed

Killing the host-side `workshop run`, or closing its terminal, doesn't stop the processes in the container. Leftover nodes keep publishing on the same domain and confuse the next run. Stop actions with Ctrl+C. To clean up:

```bash
workshop exec <w> -- pkill -INT -f "[r]os2 launch"
workshop exec <w> -- bash -c 'pkill -9 -f "[g]z sim|[g]zserver|[g]zclient|[r]viz2" || true'
```

The brackets keep `pkill -f` from matching the shell that `workshop exec` runs it in.

## No topics between workshops or with the host

- Check the domain ID on both sides. Actions use domain 0; `workshop shell` and the host use a random value from `~/.bashrc`. See [ROS 2 from the host](ros-distros.md#ros-2-from-the-host).
- Humble and Jazzy on one domain mostly work, but some types don't; see [Humble and Jazzy on the same domain](ros-distros.md#humble-and-jazzy-on-the-same-domain).

## Cyclone DDS fails to start in a Jazzy workshop

The Jazzy suite sets `CYCLONEDDS_URI` to `file:///usr/local/share/rvc/cyclonedds.xml`, which doesn't exist. With `RMW_IMPLEMENTATION=rmw_cyclonedds_cpp`, every node fails:

```text
can't open configuration file file:///usr/local/share/rvc/cyclonedds.xml
[ERROR] [rmw_cyclonedds_cpp]: rmw_create_node: failed to create domain, error Error
```

Unset the variable first:

```bash
unset CYCLONEDDS_URI
```

The default middleware, Fast DDS, is not affected.

## PCL 1.15 libraries missing on Humble

`groundfloor-humble` and `adbscan-humble` exit with:

```text
error while loading shared libraries: libpcl_common.so.1.15: cannot open shared object file
```

The Humble packages link PCL 1.15 but don't declare it as a dependency. A packaging fix is in progress. Until it is released, install the libraries from Intel®'s AMR repository:

```bash
workshop exec <w>-humble -- bash -c 'set -a; . /etc/environment; set +a; sudo -E apt-get install -y libpcl-common1.15 libpcl-filters1.15 libpcl-kdtree1.15 libpcl-segmentation1.15'
```

A rebuild removes them; install them again afterwards.

## `workshop run` returns 1 although the action succeeded

An explicit `exit` in an action makes `workshop run` return 1, even `exit 0`. If you write your own actions, end them with a command that carries the status, for example `(exit $RC)`.

## Warning: "base: ubuntu@20.04 are no longer supported"

Workshop still launches Ubuntu 20.04 workshops, used by the [ROS 1 Bridge](ros1-bridge.md) workshop, but warns that they will need 22.04 or later before its next release. Acknowledge the warning with `workshop okay`.

## "Active shell sessions may need restarting"

Workshop shows this after a plug is connected. `workshop run` starts a new session, so actions see the connection immediately. Restart only open `workshop shell` sessions.
