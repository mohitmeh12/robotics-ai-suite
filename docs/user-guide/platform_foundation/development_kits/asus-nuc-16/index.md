(asus-nuc-16-devkit)=
# ASUS NUC 16 Development Kit

## Product Link

**Product link**: [ASUS NUC 16](https://builders.intel.com/ecosystem-engagement/solution-hub/edge-ai-catalog/partner-spotlight/asus-nuc-16-338)

## Overview

This guide describes how to set up the ASUS NUC 16 development kit hardware and
confirm that it powers on and boots correctly.

The kit is powered by **Intel® Core™ Series 3 Processors** and combines CPU, GPU,
and an integrated NPU to deliver up to 40 total platform TOPS. It ships as an
ultra-compact, tool-less, upgradeable unit with up to 64 GB of DDR5-6400 memory
and support for up to three 4K displays.

> [!NOTE]
> This kit does not include GMSL or MIPI CSI camera connectivity. Connect
> cameras over USB — see the [USB Cameras](../../../components/sensors/cameras/usb/index.md)
> guide.

> [!NOTE]
> This development kit is intended for research and development purposes only.

## What you'll need

- A monitor with an **HDMI** input.
- A **USB keyboard and mouse**.
- The bundled **DC power adapter**.

## Overall Flow

1. **Unbox and inspect** the kit.
2. **Connect** the display, keyboard, and mouse.
3. **Power on** the kit.
4. **Verify** the BIOS and confirm the kit boots.

## Steps

### Step 1: Unbox and inspect

Set the kit on an anti-static surface and confirm the box contains the unit and its bundled
DC power adapter. Refer to the packing list included with your unit for the complete
contents.

> [!WARNING]
> The kit can be damaged if it is not placed on an anti-static surface. If any
> item is missing or the kit is damaged, contact Intel before proceeding.

### Step 2: Connect peripherals

Connect your monitor to the **HDMI** output, and plug a keyboard and mouse into the **USB**
ports.

### Step 3: Power on

Plug the supplied DC power adapter into the **DC-in connector** and press the power
button.

:::caution
Only use the DC power adapter supplied with the kit. Powering the board from another
source can damage it.
:::

### Step 4: Verify the BIOS

1. Press the key indicated on the boot splash screen to enter the BIOS setup screen.
2. Check the **time**, **date**, and configuration settings.
3. **Save and exit**.

The system reboots and is ready for you to install an operating system.

## Next Steps

- **[Supported Operating Systems](./supported-operating-systems.md)** — review the operating
  systems validated for this kit before installing your OS.

For product details, see the [manufacturer website](https://www.asus.com/displays-desktops/nucs/nuc-mini-pcs/asus-nuc-16/).

:::{toctree}
:caption: Components
:hidden:

Supported Operating Systems <supported-operating-systems>
:::
