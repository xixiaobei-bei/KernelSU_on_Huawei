/*
 * Copyright (c) 2009-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*
 * SOC Info Routines
 *
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

static char hisi_soc_str[SOC_STR_LEN];
static bool string_generated;

static char *hisi_read_hardware_id(void)
{
	if (string_generated)
		return hisi_soc_str;

	return "UNKNOWN SOC TYPE";
}

static int __init socinfo_init(void)
{
	int ret;
	char upper;
	const char *nm = NULL;

	nm = of_flat_dt_get_machine_name();
	if (!nm) {
		pr_err("can't get machine_name\n");
		return -EINVAL;
	}

	upper = toupper(nm[0]);

	/*
	 * Hardware:	Hisilicon Kirin970
	 */
	ret = sprintf_s(hisi_soc_str, SOC_STR_LEN,
			"Hisilicon %c%s",
			upper,
			&nm[1]);
	if (ret < 0)
		return ret;

	arch_read_hardware_id = hisi_read_hardware_id;
	string_generated = true;

	return 0;
}

static void __exit socinfo_exit(void)
{
}
module_init(socinfo_init);
module_exit(socinfo_exit);

MODULE_LICENSE("GPL");
