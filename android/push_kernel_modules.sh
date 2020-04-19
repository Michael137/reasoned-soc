adb root
adb remount -R

TARGET_DIR=$(dirname $TARGET_PREBUILT_KERNEL)

adb push ${TARGET_DIR}/*.ko /vendor/lib/modules
