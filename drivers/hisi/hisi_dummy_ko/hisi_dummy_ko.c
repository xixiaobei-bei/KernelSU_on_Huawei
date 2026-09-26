/*
 * hisi_dummy_ko.c
 *
 * for vts ProcModulesTest, which parsing /proc/modules,
 * require at least one entry.
 *
 * Copyright (c) 2001-2020 Huawei Technologies Co., Ltd.
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
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init hisi_dummy_ko_init(void)
{
	return 0;
}

static void __exit hisi_dummy_ko_exit(void)
{
}

module_init(hisi_dummy_ko_init);
module_exit(hisi_dummy_ko_exit);
