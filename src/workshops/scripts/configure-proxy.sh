#!/usr/bin/env bash
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# Pass the host's proxy to the workshops: writes sdks/proxy/hooks/proxy.env (not tracked) from http_proxy,
# https_proxy and no_proxy. With no proxy set, removes the file so workshops connect directly.
# Run before launching; for existing workshops, `workshop refresh <name>` applies the change.
set -euo pipefail
ENV_FILE=$(cd "$(dirname "$0")/../sdks/proxy/hooks" && pwd)/proxy.env
HTTP=${http_proxy:-${HTTP_PROXY:-}}
HTTPS=${https_proxy:-${HTTPS_PROXY:-$HTTP}}
HTTP=${HTTP:-$HTTPS}
NO=${no_proxy:-${NO_PROXY:-}}

if [ -z "$HTTP" ]; then
  rm -f "$ENV_FILE"
  echo "No proxy set on the host: workshops will connect directly."
  exit 0
fi
# The workshop bridge and loopback must stay direct.
NO="localhost,127.0.0.0/8,10.0.0.0/8${NO:+,$NO}"
printf "HTTP_PROXY_URL='%s'\nHTTPS_PROXY_URL='%s'\nNO_PROXY_LIST='%s'\n" "$HTTP" "$HTTPS" "$NO" > "$ENV_FILE"
echo "Wrote $ENV_FILE:"
cat "$ENV_FILE"
