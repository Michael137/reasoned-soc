#include <linux/timekeeping.h>

void work()
{
	sleep(5);
}

int main()
{
	struct timespec64 start = {0,0};
	struct timespec64 end = {0,0};
	struct timespec64 final = {0,0};

	ktime_get_ts64(&start);
	work();
	ktime_get_ts64(&end);
	final = timespec64_sub(end,start);

	printf("%llu.%0.9u\n", (u64)final.tv_sec, (u32)final.tv_nsec);

	return 0;
}
