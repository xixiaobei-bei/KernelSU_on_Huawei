/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Tzdebug data declaration.
 * Author: luchenggang
 * Create: 2019-05-08
 */

#ifndef _TZDEBUG_H_
#define _TZDEBUG_H_

#include <linux/types.h>
struct ta_mem {
	char ta_name[16];
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

#endif
