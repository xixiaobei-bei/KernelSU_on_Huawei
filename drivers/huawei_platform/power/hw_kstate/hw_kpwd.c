/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * File name: hw_kpwd.c
 * Description: implement to power watchdog driver.
 * Author: zhangguiwen@huawei.com
 * Version: 0.01
 * Date: 2019/11/06
 */

#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/string.h>

#define MAX_POWER_WATCHDOG_EXTEND_COUNT (8)
#define MAX_POWER_WATCHDOG_EXTEND_SIZE (64)

struct power_watchdog_reg_info_t {
	u32 length;
	u32 magic;
	u32 resource_type;
	u32 count;
	char data[0];
} __packed;

#define POWER_WATCHDOG_MAGIC 'k'
#define IOCTL_POWER_WATCHDOG_SET_INFO _IOW(POWER_WATCHDOG_MAGIC, 1, struct power_watchdog_reg_info_t)

struct power_watchdog_extend_info {
	char extend[MAX_POWER_WATCHDOG_EXTEND_SIZE];
};

struct power_watchdog_data_block {
	u32 resource_type;
	u64 stats_duration;
	u64 hold_time;
	char extend[MAX_POWER_WATCHDOG_EXTEND_SIZE];
};

struct power_watchdog_ctrl_info {
	struct semaphore wd_ctrl_sem;
	pid_t pid;
	size_t size;
	struct power_watchdog_data_block *block;
	struct power_watchdog_reg_info_t *reg_info;
	struct list_head list;
};

static DEFINE_MUTEX(g_pwd_mutex);
static LIST_HEAD(ctrl_info_list);

static void _power_watchdog_exit(pid_t pid)
{
	struct power_watchdog_ctrl_info *entry = NULL;
	struct power_watchdog_ctrl_info *tmp = NULL;

	mutex_lock(&g_pwd_mutex);
	list_for_each_entry_safe(entry, tmp, &ctrl_info_list, list) {
		if (pid != -1) {
			if (entry->pid != pid)
				continue;
		}
		if (entry->block != NULL)
			kfree(entry->block);
		if (entry->reg_info != NULL)
			kfree(entry->reg_info);
		list_del_init(&entry->list);
		kfree(entry);
		if (pid != -1)
			break;
	}
	mutex_unlock(&g_pwd_mutex);
}

struct power_watchdog_ctrl_info* power_watchdog_get_ctrl_info_byname(const char *extend)
{
	size_t i;
	struct power_watchdog_reg_info_t *reg_info = NULL;
	struct power_watchdog_extend_info *reg_extend = NULL;
	struct power_watchdog_ctrl_info *entry = NULL;

	list_for_each_entry(entry, &ctrl_info_list, list) {
		reg_info = entry->reg_info;
		reg_extend = (struct power_watchdog_extend_info *)reg_info->data;
		for (i = 0; i < reg_info->count; i++, reg_extend++) {
			if (0 == strcmp(reg_extend->extend, extend))
				return entry;
		}
	}

	return NULL;
}

static ssize_t power_watchdog_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	pid_t pid = current->tgid;
	ssize_t ret = -1;
	int found = 0;
	struct power_watchdog_ctrl_info *entry = NULL;

	mutex_lock(&g_pwd_mutex);
	list_for_each_entry(entry, &ctrl_info_list, list) {
		if (entry->pid == pid) {
			found = 1;
			break;
		}
	}
	mutex_unlock(&g_pwd_mutex);

	if (found == 0) {
		pr_err("Failed to power watchdog alloc pid = %d", pid);
		return -1;
	}

	down(&entry->wd_ctrl_sem);

	mutex_lock(&g_pwd_mutex);
	if (entry->block != NULL) {
		if (count < entry->size) {
			pr_err("Failed to invalid param. count = %d ", count);
			goto exit;
		}

		if (copy_to_user(buf, entry->block, entry->size)) {
			pr_err("Failed to copy_to_user");
			goto exit;
		}
		ret = entry->size;
		kfree(entry->block);
		entry->block = NULL;
	}

