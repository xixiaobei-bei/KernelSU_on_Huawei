
#include "agent.h"
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/version.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/signal.h>
#include <securec.h>
#include "teek_client_constants.h"
#include "teek_ns_client.h"
#include "smc.h"
#include "mem.h"
#include "tui.h"
#include "tc_ns_log.h"
#include "mailbox_mempool.h"
#include "tc_client_sub_driver.h"
#include "cmdmonitor.h"

#define HASH_FILE_MAX_SIZE         8192
#define AGENT_BUFF_SIZE            (4 * 1024)
#define AGENT_MAX                  32
#define MAX_PATH_SIZE              512
#define PAGE_ORDER_RATIO           2

static struct list_head g_tee_agent_list;

struct agent_control {
	spinlock_t lock;
	struct list_head agent_list;
};
static struct agent_control g_agent_control;

typedef struct tag_ca_info {
	char path[MAX_PATH_SIZE];
	uint32_t uid;
	uint32_t agent_id;
} ca_info;

static ca_info g_allowed_ext_agent_ca[] = {
	{
		"/vendor/bin/hiaiserver",
		1000,
		TEE_SECE_AGENT_ID,
	},
	{
		"/vendor/bin/hw/vendor.huawei.hardware.biometrics.hwfacerecognize@1.1-service",
		1000,
		TEE_FACE_AGENT1_ID,
	},
	{
		"/vendor/bin/hw/vendor.huawei.hardware.biometrics.hwfacerecognize@1.1-service",
		1000,
		TEE_FACE_AGENT2_ID,
	},

	/* just for test in ENG version */
};

static int is_allowed_agent_ca(const ca_info *ca, bool check_agent_id_flag)
{
	uint32_t i;
	bool tmp_check_status = false;
	ca_info *tmp_ca = g_allowed_ext_agent_ca;

	if (!check_agent_id_flag) {
		for (i = 0;
			i < sizeof(g_allowed_ext_agent_ca) /
			sizeof(g_allowed_ext_agent_ca[0]);
			i++) {
			if ((memcmp(ca->path, tmp_ca->path, MAX_PATH_SIZE) == EOK) &&
				(ca->uid == tmp_ca->uid))
				return AGENT_SUCCESS;
			tmp_ca++;
		}
	} else {
		for (i = 0;
			i < sizeof(g_allowed_ext_agent_ca) /
			sizeof(g_allowed_ext_agent_ca[0]); i++) {
			tmp_check_status =
				((memcmp(ca->path, tmp_ca->path,
				MAX_PATH_SIZE) == EOK) &&
				(ca->uid == tmp_ca->uid) &&
				(ca->agent_id == tmp_ca->agent_id));
			if (tmp_check_status)
				return AGENT_SUCCESS;
			tmp_ca++;
		}
	}
	tlogd("ca-uid is %u, ca_path is %s, agent id is %x\n", ca->uid,
		ca->path, ca->agent_id);
	return AGENT_FALSE;
}

static int get_ca_path_and_uid(struct task_struct *ca_task, ca_info *ca)
{
	char *path = NULL;
	const struct cred *cred = NULL;
	int message_size;
	int ret = -1;
	char *tpath = NULL;

	tpath = kmalloc(MAX_PATH_SIZE, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)tpath)) {
		tloge("tpath kmalloc fail\n");
		return -EPERM;
	}

	path = get_process_path(ca_task, tpath, MAX_PATH_SIZE);
	if (IS_ERR_OR_NULL(path)) {
		ret = -ENOMEM;
		tloge("get_process_path failed\n");
		goto end;
	}

	message_size = snprintf_s(ca->path, MAX_PATH_SIZE - 1,
		MAX_PATH_SIZE - 1, "%s", path);
	get_task_struct(ca_task);
	cred = get_task_cred(ca_task);
	if (cred == NULL) {
		tloge("cred is NULL\n");
		kfree(tpath);
		tpath = NULL;
		put_task_struct(ca_task);
		return -EPERM;
	}
	ca->uid = cred->uid.val;
	tlogd("ca_task->comm is %s, path is %s, ca uid is %u\n",
	      ca_task->comm, path, cred->uid.val);

	if (message_size > 0)
		ret = 0;
	put_cred(cred);
	cred = NULL;
	put_task_struct(ca_task);

end:
	kfree(tpath);
	tpath = NULL;
	return ret;
}

int check_ai_agent(struct task_struct *ca_task)
{
	int ret;
	ca_info agent_ca = { {0}, 0 };

	if (ca_task == NULL) {
		tloge("ca_task is NULL.\n");
		return -EPERM;
	}

	ret = get_ca_path_and_uid(ca_task, &agent_ca);
	if (ret) {
		tloge("get cp path or uid failed.\n");
		return ret;
	}

	if ((memcmp(agent_ca.path, g_allowed_ext_agent_ca[0].path,
		MAX_PATH_SIZE) == EOK) &&
		(agent_ca.uid == g_allowed_ext_agent_ca[0].uid)) {
		tloge("ai agent has crashed\n");
		return AGENT_SUCCESS;
	}
	return AGENT_FALSE;
}

