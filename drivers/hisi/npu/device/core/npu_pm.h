/*
 * npu_pm.h
 *
 * about npu pm
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
#ifndef __NPU_PM_H
#define __NPU_PM_H

#include <linux/types.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>

#include "npu_common.h"
#include "npu_proc_ctx.h"

typedef int (*software_ops_func) (u8 dev_id);
typedef void (*hardware_ops_func) (void);

struct devdrv_dev_res_software_ops {
	software_ops_func devdrv_res_software_init;
	software_ops_func devdrv_res_software_available;
	software_ops_func devdrv_res_software_recycle;
	software_ops_func devdrv_res_software_deinit;
	struct list_head list;
};

struct devdrv_dev_res_hardware_ops {
	hardware_ops_func devdrv_res_hardware_init;
	hardware_ops_func devdrv_res_hardware_available;
	hardware_ops_func devdrv_res_hardware_recycle;
	hardware_ops_func devdrv_res_hardware_deinit;
	struct list_head list;
};

enum devdrv_power_stage {
	DEVDRV_PM_DOWN,
	DEVDRV_PM_NPUCPU,
	DEVDRV_PM_SMMU,
	DEVDRV_PM_TS,
	DEVDRV_PM_UP
};

int npu_open(struct devdrv_dev_ctx *dev_ctx);

int npu_powerup(struct devdrv_proc_ctx *proc_ctx, struct devdrv_dev_ctx *dev_ctx, u32 secure_state);

int npu_release(struct devdrv_dev_ctx *dev_ctx);

int npu_powerdown(struct devdrv_proc_ctx *proc_ctx, struct devdrv_dev_ctx *dev_ctx);

int devdrv_dev_software_register(struct devdrv_dev_ctx *dev_ctx, software_ops_func init,
                                 software_ops_func available, software_ops_func recycle,
                                 software_ops_func deinit);

int devdrv_dev_software_unregister(struct devdrv_dev_ctx *dev_ctx);

int npu_sync_ts_time(void);

#endif
