# Android makefile for the driver instrumentation Module

CHIPSET := drv_perf
SELECT  := CONFIG_DRV_PERF=m

LOCAL_PATH := $(call my-dir)

# Build wlan.ko as $(WLAN_CHIPSET)_wlan.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := DRV_PERF_ROOT=$(DRV_PERF_BLD_DIR)/perf-mods

# We are actually building wlan.ko here, as per the
# requirement we are specifying <chipset>_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=drv_perf
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(SELECT)

include $(CLEAR_VARS)
LOCAL_MODULE              := $(CHIPSET).ko
LOCAL_MODULE_KBUILD_NAME  := drv_perf.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true

LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)

# include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Create Symbolic link
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
	ln -sf /system/lib/modules/$(CHIPSET)/$(LOCAL_MODULE) $(TARGET_OUT)/lib/modules/drv_perf.ko)
