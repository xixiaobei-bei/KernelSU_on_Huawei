/*
 * dubai_lp_stats.h
 *
 * lpm or ddr lowpower stats header file
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef __DUBAI_VOTE_TIME__
#define __DUBAI_VOTE_TIME__

#include <linux/types.h>
#include <linux/hisi/hisi_rproc.h>
#include <ddr_define.h>

#define VOTE_IP_NAME_SIZE         64

#define OK      0
#define DDR_VOTE_MSG_LEN     2
#define DDR_VOTE_ACK_LEN     8
#ifndef MAX_DDR_FREQ_NUM
#define MAX_DDR_FREQ_NUM        LPMCU_FREQ_NUM_MAX
#endif

#pragma pack(1)
struct lpm3_vote {
	char name[VOTE_IP_NAME_SIZE];
	int64_t duration;
};

struct ddr_vote {
	int32_t freq;
	char name[VOTE_IP_NAME_SIZE];
	int64_t duratiton;
};

struct ddr_vote_ip {
	int32_t logic_id;
	char name[VOTE_IP_NAME_SIZE];
};

struct ddr_state {
	int32_t freq;
	char name[VOTE_IP_NAME_SIZE];
	int64_t value;
};

struct ddr_state_ip {
	int32_t logic_id;
	char name[VOTE_IP_NAME_SIZE];
};
#pragma pack()

int ioctrl_get_lpmcu_vote_num(void __user *argp);
int ioctrl_lpmcu_vote_data(void __user *argp);
int ioctrl_get_ddr_vote_num(void __user *argp);
int ioctrl_ddr_vote_data(void __user *argp);
int ioctrl_clr_ddr_vote_data(void __user *argp);

#ifdef CONFIG_LOWPM_DDR_STATE
int ioctrl_get_ddr_state_num(void __user *argp);
int ioctrl_ddr_state_data(void __user *argp);
#else
static inline int ioctrl_get_ddr_state_num(void __user *argp __attribute((unused)))
{
	return 0;
}

static inline int ioctrl_ddr_state_data(void __user *argp __attribute((unused)))
{
	return 0;
}
#endif
#endif
