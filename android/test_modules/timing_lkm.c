#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timekeeping.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("137");
MODULE_DESCRIPTION("ktime accessor functions test");
MODULE_VERSION("0.1");

static int __init lkm_init(void)
{
	printk(KERN_INFO "137 TEST: timing module initialized\n");
	return 0;
}

static void __exit lkm_exit(void)
{
    struct timespec64 start = {0,0};
    struct timespec64 end = {0,0};
    struct timespec64 final = {0,0};

    ktime_get_ts64(&start);
    mdelay(1000);
    ktime_get_ts64(&end);
    final = timespec64_sub(end,start);

	printk(KERN_INFO "137 TEST: %llu.%0.9u\n", (u64)final.tv_sec, (u32)final.tv_nsec);
}

module_init(lkm_init);
module_exit(lkm_exit);
