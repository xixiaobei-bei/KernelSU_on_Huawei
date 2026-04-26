/*
 * devfreq_devbw.c
 *
 * devbw driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "devbw: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/devfreq.h>
#include <linux/of.h>
#include <trace/events/power.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/cpufreq.h>
#include "governor_memlat.h"
#include <linux/version.h>

struct vote_reg {
	void __iomem *reg; /* dev frequency vote register */
	u8 shift;
	u8 width;
	u32 mbits; /* mask bits */
	u32 rsb;
};

struct dev_data {
	unsigned long cur_dev_freq;
	struct vote_reg hw_vote;
	cpumask_t cpus;
	spinlock_t vote_spinlock; /* spinlock for vote */
	bool suspended; /* if suspended do not set freq and votefreq */
	struct devfreq *df;
	struct devfreq_dev_profile dp;
};

#define HISI_VOTE_REG_NUM	2
#define MBITS_LEFT_SHIFT	16
#define VOTE_RSB_DEFAULT	4
#define DEFAULT_POLLING_INTERVAL_MS	50
#define width_to_mask(width)    ((1UL << (width)) - 1)

static void find_freq(struct devfreq_dev_profile *p, unsigned long *freq,
		      u32 flags);

void set_hw_vote_reg(struct vote_reg *hw_vote, unsigned long value)
{
	u32 data = 0;

	value >>= hw_vote->rsb;
	if (value > width_to_mask(hw_vote->width))
		value = width_to_mask(hw_vote->width);

	data |= value << hw_vote->shift;
	data |= hw_vote->mbits;
	writel(data, hw_vote->reg);
}

void set_dev_freq(struct dev_data *d, unsigned long new_freq)
{
	if (new_freq == d->cur_dev_freq || d->suspended == true)
		return;

	set_hw_vote_reg(&d->hw_vote, new_freq / HZ_PER_MHZ);

	d->cur_dev_freq = new_freq;
}

void set_dev_votefreq(struct device *dev, unsigned long new_freq)
{
	struct dev_data *d = dev_get_drvdata(dev);
	unsigned long freq = new_freq;

	spin_lock(&d->vote_spinlock);
	find_freq(&d->dp, &freq, 0);

	if (freq == d->cur_dev_freq || d->suspended == true) {
		spin_unlock(&d->vote_spinlock);
		return;
	}

	set_hw_vote_reg(&d->hw_vote, freq / HZ_PER_MHZ);

	d->cur_dev_freq = freq;
	spin_unlock(&d->vote_spinlock);
}
EXPORT_SYMBOL(set_dev_votefreq);

unsigned long get_dev_votefreq(struct device *dev)
{
	struct dev_data *d = dev_get_drvdata(dev);
	unsigned long freq;

	spin_lock(&d->vote_spinlock);
	freq = d->cur_dev_freq;
	spin_unlock(&d->vote_spinlock);

	return freq;
}
EXPORT_SYMBOL(get_dev_votefreq);

static void find_freq(struct devfreq_dev_profile *p, unsigned long *freq,
		      u32 flags)
{
	int i;
	unsigned long atmost, atleast, f;

	atmost = p->freq_table[0];
	atleast = p->freq_table[p->max_state - 1];
	for (i = 0; i < p->max_state; i++) {
		f = p->freq_table[i];
		if (f <= *freq)
			atmost = max(f, atmost);
		if (f >= *freq)
			atleast = min(f, atleast);
	}

	if (flags & DEVFREQ_FLAG_LEAST_UPPER_BOUND)
		*freq = atmost;
	else
		*freq = atleast;
}

static int devbw_target(struct device *dev, unsigned long *freq, u32 flags)
{
	struct dev_data *d = dev_get_drvdata(dev);

	find_freq(&d->dp, freq, flags);

	spin_lock(&d->vote_spinlock);
	set_dev_freq(d, *freq);
	spin_unlock(&d->vote_spinlock);
	trace_memlat_set_dev_freq(dev_name(dev), "periodic_update",
				  cpumask_any(&d->cpus), 0, 0, *freq);

	return 0;
}

static int devbw_get_dev_status(struct device *dev,
				struct devfreq_dev_status *stat)
{
	return 0;
}