int check_ext_agent_access(struct task_struct *ca_task)
{
	int ret;
	ca_info agent_ca = { {0}, 0 };

	if (ca_task == NULL) {
		tloge("ca_task is NULL.\n");
		return -EPERM;
	}

	ret = get_ca_path_and_uid(ca_task, &agent_ca);
	if (ret) {
		tloge("get cp path or uid failed.\n");
		return ret;
	}

	ret = is_allowed_agent_ca(&agent_ca, 0);
	return ret;
}

int check_ext_agent_access_with_agent_id(struct task_struct *ca_task,
	uint32_t agent_id)
{
	int ret;
	ca_info agent_ca = {"", 0, 0};

	if (ca_task == NULL) {
		tloge("ca_task is NULL\n");
		return -EPERM;
	}

	ret = get_ca_path_and_uid(ca_task, &agent_ca);
	if (ret) {
		tloge("get cp path or uid failed\n");
		return ret;
	}
	agent_ca.agent_id = agent_id;
	ret = is_allowed_agent_ca(&agent_ca, 1);
	return ret;
}

int tc_ns_set_native_hash(unsigned long arg, unsigned int cmd_id)
{
	int ret;
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	uint8_t *inbuf = (uint8_t *)(uintptr_t)arg;
	uint32_t buf_len = 0;
	uint8_t *buf_to_tee = NULL;
	struct mb_cmd_pack *mb_pack = NULL;

	if (inbuf == NULL)
		return AGENT_FALSE;
	if (tc_ns_get_uid() != 0) {
		tloge("It is a fake tee agent\n");
		return -EACCES;
	}
	if (copy_from_user(&buf_len, inbuf, sizeof(buf_len))) {
		tloge("copy from user failed\n");
		return -EFAULT;
	}
	if (buf_len > HASH_FILE_MAX_SIZE) {
		tloge("ERROR: file size[0x%x] too big\n", buf_len);
		return AGENT_FALSE;
	}
	buf_to_tee = mailbox_alloc(buf_len, 0);
	if (buf_to_tee == NULL) {
		tloge("failed to alloc memory!\n");
		return AGENT_FALSE;
	}
	if (copy_from_user(buf_to_tee, inbuf, buf_len)) {
		tloge("copy from user failed\n");
		mailbox_free(buf_to_tee);
		return -EFAULT;
	}
	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("alloc cmd pack failed\n");
		mailbox_free(buf_to_tee);
		return -ENOMEM;
	}
	mb_pack->operation.paramtypes = TEE_PARAM_TYPE_VALUE_INPUT |
		(TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM);
	mb_pack->operation.params[TEE_PARAM_ONE].value.a =
		(unsigned int)virt_to_phys(buf_to_tee);
	mb_pack->operation.params[TEE_PARAM_ONE].value.b =
		(unsigned int)(virt_to_phys(buf_to_tee) >> ADDR_TRANS_NUM);
	mb_pack->operation.params[TEE_PARAM_TWO].value.a = buf_len;
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = cmd_id;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >>
		ADDR_TRANS_NUM;
	ret = tc_ns_smc(&smc_cmd);
	mailbox_free(buf_to_tee);
	mailbox_free(mb_pack);
	buf_to_tee = NULL;
	mb_pack = NULL;
	return ret;
}

int tc_ns_late_init(unsigned long arg)
{
	int ret;
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	uint32_t index = (uint32_t)arg; // index is uint32, no truncate risk
	struct mb_cmd_pack *mb_pack = NULL;

	if (tc_ns_get_uid() != 0) {
		tloge("It is a fake tee agent\n");
		return -EACCES;
	}

	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("alloc cmd pack failed\n");
		return -ENOMEM;
	}

	mb_pack->operation.paramtypes = TEE_PARAM_TYPE_VALUE_INPUT;
	mb_pack->operation.params[TEE_PARAM_ONE].value.a = index;

	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_LATE_INIT;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >>
		ADDR_TRANS_NUM;

	ret = tc_ns_smc(&smc_cmd);

	mailbox_free(mb_pack);
	mb_pack = NULL;

	return ret;
}

void tc_ns_send_event_response_all(void)
{
	send_crashed_event_response(AGENT_FS_ID);
	send_crashed_event_response(SECFILE_LOAD_AGENT_ID);
	send_crashed_event_response(AGENT_MISC_ID);
	send_crashed_event_response(AGENT_SOCKET_ID);
}

