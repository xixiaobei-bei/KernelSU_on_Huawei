/*
 * lp8758.c
 *
 * lp8758 cpu buck driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "../cpu_buck.h"
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cpu_buck_lp8758
HWLOG_REGIST();

#define LP8758_REG_SIZE      2
#define LP8758_PMU_TYPE      "0004_"

static struct cpu_buck_sample g_lp8758_cbs;
static char g_lp8758_val[LP8758_REG_SIZE];
static int g_p8758_flag;

static struct cpu_buck_error_info g_lp8758_error[] = {
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

static int __init lp8758_parse_early_cmdline(char *p)
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
		g_p8758_flag = 1;

		/* 5: begin fifth index */
		cpu_buck_str_to_reg(start + 5, g_lp8758_val,
			LP8758_REG_SIZE);
		for (i = 0; i < LP8758_REG_SIZE; ++i)
			hwlog_info("reg[%d]=0x%x\n",
				i, g_lp8758_val[i]);
	} else {
		g_p8758_flag = 0;
		hwlog_info("no error found\n");
	}

	return 0;
}
early_param("cpu_buck_reg", lp8758_parse_early_cmdline);

static int lp8758_probe(struct platform_device *pdev)
{
	g_lp8758_cbs.cbs = NULL;
	g_lp8758_cbs.cbi = g_lp8758_error;
	g_lp8758_cbs.reg = g_lp8758_val;
	g_lp8758_cbs.info_size = ARRAY_SIZE(g_lp8758_error);
	g_lp8758_cbs.dev_id = CPU_BUCK_LP8758;

	if (g_p8758_flag)
		cpu_buck_register(&g_lp8758_cbs);

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
