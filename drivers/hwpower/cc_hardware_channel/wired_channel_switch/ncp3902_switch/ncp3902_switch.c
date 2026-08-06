/*
 * ncp3902_switch.c
 *
 * ncp3902 switch to control wired channel
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

#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG ncp3902_chsw
HWLOG_REGIST();

static u32 g_chsw_by_ncp3902;
static int g_ncp3902_chsw_en;
static int g_ncp3902_chsw_flag_n;
static int g_ncp3902_chsw_status = WIRED_CHANNEL_RESTORE;

static int ncp3902_chsw_get_wired_channel(int channel_type)
{
	return g_ncp3902_chsw_status;
}

static int ncp3902_chsw_set_wired_channel(int channel_type, int flag)
{
	int gpio_val;

	if (!g_chsw_by_ncp3902 || !g_ncp3902_chsw_en) {
		hwlog_err("ncp3902 channel switch not initialized\n");
		return -ENODEV;
	}

	/* 1:cutoff, 0:restore */
	gpio_val = (flag == WIRED_CHANNEL_CUTOFF) ? 1 : 0;
	gpio_set_value(g_ncp3902_chsw_en, gpio_val);
	g_ncp3902_chsw_status = flag;

	hwlog_info("ncp3902 channel switch set en=%d\n", gpio_val);
	return 0;
}

static int ncp3902_chsw_set_wired_reverse_channel(int flag)
{
	int wired_channel_flag;
	int gpio_val;

	if (!g_ncp3902_chsw_flag_n) {
		hwlog_err("ncp3902 channel switch not initialized\n");
		return -ENODEV;
	}

	wired_channel_flag = ((flag == WIRED_REVERSE_CHANNEL_CUTOFF) ?
		WIRED_CHANNEL_CUTOFF : WIRED_CHANNEL_RESTORE);
	/* 1:mos on, 0:mos off */
	gpio_val = (flag == WIRED_REVERSE_CHANNEL_CUTOFF) ? 0 : 1;
	if (wired_channel_flag == WIRED_CHANNEL_CUTOFF)
		(void)ncp3902_chsw_set_wired_channel(WIRED_CHANNEL_ALL, wired_channel_flag);
	else
		(void)ncp3902_chsw_set_wired_channel(WIRED_CHANNEL_MAIN, wired_channel_flag);
	gpio_set_value(g_ncp3902_chsw_flag_n, gpio_val);

	hwlog_info("ncp3902 channel switch set flag_n=%d:%s\n",
		gpio_val, (gpio_val == 0) ? "high" : "low");
	return 0;
}

static struct wired_chsw_device_ops ncp3902_chsw_ops = {
	.set_wired_channel = ncp3902_chsw_set_wired_channel,
	.get_wired_channel = ncp3902_chsw_get_wired_channel,
	.set_wired_reverse_channel = ncp3902_chsw_set_wired_reverse_channel,
};

static void ncp3902_chsw_parse_dts(struct device_node *np)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,wired_channel_switch",
		"use_wireless_switch_cutoff_wired_channel",
		&g_chsw_by_ncp3902, 0);
}

static int ncp3902_chsw_gpio_init(struct device_node *np)
{
	g_ncp3902_chsw_en = of_get_named_gpio(np, "gpio_chgsw_en", 0);
	hwlog_info("g_ncp3902_chsw_en=%d\n", g_ncp3902_chsw_en);

	if (!gpio_is_valid(g_ncp3902_chsw_en)) {
		hwlog_err("gpio is not valid\n");
		return -EINVAL;
	}

	if (gpio_request(g_ncp3902_chsw_en, "gpio_chgsw_en")) {
		hwlog_err("gpio request fail\n");
		return -ENOMEM;
	}

	/* 0:enable 1:disable */
	gpio_direction_output(g_ncp3902_chsw_en, 0);

	g_ncp3902_chsw_flag_n = of_get_named_gpio(np, "gpio_chgsw_flag_n", 0);
	hwlog_info("g_ncp3902_chsw_flag_n=%d\n", g_ncp3902_chsw_flag_n);

	if (!gpio_is_valid(g_ncp3902_chsw_flag_n)) {
		hwlog_err("gpio is not valid\n");
		g_ncp3902_chsw_flag_n = 0;
		return 0;
	}

	if (gpio_request(g_ncp3902_chsw_flag_n, "gpio_chgsw_flag_n")) {
		hwlog_err("gpio request fail\n");
		return -ENOMEM;
	}

	/* 1:mos on, 0:mos off */
	gpio_direction_output(g_ncp3902_chsw_flag_n, 0);
	return 0;
}

static int ncp3902_chsw_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	np = pdev->dev.of_node;
	ncp3902_chsw_parse_dts(np);

	if (g_chsw_by_ncp3902) {
		ret = ncp3902_chsw_gpio_init(np);
		if (ret)
			return -1;

		ret = wired_chsw_ops_register(&ncp3902_chsw_ops);
		if (ret) {
			gpio_free(g_ncp3902_chsw_en);
			return -1;
		}
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = pdev->dev.driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	return 0;
}

static int ncp3902_chsw_remove(struct platform_device *pdev)
{
	if (gpio_is_valid(g_ncp3902_chsw_en))
		gpio_free(g_ncp3902_chsw_en);

	if (gpio_is_valid(g_ncp3902_chsw_flag_n))
		gpio_free(g_ncp3902_chsw_flag_n);

	return 0;
}

static const struct of_device_id ncp3902_chsw_match_table[] = {
	{
		.compatible = "huawei,ncp3902_channel_switch",
		.data = NULL,
	},
	{},
};

static struct platform_driver ncp3902_chsw_driver = {
	.probe = ncp3902_chsw_probe,
	.remove = ncp3902_chsw_remove,
	.driver = {
		.name = "huawei,ncp3902_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ncp3902_chsw_match_table),
	},
};

static int __init ncp3902_chsw_init(void)
{
	return platform_driver_register(&ncp3902_chsw_driver);
}

static void __exit ncp3902_chsw_exit(void)
{
	platform_driver_unregister(&ncp3902_chsw_driver);
}

fs_initcall_sync(ncp3902_chsw_init);
module_exit(ncp3902_chsw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ncp3902 switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
