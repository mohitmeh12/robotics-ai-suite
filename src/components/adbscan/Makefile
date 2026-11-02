# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

SHELL := bash -eu -o pipefail

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# Version info for packaging
COMMIT := $(shell git rev-parse --short HEAD)
DATE := $(shell git log -1 --format=%cd --date=format:"%Y%m%d")
VERSION := $(COMMIT)-$(DATE)

ROS_DISTRO ?= humble
PACKAGE_VERSION_SUFFIX ?=
ifeq ($(ROS_DISTRO),humble)
WITH_ONEAPI ?= OFF
else
WITH_ONEAPI ?= ON
endif
PROJECT_DEFINITION ?= $(ROOT_DIR)/robotics-project.json
TEST_SUMMARY ?= $(ROOT_DIR)/testout/test-summary.md

# Map ROS_DISTRO to Ubuntu codename
ifeq ($(ROS_DISTRO),humble)
	UBUNTU_CODENAME := jammy
else
	UBUNTU_CODENAME := noble
endif

.DEFAULT_GOAL := help
.PHONY: build check check-ci changelog-check clean coverage debian-build-deps format install-debian-build-deps license-check package test test-e2e test-results tools-check lint lint-all lint-all-codebase lint-clang lint-python lint-yaml lint-githubactions lint-bash lint-markdown lint-json lint-actionlint lint-clang-format lint-ruff source-package help

PACKAGE_COMPONENTS := $(shell PROJECT_DEFINITION="$(PROJECT_DEFINITION)" ROOT_DIR="$(ROOT_DIR)" python3 -c 'import json, os, pathlib, re; project = json.loads(pathlib.Path(os.environ["PROJECT_DEFINITION"]).read_text(encoding="utf-8")); components = project.get("ci", {}).get("package_components", []); root = pathlib.Path(os.environ["ROOT_DIR"]) / "src"; valid = project.get("format_version") == 1 and isinstance(components, list) and components and all(isinstance(component, dict) and isinstance(component.get("path"), str) and re.fullmatch(r"[a-z0-9][a-z0-9_-]*", component["path"]) and (root / component["path"]).is_dir() and ("ros_package" not in component or isinstance(component["ros_package"], str) and re.fullmatch(r"[a-z0-9][a-z0-9_]*", component["ros_package"])) for component in components); (_ for _ in ()).throw(SystemExit("Invalid package components in project definition")) if not valid else None; print(",".join(component["path"] for component in components))')
ROS_PACKAGES := $(shell PROJECT_DEFINITION="$(PROJECT_DEFINITION)" python3 -c 'import json, os, pathlib, re; project = json.loads(pathlib.Path(os.environ["PROJECT_DEFINITION"]).read_text(encoding="utf-8")); components = project.get("ci", {}).get("package_components", []); ros_components = [component for component in components if "ros_package" in component]; valid = project.get("format_version") == 1 and isinstance(components, list) and ros_components and all(isinstance(component, dict) and isinstance(component.get("ros_package"), str) and re.fullmatch(r"[a-z0-9][a-z0-9_]*", component["ros_package"]) for component in ros_components); (_ for _ in ()).throw(SystemExit("Invalid ROS packages in project definition")) if not valid else None; print(" ".join(component["ros_package"] for component in ros_components))')
PACKAGE_OUTPUT_DIR = $(ROOT_DIR)/build/debian-packages/packages
BUILD_DEPENDS_OUTPUT_DIR = $(ROOT_DIR)/build/debian-build-deps
MISSING_BUILD_DEPENDENCIES_FILE = $(BUILD_DEPENDS_OUTPUT_DIR)/missing-dependencies.txt
UV ?= uv
MARKDOWNLINT ?= $(shell command -v markdownlint || command -v markdownlint-cli2 || printf 'markdownlint')

license-check:
	@# Help: Perform a REUSE license check using docker container https://hub.docker.com/r/fsfe/reuse
	docker run --rm --security-opt label=disable --volume ${ROOT_DIR}:/data:ro fsfe/reuse:5.0.2 lint

package:
	@# Help: Build Debian packages
	command -v ninja >/dev/null || { echo "Error: Ninja is required. Install ninja-build." >&2; exit 1; }
	rm -rf $(ROOT_DIR)/build/debian-packages
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	cmake -G Ninja -S $(ROOT_DIR) -B $(ROOT_DIR)/build/debian-packages \
		-DROS_DISTRO=$(ROS_DISTRO) \
		-DADBSCAN_PACKAGE_OUTPUT_DIR=$(PACKAGE_OUTPUT_DIR) \
		-DADBSCAN_PACKAGE_COMPONENTS="$(PACKAGE_COMPONENTS)" \
		-DADBSCAN_PACKAGE_VERSION_SUFFIX=$(PACKAGE_VERSION_SUFFIX) \
		-DADBSCAN_WITH_ONEAPI=$(WITH_ONEAPI) && \
	cmake --build $(ROOT_DIR)/build/debian-packages --target package

