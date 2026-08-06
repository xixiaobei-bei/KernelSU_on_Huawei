/*
 * usb_short_circuit_protect_sh.c
 *
 * usb port short circuit protect (control by inputhub) monitor driver
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
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/notifier.h>
#include <linux/wakelock.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/delay.h>
#include <huawei_platform/power/huawei_charger_sh.h>
#include <inputhub_route.h>
#include <inputhub_bridge.h>

#define HWLOG_TAG uscp_sh
HWLOG_REGIST();

struct uscp_device_info_sh *g_device_uscp;

void dsm_uscp_report(uint32_t uscp_id)
{
	power_dsm_dmd_report(POWER_DSM_USCP, uscp_id,
		"uscp happened(sensorhub)\n");
}

static int uscp_parse_dts(struct device_node *np,
	struct uscp_device_info_sh *di)
{
	int ret;

	di->gpio_uscp = of_get_named_gpio(np,
		"gpio_usb_short_circuit_protect", 0);
	hwlog_info("gpio_uscp=%d\n", di->gpio_uscp);

	if (!gpio_is_valid(di->gpio_uscp)) {
		hwlog_err("gpio is not valid\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "adc_channel_uscp",
		&(di->adc_channel_uscp));
	if (ret) {
		hwlog_err("adc_channel_uscp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("adc_channel_uscp=%d\n", di->adc_channel_uscp);

	ret = of_property_read_u32(np, "open_mosfet_temp",
		&(di->open_mosfet_temp));
	if (ret) {
		hwlog_err("open_mosfet_temp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("open_mosfet_temp=%d\n", di->open_mosfet_temp);

	ret = of_property_read_u32(np, "close_mosfet_temp",
		&(di->close_mosfet_temp));
	if (ret) {
		hwlog_err("close_mosfet_temp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("close_mosfet_temp=%d\n", di->close_mosfet_temp);

	ret = of_property_read_u32(np, "interval_switch_temp",
		&(di->interval_switch_temp));
	if (ret) {
		hwlog_err("interval_switch_temp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("interval_switch_temp=%d\n", di->interval_switch_temp);

	return 0;
}

static int uscp_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct uscp_device_info_sh *di = NULL;
	int ret;

	hwlog_info("probe begin\n");

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_device_uscp = di;

	if (!pdev || !(&pdev->dev) || !(&pdev->dev)->of_node) {
		hwlog_err("device_node is null\n");
		ret = -EINVAL;
		goto fail_free_mem;
	}
	np = (&pdev->dev)->of_node;

	ret = uscp_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	kfree(di);
	g_device_uscp = NULL;

	return ret;
}

static const struct of_device_id uscp_match_table[] = {
	{
		.compatible = "huawei,usb_short_circuit_protect_sensorhub",
		.data = NULL,
	},
	{},
};

static struct platform_driver uscp_driver = {
	.probe = uscp_probe,
	.driver = {
		.name = "huawei,usb_short_circuit_protect_sensorhub",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(uscp_match_table),
	},
};

static int __init uscp_init(void)
{
	return platform_driver_register(&uscp_driver);
}

static void __exit uscp_exit(void)
{
	platform_driver_unregister(&uscp_driver);
}

device_initcall_sync(uscp_init);
module_exit(uscp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("usb port short circuit protect module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
