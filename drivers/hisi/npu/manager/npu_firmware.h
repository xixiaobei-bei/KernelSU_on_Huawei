/*
 * npu_firmware.c
 *
 * about npu firmware
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
#ifndef __NPU_FIRMWARE_H
#define __NPU_FIRMWARE_H

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <linux/pm.h>

#define DEVDRV_NPU_FW_IMA_PATH      "/vendor/firmware/npu_fw.img"
#define DEVDRV_NPU_FW_ES_IMA_PATH   "/vendor/firmware/npu_fw_es.img"
#define DEVDRV_TS_BINARY_PATH       "/vendor/firmware/tsch_fw.bin"
#define DEVDRV_AICPU_BINARY_PATH    "/vendor/firmware/aicpu_fw.bin"

#define TS_FW_REMAP_SIZE (512 * 1024)
#define TS_FW_MAX_SIZE (1024 * 1024)

#define DEVDRV_TS_BIN_CHEKC_LEN        32
#define DEVDRV_TS_BIN_MAX_SEGMENT_NUM  16

#define BIN_VALID_BUFF_LEN 1024

enum devdrv_cpu_type {
	DEVDRV_FW_TYPE_AICPU,
	DEVDRV_FW_TYPE_TS,
	DEVDRV_FW_TYPE_MAX
};

struct devdrv_check_sum {
	u8 check[DEVDRV_TS_BIN_CHEKC_LEN];
};

struct devdrv_ts_bin_segment {
	u32 offset;
	u32 len;
	struct devdrv_check_sum segment_check;
};

struct devdrv_ts_bin_info {
	u32 ts_check_file;
	u32 fw_data_len;
	struct devdrv_check_sum fw_data_check;
	u32 segment_num;
	struct devdrv_ts_bin_segment segment[DEVDRV_TS_BIN_MAX_SEGMENT_NUM];
};

int devdrv_load_cpu_fw(void);
u64 devdrv_get_firmware_phy_addr(int type);

#endif
