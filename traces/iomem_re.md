- vmpm: 4.095 KB
  - [MSM Sleep Power Manager](https://android.googlesource.com/kernel/msm/+/android-msm-flo-3.4-kitkat-mr1/Documentation/devicetree/bindings/arm/msm/mpm.txt)
  - "reg-names: "vmpm" - string to identify the shared memory space region"
  - "MPM driver: Modem power manager driver is used to program the MPM hardware block
during system-wide sleep to monitor the wakeup-capable interrupts." - [Resource Power Manager API Reference](https://developer.qualcomm.com/qfile/35136/lm80-p0436-73_a_qualcomm_snapdragon_410e_processor_apq8016e_system_power_overview.pdf)
- tsens_physical: 8.191 KB
  - [Qualcomm's TSENS driver](https://android.googlesource.com/kernel/msm/+/android-msm-bullhead-3.10-marshmallow-dr/Documentation/devicetree/bindings/thermal/tsens.txt)
  - "supports reading temperature from sensors across the MSM"
  - "reg-names : resource names used for the physical address of the TSENS
	      registers, the QFPROM efuse primary calibration address region,
	      Should be "tsens_physical" for physical address of the TSENS,
	      "tsens_eeprom_physical" for physical address where primary
	      calibration data is stored."
- pshold-base: 3 B
  - [MSM Restart Driver](https://android.googlesource.com/kernel/msm/+/android-7.1.0_r0.2/Documentation/devicetree/bindings/power_supply/msm-poweroff.txt)
  - A power supply hold (ps-hold) bit is set to power the msm chipsets
  - "-reg: Specifies the physical address of the ps-hold register
      This value must correspond to "pshold-base" as defined in reg-names"
- tsens_physical: 8.191 KB
  - **REPEATED**
- /soc/arm,smmu-anoc1@1680000: 65.53 KB
  - Defined in [msm-arm-smmu-8998.dtsi](https://android.googlesource.com/kernel/msm/+/android-msm-wahoo-4.4-oreo-dr1/arch/arm/boot/dts/qcom/msm-arm-smmu-8998.dtsi)
  - [ARM SMMU](https://android.googlesource.com/kernel/msm/+/android-7.1.0_r0.2/Documentation/devicetree/bindings/iommu/arm,smmu.txt)
  - ".dtsi" files contain SoC level definitions that get included into the final device ".dts" files (see [here](https://bootlin.com/pub/conferences/2013/elce/petazzoni-device-tree-dummies/petazzoni-device-tree-dummies.pdf))
    - More on device tree sources [here](https://www.digi.com/resources/documentation/digidocs/90002287/reference/bsp/r_device_tree_files.htm)
  - [Format of dts files](https://devicetree-specification.readthedocs.io/en/latest/source-language.html):
    - `[label:] node-name[@unit-address] {
			[properties definitions]
			[child nodes]
		};`
- /soc/arm,smmu-anoc2@16c0000: 262.1 KB
  - **SEE ABOVE**
- sp2soc_irq_status: 3 B
- sp2soc_irq_clr: 3 B
- sp2soc_irq_mask: 3 B
- rmb_err: 3 B
- rmb_err_spare2: 3 B
- /soc/ufshc@1da4000: 9.471 KB
  - Related to Qualcomm Inline Crypto Engine (?)
- phy_mem: 3.495 KB
- /soc/ufsice@1db0000: 32.77 KB
  - [Inline Crypto Engine](https://android.googlesource.com/kernel/msm.git/+/android-msm-bullhead-3.10-n-preview-1/Documentation/devicetree/bindings/crypto/msm/ice.txt)
  - [More on Qualcomm ICE](https://csrc.nist.gov/CSRC/media/projects/cryptographic-module-validation-program/documents/security-policies/140sp3124.pdf)
    - "Qualcomm Inline Crypto Engine (UFS) is classified as a single chip hardware module for the
purpose of FIPS 140-2 validation. It provides AES-XTS encryption and decryption of block storage
devices. The logical cryptographic boundary is the Qualcomm Inline Crypto Engine (UFS) 3.1.0,
which is a sub-chip hardware component contained within the Qualcomm Snapdragon 845 SoC,
the Snapdragon 855 SoC, and the Snapdragon 865 Mobile Platform SoC"
- vls_clamp_reg: 3 B
  - [USB related](https://android.googlesource.com/kernel/msm/+/android-7.1.0_r0.2/Documentation/devicetree/bindings/usb/msm-phy.txt)
- tcsr_usb3_dp_phymode: 3 B
- tcsr_clamp_dig_n_1p8: 3 B
- /soc/pinctrl@03400000: 12.58 MB
- qdsp6_base: 255 B
  - [Qualcomm MSS QDSP6v5 Peripheral Image Loader](https://android.googlesource.com/kernel/msm/+/android-msm-3.9-usb-and-mmc-hacks/Documentation/devicetree/bindings/pil/pil-q6v5-mss.txt)
  - "peripheral image loader (PIL) driver. It is used for
loading QDSP6v5 (Hexagon) firmware images for modem subsystems into memory and
preparing the subsystem's processor to execute code. It's also responsible for
shutting down the processor when it's not needed."
- rmb_base: 31 B
  - Related to [Qualcomm Femtocell (FSM99XX and FSM90XX) Peripheral Image Loader](https://android.googlesource.com/kernel/msm/+/android-msm-bullhead-3.10-marshmallow-dr/Documentation/devicetree/bindings/pil/pil-femto-modem.txt)
  - "pil-femto-modem is a peripheral image loader (PIL) driver. It is used for loading firmware images on multiple modems resident on the FSM99XX and FSM90XX platforms."
  - "- reg: Pair of physical base address and region size of the
            Relay Message Buffer (RMB) registers for this modem.
     - reg-names:            "rmb_base" is required."
- kgsl-3d0: 262.1 KB
  - [Related to the KGSL GPU Driver](https://android.googlesource.com/kernel/msm/+/android-msm-sony-cm-jb-3.0/Documentation/arm/msm/kgsl-sysfs.txt)
  - "Each individual GPU device (2D or 3D) will have its own device node in
  this directory. All platforms will have kgsl-3d0 (3D device)"
  - Related KGSL Exploits:
    1. [Heap Overflow (with code examples)](https://www.exploit-db.com/exploits/39504)
    2. [Slides](https://www.blackhat.com/docs/eu-16/materials/eu-16-Taft-GPU-Security-Exposed.pdf)
- /soc/arm,smmu-kgsl@5040000: 65.53 KB
  - [KGSL memory management unit](https://android.googlesource.com/kernel/msm/+/android-msm-wahoo-4.4-oreo-dr1/arch/arm/boot/dts/qcom/msm-arm-smmu-8998.dtsi)
- /soc/arm,smmu-lpass_q6@5100000: 262.1 KB
  - LPASS memory management unit (same location of definition as smmu-kgsl above)
  - [Qualcomm Technologies LPASS CPU DAI](https://www.kernel.org/doc/Documentation/devicetree/bindings/sound/qcom%2Clpass-cpu.txt)
- stm-base: 4.095 KB
  - ARM Coresight STM
  - HW component **used for tracing/debugging SoC**
- stm-base: 4.095 KB
- funnel-base: 4.095 KB
  - part of [ARM trace collection flow](http://www2.lauterbach.com/pdf/app_arm_coresight.pdf)
  - " data can be passed directly or combined with other trace sources via Funnel (CSTF) and AMBA Trace Bus (ATB) off-chip to the Trace Port Interface Unit (TPIU)"
  - "The ATB funnel component merges multiple ATB buses into a single ATB bus"
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- replicator-base: 4.095 KB
  - part of ATB flow
  - "ATB replicator propagates the data from a single ATB master to two ATB slaves at the same time"
- replicator-base: 4.095 KB
- tmc-base: 4.095 KB
  - [Trace Memory Controller](https://developer.arm.com/ip-products/system-ip/coresight-debug-and-trace/coresight-components/trace-memory-controller)
  - "configurable trace component to terminate trace buses into buffers, FIFOs, or alternatively, to route trace data over AXI to memory or off-chip to interface controllers"
- tmc-base: 4.095 KB
- tmc-base: 4.095 KB
- tmc-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- /soc/etm@7840000: 4.095 KB
  - [Embedded Trace Macrocell](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337h/CHDJAECG.html)
  - "ETM is an optional debug component that enables reconstruction of program execution. The ETM is designed to be a high-speed, low-power debug tool that only supports instruction trace"
- /soc/etm@7840000: 4.095 KB
- /soc/etm@7940000: 4.095 KB
- /soc/etm@7940000: 4.095 KB
- /soc/etm@7A40000: 4.095 KB
- /soc/etm@7A40000: 4.095 KB
- /soc/etm@7B40000: 4.095 KB
- /soc/etm@7B40000: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- funnel-base: 4.095 KB
- /soc/etm@7C40000: 4.095 KB
- /soc/etm@7C40000: 4.095 KB
- /soc/etm@7D40000: 4.095 KB
- /soc/etm@7D40000: 4.095 KB
- /soc/etm@7E40000: 4.095 KB
- /soc/etm@7E40000: 4.095 KB
- /soc/etm@7F40000: 4.095 KB
- /soc/etm@7F40000: 4.095 KB
- cnfg: 12.29 KB
  - Part of [Qualcomm SPMI Controller (PMIC Arbiter)](https://www.kernel.org/doc/Documentation/devicetree/bindings/spmi/qcom%2Cspmi-pmic-arb.txt)
- core: 4.095 KB
  - For SPMI as above
- chnls: 16.78 MB
  - For SPMI as above
- obsrvr: 16.78 MB
  - For SPMI as above
- intr: 2.228 MB
  - For SPMI as above
- /soc/ssusb@a800000/dwc3@a800000: 3.071 KB
  - [USB3 controller](https://android.googlesource.com/kernel/msm/+/android-wear-5.1.1_r0.6/Documentation/devicetree/bindings/usb/dwc3.txt)
- qmp_phy_base: 3.595 KB
  - Related to [Qualcomm QMP PHY Controller](https://www.kernel.org/doc/Documentation/devicetree/bindings/phy/qcom-qmp-phy.txt)
  - "supports physical layer functionality for a number of controllers on Qualcomm chipsets, such as, PCIe, UFS, and USB."
- qusb_phy_base: 679 B
  - Same as above
- mmc0: 787 B
  - SSD card driver
- c179000.i2c: 1.535 KB
  - I2C controller
- c17a000.i2c: 1.535 KB
  - I2C controller
- msm_serial: 4.095 KB
  - Serial controller
- c1b5000.i2c: 1.535 KB
  - I2C controller
- spi_qsd: 1.535 KB
  - SPI controller
- reg-base: 106 B
- cpp: 255 B
  - Related to [Qualcomm Video Camera Driver](https://android.googlesource.com/kernel/msm/+/android-7.1.0_r0.2/Documentation/devicetree/bindings/media/video/msm-cpp.txt)
- cci: 16.38 KB
  - [Camera Control Interface](https://lwn.net/Articles/735236/)
- cpp_hw: 12.29 KB
- jpeg_hw: 16.38 KB
  - [Camera Driver](https://android.googlesource.com/kernel/msm/+/android-msm-wahoo-4.4-oreo-dr1/arch/arm/boot/dts/qcom/msm8998-camera.dtsi)
- csid: 1.023 KB
- csid: 1.023 KB
- csid: 1.023 KB
- csid: 1.023 KB
- ispif: 3.071 KB
- csiphy: 4.095 KB
- csiphy: 4.095 KB
- csiphy: 4.095 KB
- jpeg_hw: 16.38 KB
- fd_core: 2.047 KB
  - [Qualcomm Face Detection](https://android.googlesource.com/kernel/msm.git/+/android-msm-bullhead-3.10-n-preview-1/Documentation/devicetree/bindings/media/video/msm-fd.txt)
  - "Face detection hardware block. The Face Detection Hardware Block will offload processing on the host and also reduce power consumption."
- fd_misc: 1.023 KB
- /soc/arm,smmu-mmss@cd00000: 262.1 KB
- msm-watchdog: 4.095 KB
- msm-gladiator-erp: 57.34 KB
- System RAM: 92.27 MB
- Kernel code: 25.03 MB
- Kernel data: 6.386 MB
- System RAM: 29.36 MB
- System RAM: 3.91 GB

