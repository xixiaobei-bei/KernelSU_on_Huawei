/*
 * wireless_keyboard.c
 *
 * wireless keyboard detect driver
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
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <huawei_platform/power/wireless/wireless_keyboard.h>

#define HWLOG_TAG wireless_kb
HWLOG_REGIST();

static int g_wireless_kb_support;
static int g_gpio_kb;

static bool wireless_is_need_kb_detect(void)
{
	/* dts config keyboard_detect */
	if (g_wireless_kb_support) {
		hwlog_info("[%s] keyboard support\n", __func__);
		return true;
	}

	return false;
}

static bool wireless_check_keyboard_status(void)
{
	int gpio_val;

	gpio_val = gpio_get_value(g_gpio_kb);
	hwlog_info("[%s] gpio=%d, val=%d\n", __func__, g_gpio_kb, gpio_val);

	return gpio_val == 0; /* gpio 0 means keyboard on */
}

void wlc_update_kb_control(int status)
{
	if (!wireless_is_need_kb_detect())
		return;

	if (wireless_check_keyboard_status() && (status == WLC_START_CHARING)) {
		hwlog_info("[%s] start wireless charing and kb on\n", __func__);
		wlrx_set_plim_src(WLRX_PLIM_SRC_KB);
	} else {
		hwlog_info("[%s] stop wireless charing or kb not on\n",
			__func__);
		wlrx_clear_plim_src(WLRX_PLIM_SRC_KB);
	}
}

static int wireless_kb_detect_gpio_init(struct device_node *np)
{
	g_gpio_kb = of_get_named_gpio(np, "gpio_wireless_keyboard", 0);
	if (g_gpio_kb > 0) {
		if (!gpio_is_valid(g_gpio_kb)) {
			hwlog_err("%s: gpio is not valid\n", __func__);
			return -1;
		}
		hwlog_info("[%s] gpio=%d\n", __func__, g_gpio_kb);
	}

	return 0;
}

static int wireless_kb_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	np = pdev->dev.of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"keyboard_detect", &g_wireless_kb_support, 0);

	ret = wireless_kb_detect_gpio_init(np);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		g_wireless_kb_support = 0;
		return -1;
	}

	return 0;
}

static int wireless_kb_remove(struct platform_device *pdev)
{
	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	if (!gpio_is_valid(g_gpio_kb))
		gpio_free(g_gpio_kb);

	return 0;
}

static const struct of_device_id wireless_kb_match_table[] = {
	{
		.compatible = "huawei, wireless_keyboard",
		.data = NULL,
	},
	{},
};

static struct platform_driver wireless_kb_driver = {
	.probe = wireless_kb_probe,
	.remove = wireless_kb_remove,
	.driver = {
		.name = "huawei, wireless_keyboard",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wireless_kb_match_table),
	},
};

static int __init wireless_kb_init(void)
{
	return platform_driver_register(&wireless_kb_driver);
}

static void __exit wireless_kb_exit(void)
{
	platform_driver_unregister(&wireless_kb_driver);
}

fs_initcall(wireless_kb_init);
module_exit(wireless_kb_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless keyboard detect driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
