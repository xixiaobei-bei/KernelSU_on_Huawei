/*
 * batt_info.h
 *
 * battery information head file
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

#ifndef _BATT_INFO_H_
#define _BATT_INFO_H_

#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/of_platform.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/timekeeping.h>

#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/mtd/hisi_nve_interface.h>

#include <dsm/dsm_pub.h>

#include <huawei_platform/power/power_genl.h>
#include <huawei_platform/power/power_mesg.h>
#include <huawei_platform/power/power_dsm.h>
#include <huawei_platform/power/batt_info_pub.h>

#include "batt_aut_checker.h"
#include "batt_info_util.h"

enum batt_info_type {
	DMD_INVALID = 0,
	DMD_ROM_ID_ERROR,
	DMD_IC_STATE_ERROR,
	DMD_IC_KEY_ERROR,
	DMD_OO_UNMATCH,
	DMD_OBD_UNMATCH,
	DMD_OBT_UNMATCH,
	DMD_NV_ERROR,
	DMD_SERVICE_ERROR,
	DMD_UNMATCH_BATTS,
};

enum {
	IC_DMD_GROUP = 0,
	KEY_DMD_GROUP,
	SN_DMD_GROUP,
	BATT_INFO_DMD_GROUPS,
};

enum {
	SCT_SIGN_GLOAD_BIT_MASK = 0x1,
	SCT_SIGN_BOARD_BIT_MASK = 0x2,
};

struct batt_info;
typedef int (*final_sn_checker_t)(struct batt_info *drv_data);

struct dmd_record {
	struct list_head node;
	int dmd_type;
	int dmd_no;
	int content_len;
	char *content;
};

struct dmd_record_list {
	struct list_head dmd_head;
	struct mutex lock;
	struct delayed_work dmd_record_report;
};

#define DMD_INFO_MESG_SIZE  128
struct batt_info {
	char sn_buff[MAX_SN_LEN];
	int dmd_retry;
	int dmd_no;
	unsigned int sn_version;
	unsigned int sn_len;
	unsigned int total_checkers;
	const unsigned char *sn;
	struct delayed_work dmd_report_dw;
	struct work_struct check_work;
	final_sn_checker_t sn_checker;
	struct batt_chk_rslt result;
};

#endif /* _BATT_INFO_H_ */
