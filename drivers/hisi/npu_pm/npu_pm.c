/*
 * npu_pm.c
 *
 * npu pwr and module init
 *
 * Copyright (C) 2017-2020 Huawei Technologies Co., Ltd.
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
#include <npu_pm.h>
#include "npu_pm_private.h"

static struct npu_pm_device *g_npu_pm_dev;
DEFINE_MUTEX(power_mutex);

static inline void npu_pm_devfreq_lock(struct devfreq *df)
{
	if (!IS_ERR_OR_NULL(df))
		mutex_lock(&df->lock);
}

static inline void npu_pm_devfreq_unlock(struct devfreq *df)
{
	if (!IS_ERR_OR_NULL(df))
		mutex_unlock(&df->lock);
}

int npu_pm_power_on(void)
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

	if (g_npu_pm_dev->power_on) {
		mutex_unlock(&power_mutex);
		return 0;
	}

	g_npu_pm_dev->last_freq = devfreq->previous_freq;
	g_npu_pm_dev->target_freq = devfreq->previous_freq;

	npu_pm_devfreq_lock(devfreq);

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

	npu_pm_devfreq_unlock(devfreq);

	/* must out of devfreq lock */
	ret = npu_pm_devfreq_resume(devfreq);
	if (ret != 0)
		pr_err("Resume device failed, ret=%d!\n", ret);

	delta_time = ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_npu_pm_dev->max_pwron_time)
		g_npu_pm_dev->max_pwron_time = delta_time;

	mutex_unlock(&power_mutex);

	return 0;

err_power_on:
	npu_pm_devfreq_unlock(devfreq);
	mutex_unlock(&power_mutex);

	return ret;
}

int npu_pm_power_off(void)
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

	if (!g_npu_pm_dev->power_on) {
		mutex_unlock(&power_mutex);
		return 0;
	}

	/* out of devfreq lock */
	ret = npu_pm_devfreq_suspend(devfreq);
	if (ret != 0)
		pr_err("Suspend device failed, ret=%d!\n", ret);

	npu_pm_devfreq_lock(devfreq);

	if (!IS_ERR_OR_NULL(g_npu_pm_dev->dvfs_data))
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
	npu_pm_devfreq_unlock(devfreq);

	delta_time = ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_npu_pm_dev->max_pwroff_time)
		g_npu_pm_dev->max_pwroff_time = delta_time;

	mutex_unlock(&power_mutex);

	return ret;
}

static int npu_pm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	unsigned int init_freq = 0;
	int ret;

	g_npu_pm_dev = devm_kzalloc(dev, sizeof(struct npu_pm_device), GFP_KERNEL);
	if (g_npu_pm_dev == NULL) {
		dev_err(dev, "Failed to allocate npu pm device\n");
		ret = -ENOMEM;
		goto err_out;
	}

	mutex_init(&g_npu_pm_dev->mutex);

	g_npu_pm_dev->regulator = devm_regulator_get(dev, "npu");
	if (IS_ERR(g_npu_pm_dev->regulator)) {
		dev_err(dev, "get npu regulator fail\n");
		g_npu_pm_dev->regulator = NULL;
		ret = -ENODEV;
		goto err_out;
	}

	ret = of_property_read_u32(dev->of_node, "initial_freq", &init_freq);
	if (ret != 0) {
		dev_err(dev, "parse npu initial frequency fail%d\n", ret);
		ret = -EINVAL;
		goto err_out;
	}
	g_npu_pm_dev->last_freq = (unsigned long)init_freq * KHZ;
	g_npu_pm_dev->target_freq = g_npu_pm_dev->last_freq;

	g_npu_pm_dev->dev = dev;
	g_npu_pm_dev->pm_qos_class = PM_QOS_NPU_FREQ_DNLIMIT;
	g_npu_pm_dev->power_on = false;

	ret = npu_pm_dvfs_init(g_npu_pm_dev);
	if (ret != 0) {
		dev_err(dev, "npu dvfs init fail%d\n", ret);
		ret = -EINVAL;
		goto err_out;
	}

	ret = npu_pm_devfreq_init(g_npu_pm_dev);
	if (ret != 0)
		dev_err(dev, "npu devfreq init fail%d\n", ret);

	npu_pm_debugfs_init(g_npu_pm_dev);

err_out:

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id npu_pm_of_match[] = {
	{
		.compatible = "lpm,npu-pm",
	},
	{},
};

MODULE_DEVICE_TABLE(of, npu_pm_of_match);
#endif

static struct platform_driver npu_pm_driver = {
	.probe  = npu_pm_probe,
	.driver = {
			.name = "lpm-npu-pm",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(npu_pm_of_match),
		},
};

static int __init npu_pm_init(void)
{
	return platform_driver_register(&npu_pm_driver);
}
device_initcall(npu_pm_init);

static void __exit npu_pm_exit(void)
{
	npu_pm_debugfs_exit();

	npu_pm_devfreq_term(g_npu_pm_dev);

	platform_driver_unregister(&npu_pm_driver);
}
module_exit(npu_pm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("npu power manager framework");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
