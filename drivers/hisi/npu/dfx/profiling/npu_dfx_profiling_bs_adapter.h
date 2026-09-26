/*
 * npu_dfx_profiling_bs_adapter.h
 *
 * about npu dfx profiling
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
#ifndef _PROF_DRV_DEV_BS_ADAPT_H_
#define _PROF_DRV_DEV_BS_ADAPT_H_

#include <linux/cdev.h>
#include "npu_user_common.h"

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

enum prof_channel {
	PROF_CHANNEL_TSCPU = 0,
	PROF_CHANNEL_AICPU,
	PROF_CHANNEL_HWTS_LOG = PROF_CHANNEL_AICPU,
	PROF_CHANNEL_AICORE,
	PROF_CHANNEL_HWTS_PROFILING = PROF_CHANNEL_AICORE,
	PROF_CHANNEL_MAX,
};

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

struct ring_buff_manager {
	volatile uint32_t read;
	uint32_t reserved0[15];  /* because cache line size  is 64 bytes */
	volatile uint32_t write;
	uint32_t reserved1[15];
};

struct prof_buff_manager {
	union {
		struct prof_setting_info info;
		char data[PROF_SETTING_INFO_SIZE];
	} cfg;
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

#endif /* _PROF_DRV_DEV_V100_H_ */
