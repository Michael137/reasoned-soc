# SoC RE
## dts Files
- DTS are used by the kernel to identify the drivers to call
- These files get compiled to device tree blobs by a **device tree compiler (DTC)**
  - See [these slides on the Device Tree](https://bootlin.com/pub/conferences/2013/elce/petazzoni-device-tree-dummies/petazzoni-device-tree-dummies.pdf)
  - "A device tree is a tree data structure with nodes that describe the physical devices in a system"
  - device tree blob is "binary that gets loaded by the bootloader and parsed by the kernel at boot time"
  - "ranges property can describe an address translation between the child bus and the parent bus. When simply defined as ranges;, it means that the translation is an identity translation"
- To determine the device tree source for your kernel you can use the **dtc** on Linux
  - Install using: `sudo apt install device-tree-compiler`
  - As described [here](https://unix.stackexchange.com/questions/265890/is-it-possible-to-get-the-information-for-a-device-tree-using-sys-of-a-running), you can run **dtc** on your device tree binary to reconstruct the monolithic combined DTS file
    - Run: `dtc -I fs -O dts /sys/firmware/devicetree/base`
    - In the case of an Android SoC, first run: `adb pull /sys/firmware/devicetree devicetree`
- [Device Tree Overview/Specification](https://devicetree-specification.readthedocs.io/en/latest/index.html)

The APQ8098 DTS we generated is [here](apq8098.dts)
