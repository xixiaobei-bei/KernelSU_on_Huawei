#ifndef DUBAI_UTILS_H
#define DUBAI_UTILS_H

#include <linux/sched.h>
#include <linux/uaccess.h>

#include <chipset_common/dubai/dubai_log.h>
#ifdef CONFIG_LOG_EXCEPTION
#include <log/log_usertype.h>
#endif

// keep CMDLINE_LEN as 128 bytes
#define CMDLINE_LEN				128
#define MAX_PID_NUM				5
#define MAX_ALLOC_MEM_LENGTH	0x100000

struct dev_transmit_t {
	int length;
	char data[0];
} __packed;

struct proc_cmdline {
	uid_t uid;
	pid_t pid;
	char comm[TASK_COMM_LEN];
	char cmdline[CMDLINE_LEN];
} __packed;

struct uid_name_to_pid {
	uid_t uid;
	pid_t pid[MAX_PID_NUM];
	char name[CMDLINE_LEN];
} __packed;

long dubai_ioctl_utils(unsigned int cmd, void __user *argp);
int dubai_get_cmdline(struct task_struct *task, char *cmdline, size_t size);
int dubai_get_cmdline_by_uid_pid(uid_t uid, pid_t pid, char *buffer, size_t size);
struct dev_transmit_t *dubai_alloc_transmit(size_t data_size);
size_t dubai_get_transmit_size(struct dev_transmit_t *transmit);
void dubai_free_transmit(void *transmit);

static inline int get_enable_value(void __user *argp, bool *enable)
{
	uint8_t value;

	if (copy_from_user(&value, argp, sizeof(uint8_t)))
		return -EFAULT;

	if (value != 1 && value != 0) {
		dubai_err("Invalid enable value: %u", value);
		return -EFAULT;
	}
	*enable = (value == 1);

	return 0;
}

static inline int get_timestamp_value(void __user *argp, long long *timestamp)
{
	if (copy_from_user(timestamp, argp, sizeof(long long)))
		return -EFAULT;

	return 0;
}

static inline bool dubai_is_beta_user(void)
{
#ifdef CONFIG_LOG_EXCEPTION
	return (get_logusertype_flag() == BETA_USER || get_logusertype_flag() == OVERSEA_USER);
#else
	return false;
#endif
}

#endif // DUBAI_UTILS_H
