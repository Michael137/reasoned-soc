Throughout this document we are using a rooted "Google Pixel 3 (blueline)" device.

# Backing up kernel
Steps are taken from [this SO post](https://android.stackexchange.com/questions/168374/backup-dump-kernel-image-without-root-or-twrp-cwm)

1. Download the image and TWRP source zip for your device on the [TWRP site](https://twrp.me/google/googlepixel3.html) ([link](https://twrp.me/google/googlepixel3.html) Pixel 3).
2. Install ADB (through Android Studio) and Fastboot
3. Identify the boot partition to back-up (as described in above SO post)
5. Run `adb pull /dev/block/<partition-name>`
6. Run `adb reboot-bootloader` and wait until the device is in the boot screen
7. While still in the boot screen, run `fastboot boot twrp-<recovery image name>.img` that you downloaded in step 1
8. You can now backup to the SDCard from within the TWRP OS and then adb pull again. E.g., `adb pull /data/media/0/TWRP/BACKUPS/824X002UM/1970-03-16--01-43-56_blueline-userdebug_9_PI_engguchen2018071817`

# Reverting to Back-up Kernel
## From TWRP back-up
1. Run `adb reboot-bootloader` and wait until the device is in the boot screen
2. While still in the boot screen, run `fastboot boot twrp-<recovery image name>.img` for your device
3. Selet **Restore** and locate the backup folder on the sdcard
4. Reboot

### Troubleshooting
- See some problems we ran into in [this](https://forum.xda-developers.com/pixel-3/help/android-recovery-factory-reset-t4072515/) and [this](https://forum.xda-developers.com/pixel-3/help/building-aospblueline-userdebug-server-t4073455) XDA Developers thread
## Flash factory image
0. Boot into fastboot bootloader menu (press power and volume down for a few seconds)
1. Download and unzip factory image from [here](https://developers.google.com/android/images)
2. Run `flash-all.sh`. **Note**: be patient or erase system, data and cache using TWRP and run the flash command after that
3. **Optional**: To re-install the **userdebug** image, check it out using `lunch`, build it within AOSP for your target device and flash it

# Building AOSP
- Useful references:
  - https://medium.com/@christopherney/building-android-o-with-a-mac-da07e8bd94f9
  - http://effie.io/building-aosp-part-2/
  - https://medium.com/@bojanbelic/aosp-dev-3-building-your-first-android-image-on-ubuntu-part-2-ae90b0b489ba
  - http://skaleguru.com/2019/06/15/building-aosp-on-macos/
  - [Downloading/initializing AOSP source](https://source.android.com/setup/build/downloading)
  - https://www.polidea.com/blog/How-to-Build-your-Own-Android-Based-on-AOSP/
  - https://forum.xda-developers.com/pixel-3a/development/compiling-aosp-scratch-t3958141

1. add `repo` command to path
2. `repo init`
  - Choose compatible branch from: https://source.android.com/setup/start/build-numbers#source-code-tags-and-builds
3. `repo sync -qc -j12`
4. `build/envsetup.sh`
5. `lunch <variation>`
  - For Pixel 3 <variation> is: aosp_blueline-userdebug (see https://source.android.com/setup/build/running)
6. Download the vendor binaries and extract them at the root of AOSP
  - Qualcomm and Google binaries for the particular build and device numbers are available [here](https://developers.google.com/android/drivers)
  - For more info see the [Android docs on this topic](https://source.android.com/setup/build/downloading#extracting-proprietary-binaries)
7. `make -j14`
  - You might have to run: `make SELINUX_IGNORE_NEVERALLOWS=true -j14` if you are on MacOS
8. Reboot
9. (Optional) Install Google Play Store:
  - Follow [this SO post](https://stackoverflow.com/questions/41695566/install-google-apps-on-aosp-build/41818710#41818710)
    - In the above guide, **PrebuiltGmsCore** is renamed **PrebuiltGmsCorePi** in the Android 9 build
    - To be able to push files run:
      - `adb disable-verity`
      - `adb root`
      - `adb remount`
      - `adb shell mount -o rw,system /;`
    - Once installed, if the phone enters a bootloop this is likely due to whitelisting issues. You will have to add the neccessary permissions to the **/etc/permissions/privapp-permissions-blueline** file. To get the package name and permissions to add follow the [Android docs](https://source.android.com/devices/tech/config/perms-whitelist) on this topic. In short you have do the following:
      - `adb pull /system/build.prop`
      - Edit build.prop s.t. **ro.control_privapp_permissions=log**
      - `adb push build.prop /system`
      - `adb reboot`
      - `adb shell`
        - Search the `logcat` for the string "Privileged permission". Now add a permissions whitelist file for your device into /etc/permissions. E.g., for Pixel 3 blueline it is **/etc/permissions/privapp-permissions-blueline**
      - `adb reboot`
  - **Troubleshooting**: For Android 10 you can download the Google Services Framework (GSF), Play Services (GMS) and [Phonesky](https://androidforums.com/threads/what-exactly-is-phonesky-apk.755972/) APKs and install them to the directories above. For Android 10 (i.e., not Android Pie), install the GMS APK to `/system/priv-app/GmsCore/GmsCore.apk`. Then add the necessary whitelist permissions as outlined above.
    - See [this SO post](https://android.stackexchange.com/questions/209781/install-google-play-services-without-google-play-store) and [this forum post](https://www.element14.com/community/community/designcenter/single-board-computers/riotboard/blog/2014/05/14/installing-google-play-services-and-google-play-store-on-riotboard) for more info on these APKs

10. (Optional) Install Chrome browser:
  - Download and [install](https://stackoverflow.com/questions/7076240/install-an-apk-file-from-command-prompt) the Chrome browser apk
11. (Optional) Make sure the Hexagon DSP runtime works. [If the libadsprpc.so is missing copy it from the Hexagon SDK](https://developer.qualcomm.com/forum/qdn-forums/software/hexagon-dsp-sdk/toolsinstallation/34446)
12. (Optional) Make sure Qualcomm FastRPC works. Check the `$HEXAGON_SDK/examples/common/rpcperf` example
  - More info in the [../dsp_offload/README.md](../dsp_offload/README.md) directory of this repository

## Embedding Custom Kernel
The kernel and AOSP projects are separate. To change the kernel of an AOSP build one has to check out and build the kernel image separately, embed it into the AOSP tree, rebuild the boot image and reflash the device.

### Modern Kernel
Instructions are mostly from [Android docs](https://source.android.com/setup/build/building-kernels) to build and embed the kernel in the AOSP tree

1. Build kernel
2. Copy `Image.lz4` file to some `<DIR>`
3. `export TARGET_PREBUILT_KERNEL=<DIR>/Image.lz4`
4. From the AOSP root run: `m bootimage -j12`
8. `adb root`
9. `adb remount -R`
5. Copy the *.ko files from the kernel build output to `/vendor/lib/modules` on your device
  - `adb push <KERNEL SOURCE>/group/brooks/mbuch/kernels/msm/out/android-msm-pixel-4.9/dist/*.ko /vendor/lib/modules`
6. `adb reboot-bootloader`
6. `fastboot flash boot out/target/product/blueline/boot.img`
7. Reboot
8. Check kernel version in adb shell using `uname -a` or `cat /proc/version`

(Above instructions were taken from [this thread on a broken touchscreen driver](https://groups.google.com/forum/#!topic/android-building/ou630PviyDc))

### Old kernel
1. Get the kernel version to install from your AOSP build. See the [docs](https://source.android.com/setup/build/building-kernels#id-version-from-aosp)
2. Follow the build instructions in the [docs](https://source.android.com/setup/build/building-kernels-deprecated)
  - The prebuilt Google toolchain does not come with a cross-compiler anymore. Instead set `CROSS_COMPILE=<some prefix>/android-ndk-r10e/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-`
  - Download the Android toolchain with the cross compiler from [here](https://android.magicer.xyz/ndk/guides/standalone_toolchain.html)

### Customizing Kernel Configuration
For modern Kernels the defconfig can be specified in the top-level `build.config` file. The custom defconfig file should be called `<custom name>_defconfig`.
1. Navigate to `private/msm-google/arch/arm64/configs/`
2. `cp <defconfig template> <custom name>_defconfig`
  - For Pixel 3 Android 10 blueline the config is **b1c1_defconfig**
3. Make any changes to your custom config
4. Comment out the `POST_DEFCONFIG_CMDS` line in `build.config`
  - These perform `check_defconfig` and `update_nocfig_config` which will prevent you from customizing the defconfig
5. Comment out following chunk of the `prepare3` target in the main kernel makefile (e.g., `msm/private/msm-google/Makefile`):
`ifneq ($(KBUILD_SRC),)
...
... $(srctree) is not clean, please run 'make mkrpoper'
endif
`
  - By default the build does not allow modifications to the source. By commenting out the above check the compilation should now go through with your custom defconfig
6. Follow [these steps](https://unix.stackexchange.com/q/421479) to save the custom defconfig configuration. These steps are outlined below (run from kernel source root, i.e., `msm/private/msm-google`)
  - `make ARCH=arm64 <custom name>_defconfig`
  - `diff -u .config ./arch/arm64/configs/<custom name>_defconfig | diffstat` should show your additions and potentially some more added by the build system
  - `grep <your added feature> .config` should show your features added
  - make ARCH=arm64 savedefconfig
  - `grep <your added feature> defconfig` should still show the additions you are interested in
7. OPTIONAL: If you want to build the `msm-google-modules` modules (e.g., the wlan drivers) then you add `EXT_MODULES=private/msm-google-modules` to your `build.config`
8. Run `build/build.sh -j30` from the root of the overall msm build repo (not necessarily the kernel source)

### Adding and Compiling Custom Device Driver/Kernel Module
If you're using Google's `build.sh` script to build your kernel then follow below instructions on how to add your own module subdirectory and quickly compile it. The following instructions assume that you already modified the build configuration as in the section **Customizing Kernel Configuration** above to prevent any error when changing kernel code.

1. Add a directory under `msm/private/` where your new modules will live.
2. Edit the `EXT_MODULES` variable in `private/msm-google/build.config.common` to include your directory from step 1
3. Add a `Kbuild`, `Kconfig`, `Makefile` and `Android.mk` to your module directory using our custom module [template](custom_modules/perf-mods/) as a template. Your directory structure should look like:
```
msm/
    my-modules/
        my-module/
           Kbuild
           Kconfig
           Android.mk
           Makefile
		   src/
           inc/
```
**Note**: in our module example the `src` and `inc` directories are nested deeper inside other directories. This is not necessary unless you need to support multiple sub-devices or complex module dependencies
4. To build the module simply run `build/build.sh`
- **Note**: to speed up the build you can add `SKIP_MRPROPER=1` to `build.config` in the project root which will prevent cleanup of previous builds
5. To use your module do:
- `adb push <my_module>.ko /vendor/lib/modules`
- `adb shell insmod /vendor/lib/modules/<my_module>.ko`
- Check using: `adb shell lsmod`
- Remove using: `adb shell rmmod my_module.ko`

See [this SO post](https://stackoverflow.com/questions/6282669/how-do-you-create-a-loadable-kernel-module-for-android) for more info on writing drivers

## Troubleshooting
- sha256sum not found: brew install coreutils
- UTF-8 encoding:https://stackoverflow.com/questions/26067350/unmappable-character-for-encoding-ascii-but-my-files-are-in-utf-8
- Mac sdk problems (see https://stackoverflow.com/questions/50760701/could-not-find-a-supported-mac-sdk-10-10-10-11-10-12-10-13): download the old sdk from (https://roadfiresoftware.com/2017/09/how-to-install-multiple-versions-of-xcode-at-the-same-time/)
  - make sure `xcodebuild -showsdks` returns a compatible SDK with the AOSP build
    - If it throws an error like `Symbol not found: _OBJC_IVAR_$_NSScroller._action` you need to xselect Xcode
  - Alternatively you can also copy the SDK into the existing Xcode.app: Download from https://github.com/phracker/MacOSX-SDKs/releases and copy it to /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
- Case sensitive file system: reformat drive to be MacOS extended (journaled, case-sensitive)
- sepolicy_tests segfault:
  - https://stackoverflow.com/questions/58763047/failed-sepolicy-check-android-10-building-pixel-3a-xl
  - https://groups.google.com/forum/#!topic/android-building/_VyLXSosgoo
- sed: illegal option -- z:
  - https://stackoverflow.com/questions/43696304/how-do-i-fix-sed-illegal-option-r-in-macos-sierra-android-build
- On following th errror **ld: symbol(s) not found for architecture i386**, your mac sdk is likely too high. Solution: downgrade SDK to what is supported by your AOSP build
- **ERROR: Couldn't create a device interface iterator:**
  - Your fastboot is likely outdated because it is using the version that got built with AOSP (check using `which fastboot`)
  - [Download the latest fastboot and adb](https://android.stackexchange.com/questions/209725/fastboot-devices-command-doesnt-work-after-macos-high-sierra-10-14-4-upgrade) and use these
  - E.g., /Users/gardei/platform-tools/fastboot flashall -w
- Fastboot operation not supported:
  - Flash the factory image for the operating system you are replacing and retry the fastboot flashall
- `fastboot devices` doesn't list your device:
  - Unplug all other USB devices and try again
    - Other USB connections can interfere with your phone's connection

# Compiling code to run on an Android phone
Here we describe the process of compiling C code, pushing the binary to an Android device and running it through ADB.

1. Download the Android NDK
2. See the [Linux compile.sh](./compile_and_run_linux.sh) or the [MacOS compile.sh](./compile_and_run_macos.sh) for the actual compilation/running the binary
