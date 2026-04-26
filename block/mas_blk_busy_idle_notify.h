/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: mas block busy idle framework
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

#ifndef __MAS_BLK_BUSY_IDLE_NOTIFY__
#define __MAS_BLK_BUSY_IDLE_NOTIFY__
#include <linux/blkdev.h>

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
#define BLK_IDLE_100MS 100
#define BLK_IDLE_500MS 500
#define BLK_IDLE_1000MS 1000
#define BLK_IDLE_2000MS 2000
#define BLK_IDLE_4000MS 4000
#define BLK_IDLE_6000MS 6000
#define BLK_IDLE_8000MS 8000
#define BLK_IDLE_10000MS 10000
#endif

/* Internal structure! */
struct blk_busyidle_nb {
	/* list into subscribed_event_list */
	struct list_head subscribed_node;
	struct timer_list handler_dur_check_timer;
	/* would be initialized by block layer rather than subscriber module */
	struct notifier_block busyidle_nb;
	/* Copy of subscriber's event_node */
	struct blk_busyidle_event_node event_node;
	/* provided by subscriber module */
	/* for internal maintenance */
	enum blk_io_state last_state;
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	enum blk_fg_io_state last_fg_io_state;
#endif
};

#endif /* __MAS_BLK_BUSY_IDLE_NOTIFY__ */
