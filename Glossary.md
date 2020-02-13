[MSM](https://en.wikichip.org/wiki/qualcomm/msm): series of Qualcomm SoCs with networking capabilities
  - [Android Kernel Source](https://android.googlesource.com/kernel/msm.git/)
SMMU: system memory management unit
DT: device tree
.dts: device tree source file
.dtsi: device tree source include file
[/proc/iomem](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/4/html/Reference_Guide/s2-proc-iomem.html):
  - current map of the system's memory for each physical device
  - first column displays the memory registers used by each of the different types of memory. The second column lists the kind of memory located within those registers and displays which memory registers are used by the kernel within the system RAM
  - [Mapping virtual to physical memory in the Linux Kernel](https://medium.com/@gabrio.tognozzi/linux-memory-cheat-sheet-2c7454aa1e29)
    - in the Kernel [source](https://elixir.bootlin.com/linux/v3.10.92/source/arch/x86/include/asm/io.h)
- [/sys/devices/soc](https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-devices-soc)
- LPASS: Low-power audio subsystem
  - [Used in Qualcomm's Voice Assistant](https://www.qualcomm.com/products/features/voice-assist)
  - [Details of its interaction with rest of SoC](https://www.qualcomm.com/news/onq/2018/03/22/how-can-qualcomm-aqstic-support-dynamic-voice-ui-experiences-video)
    - "The audio codec DSP then loops into the Snapdragon 845’s sophisticated low-power audio subsystem (LPASS), which includes a Qualcomm Hexagon 685 Scalar Processor, architecturally designed for audio uses. LPASS is the heart of the audio and voice experience on your Snapdragon-powered phone, playing a vital part in voice UI processing. In this role, LPASS performs voice verification to confirm your identity"
    - "The LPASS and Kryo CPU also include specialized hardware for high-quality audio processing, which is an area of circuitry that serves as a bridge to the digital-to-analog converter (DAC) on the audio codec. As the LPASS arrives at your answer, it’s engineered to direct it to the DAC, which converts the signal back to analog and pipes it to the output speaker for you to hear."
    - ![LPASS SoC Component](figs/lpass_hardware.png)
