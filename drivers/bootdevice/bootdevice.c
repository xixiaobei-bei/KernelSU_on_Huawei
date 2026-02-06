/*
 * bootdevice.c
 *
 * bootdevice init
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/bootdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define SCINNERSTAT 0x3A0
#define EMMC_UFS_SEL BIT(15)
#define BOOT_EMMC 0
#define BOOT_UFS 1

static int __init bootdevice_init(void)
{
	int err;
	void __iomem *sysctrl = NULL;
	struct device_node *sys_np = NULL;
	struct device_node *boot_np = NULL;
	enum bootdevice_type type;

	sys_np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");

	if (!sys_np) {
		pr_err("can't find sysctrl, get bootdevice failed\n");
		err = -ENXIO;
		goto out;
	}

	sysctrl = of_iomap(sys_np, 0);
	if (!sysctrl) {
		pr_err("sysctrl of_iomap failed, can not get bootdevice type\n");
		err = -ENOMEM;
		goto out;
	}

	boot_np = of_find_compatible_node(NULL, NULL, "hisilicon,bootdevice");
	if (!boot_np) {
		pr_err("can't find bootdevice dts node\n");
		err = -ENODEV;
		goto sys_unmap;
	}

	if (of_find_property(boot_np, "boot-from-emmc", NULL))
		type = BOOT_DEVICE_EMMC;
	else if (of_find_property(boot_np, "boot-from-ufs", NULL))
		type = BOOT_DEVICE_UFS;
	else
		type =
		    ((readl(sysctrl + SCINNERSTAT) & EMMC_UFS_SEL) == BOOT_EMMC)
			? BOOT_DEVICE_EMMC
			: BOOT_DEVICE_UFS;

	set_bootdevice_type(type);
	pr_info("storage bootdevice: %s\n",
		type == BOOT_DEVICE_EMMC ? "eMMC" : "UFS");
	err = 0;

sys_unmap:
	iounmap(sysctrl);
out:
	return err;
}
arch_initcall(bootdevice_init);
