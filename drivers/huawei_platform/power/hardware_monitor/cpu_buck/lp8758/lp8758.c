/*
 * lp8758.c
 *
 * lp8758 cpu buck driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <../cpu_buck.h>

#define HWLOG_TAG lp8758
HWLOG_REGIST();

#define LP8758_REG_SIZE      2
#define LP8758_PMU_TYPE      "0004_"

static struct cpu_buck_sample lp8758;
static char lp8758_reg_val[LP8758_REG_SIZE];
static int lp8758_error_flag;

static struct cpu_buck_error_info lp8758_error_array[] = {
	/* 0x18 */
	{ LP8758_INT_BUCK3, 0x80, 0, "LP8758_INT_BUCK3" },
	{ LP8758_INT_BUCK2, 0x40, 0, "LP8758_INT_BUCK2" },
	{ LP8758_INT_BUCK1, 0x20, 0, "LP8758_INT_BUCK1" },
	{ LP8758_INT_BUCK0, 0x10, 0, "LP8758_INT_BUCK0" },
	{ LP8758_TDIE_SD, 0x08, 0, "LP8758_TDIE_SD" },
	{ LP8758_TDIE_WARN, 0x04, 0, "LP8758_TDIE_WARN" },
	/* 0x19 */
	{ LP8758_BUCK1_ILIM_INT, 0x10, 1, "LP8758_BUCK1_ILIM_INT" },
	{ LP8758_BUCK0_PG_INT, 0x04, 1, "LP8758_BUCK0_PG_INT" },
	{ LP8758_BUCK0_SC_INT, 0x02, 1, "LP8758_BUCK0_SC_INT" },
	{ LP8758_BUCK0_ILIM_INT, 0x01, 1, "LP8758_BUCK0_ILIM_INT" },
};

static int __init early_parse_cpu_buck_reg_cmdline(char *p)
{
	char *start = NULL;
	int i;

	if (!p) {
		hwlog_err("cmdline is null\n");
		return 0;
	}

	hwlog_info("cpu_buck_reg=%s\n", p);

	start = strstr(p, LP8758_PMU_TYPE);
	if (start) {
		lp8758_error_flag = 1;

		/* 5: begin fifth index */
		cpu_buck_str_to_reg(start + 5, lp8758_reg_val,
			LP8758_REG_SIZE);
		for (i = 0; i < LP8758_REG_SIZE; ++i)
			hwlog_info("reg[%d] = 0x%x\n",
				i, lp8758_reg_val[i]);
	} else {
		lp8758_error_flag = 0;
		hwlog_info("lp8758 no error found\n");
	}

	return 0;
}

early_param("cpu_buck_reg", early_parse_cpu_buck_reg_cmdline);

static int lp8758_probe(struct platform_device *pdev)
{
	hwlog_info("probe begin\n");

	lp8758.cbs = NULL;
	lp8758.cbi = lp8758_error_array;
	lp8758.reg = lp8758_reg_val;
	lp8758.info_size = ARRAY_SIZE(lp8758_error_array);
	lp8758.dev_id = CPU_BUCK_LP8758;

	if (lp8758_error_flag == 1)
		cpu_buck_register(&lp8758);

	hwlog_info("probe end\n");
	return 0;
}

static const struct of_device_id lp8758_match_table[] = {
	{
		.compatible = "huawei,lp8758",
		.data = NULL,
	},
	{},
};

static struct platform_driver lp8758_driver = {
	.probe = lp8758_probe,
	.driver = {
		.name = "huawei,lp8758",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lp8758_match_table),
	},
};

static int __init lp8758_init(void)
{
	return platform_driver_register(&lp8758_driver);
}

static void __exit lp8758_exit(void)
{
	platform_driver_unregister(&lp8758_driver);
}

fs_initcall_sync(lp8758_init);
module_exit(lp8758_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("lp8758 cpu buck module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