struct smc_event_data *find_event_control(unsigned int agent_id)
{
	struct smc_event_data *event_data = NULL;
	struct smc_event_data *tmp_data = NULL;
	unsigned long flags;

	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_for_each_entry(event_data, &g_agent_control.agent_list, head) {
		if (event_data->agent_id == agent_id) {
			tmp_data = event_data;
			get_agent_event(event_data);
			break;
		}
	}
	spin_unlock_irqrestore(&g_agent_control.lock, flags);

	return tmp_data;
}

static void release_shared_mem_by_addr(tc_ns_dev_file *dev_file,
	const void *kernel_addr)
{
	tc_ns_shared_mem *shared_mem = NULL;
	tc_ns_shared_mem *shared_mem_temp = NULL;

	mutex_lock(&dev_file->shared_mem_lock);
	list_for_each_entry_safe(shared_mem, shared_mem_temp,
		&dev_file->shared_mem_list, head) {
		if (shared_mem != NULL &&
			shared_mem->kernel_addr == kernel_addr) {
			if (atomic_read(&shared_mem->usage) == 1)
				list_del(&shared_mem->head);
			break;
		}
	}
	mutex_unlock(&dev_file->shared_mem_lock);
}

static void free_event_control(unsigned int agent_id)
{
	struct smc_event_data *event_data = NULL;
	struct smc_event_data *tmp_event = NULL;
	unsigned long flags;
	bool check_status = false;

	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_for_each_entry_safe(event_data, tmp_event,
		&g_agent_control.agent_list, head) {
		if (event_data->agent_id == agent_id) {
			list_del(&event_data->head);
			break;
		}
	}
	spin_unlock_irqrestore(&g_agent_control.lock, flags);

	check_status = (event_data != NULL) && (event_data->buffer != NULL) &&
		(event_data->owner != NULL);
	if (check_status == true)
		release_shared_mem_by_addr(event_data->owner,
			event_data->buffer->kernel_addr);
	put_sharemem_struct(event_data->buffer);
	put_agent_event(event_data);

}

static void wait_agent_response(struct smc_event_data *event_data)
{
	/* only userspace CA need freeze */
	bool need_freeze = !(current->flags & PF_KTHREAD);
	bool sig_pending = !sigisemptyset(&current->pending.signal);
	bool answered = true;

	do {
		answered = true;
		/*
		 * wait_event_freezable will be interrupted by signal and freezer
		 which is called to free a userspace task in suspend. Freezing
		 * a task means wakeup a task by fake_signal_wake_up and let it
		 * have an opportunity to enter into 'refrigerator' by try_to_freeze
		 * used in wait_event_freezable.
		 *
		 * What scenes can be freezed ?
		 * 1. CA is waiting agent -> suspend -- OK
		 * 2. suspend -> CA start agent request -- OK
		 * 3. CA is waiting agent -> CA is killed -> suspend  -- NOK
		 */
		if (need_freeze && !sig_pending) {
			int r = wait_event_freezable(event_data->ca_pending_wq,
				atomic_read(&event_data->ca_run));
			if (r == -ERESTARTSYS) {
				if (!sigisemptyset(&current->pending.signal))
					sig_pending = true;
				tloge("agent wait event is interrupted by %s\n",
					sig_pending ? "signal" : "freezer");
				/*
				 * When freezing a userspace task, fake_signal_wake_up only set
				 * TIF_SIGPENDING but not set a real signal. After task thawed,
				 * CA need wait agent response again, so TIF_SIGPENDING need to
				 * be cleared.
				 */
				if (!sig_pending)
					clear_thread_flag(TIF_SIGPENDING);
				answered = false;
			}
		} else {
			wait_event(event_data->ca_pending_wq, atomic_read(&event_data->ca_run));
		}
	} while (!answered);
}

