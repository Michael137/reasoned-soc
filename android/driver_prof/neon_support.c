#include <sys/auxv.h>
#include <asm/hwcap.h>
#include <stdio.h>

#define HWCAP_NEON	(1 << 12)

int main() {
	unsigned long found = getauxval(AT_HWCAP) & HWCAP_ASIMD;
	printf("Neon support: %lu\n", found);
	return 0;
}
