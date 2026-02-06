/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: block layer DMD info
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "dsm_block.h"

static struct block_dsm_body block_dsm;

static struct dsm_block_type errno_arry[] = {
	{ CP_TIMEOUT_THRESHOLD, BLOCK_DMD_CP_IO },
	{ NORMAL_TIMEOUT_THRESHOLD, BLOCK_DMD_NORMAL_IO },
};

inline void dsm_block_record(enum DSM_BLOCK_TYPE type)
{
	if (unlikely(type >= DSM_BLOCK_MAX))
		return;

	block_dsm.rec.buff[type]++;
}

static inline void dsm_block_info_init(void)
{
	memset((void *)block_dsm.rec.buff, 0, sizeof(block_dsm.rec.buff));
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

void mas_blk_dsm_init(void)
{
	dsm_block_info_init();
	INIT_DELAYED_WORK(&block_dsm.dsm_work, dsm_block_handle_work);
	schedule_delayed_work(&block_dsm.dsm_work, ONE_DAY_JIFFIES);
}
