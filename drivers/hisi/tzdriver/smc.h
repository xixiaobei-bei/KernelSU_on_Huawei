/*
 * smc.h
 *
 * function declaration for sending smc cmd
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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
#ifndef _SMC_H_
#define _SMC_H_

#include <linux/of_device.h>
#include "teek_client_constants.h"
#include "teek_ns_client.h"

enum TC_NS_CMD_type {
	TC_NS_CMD_TYPE_INVALID = 0,
	TC_NS_CMD_TYPE_NS_TO_SECURE,
	TC_NS_CMD_TYPE_SECURE_TO_NS,
	TC_NS_CMD_TYPE_SECURE_TO_SECURE,
	TC_NS_CMD_TYPE_SECURE_CONFIG = 0xf,
	TC_NS_CMD_TYPE_MAX
};

#ifdef CONFIG_TEE_SMP
struct pending_entry {
	atomic_t users;
	pid_t pid;
	wait_queue_head_t wq;
	atomic_t run;
	struct list_head list;
};
#endif

#define RESLEEP_TIMEOUT 15

extern struct session_crypto_info *g_session_root_key;
extern unsigned long g_secs_suspend_status;

int sigkill_pending(struct task_struct *tsk);
int smc_init_data(struct device *class_dev);
void smc_free_data(void);
int tc_ns_smc(tc_ns_smc_cmd *cmd);
int tc_ns_smc_with_no_nr(tc_ns_smc_cmd *cmd);
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
void show_cmd_bitmap_with_lock(void);
static inline int switch_low_temperature_mode(unsigned int mode)
{
	return -EINVAL;
}
void wakeup_tc_siq(void);

#endif
