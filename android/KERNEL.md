Throughout this document we are using a rooted "Google Pixel 3 (blueline)" device.

# Backing up kernel
Steps are taken from [this SO post](https://android.stackexchange.com/questions/168374/backup-dump-kernel-image-without-root-or-twrp-cwm)

1. Download the image and TWRP source zip for your device on the [TWRP site](https://twrp.me/google/googlepixel3.html) ([link](https://twrp.me/google/googlepixel3.html) Pixel 3).
2. Install ADB (through Android Studio) and Fastboot
3. Identify the boot partition to back-up (as described in above SO post). For us these were:
```/dev/block/sda11 (boot_a)
/dev/block/sda12 (boot_b)

/dev/block/sda3 (keystore)
/dev/block/sda1 (ssd)

/dev/block/sda5 (system_a)
/dev/block/sda6 (system_b)

/dev/block/sda9 (vendor_a)
/dev/block/sda9 (vendor_b)```
5. Run `adb pull /dev/block/<partition-name>`
6. Run `adb reboot-bootloader` and wait until the device is in the boot screen
7. While still in the boot screen, run `fastboot boot twrp-<recovery image name>.img` that you downloaded in step 1
8. You can now backup to the SDCard from within the TWRP OS and then adb pull again. E.g., `adb pull /data/media/0/TWRP/BACKUPS/824X002UM/1970-03-16--01-43-56_blueline-userdebug_9_PI_engguchen2018071817`

# Installing Custom Kernel

# Reverting to Back-up Kernel
## From TWRP back-up
1. Run `adb reboot-bootloader` and wait until the device is in the boot screen
2. While still in the boot screen, run `fastboot boot twrp-<recovery image name>.img` for your device
3. Selet **Restore** and locate the backup folder on the sdcard
4. Reboot
