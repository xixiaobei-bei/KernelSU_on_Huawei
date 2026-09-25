/*
 *  Copyright (C) 2017 Hisilicon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/pm_opp.h>
#include <linux/pm_qos.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <hisi_npu_pm.h>
#include "hisi_npu_pm_private.h"


struct npu_pm_device *g_npu_pm_dev = NULL;
DEFINE_MUTEX(power_mutex);

static inline void hisi_npu_devfreq_lock(struct devfreq *df)
{
	if (IS_ERR_OR_NULL(df) == false)
		mutex_lock(&df->lock);
}//lint !e454 !e456

static inline void hisi_npu_devfreq_unlock(struct devfreq *df)
{
	if (IS_ERR_OR_NULL(df) == false) {
		mutex_unlock(&df->lock);//lint !e455
	}
}

int hisi_npu_power_on(void)
{
	struct devfreq *devfreq = NULL;
	ktime_t in_ktime;
	unsigned long delta_time;
	int ret;

	if (IS_ERR_OR_NULL(g_npu_pm_dev)) {
		pr_err("[npupm] npu pm device not exist\n");
		return -ENODEV;
	}
	devfreq = g_npu_pm_dev->devfreq;
	if (IS_ERR_OR_NULL(devfreq)) {
		pr_err("npu pm devfreq devices not exist\n");
		return -ENODEV;
	}
	g_npu_pm_dev->power_on_count++;

	mutex_lock(&power_mutex);

	in_ktime = ktime_get();

	if (g_npu_pm_dev->power_on == true) {
		mutex_unlock(&power_mutex);
		return 0;
	}

	g_npu_pm_dev->last_freq = devfreq->previous_freq;
	g_npu_pm_dev->target_freq = devfreq->previous_freq;

	hisi_npu_devfreq_lock(devfreq);

	ret = hisi_npu_profile_hal_init(g_npu_pm_dev->target_freq);
	if (ret != 0) {
		pr_err("[npupm] Failed to enable\n");
		goto err_power_on;
	}

	if (IS_ERR_OR_NULL(g_npu_pm_dev->regulator)) {
		ret = -ENODEV;
		pr_err("Regulator is null\n");
		goto err_power_on;
	}
	ret = regulator_enable(g_npu_pm_dev->regulator);
	if (ret != 0) {
		pr_err("Failed to enable regulator, ret=%d\n", ret);
		goto err_power_on;
	}

	g_npu_pm_dev->power_on = true;

	if (!IS_ERR_OR_NULL(g_npu_pm_dev->dvfs_data))
		g_npu_pm_dev->dvfs_data->dvfs_enable = true;

	hisi_npu_devfreq_unlock(devfreq);

	/* must out of devfreq lock */
	ret = hisi_npu_devfreq_resume(devfreq);
	if (ret != 0)
		pr_err("Resume device failed, ret=%d!\n", ret);

	delta_time = ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_npu_pm_dev->max_pwron_time)
		g_npu_pm_dev->max_pwron_time = delta_time;

	mutex_unlock(&power_mutex);

	return 0;

err_power_on:
	hisi_npu_devfreq_unlock(devfreq);
	mutex_unlock(&power_mutex);

	return ret;
}

