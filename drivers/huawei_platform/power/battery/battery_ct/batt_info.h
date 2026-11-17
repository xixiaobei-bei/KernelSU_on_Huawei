/*
 * batt_info.h
 *
 * battery information head file
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
#include <linux/spinlock.h>
#include <linux/timekeeping.h>

#include <linux/power/hisi/coul/coul_drv.h>

#include <huawei_platform/power/power_mesg_srv.h>
#include <chipset_common/hwpower/common_module/power_genl.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <huawei_platform/power/batt_info_pub.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_nv.h>

#include <chipset_common/hwpower/battery/battery_soh.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

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

enum check_strategy {
	CHECK_STRATEGY_INVALID = 0,
	CHECK_STRATEGY_DEBUG = 1,
	CHECK_STRATEGY_BOOTING = 2,
	CHECK_STRATEGY_PERIOD = 3,
	CHECK_STRATEGY_MAX_NO = CHECK_STRATEGY_PERIOD,
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
	spinlock_t request_lock;
	bool is_checking;
	bool can_check_in_running;
	bool is_first_check_done;
	int check_strategy_no;
	struct wakeup_source checking_wakelock;
};

enum powerct_error_code {
	ERROR_CODE_START,
	ERROR_CODE_DEFAULT = ERROR_CODE_START,
	ERROR_CODE_IS_PROHIBITED_ID,
	ERROR_CODE_END = ERROR_CODE_IS_PROHIBITED_ID,
};

#endif /* _BATT_INFO_H_ */
