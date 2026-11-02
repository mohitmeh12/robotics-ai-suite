# Blueprints

A blueprint is a validated stack you can replicate. Start with the parts list, follow
the steps, and end with the same working robot.


::::{grid} 3

:::{grid-item-card} **Stationary Robot**
:img-top: images/arm.svg
:link: stationary_arm/index
:link-type: doc
:link-alt: clickable cards

A stationary arm picks up an object using a depth camera and a trained policy.
:::

:::{grid-item-card} **Autonomous Mobile Robot**
:img-top: images/amr.svg
:link: amr/index
:link-type: doc
:link-alt: clickable cards

SLAM-based mapping and autonomous navigation on a mobile robot using Nav2, FastMapping, and the ITS Path Planner.
:::

:::{grid-item-card} **Humanoid Robot**
:img-top: images/humanoid.svg
:link: humanoid/index
:link-type: doc
:link-alt: clickable cards

Collect teleoperation data, train an ACT policy, convert it with OpenVINO™, and run inference on-device.
:::
::::

## Operating System & Hardware Requirements

Before deploying a blueprint, ensure your system meets the platform and middleware specifications and is configured with the required drivers, real-time kernel optimizations, and base software packages.

::::{grid} 2

:::{grid-item-card} **Development Kits**
:link: ../platform_foundation/development_kits/index
:link-type: doc
:link-alt: clickable cards

Review supported Intel® Core™ and Core™ Ultra processors, validated Ubuntu OS distributions, supported [development kits](../platform_foundation/development_kits/index.md), and baseline hardware specifications.
:::

:::{grid-item-card} **Getting Started**
:link: ../platform_foundation/getting_started
:link-type: doc
:link-alt: clickable cards

Follow step-by-step or express installation guides to configure drivers, platform packages, and the ROS 2 environment on your target hardware.
:::
::::

:::{toctree}
:hidden:

Autonomous Mobile Robot <amr/index>
Humanoid Robot <humanoid/index>
Stationary Arm <stationary_arm/index>
:::
