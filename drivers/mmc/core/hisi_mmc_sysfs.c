/*
 * add support for manual bkops
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#ifdef CONFIG_HISI_MMC_MANUAL_BKOPS
#include <linux/blkdev.h>
#endif

int hisi_mmc_add_card_debugfs(struct mmc_card *card, struct dentry *root)
{
#ifdef CONFIG_HISI_MMC_FLUSH_REDUCE
	if (!debugfs_create_u8("flush_reduce_en", S_IRUSR, root,
		    &(card->host->mmc_flush_reduce_enable)))
		return -1;
#endif

	return 0;
}
