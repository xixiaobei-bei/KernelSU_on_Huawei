/*
 * hall_interface.c
 *
 * interface for hall driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwsensor/hall/hall_interface.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hall_interface
HWLOG_REGIST();

static struct hall_interface_dev *g_hall_interface_dev;

bool hall_interface_get_hall_status(void)
{
	if (!g_hall_interface_dev || !g_hall_interface_dev->p_ops)
		return false;

	return g_hall_interface_dev->p_ops->get_hall_status();
}

int hall_interface_ops_register(struct hall_interface_ops *ops)
{
	if (!g_hall_interface_dev || !ops) {
		hwlog_err("g_hall_interface or ops is null\n");
		return -1;
	}

	g_hall_interface_dev->p_ops = ops;
	return 0;
}

static int __init hall_interface_init(void)
{
	struct hall_interface_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_hall_interface_dev = l_dev;
	return 0;
}

static void __exit hall_interface_exit(void)
{
	if (!g_hall_interface_dev)
		 return;

	kfree(g_hall_interface_dev);
	g_hall_interface_dev = NULL;
}

subsys_initcall(hall_interface_init);
module_exit(hall_interface_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("interface for hall driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
