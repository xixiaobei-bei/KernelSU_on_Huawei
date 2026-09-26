/*
 * direct_charge_power_supply.c
 *
 * power supply for direct charge
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
#include <linux/delay.h>
#include <linux/gpio.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#ifdef CONFIG_USB_AUDIO_POWER
#include <huawei_platform/audio/usb_audio_power.h>
#endif
#ifdef CONFIG_DIG_HS_POWER
#include <huawei_platform/audio/dig_hs_power.h>
#endif
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_adapter.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_ps
HWLOG_REGIST();

static u32 g_scp_ps_by_5vboost;
static u32 g_scp_ps_by_charger;

static u32 g_is_need_bst_ctrl;
static int g_bst_ctrl;
static u32 g_bst_ctrl_use_common_gpio;

int dc_set_bst_ctrl(int enable)
{
	int ret = 0;

	if (!g_is_need_bst_ctrl)
		return 0;

	if (!g_bst_ctrl_use_common_gpio)
		return gpio_direction_output(g_bst_ctrl, enable);

#ifdef CONFIG_USB_AUDIO_POWER
	if (bst_ctrl_enable(enable, VBOOST_CONTROL_PM))
		ret = -1;
#endif /* CONFIG_USB_AUDIO_POWER */
#ifdef CONFIG_DIG_HS_POWER
	if (dig_hs_bst_ctrl_enable(enable, DIG_HS_VBOOST_CONTROL_PM))
		ret = -1;
#endif /* CONFIG_DIG_HS_POWER */

	return ret;
}

static int scp_ps_enable_by_5vboost(int enable)
{
	int ret = 0;

	hwlog_info("by_5vboost=%u,%u,%u, enable=%d\n",
		g_scp_ps_by_5vboost, g_is_need_bst_ctrl, g_bst_ctrl_use_common_gpio,
		enable);

	if (!g_scp_ps_by_5vboost)
		return 0;

	if (boost_5v_enable(enable, BOOST_CTRL_DC))
		ret = -1;

	if (dc_set_bst_ctrl(enable))
		ret = -1;

	return ret;
}

static int scp_ps_enable_by_charger(int enable)
{
	hwlog_info("by_charger=%u, enable=%d\n", g_scp_ps_by_charger, enable);

	if (!g_scp_ps_by_charger)
		return 0;

	if (enable)
		vbus_ch_open(VBUS_CH_USER_DC, VBUS_CH_TYPE_CHARGER, false);
	else
		vbus_ch_close(VBUS_CH_USER_DC, VBUS_CH_TYPE_CHARGER, false, false);

	return 0;
}

static int scp_ps_enable_by_dummy(int enable)
{
	hwlog_info("by_dummy=dummy, enable=%d\n", enable);
	return 0;
}

static struct dc_pps_ops scp_ps_dummy_ops = {
	.enable = scp_ps_enable_by_dummy,
};

static struct dc_pps_ops scp_ps_5vboost_ops = {
	.enable = scp_ps_enable_by_5vboost,
};

static struct dc_pps_ops scp_ps_charger_ops = {
	.enable = scp_ps_enable_by_charger,
};

static int dc_ps_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	np = pdev->dev.of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_5v_support_scp_power", &g_scp_ps_by_5vboost, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"huawei_charger_support_scp_power", &g_scp_ps_by_charger, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_need_bst_ctrl", &g_is_need_bst_ctrl, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bst_ctrl_use_common_gpio", &g_bst_ctrl_use_common_gpio, 0);

	if (g_is_need_bst_ctrl && !g_bst_ctrl_use_common_gpio) {
		if (power_gpio_request(np, "bst_ctrl", "bst_ctrl", &g_bst_ctrl))
			return -1;
	}

	if (g_scp_ps_by_5vboost)
		(void)dc_pps_ops_register(&scp_ps_5vboost_ops);
	else if (g_scp_ps_by_charger)
		(void)dc_pps_ops_register(&scp_ps_charger_ops);
	else
		(void)dc_pps_ops_register(&scp_ps_dummy_ops);

	return 0;
}

static int dc_ps_remove(struct platform_device *pdev)
{
	if (g_is_need_bst_ctrl && !g_bst_ctrl_use_common_gpio)
		gpio_free(g_bst_ctrl);

	return 0;
}

static const struct of_device_id dc_ps_match_table[] = {
	{
		.compatible = "huawei,direct_charge_ps",
		.data = NULL,
	},
	{},
};

static struct platform_driver dc_ps_driver = {
	.probe = dc_ps_probe,
	.remove = dc_ps_remove,
	.driver = {
		.name = "huawei,direct_charge_ps",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dc_ps_match_table),
	},
};

static int __init dc_ps_init(void)
{
	return platform_driver_register(&dc_ps_driver);
}

static void __exit dc_ps_exit(void)
{
	platform_driver_unregister(&dc_ps_driver);
}

device_initcall_sync(dc_ps_init);
module_exit(dc_ps_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charge power supply module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
