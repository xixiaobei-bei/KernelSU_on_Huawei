/*
 * dsm_block.c -- block layer DMD info
 *
 * Copyright (C) 2019 Hisilicon. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "dsm_block.h"

struct block_dsm_body block_dsm;

static struct dsm_block_type errno_arry[] = {
	{ CP_TIMEOUT_THRESHOLD, BLOCK_DMD_CP_IO},
	{ NORMAL_TIMEOUT_THRESHOLD, BLOCK_DMD_NORMAL_IO},
};

inline void dsm_block_record(enum DSM_BLOCK_TYPE type)
{
	if (unlikely(type >= DSM_BLOCK_MAX))
		return;

	block_dsm.rec.buff[type]++;
}

static void dsm_block_handle_work(struct work_struct *work)
{
	int i;

	for (i = 0; i < DSM_BLOCK_MAX; i++) {
		if (block_dsm.rec.buff[i] >= errno_arry[i].threshold)
			break;
	}

	if (i != DSM_BLOCK_MAX) {
		/* get disk space_left here */
		block_dsm.space_left = f2fs_get_free_size();
		/* treat the first type as the errno even there several types */
		dsm_block_log(BLOCK_DMD_CP_IO);
		dsm_block_info_init();
	}
	/* check every ONE_DAY_JIFFIES */
	schedule_delayed_work(&block_dsm.dsm_work, ONE_DAY_JIFFIES);
}

void hisi_blk_dsm_init(void)
{
	dsm_block_info_init();
	INIT_DELAYED_WORK(&block_dsm.dsm_work, dsm_block_handle_work);
	schedule_delayed_work(&block_dsm.dsm_work, ONE_DAY_JIFFIES);
}
