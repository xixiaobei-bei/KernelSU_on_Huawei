#include "dubai_utils.h"

#include <linux/cred.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#define IOC_PROC_CMDLINE_GET DUBAI_UTILS_DIR_IOC(WR, 1, struct proc_cmdline)
#define IOC_RSS_GET DUBAI_UTILS_DIR_IOC(R, 2, unsigned long long)
#define IOC_PIDS_GET DUBAI_UTILS_DIR_IOC(WR, 3, struct uid_name_to_pid)

static int dubai_get_proc_cmdline(void __user *argp);
static int dubai_get_rss(void __user *argp);
static int dubai_get_pids_by_uid_name(void __user *argp);

long dubai_ioctl_utils(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_PROC_CMDLINE_GET:
		rc = dubai_get_proc_cmdline(argp);
		break;
	case IOC_RSS_GET:
		rc = dubai_get_rss(argp);
		break;
	case IOC_PIDS_GET:
		rc = dubai_get_pids_by_uid_name(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int dubai_get_rss(void __user *argp)
{
	unsigned long rss = get_mm_rss(current->mm);
	unsigned long long size = rss * PAGE_SIZE;

	if (copy_to_user(argp, &size, sizeof(size))) {
		dubai_err("Failed to copy_to_user %llu.", size);
		return -EINVAL;
	}

	return 0;
}

/*
 * dubai_get_task_cmdline - get task cmdline
 * @struct task_struct *task: The task to get cmdline
 * @char *cmdline: Buffer of cmdline
 * @size_t size: The buffer size, must be equal or bigger than CMDLINE_LEN
 */
int dubai_get_cmdline(struct task_struct *task, char *cmdline, size_t size)
{
	int ret;

	if (cmdline == NULL || size < CMDLINE_LEN) {
		dubai_err("Null buffer or size must not be less than CMDLINE_LEN");
		return -EINVAL;
	}
	ret = get_cmdline(task, cmdline, size - 1);
	cmdline[size - 1] = '\0';
	if (ret <= 0 || strlen(cmdline) == 0)
		return -EFAULT;

	return 0;
}

static int __dubai_get_proc_cmdline(uid_t uid, pid_t pid, const char *comm, char *buffer, size_t size)
{
	int ret = 0;
	struct task_struct *task = NULL;
	bool cmp_comm = false, cmp_leader = false, cmp_cmdline = false;
	char cmdline[CMDLINE_LEN] = {0};

	if (comm == NULL || buffer == NULL || size < CMDLINE_LEN) {
		dubai_err("Invalid parameter");
		return -EINVAL;
	}

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		return -EFAULT;

	if (uid != from_kuid_munged(current_user_ns(), task_uid(task))) {
		dubai_err("Mismatched uid");
		ret = -EFAULT;
		goto exit;
	}
	ret = dubai_get_cmdline(task, cmdline, CMDLINE_LEN);
	if (ret == 0) {
		// compare the command
		if (strlen(comm) > 0) {
			cmp_comm = strncmp(comm, task->comm, TASK_COMM_LEN - 1) == 0;
			if (task->group_leader != NULL)
				cmp_leader = strncmp(comm, task->group_leader->comm, TASK_COMM_LEN - 1) == 0;
			cmp_cmdline = strstr(cmdline, comm) != NULL;
			if (!cmp_comm && !cmp_leader && !cmp_cmdline) {
				ret = -EFAULT;
				goto exit;
			}
		}
		strncpy(buffer, cmdline, size - 1);
	}

exit:
	put_task_struct(task);

	return ret;
}

int dubai_get_cmdline_by_uid_pid(uid_t uid, pid_t pid, char *buffer, size_t size)
{
	char comm[TASK_COMM_LEN] = {0};

	return __dubai_get_proc_cmdline(uid, pid, comm, buffer, size);
}

static int dubai_get_proc_cmdline(void __user *argp)
{
	struct proc_cmdline proc_cmd;

	if (copy_from_user(&proc_cmd, argp, sizeof(struct proc_cmdline)))
		return -EFAULT;

	proc_cmd.comm[TASK_COMM_LEN - 1] = '\0';
	proc_cmd.cmdline[CMDLINE_LEN - 1] = '\0';
	__dubai_get_proc_cmdline(proc_cmd.uid, proc_cmd.pid, proc_cmd.comm, proc_cmd.cmdline, CMDLINE_LEN);
	if (copy_to_user(argp, &proc_cmd, sizeof(struct proc_cmdline)))
		return -EFAULT;

	return 0;
}

static int dubai_get_pids_by_uid_name(void __user *argp)
{
	uid_t uid;
	int count = 0;
	struct uid_name_to_pid data;
	struct task_struct *task = NULL, *temp = NULL;

	if (copy_from_user(&data, argp, sizeof(struct uid_name_to_pid)))
		return -EFAULT;

	data.name[CMDLINE_LEN - 1] = '\0';
	if (!strlen(data.name)) {
		dubai_err("Invalid parameter");
		return -EINVAL;
	}
	read_lock(&tasklist_lock);
	do_each_thread(temp, task) {
		if (count >= MAX_PID_NUM)
			goto exit;
		uid = from_kuid_munged(current_user_ns(), task_uid(task));
		if (uid == data.uid && strstr(data.name, task->comm)) {
			data.pid[count] = task->pid;
			count++;
		}
	} while_each_thread(temp, task);

exit:
	read_unlock(&tasklist_lock);
	if (copy_to_user(argp, &data, sizeof(struct uid_name_to_pid)))
		return -EFAULT;

	return 0;
}

struct dev_transmit_t *dubai_alloc_transmit(size_t data_size)
{
	int total_size;
	struct dev_transmit_t *transmit = NULL;

	if (data_size > MAX_ALLOC_MEM_LENGTH) {
		dubai_err("Invalid length %d", (int)data_size);
		return NULL;
	}
	total_size = sizeof(struct dev_transmit_t) + data_size;
	transmit = kzalloc(total_size, GFP_KERNEL);
	if (transmit == NULL)
		return NULL;
	transmit->length = data_size;

	return transmit;
}

size_t dubai_get_transmit_size(struct dev_transmit_t *transmit)
{
	if (transmit == NULL)
		return 0;

	return sizeof(struct dev_transmit_t) + transmit->length;
}

void dubai_free_transmit(void *transmit)
{
	kfree(transmit);
	transmit = NULL;
}

