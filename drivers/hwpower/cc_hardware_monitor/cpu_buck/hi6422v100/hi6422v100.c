/*
 * hi6422v100.c
 *
 * hi6422v100 cpu buck driver
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

#define HWLOG_TAG cpu_buck_hi6422v100
HWLOG_REGIST();

#define HI6422V100_REG_SIZE   3
#define HI6422V100_PMU_TYPE   "0003_"

static struct cpu_buck_sample g_hi6422v100_cbs;
static char g_hi6422v100_val[HI6422V100_REG_SIZE];
static int g_hi6422v100_flag;

static struct cpu_buck_error_info hi6422v100_error[] = {
	/* irq0 */
	{ HI6422V100_VSYS_UV, 0x40, 0, "HI6422V100_VSYS_UV" },
	{ HI6422V100_VSYS_OV, 0x20, 0, "HI6422V100_VSYS_OV" },
	{ HI6422V100_OTMP_R, 0x10, 0, "HI6422V100_OTMP_R" },
	{ HI6422V100_OTMP150_D10R, 0x08, 0, "HI6422V100_OTMP150_D10R" },
	{ HI6422V100_VBAT2P6_F, 0x04, 0, "HI6422V100_VBAT2P6_F" },
	{ HI6422V100_VBAT2P3_2D, 0x02, 0, "HI6422V100_VBAT2P3_2D" },
	/* ocp_irq0 */
	{ HI6422V100_BUCK34_SCP, 0x40, 1, "HI6422V100_BUCK34_SCP" },
	{ HI6422V100_BUCK012_SCP, 0x20, 1, "HI6422V100_BUCK012_SCP" },
	{ HI6422V100_OCPBUCK4, 0x10, 1, "HI6422V100_OCPBUCK4" },
	{ HI6422V100_OCPBUCK3, 0x08, 1, "HI6422V100_OCPBUCK3" },
	{ HI6422V100_OCPBUCK2, 0x04, 1, "HI6422V100_OCPBUCK2" },
	{ HI6422V100_OCPBUCK1, 0x02, 1, "HI6422V100_OCPBUCK1" },
	{ HI6422V100_OCPBUCK0, 0x01, 1, "HI6422V100_OCPBUCK0" },
	/* ocp_irq1 */
	{ VBAT_OCPBUCK4, 0x10, 2, "VBAT_OCPBUCK4" },
	{ VBAT_OCPBUCK3, 0x08, 2, "VBAT_OCPBUCK3" },
	{ VBAT_OCPBUCK2, 0x04, 2, "VBAT_OCPBUCK2" },
	{ VBAT_OCPBUCK1, 0x02, 2, "VBAT_OCPBUCK1" },
	{ VBAT_OCPBUCK0, 0x01, 2, "VBAT_OCPBUCK0" },
};

static int __init hi6422v100_parse_early_cmdline(char *p)
{
	char *start = NULL;
	int i;

	if (!p) {
		hwlog_err("cmdline is null\n");
		return 0;
	}

	hwlog_info("cpu_buck_reg=%s\n", p);

	start = strstr(p, HI6422V100_PMU_TYPE);
	if (start) {
		g_hi6422v100_flag = 1;

		/* 5: begin fifth index */
		cpu_buck_str_to_reg(start + 5, g_hi6422v100_val,
			HI6422V100_REG_SIZE);
		for (i = 0; i < HI6422V100_REG_SIZE; ++i)
			hwlog_info("reg[%d]=0x%x\n",
				i, g_hi6422v100_val[i]);
	} else {
		g_hi6422v100_flag = 0;
		hwlog_info("no error found\n");
	}

	return 0;
}
early_param("cpu_buck_reg", hi6422v100_parse_early_cmdline);

static int hi6422v100_probe(struct platform_device *pdev)
{
	g_hi6422v100_cbs.cbs = NULL;
	g_hi6422v100_cbs.cbi = hi6422v100_error;
	g_hi6422v100_cbs.reg = g_hi6422v100_val;
	g_hi6422v100_cbs.info_size = ARRAY_SIZE(hi6422v100_error);
	g_hi6422v100_cbs.dev_id = CPU_BUCK_HI6422V100;

	if (g_hi6422v100_flag)
		cpu_buck_register(&g_hi6422v100_cbs);

	return 0;
}

static const struct of_device_id hi6422v100_match_table[] = {
	{
		.compatible = "huawei,hi6422v100",
		.data = NULL,
	},
	{},
};

static struct platform_driver hi6422v100_driver = {
	.probe = hi6422v100_probe,
	.driver = {
		.name = "huawei,hi6422v100",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6422v100_match_table),
	},
};

static int __init hi6422v100_init(void)
{
	return platform_driver_register(&hi6422v100_driver);
}

static void __exit hi6422v100_exit(void)
{
	platform_driver_unregister(&hi6422v100_driver);
}

fs_initcall_sync(hi6422v100_init);
module_exit(hi6422v100_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hi6422v100 cpu buck module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
