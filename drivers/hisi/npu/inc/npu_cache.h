/*
 * npu_cache.h
 *
 * about npu cache
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef _NPU_CACHE_H_
#define _NPU_CACHE_H_

#define CACHE_LINE_LENTH    64

#ifdef __aarch64__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void devdrv_flush_cache(unsigned char *base, unsigned int len)
{
	int i;
	int num = len / CACHE_LINE_LENTH;

	i = ((len % CACHE_LINE_LENTH) > 0) ? num++ : num;
	num = i;

	asm volatile ("dsb st" : : : "memory");
	for (i = 0; i < num; i++) {
		asm volatile ("DC CIVAC ,%x0" ::"r" (base + i * CACHE_LINE_LENTH));
		mb();
	}

	asm volatile ("dsb st" : : : "memory");
}
#pragma GCC diagnostic pop
#endif

#endif /* _NPU_CACHE_H_ */
