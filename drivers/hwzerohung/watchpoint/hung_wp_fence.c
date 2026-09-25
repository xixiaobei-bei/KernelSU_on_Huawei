/*
 * hung_wp_fence.c
 *
 * fence timeout watchpoint implementation file.
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/version.h>
#include "chipset_common/hwzrhung/zrhung.h"

/*
 * Exception: sync_dump in linux-4.9/drivers/dma-buf/sync_debug.h,
 * can't hack into the kernel code, but not in the default search path
 */
#ifdef CONFIG_SW_SYNC
extern void sync_dump(void);
#endif

/* 4.14 kernel, there is no fence.c */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))

void fencewp_report(long timeout, bool dump)
{
	char fence_buf[128] = {0};	/* buffer fize */

	if (dump == true) {
#ifdef CONFIG_SW_SYNC
		sync_dump();
#else
		;
#endif
	}

	snprintf(fence_buf, sizeof(fence_buf), "fence timeout after %dms\n",
		 jiffies_to_msecs(timeout));
	zrhung_send_event(ZRHUNG_WP_FENCE, "K", fence_buf);
}
#endif
