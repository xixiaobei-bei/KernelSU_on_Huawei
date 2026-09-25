/*
 * npu_dfx_profiling.h
 *
 * about npu dfx profiling
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
#ifndef _PROF_DRV_DEV_H_
#define _PROF_DRV_DEV_H_

#include <linux/cdev.h>
#include "devdrv_user_common.h"

#ifndef STATIC
#define STATIC
#endif

#define CHAR_DRIVER_NAME  "prof_drv"
#define DRV_MODE_VERSION  "DAVINCI 1.01"

#define AI_CPU_MAX_PMU_NUM      8
#define AI_CORE_MAX_PMU_NUM     8

/* profiling reserved memory size is 1M;
* head size = 4K; TSCPU DATA size  256k;
* AICPU DATA SIZE:256K
* AICORE DATA SIZE 508k
*/
#define PROF_HEAD_MANAGER_SIZE          0x1000
#define PROF_TSCPU_DATA_SIZE            0x40000
#define PROF_AICPU_DATA_SIZE            0x40000
#define PROF_AICORE_DATA_SIZE           (0x80000 - 0x1000)

#define PROF_SETTING_INFO_SIZE          256

#define PROF_OK                         0
#define PROF_ERROR                      (-1)

struct prof_ts_cpu_config {
	uint32_t en; /* enable=1 or disable=0 */
	uint32_t task_state_switch; /* 1 << TsTaskState means report */
	uint32_t task_type_switch; /* 1 << TsTaskType means report */
};

struct prof_ai_cpu_config {
	uint32_t en;
	uint32_t event_num;
	uint32_t event[AI_CPU_MAX_PMU_NUM];
};

struct prof_ai_core_config {
	uint32_t en;
	uint32_t event_num;
	uint32_t event[AI_CORE_MAX_PMU_NUM];
};

struct prof_setting_info {
	struct prof_ts_cpu_config tscpu;
	struct prof_ai_cpu_config aicpu;
	struct prof_ai_core_config aicore;
};

enum prof_channel{
	PROF_CHANNEL_TSCPU = 0,
	PROF_CHANNEL_AICPU,
	PROF_CHANNEL_AICORE,
	PROF_CHANNEL_MAX,
};

struct prof_read_config{
	char __user *dest_buff;
	uint32_t dest_size;
	uint32_t src_size;
	uint32_t channel;
};

struct ring_buff_manager {
	volatile uint32_t read;
	uint32_t reserved0[15];  /* because cache line size  is 64 bytes*/
	volatile uint32_t write;
	uint32_t reserved1[15];
};

struct prof_buff_manager {
	union {
		struct prof_setting_info info;
		char data[PROF_SETTING_INFO_SIZE];
	}cfg;
	struct ring_buff_manager ring_buff[PROF_CHANNEL_MAX];
	u32 start_flag;
};

struct prof_buff_desc {
	union {
		struct prof_buff_manager manager;
		char data[PROF_HEAD_MANAGER_SIZE];
	} head;
	volatile char tscpu_data[PROF_TSCPU_DATA_SIZE];
	volatile char aicpu_data[PROF_AICPU_DATA_SIZE];
	volatile char aicore_data[PROF_AICORE_DATA_SIZE];
};

/* profiling command */
enum prof_cmd_type {
	PROF_SETTING = 201,
	PROF_START,
	PROF_READ,
	PROF_STOP,
	PROF_CMD_MAX
};

typedef struct char_device {
	struct class *dev_class;
	struct cdev   cdev;
	dev_t devno;
	struct device *device;
} char_device_t;


#endif /* _PROF_DRV_DEV_H_ */
