# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

SHELL := bash -eu -o pipefail

COMMIT := $(shell git rev-parse --short HEAD)
DATE := $(shell git log -1 --format=%cd --date=format:"%Y%m%d")
VERSION := $(COMMIT)-$(DATE)

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

ROS_DISTRO ?= humble

# Map ROS_DISTRO to Ubuntu codename and OpenVINO distro tag
ifeq ($(ROS_DISTRO),humble)
	UBUNTU_CODENAME := jammy
	OPENVINO_DISTRO := ubuntu22
else
	UBUNTU_CODENAME := noble
	OPENVINO_DISTRO := ubuntu24
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	MAKE_JOBS ?= $(shell sysctl -n hw.ncpu)
else
	MAKE_JOBS ?= $(shell nproc)
endif

.DEFAULT_GOAL := help
.PHONY: build clean help license-check lint lint-all lint-bash lint-clang lint-python lint-yaml lint-githubactions lint-markdown lint-json package test source-package debian-build-deps install-debian-build-deps openvino-apt-source container-package container-test

PROJECT_DEFINITION ?= $(ROOT_DIR)/robotics-project.json
PACKAGE_VERSION_SUFFIX ?=
PACKAGE_COMPONENTS := $(shell PROJECT_DEFINITION="$(PROJECT_DEFINITION)" ROOT_DIR="$(ROOT_DIR)" python3 -c 'import json, os, pathlib, re; project = json.loads(pathlib.Path(os.environ["PROJECT_DEFINITION"]).read_text(encoding="utf-8")); components = project.get("ci", {}).get("package_components", []); root = pathlib.Path(os.environ["ROOT_DIR"]) / "src"; valid = project.get("format_version") == 1 and isinstance(components, list) and components and all(isinstance(component, dict) and isinstance(component.get("path"), str) and re.fullmatch(r"[a-z0-9][a-z0-9_-]*", component["path"]) and (root / component["path"]).is_dir() and ("ros_package" not in component or isinstance(component["ros_package"], str) and re.fullmatch(r"[a-z0-9][a-z0-9_]*", component["ros_package"])) for component in components); (_ for _ in ()).throw(SystemExit("Invalid package components in project definition")) if not valid else None; print(",".join(component["path"] for component in components))')
ROS_PACKAGES := $(shell PROJECT_DEFINITION="$(PROJECT_DEFINITION)" python3 -c 'import json, os, pathlib, re; project = json.loads(pathlib.Path(os.environ["PROJECT_DEFINITION"]).read_text(encoding="utf-8")); components = project.get("ci", {}).get("package_components", []); ros_components = [component for component in components if "ros_package" in component]; valid = project.get("format_version") == 1 and isinstance(components, list) and ros_components and all(isinstance(component, dict) and isinstance(component.get("ros_package"), str) and re.fullmatch(r"[a-z0-9][a-z0-9_]*", component["ros_package"]) for component in ros_components); (_ for _ in ()).throw(SystemExit("Invalid ROS packages in project definition")) if not valid else None; print(" ".join(component["ros_package"] for component in ros_components))')
# Colcon base path that contains the ROS package sources
COLCON_BASE_PATHS := src
PACKAGE_OUTPUT_DIR = $(ROOT_DIR)/build/debian-packages/packages
BUILD_DEPENDS_OUTPUT_DIR = $(ROOT_DIR)/build/debian-build-deps
MISSING_BUILD_DEPENDENCIES_FILE = $(BUILD_DEPENDS_OUTPUT_DIR)/missing-dependencies.txt
CONTAINER_IMAGE ?= amd64/ros:$(ROS_DISTRO)-ros-base
# Forward host proxy settings so container builds can reach the Intel OpenVINO APT repo
DOCKER_PROXY_ENV := -e HTTP_PROXY -e HTTPS_PROXY -e NO_PROXY -e http_proxy -e https_proxy -e no_proxy

license-check:
	@# Help: Perform a REUSE license check using docker container https://hub.docker.com/r/fsfe/reuse
	docker run --rm --volume ${ROOT_DIR}:/data fsfe/reuse:5.0.2 lint

openvino-apt-source:
	@# Help: Configure the Intel OpenVINO APT repository (needed to resolve the openvino build dependency)
	@if ! apt-cache policy openvino 2>/dev/null | grep -q Candidate; then \
		echo "Configuring Intel OpenVINO APT repository for $(OPENVINO_DISTRO)"; \
		curl -fsSL https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \
			| gpg --dearmor | tee /usr/share/keyrings/openvino-archive-keyring.gpg > /dev/null; \
		echo "deb [signed-by=/usr/share/keyrings/openvino-archive-keyring.gpg] https://apt.repos.intel.com/openvino $(OPENVINO_DISTRO) main" \
			| tee /etc/apt/sources.list.d/intel-openvino.list; \
		apt-get update -qq; \
	fi

