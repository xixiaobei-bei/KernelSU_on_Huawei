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
#include <linux/devfreq_cooling.h>
#include <hisi_npu_pm.h>
#include "hisi_npu_pm_private.h"

int hisi_npu_recommended_frequency(struct device *dev,
				   unsigned long *target_freq,
				   u32 flags)
{
	struct dev_pm_opp *opp = NULL;
	unsigned long freq;

	if (target_freq == NULL) {
		dev_err(dev, "%s target_freq is null\n", __func__);
		return -EFAULT;
	}

	freq = *target_freq;

	opp = devfreq_recommended_opp(dev, &freq, flags);

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

static int hisi_npu_devfreq_target(struct device *dev,
				   unsigned long *target_freq,
				   u32 flags)
{
	int err;

	err = hisi_npu_recommended_frequency(dev, target_freq, flags);
	if (err != 0) {
		dev_err(dev, "failed to get recommended frequency\n");
		return err;
	}

	err = hisi_npu_dvfs_hal(*target_freq);

	return err;
}

static int hisi_npu_devfreq_cur_freq(struct device *dev, unsigned long *freq)
{
	struct npu_pm_device *pmdev = dev_get_drvdata(dev);
	struct devfreq *df = pmdev->devfreq;

	*freq = df->previous_freq;

	return 0;
}

static int hisi_npu_get_dev_status(struct device *dev,
				   struct devfreq_dev_status *stat)
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
	/*
	 * it would be abnormal to enable devfreq monitor during initialization.
	 */
	.polling_ms     = 20,
	.target         = hisi_npu_devfreq_target,
	.get_dev_status = hisi_npu_get_dev_status,
	.get_cur_freq   = hisi_npu_devfreq_cur_freq,
	.exit           = hisi_npu_devfreq_exit,
};

#ifdef CONFIG_DEVFREQ_THERMAL
static int npu_cooling_get_max_state(struct thermal_cooling_device *cdev,
				     unsigned long *state)
{
	struct npu_pm_device *pmdev = cdev->devdata;

	*state = (pmdev->devfreq->profile->max_state - 1);
	return 0;
}

static int npu_cooling_get_cur_state(struct thermal_cooling_device *cdev,
				     unsigned long *state)
{
	struct npu_pm_device *pmdev = cdev->devdata;

	*state = pmdev->cur_state;
	return 0;
}

static int npu_cooling_set_cur_state(struct thermal_cooling_device *cdev,
				     unsigned long state)
{
	struct npu_pm_device *pmdev = cdev->devdata;

	if (state == pmdev->cur_state)
		return 0;

	if (state > (pmdev->devfreq->profile->max_state - 1)) {
		dev_err(pmdev->dev, "set cur state error:%lu!\n", state);
		return -EINVAL;
	}
	pmdev->cur_state = state;
	return 0;
}

static struct thermal_cooling_device_ops npu_cooling_dev_ops = {
	.get_max_state = npu_cooling_get_max_state,
	.get_cur_state = npu_cooling_get_cur_state,
	.set_cur_state = npu_cooling_set_cur_state,
};
#endif

int hisi_npu_devfreq_init(struct npu_pm_device *pmdev)
{
	struct devfreq_dev_profile *dp = &hisi_npu_devfreq_profile;
	struct device *dev = pmdev->dev;
	struct device_node *np = dev->of_node;
	u32 init_freq = 0;
	struct dev_pm_opp *opp = NULL;
	int ret;

	if (np == NULL) {
		dev_err(dev, "npu dts not found\n");
		return -ENODEV;
	}

	if (dev_pm_opp_of_add_table(dev))
		return -EFAULT;

	ret = of_property_read_u32(np, "initial_freq", &init_freq);
	if (ret != 0) {
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

		/*
		 * devfreq_add_device only copies a few of npu_pm_dev's fields,
		 * so set drvdata explicitly so other models can access pmdev.
		 */
		dev_set_drvdata(dev, pmdev);
		dev_set_name(dev, "npufreq");
		pmdev->devfreq = devfreq_add_device(dev,
						    dp,
						    "npu_pm_qos",
						    NULL);
		if (IS_ERR(pmdev->devfreq))
			return PTR_ERR(pmdev->devfreq);

		dev_set_name(dev, "npu_pm");

#ifdef CONFIG_DEVFREQ_THERMAL
		pmdev->cur_state = pmdev->devfreq->profile->max_state;
		pmdev->devfreq_cooling = thermal_cooling_device_register(
				"npu_pm",
				pmdev,
				&npu_cooling_dev_ops);
		if (IS_ERR_OR_NULL(pmdev->devfreq_cooling)) {
			ret = PTR_ERR(pmdev->devfreq_cooling);
			dev_err(pmdev->dev,
				"Failed to register cooling device (%d)\n",
				ret);
			return ret;
		}
#endif
	}

	return ret;
}

void hisi_npu_devfreq_term(struct npu_pm_device *pmdev)
{
	struct device *dev = pmdev->dev;
	int err;

	err = devfreq_remove_device(pmdev->devfreq);
	if (err != 0)
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
