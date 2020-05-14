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
	struct kgsl_perfcounter_query data;
	unsigned int countables[16];

	fd = open("/dev/kgsl-3d0", O_RDWR);

	if (fd == -1) {
		perror("open");
		return -1;
	}

	memset(&data, 0, sizeof(struct kgsl_perfcounter_query));

	data.groupid = 1;
	data.countables = (unsigned int *) &countables;
	data.count = 0x80000001;

	ioctl(fd, IOCTL_KGSL_PERFCOUNTER_QUERY, &data);
	printf("max_counters: %u\n"
			"count: %u\n"
			"groupid: %u\n", data.max_counters, data.count, data.groupid);

	close(fd);

	return 0;
}
