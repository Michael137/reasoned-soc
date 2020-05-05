#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

struct fastrpc_ioctl_perf {
	/* kernel performance data */
	uintptr_t data;
	uint32_t numkeys;
	uintptr_t keys;
};

#define FASTRPC_IOCTL_GETPERF _IOWR('R', 9, struct fastrpc_ioctl_perf)

int main(void) {
	int fd;
	struct fastrpc_ioctl_perf data;

	fd = open("/dev/adsprpc-smd", O_RDWR);

	if (fd == -1) {
		perror("open");
		return -1;
	}

	memset(&data, 0, sizeof(struct fastrpc_ioctl_perf));

	ioctl(fd, FASTRPC_IOCTL_GETPERF, &data);
	printf("keys: %lu\n"
			"numkeys: %u\n"
			"data: %lu\n", data.keys, data.numkeys, data.data);

	close(fd);

	return 0;
}