int agent_process_work(const tc_ns_smc_cmd *smc_cmd, unsigned int agent_id)
{
	struct smc_event_data *event_data = NULL;
	int ret = TEEC_SUCCESS;

	if (smc_cmd == NULL) {
		tloge("smc_cmd is null\n");
		return TEEC_ERROR_GENERIC;
	}
	tlogd("agent_id=0x%x\n", agent_id);
	event_data = find_event_control(agent_id);
	if (event_data == NULL) {
		tloge("agent %u not exist\n", agent_id);
		return TEEC_ERROR_GENERIC;
	}

	tlogd("agent_process_work: returning client command");

	/* store tui working device for terminate tui when device is closed. */
	if (agent_id == TEE_TUI_AGENT_ID) {
		tloge("TEE_TUI_AGENT_ID: pid-%d", current->pid);
		set_tui_caller_info(smc_cmd->dev_file_id, current->pid);
	}

#ifndef CONFIG_TEE_SMP
	/* Keep a copy of the SMC cmd to return to TEE when the work is done */
	if (memcpy_s(&event_data->cmd, sizeof(event_data->cmd), smc_cmd,
		sizeof(*smc_cmd))) {
		tloge("failed to memcpy_s smc_cmd\n");
		put_agent_event(event_data);
		return TEEC_ERROR_GENERIC;
	}
	isb();
	wmb();
#endif
	event_data->ret_flag = 1;
	/* Wake up the agent that will process the command */
	tlogd("agent_process_work: wakeup the agent");
	wake_up(&event_data->wait_event_wq);
#ifdef CONFIG_TEE_SMP
	tlogd("agent 0x%x request, goto sleep, pe->run=%d\n",
	      agent_id, atomic_read(&event_data->ca_run));
	if (atomic_read(&event_data->agent_alive) == 0) {
		tloge("agent %u is killed and restarting\n", agent_id);
		event_data->ret_flag = 0;
		put_agent_event(event_data);
		return TEEC_ERROR_GENERIC;
	}
	wait_agent_response(event_data);
	atomic_set(&event_data->ca_run, 0);
#endif
	put_agent_event(event_data);
	/* when agent work is done, reset cmd monitor time cause it's a new smc cmd */
	cmd_monitor_reset_send_time();
	return ret;
}

int is_agent_alive(unsigned int agent_id)
{
	struct smc_event_data *event_data = NULL;

	event_data = find_event_control(agent_id);
	if (event_data != NULL) {
		put_agent_event(event_data);
		return AGENT_ALIVE;
	} else {
		return AGENT_DEAD;
	}
}

int tc_ns_wait_event(unsigned int agent_id)
{
	int ret = -EINVAL;
	struct smc_event_data *event_data = NULL;

	if ((tc_ns_get_uid() != 0) &&
	    check_ext_agent_access_with_agent_id(current, agent_id)) {
		tloge("It is a fake tee agent\n");
		return -EACCES;
	}
	tlogd("agent %u waits for command\n", agent_id);
	event_data = find_event_control(agent_id);
	if (event_data != NULL) {
		/* wait event will return either 0 or -ERESTARTSYS so just
		 * return it further to the ioctl handler
		 */
		ret = wait_event_interruptible(event_data->wait_event_wq,
			event_data->ret_flag);
		put_agent_event(event_data);
	}

	return ret;
}

int tc_ns_sync_sys_time(const tc_ns_client_time *tc_ns_time)
{
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret;
	tc_ns_client_time tmp_tc_ns_time = {0};
	struct mb_cmd_pack *mb_pack = NULL;

	if (tc_ns_time == NULL) {
		tloge("tc_ns_time is NULL input buffer\n");
		return -EINVAL;
	}
	if (tc_ns_get_uid() != 0) {
		tloge("It is a fake tee agent\n");
		return TEEC_ERROR_GENERIC;
	}
	if (copy_from_user(&tmp_tc_ns_time, tc_ns_time,
		sizeof(tmp_tc_ns_time))) {
		tloge("copy from user failed\n");
		return -EFAULT;
	}

	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("alloc mb pack failed\n");
		return -ENOMEM;
	}

	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_ADJUST_TIME;
	smc_cmd.err_origin = tmp_tc_ns_time.seconds;
	smc_cmd.ret_val = (int)tmp_tc_ns_time.millis;

	ret = tc_ns_smc(&smc_cmd);
	if (ret)
		tloge("tee adjust time failed, return error %x\n", ret);

	mailbox_free(mb_pack);
	mb_pack = NULL;

	return ret;
}

static struct smc_event_data *check_for_send_event_response(
	unsigned int agent_id)
{
	struct smc_event_data *event_data = find_event_control(agent_id);
	bool tmp_check_status = false;

	if (event_data == NULL) {
		tloge("Can't get event_data\n");
		return NULL;
	}
	tmp_check_status = ((tc_ns_get_uid() != 0) &&
		check_ext_agent_access_with_agent_id(current, agent_id));
	if (tmp_check_status) {
		tloge("It is a fake tee agent\n");
		put_agent_event(event_data);
		return NULL;
	}
	return event_data;
}

static int process_send_event_response(struct smc_event_data *event_data)
{
	int ret = 0;
	if (event_data->ret_flag) {
		event_data->ret_flag = 0;
		/* Send the command back to the TA session waiting for it */
#ifdef CONFIG_TEE_SMP
		tlogd("agent wakeup ca\n");
		atomic_set(&event_data->ca_run, 1);
		/* make sure reset working_ca before wakeup CA */
		wake_up(&event_data->ca_pending_wq);
		ret = 0;
#else
		ret = tc_ns_post_smc(&event_data->cmd);
#endif
	}
	return ret;
}