exit:
	mutex_unlock(&g_pwd_mutex);

	return ret;
}

static int power_watchdog_get_data(const char __user *buf,
	size_t count, struct power_watchdog_data_block *block)
{
	int ret = -EINVAL;
	size_t size;
	char *kbuf = NULL;
	char *position = NULL;

	kbuf = kzalloc(count + 1, GFP_KERNEL);
	if (kbuf == NULL) {
		pr_err("Failed to kzalloc. count = %d", count);
		return -ENOMEM;
	}
	if (copy_from_user(kbuf, buf, count)) {
		pr_err("Failed to copy_from_user count %d", count);
		goto exit;
	}

	if (3 != sscanf(kbuf, "TYPE=%d DURATION=%ld HOLDTIME=%ld EXTEND=", &block->resource_type, &block->stats_duration,
		&block->hold_time)) {
		pr_err("Failed to parse kbuf %s", kbuf);
		goto exit;
	}

	position = strstr(kbuf, "EXTEND=");
	if (position == NULL) {
		pr_err("Failed to strstr kbuf %s", kbuf);
		goto exit;
	}

	position += strlen("EXTEND=");
	size = strlen(position);
	if (size > (MAX_POWER_WATCHDOG_EXTEND_SIZE - 1))
		size = MAX_POWER_WATCHDOG_EXTEND_SIZE - 1;
	strncpy(block->extend, position, size);
	block->extend[size] = '\0';
	ret = 0;
exit:
	if (kbuf != NULL) {
		kfree(kbuf);
		kbuf = NULL;
	}

	return ret;
}

static ssize_t power_watchdog_write(struct file *file, const char __user *buf,
	size_t count, loff_t *pos)
{
	int ret = 0;
	struct power_watchdog_data_block *block = NULL;
	struct power_watchdog_ctrl_info *ctrl_info = NULL;

	if (count > PAGE_SIZE) {
		pr_err("Failed to invalid param. count = %d", (int)count);
		return -EINVAL;
	}

	block = kzalloc(sizeof(struct power_watchdog_data_block), GFP_KERNEL);
	if (block == NULL) {
		pr_err("Failed to kzalloc. size = %d", sizeof(struct power_watchdog_data_block));
		ret = -ENOMEM;
		goto exit;
	}

	ret = power_watchdog_get_data(buf, count, block);
	if (ret != 0) {
		pr_err("Failed to get block. count = %d ret = %d", count, ret);
		goto exit;
	}

	mutex_lock(&g_pwd_mutex);
	ctrl_info = power_watchdog_get_ctrl_info_byname(block->extend);
	if (ctrl_info == NULL) {
		pr_err("Failed to getpid_byname extend=%s", block->extend);
		mutex_unlock(&g_pwd_mutex);
		ret = -EINVAL;
		goto exit;
	}

	if (ctrl_info->block != NULL)
		kfree(ctrl_info->block);
	ctrl_info->block = block;
	ctrl_info->size = sizeof(struct power_watchdog_data_block);
	mutex_unlock(&g_pwd_mutex);

	up(&ctrl_info->wd_ctrl_sem);

	return count;

exit:
	if (block != NULL) {
		kfree(block);
		block = NULL;
	}

	return ret;
}

int power_watchdog_add_ctrl_info(pid_t pid, struct power_watchdog_reg_info_t* reg_info)
{
	int found = 0;
	int ret = 0;
	struct power_watchdog_ctrl_info *entry = NULL;

	mutex_lock(&g_pwd_mutex);
	list_for_each_entry(entry, &ctrl_info_list, list) {
		if (entry->pid == pid) {
			if (entry->reg_info != NULL)
				kfree(entry->reg_info);
			entry->reg_info = reg_info;
			found = 1;
			break;
		}
	}

	if (found == 0) {
		entry = kzalloc(sizeof(struct power_watchdog_ctrl_info), GFP_KERNEL);
		if (entry == NULL) {
			pr_err("Failed to allocate memory");
			ret = -ENOMEM;
			goto exit;
		}

		sema_init(&entry->wd_ctrl_sem, 0);
		entry->pid = pid;
		entry->size = 0;
		entry->block = NULL;
		entry->reg_info = reg_info;
		list_add_tail(&entry->list, &ctrl_info_list);
	}
exit:
	mutex_unlock(&g_pwd_mutex);

	return ret;
}

