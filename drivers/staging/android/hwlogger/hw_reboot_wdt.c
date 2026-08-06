/*
 * hw_reboot_wdt.c
 *
 * watch reboot/shutdown and notify logcat/xlogcat by sending signal SIGUSR2
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
 *
 */

#include <asm/siginfo.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/proc_fs.h>
#include <linux/rcupdate.h>
#include <linux/reboot.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/signal.h>
#endif
#include <linux/uaccess.h>

#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG	reboot_wdt
HWLOG_REGIST();

#define SIG_TEST	12	/* 12 for SIGUSR2 */
#define LIST_MAX	10
#define SIG_VAL		1234

struct proc_dir_entry *proc_file;
static struct gs_pid_list_t {
	int pid;
	char name[NAME_MAX];
} gs_pid_list[LIST_MAX];

static void clear_pid_list(void)
{
	int i;

	for (i = 0; i < LIST_MAX; i++) {
		gs_pid_list[i].pid = 0;
		memset(gs_pid_list[i].name, 0, NAME_MAX);
	}
}

static void insert_pid_list(int pid)
{
	int i;
	struct task_struct *t = NULL;

	rcu_read_lock();
	/* find the task_struct associated with this pid */
	t = find_task_by_vpid(pid);
	if (!t) {
		hwlog_debug("%s no such pid\n", __func__);
		rcu_read_unlock();
		return;
	}
	rcu_read_unlock();

	for (i = 0; i < LIST_MAX; i++) {
		if (strncmp(t->comm, gs_pid_list[i].name, NAME_MAX) == 0) {
			if (gs_pid_list[i].pid == pid)
				gs_pid_list[i].pid = 0;
			else
				gs_pid_list[i].pid = pid;

			return;
		}
	}
	/* there was no same task name, so store a new one */
	for (i = 0; i < LIST_MAX; i++) {
		if (gs_pid_list[i].pid == 0) {
			gs_pid_list[i].pid = pid;
			memcpy(gs_pid_list[i].name, t->comm, strlen(t->comm));
			hwlog_info("%s add new (%d)pid:%d, %s\n",
				   __func__, i, pid, t->comm);
			return;
		}
	}
	hwlog_info("gs_pid_list was full\n");
}

static ssize_t recv_pid(struct file *file, const char __user *buf,
			size_t count, loff_t *ppos)
{
	char mybuf[LIST_MAX];
	unsigned int pid = 0;
	/* read the value from user space */
	if (count > LIST_MAX)
		return -EINVAL;

	if (copy_from_user(mybuf, buf, count)) {
		hwlog_debug("%s copy failed\n", __func__);
	} else {
		if (kstrtoint(mybuf, 0, &pid) == 0)
			insert_pid_list(pid);
	}

	return count;
}

static int reboot_callback(struct notifier_block *self, unsigned long event,
			   void *data)
{
	struct siginfo info;
	struct task_struct *t = NULL;
	int i;
	int ret;

	hwlog_info("Shutdown... Event code: %li\n", event);

	/* send the signal */
	memset(&info, 0, sizeof(info));
	info.si_signo = SIG_TEST;
	info.si_code = SI_QUEUE;
	/*
	 * this is bit of a trickery: SI_QUEUE is normally used by sigqueue from
	 * user space, and kernel space should use SI_KERNEL. But if SI_KERNEL
	 * is used the real_time data is not delivered to the user space signals
	 * handler function.
	 */
	/* real time signals may have 32 bits of data */
	info.si_int = SIG_VAL;

	for (i = 0; i < LIST_MAX; i++) {
		if (gs_pid_list[i].pid != 0) {
			rcu_read_lock();
			/* find the task_struct associated with this pid */
			t = find_task_by_vpid(gs_pid_list[i].pid);
			if (!t) {
				hwlog_err("%s no such pid\n", __func__);
				rcu_read_unlock();
				continue;
			}
			rcu_read_unlock();
			ret = send_sig_info(SIG_TEST, &info, t);
			hwlog_info("%s will notify pid(%d):%d, %s\n", __func__,
				   i, gs_pid_list[i].pid, t->comm);
			if (ret < 0)
				hwlog_err("%s error sending signal\n", __func__);
		}
		hwlog_info("%s each (%d)pid:%d\n", __func__, i,
			   gs_pid_list[i].pid);
	}
	return NOTIFY_OK;
}

static const struct file_operations config_node_fops = {
	.write = recv_pid,
};

static struct notifier_block reboot_wdt_notifier = {
	.notifier_call = reboot_callback,
};

static int __init reboot_wdt_module_init(void)
{
	hwlog_info("%s\n", __func__);
	clear_pid_list();
	/*
	 * we need to know the pid of the user space process
	 * -> we use debugfs for this. As soon as a pid is written to
	 * this file, a signal is sent to that pid
	 */
	/* only root can write to this file (no read) */
	proc_file = proc_create("reboot_watchdog", 0200, NULL,
				&config_node_fops);
	register_reboot_notifier(&reboot_wdt_notifier);
	return 0;
}

static void __exit reboot_wdt_module_exit(void)
{
	hwlog_info("%s\n", __func__);
	remove_proc_entry("reboot_watchdog", NULL);
	unregister_reboot_notifier(&reboot_wdt_notifier);
}

module_init(reboot_wdt_module_init);
module_exit(reboot_wdt_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("reboot watchdog");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