int tc_ns_send_event_response(unsigned int agent_id)
{
	struct smc_event_data *event_data = NULL;
	int ret;

	event_data = check_for_send_event_response(agent_id);
	if (event_data == NULL) {
		tlogd("agent %u pre-check failed\n", agent_id);
		return -1;
	}
	tlogd("agent %u sends answer back\n", agent_id);
	ret = process_send_event_response(event_data);
	put_agent_event(event_data);
	return ret;
}

void send_crashed_event_response(unsigned int agent_id)
{
	struct smc_event_data *event_data = find_event_control(agent_id);
	int ret;

	if (event_data == NULL) {
		tloge("Can't get event_data\n");
		return;
	}
	tloge("agent %u crashed and sends answer back\n", agent_id);
	atomic_set(&event_data->agent_alive, 0);
	ret = process_send_event_response(event_data);
	put_agent_event(event_data);
	if (ret)
		tloge("agent 0x%x send_event_response failed\n", agent_id);
	return;
}

static void init_event_data_for_restart(tc_ns_dev_file *dev_file,
	struct smc_event_data *event_data)
{
	event_data->ret_flag = 0;
	event_data->owner = dev_file;
	atomic_set(&(event_data->agent_alive), 1);
	init_waitqueue_head(&(event_data->wait_event_wq));
	init_waitqueue_head(&(event_data->send_response_wq));
#ifdef CONFIG_TEE_SMP
	init_waitqueue_head(&(event_data->ca_pending_wq));
	atomic_set(&(event_data->ca_run), 0);
#endif
	return;

}

static int alloc_and_init_event_data(tc_ns_dev_file *dev_file,
	struct smc_event_data **event_data, unsigned int agent_id,
	tc_ns_shared_mem *shared_mem)
{

	*event_data = kzalloc(sizeof(**event_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*event_data)))
		return -ENOMEM;
	(*event_data)->agent_id = agent_id;
	(*event_data)->ret_flag = 0;
	(*event_data)->buffer = shared_mem;
	(*event_data)->owner = dev_file;
	atomic_set(&(*event_data)->agent_alive, 1);
	init_waitqueue_head(&(*event_data)->wait_event_wq);
	init_waitqueue_head(&(*event_data)->send_response_wq);
	INIT_LIST_HEAD(&(*event_data)->head);
#ifdef CONFIG_TEE_SMP
	init_waitqueue_head(&(*event_data)->ca_pending_wq);
	atomic_set(&(*event_data)->ca_run, 0);
#endif
	return TEEC_SUCCESS;
}

static void init_for_smc_call(tc_ns_shared_mem *shared_mem,
	struct mb_cmd_pack *mb_pack, unsigned int agent_id,
	tc_ns_smc_cmd *smc_cmd)
{
	mb_pack->operation.paramtypes = TEE_PARAM_TYPE_VALUE_INPUT |
		(TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM);
	mb_pack->operation.params[TEE_PARAM_ONE].value.a =
		virt_to_phys(shared_mem->kernel_addr);
	mb_pack->operation.params[TEE_PARAM_ONE].value.b =
		virt_to_phys(shared_mem->kernel_addr) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[TEE_PARAM_TWO].value.a = shared_mem->len;
	smc_cmd->global_cmd = true;
	smc_cmd->cmd_id = GLOBAL_CMD_ID_REGISTER_AGENT;
	smc_cmd->operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd->operation_h_phys = virt_to_phys(&mb_pack->operation) >>
		ADDR_TRANS_NUM;
	smc_cmd->agent_id = agent_id;
}

static bool is_built_in_agent(unsigned int agent_id)
{
	bool check_value = false;

	check_value = ((agent_id == AGENT_FS_ID) ||
		(agent_id == AGENT_MISC_ID) ||
		(agent_id == AGENT_SOCKET_ID) ||
		(agent_id == SECFILE_LOAD_AGENT_ID));
	return check_value;
}

