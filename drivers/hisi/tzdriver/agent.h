/*
 * agent.h
 *
 * agent manager function definition, such as register and send cmd
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
#ifndef AGENT_H
#define AGENT_H
#include <linux/fs.h>
#include "teek_ns_client.h"

#define AGENT_FS_ID           0x46536673 /* FSfs */
#define AGENT_MISC_ID         0x4d495343 /* MISC */

#ifdef CONFIG_RPMB_AGENT
#define TEE_RPMB_AGENT_ID     0x4abe6198 /* RPMB */
#endif

#define AGENT_SOCKET_ID       0x69e85664 /* socket */
#define SECFILE_LOAD_AGENT_ID 0x4c4f4144 /* SECFILE-LOAD-AGENT */
#define TEE_SECE_AGENT_ID   0x53656345 /* npu agent id */
#define TEE_FACE_AGENT1_ID  0x46616365 /* face agent id */
#define TEE_FACE_AGENT2_ID  0x46616345 /* face agent id */
#define TEE_VLTMM_AGENT_ID  0x564c544d /* vltmm agent id */
#define SYSTEM_UID          1000

enum agent_state_type {
	AGENT_CRASHED = 0,
	AGENT_REGISTERED,
	AGENT_READY,
};

enum agent_status {
	AGENT_ALIVE = 1,
	AGENT_DEAD = 0,
};

/* for secure agent */
struct smc_event_data {
	unsigned int agent_id;
	atomic_t agent_ready;
	wait_queue_head_t wait_event_wq;
	int ret_flag; /* indicate whether agent is returned from TEE */
	wait_queue_head_t send_response_wq;
	struct list_head head;
	struct tc_ns_smc_cmd cmd;
	struct tc_ns_dev_file *owner;
	pid_t pid;
	void *agent_buff_kernel;
	void *agent_buff_user; /* used for unmap */
	unsigned int agent_buff_size;
	atomic_t usage;
	wait_queue_head_t ca_pending_wq;
	/* indicate whether agent is allowed to return to TEE */
	atomic_t ca_run;
};

struct tee_agent_kernel_ops {
	const char *agent_name;
	unsigned int agent_id;
	int (*tee_agent_init)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_run)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_work)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_stop)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_exit)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_crash_work)(
		struct tee_agent_kernel_ops *agent_instance,
		struct tc_ns_client_context *context,
		unsigned int dev_file_id);
	struct task_struct *agent_thread;
	void *agent_data;
	void *agent_buff;
	unsigned int agent_buff_size;
	struct list_head list;
};

static inline void get_agent_event(struct smc_event_data *event_data)
{
	if (event_data)
		atomic_inc(&event_data->usage);
}

static inline void put_agent_event(struct smc_event_data *event_data)
{
	if (event_data) {
		if (atomic_dec_and_test(&event_data->usage))
			kfree(event_data);
	}
}

void agent_init(void);
void agent_exit(void);
struct smc_event_data *find_event_control(unsigned int agent_id);
void send_event_response(unsigned int agent_id);
int agent_process_work(const struct tc_ns_smc_cmd *smc_cmd, unsigned int agent_id);
int is_agent_alive(unsigned int agent_id);
int tc_ns_set_native_hash(unsigned long arg, unsigned int cmd_id);
int tc_ns_late_init(unsigned long arg);
int tc_ns_register_agent(struct tc_ns_dev_file *dev_file, unsigned int agent_id,
	unsigned int buffer_size, void **buffer, bool user_agent);
int tc_ns_unregister_agent(unsigned int agent_id);
void send_crashed_event_response_all(const struct tc_ns_dev_file *dev_file);
int tc_ns_wait_event(unsigned int agent_id);
int tc_ns_send_event_response(unsigned int agent_id);
void send_event_response_single(const struct tc_ns_dev_file *dev_file);
int tc_ns_sync_sys_time(const struct tc_ns_client_time *tc_ns_time);
int tee_agent_clear_work(struct tc_ns_client_context *context,
	unsigned int dev_file_id);
int tee_agent_kernel_register(struct tee_agent_kernel_ops *new_agent);
bool is_system_agent(const struct tc_ns_dev_file *dev_file);
void tee_agent_clear_dev_owner(const struct tc_ns_dev_file *dev_file);
char *get_proc_dpath(char *path, int path_len);
void clean_agent_pid_info(struct tc_ns_dev_file *dev_file);

#endif
