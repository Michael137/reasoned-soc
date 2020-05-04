- [Inline Crypto Engine](https://android.googlesource.com/kernel/msm.git/+/android-msm-bullhead-3.10-n-preview-1/Documentation/devicetree/bindings/crypto/msm/ice.txt):
	+ drivers/crypto/msm/ice.c -> drivers/scsi/ufs/ufs-qcom-ice.c 
- Qualcomm Internet Packet Accelerator (IPA):
	+ platform/msm/ipa/ipa_api.c
	+ private/msm-google-modules/wlan/qcacld-3.0
- [Related to the KGSL GPU Driver](https://android.googlesource.com/kernel/msm/+/android-msm-sony-cm-jb-3.0/Documentation/arm/msm/kgsl-sysfs.txt)
	+ drivers/gpu/msm/kgsl.c
	+ IOCTLS defined in: drivers/gpu/msm/kgsl_iotctl.c
- [Qualcomm Face Detection](https://android.googlesource.com/kernel/msm.git/+/android-msm-bullhead-3.10-n-preview-1/Documentation/devicetree/bindings/media/video/msm-fd.txt)
	+ drivers/media/platform/msm/camera_v2/fd/msm_fd_hw.c
- [Qualcomm Video Camera Driver](https://android.googlesource.com/kernel/msm/+/android-7.1.0_r0.2/Documentation/devicetree/bindings/media/video/msm-cpp.txt)
	+ Post-processing engine: media/platform/msm/camera_v2/pproc/cpp/msm_cpp.h
	+ QCom Camera Hardware IPs: drivers/media/platform/msm/camera_v2/Kconfig
	+ ISP: media/platform/msm/camera_v2/isp/msm_isp.c
		+ IOCTL defined in: media/platform/msm/camera_v2/isp/msm_isp_util.c:msm_isp_ioctl()
	+ Video processing engine: media/platform/msm/camera_v2/pproc/vpe/msm_vpe.c
	+ jpeg HW: media/platform/msm/camera_v2/jpeg_10/msm_jpeg_hw.c
	+ Video Codec: private/msm-google/drivers/media/platform/msm/vidc/msm_vidc.c
- [Camera Control Interface](https://lwn.net/Articles/735236/)
- [Camera Driver](https://android.googlesource.com/kernel/msm/+/android-msm-wahoo-4.4-oreo-dr1/arch/arm/boot/dts/qcom/msm8998-camera.dtsi)
- [Qualcomm MSS QDSP6v5 Peripheral Image Loader](https://android.googlesource.com/kernel/msm/+/android-msm-3.9-usb-and-mmc-hacks/Documentation/devicetree/bindings/pil/pil-q6v5-mss.txt)
- [Qualcomm Technologies LPASS CPU DAI](https://www.kernel.org/doc/Documentation/devicetree/bindings/sound/qcom%2Clpass-cpu.txt)
- DSP:
	+ drivers/char/adsprpc.c
