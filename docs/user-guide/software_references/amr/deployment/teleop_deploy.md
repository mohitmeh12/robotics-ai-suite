```{eval-rst}
.. meta::
   :description: Validate mobile robot motor control with keyboard teleoperation.
   :keywords: AMR, teleoperation, keyboard, AAEON
```

# Deploy Robot Teleop Using a Keyboard

Use this software reference to validate robot motor control with a keyboard before deploying
autonomous applications.

## Prerequisites

First, complete the [Getting Started](../../../platform_foundation/getting_started.md) guide.

## Hardware Prerequisites

You have a robot and a keyboard or an SSH/VNC connection to the robot.

## Example for the AAEON UP Xtreme i11 Robotic Development Kit

1. Connect to your robot through SSH or VNC, or use direct access. For direct access, connect
   a monitor and keyboard to the robot's compute system.

2. Set up your system by following the steps in the
   [Getting Started](../../../platform_foundation/getting_started.md) guide.

3. Ensure the `ros2-amr-interface` Debian package is installed:

   ::::{tab-set}
   :::{tab-item} **Jazzy**
   :sync: jazzy

   ```bash
   sudo apt update
   sudo apt install ros-jazzy-aaeon-ros2-amr-interface
   ```

   :::
   :::{tab-item} **Humble**
   :sync: humble

   ```bash
   sudo apt update
   sudo apt install ros-humble-aaeon-ros2-amr-interface
   ```

   :::
   ::::

4. Open a terminal on the robot or establish a new SSH connection to start the control node.

   - Check the motor controller device name:

     ```bash
     sudo dmesg | grep ttyUSB
     ```

   - The output includes the `ch341-uart` device, which provides the interface to the motor
     controller board. For example:

     ```bash
     [1452443.462213] usb 1-9: ch341-uart converter now attached to ttyUSB0
     [1452444.061111] ch341-uart ttyUSB0: ch341-uart converter now disconnected from ttyUSB0
     ```

   - Ensure the AAEON node configuration file sets `port_name` to the correct USB device:

     ::::{tab-set}
     :::{tab-item} **Jazzy**
     :sync: jazzy

     ```bash
     vi /opt/ros/jazzy/share/ros2_amr_interface/params/aaeon_node_params.yaml
     ```

     :::
     :::{tab-item} **Humble**
     :sync: humble

     ```bash
     vi /opt/ros/humble/share/ros2_amr_interface/params/aaeon_node_params.yaml
     ```

     :::
     ::::

   - Start the motor control node:

     ::::{tab-set}
     :::{tab-item} **Jazzy**
     :sync: jazzy

     ```bash
     AAEON_NODE_CONFIG_FILE=/opt/ros/jazzy/share/ros2_amr_interface/params/aaeon_node_params.yaml

     # Launch the AAEON Robot Motor Board Interface
     ros2 run ros2_amr_interface amr_interface_node --ros-args \
        --params-file $AAEON_NODE_CONFIG_FILE \
        --remap /amr/cmd_vel:=/cmd_vel \
        --remap /amr/battery:=/sensors/battery_state
     ```

     :::
     :::{tab-item} **Humble**
     :sync: humble

     ```bash
     AAEON_NODE_CONFIG_FILE=/opt/ros/humble/share/ros2_amr_interface/params/aaeon_node_params.yaml

     # Launch the AAEON Robot Motor Board Interface
     ros2 run ros2_amr_interface amr_interface_node --ros-args \
        --params-file $AAEON_NODE_CONFIG_FILE \
        --remap /amr/cmd_vel:=/cmd_vel \
        --remap /amr/battery:=/sensors/battery_state
     ```

     :::
     ::::

5. [Only for Robot Operating System (ROS) Jazzy] Install the required package if it is not
   already installed:

   ```bash
   sudo apt-get install ros-jazzy-teleop-twist-keyboard
   ```

6. On another terminal or new SSH connection, start the teleop keyboard:

   ::::{tab-set}
   :::{tab-item} **Jazzy**
   :sync: jazzy

   ```bash
   ros2 run teleop_twist_keyboard teleop_twist_keyboard
   ```

   :::
   :::{tab-item} **Humble**
   :sync: humble

   ```bash
   TURTLEBOT3_MODEL=aaeon ros2 run turtlebot3_teleop teleop_keyboard
   ```

   :::
   ::::

   When the node starts, it displays the available keyboard commands for controlling the robot.
   The commands depend on the selected robot model.

## Troubleshooting

You can stop the demo at any time by pressing `Ctrl+C`.

For general robot issues, see the [troubleshooting guide](../../../resources/troubleshooting).
