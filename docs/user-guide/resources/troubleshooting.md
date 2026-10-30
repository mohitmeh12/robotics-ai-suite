# Troubleshooting

(humanoid_embodied_troubleshooting)=
## Humanoid Imitation Learning

1. **OpenVINO™ cannot detect a graphics processing unit (GPU)**

   ```bash
   python3 -c "from openvino import Core; ie = Core(); print(ie.available_devices)"
   ['CPU']
   ```

   First, make sure you have installed the appropriate GPU firmware and driver. If you are still
   encountering this issue:

   ```bash
   export OCL_ICD_VENDORS=/etc/OpenCL/vendors
   ```

2. **MESA: warning: Driver does not support the 0x7d51 PCI ID**

   ```console
   MESA: warning: driver does not support the 0x7d51 PCI ID.
   DRI3 not available
   ```

   Update Mesa to resolve this issue:

   ```bash
   sudo apt update
   sudo apt install mesa-utils libgl1-mesa-dri libglx-mesa0 libigc2
   ```

3. **IPEX workloads are incompatible with the Intel NPU driver**

   ```console
   RuntimeError: Native API failed. Native API returns: -1102 (PI_ERROR_UNINITIALIZED) -1102 (PI_ERROR_UNINITIALIZED)
   ```

   To run IPEX workloads, uninstall the Intel NPU deb packages:

   ```bash
   sudo dpkg --purge --force-remove-reinstreq intel-driver-compiler-npu intel-fw-npu intel-level-zero-npu
   ```

4. **DepthAnythingV2 checkpoint download failed**

   (depthanythingv2_troubleshooting)=

   In the script `<Depth-Anything-ONNX_project_dir>/depth_anything_v2/config.py`, update the
   download link:

   ```python
   Encoder.vitb: {
       None: "https://hf-mirror.com/depth-anything/Depth-Anything-V2-Base/resolve/main/depth_anything_v2_vitb.pth?download=true",
       ..
   },
   ```

5. **OpenVINO™ inference failed**

   (ov_inference_troubleshooting)=

   If you encounter errors when running OpenVINO™ inference of models from
   [OpenVINO™ Supported Models](../ai_resources/openvino/models/index.md), check the OpenVINO™
   version used for model conversion and the runtime version used for inference. The OpenVINO™
   version used for model conversion should be the same as the runtime version used for inference.
   Otherwise, unexpected errors may occur, especially if the model is converted using a newer
   version and the runtime is an older version.

   You can check the OpenVINO™ version used for model conversion at the end of the OpenVINO™ IR
   file `*.xml`. For example:

   ```xml
   ...
    <rt_info>
           <Runtime_version value="2025.0.0-17942-1f68be9f594-releases/2025/0" />
           <conversion_parameters>
                   <framework value="pytorch" />
                   <is_python_object value="True" />
           </conversion_parameters>
   </rt_info>
   ...
   ```

6. **IOMMU device unassignment failed during ACRN hypervisor startup**

   ```console
   failed to unassign iommu device!
   ```

   1. Check whether VT-d is enabled in BIOS. Refer to the BIOS configuration in the
      [platform getting-started guide](../platform_foundation/getting_started.md).

   2. Check that all connected PCIe devices are enabled correctly.

7. **Docker pull timed out**

   (docker_proxy_troubleshooting)=

   If your network requires a proxy, refer to the
   [Docker documentation website](https://docs.docker.com/engine/daemon/proxy/)
   to configure proxy settings.

8. **iGPU performance degrades when passed through to a VM on ACRN**

   (ACRN_troubleshooting)=

   Passthrough from the iGPU to a guest VM on ACRN causes a decrese in AI model performance
   compared to native execution.


(rvc_troubleshooting)=

## Stationary Robot Vision and Control

1. Inference on GPU does not work.

   ```bash
   sudo -E apt install clinfo
   clinfo
   ```

   Verify that the GPU is supported on the platform:

   ```text
   Number of platforms                             1
   Platform Name                                   Intel(R) OpenCL HD Graphics
   Platform Vendor                                 Intel(R) Corporation
   Platform Version                                OpenCL 3.0
   Platform Profile                                FULL_PROFILE
   ```

2. The robot does not move.

   First start the motion controller, then press play on the pendant.

3. Robot arm does not move accurately to pick up the object.

   Check [Camera Setup and Extrinsics Registration](../hardware_blueprints/stationary_arm/ur5e-robotiq-realsense.md#camera-setup-and-extrinsics-registration).