debian-build-deps:
	@# Help: Generate Debian Build-Depends control files without configuring components
	rm -rf $(BUILD_DEPENDS_OUTPUT_DIR) $(ROOT_DIR)/build/debian-build-deps-config
	cmake -S $(ROOT_DIR) -B $(ROOT_DIR)/build/debian-build-deps-config \
		-DROS_DISTRO=$(ROS_DISTRO) \
		-DADBSCAN_WITH_ONEAPI=$(WITH_ONEAPI) \
		-DADBSCAN_PACKAGE_COMPONENTS="$(PACKAGE_COMPONENTS)" \
		-DADBSCAN_BUILD_DEPENDS_OUTPUT_DIR=$(BUILD_DEPENDS_OUTPUT_DIR) && \
	cmake --build $(ROOT_DIR)/build/debian-build-deps-config --target debian-build-deps

install-debian-build-deps: debian-build-deps
	@# Help: Install generated Debian Build-Depends with APT
	@command -v mk-build-deps >/dev/null && command -v equivs-build >/dev/null && command -v dpkg-checkbuilddeps >/dev/null || { echo "Error: mk-build-deps, equivs-build, and dpkg-checkbuilddeps are required. Install devscripts, equivs, and dpkg-dev." >&2; exit 1; }
	@apt-get update; \
	install_status=0; \
	for control_file in $(BUILD_DEPENDS_OUTPUT_DIR)/*/debian/control; do \
		mk-build-deps --install --remove \
			--tool 'apt-get --no-install-recommends --no-remove --yes' \
			"$$control_file" || install_status=1; \
	done; \
	: > $(MISSING_BUILD_DEPENDENCIES_FILE); \
	for control_file in $(BUILD_DEPENDS_OUTPUT_DIR)/*/debian/control; do \
		if missing_dependencies="$$(dpkg-checkbuilddeps "$$control_file" 2>&1)"; then \
			continue; \
		fi; \
		printf '%s\n%s\n\n' "$$control_file" "$$missing_dependencies" >> $(MISSING_BUILD_DEPENDENCIES_FILE); \
	done; \
	exit "$$install_status"

build:
	@# Help: Build selected ROS packages with Colcon for local development
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	colcon build --base-paths $(ROOT_DIR)/src --packages-select $(ROS_PACKAGES) --symlink-install --executor sequential --event-handlers console_direct+

test:
	@# Help: Build & test with Colcon
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	colcon build --base-paths $(ROOT_DIR)/src --packages-select $(ROS_PACKAGES) --symlink-install --cmake-clean-cache \
		--cmake-args -DBUILD_TESTING=ON --executor sequential --event-handlers console_direct+ && \
	set +u && source $(ROOT_DIR)/install/setup.bash && set -u && \
	colcon test --base-paths $(ROOT_DIR)/src --packages-select $(ROS_PACKAGES) --return-code-on-test-failure --executor sequential --event-handlers console_direct+

coverage:
	@# Help: Generate non-gating C++ line and branch coverage reports
	@command -v "$(UV)" >/dev/null || { echo "Error: uv is required. Install it from https://docs.astral.sh/uv/." >&2; exit 1; }
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	colcon build --base-paths $(ROOT_DIR)/src --packages-select $(ROS_PACKAGES) --symlink-install --cmake-clean-cache \
		--cmake-args -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=--coverage -DCMAKE_EXE_LINKER_FLAGS=--coverage -DCMAKE_SHARED_LINKER_FLAGS=--coverage \
		--executor sequential --event-handlers console_direct+ && \
	set +u && source $(ROOT_DIR)/install/setup.bash && set -u && \
	colcon test --base-paths $(ROOT_DIR)/src --packages-select $(ROS_PACKAGES) --return-code-on-test-failure --executor sequential --event-handlers console_direct+
	mkdir -p $(ROOT_DIR)/testout/coverage
	$(UV) tool run --from gcovr gcovr --root $(ROOT_DIR) --filter $(ROOT_DIR)/src --exclude '.*/(test|tests)/.*' --exclude '.*/build/.*' \
		--print-summary --txt-metric branch --html-details $(ROOT_DIR)/testout/coverage/index.html \
		--xml-pretty --output $(ROOT_DIR)/testout/coverage/coverage.xml

test-e2e:
	@# Help: Run synthetic headless launch tests from the active ROS prefix
	mkdir -p testout
	set +u && source /opt/ros/$(ROS_DISTRO)/setup.bash && set -u && \
	if [[ -f "$(ROOT_DIR)/install/setup.bash" ]] && { ! ros2 pkg prefix adbscan_ros2 >/dev/null 2>&1 || ! ros2 pkg prefix adbscan_ros2_follow_me >/dev/null 2>&1; }; then \
		set +u && source "$(ROOT_DIR)/install/setup.bash" && set -u; \
	fi && \
	export ROS_DOMAIN_ID=42 && /usr/bin/python3 -m pytest tests/ --junit-xml=testout/adbscan_e2e.xml

