/* Include Files */
#include <drv_perf_dsp_includes.h>

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h> /* for MKDEV */

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

#define DEVICE_NAME "drv_perf"
#define DEVICE_PATH "/dev/drv_perf"

static int major_no;
#define MAGIC_NO '4'
/* 
 * Set the message of the device driver 
 */
#define IOCTL_CMD _IOR(MAGIC_NO, 0, char *)

static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT "DRV_PERF: opened device \n");
	return 0;
}

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	switch(cmd)
	{
		case IOCTL_CMD:
			printk(KERN_ALERT "DRV_PERF: %s\n", (char *)args);
			break;
	}
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT "DRV_PERF: released device \n");
	return 0;
}

struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};

struct class *cl;

static int drv_perf_module_init(void)
{
	printk(KERN_ALERT "DRV_PERF: driver initialized %s\n", __FUNCTION__);

	major_no = register_chrdev(0, DEVICE_NAME, &fops);
	printk(KERN_ALERT "DRV_PERF: Major_no : %d\n", major_no);

	cl = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cl, NULL, MKDEV(major_no,0), NULL, DEVICE_NAME);
	printk(KERN_ALERT "DRV_PERF: Device Initialized in kernel ....!!\n");
	return 0;
}

static void __exit drv_perf_module_exit(void)
{
	device_destroy(cl,MKDEV(major_no,0));
	class_unregister(cl);
	class_destroy(cl);
	unregister_chrdev(major_no, DEVICE_NAME);
	printk(KERN_ALERT "DRV_PERF: driver exited %s\n", __FUNCTION__);
}

/* Register the module init/exit functions */
module_init(drv_perf_module_init);
module_exit(drv_perf_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("137");
MODULE_DESCRIPTION("Driver Instrumentation Module");
