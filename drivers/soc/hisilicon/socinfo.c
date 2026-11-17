/*
 * socinfo.c
 *
 * information about soc info
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "%s: " fmt, __func__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/of_fdt.h>
#include <linux/slab.h>
#include <asm/system_misc.h>
#include <securec.h>

#define SOC_STR_LEN 256

static char lpcpu_soc_str[SOC_STR_LEN];
static bool string_generated;

static char *lpcpu_read_hardware_id(void)
{
	if (string_generated)
		return lpcpu_soc_str;

	return "UNKNOWN SOC TYPE";
}

static int __init socinfo_init(void)
{
	int ret;
	char upper;
	const char *nm = NULL;

	nm = of_flat_dt_get_machine_name();
	if (nm == NULL) {
		pr_err("can't get machine_name\n");
		return -EINVAL;
	}

	upper = toupper(nm[0]);

#ifdef CONFIG_PRODUCT_ARMPC
	ret = sprintf_s(lpcpu_soc_str, SOC_STR_LEN, "%c%s", upper, &nm[1]);
#else
	ret = sprintf_s(lpcpu_soc_str, SOC_STR_LEN, "vendor %c%s", upper, &nm[1]);
#endif
	if (ret < 0)
		return ret;

	arch_read_hardware_id = lpcpu_read_hardware_id;
	string_generated = true;

	return 0;
}

static void __exit socinfo_exit(void)
{
}

module_init(socinfo_init);
module_exit(socinfo_exit);
MODULE_LICENSE("GPL");