test-results:
	@# Help: Summarize Colcon test results
	@mkdir -p "$(dir $(TEST_SUMMARY))"
	python3 .github/scripts/summarize_test_results.py \
		--results-glob 'build/*/test_results/**/*.xml' \
		--ros-distro $(ROS_DISTRO) \
		--summary-file "$(TEST_SUMMARY)"

check: lint license-check
	@# Help: Run locally reproducible pre-push checks

check-ci:
	@# Help: Run package and Colcon validation for Humble and Jazzy
	$(MAKE) ROS_DISTRO=humble package test
	$(MAKE) ROS_DISTRO=jazzy package test

tools-check:
	@# Help: Verify locally installed developer tools
	@missing=0; \
	for tool in actionlint yamllint ruff shellcheck clang-format ninja colcon docker python3; do \
		if ! command -v "$$tool" >/dev/null; then \
			echo "Missing required tool: $$tool" >&2; \
			missing=1; \
		fi; \
	done; \
	if ! command -v "$(MARKDOWNLINT)" >/dev/null; then \
		echo "Missing required tool: markdownlint or markdownlint-cli2" >&2; \
		missing=1; \
	fi; \
	exit "$$missing"

format:
	@# Help: Apply local source formatting
	git ls-files -z -- '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx' | xargs -0 -r -n1 sh -c 'test ! -e "$$1" || clang-format -i "$$1"' sh
	git ls-files -z -- '*.py' | xargs -0 -r -n1 sh -c 'test ! -e "$$1" || ruff check --fix "$$1"' sh
	git ls-files -z -- '*.py' | xargs -0 -r -n1 sh -c 'test ! -e "$$1" || ruff format "$$1"' sh
	git ls-files -z -- '*.md' | xargs -0 -r $(MARKDOWNLINT) --config .github/linters/.markdown-lint.yml --fix

clean:
	@# Help: Remove local build, install, log, test, and package artifacts
	rm -rf $(ROOT_DIR)/build $(ROOT_DIR)/install $(ROOT_DIR)/log $(ROOT_DIR)/testout

lint: lint-actionlint lint-yaml lint-json lint-ruff lint-bash lint-markdown lint-clang-format
	@# Help: Run all local linters on tracked files

lint-all lint-all-codebase: lint
	@# Help: Run all local linters on tracked files

lint-actionlint lint-githubactions:
	@# Help: Run GitHub Actions workflow linting
	actionlint -config-file .github/linters/actionlint.yml -ignore 'SC2016' $$(git ls-files -- '.github/workflows/*.yml' '.github/workflows/*.yaml')

lint-yaml:
	@# Help: Run YAML linting
	git ls-files -z -- '*.yaml' '*.yml' | xargs -0 -r -n1 sh -c 'test ! -e "$$1" || yamllint -c .github/linters/.yaml-lint.yml "$$1"' sh

lint-json:
	@# Help: Run JSON linting
	git ls-files -z -- '*.json' | xargs -0 -r -n1 python3 -m json.tool >/dev/null

lint-ruff lint-python:
	@# Help: Run Python linting
	git ls-files -z -- '*.py' | xargs -0 -r -n1 sh -c 'test ! -e "$$1" || ruff check "$$1"' sh

lint-bash:
	@# Help: Run Bash linting
	git ls-files -z -- '*.sh' | xargs -0 -r shellcheck --severity=error

lint-markdown:
	@# Help: Run Markdown linting
	git ls-files -z -- '*.md' | xargs -0 -r $(MARKDOWNLINT) --config .github/linters/.markdown-lint.yml

lint-clang-format lint-clang:
	@# Help: Run C/C++ formatting linting
	git ls-files -z -- '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx' | xargs -0 -r -n1 sh -c 'test ! -e "$$1" || clang-format --dry-run --Werror "$$1"' sh

source-package:
	@# Help: Create source package tarball
	git archive --format=zip -o adbscan-$(VERSION).zip HEAD \
		.gitattributes Makefile README.md REUSE.toml \
		.github/linters docs images LICENSES \
		src

help:
	@printf "%-20s %s\n" "Target" "Description"
	@printf "%-20s %s\n" "------" "-----------"
	@grep -E '^[a-zA-Z0-9_%-]+([[:space:]][a-zA-Z0-9_%-]+)*:|^[[:space:]]+@# Help:' Makefile | \
	awk '\
		/^[a-zA-Z0-9_%-]+([[:space:]][a-zA-Z0-9_%-]+)*:/ { \
			targets = $$0; \
			sub(":.*", "", targets); \
		} \
		/^[[:space:]]+@# Help:/ { \
			if (targets != "") { \
				help_line = $$0; \
				sub("^[[:space:]]+@# Help: ", "", help_line); \
				n = split(targets, names, " "); \
				for (position = 1; position <= n; position++) { \
					printf "%-20s %s\n", names[position], help_line; \
				} \
			} \
		}'
