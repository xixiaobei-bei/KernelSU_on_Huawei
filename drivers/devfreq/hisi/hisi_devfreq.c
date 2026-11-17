/*
 * hisi_devfreq.c
 *
 * devfreq for ddr
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/device.h>
#include <linux/module.h>
#include <linux/hisi/hisi_devfreq.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>

int hisi_devfreq_init_freq_table(struct device *dev, unsigned long **table)
{
	struct dev_pm_opp *opp = NULL;
	unsigned long *freq_table = NULL;
	unsigned long freq = 0;
	unsigned int i = 0;
	int opp_count;

	if (dev == NULL || table == NULL)
		return -ENOMEM;

	opp_count = dev_pm_opp_get_opp_count(dev);
	if (opp_count <= 0)
		return -ENOMEM;

	freq_table = kzalloc(sizeof(unsigned long) * opp_count, GFP_KERNEL);
	if (freq_table == NULL) {
		dev_warn(dev, "%s: Unable to allocate frequency table\n",
			 __func__);
		return -ENOMEM;
	}

	rcu_read_lock();
	do {
		opp = dev_pm_opp_find_freq_ceil(dev, &freq);
		if (IS_ERR(opp))
			break;

		freq_table[i++] = freq;
		freq++;
	} while (1);
	rcu_read_unlock();

	*table = freq_table;

	return 0;
}
EXPORT_SYMBOL(hisi_devfreq_init_freq_table);

int hisi_devfreq_free_freq_table(struct device *dev, unsigned long **table)
{
	if (table == NULL || (*table) == NULL)
		return -ENOMEM;

	kfree(*table);
	*table = NULL;

	return 0;
}
EXPORT_SYMBOL(hisi_devfreq_free_freq_table);
