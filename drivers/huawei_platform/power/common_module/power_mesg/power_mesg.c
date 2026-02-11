/*
 * power_mesg.c
 *
 * power netlink message source file
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/power_mesg.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG power_mesg
HWLOG_REGIST();

static struct device *power_mesg_dev;

int power_easy_send(struct power_mesg_node *node, unsigned char cmd,
	unsigned char version, void *data, unsigned int len)
{
	return power_genl_easy_send(node, cmd, version, data, len);
}

int power_easy_node_register(struct power_mesg_node *node)
{
	return power_genl_easy_node_register(node);
}

static struct device *power_mesg_get_device(void)
{
	struct class *power_class = NULL;

	if (!power_mesg_dev) {
		power_class = hw_power_get_class();
		if (power_class)
			power_mesg_dev = device_create(power_class, NULL, 0,
				NULL, "power_mesg");
	}

	return power_mesg_dev;
}

int power_mesg_create_attr(const struct device_attribute *attr)
{
	struct device *dev = NULL;

	dev = power_mesg_get_device();
	if (dev)
		return device_create_file(dev, attr);

	hwlog_err("power mesg device create failed\n");
	return -1;
}

static int __init power_msg_init(void)
{
	hwlog_info("probe begin\n");

	if (power_genl_init())
		hwlog_info("power genl init failed\n");

	hwlog_info("probe end\n");
	return 0;
}

static void __exit power_msg_exit(void)
{
	hwlog_info("exit\n");
}

late_initcall(power_msg_init);
module_exit(power_msg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("netlink message for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
