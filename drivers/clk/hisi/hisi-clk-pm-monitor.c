/*
 * hisi-clk-pm-monitor.c
 *
 * Hisilicon clock driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>

static HLIST_HEAD(clocks_pm);
static DEFINE_MUTEX(clocks_pm_lock);

struct pmclk_node {
	const char *name;
	struct hlist_node next_node;
};

void pmclk_monitor_enable(void)
{
	struct pmclk_node *pmclk = NULL;
	struct clk *clk_node = NULL;
	unsigned int ret = 0;

	hlist_for_each_entry(pmclk, &clocks_pm, next_node) {
		clk_node = __clk_lookup(pmclk->name);
		if (IS_ERR_OR_NULL(clk_node)) {
			pr_err("%s get failed!\n", __clk_get_name(clk_node));
			return;
		}
		if (__clk_get_enable_count(clk_node) > 0) {
			pr_err("[%s]:cnt = %u !\n", __clk_get_name(clk_node),
				__clk_get_enable_count(clk_node));
			ret++;
		}
	}
}

static void pmclk_add(struct pmclk_node *clk)
{
	mutex_lock(&clocks_pm_lock);

	hlist_add_head(&clk->next_node, &clocks_pm);

	mutex_unlock(&clocks_pm_lock);
}

static int hisi_pmclk_monitor_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct pmclk_node *pm_clk = NULL;
	int ret;
	unsigned int num, i;

	ret = of_property_read_u32(np, "hisi-pmclk-num", &num);
	if (ret) {
		pr_err("cound not find hisi-pmclk-num property!\n");
		return -EINVAL;
	}
	for (i = 0; i < num; i++) {
		pm_clk = kzalloc(sizeof(*pm_clk), GFP_KERNEL);
		if (pm_clk == NULL) {
			pr_err("[%s] fail to alloc pm_clk!\n", __func__);
			goto out;
		}
		ret = of_property_read_string_index(np, "clock-names", i, &(pm_clk->name));
		if (ret) {
			pr_err("%s:Failed to get clk-names\n", __func__);
			kfree(pm_clk);
			goto out;
		}
		pmclk_add(pm_clk);
		pm_clk = NULL;
	}
out:
	pr_err("pm clk monitor setup!\n");
	return 0;
}

static int hisi_pmclk_monitor_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id hisi_pmclk_of_match[] = {
	{ .compatible = "hisilicon,pm-clk-monitor" },
	{},
};
MODULE_DEVICE_TABLE(of, hisi_pmclk_of_match);

static struct platform_driver hisi_pmclk_monitor_driver = {
	.probe          = hisi_pmclk_monitor_probe,
	.remove         = hisi_pmclk_monitor_remove,
	.driver         = {
		.name   = "hisi_pmclk",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_pmclk_of_match),
	},
};

static int __init hisi_pmclk_monitor_init(void)
{

	return platform_driver_register(&hisi_pmclk_monitor_driver);

}

fs_initcall(hisi_pmclk_monitor_init);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisilicon clock driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