int power_watchdog_set_reg_info(void __user *argp)
{
	pid_t pid = current->tgid;
	int ret = -1;
	size_t size;
	size_t i;
	struct power_watchdog_reg_info_t head;
	struct power_watchdog_reg_info_t *reg_info = NULL;
	struct power_watchdog_extend_info *reg_extend = NULL;

	if (copy_from_user(&head, argp, sizeof(struct power_watchdog_reg_info_t))) {
		pr_err("Failed to copy_from_user size = %d", sizeof(struct power_watchdog_reg_info_t));
		return -EFAULT;
	}
	if ((head.count > MAX_POWER_WATCHDOG_EXTEND_COUNT) || (head.magic != 0x6B707764) ||
		(head.length != sizeof(struct power_watchdog_reg_info_t) +
		head.count * sizeof(struct power_watchdog_extend_info))) {
		pr_err("Failed to invalid param length = %d count = %d magic = 0x%x", head.length, head.count, head.magic);
		return -EINVAL;
	}
	size = sizeof(struct power_watchdog_reg_info_t) + head.count * sizeof(struct power_watchdog_extend_info);
	reg_info = kzalloc(size, GFP_KERNEL);
	if (reg_info == NULL) {
		pr_err("Failed to kzalloc size = %d", size);
		return -ENOMEM;
	}
	if (copy_from_user(reg_info, argp, size)) {
		pr_err("Failed to copy_from_user size = %d", size);
		ret = -EFAULT;
		goto exit;
	}

	reg_extend = (struct power_watchdog_extend_info *)reg_info->data;
	for (i = 0; i < reg_info->count; i++, reg_extend++) {
		reg_extend->extend[MAX_POWER_WATCHDOG_EXTEND_SIZE - 1] = '\0';
	}

	return power_watchdog_add_ctrl_info(pid, reg_info);
exit:
	if (reg_info != NULL) {
		kfree(reg_info);
		reg_info = NULL;
	}

	return ret;
}

static int power_watchdog_process_notifier(struct notifier_block *self, unsigned long cmd, void *v)
{
	struct task_struct *task = v;

	if (task == NULL)
		goto exit;

	if (task->tgid == task->pid)
		_power_watchdog_exit(task->tgid);

exit:
	return NOTIFY_OK;
}

static struct notifier_block process_notifier_block = {
	.notifier_call = power_watchdog_process_notifier,
};

static long power_watchdog_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	void __user *argp = (void __user *)arg;

	switch (cmd) {
	case IOCTL_POWER_WATCHDOG_SET_INFO:
		rc = power_watchdog_set_reg_info(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

#ifdef CONFIG_COMPAT
static long power_watchdog_compat_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	return power_watchdog_ioctl(filp, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static const struct file_operations power_watchdog_device_fops = {
	.owner = THIS_MODULE,
	.read = power_watchdog_read,
	.write = power_watchdog_write,
	.unlocked_ioctl = power_watchdog_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= power_watchdog_compat_ioctl,
#endif
};

static struct miscdevice power_watchdog_device = {
	.name = "power_watchdog",
	.fops = &power_watchdog_device_fops,
	.minor = MISC_DYNAMIC_MINOR,
};

static int __init power_watchdog_init(void)
{
	int ret = 0;

	ret = misc_register(&power_watchdog_device);
	if (ret)
		pr_err("Failed to register power watchdog device");
	profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);
	return ret;
}

static void __exit power_watchdog_exit(void)
{
	profile_event_unregister(PROFILE_TASK_EXIT, &process_notifier_block);
	_power_watchdog_exit(-1);
}

late_initcall(power_watchdog_init);
module_exit(power_watchdog_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guiwen Zhang, <zhangguiwen@huawei.com>");
MODULE_DESCRIPTION("Huawei Power watchdog Device Driver");
