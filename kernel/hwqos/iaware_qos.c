/*
 * iaware_qos.c
 *
 * Qos schedule implementation
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwqos/iaware_qos.h>

#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

struct trans_qos_allow g_qos_trans_allows[QOS_TRANS_THREADS_NUM];
spinlock_t g_trans_qos_lock;

static void init_trans_qos_allows(void)
{
	int i;

	spin_lock_init(&g_trans_qos_lock);
	for (i = 0; i < QOS_TRANS_THREADS_NUM; i++)
		memset(&g_qos_trans_allows[i],
			0,
			sizeof(g_qos_trans_allows[i]));
}

static long qos_ctrl_get_qos_stat(unsigned long arg)
{
	struct task_struct *tsk = NULL;
	struct task_config qs;
	void __user *uarg = (void __user *)arg;
	long ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&qs, uarg, sizeof(qs))) {
		pr_warn("QOS_CTRL_GET_QOS_STAT: failed to copy from user\n");
		return -EFAULT;
	}
	// get qos
	rcu_read_lock();
	if (qs.pid) {
		tsk = find_task_by_vpid(qs.pid);
		if (!tsk)
			ret = -EFAULT;
		else
			qs.value = get_task_qos(tsk);
	}
	rcu_read_unlock();

	if (copy_to_user(uarg, &qs, sizeof(qs))) {
		pr_warn("QOS_CTRL_GET_QOS_STAT: failed to copy to user\n");
		return -EFAULT;
	}

	return ret;
}

static long qos_ctrl_set_qos_stat(unsigned long arg, bool is_thread)
{
	struct task_struct *tsk = NULL;
	struct task_config qs;
	void __user *uarg = (void __user *)arg;
	long ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&qs, uarg, sizeof(qs))) {
		pr_warn("QOS_CTRL_SET_QOS_STAT: failed to copy from user\n");
		return -EFAULT;
	}

	if ((qs.pid <= 0) || (qs.value < VALUE_QOS_LOW)
		|| (qs.value > VALUE_QOS_CRITICAL)) {
		pr_warn("QOS_CTRL_SET_QOS_STAT: bad parameter\n");
		return -EINVAL;
	}
	// set qos
	if (qs.pid) {
		rcu_read_lock();
		tsk = find_task_by_vpid(qs.pid);
		if (!tsk) {
			rcu_read_unlock();
			ret = -EFAULT;
			return ret;
		}
		get_task_struct(tsk);
		rcu_read_unlock();
		if (is_thread)
			set_task_qos_by_tid(tsk, qs.value);
		else
			set_task_qos_by_pid(tsk, qs.value);
		put_task_struct(tsk);
	}

	return ret;
}

static long qos_ctrl_get_qos_whole(unsigned long arg)
{
	struct task_struct *tsk = NULL;
	struct qos_whole qs_whole;
	void __user *uarg = (void __user *)arg;
	long ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&qs_whole, uarg, sizeof(qs_whole))) {
		pr_warn("QOS_CTRL_GET_QOS_WHOLE: failed to copy from user\n");
		return -EFAULT;
	}
	// get qos whole
	rcu_read_lock();
	if (qs_whole.pid) {
		tsk = find_task_by_vpid(qs_whole.pid);
		if (!tsk) {
			ret = -EFAULT;
		} else {
			qs_whole.proc_qos =
				atomic_read(&tsk->proc_qos->dynamic_qos);
			qs_whole.proc_usage =
				atomic_read(&tsk->proc_qos->usage);
			qs_whole.thread_qos =
				atomic_read(&tsk->thread_qos.dynamic_qos);
			qs_whole.thread_usage =
				atomic_read(&tsk->thread_qos.usage);
			qs_whole.trans_flags =
				atomic_read(&tsk->trans_flags);
		}
	}
	rcu_read_unlock();

	if (copy_to_user(uarg, &qs_whole, sizeof(qs_whole))) {
		pr_warn("QOS_CTRL_GET_QOS_WHOLE: failed to copy to user\n");
		return -EFAULT;
	}

	return ret;
}

static int sched_set_task_vip_prio(unsigned long arg)
{
#ifdef CONFIG_HUAWEI_SCHED_VIP
	struct task_struct *tsk = NULL;
	struct task_config config;
	void __user *uarg = (void __user *)arg;
	int ret = 0;

	if (unlikely(!VIP_PRIO_SET_ENABLE))
		return ret;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&config, uarg, sizeof(config))) {
		pr_warn("sched_set_task_vip_prio: failed to copy from user\n");
		return -EFAULT;
	}

	if (config.pid <= 0 || config.value < 0) {
		pr_warn("sched_set_task_vip_prio: bad parameter\n");
		return -EINVAL;
	}

	rcu_read_lock();
	tsk = find_task_by_vpid(config.pid);
	if (!tsk) {
		rcu_read_unlock();
		return -ESRCH;
	}
	get_task_struct(tsk);
	rcu_read_unlock();

	ret = set_vip_prio(tsk, (unsigned int)config.value);
	put_task_struct(tsk);

	return ret;
#else
	return 0;
#endif
}

static int sched_set_task_min_util(unsigned long arg)
{
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
	struct task_struct *tsk = NULL;
	struct task_config config;
	void __user *uarg = (void __user *)arg;
	int ret = 0;

	if (unlikely(!MIN_UTIL_SET_ENABLE))
		return ret;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&config, uarg, sizeof(config))) {
		pr_warn("sched_set_task_min_util: failed to copy from user\n");
		return -EFAULT;
	}

	if (config.pid <= 0 || config.value < 0) {
		pr_warn("sched_set_task_min_util: bad parameter\n");
		return -EINVAL;
	}

	rcu_read_lock();
	tsk = find_task_by_vpid(config.pid);
	if (!tsk) {
		rcu_read_unlock();
		return -ESRCH;
	}
	get_task_struct(tsk);
	rcu_read_unlock();

	ret = set_task_min_util(tsk, (unsigned int)config.value);
	put_task_struct(tsk);

	return ret;
#else
	return 0;
#endif
}

static long qos_ctrl_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return ret;

	if (_IOC_TYPE(cmd) != QOS_CTRL_MAGIC) {
		pr_warn("qos_ctrl: invalid magic number. type = %d\n",
			_IOC_TYPE(cmd));
		return -EINVAL;
	}

	if (_IOC_NR(cmd) > CTRL_MAX_NR) {
		pr_warn("qos_ctrl: invalid qos cmd. cmd = %d\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	switch (cmd) {
	case QOS_CTRL_GET_QOS_STAT:
		ret = qos_ctrl_get_qos_stat(arg);
		break;
	case QOS_CTRL_SET_QOS_STAT_THREAD:
		ret = qos_ctrl_set_qos_stat(arg, true);
		break;
	case QOS_CTRL_SET_QOS_STAT_PROC:
		ret = qos_ctrl_set_qos_stat(arg, false);
		break;
	case QOS_CTRL_GET_QOS_WHOLE:
		ret = qos_ctrl_get_qos_whole(arg);
		break;
	case CTRL_SET_TASK_VIP_PRIO:
		ret = sched_set_task_vip_prio(arg);
		break;
	case CTRL_SET_TASK_MIN_UTIL:
		ret = sched_set_task_min_util(arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long qos_ctrl_compat_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	/*lint -e712*/
	return qos_ctrl_ioctl(file, cmd, (unsigned long)(compat_ptr(arg)));
	/*lint +e712*/
}
#endif

static int qos_ctrl_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int qos_ctrl_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations qos_ctrl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = qos_ctrl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = qos_ctrl_compat_ioctl,
#endif
	.open = qos_ctrl_open,
	.release = qos_ctrl_release,
};

static struct miscdevice qos_ctrl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "iaware_qos_ctrl",
	.fops = &qos_ctrl_fops,
};

static int __init qos_ctrl_dev_init(void)
{
	int err;

	err = misc_register(&qos_ctrl_device);
	if (err)
		return err;
	init_trans_qos_allows();
	return 0;
}

static void __exit qos_ctrl_dev_exit(void)
{
	misc_deregister(&qos_ctrl_device);
}

module_init(qos_ctrl_dev_init);
module_exit(qos_ctrl_dev_exit);
