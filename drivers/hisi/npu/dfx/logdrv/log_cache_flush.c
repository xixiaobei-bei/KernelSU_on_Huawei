#include "log_drv_dev.h"

void log_flush_cache(const unsigned char *base, unsigned int len)
{
	int i;
	int num = len / LOG_CACHE_LINE;

	i = ((len % LOG_CACHE_LINE) > 0) ? num++ : num;
	num = i;

	asm volatile ("dsb st" : : : "memory");
	for (i = 0; i < num; i++) {
		asm volatile ("DC CIVAC ,%x0" ::"r" (base + i * LOG_CACHE_LINE));
		mb();
	}

	asm volatile ("dsb st" : : : "memory");
}

