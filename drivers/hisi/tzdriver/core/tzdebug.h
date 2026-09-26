/*
 * tzdebug.h
 *
 * for tzdriver debug
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef TZDEBUG_H
#define TZDEBUG_H

#include <linux/types.h>
struct ta_mem {
	char ta_name[64];
	uint32_t pmem;
	uint32_t pmem_max;
	uint32_t pmem_limit;
};

#define MEMINFO_TA_MAX 100
struct tee_mem {
	uint32_t total_mem;
	uint32_t pmem;
	uint32_t free_mem;
	uint32_t free_mem_min;
	uint32_t ta_num;
	struct ta_mem ta_mem_info[MEMINFO_TA_MAX];
};

int get_tee_meminfo(struct tee_mem *meminfo);
void tee_dump_mem(void);
int tzdebug_init(void);
void tzdebug_exit(void);

#endif
