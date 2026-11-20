/*
 * adapter_protocol_pd.c
 *
 * pd protocol driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_pd.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG pd_protocol
HWLOG_REGIST();

static struct pd_protocol_dev *g_pd_protocol_dev;

static const struct adapter_protocol_device_data g_pd_protocol_dev_data[] = {
	{ PROTOCOL_DEVICE_ID_SCHARGER_V600, "scharger_v600" },
	{ PROTOCOL_DEVICE_ID_FUSB3601, "fusb3601" },
	{ PROTOCOL_DEVICE_ID_RT1711H, "rt1711h" },
	{ PROTOCOL_DEVICE_ID_FUSB30X, "fusb30x" },
};

static int pd_protocol_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_pd_protocol_dev_data); i++) {
		if (!strncmp(str, g_pd_protocol_dev_data[i].name,
			strlen(str)))
			return g_pd_protocol_dev_data[i].id;
	}

	return -1;
}

static struct pd_protocol_dev *pd_protocol_get_dev(void)
{
	if (!g_pd_protocol_dev) {
		hwlog_err("g_pd_protocol_dev is null\n");
		return NULL;
	}

	return g_pd_protocol_dev;
}

static struct pd_protocol_ops *pd_protocol_get_ops(void)
{
	if (!g_pd_protocol_dev || !g_pd_protocol_dev->p_ops) {
		hwlog_err("g_pd_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_pd_protocol_dev->p_ops;
}

int pd_protocol_ops_register(struct pd_protocol_ops *ops)
{
	int dev_id;

	if (!g_pd_protocol_dev || !ops || !ops->chip_name) {
		hwlog_err("g_pd_protocol_dev or ops or chip_name is null\n");
		return -1;
	}

	dev_id = pd_protocol_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->chip_name);
		return -1;
	}

	g_pd_protocol_dev->p_ops = ops;
	g_pd_protocol_dev->dev_id = dev_id;

	hwlog_info("%d:%s ops register ok\n", dev_id, ops->chip_name);
	return 0;
}

static int pd_protocol_get_output_voltage(int *volt)
{
	struct pd_protocol_dev *l_dev = NULL;

	l_dev = pd_protocol_get_dev();
	if (!l_dev || !volt)
		return -1;

	*volt = l_dev->volt;
	return 0;
}

static int pd_protocol_set_output_voltage(int volt)
{
	struct pd_protocol_ops *l_ops = NULL;

	l_ops = pd_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->set_output_voltage) {
		hwlog_err("set_output_voltage is null\n");
		return -1;
	}

	hwlog_info("set output voltage: %d\n", volt);
	g_pd_protocol_dev->volt = volt;
	l_ops->set_output_voltage(volt, l_ops->dev_data);
	return 0;
}

static int pd_protocol_hard_reset_master(void)
{
	struct pd_protocol_ops *l_ops = NULL;

	l_ops = pd_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->hard_reset_master) {
		hwlog_err("hard_reset_master is null\n");
		return -1;
	}

	l_ops->hard_reset_master(l_ops->dev_data);
	return 0;
}

static struct adapter_protocol_ops adapter_protocol_pd_ops = {
	.type_name = "hw_pd",
	.hard_reset_master = pd_protocol_hard_reset_master,
	.set_output_voltage = pd_protocol_set_output_voltage,
	.get_output_voltage = pd_protocol_get_output_voltage,
};

static int __init pd_protocol_init(void)
{
	int ret;
	struct pd_protocol_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_pd_protocol_dev = l_dev;
	l_dev->dev_id = PROTOCOL_DEVICE_ID_END;
	l_dev->volt = PD_ADAPTER_9V;

	ret = adapter_protocol_ops_register(&adapter_protocol_pd_ops);
	if (ret)
		goto fail_register_ops;

	return 0;

fail_register_ops:
	kfree(l_dev);
	g_pd_protocol_dev = NULL;
	return ret;
}

static void __exit pd_protocol_exit(void)
{
	if (!g_pd_protocol_dev)
		return;

	kfree(g_pd_protocol_dev);
	g_pd_protocol_dev = NULL;
}

subsys_initcall_sync(pd_protocol_init);
module_exit(pd_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pd protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