int tc_ns_register_agent(tc_ns_dev_file *dev_file, unsigned int agent_id,
	tc_ns_shared_mem *shared_mem)
{
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	struct smc_event_data *event_data = NULL;
	int ret = TEEC_ERROR_GENERIC;
	int find_flag = 0;
	unsigned long flags;
	struct mb_cmd_pack *mb_pack = NULL;
	bool check_value = ((tc_ns_get_uid() != 0) &&
		check_ext_agent_access_with_agent_id(current, agent_id));

	if (check_value) {
		tloge("It is a fake tee agent\n");
		goto error;
	}
	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_for_each_entry(event_data, &g_agent_control.agent_list, head) {
		if (event_data->agent_id == agent_id) {
			find_flag = 1;
			break;
		}
	}
	spin_unlock_irqrestore(&g_agent_control.lock, flags);
	if (find_flag) {
		check_value = (is_built_in_agent(agent_id) ||
			agent_id == TEE_SECE_AGENT_ID);
		if (check_value) {
			/* this is for agent restart because agent's event data
			 * node will not be released when agent crashed.
			 */
			tlogd("init_event_data_for_restart : 0x%x\n", agent_id);
			init_event_data_for_restart(dev_file, event_data);
		}
		return TEEC_SUCCESS;
	}
	if (shared_mem == NULL) {
		tloge("shared mem is not exist\n");
		goto error;
	}
	/* Obtain share memory which is released in tc_ns_unregister_agent */
	get_sharemem_struct(shared_mem);
	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("alloc mailbox failed\n");
		put_sharemem_struct(shared_mem);
		goto error;
	}
	init_for_smc_call(shared_mem, mb_pack, agent_id, &smc_cmd);
	ret = tc_ns_smc(&smc_cmd);
	if (ret != TEEC_SUCCESS) {
		/* release share mem when sending smc failure. */
		put_sharemem_struct(shared_mem);
		goto error;
	}
	ret = alloc_and_init_event_data(dev_file, &event_data,
		agent_id, shared_mem);
	if (ret != TEEC_SUCCESS) {
		put_sharemem_struct(shared_mem);
		goto error;
	}
	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_add_tail(&event_data->head, &g_agent_control.agent_list);
	atomic_set(&event_data->usage, 1);
	spin_unlock_irqrestore(&g_agent_control.lock, flags);
error:
	if (mb_pack != NULL) {
		mailbox_free(mb_pack);
		mb_pack = NULL;
	}
	return ret;
}

static int check_for_unregister_agent(unsigned int agent_id)
{
	bool check_value = false;

	check_value = ((tc_ns_get_uid() != 0) &&
		(tc_ns_get_uid() != SYSTEM_UID));
	if (check_value) {
		tloge("It is a fake tee agent\n");
		return TEEC_ERROR_GENERIC;
	}

	check_value = (is_built_in_agent(agent_id) ||
		agent_id == TEE_TUI_AGENT_ID ||
		agent_id == TEE_RPMB_AGENT_ID);
	if (check_value) {
		tloge("agent is not allowed to unregister agent_id=0x%x\n",
			agent_id);
		return TEEC_ERROR_GENERIC;
	}
	return TEEC_SUCCESS;
}

int tc_ns_unregister_agent(unsigned int agent_id)
{
	struct smc_event_data *event_data = NULL;
	int ret;
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	struct mb_cmd_pack *mb_pack = NULL;

	if (check_for_unregister_agent(agent_id) != TEEC_SUCCESS)
		return TEEC_ERROR_GENERIC;

	if (agent_id == TEE_SECE_AGENT_ID) {
		send_crashed_event_response(agent_id);
		return TEEC_ERROR_GENERIC;
	}

	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("alloc mailbox failed\n");
		return TEEC_ERROR_GENERIC;
	}
	event_data = find_event_control(agent_id);
	if (event_data == NULL || event_data->buffer == NULL ||
	    event_data->buffer->kernel_addr == NULL) {
		mailbox_free(mb_pack);
		tloge("agent is not found or kernel_addr is not allocated\n");
		put_agent_event(event_data);
		return TEEC_ERROR_GENERIC;
	}
	mb_pack->operation.paramtypes = TEE_PARAM_TYPE_VALUE_INPUT |
		(TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM);
	mb_pack->operation.params[TEE_PARAM_ONE].value.a =
		virt_to_phys(event_data->buffer->kernel_addr);
	mb_pack->operation.params[TEE_PARAM_ONE].value.b =
		virt_to_phys(event_data->buffer->kernel_addr) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[TEE_PARAM_TWO].value.a = SZ_4K;
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_UNREGISTER_AGENT;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >>
		ADDR_TRANS_NUM;
	smc_cmd.agent_id = agent_id;
	tlogd("Unregistering agent %u\n", agent_id);
	ret = tc_ns_smc(&smc_cmd);
	if (ret == TEEC_SUCCESS)
		free_event_control(agent_id);
	put_agent_event(event_data);
	mailbox_free(mb_pack);
	mb_pack = NULL;
	return ret;
}

bool is_system_agent(const tc_ns_dev_file *dev_file)
{
	struct smc_event_data *event_data = NULL;
	struct smc_event_data *tmp = NULL;
	bool system_agent = false;
	unsigned long flags;

	if (dev_file == NULL)
		return system_agent;

	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_for_each_entry_safe(event_data, tmp, &g_agent_control.agent_list,
		head) {
		if (event_data->owner == dev_file) {
			system_agent = true;
			break;
		}
	}
	spin_unlock_irqrestore(&g_agent_control.lock, flags);

	return system_agent;
}

