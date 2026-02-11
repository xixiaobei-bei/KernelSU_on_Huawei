/*
 * battery_soh.h
 *
 * Driver adapter for bsoh.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_SOH_H_
#define _BATTERY_SOH_H_

#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/mutex.h>

#define MAX_DMD_BUF_SIZE         1024
#define MAX_DMD_REPORT_TIMES     3
#define MAX_RD_BUF_SIZE          64

enum bsoh_sub_sys_type {
	BSOH_SUB_SYS_BEGIN = 0,
	BSOH_SUB_SYS_BASP = BSOH_SUB_SYS_BEGIN,
	BSOH_SUB_SYS_ASW,
	BSOH_SUB_SYS_END,
};

struct bsoh_dmd_entry {
	struct list_head node;
	unsigned int dmd_no;
	unsigned int retry_times;
	void (*prepare)(char *buff, unsigned int size);
};

struct bsoh_device {
	struct device *dev;
	struct delayed_work bsoh_dmd_wrok;
	struct list_head dmd_list_head;
	struct mutex dmd_list_lock;
	unsigned int sub_sys_enable;
};

struct bsoh_sub_sys {
	int (*sys_init)(struct bsoh_device *di);
	void (*sys_exit)(struct bsoh_device *di);
	void (*event_notify)(struct bsoh_device *di, unsigned int event);
	void (*dmd_prepare)(char *buff, unsigned int size);
	const char *type_name;
	const char *notify_node;
};

#ifdef CONFIG_HUAWEI_BATTERY_SOH
void bsoh_event_rcv(unsigned int event);
void bsoh_dmd_append(const char *type_name, unsigned int dmd_no);
#else
void bsoh_event_rcv(unsigned int event) {}
void bsoh_dmd_append(const char *type_name, unsigned int dmd_no) {}
#endif /* CONFIG_HUAWEI_BATTERY_SOH */

#endif /* _BATTERY_SOH_H_ */
