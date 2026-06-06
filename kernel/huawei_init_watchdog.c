/*
 * huawei_init_watchdog.c
 *
 * A kernel thread for monitoring the init process
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <linux/mm.h>
#include <linux/cpu.h>
#include <linux/nmi.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/lockdep.h>
#include <linux/export.h>
#include <linux/sysctl.h>
#include <linux/utsname.h>
#include <trace/events/sched.h>
#include <linux/ptrace.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <asm/traps.h>
#include <linux/suspend.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#endif
#include <chipset_common/hwzrhung/zrhung.h>

#define ENABLE_SHOW_LEN  6
#define SIG_TO_INIT 40
#define SIG_INT_VALUE 1234
#define WATCHDOG_CHECK_TIME 30

static unsigned int init_watchdog_enable;
static unsigned int init_watchdog_firstkick;
static unsigned int init_watchdog_status = 1;
static unsigned int init_block_times;
static struct timer_list init_watchdog_timer;
static struct workqueue_struct *queue;
static struct work_struct work;

static void send_signal_to_init(int signum)
{
	int pid = 1;
	int ret;
	struct siginfo info = {0};
	struct task_struct *t = NULL;

	info.si_signo = signum;
	info.si_code = SI_QUEUE;
	info.si_int = SIG_INT_VALUE;
	rcu_read_lock();
	t = find_task_by_vpid(pid);
	rcu_read_unlock();
	if (t == NULL) {
		pr_err("InitWatchdog: no such pid\n");
		return;
	}

	ret = send_sig_info(signum, &info, t);
	if (ret < 0)
		pr_err("InitWatchdog: error sending signal\n");
	else
		pr_err("InitWatchdog: sending signal success\n");
}

static void init_watchdog_check(void)
{
	struct task_struct *p = NULL;

	if (!init_watchdog_enable || !init_watchdog_firstkick) {
		pr_info("InitWatchdog: init_watchdog is not enabled\n");
		goto no_block;
	}
	if (init_watchdog_status) {
		init_watchdog_status = 0;
		goto no_block;
	}
	p = pid_task(find_vpid(1), PIDTYPE_PID);
	if (p == NULL) {
		pr_err("InitWatchdog: can not find pid 1!");
		return;
	}
	if (p->flags & PF_FROZEN) {
		pr_info("InitWatchdog: init process is frozen\n");
		goto no_block;
	}
	init_block_times++;
	if (p->state == TASK_UNINTERRUPTIBLE) {
		pr_err("InitWatchdog: init process is in d-state\n");
#ifdef CONFIG_HW_ZEROHUNG
		if (init_block_times == 1) {
			pr_err("InitProcessWatchdog_KernelStack start\n");
			sched_show_task(p);
			pr_err("InitProcessWatchdog_KernelStack end\n");
			zrhung_send_event(ZRHUNG_WP_INIT,
					  "R=InitProcessWatchdog_KernelStack",
					  "Init process in d-state for 30s");
		}
#endif
		goto blocked;
	}
	send_signal_to_init(SIG_TO_INIT);
	pr_err("InitWatchdog: init process blocked for long time\n");
#ifdef CONFIG_HW_ZEROHUNG
	if (init_block_times == 1)
		zrhung_send_event(ZRHUNG_WP_INIT,
				  "R=InitProcessWatchdog_UserStack",
				  "Init process blocked for 30s");
#endif
	goto blocked;

blocked:
	sched_show_task(p);
	return;
no_block:
	init_block_times = 0;
}

static void work_handler(struct work_struct *data)
{
	init_watchdog_check();
}

static ssize_t init_watchdog_show(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	if (init_watchdog_status)
		return snprintf(buf, ENABLE_SHOW_LEN, "kick\n");
	if (init_watchdog_enable)
		return snprintf(buf, ENABLE_SHOW_LEN, "on\n");
	else
		return snprintf(buf, ENABLE_SHOW_LEN, "off\n");
}

static ssize_t init_watchdog_store(struct kobject *kobj,
				   struct kobj_attribute *attr,
				   const char *buf, size_t count)
{
	char tmp[6]; /* on/off/kick */
	size_t len;
	char *p = NULL;

	if ((count < 2) || (count > (sizeof(tmp) - 1))) {
		pr_err("InitWatchdog: string too long or too short\n");
		return -EINVAL;
	}
	if (!buf)
		return -EINVAL;
	p = memchr(buf, '\n', count);
	len = p ? (size_t)(p - buf) : count;
	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, buf, len);
	if (strncmp(tmp, "on", strlen(tmp)) == 0) {
		init_watchdog_enable = 1;
		pr_info("InitWatchdog: init_watchdog_enable is set to enable\n");
	} else if (unlikely(strncmp(tmp, "off", strlen(tmp)) == 0)) {
		init_watchdog_enable = 0;
		pr_info("InitWatchdog: init_watchdog_enable is set to disable\n");
	} else if (likely(strncmp(tmp, "kick", strlen(tmp)) == 0)) {
		init_watchdog_firstkick = 1;
		init_watchdog_status = 1;
		pr_info("InitWatchdog: init_watchdog is kicked\n");
	} else {
		pr_err("InitWatchdog: only accept on off or kick\n");
	}
	return (ssize_t) count;
}

static struct kobj_attribute init_watchdog_attribute = {
	.attr = {
		 .name = "init",
		 .mode = 0640,
	},
	.show = init_watchdog_show,
	.store = init_watchdog_store,
};

static struct attribute *attrs[] = {
	&init_watchdog_attribute.attr,
	NULL
};

static struct attribute_group init_attr_group = {
	.attrs = attrs,
};

struct kobject *init_kobj;
int create_sysfs_init(void)
{
	int retval;

	while (kernel_kobj == NULL)
		msleep(1000); /* sleep 1000ms */

	init_kobj = kobject_create_and_add("init_watchdog", kernel_kobj);
	if (!init_kobj) {
		pr_err("InitWatchdog: create init_watchdog failed\n");
		return -ENOMEM;
	}
	retval = sysfs_create_group(init_kobj, &init_attr_group);
	if (retval)
		kobject_put(init_kobj);

	return retval;
}

static void init_watchdog_handler(unsigned long time)
{
	mod_timer(&init_watchdog_timer, jiffies +
		  msecs_to_jiffies(1000 * WATCHDOG_CHECK_TIME)); /* to ms */
	queue_work(queue, &work);
}

static int __init init_watchdog_init(void)
{
	int init_ret;

	init_ret = create_sysfs_init();
	if (init_ret) {
		pr_err("InitWatchdog: create_sysfs_init fail\n");
		return -1;
	}
	queue = create_workqueue("init_wdt_wkq");
	if (queue == NULL) {
		pr_err("InitWatchdog: creat workqueue failed\n");
		return -1;
	}
	INIT_WORK(&work, work_handler);
	init_timer(&init_watchdog_timer);
	init_watchdog_timer.function = init_watchdog_handler;
	init_watchdog_timer.expires = jiffies + HZ * WATCHDOG_CHECK_TIME;
	add_timer(&init_watchdog_timer);
	return 0;
}

subsys_initcall(init_watchdog_init);
