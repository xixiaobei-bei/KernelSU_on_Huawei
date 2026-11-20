/*
 * proxy-hcd-stat.h
 *
 * utilityies for operating proxy-hcd status
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef _PROXY_HCD_STAT_H_
#define _PROXY_HCD_STAT_H_

#include <linux/time.h>

struct proxy_hcd_usb_device_stat {
	unsigned long stat_urb_complete_pipe_err;
};

static inline void urb_complete_pipe_err_add_stat(struct proxy_hcd_usb_device_stat *stat)
{
	stat->stat_urb_complete_pipe_err++;
}

struct proxy_hcd_urb_stat {
	unsigned long		stat_urb_enqueue;
	unsigned long		stat_urb_dequeue;
	unsigned long		stat_urb_enqueue_fail;
	unsigned long		stat_urb_dequeue_fail;
	unsigned long		stat_urb_dequeue_giveback;
	unsigned long		stat_urb_giveback;
	unsigned long		stat_urb_complete;
	unsigned long		stat_urb_complete_fail;
};

static inline void urb_enqueue_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_enqueue++;
}

static inline void urb_dequeue_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_dequeue++;
}

static inline void urb_enqueue_fail_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_enqueue_fail++;
}

static inline void urb_dequeue_fail_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_dequeue_fail++;
}

static inline void urb_dequeue_giveback_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_dequeue_giveback++;
}

static inline void urb_giveback_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_giveback++;
}

static inline void urb_complete_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_complete++;
}

static inline void urb_complete_fail_add_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_complete_fail++;
}

struct proxy_hcd_stat {
	unsigned int stat_alloc_dev;
	unsigned int stat_free_dev;

	unsigned int stat_hub_control;
	unsigned int stat_hub_status_data;

	unsigned long last_hub_control_time;
	unsigned long last_hub_status_data_time;

	unsigned int stat_bus_suspend;
	unsigned int stat_bus_resume;
};

static inline void alloc_dev_add_stat(struct proxy_hcd_stat *stat)
{
	stat->stat_alloc_dev++;
}

static inline void free_dev_add_stat(struct proxy_hcd_stat *stat)
{
	stat->stat_free_dev++;
}

static inline void bus_suspend_add_stat(struct proxy_hcd_stat *stat)
{
	stat->stat_bus_suspend++;
}

static inline void bus_resume_add_stat(struct proxy_hcd_stat *stat)
{
	stat->stat_bus_resume++;
}

#endif /* _PROXY_HCD_STAT_H_ */
