<!--
Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# Troubleshooting

Problems seen while running these workshops, and their fixes. Workshop names below are placeholders: `<w>` is the workshop, `<sdk>` is the SDK that declares the plug (`robotics-ai-suite` for the ROS workshops, `intel-accel` for accel-check).

## Workshop won't start after a host reboot: "Failed detecting requested GPU device"

```
error: ... Start "<w>" workshop (Failed starting device "robotics-ai-suite_gpu_intel_1": Failed detecting requested GPU device)
```

Workshop records the GPU by its DRM card number when the `gpu` plug is connected. The card number isn't stable across boots: after a reboot the same GPU can come up as `card0` instead of `card1`, and then every workshop with a `gpu` plug fails to start. Check with `ls /dev/dri`. Reconnect the plug, then start:

```bash
workshop disconnect <w>/<sdk>:gpu
workshop connect <w>/<sdk>:gpu
workshop start <w>
```

Do this for each workshop; it's safe on stopped workshops.

## GUI action fails: "Cannot open X display"

A `desktop` connection keeps the host's X display from when it was connected. If the display changes (for example `:0` to `:1` after logging out and back in), windows in the workshop fail to open. Every action that opens a window checks the display first and prints:

```bash
workshop disconnect <w>/robotics-ai-suite:desktop
workshop connect <w>/robotics-ai-suite:desktop
```

The `desktop` plug also needs a local desktop session on the host; it doesn't work over SSH without one.

## NPU missing in the workshop

`/dev/accel` is empty, or OpenVINO lists only `CPU` and `GPU`. The `npu` plug is a `custom-device` plug, which never connects automatically:

```bash
workshop connect <w>/<sdk>:npu :custom-device
workshop connections <w>                         # npu -> system:custom-device
```

Reconnect after `workshop restore`, which resets connections. On Humble the NPU is visible but OpenVINO can't use it; see [Known issues on Humble](humble-and-interop.md#known-issues-on-humble).

## Launch or refresh fails in a hook

```
error: cannot perform the following tasks:
- Run hook "setup-project" for "robotics-ai-suite" SDK (command exit code 100)
```

Find the failing command in the change log:

```bash
workshop changes                 # find the change ID with status Error
workshop tasks <id> | grep -B2 -A2 -E "^E:|ERROR"
```

Exit code 100 is apt. A `502 Bad Gateway` or `Failed to fetch` is usually a passing proxy error; launch again. To keep the container and fix the problem in place, launch with `workshop launch --wait-on-error <w>`, then resume or abort the launch.

## Downloads fail or time out (proxy)

The `proxy` SDK sets the host's proxy for apt, curl, wget, cargo and dirmngr. It must come first in the definition's `sdks:` list so later hooks use it.

- Behind a proxy, run `src/workshops/scripts/configure-proxy.sh` on the host (with `http_proxy` and `https_proxy` set), then `workshop refresh <name>`. The launch log shows `proxy: no proxy.env, using a direct connection` when the file is missing.
- On a direct connection, run the script with no proxy set: it removes `proxy.env`. Then `workshop refresh <name>`.
- Hooks don't load `/etc/environment`. Hook commands that need the proxy start with `set -a; . /etc/environment; set +a`. `workshop run` and `workshop exec` sessions already have the proxy variables.
- `sudo` drops the proxy variables; use `sudo -E`.

## Storage pool full

Workshop's `workshop` pool was 28.6 GiB on our host before growing it, and each workshop with the full suite uses about 5 GiB once installed, more while installing. Check and grow the pool:

```bash
lxc storage info workshop --project workshop.intel | grep space
lxc storage set workshop size=150GiB
```

Remove workshops you don't use with `workshop remove <w>`.

## Something I built in the workshop is gone

`workshop refresh` rebuilds the container when an SDK changed, which discards anything built or configured inside it: `~/ov_ws` from `build-npu-fix`, or SSH pairing from `src/workshops/scripts/authorize-remote.sh`. Run those steps again. See [What a refresh keeps](architecture.md#what-a-refresh-keeps).

## ROS processes keep running after `workshop run` is killed

Killing the host-side `workshop run` (or closing its terminal) doesn't stop the processes in the container. Stop actions with Ctrl+C. To clean up:

```bash
workshop exec <w> -- pkill -INT -f "[r]os2 launch"
workshop exec <w> -- bash -c 'pkill -9 -f "[g]z sim|[g]zserver|[g]zclient|[r]viz2" || true'
```

The brackets keep `pkill -f` from matching the shell that `workshop exec` runs it in.

Leftover nodes publish on the same domain and confuse the next run.

## No topics between workshops or with the host

- Check the domain ID on both sides. Actions use domain 0; `workshop shell` and the host use a random value from `~/.bashrc`. See [ROS 2 from the host](humble-and-interop.md#ros-2-from-the-host).
- Humble and Jazzy on one domain mostly work, but some types don't; see [Humble and Jazzy on the same domain](humble-and-interop.md#humble-and-jazzy-on-the-same-domain).

## Cyclone DDS fails to start in a Jazzy workshop

The Jazzy suite sets `CYCLONEDDS_URI` to `file:///usr/local/share/rvc/cyclonedds.xml`, which doesn't exist. With `RMW_IMPLEMENTATION=rmw_cyclonedds_cpp`, every node fails:

```
can't open configuration file file:///usr/local/share/rvc/cyclonedds.xml
[ERROR] [rmw_cyclonedds_cpp]: rmw_create_node: failed to create domain, error Error
```

Unset it first:

```bash
unset CYCLONEDDS_URI
```

The default middleware (Fast DDS) is not affected.

## PCL 1.15 libraries missing on Humble

`groundfloor-humble` and `adbscan-humble` exit with:

```
error while loading shared libraries: libpcl_common.so.1.15: cannot open shared object file
```

The Humble packages link PCL 1.15 but don't depend on it. A packaging fix is in progress. Until then, install the libraries from the AMR repo:

```bash
workshop exec <w>-humble -- bash -c 'set -a; . /etc/environment; set +a; sudo -E apt-get install -y libpcl-common1.15 libpcl-filters1.15 libpcl-kdtree1.15 libpcl-segmentation1.15'
```

A rebuild (`workshop refresh` after an SDK change) removes them; install them again afterwards.

## `workshop run` returns 1 although the action succeeded

An explicit `exit` in an action makes `workshop run` return 1, even `exit 0`. End the action with a command that carries the status, for example `(exit $RC)`.

## Warning: "base: ubuntu@20.04 are no longer supported"

Workshop still launches 20.04 workshops (used by [ros1-bridge](../ros1-bridge/README.md)) but warns that they need 22.04 or later before the next Workshop release. Acknowledge with `workshop okay`.

## "Active shell sessions may need restarting"

Shown after connecting a plug. `workshop run` starts a new session, so actions see the connection immediately; restart only open `workshop shell` sessions.
