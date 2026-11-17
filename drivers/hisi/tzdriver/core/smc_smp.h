/*
 * smc_smp.h
 *
 * function declaration for sending smc cmd
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
 */
#ifndef SMC_SMP_H
#define SMC_SMP_H

#include <linux/of_device.h>
#include "teek_client_constants.h"
#include "teek_ns_client.h"

enum tc_ns_cmd_type {
	TC_NS_CMD_TYPE_INVALID = 0,
	TC_NS_CMD_TYPE_NS_TO_SECURE,
	TC_NS_CMD_TYPE_SECURE_TO_NS,
	TC_NS_CMD_TYPE_SECURE_TO_SECURE,
	TC_NS_CMD_TYPE_SECURE_CONFIG = 0xf,
	TC_NS_CMD_TYPE_MAX
};

struct pending_entry {
	atomic_t users;
	struct task_struct *task;
#ifdef CONFIG_TA_AFFINITY
	struct cpumask ca_mask;
	struct cpumask ta_mask;
#endif
	pid_t pid;
	wait_queue_head_t wq;
	atomic_t run;
	struct list_head list;
};

#ifdef CONFIG_BIG_SESSION
#define MAX_SMC_CMD CONFIG_BIG_SESSION
#else
#ifdef CONFIG_AUTH_ENHANCE
#define MAX_SMC_CMD 18
#else
#define MAX_SMC_CMD 23
#endif
#endif

typedef uint32_t smc_buf_lock_t;

struct tc_ns_smc_queue {
	/* set when CA send cmd_in, clear after cmd_out return */
	DECLARE_BITMAP(in_bitmap, MAX_SMC_CMD);
	/* set when gtask get cmd_in, clear after cmd_out return */
	DECLARE_BITMAP(doing_bitmap, MAX_SMC_CMD);
	/* set when gtask get cmd_out, clear after cmd_out return */
	DECLARE_BITMAP(out_bitmap, MAX_SMC_CMD);
	smc_buf_lock_t smc_lock;
	uint32_t last_in;
	struct tc_ns_smc_cmd in[MAX_SMC_CMD];
	uint32_t last_out;
	struct tc_ns_smc_cmd out[MAX_SMC_CMD];
};

#define RESLEEP_TIMEOUT 15

#ifdef CONFIG_HISI_SECS_CTRL
unsigned long get_secs_suspend_status(void);
#endif

bool sigkill_pending(struct task_struct *tsk);
int smc_context_init(const struct device *class_dev);
void smc_free_data(void);
int tc_ns_smc(struct tc_ns_smc_cmd *cmd);
int tc_ns_smc_with_no_nr(struct tc_ns_smc_cmd *cmd);
int teeos_log_exception_archive(unsigned int eventid, const char *exceptioninfo);
void set_cmd_send_state(void);
int init_smc_svc_thread(void);
int smc_wakeup_ca(pid_t ca);
int smc_wakeup_broadcast(void);
int smc_shadow_exit(pid_t ca);
int smc_queue_shadow_worker(uint64_t target);
void fiq_shadow_work_func(uint64_t target);
struct pending_entry *find_pending_entry(pid_t pid);
void foreach_pending_entry(void (*func)(struct pending_entry *));
void put_pending_entry(struct pending_entry *pe);
void show_cmd_bitmap(void);
void wakeup_tc_siq(void);
struct tc_ns_smc_queue *get_cmd_data_buffer(void);

#endif
