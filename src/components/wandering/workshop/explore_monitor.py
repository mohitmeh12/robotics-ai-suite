# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""Follow a wandering launch log; succeed once the explored area stops growing."""
import re
import sys
import time

LOG = sys.argv[1]
STABLE_SECS = float(sys.argv[2]) if len(sys.argv) > 2 else 120
TIMEOUT_SECS = float(sys.argv[3]) if len(sys.argv) > 3 else 1200

SPACE = re.compile(r"Space: Free: (\d+)%, Occupied: (\d+)%, Unexplored: (\d+)%")
DIED = re.compile(r"\[ERROR\] \[([\w.-]+)\]: process has died")


def elapsed():
    return time.time() - start


start = time.time()
while True:
    try:
        f = open(LOG)
        break
    except FileNotFoundError:
        time.sleep(1)

first = best = None
best_at = start
with f:
    while True:
        line = f.readline()
        if line:
            if m := DIED.search(line):
                sys.exit(f"FAIL after {elapsed():.0f} s: {m.group(1)} died")
            if m := SPACE.search(line):
                unexplored = int(m.group(3))
                if first is None:
                    first = best = unexplored
                    best_at = time.time()
                    print(f"[{elapsed():4.0f} s] exploring, unexplored {unexplored}%", flush=True)
                elif unexplored < best:
                    best, best_at = unexplored, time.time()
                    print(f"[{elapsed():4.0f} s] unexplored {unexplored}%", flush=True)
            continue
        # Only a new low counts as progress; the value jitters by a point as the map updates.
        if best is not None and time.time() - best_at >= STABLE_SECS:
            if best < first:
                print(f"PASS after {elapsed():.0f} s: unexplored {first}% -> {best}%,"
                      f" no further progress for {STABLE_SECS:.0f} s", flush=True)
                sys.exit(0)
            sys.exit(f"FAIL after {elapsed():.0f} s: unexplored stuck at {first}%")
        if elapsed() > TIMEOUT_SECS:
            sys.exit(f"FAIL: timeout after {TIMEOUT_SECS:.0f} s"
                     + (f", unexplored {first}% -> {best}%" if first is not None else ", explorer never reported"))
        time.sleep(1)
