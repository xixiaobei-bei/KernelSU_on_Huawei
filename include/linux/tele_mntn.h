/*
 * tele_mntn.h
 *
 * tele mntn common header
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
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

#ifndef __TELE_MNTN_H__
#define __TELE_MNTN_H__

#define ACORE_CPUIDLE_CUR_CPU_NUM	8
#define PERI_SLEEP_CUR_STATE_NUM	5
#define VOTE_STAT_LOCK	1 /* vote avoid sleep */
#define VOTE_STAT_UNLOCK	0 /* non-vote avoid sleep */
#define LPMCU_TICKMARK_BUF_SIZE	32

enum tele_mntn_type_id {
	TELE_MNTN_TYPE_RESERVED = -1,
	/* other, reserved */
	TELE_MNTN_NVME_LOGCAT = 0,
	TELE_MNTN_NVME_WAKEUP_ACPU,
	TELE_MNTN_DEF_STR = 5,
	TELE_MNTN_DEF_U8,
	TELE_MNTN_DEF_U16,
	TELE_MNTN_DEF_U32,
	TELE_MNTN_DEF_U64,
	TELE_MNTN_SLEEPWAKE_CCPU = 15,
	TELE_MNTN_NOTSLEEPREASON_CCPU,
	TELE_MNTN_SLEEPWAKE_MCU,
	TELE_MNTN_NOTSLEEPREASON_MCU,
	TELE_MNTN_PUPD_CCPU,
	TELE_MNTN_WFIFAIL_CCPU, /* 20 */
	TELE_MNTN_PUPD_ACPU,
	TELE_MNTN_WFIFAIL_ACPU,
	TELE_MNTN_PUPD_HIFI,
	TELE_MNTN_WFIFAIL_HIFI,
	TELE_MNTN_CPUIDLE, /* 25 */
	TELE_MNTN_VOTE_CCPU,
	TELE_MNTN_VOTE_MCU,
	TELE_MNTN_ICC_WAKE_ACPU,
	TELE_MNTN_ICC_WAKE_CCPU,
	TELE_MNTN_IPC_MCU, /* 30 */
	TELE_MNTN_DFS_DDR,
	TELE_MNTN_PLI_DDR,
	TELE_MNTN_MAXFREQ_REQ_DDR,
	TELE_MNTN_MINFREQ_REQ_DDR,
	TELE_MNTN_QOS_DDR_MCU, /* 35 */
	TELE_MNTN_QOS_DDR_CCPU,
	TELE_MNTN_QOS_DDR_ACPU,
	TELE_MNTN_DFS_CCPU,
	TELE_MNTN_PLI_CCPU,
	TELE_MNTN_QOS_CCPU, /* 40 */
	TELE_MNTN_DFS_ACPU,
	TELE_MNTN_DRX,
	TELE_MNTN_PLI_ACPU,
	TELE_MNTN_SR_ACPU,
	TELE_MNTN_DFS_GPU,
	TELE_MNTN_RESET_HIFI,
	TELE_MNTN_PUPD_IOMCU,
	TELE_MNTN_WFIFAIL_IOMCU,
	TELE_MNTN_PUPD_REGULATOR,
	TELE_MNTN_TEMPERATURE = 52,
	TELE_MNTN_LPREG_SAVE = 62,
	TELE_MNTN_LPREG_DUMP = 63,
	TELE_MNTN_TYPE_BUTT
};

struct lpmcu_tele_mntn_pu_stru_s {
	unsigned int wake_count;
	unsigned int wake_src;
};

struct lpmcu_tele_mntn_pd_stru_s {
	unsigned int sleep_count;
	unsigned int sleep_flag;
};

struct lpmcu_tele_mntn_wfifail_stru_s {
	unsigned int wfi_fail_count;
};

struct lpmcu_tele_mntn_pupd_stru_s {
	struct lpmcu_tele_mntn_pd_stru_s pd_stat;
	struct lpmcu_tele_mntn_pu_stru_s pu_stat;
	struct lpmcu_tele_mntn_wfifail_stru_s wfi_fail_stat;
	unsigned int pd_slice_time;
	unsigned int pu_slice_time;
	unsigned int wfi_fail_slice_time;
};

struct lpmcu_tele_mntn_sleepwakestat_stru_s {
	int sx;
	unsigned int sleep_count;
	unsigned int wake_count;
	unsigned int sleep_slice_time;
	unsigned int wake_slice_time;
	unsigned int wake_src0;
	unsigned int wake_src1;
};

struct lpmcu_tele_mntn_notsleepreason_stru_s {
	int can_sleep_ret;
};

