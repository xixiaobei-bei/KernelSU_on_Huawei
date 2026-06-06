#ifndef _NPU_CACHE_H_
#define _NPU_CACHE_H_

#define CACHE_LINE_LEN	(64)

#ifdef __aarch64__
static inline void devdrv_flush_cache(unsigned char *base, unsigned int len)
{
	int i;
	int num = len / CACHE_LINE_LEN;

	i = ((len % CACHE_LINE_LEN) > 0) ? num++ : num;
	num = i;

	asm volatile ("dsb st" : : : "memory");
	for (i = 0; i < num; i++) {
		asm volatile ("DC CIVAC ,%x0" ::"r" (base + i * CACHE_LINE_LEN));
		mb();
	}

	asm volatile ("dsb st" : : : "memory");
}
#endif

#endif/* _NPU_CACHE_H_ */
