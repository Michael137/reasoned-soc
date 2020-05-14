#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "driver_prof_shared.h"

int main(void) {
	int fd;
	struct fastrpc_ioctl_perf data;

	fd = open("/dev/adsprpc-smd", O_RDWR);

	if (fd == -1) {
		perror("open");
		return -1;
	}

	memset(&data, 0, sizeof(struct fastrpc_ioctl_perf));

	struct fastrpc_perf frpc_d;
	memset(&frpc_d, 0, sizeof(struct fastrpc_perf));
	data.data = (uintptr_t)&frpc_d;

	ioctl(fd, FASTRPC_IOCTL_GETPERF, &data);
	printf("keys: %lu\n"
			"numkeys: %u\n"
			"data: %lu\n", data.keys, data.numkeys, data.data);

	close(fd);

	printf("Data content: %ld %ld %ld\n", frpc_d.count, frpc_d.flush, frpc_d.tid);

	return 0;
}