int hisi_npu_power_off(void)
{
	struct devfreq *devfreq = NULL;
	ktime_t in_ktime;
	unsigned long delta_time;
	int ret;

	if (IS_ERR_OR_NULL(g_npu_pm_dev)) {
		pr_err("[npupm] npu pm device not exist\n");
		return -ENODEV;
	}
	devfreq = g_npu_pm_dev->devfreq;
	if (IS_ERR_OR_NULL(devfreq)) {
		pr_err("npu pm devfreq devices not exist\n");
		return -ENODEV;
	}
	g_npu_pm_dev->power_off_count++;

	mutex_lock(&power_mutex);

	in_ktime = ktime_get();

	if (g_npu_pm_dev->power_on == false) {
		mutex_unlock(&power_mutex);
		return 0;
	}

	/* out of devfreq lock */
	ret = hisi_npu_devfreq_suspend(devfreq);
	if (ret != 0)
		pr_err("Suspend device failed, ret=%d!\n", ret);

	hisi_npu_devfreq_lock(devfreq);

	if (IS_ERR_OR_NULL(g_npu_pm_dev->dvfs_data) == false)
		g_npu_pm_dev->dvfs_data->dvfs_enable = false;

	if (IS_ERR_OR_NULL(g_npu_pm_dev->regulator)) {
		ret = -ENODEV;
		pr_err("Regulator is NULL\n");
		goto err_power_off;
	}
	ret = regulator_disable(g_npu_pm_dev->regulator);
	if (ret != 0)
		pr_err("Failed to disable regulator, ret=%d\n", ret);

	hisi_npu_profile_hal_exit();

	g_npu_pm_dev->power_on = false;

err_power_off:
	hisi_npu_devfreq_unlock(devfreq);

	delta_time = ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_npu_pm_dev->max_pwroff_time)
		g_npu_pm_dev->max_pwroff_time = delta_time;

	mutex_unlock(&power_mutex);

	return ret;
}

static int hisi_npu_pm_probe(struct platform_device *pdev)
{
	struct device *dev = &(pdev->dev);
	unsigned int init_freq = 0;
	int ret;

	g_npu_pm_dev = devm_kzalloc(dev, sizeof(*g_npu_pm_dev), GFP_KERNEL);
	if (g_npu_pm_dev == NULL) {
		dev_err(dev, "Failed to allocate npu pm device\n");
		ret = -ENOMEM;
		goto err_out;
	}

	mutex_init(&(g_npu_pm_dev->mutex));

	g_npu_pm_dev->regulator = devm_regulator_get(dev, "npu");
	if (IS_ERR(g_npu_pm_dev->regulator)) {
		dev_err(dev, "get npu regulator fail\n");
		g_npu_pm_dev->regulator = NULL;
		ret = -ENODEV;
		goto err_out;
	}

	ret = of_property_read_u32(dev->of_node, "initial_freq", &init_freq);
	if (ret != 0) {
		dev_err(dev, "parse npu initial frequency fail(%d)\n", ret);
		ret = -EINVAL;
		goto err_out;
	}
	g_npu_pm_dev->last_freq = (unsigned long)init_freq * KHz;
	g_npu_pm_dev->target_freq = g_npu_pm_dev->last_freq;

	g_npu_pm_dev->dev = dev;
	g_npu_pm_dev->pm_qos_class = PM_QOS_HISI_NPU_FREQ_DNLIMIT;
	g_npu_pm_dev->power_on = false;

	ret = hisi_npu_dvfs_init(g_npu_pm_dev);
	if (ret != 0) {
		dev_err(dev, "npu dvfs init fail(%d)\n", ret);
		ret = -EINVAL;
		goto err_out;
	}

	ret = hisi_npu_devfreq_init(g_npu_pm_dev);
	if (ret != 0)
		dev_err(dev, "npu devfreq init fail(%d)\n", ret);

	hisi_npu_pm_debugfs_init(g_npu_pm_dev);

err_out:
	of_node_put(dev->of_node);

	return ret;
}

static int hisi_npu_pm_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id hisi_npu_pm_of_match[] = {
	{.compatible = "hisi,npu-pm",},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_npu_pm_of_match);
#endif

static struct platform_driver hisi_npu_pm_driver = {
	.probe  = hisi_npu_pm_probe,
	.remove = hisi_npu_pm_remove,
	.driver = {
			.name = "hisi-npu-pm",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(hisi_npu_pm_of_match),
		},
};

static int __init hisi_npu_pm_init(void)
{
	return platform_driver_register(&hisi_npu_pm_driver);
}
device_initcall(hisi_npu_pm_init);

static void __exit hisi_npu_pm_exit(void)
{
	hisi_npu_pm_debugfs_exit();

	hisi_npu_devfreq_term(g_npu_pm_dev);

	hisi_npu_dvfs_exit(g_npu_pm_dev);

	platform_driver_unregister(&hisi_npu_pm_driver);
	return;
}
module_exit(hisi_npu_pm_exit);

MODULE_DESCRIPTION("hisi,npu power management");
MODULE_LICENSE("GPL V2");