struct lpmcu_tele_mntn_sleepwakeinfo_stru_s {
	struct lpmcu_tele_mntn_sleepwakestat_stru_s stat;
	struct lpmcu_tele_mntn_notsleepreason_stru_s not_sleep_reason;
};

struct lpmcu_tele_mntn_sleepwake_stru_s {
	int sx;
	struct lpmcu_tele_mntn_sleepwakeinfo_stru_s info;
};

struct lpmcu_tele_mntn_dfs_qosinfo_stru_s {
	short cmd_id;
	short qos_id;
	int req_id;
	int ret;
	unsigned int task_id; /* CCPU:task_id MCU:core_id */
	unsigned int tag_freq;
	unsigned int new_freq;
	unsigned int max_freq;
	unsigned int min_freq;
};

struct lpmcu_tele_mntn_dfs_qos_stru_s {
	struct lpmcu_tele_mntn_dfs_qosinfo_stru_s info;
	unsigned int qos_slice_time;
};

struct lpmcu_tele_mntn_dfs_policyinfo_stru_s {
	unsigned int load;
	unsigned int target_freq;
};

struct lpmcu_tele_mntn_dfs_policy_stru_s {
	struct lpmcu_tele_mntn_dfs_policyinfo_stru_s info;
	unsigned int policy_slice_time;
};

struct lpmcu_tele_mntn_dfs_targetinfo_stru_s {
	char old_freq;
	char new_freq;
	char tag_freq;
	char id;
};

struct lpmcu_tele_mntn_dfs_target_stru_s {
	struct lpmcu_tele_mntn_dfs_targetinfo_stru_s info;
	unsigned int target_slice_time;
};

struct lpmcu_tele_mntn_dfs_ddr_stru_s {
	struct lpmcu_tele_mntn_dfs_qos_stru_s qos;
	struct lpmcu_tele_mntn_dfs_policy_stru_s policy;
	struct lpmcu_tele_mntn_dfs_target_stru_s target;
};

struct lpmcu_tele_mntn_dfs_acpu_stru_s {
	struct lpmcu_tele_mntn_dfs_target_stru_s target;
};

struct lpmcu_tele_mntn_dfs_gpu_stru_s {
	struct lpmcu_tele_mntn_dfs_target_stru_s target;
};

struct lpmcu_tele_mntn_ipcrecvstatinfo_stru_s {
	unsigned char ipc_id;
	unsigned char mailbox_id;
	unsigned char obj; /* receiver */
	unsigned char mode; /* sender */
	unsigned int data0;
	unsigned int data1;
	unsigned int handle_slice;
};

struct lpmcu_tele_mntn_ipcsendstatinfo_stru_s {
	unsigned char ipc_id;
	unsigned char mailbox_id;
	unsigned char obj; /* receiver */
	unsigned char mode; /* sender */
	unsigned int msg;
};

struct lpmcu_tele_mntn_ipcsendexception_stru_s {
	unsigned char ipc;
	unsigned char mbx;
	unsigned char iclr;
	unsigned char mode;
	unsigned int data0;
	unsigned int data1;
};

struct lpmcu_tele_mntn_ipcstat_stru_s {
	struct lpmcu_tele_mntn_ipcrecvstatinfo_stru_s recv;
	struct lpmcu_tele_mntn_ipcsendstatinfo_stru_s send;
	struct lpmcu_tele_mntn_ipcsendexception_stru_s exp;
	unsigned int recv_slice_time;
	unsigned int send_slice_time;
};

struct lpmcu_tele_mntn_tickmark_stru_s {
	unsigned int tick[LPMCU_TICKMARK_BUF_SIZE];
};

struct lpmcu_tele_mntn_ncore_pu_stru_s {
	unsigned int state;
	unsigned int up_cnt;
	unsigned short wake_irq;
	unsigned char wake_mbx_id;
	unsigned char wake_mbx_mode;
	unsigned int wake_mbx_data0;
};

struct lpmcu_tele_mntn_ncore_pd_stru_s {
	unsigned int state;
	unsigned int dn_cnt;
};

/* hifi/iomcu etc */
struct lpmcu_tele_mntn_ncore_pupd_stru_s {
	struct lpmcu_tele_mntn_ncore_pu_stru_s pu;
	struct lpmcu_tele_mntn_ncore_pd_stru_s pd;
};

struct lpmcu_tele_mntn_mbx_exp_state_stru_s {
	unsigned char source;
	unsigned char dstatus;
	unsigned char mode;
	unsigned char iclr;
	unsigned int data0;
	unsigned int data1;
};