package:
	@# Help: Build native Debian packages using CMake + CPack
	command -v ninja >/dev/null || { echo "Error: Ninja is required. Install ninja-build." >&2; exit 1; }
	rm -rf $(ROOT_DIR)/build/debian-packages
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	cmake -G Ninja -S $(ROOT_DIR) -B $(ROOT_DIR)/build/debian-packages \
		-DROS_DISTRO=$(ROS_DISTRO) \
		-DOBJECTDETECTION_PACKAGE_OUTPUT_DIR=$(PACKAGE_OUTPUT_DIR) \
		-DOBJECTDETECTION_PACKAGE_COMPONENTS="$(PACKAGE_COMPONENTS)" \
		-DOBJECTDETECTION_PACKAGE_VERSION_SUFFIX=$(PACKAGE_VERSION_SUFFIX) && \
	cmake --build $(ROOT_DIR)/build/debian-packages --target package

debian-build-deps:
	@# Help: Generate Debian Build-Depends control files without configuring components
	rm -rf $(BUILD_DEPENDS_OUTPUT_DIR) $(ROOT_DIR)/build/debian-build-deps-config
	cmake -S $(ROOT_DIR) -B $(ROOT_DIR)/build/debian-build-deps-config \
		-DROS_DISTRO=$(ROS_DISTRO) \
		-DOBJECTDETECTION_PACKAGE_COMPONENTS="$(PACKAGE_COMPONENTS)" \
		-DOBJECTDETECTION_BUILD_DEPENDS_OUTPUT_DIR=$(BUILD_DEPENDS_OUTPUT_DIR) && \
	cmake --build $(ROOT_DIR)/build/debian-build-deps-config --target debian-build-deps

install-debian-build-deps: openvino-apt-source debian-build-deps
	@# Help: Install generated Debian Build-Depends with APT
	@command -v mk-build-deps >/dev/null && command -v dpkg-checkbuilddeps >/dev/null || { echo "Error: mk-build-deps and dpkg-checkbuilddeps are required. Install devscripts, equivs, and dpkg-dev." >&2; exit 1; }
	@apt-get update; \
	install_status=0; \
	for control_file in $$(find $(BUILD_DEPENDS_OUTPUT_DIR) -name control -path '*/debian/control'); do \
		mk-build-deps --install --remove \
			--tool 'apt-get --no-install-recommends --no-remove --yes' \
			"$$control_file" || install_status=1; \
	done; \
	: > $(MISSING_BUILD_DEPENDENCIES_FILE); \
	for control_file in $$(find $(BUILD_DEPENDS_OUTPUT_DIR) -name control -path '*/debian/control'); do \
		if missing_dependencies="$$(dpkg-checkbuilddeps "$$control_file" 2>&1)"; then \
			continue; \
		fi; \
		printf '%s\n%s\n\n' "$$control_file" "$$missing_dependencies" >> $(MISSING_BUILD_DEPENDENCIES_FILE); \
	done; \
	exit "$$install_status"

build:
	@# Help: Build selected ROS packages with Colcon for local development
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	colcon build --base-paths $(COLCON_BASE_PATHS) --packages-select $(ROS_PACKAGES) --symlink-install --executor sequential --event-handlers console_direct+

test:
	@# Help: Build & test with Colcon
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	colcon build --base-paths $(COLCON_BASE_PATHS) --packages-select $(ROS_PACKAGES) --symlink-install --cmake-clean-cache \
		--cmake-args -DBUILD_TESTING=ON \
			-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
			-DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
		--executor sequential --event-handlers console_direct+ && \
	set +u && source $(ROOT_DIR)/install/setup.bash && set -u && \
	colcon test --base-paths $(COLCON_BASE_PATHS) --packages-select $(ROS_PACKAGES) --return-code-on-test-failure --executor sequential --event-handlers console_direct+

container-package:
	@# Help: Build Debian packages inside a ROS container on the host
	docker run --rm -t --platform linux/amd64 \
		-v $(ROOT_DIR):/workspace \
		-w /workspace \
		-e ROS_DISTRO=$(ROS_DISTRO) \
		-e PACKAGE_VERSION_SUFFIX=$(PACKAGE_VERSION_SUFFIX) \
		-e DEBIAN_FRONTEND=noninteractive \
		$(DOCKER_PROXY_ENV) \
		$(CONTAINER_IMAGE) \
		bash -c "git config --global --add safe.directory /workspace && \
			apt-get update -qq && apt-get install -y --no-install-recommends curl gnupg cmake ninja-build build-essential devscripts equivs dpkg-dev lld python3-colcon-common-extensions && \
			source /opt/ros/$(ROS_DISTRO)/setup.bash && \
			make install-debian-build-deps ROS_DISTRO=$(ROS_DISTRO) && \
			make package ROS_DISTRO=$(ROS_DISTRO) PACKAGE_VERSION_SUFFIX=$(PACKAGE_VERSION_SUFFIX)"

