/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_mntn_spmi.c
 *
 * Device driver for PMU DRIVER
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hisi_pmic_mntn_inner.h"
#include <linux/mfd/hisi_pmic_mntn.h>
#include <linux/hisi-spmi.h>
#include <linux/version.h>
#include <pr_log.h>
#define PR_LOG_TAG PMIC_MNTN_TAG

#ifdef CONFIG_GCOV_KERNEL
#define STATIC
#else
#define STATIC static
#endif

const static struct of_device_id pmic_mntn_match_tbl[] = {
	{
		.compatible = PMU0_MNTN_COMP_STR,
	},
	{
		.compatible = PMU1_MNTN_COMP_STR,
	},
	{}
};

static int pmic_mntn_spmi_probe(struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pmic_mntn_desc *pmic_mntn = NULL;
	int ret;

	dev_err(dev, "[%s] +\n", __func__);

	pmic_mntn = (struct pmic_mntn_desc *)devm_kzalloc(
		dev, sizeof(*pmic_mntn), GFP_KERNEL);
	if (pmic_mntn == NULL)
		return -ENOMEM;

	pmic_mntn->dev = dev;

	ret = pmic_mntn_drv_init(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_mntn_drv_init failed\n",
			__func__);
		return ret;
	}

	pmic_vsys_surge_init(pmic_mntn);

	dev_err(dev, "[%s] succ\n", __func__);

	return 0;
}

static int pmic_mntn_spmi_remove(struct spmi_device *pdev)
{
	pmic_mntn_drv_deinit();
	return 0;
}

static struct spmi_driver spmi_pmic_mntn_driver = {
	.driver = {
			.name = "hisilicon-hisi-spmi-pmic-mntn",
			.owner = THIS_MODULE,
			.of_match_table = pmic_mntn_match_tbl,
		},
	.probe = pmic_mntn_spmi_probe,
	.remove = pmic_mntn_spmi_remove,
};

STATIC int __init spmi_pmic_mntn_init(void)
{
	int ret;

	pr_info("\n===============[in %s ]=============\n", __func__);

	ret = spmi_driver_register(&spmi_pmic_mntn_driver);
	if (ret)
		pr_err("[%s]spmi_driver_register failed\n", __func__);

	return ret;
}

static void __exit spmi_pmic_mntn_exit(void)
{
	spmi_driver_unregister(&spmi_pmic_mntn_driver);
}

module_init(spmi_pmic_mntn_init);
module_exit(spmi_pmic_mntn_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SPMI PMU MNTN Driver");
