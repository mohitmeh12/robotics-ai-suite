# Virtualization

The virtualization documentation covers two ways to isolate robotics workloads on one host. KVM virtual machines get their own kernel and can run next to a real-time host. Canonical Workshop containers share the host's kernel and drivers, and give each Robotics AI Suite component its own reproducible environment.


::::{grid} 2

:::{grid-item-card} **KVM Guide**
:link: kvm_guide
:link-type: doc
:link-alt: clickable cards

Set up the KVM environment, launch a guest VM, and debug and manage VMs.
:::

:::{grid-item-card} **Image Build**
:link: image_build
:link-type: doc
:link-alt: clickable cards

Build a `qcow2` ubuntu guest image.
:::

:::{grid-item-card} **Canonical Workshop Environments**
:link: workshops/index
:link-type: doc
:link-alt: clickable cards

Run Robotics AI Suite components in containers with the host's GPU, NPU, cameras and desktop, on ROS 2 Jazzy and Humble side by side.
:::
::::



:::{toctree}
:hidden:

KVM Guide <kvm_guide>
Image Build <image_build>
Canonical Workshop Environments <workshops/index>
:::