container-test:
	@# Help: Build & test with Colcon inside a ROS container on the host
	docker run --rm -t --platform linux/amd64 \
		-v $(ROOT_DIR):/workspace \
		-w /workspace \
		-e ROS_DISTRO=$(ROS_DISTRO) \
		-e DEBIAN_FRONTEND=noninteractive \
		$(DOCKER_PROXY_ENV) \
		$(CONTAINER_IMAGE) \
		bash -c "git config --global --add safe.directory /workspace && \
			apt-get update -qq && apt-get install -y --no-install-recommends curl gnupg cmake ninja-build build-essential devscripts equivs dpkg-dev lld python3-colcon-common-extensions && \
			source /opt/ros/$(ROS_DISTRO)/setup.bash && \
			make install-debian-build-deps ROS_DISTRO=$(ROS_DISTRO) && \
			make test ROS_DISTRO=$(ROS_DISTRO)"

lint:
	@# Help: Run all sub-linters using super-linter (using linters defined for this repo only)
	VALIDATE_GITHUB_ACTIONS=true \
	VALIDATE_YAML=true \
	VALIDATE_JSON=true \
	VALIDATE_PYTHON_PYLINT=true \
	VALIDATE_PYTHON_FLAKE8=true \
	VALIDATE_BASH=true \
	VALIDATE_MARKDOWN=true \
	VALIDATE_CLANG_FORMAT=true \
		make lint-all

lint-all:
	@# Help: Run super-linter over entire repository (auto-detects code to lint)
	docker run --rm --platform linux/amd64 \
		-e SHELL=/bin/bash \
		-e RUN_LOCAL=true \
		--env-file ".github/linters/super-linter.env" \
		$(if $(VALIDATE_GITHUB_ACTIONS),-e VALIDATE_GITHUB_ACTIONS=$(VALIDATE_GITHUB_ACTIONS)) \
		$(if $(VALIDATE_YAML),-e VALIDATE_YAML=$(VALIDATE_YAML)) \
		$(if $(VALIDATE_JSON),-e VALIDATE_JSON=$(VALIDATE_JSON)) \
		$(if $(VALIDATE_ALL_CODEBASE),-e VALIDATE_ALL_CODEBASE=$(VALIDATE_ALL_CODEBASE)) \
		$(if $(VALIDATE_PYTHON_PYLINT),-e VALIDATE_PYTHON_PYLINT=$(VALIDATE_PYTHON_PYLINT)) \
		$(if $(VALIDATE_PYTHON_FLAKE8),-e VALIDATE_PYTHON_FLAKE8=$(VALIDATE_PYTHON_FLAKE8)) \
		$(if $(VALIDATE_BASH),-e VALIDATE_BASH=$(VALIDATE_BASH)) \
		$(if $(VALIDATE_MARKDOWN),-e VALIDATE_MARKDOWN=$(VALIDATE_MARKDOWN)) \
		$(if $(VALIDATE_CLANG_FORMAT),-e VALIDATE_CLANG_FORMAT=$(VALIDATE_CLANG_FORMAT)) \
		-v $(ROOT_DIR):/tmp/lint \
			ghcr.io/super-linter/super-linter:slim-v8.1.0

lint-clang:
	@# Help: Run clang-format linter using super-linter
	VALIDATE_CLANG_FORMAT=true make lint-all

lint-python:
	@# Help: Run Python linters (pylint, flake8) using super-linter
	VALIDATE_PYTHON_PYLINT=true VALIDATE_PYTHON_FLAKE8=true make lint-all

lint-bash:
	@# Help: Run Bash linter using super-linter
	VALIDATE_BASH=true make lint-all

lint-yaml:
	@# Help: Run YAML linter using super-linter
	VALIDATE_YAML=true make lint-all

lint-githubactions:
	@# Help: Run GitHub Actions linter using super-linter
	VALIDATE_GITHUB_ACTIONS=true make lint-all

lint-markdown:
	@# Help: Run Markdown linter using super-linter
	VALIDATE_ALL_CODEBASE=true VALIDATE_MARKDOWN=true make lint-all

lint-json:
	@# Help: Run JSON linter using super-linter
	VALIDATE_JSON=true make lint-all

clean:
	@# Help: Remove local build, install, log, and package artifacts
	rm -rf $(ROOT_DIR)/build $(ROOT_DIR)/install $(ROOT_DIR)/log *.deb *.buildinfo *.changes 2>/dev/null || \
	docker run --rm -v $(ROOT_DIR):/workspace amd64/ros:$(ROS_DISTRO)-ros-base rm -rf /workspace/build /workspace/install /workspace/log

source-package:
	@# Help: Create source package tarball
	git archive --format=zip -o object-detection-$(VERSION).zip HEAD \
		.clang-format .gitattributes CMakeLists.txt Makefile README.md REUSE.toml robotics-project.json \
		.github/linters LICENSES cmake src

help:
	@printf "%-24s %s\n" "Target" "Description"
	@printf "%-24s %s\n" "------" "-----------"
	@grep -E '^[a-zA-Z0-9_%-]+:|^[[:space:]]+@# Help:' Makefile | \
	awk '\
		/^[a-zA-Z0-9_%-]+:/ { \
			target = $$1; \
			sub(":", "", target); \
		} \
		/^[[:space:]]+@# Help:/ { \
			if (target != "") { \
				help_line = $$0; \
				sub("^[[:space:]]+@# Help: ", "", help_line); \
				printf "%-24s %s\n", target, help_line; \
				target = ""; \
			} \
		}' | sort -k1,1
