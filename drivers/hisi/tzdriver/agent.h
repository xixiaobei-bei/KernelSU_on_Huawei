

#ifndef AGENT_H
#define AGENT_H
#include <linux/fs.h>
#include <linux/hisi/rpmb.h>
#include "teek_ns_client.h"

#define AGENT_FS_ID           0x46536673 /* FSfs */
#define AGENT_MISC_ID         0x4d495343 /* MISC */
#define TEE_RPMB_AGENT_ID     0x4abe6198 /* RPMB */
#define AGENT_SOCKET_ID       0x69e85664 /* socket */
#define SECFILE_LOAD_AGENT_ID 0x4c4f4144 /* SECFILE-LOAD-AGENT */
#define TEE_SECE_AGENT_ID   0x53656345 // npu agent id
#define TEE_FACE_AGENT1_ID  0x46616365 // face agent id
#define TEE_FACE_AGENT2_ID  0x46616345 // face agent id
#define SYSTEM_UID          1000

typedef enum {
	AGENT_FALSE = -1,
	AGENT_SUCCESS = 0,
} AgentErrCode;

typedef enum {
	AGENT_ALIVE = 1,
	AGENT_DEAD = 0,
} AgentStatusCode;

struct smc_event_data *find_event_control(unsigned int agent_id);

/* for secure agent */
struct smc_event_data {
	unsigned int agent_id;
	atomic_t agent_alive;
	wait_queue_head_t wait_event_wq;
	int ret_flag;
	wait_queue_head_t send_response_wq;
	struct list_head head;
	tc_ns_smc_cmd cmd;
	tc_ns_dev_file *owner;
	tc_ns_shared_mem *buffer;
	atomic_t usage;
#ifdef CONFIG_TEE_SMP
	wait_queue_head_t ca_pending_wq;
	atomic_t ca_run;
#endif
};

struct tee_agent_kernel_ops {
	const char *agent_name;  /* MUST NOT be NULL */
	unsigned int agent_id;   /* MUST NOT be zero */
	int (*tee_agent_init)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_run)(struct tee_agent_kernel_ops *agent_instance);
	/* MUST NOT be NULL */
	int (*tee_agent_work)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_stop)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_exit)(struct tee_agent_kernel_ops *agent_instance);
	int (*tee_agent_crash_work)(
		struct tee_agent_kernel_ops *agent_instance,
		tc_ns_client_context *context,
		unsigned int dev_file_id);
	struct task_struct *agent_thread;
	void *agent_data;
	tc_ns_shared_mem *agent_buffer;
	struct list_head list;
};

static inline void get_agent_event(struct smc_event_data *event_data)
{
	if (event_data != NULL)
		atomic_inc(&event_data->usage);
}

static inline void put_agent_event(struct smc_event_data *event_data)
{
	if (event_data != NULL) {
		if (atomic_dec_and_test(&event_data->usage))
			kfree(event_data);
	}
}

void agent_init(void);
int agent_exit(void);
void send_crashed_event_response(unsigned int agent_id);
int check_ai_agent(struct task_struct *ca_task);
int agent_process_work(const tc_ns_smc_cmd *smc_cmd, unsigned int agent_id);
int is_agent_alive(unsigned int agent_id);
int tc_ns_set_native_hash(unsigned long arg, unsigned int cmd_id);
int tc_ns_late_init(unsigned long arg);
int tc_ns_register_agent(tc_ns_dev_file *dev_file, unsigned int agent_id,
	tc_ns_shared_mem *shared_mem);
int tc_ns_unregister_agent(unsigned int agent_id);
int tc_ns_unregister_agent_client(const tc_ns_dev_file *dev_file);
unsigned int tc_ns_incomplete_proceed(tc_ns_smc_cmd *smc_cmd,
	unsigned int agent_id, uint8_t flags);
int tc_ns_wait_event(unsigned int agent_id);
int tc_ns_send_event_response(unsigned int agent_id);
void tc_ns_send_event_response_all(void);
int tc_ns_sync_sys_time(const tc_ns_client_time *tc_ns_time);
int tee_agent_clear_work(tc_ns_client_context *context,
	unsigned int dev_file_id);
int tee_agent_kernel_register(struct tee_agent_kernel_ops *new_agent);
bool is_system_agent(const tc_ns_dev_file *dev_file);
void tee_agent_clear_dev_owner(const tc_ns_dev_file *dev_file);
extern struct mmc_card *get_mmc_card(struct block_device *bdev);
extern int check_ext_agent_access(struct task_struct *ca_task);
char *get_process_path(struct task_struct *task, char *tpath, int path_len);
extern int hisi_rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
	struct storage_blk_ioc_rpmb_data *storage_data);
#if defined(CONFIG_HISI_RPMB) || defined(CONFIG_HISI_MMC_SECURE_RPMB)
extern int rpmb_agent_register(void);
#endif

#endif /* AGENT_H */
