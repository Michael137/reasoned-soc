#!/bin/bash

cd ../kernels/

scp -i ~/.ssh/chips mbuch@acc40.int.seas.harvard.edu:/group/brooks/mbuch/kernels/msm/out/android-msm-pixel-4.9/dist/Image.lz4 .

RAND=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 13 ; echo '')
KERNEL_DIRNAME="msm-android10-${RAND}"

mkdir "${KERNEL_DIRNAME}"

mv Image.lz4 "${KERNEL_DIRNAME}/"

scp -i ~/.ssh/chips mbuch@acc40.int.seas.harvard.edu:/group/brooks/mbuch/kernels/msm/out/android-msm-pixel-4.9/dist/*.ko ${KERNEL_DIRNAME}/

echo "Copied to: ${KERNEL_DIRNAME}"
echo "Setting TARGET_PREBUILT_KERNEL=$PWD/${KERNEL_DIRNAME}/Image.lz4"

export TARGET_PREBUILT_KERNEL=$PWD/${KERNEL_DIRNAME}/Image.lz4

cd ../aosp/

adb root
adb remount -R

TARGET_DIR=$(dirname $TARGET_PREBUILT_KERNEL)

adb push ${TARGET_DIR}/*.ko /vendor/lib/modules

m bootimage -j12

adb reboot

~/platform-tools/fastboot flash boot out/target/product/blueline/boot.img
