/*
 * hi6422v200_aux.c
 *
 * hi6422v200 cpu buck driver
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

#define HWLOG_TAG cpu_buck_hi6422v200_aux
HWLOG_REGIST();

#define HI6422V200_REG_SIZE   3
#define HI6422V200_AUX_TYPE   "0105_"

static struct cpu_buck_sample g_hi6422v200_aux_cbs;
static char g_hi6422v200_aux_val[HI6422V200_REG_SIZE];
static int g_hi6422v200_aux_flag;

static struct cpu_buck_error_info g_hi6422v200_aux_error[] = {
	/* np_irq1_record_reg */
	{ HI6422V200_VSYS_PWRON_D60UR, 0x10, 0, "HI6422V200_VSYS_PWRON" },
	{ HI6422V200_VSYS_OV_D200UR, 0x08, 0, "HI6422V200_VSYS_OV" },
	{ HI6422V200_VSYS_PWROFF_ABS_2D, 0x04, 0, "HI6422V200_VSYS_PWROFF" },
	{ HI6422V200_THSD_OTMP125_D1MR, 0x02, 0, "HI6422V200_THSD_OTMP125" },
	{ HI6422V200_THSD_OTMP140_D180UR, 0x01, 0, "HI6422V200_THSD_OTMP140" },
	/* np_ocp_record_reg */
	{ HI6422V200_BUCK3_OCP, 0x08, 1, "HI6422V200_BUCK3_OCP" },
	{ HI6422V200_BUCK2_OCP, 0x04, 1, "HI6422V200_BUCK2_OCP" },
	{ HI6422V200_BUCK1_OCP, 0x02, 1, "HI6422V200_BUCK1_OCP" },
	{ HI6422V200_BUCK0_OCP, 0x01, 1, "HI6422V200_BUCK0_OCP" },
	/* np_scp_record_reg */
	{ HI6422V200_BUCK3_SCP, 0x08, 2, "HI6422V200_BUCK3_SCP" },
	{ HI6422V200_BUCK2_SCP, 0x04, 2, "HI6422V200_BUCK2_SCP" },
	{ HI6422V200_BUCK1_SCP, 0x02, 2, "HI6422V200_BUCK1_SCP" },
	{ HI6422V200_BUCK0_SCP, 0x01, 2, "HI6422V200_BUCK0_SCP" },
};

static int __init hi6422v200_aux_parse_early_cmdline(char *p)
{
	char *start = NULL;
	int i;

	if (!p) {
		hwlog_err("cmdline is null\n");
		return 0;
	}

	hwlog_info("cpu_buck_reg=%s\n", p);

	start = strstr(p, HI6422V200_AUX_TYPE);
	if (start) {
		g_hi6422v200_aux_flag = 1;

		/* 5: begin fifth index */
		cpu_buck_str_to_reg(start + 5, g_hi6422v200_aux_val,
			HI6422V200_REG_SIZE);
		for (i = 0; i < HI6422V200_REG_SIZE; ++i)
			hwlog_info("reg[%d]=0x%x\n",
				i, g_hi6422v200_aux_val[i]);
	} else {
		g_hi6422v200_aux_flag = 0;
		hwlog_info("no error found\n");
	}

	return 0;
}
early_param("cpu_buck_reg", hi6422v200_aux_parse_early_cmdline);

static int hi6422v200_aux_probe(struct platform_device *pdev)
{
	g_hi6422v200_aux_cbs.cbs = NULL;
	g_hi6422v200_aux_cbs.cbi = g_hi6422v200_aux_error;
	g_hi6422v200_aux_cbs.reg = g_hi6422v200_aux_val;
	g_hi6422v200_aux_cbs.info_size = ARRAY_SIZE(g_hi6422v200_aux_error);
	g_hi6422v200_aux_cbs.dev_id = CPU_BUCK_HI6422V200_AUX;

	if (g_hi6422v200_aux_flag)
		cpu_buck_register(&g_hi6422v200_aux_cbs);

	return 0;
}

static const struct of_device_id hi6422v200_aux_match_table[] = {
	{
		.compatible = "huawei,hi6422v200_pmu2",
		.data = NULL,
	},
	{},
};

static struct platform_driver hi6422v200_aux_driver = {
	.probe = hi6422v200_aux_probe,
	.driver = {
		.name = "huawei,hi6422v200_pmu2",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6422v200_aux_match_table),
	},
};

static int __init hi6422v200_aux_init(void)
{
	return platform_driver_register(&hi6422v200_aux_driver);
}

static void __exit hi6422v200_aux_exit(void)
{
	platform_driver_unregister(&hi6422v200_aux_driver);
}

fs_initcall_sync(hi6422v200_aux_init);
module_exit(hi6422v200_aux_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hi6422v200 cpu buck module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
