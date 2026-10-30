# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""Check that Prometheus scrapes the profiler exporter and Grafana serves the KPI dashboard."""
import base64
import json
import sys
import time
import urllib.request

# Local stack only; Grafana's default credentials come from the profiler's docker-compose.yml.
opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
AUTH = "Basic " + base64.b64encode(b"admin:admin").decode()


def get(url, auth=False):
    req = urllib.request.Request(url, headers={"Authorization": AUTH} if auth else {})
    with opener.open(req, timeout=5) as r:
        return json.load(r)


deadline = time.time() + 90
target = series = dashboards = None
while time.time() < deadline:
    try:
        targets = get("http://localhost:9090/api/v1/targets")["data"]["activeTargets"]
        target = next((t["health"] for t in targets if t["labels"].get("job") == "ros2-kpi"), None)
        result = get("http://localhost:9090/api/v1/query?query=count(%7Bjob%3D%22ros2-kpi%22%7D)")["data"]["result"]
        series = int(result[0]["value"][1]) if result else 0
        dashboards = [d["title"] for d in get("http://localhost:30000/api/search?type=dash-db", auth=True)]
        if target == "up" and series and dashboards:
            break
    except OSError:
        pass
    time.sleep(3)

print(f"Prometheus ros2-kpi target: {target}, {series} series; Grafana dashboards: {dashboards}")
sys.exit(0 if target == "up" and series and dashboards else "FAIL: dashboards not fed")
