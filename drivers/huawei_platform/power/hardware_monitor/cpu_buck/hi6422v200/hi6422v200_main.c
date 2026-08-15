/*
 * hi6422v200_main.c
 *
 * hi6422v200 cpu buck driver
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

#define HWLOG_TAG hi6422v200_main
HWLOG_REGIST();

#define HI6422V200_REG_SIZE   3
#define HI6422V200_MAIN_TYPE  "0005_"

static struct cpu_buck_sample hi6422v200_main;
static char hi6422v200_main_reg_val[HI6422V200_REG_SIZE];
static int hi6422v200_main_error_flag;

static struct cpu_buck_error_info hi6422v200_main_error_array[] = {
	/* NP_IRQ1_RECORD_REG */
	{ HI6422V200_VSYS_PWRON_D60UR, 0x10, 0, "HI6422V200_VSYS_PWRON" },
	{ HI6422V200_VSYS_OV_D200UR, 0x08, 0, "HI6422V200_VSYS_OV" },
	{ HI6422V200_VSYS_PWROFF_ABS_2D, 0x04, 0, "HI6422V200_VSYS_PWROFF" },
	{ HI6422V200_THSD_OTMP125_D1MR, 0x02, 0, "HI6422V200_THSD_OTMP125" },
	{ HI6422V200_THSD_OTMP140_D180UR, 0x01, 0, "HI6422V200_THSD_OTMP140" },
	/* NP_OCP_RECORD_REG */
	{ HI6422V200_BUCK3_OCP, 0x08, 1, "HI6422V200_BUCK3_OCP" },
	{ HI6422V200_BUCK2_OCP, 0x04, 1, "HI6422V200_BUCK2_OCP" },
	{ HI6422V200_BUCK1_OCP, 0x02, 1, "HI6422V200_BUCK1_OCP" },
	{ HI6422V200_BUCK0_OCP, 0x01, 1, "HI6422V200_BUCK0_OCP" },
	/* NP_SCP_RECORD_REG */
	{ HI6422V200_BUCK3_SCP, 0x08, 2, "HI6422V200_BUCK3_SCP" },
	{ HI6422V200_BUCK2_SCP, 0x04, 2, "HI6422V200_BUCK2_SCP" },
	{ HI6422V200_BUCK1_SCP, 0x02, 2, "HI6422V200_BUCK1_SCP" },
	{ HI6422V200_BUCK0_SCP, 0x01, 2, "HI6422V200_BUCK0_SCP" },
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

	start = strstr(p, HI6422V200_MAIN_TYPE);
	if (start) {
		hi6422v200_main_error_flag = 1;

		/* 5: begin fifth index */
		cpu_buck_str_to_reg(start + 5, hi6422v200_main_reg_val,
			HI6422V200_REG_SIZE);
		for (i = 0; i < HI6422V200_REG_SIZE; ++i)
			hwlog_info("main reg[%d] = 0x%x\n",
				i, hi6422v200_main_reg_val[i]);
	} else {
		hi6422v200_main_error_flag = 0;
		hwlog_err("hi6422v200_main no error found\n");
	}

	return 0;
}

early_param("cpu_buck_reg", early_parse_cpu_buck_reg_cmdline);

static int hi6422v200_main_probe(struct platform_device *pdev)
{
	hwlog_info("probe begin\n");

	hi6422v200_main.cbs = NULL;
	hi6422v200_main.cbi = hi6422v200_main_error_array;
	hi6422v200_main.reg = hi6422v200_main_reg_val;
	hi6422v200_main.info_size = ARRAY_SIZE(hi6422v200_main_error_array);
	hi6422v200_main.dev_id = CPU_BUCK_HI6422V200_MAIN;

	if (hi6422v200_main_error_flag == 1)
		cpu_buck_register(&hi6422v200_main);

	hwlog_info("probe end\n");
	return 0;
}

static const struct of_device_id hi6422v200_main_match_table[] = {
	{
		.compatible = "huawei,hi6422v200_pmu1",
		.data = NULL,
	},
	{},
};

static struct platform_driver hi6422v200_main_driver = {
	.probe = hi6422v200_main_probe,
	.driver = {
		.name = "huawei,hi6422v200_pmu1",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6422v200_main_match_table),
	},
};

static int __init hi6422v200_main_init(void)
{
	return platform_driver_register(&hi6422v200_main_driver);
}

static void __exit hi6422v200_main_exit(void)
{
	platform_driver_unregister(&hi6422v200_main_driver);
}

fs_initcall_sync(hi6422v200_main_init);
module_exit(hi6422v200_main_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hi6422v200 cpu buck module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