int tc_ns_unregister_agent_client(const tc_ns_dev_file *dev_file)
{
	struct smc_event_data *event_data = NULL;
	struct smc_event_data *tmp = NULL;
	unsigned int agent_id[AGENT_MAX] = {0};
	unsigned int i = 0;
	unsigned int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_for_each_entry_safe(event_data, tmp, &g_agent_control.agent_list,
		head) {
		if ((event_data->owner == dev_file) && (i < AGENT_MAX))
			agent_id[i++] = event_data->agent_id;
	}
	spin_unlock_irqrestore(&g_agent_control.lock, flags);

	for (i = 0; i < AGENT_MAX; i++) {
		if (agent_id[i]) {
			if (tc_ns_unregister_agent(agent_id[i])) {
				tloge("TC_NS_unregister_agent[%d] failed\n",
					agent_id[i]);
				ret |= 1;
			}
		}
	}

	return ret;
}

void tee_agent_clear_dev_owner(const tc_ns_dev_file *dev_file)
{
	struct smc_event_data *event_data = NULL;
	struct smc_event_data *tmp = NULL;
	unsigned long flags;

	spin_lock_irqsave(&g_agent_control.lock, flags);
	list_for_each_entry_safe(event_data, tmp, &g_agent_control.agent_list,
		head) {
		if (event_data->owner == dev_file) {
			event_data->owner = NULL;
			break;
		}
	}
	spin_unlock_irqrestore(&g_agent_control.lock, flags);
	return;
}


static int def_tee_agent_work(void *instance)
{
	int ret = 0;
	struct tee_agent_kernel_ops *agent_instance = NULL;

	agent_instance = (struct tee_agent_kernel_ops *)instance;
	while (!kthread_should_stop()) {
		tlogd("%s agent loop++++\n", agent_instance->agent_name);
		ret = tc_ns_wait_event(agent_instance->agent_id);
		if (ret) {
			tloge("%s wait event fail\n",
				agent_instance->agent_name);
			break;
		}
		if (agent_instance->tee_agent_work != NULL) {
			ret = agent_instance->tee_agent_work(agent_instance);
			if (ret)
				tloge("%s agent work fail\n",
					agent_instance->agent_name);
		}
		ret = tc_ns_send_event_response(agent_instance->agent_id);
		if (ret) {
			tloge("%s send event response fail\n",
				agent_instance->agent_name);
			break;
		}
		tlogd("%s agent loop----\n", agent_instance->agent_name);
	}

	return ret;
}

static int def_tee_agent_run(struct tee_agent_kernel_ops *agent_instance)
{
	tc_ns_shared_mem *shared_mem = NULL;
	tc_ns_dev_file dev = {0};
	int ret;
	int page_order = 8;

	/* 1. Allocate agent buffer */
	shared_mem = tc_mem_allocate(
		(unsigned int)(AGENT_BUFF_SIZE * page_order), true);
	while ((IS_ERR_OR_NULL(shared_mem)) && (page_order > 0)) {
		page_order /= PAGE_ORDER_RATIO;
		shared_mem = tc_mem_allocate(
			(unsigned int)(AGENT_BUFF_SIZE * page_order), true);
	}
	if (IS_ERR_OR_NULL(shared_mem)) {
		tloge("allocate agent buffer fail\n");
		ret = PTR_ERR(shared_mem);
		goto out;
	}

	atomic_set(&shared_mem->usage, 1);
	agent_instance->agent_buffer = shared_mem;

	/* 2. Register agent buffer to TEE */
	ret = tc_ns_register_agent(&dev, agent_instance->agent_id, shared_mem);
	if (ret) {
		tloge("register agent buffer fail,ret =0x%x\n", ret);
		ret = -1;
		goto out;
	}

	/* 3. Creat thread to run agent */
	agent_instance->agent_thread =
		kthread_run(def_tee_agent_work, (void *)agent_instance,
			"agent_%s", agent_instance->agent_name);
	if (IS_ERR_OR_NULL(agent_instance->agent_thread)) {
		tloge("kthread creat fail\n");
		ret = PTR_ERR(agent_instance->agent_thread);
		agent_instance->agent_thread = NULL;
		goto out;
	}
	return AGENT_SUCCESS;

out:
	if (!IS_ERR_OR_NULL(shared_mem)) {
		tc_mem_free(shared_mem);
		agent_instance->agent_buffer = NULL;
		shared_mem = NULL;
	}

	return ret;
}

static int def_tee_agent_stop(struct tee_agent_kernel_ops *agent_instance)
{
	int ret;

	if (tc_ns_send_event_response(agent_instance->agent_id))
		tloge("failed to send response for agent %d\n",
			agent_instance->agent_id);
	ret = tc_ns_unregister_agent(agent_instance->agent_id);
	if (ret != 0)
		tloge("failed to unregister agent %d\n",
			agent_instance->agent_id);
	if (!IS_ERR_OR_NULL(agent_instance->agent_thread))
		kthread_stop(agent_instance->agent_thread);

	/* release share mem obtained in def_tee_agent_run() */
	put_sharemem_struct(agent_instance->agent_buffer);
	return AGENT_SUCCESS;
}

