/*
 * perf_ctrl.c
 *
 * interfaces defined for perf_ctrl
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "perf_ctrl: " fmt

#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/perf_ctrl.h>

static unsigned long get_dev_cap(void)
{
	unsigned long cap = 0;

	cap |= BIT(CAP_AI_SCHED_COMM_CMD);
#ifdef CONFIG_SCHED_RTG
	cap |= BIT(CAP_RTG_CMD);
#endif
#ifdef CONFIG_RENDER_RT
	cap |= BIT(CAP_RENDER_RT_CMD);
#endif

	return cap;
}

static int perf_ctrl_get_dev_cap(void __user *uarg)
{
	unsigned long cap = get_dev_cap();

	if (uarg == NULL)
		return -EINVAL;

	if (copy_to_user(uarg, &cap, sizeof(unsigned long))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		return -EFAULT;
	}

	return 0;
}

/* main function entry point */
static long perf_ctrl_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg)
{
	void __user *uarg = (void __user *)(uintptr_t)arg;
	unsigned int func_id = _IOC_NR(cmd);
	long ret;

	if (uarg == NULL) {
		pr_err("%s: invalid user uarg\n", __func__);
		return -EINVAL;
	}

	if (_IOC_TYPE(cmd) != PERF_CTRL_MAGIC) {
		pr_err("%s: PERF_CTRL_MAGIC fail, TYPE=%d\n",
		       __func__, _IOC_TYPE(cmd));
		return -EINVAL;
	}

	if (func_id >= PERF_CTRL_MAX_NR) {
		pr_err("%s: PERF_CTRL_MAX_NR fail, _IOC_NR(cmd)=%d, MAX_NR=%d\n",
		       __func__, _IOC_NR(cmd), PERF_CTRL_MAX_NR);
		return -EINVAL;
	}

	switch (cmd) {
	case PERF_CTRL_GET_SCHED_STAT:
		return perf_ctrl_get_sched_stat(uarg);

	case PERF_CTRL_GET_IPA_STAT:
		return perf_ctrl_get_ipa_stat(uarg);

	case PERF_CTRL_GET_DDR_FLUX:
		return perf_ctrl_get_ddr_flux(uarg);

	case PERF_CTRL_GET_RELATED_TID:
		return perf_ctrl_get_related_tid(uarg);

	case PERF_CTRL_DRG_GET_DEV_FREQ:
		return perf_ctrl_get_drg_dev_freq(uarg);

	case PERF_CTRL_GET_THERMAL_CDEV_POWER:
		return perf_ctrl_get_thermal_cdev_power(uarg);

	case PERF_CTRL_GET_GPU_FENCE:
		return perf_ctrl_get_gpu_fence(uarg);

	case PERF_CTRL_GET_GPU_BUFFER_SIZE:
		return perf_ctrl_get_gpu_buffer_size(uarg);

	case PERF_CTRL_SET_LB_POLICY:
		return perf_ctrl_lb_set_policy(uarg);

	case PERF_CTRL_RESET_LB_POLICY:
		return perf_ctrl_lb_reset_policy(uarg);

	case PERF_CTRL_GET_LB_LITE_INFO:
		return perf_ctrl_lb_lite_info(uarg);

	case PERF_CTRL_ENABLE_GPU_LB:
		return perf_ctrl_enable_gpu_lb(uarg);

	case PERF_CTRL_GET_DEV_CAP:
		return perf_ctrl_get_dev_cap(uarg);

	case PERF_CTRL_SET_FRAME_RATE:
		return perf_ctrl_set_frame_rate(uarg);

	case PERF_CTRL_SET_FRAME_MARGIN:
		return perf_ctrl_set_frame_margin(uarg);

	case PERF_CTRL_SET_FRAME_STATUS:
		return perf_ctrl_set_frame_status(uarg);

	case PERF_CTRL_SET_TASK_RTG:
		return perf_ctrl_set_task_rtg(uarg);

	case PERF_CTRL_SET_RTG_CPUS:
		return perf_ctrl_set_rtg_cpus(uarg);

	case PERF_CTRL_SET_RTG_FREQ:
		return perf_ctrl_set_rtg_freq(uarg);

	case PERF_CTRL_SET_RTG_FREQ_UPDATE_INTERVAL:
		return perf_ctrl_set_rtg_freq_update_interval(uarg);

	case PERF_CTRL_SET_RTG_UTIL_INVALID_INTERVAL:
		return perf_ctrl_set_rtg_util_invalid_interval(uarg);

	case PERF_CTRL_SET_RTG_LOAD_MODE:
		return perf_ctrl_set_rtg_load_mode(uarg);

	case PERF_CTRL_SET_RTG_ED_PARAMS:
		return perf_ctrl_set_rtg_ed_params(uarg);

	case PERF_CTRL_INIT_RENDER_PID:
		return init_render_rthread(uarg);

	case PERF_CTRL_DESTROY_RENDER_RT:
		return destroy_render_rthread(uarg);

	case PERF_CTRL_STOP_RENDER_RT:
		return stop_render_rthread(uarg);

	case PERF_CTRL_GET_RENDER_RT:
		return get_render_rthread(uarg);

	case PERF_CTRL_GET_RENDER_HT:
		return get_render_hrthread(uarg);

	case PERF_CTRL_SET_VIP_PRIO:
		return perf_ctrl_set_vip_prio(uarg);

	case PERF_CTRL_SET_FAVOR_SMALL_CAP:
		return perf_ctrl_set_favor_small_cap(uarg);

	case PERF_CTRL_SET_TASK_RTG_MIN_FREQ:
		return perf_ctrl_set_task_rtg_min_freq(uarg);

	case PERF_CTRL_SET_TASK_MIN_UTIL:
		return perf_ctrl_set_task_min_util(uarg);

	case PERF_CTRL_GET_CPU_BUSY_TIME:
		return perf_ctrl_get_cpu_busy_time(uarg);

	case PERF_CTRL_SET_TASK_MAX_UTIL:
		return perf_ctrl_set_task_max_util(uarg);

	case PERF_CTRL_GET_TASK_YIELD_TIME:
		return perf_ctrl_get_task_yield_time(uarg);

	case PERF_CTRL_SET_TASK_L3C_PART:
		return perf_ctrl_set_task_l3c_part(uarg);

	default:
		pr_err("cmd error, here is default, cmd=%u\n", cmd);
		ret = -EINVAL;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long perf_ctrl_compact_ioctl(struct file *file,
				    unsigned int cmd, unsigned long arg)
{
	return perf_ctrl_ioctl(file, cmd, (unsigned long)(compat_ptr(arg)));
}
#endif

static int perf_ctrl_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int perf_ctrl_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations perf_ctrl_fops = {
	.owner = THIS_MODULE,
#ifdef CONFIG_COMPAT
	.compat_ioctl = perf_ctrl_compact_ioctl,
#endif
	.unlocked_ioctl = perf_ctrl_ioctl,
	.open = perf_ctrl_open,
	.release = perf_ctrl_release,
};

static struct miscdevice perf_ctrl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hisi_perf_ctrl",
	.fops = &perf_ctrl_fops,
};

static int __init perf_ctrl_dev_init(void)
{
	int err;

	err = misc_register(&perf_ctrl_device);
	if (err != 0)
		return err;

	return 0;
}

static void __exit perf_ctrl_dev_exit(void)
{
	misc_deregister(&perf_ctrl_device);
}

module_init(perf_ctrl_dev_init);
module_exit(perf_ctrl_dev_exit);