struct lpmcu_tele_mntn_iomcu_reset_stru_s {
	struct lpmcu_tele_mntn_mbx_exp_state_stru_s mailbox;
	/* add vote peri\sys_state state */
};

struct lpmcu_tele_mntn_votestat_stru_s {
	/* VOTE_STAT_LOCK or VOTE_STAT_UNLOCK */
	unsigned char stat;
	unsigned char client_id;
	unsigned char state_id;
	unsigned char reserved;
	unsigned int vote_map;
};

struct lpmcu_tele_mntn_vote_stru_s {
	struct lpmcu_tele_mntn_votestat_stru_s vote_stat;
	unsigned int slice_time;
};

struct lpmcu_tele_mntn_temperature_info_stru_s {
	unsigned short vol;
	short tem;
	unsigned int chan;
};

struct lpmcu_tele_mntn_temperature_stru_s {
	struct lpmcu_tele_mntn_temperature_info_stru_s info;
	unsigned int tem_slice_time;
};

struct lpmcu_tele_mntn_stru_s {
	struct lpmcu_tele_mntn_tickmark_stru_s tick_mask;
	struct lpmcu_tele_mntn_pupd_stru_s ccore;
	struct lpmcu_tele_mntn_pupd_stru_s acore;
	struct lpmcu_tele_mntn_sleepwake_stru_s mcu;
	struct lpmcu_tele_mntn_dfs_ddr_stru_s dfs_ddr;
	struct lpmcu_tele_mntn_dfs_acpu_stru_s dfs_acpu;
	struct lpmcu_tele_mntn_dfs_gpu_stru_s dfs_gpu;
	struct lpmcu_tele_mntn_ipcstat_stru_s ipc;
	struct lpmcu_tele_mntn_ncore_pupd_stru_s hifi;
	struct lpmcu_tele_mntn_ncore_pupd_stru_s iomcu;
	struct lpmcu_tele_mntn_vote_stru_s vote[PERI_SLEEP_CUR_STATE_NUM];
	struct lpmcu_tele_mntn_temperature_stru_s tem;
	struct lpmcu_tele_mntn_iomcu_reset_stru_s iomcu_rst;
	/* add here */
};

struct acore_tele_mntn_dfs_acpu_boostinfo_stru_s {
	unsigned int cpu;
	unsigned int tag_freq;
	unsigned int new_freq;
	unsigned int min_freq;
	unsigned int max_freq;
};

struct acore_tele_mntn_dfs_acpu_policyinfo_stru_s {
	unsigned int cpu;
	unsigned int tag_freq;
	unsigned int new_freq;
	unsigned int min_freq;
	unsigned int max_freq;
	unsigned char load[ACORE_CPUIDLE_CUR_CPU_NUM];
};

struct acore_tele_mntn_dfs_acpu_boost_stru_s {
	struct acore_tele_mntn_dfs_acpu_boostinfo_stru_s info;
	unsigned int boost_slice_time;
};

struct acore_tele_mntn_dfs_acpu_policy_stru_s {
	struct acore_tele_mntn_dfs_acpu_policyinfo_stru_s info;
	unsigned int policy_slice_time;
};

struct acore_tele_mntn_dfs_acpu_stru_s {
	struct acore_tele_mntn_dfs_acpu_boost_stru_s boost;
	struct acore_tele_mntn_dfs_acpu_policy_stru_s policy;
};

struct acore_tele_mntn_dfs_ddr_qosinfo_stru_s {
	short cmd_id;
	short qos_id;
	unsigned short pid;
	unsigned short ppid;
	int tag_val;
	unsigned int new_freq;
	unsigned int min_freq;
	unsigned int max_freq;
};

struct pwc_tele_mntn_dfs_ddr_qos_stru_s {
	struct acore_tele_mntn_dfs_ddr_qosinfo_stru_s info;
	unsigned int qos_slice_time;
};

struct acore_tele_mntn_dfs_ddr_stru_s {
	struct pwc_tele_mntn_dfs_ddr_qos_stru_s qos;
};

struct acore_tele_mntn_stru_s {
	struct acore_tele_mntn_dfs_ddr_stru_s dfs_ddr;
	struct acore_tele_mntn_dfs_acpu_stru_s dfs_acpu;
};

struct acore_tele_mntn_stru_s *acore_tele_mntn_get(void);
unsigned int get_slice_time(void);
int tele_mntn_write_log(enum tele_mntn_type_id type_id, unsigned int len,
			const void *data);
#endif