static struct tee_agent_kernel_ops g_def_tee_agent_ops = {
	.agent_name = "default",
	.agent_id = 0,
	.tee_agent_init = NULL,
	.tee_agent_run = def_tee_agent_run,
	.tee_agent_work = NULL,
	.tee_agent_exit = NULL,
	.tee_agent_stop = def_tee_agent_stop,
	.tee_agent_crash_work = NULL,
	.list = LIST_HEAD_INIT(g_def_tee_agent_ops.list)
};

static int tee_agent_kernel_init(void)
{
	struct tee_agent_kernel_ops *agent_ops = NULL;
	int ret = 0;
	bool tmp_check_status = false;

	list_for_each_entry(agent_ops, &g_tee_agent_list, list) {
		/* Check the agent validity */
		tmp_check_status = ((agent_ops->agent_id == 0) ||
			(agent_ops->agent_name == NULL) ||
			(agent_ops->tee_agent_work == NULL));
		if (tmp_check_status) {
			tloge("agent is invalid\n");
			continue;
		}
		tlogd("ready to init %s agent, id=0x%x\n",
			agent_ops->agent_name, agent_ops->agent_id);

		/* Initialize the agent */
		if (agent_ops->tee_agent_init != NULL) {
			ret = agent_ops->tee_agent_init(agent_ops);
		} else if (g_def_tee_agent_ops.tee_agent_init != NULL) {
			ret = g_def_tee_agent_ops.tee_agent_init(agent_ops);
		} else {
			tlogw("agent id %d has no init function\n",
				agent_ops->agent_id);
		}
		if (ret) {
			tloge("tee_agent_init %s failed\n",
				agent_ops->agent_name);
			continue;
		}

		/* Run the agent */
		if (agent_ops->tee_agent_run != NULL) {
			ret = agent_ops->tee_agent_run(agent_ops);
		} else if (g_def_tee_agent_ops.tee_agent_run != NULL) {
			ret = g_def_tee_agent_ops.tee_agent_run(agent_ops);
		} else {
			tlogw("agent id %d has no run function\n",
				agent_ops->agent_id);
		}
		if (ret) {
			tloge("tee_agent_run %s failed\n",
				agent_ops->agent_name);
			if (agent_ops->tee_agent_exit != NULL)
				agent_ops->tee_agent_exit(agent_ops);
			continue;
		}
	}

	return AGENT_SUCCESS;
}

static int tee_agent_kernel_exit(void)
{
	struct tee_agent_kernel_ops *agent_ops = NULL;

	list_for_each_entry(agent_ops, &g_tee_agent_list, list) {
		/* Stop the agent */
		if (agent_ops->tee_agent_stop != NULL) {
			agent_ops->tee_agent_stop(agent_ops);
		} else if (g_def_tee_agent_ops.tee_agent_stop != NULL) {
			g_def_tee_agent_ops.tee_agent_stop(agent_ops);
		} else {
			tlogw("agent id %d has no stop function\n",
				agent_ops->agent_id);
		}
		/* Uninitialize the agent */
		if (agent_ops->tee_agent_exit != NULL) {
			agent_ops->tee_agent_exit(agent_ops);
		} else if (g_def_tee_agent_ops.tee_agent_exit != NULL) {
			g_def_tee_agent_ops.tee_agent_exit(agent_ops);
		} else {
			tlogw("agent id %d has no exit function\n",
				agent_ops->agent_id);
		}
	}
	return AGENT_SUCCESS;
}

int tee_agent_clear_work(tc_ns_client_context *context,
	unsigned int dev_file_id)
{
	struct tee_agent_kernel_ops *agent_ops = NULL;

	list_for_each_entry(agent_ops, &g_tee_agent_list, list) {
		if (agent_ops->tee_agent_crash_work != NULL)
			agent_ops->tee_agent_crash_work(agent_ops,
				context, dev_file_id);
	}
	return AGENT_SUCCESS;
}

int tee_agent_kernel_register(struct tee_agent_kernel_ops *new_agent)
{
	if (new_agent == NULL)
		return AGENT_FALSE;
	INIT_LIST_HEAD(&new_agent->list);
	list_add_tail(&new_agent->list, &g_tee_agent_list);
	return AGENT_SUCCESS;
}

void agent_init(void)
{
	spin_lock_init(&g_agent_control.lock);
	INIT_LIST_HEAD(&g_agent_control.agent_list);

	INIT_LIST_HEAD(&g_tee_agent_list);
#if defined(CONFIG_HISI_RPMB)
	rpmb_agent_register();
#endif
	tee_agent_kernel_init();
	return;
}

int agent_exit(void)
{
	tee_agent_kernel_exit();
	return AGENT_SUCCESS;
}
