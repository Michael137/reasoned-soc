/* Include Files */
#include <wlan_hdd_includes.h>

#ifdef MSM_PLATFORM
#include <soc/qcom/subsystem_restart.h>
#endif

#ifdef MODULE
#define DRV_PERF_MODULE_NAME  module_name(THIS_MODULE)
#else
#define DRV_PERF_MODULE_NAME  "drv_perf"
#endif

#ifdef TIMER_MANAGER
#define TIMER_MANAGER_STR " +TIMER_MANAGER"
#else
#define TIMER_MANAGER_STR ""
#endif

#ifdef MEMORY_DEBUG
#define MEMORY_DEBUG_STR " +MEMORY_DEBUG"
#else
#define MEMORY_DEBUG_STR ""
#endif

#ifdef PANIC_ON_BUG
#define PANIC_ON_BUG_STR " +PANIC_ON_BUG"
#else
#define PANIC_ON_BUG_STR ""
#endif

#define MODULE_INITIALIZED 1

static int drv_perf_module_init(void)
{
	return 0;
}

static void __exit drv_perf_module_exit(void)
{
}

/* Register the module init/exit functions */
module_init(drv_perf_module_init);
module_exit(drv_perf_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("137");
MODULE_DESCRIPTION("Driver Instrumentation Module");