static int get_mask_from_dev_handle(struct platform_device *pdev,
				    cpumask_t *mask)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np = NULL;
	struct device_node *mon_np = NULL;
	struct device_node *tmp_np = NULL;
	int i, cpu;

	cpumask_clear(mask);

	for_each_possible_cpu(cpu) {
		cpu_np = of_get_cpu_node(cpu, NULL);
		if (!cpu_np) {
			pr_err("%s: failed to get cpu%d node\n",
			       __func__, cpu);
			return -ENOENT;
		}

		/* Get memlat monitor descriptor node */
		for (i = 0; ; i++) {
			mon_np = of_parse_phandle(cpu_np, "memlat-monitors", i);
			if (!mon_np)
				break;

			tmp_np = of_parse_phandle(mon_np, "hisi,target-dev", 0);
			/* CPUs are sharing memlat monitor node */
			if (np == tmp_np) {
				cpumask_set_cpu(cpu, mask);
				of_node_put(tmp_np);
				of_node_put(mon_np);
				break;
			}
			of_node_put(tmp_np);
			of_node_put(mon_np);
		}

		of_node_put(cpu_np);
	}

	if (cpumask_empty(mask))
		return -ENOENT;

	return 0;
}

#define PROP_TBL "hisi,freq-tbl"

int devfreq_add_devbw_hw_vote(struct dev_data *d, struct device *dev,
			      void __iomem *reg_base)
{
	struct vote_reg *hw_vote = NULL;
	u32 temp[HISI_VOTE_REG_NUM] = {0};
	u32 reg_mask = 0;
	u32 rsb = 0;

	hw_vote = &d->hw_vote;
	if (of_property_read_u32_array(dev->of_node, "hisi,vote-reg",
				       &temp[0], HISI_VOTE_REG_NUM) != 0) {
		dev_err(dev, "[%s] node %s doesn't have hisi,vote-reg property!\n",
			__func__, dev->of_node->name);
		return -EINVAL;
	}

	hw_vote->reg = reg_base + temp[0];
	hw_vote->shift = ffs(temp[1]) - 1;
	hw_vote->width = fls(temp[1]) - ffs(temp[1]) + 1;

	if (of_property_read_u32(dev->of_node, "hisi,vote-reg-mask",
				 &reg_mask) != 0) {
		hw_vote->mbits = temp[1] << MBITS_LEFT_SHIFT;
		dev_info(dev, "[%s] node %s doesn't have hisi,vote-reg mask property!\n",
			 __func__, dev->of_node->name);
	} else {
		hw_vote->mbits = reg_mask;
	}

	if (of_property_read_u32(dev->of_node, "hisi,vote-value-rsb",
				 &rsb) != 0) {
		hw_vote->rsb = VOTE_RSB_DEFAULT;
		dev_info(dev, "[%s] node %s doesn't have hisi,vote-value-rsb property!\n",
			 __func__, dev->of_node->name);
	} else {
		hw_vote->rsb = rsb;
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
static int devfreq_add_freq_table(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dev_data *d = dev_get_drvdata(dev);
	struct devfreq_dev_profile *p = &d->dp;
	u32 *data = NULL;
	int ret = 0;
	int len, i;

	if (of_find_property(dev->of_node, PROP_TBL, &len) != NULL) {
		len /= sizeof(*data);
		data = devm_kzalloc(dev, len * sizeof(*data), GFP_KERNEL);
		if (data == NULL) {
			dev_err(dev, "fail to alloc mem for freq_table\n");
			ret = -ENOMEM;
			goto error;
		}

		p->freq_table = devm_kzalloc(dev,
					     len * sizeof(*p->freq_table),
					     GFP_KERNEL);
		if (p->freq_table == NULL) {
			dev_err(dev, "fail to alloc mem for freq_table\n");
			ret = -ENOMEM;
			goto error;
		}

		ret = of_property_read_u32_array(dev->of_node, PROP_TBL,
						 data, len);
		if (ret != 0) {
			dev_err(dev, "fail to alloc mem for freq_table\n");
			goto error;
		}

		for (i = 0; i < len; i++)
			p->freq_table[i] = data[i] * HZ_PER_MHZ;

		p->max_state = len;
	}
error:
	return ret;
}
#else
static int devfreq_add_freq_table(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	if (dev_pm_opp_of_add_table(dev)) {
		dev_err(dev, "device add opp table failed.\n");
		return -ENODEV;
	}
	return 0;
}
#endif

int devfreq_add_devbw(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dev_data *d = NULL;
	struct devfreq_dev_profile *p = NULL;
	const char *gov_name = NULL;
	int ret;
	void __iomem *reg_base = NULL;
	const char *name_str = NULL;
	const char *dev_name_str = NULL;

	reg_base = of_iomap(dev->of_node, 0);
	if (reg_base == NULL) {
		dev_err(dev, "fail to map io!\n");
		ret = -ENOMEM;
		goto error;
	}

	d = devm_kzalloc(dev, sizeof(*d), GFP_KERNEL);
	if (!d) {
		dev_err(dev, "fail to alloc mem!\n");
		ret = -ENOMEM;
		goto error;
	}
	dev_set_drvdata(dev, d);

	if (get_mask_from_dev_handle(pdev, &d->cpus)) {
		dev_err(dev, "No CPU use this devbw\n");
		ret = -ENODEV;
		goto error;
	}

	name_str = of_device_get_match_data(dev);
	dev_name_str = dev_name(dev);
	if (name_str != NULL && dev_name_str && strlen(dev_name_str) != 0)
		dev_set_name(dev, "%s%c", name_str, dev_name_str[strlen(dev_name_str) - 1]);
	else
		dev_err(dev, "name_str cannot found!\n");

	spin_lock_init(&d->vote_spinlock);
	d->suspended = false;

	ret = devfreq_add_devbw_hw_vote(d, dev, reg_base);
	if (ret != 0)
		goto error;

	p = &d->dp;
	p->polling_ms = DEFAULT_POLLING_INTERVAL_MS;
	p->target = devbw_target;
	p->get_dev_status = devbw_get_dev_status;

	ret = devfreq_add_freq_table(pdev);
	if (ret != 0)
		goto error;

	if (of_property_read_string(dev->of_node, "governor", &gov_name) != 0)
		gov_name = "mem_latency";

	d->df = devfreq_add_device(dev, p, gov_name, NULL);
	if (IS_ERR(d->df)) {
		dev_err(dev, "fail to register devfreq memlatency\n");
		ret = PTR_ERR(d->df);
		goto error;
	}

	return 0;
error:
	iounmap(reg_base);
	return ret;
}

int devfreq_remove_devbw(struct device *dev)
{
	struct dev_data *d = dev_get_drvdata(dev);
	unsigned long freq = 0;

	find_freq(&d->dp, &freq, 0);

	spin_lock(&d->vote_spinlock);
	set_dev_freq(d, freq);
	spin_unlock(&d->vote_spinlock);

	devfreq_remove_device(d->df);
	return 0;
}

int devfreq_suspend_devbw(struct device *dev)
{
	struct dev_data *d = dev_get_drvdata(dev);
	unsigned long freq = 0;

	find_freq(&d->dp, &freq, 0);

	spin_lock(&d->vote_spinlock);
	set_dev_freq(d, freq);
	d->suspended = true;
	spin_unlock(&d->vote_spinlock);

	return 0;
}

int devfreq_resume_devbw(struct device *dev)
{
	struct dev_data *d = dev_get_drvdata(dev);

	spin_lock(&d->vote_spinlock);
	d->suspended = false;
	spin_unlock(&d->vote_spinlock);

	return 0;
}

static SIMPLE_DEV_PM_OPS(devfreq_devbw_pm, devfreq_suspend_devbw,
			 devfreq_resume_devbw);

static int devfreq_devbw_probe(struct platform_device *pdev)
{
	return devfreq_add_devbw(pdev);
}

static int devfreq_devbw_remove(struct platform_device *pdev)
{
	return devfreq_remove_devbw(&pdev->dev);
}

static const struct of_device_id match_table[] = {
	{ .compatible = "hisi,devbw", .data = "memlat_cpu" },
	{ .compatible = "hisi,l3-devbw", .data = "l3_memlat_cpu"},
	{}
};

static struct platform_driver devbw_driver = {
	.probe = devfreq_devbw_probe,
	.remove = devfreq_devbw_remove,
	.driver = {
		.name = "devbw",
		.pm = &devfreq_devbw_pm,
		.of_match_table = match_table,
		.owner = THIS_MODULE,
	},
};

static int __init devbw_init(void)
{
	platform_driver_register(&devbw_driver);
	return 0;
}
device_initcall(devbw_init);

MODULE_DESCRIPTION("Device DDR Frequency hw-voting driver");
MODULE_LICENSE("GPL v2");

