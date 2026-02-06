/*
 * l3cache_common.h
 *
 * L3cache Common Head File
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#ifndef __L3CACHE_COMMON_H__
#define __L3CACHE_COMMON_H__

#include <asm/compiler.h>
#include <linux/hisi/l3cache_share.h>

#define DSU_PCTRL_SVC			0xc500f080
#define L3_SHARE_SVC                    0xc500f008
#define L3_DEBUG_SVC                    0xc500f001
#define L3_EXTENSION_SVC		0xc500f002

#define MAX_COUNT_VAL			0xFFFFFFFF
#define MAX_COUNT_LEN			32

enum l3cache_share_cmd {
	CMD_ENABLE_ACP = 0,
	CMD_DISABLE_ACP,
};

enum l3cache_debug_cmd {
	CMD_SET_CLSPWRDN = 0,
	CMD_SET_PART_CTRL,
	CMD_SET_SCHEME_ID,
	CMD_SET_POWER_CTRL,
	CMD_SET_FUNC_RETEN,
	CMD_GET_MISS_COUNT,
	CMD_GET_HIT_COUNT,
};

enum dsu_pctrl_cmd {
	CMD_SET_PWR_CTRL = 0,
	CMD_SET_PORTION_CTRL,
};

enum l3cache_extension_cmd {
	CMD_READ_HIT_MISS = 0,
	CMD_ENABLE_WE,
};

#ifdef CONFIG_L3CACHE_SHARE_PERF
extern struct mutex g_l3c_acp_lock;
extern bool g_req_pending_flag;
extern bool g_acp_enable_flag;
extern bool g_acp_first_flag;

void l3_cache_enable_acp(unsigned int id, unsigned int size);
void l3_cache_disable_acp(unsigned int id);
#endif

#ifdef CONFIG_L3CACHE_EXTENSION_DYNAMIC
extern struct spinlock g_l3extension_lock;
extern bool g_dpc_extension_flag;
extern bool g_extension_enable_status;
#endif

#ifdef CONFIG_LPCPU_MULTIDRV_CPUIDLE
extern bool lpcpu_fcm_cluster_pwrdn(void);
#else
static inline bool lpcpu_fcm_cluster_pwrdn(void)
{
	return 0;
}
#endif

#endif /* __L3CACHE_COMMON_H__ */
