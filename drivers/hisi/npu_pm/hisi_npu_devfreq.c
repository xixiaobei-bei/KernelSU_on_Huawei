/*
 *  Copyright (C) 2017 Hisilicon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/pm_opp.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <hisi_npu_pm.h>
#include "hisi_npu_pm_private.h"



int hisi_npu_recommended_frequency(struct device *dev, unsigned long *target_freq, u32 flags)
{
	struct dev_pm_opp *opp;
	unsigned long freq;

	if (!target_freq) {
		dev_err(dev, "%s target_freq is null\n", __func__);
		return -EFAULT;
	}

	freq = *target_freq;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
	rcu_read_lock();
#endif
	opp = devfreq_recommended_opp(dev, &freq, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
	rcu_read_unlock();
#endif
	if (IS_ERR_OR_NULL(opp)) {
		dev_err(dev, "failed to get opp (%ld)\n", PTR_ERR(opp));
		return -ESPIPE;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
	dev_pm_opp_put(opp);
#endif

	*target_freq = freq;

	return 0;
}

static int
hisi_npu_devfreq_target(struct device *dev, unsigned long *target_freq, u32 flags)
{
	int err;

	err = hisi_npu_recommended_frequency(dev, target_freq, flags);
	if (err) {
		dev_err(dev, "failed to get recommended frequency\n");
		return err;
	}

	err = hisi_npu_dvfs_hal(*target_freq);

	return err;
}

static int
hisi_npu_devfreq_cur_freq(struct device *dev, unsigned long *freq)
{
	struct npu_pm_device *pmdev = dev_get_drvdata(dev);
	struct devfreq *df = pmdev->devfreq;

	*freq = df->previous_freq;

	return 0;
}

static int
hisi_npu_get_dev_status(struct device *dev, struct devfreq_dev_status *stat)
{
	return 0;
}

static void hisi_npu_devfreq_exit(struct device *dev)
{
	struct npu_pm_device *pmdev = dev_get_drvdata(dev);
	struct devfreq_dev_profile *dp = pmdev->devfreq->profile;

	devm_kfree(pmdev->dev, dp->freq_table);
}

static struct devfreq_dev_profile hisi_npu_devfreq_profile = {
	/* it would be abnormal to enable devfreq monitor during initialization. */
	.polling_ms     = 20,
	.target         = hisi_npu_devfreq_target,
	.get_dev_status = hisi_npu_get_dev_status,
	.get_cur_freq   = hisi_npu_devfreq_cur_freq,
	.exit           = hisi_npu_devfreq_exit,
};

int hisi_npu_devfreq_init(struct npu_pm_device *pmdev)
{
	struct devfreq_dev_profile *dp = &hisi_npu_devfreq_profile;
	struct device *dev = pmdev->dev;
	struct device_node *np = dev->of_node;
	u32 init_freq = 0;
	struct dev_pm_opp *opp;
	int ret;


	if (!np) {
		dev_err(dev, "npu dts not found\n");
		return -ENODEV;
	}

	if (dev_pm_opp_of_add_table(dev))
		return -EFAULT;

	ret = of_property_read_u32(np, "initial_freq", &init_freq);
	if (ret) {
		dev_err(dev, "parse npu initial frequency fail(%d)\n", ret);
		return -EINVAL;
	}
	dp->initial_freq = (unsigned long)init_freq * KHz;

	/* avoid that the initial_freq from dts is not exist */
	opp = dev_pm_opp_find_freq_ceil(dev, &(dp->initial_freq));
	if (IS_ERR(opp)) {
		dev_warn(dev, "get initial freq from freq table fail\n");
		ret = -EFAULT;
	} else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
		dev_pm_opp_put(opp);
#endif

		dev_set_name(dev, "npufreq");
		pmdev->devfreq = devfreq_add_device(dev, dp, "npu_pm_qos", NULL);
		if (IS_ERR(pmdev->devfreq)) {
			return PTR_ERR(pmdev->devfreq);
		}
		dev_set_name(dev, "npu_pm");

		/* devfreq_add_device only copies a few of npu_pm_dev's fields, so
		 * set drvdata explicitly so other models can access pmdev. */
		dev_set_drvdata(dev, pmdev);
	}

	return ret;
}

void hisi_npu_devfreq_term(struct npu_pm_device *pmdev)
{
	struct device *dev = pmdev->dev;
	int err;

	dev_dbg(dev, "Term npu devfreq\n");

	err = devfreq_remove_device(pmdev->devfreq);
	if (err)
		dev_err(dev, "Failed to terminate devfreq (%d)\n", err);
	else
		pmdev->devfreq = NULL;
}

int hisi_npu_devfreq_suspend(struct devfreq *devfreq)
{
	return devfreq_suspend_device(devfreq);
}

int hisi_npu_devfreq_resume(struct devfreq *devfreq)
{
	return devfreq_resume_device(devfreq);
}
