/*
 * cmfup_devfreq.c
 *
 * cmfup driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#include <linux/devfreq.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include "governor.h"
#include <securec.h>

#ifdef CONFIG_DRG
#include <linux/drg.h>
#endif

#define DEFAULT_POLLING_INTERVAL_MS 50
#define CMFUP_DEVFREQ_PLATFORM_DEVICE_NAME	"cmfup_devfreq"
#define CMFUP_GOVERNOR_NAME	"cmfup_governor"
#define DEFAULT_MAX_HEAVY_TASKS	0
#define DEFAULT_HEAVY_TASK_LOAD 80
#define DEFAULT_HEAVY_CPU_LOAD	80

struct cmfup_devfreq {
	struct devfreq *devfreq;
	struct platform_device *pdev;
	struct devfreq_dev_profile *df_profile;
	u32 polling_ms;
	unsigned int max_heavy_task;
	unsigned int heavy_task_load;
	unsigned int heavy_cpu_load;
};

static int cmfup_devfreq_get_target_freq(struct devfreq *df,
					 unsigned long *freq)
{
	int count = 0;
	int cpu, cpu_load, task_load, ret;
	struct cmfup_devfreq *cmfup = dev_get_drvdata(df->dev.parent);
	struct devfreq_dev_profile *df_profile = cmfup->df_profile;

	for (cpu = 0; cpu < nr_cpu_ids; cpu++) {
		ret = get_cpu_task_load(cpu, &cpu_load, &task_load);
		if (ret < 0)
			continue;
		if (cpu_load > cmfup->heavy_cpu_load ||
		    task_load > cmfup->heavy_task_load)
			count++;
	}
	if (count > 0 && count <= cmfup->max_heavy_task)
		*freq = df_profile->freq_table[1];
	else
		*freq = 0;
	return 0;
}

static int cmfup_devfreq_event_handler(struct devfreq *devfreq,
				       unsigned int event, void *data)
{
	switch (event) {
	case DEVFREQ_GOV_START:
		devfreq_monitor_start(devfreq);
		break;

	case DEVFREQ_GOV_STOP:
		devfreq_monitor_stop(devfreq);
		break;

	case DEVFREQ_GOV_INTERVAL:
		devfreq_interval_update(devfreq, (unsigned int *)data);
		break;
	default:
		break;
	}

	return 0;
}

static ssize_t heavy_cpu_load_show(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct cmfup_devfreq *cmfup = dev_get_drvdata(devfreq->dev.parent);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%d\n", cmfup->heavy_cpu_load);
}

static ssize_t heavy_cpu_load_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret, value;
	struct devfreq *devfreq = to_devfreq(dev);
	struct cmfup_devfreq *cmfup = dev_get_drvdata(devfreq->dev.parent);

	ret = sscanf_s(buf, "%u", &value);
	if (ret != 1 || value >= 100)
		return -EINVAL;
	mutex_lock(&devfreq->lock);
	cmfup->heavy_cpu_load = value;
	update_devfreq(devfreq);
	mutex_unlock(&devfreq->lock);
	return count;
}

static DEVICE_ATTR(heavy_cpu_load, 0660,
		   heavy_cpu_load_show, heavy_cpu_load_store);
static ssize_t heavy_task_load_show(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct cmfup_devfreq *cmfup = dev_get_drvdata(devfreq->dev.parent);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%d\n", cmfup->heavy_task_load);
}

static ssize_t heavy_task_load_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned int ret, value;
	struct devfreq *devfreq = to_devfreq(dev);
	struct cmfup_devfreq *cmfup = dev_get_drvdata(devfreq->dev.parent);

	ret = sscanf_s(buf, "%u", &value);
	if (ret != 1 || value >= 100)
		return -EINVAL;
	mutex_lock(&devfreq->lock);
	cmfup->heavy_task_load = value;
	update_devfreq(devfreq);
	mutex_unlock(&devfreq->lock);
	return count;
}

static DEVICE_ATTR(heavy_task_load, 0660,
		   heavy_task_load_show, heavy_task_load_store);
static ssize_t max_heavy_task_show(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct cmfup_devfreq *cmfup = dev_get_drvdata(devfreq->dev.parent);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%d\n", cmfup->max_heavy_task);
}

static ssize_t max_heavy_task_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret, value;
	struct devfreq *devfreq = to_devfreq(dev);
	struct cmfup_devfreq *cmfup = dev_get_drvdata(devfreq->dev.parent);

	ret = sscanf_s(buf, "%u", &value);
	if (ret != 1 || value >= nr_cpu_ids)
		return -EINVAL;
	mutex_lock(&devfreq->lock);
	cmfup->max_heavy_task = value;
	update_devfreq(devfreq);
	mutex_unlock(&devfreq->lock);
	return count;
}

static DEVICE_ATTR(max_heavy_task, 0660,
		   max_heavy_task_show, max_heavy_task_store);
static struct attribute *dev_attr[] = {
	&dev_attr_max_heavy_task.attr,
	&dev_attr_heavy_task_load.attr,
	&dev_attr_heavy_cpu_load.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.name = "cmfup",
	.attrs = dev_attr,
};

static struct devfreq_governor cmfup_devfreq_governor = {
	.name = CMFUP_GOVERNOR_NAME,
	.immutable = 1,
	.get_target_freq = cmfup_devfreq_get_target_freq,
	.event_handler = cmfup_devfreq_event_handler,
};

static int cmfup_get_dev_status(struct device *dev,
				struct devfreq_dev_status *stat)
{
	return 0;
}

static int cmfup_target(struct device *dev, unsigned long *_freq, u32 flags)
{
	return 0;
}

static int cmfup_devfreq_set_profile(struct platform_device *pdev)
{
	int ret;
	struct devfreq_dev_profile *df_profile = NULL;
	struct cmfup_devfreq *cmfup = platform_get_drvdata(pdev);

	df_profile = devm_kzalloc(&pdev->dev, sizeof(*df_profile), GFP_KERNEL);
	if (IS_ERR_OR_NULL(df_profile))
		return PTR_ERR(df_profile);

	ret = of_property_read_u32(pdev->dev.of_node, "polling",
				   &cmfup->polling_ms);
	if (ret != 0)
		cmfup->polling_ms = DEFAULT_POLLING_INTERVAL_MS;
	ret = of_property_read_u32(pdev->dev.of_node, "hisi,max_heavy_task",
				   &cmfup->max_heavy_task);
	if (ret != 0)
		cmfup->max_heavy_task = DEFAULT_MAX_HEAVY_TASKS;
	ret = of_property_read_u32(pdev->dev.of_node, "hisi,heavy_task_load",
				   &cmfup->heavy_task_load);
	if (ret != 0)
		cmfup->heavy_task_load = DEFAULT_HEAVY_TASK_LOAD;
	ret = of_property_read_u32(pdev->dev.of_node, "hisi,heavy_cpu_load",
				   &cmfup->heavy_cpu_load);
	if (ret != 0)
		cmfup->heavy_cpu_load = DEFAULT_HEAVY_CPU_LOAD;

	df_profile->polling_ms = cmfup->polling_ms;
	df_profile->get_dev_status = cmfup_get_dev_status;
	df_profile->target = cmfup_target;
	cmfup->df_profile = df_profile;
	return 0;
}

static int cmfup_devfreq_probe(struct platform_device *pdev)
{
	struct cmfup_devfreq *cmfup = NULL;
	struct devfreq_dev_profile *df_profile = NULL;
	int ret;

	dev_err(&pdev->dev, "registering cmfup devfreq.\n");
	cmfup = devm_kzalloc(&pdev->dev, sizeof(*cmfup), GFP_KERNEL);
	if (IS_ERR_OR_NULL(cmfup)) {
		ret = -ENOMEM;
		goto error;
	}
	platform_set_drvdata(pdev, cmfup);
	cmfup->pdev = pdev;
	if (dev_pm_opp_of_add_table(&pdev->dev)) {
		ret = -EINVAL;
		goto error;
	}

	ret = cmfup_devfreq_set_profile(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "set profile error.\n");
		goto remove_table;
	}

	cmfup->devfreq = devfreq_add_device(&pdev->dev, cmfup->df_profile,
					    CMFUP_GOVERNOR_NAME, NULL);
	if (IS_ERR(cmfup->devfreq)) {
		dev_err(&pdev->dev, "add devfreq error.\n");
		goto remove_table;
	}

	ret = sysfs_create_group(&cmfup->devfreq->dev.kobj, &dev_attr_group);
	if (ret != 0) {
		dev_err(&pdev->dev, "sysfs create err %d\n", ret);
		goto remove_devfreq;
	}
#ifdef CONFIG_DRG
	drg_devfreq_register(cmfup->devfreq);
#endif
	return 0;

remove_devfreq:
	devfreq_remove_device(cmfup->devfreq);
remove_table:
	dev_pm_opp_of_remove_table(&pdev->dev);
error:
	dev_err(&pdev->dev, "%s: probe error: %d.\n", __func__, ret);
	return ret;
}

static int cmfup_devfreq_remove(struct platform_device *pdev)
{
	struct cmfup_devfreq *cmfup = platform_get_drvdata(pdev);

	sysfs_remove_group(&cmfup->devfreq->dev.kobj, &dev_attr_group);
#ifdef CONFIG_DRG
	drg_devfreq_unregister(cmfup->devfreq);
#endif
	devfreq_remove_device(cmfup->devfreq);
	dev_pm_opp_of_remove_table(&pdev->dev);
	return 0;
}

static const struct of_device_id cmfup_devfreq_id[] = {
	{.compatible = "hisi,cmfup_devfreq"},
	{}
};

MODULE_DEVICE_TABLE(of, cmfup_devfreq_id);
static struct platform_driver cmfup_devfreq_driver = {
	.probe = cmfup_devfreq_probe,
	.remove = cmfup_devfreq_remove,
	.driver = {
		.name = CMFUP_DEVFREQ_PLATFORM_DEVICE_NAME,
		.of_match_table = cmfup_devfreq_id,
		.owner = THIS_MODULE,
	},
};

static int __init cmfup_devfreq_init(void)
{
	int ret;

	ret = devfreq_add_governor(&cmfup_devfreq_governor);
	if (ret != 0) {
		pr_err("%s: failed to add governor: %d.\n", __func__, ret);
		return ret;
	}

	ret = platform_driver_register(&cmfup_devfreq_driver);
	if (ret != 0) {
		ret = devfreq_remove_governor(&cmfup_devfreq_governor);
		if (ret != 0)
			pr_err("%s: failed to remove governor: %d.\n",
			       __func__, ret);
	}

	return ret;
}

late_initcall(cmfup_devfreq_init);
MODULE_DESCRIPTION("cmfup devfreq driver");
MODULE_LICENSE("GPL v2");
