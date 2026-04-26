/*
 * ovp_switch.c
 *
 * ovp switch to control wired channel
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
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG ovp_chsw
HWLOG_REGIST();

static u32 g_chsw_by_ovp;
static int g_ovp_chsw_count;
static int g_ovp_chsw_en[WIRED_CHANNEL_ALL];
static int g_ovp_chsw_initialized;
static u32 g_ovp_chsw_status = 1; /* default status : main channel is restore */
static u32 g_gpio_low_by_set_input = 1;

static void ovp_chsw_set_wired_channel_status(int channel_type, int flag)
{
	switch (channel_type) {
	case WIRED_CHANNEL_MAIN:
	case WIRED_CHANNEL_AUX:
		if (flag == WIRED_CHANNEL_CUTOFF)
			g_ovp_chsw_status &= ~(BIT(channel_type));
		else
			g_ovp_chsw_status |= BIT(channel_type);
		break;
	case WIRED_CHANNEL_ALL:
		if (flag == WIRED_CHANNEL_CUTOFF)
			g_ovp_chsw_status = 0;
		else
			g_ovp_chsw_status = BIT(WIRED_CHANNEL_MAIN) | BIT(WIRED_CHANNEL_AUX);
		break;
	default:
		break;
	}
}

static int ovp_chsw_get_wired_channel(int channel_type)
{
	u32 flag;

	switch (channel_type) {
	case WIRED_CHANNEL_MAIN:
	case WIRED_CHANNEL_AUX:
		flag = g_ovp_chsw_status & BIT(channel_type);
		break;
	case WIRED_CHANNEL_ALL:
		flag = g_ovp_chsw_status & (BIT(WIRED_CHANNEL_MAIN) | BIT(WIRED_CHANNEL_AUX));
		break;
	default:
		flag = 1; /* restore */
		break;
	}

	if (flag)
		return WIRED_CHANNEL_RESTORE;

	return WIRED_CHANNEL_CUTOFF;
}

static void ovp_chsw_gpio_free(void)
{
	int i;

	for (i = 0; i < g_ovp_chsw_count; i++) {
		if (gpio_is_valid(g_ovp_chsw_en[i]))
			gpio_free(g_ovp_chsw_en[i]);
	}
}

static int ovp_chsw_set_gpio_input(int channel_type)
{
	int i;

	if ((g_ovp_chsw_count <= channel_type) && (channel_type != WIRED_CHANNEL_ALL))
		return 0;

	switch (channel_type) {
	case WIRED_CHANNEL_MAIN:
	case WIRED_CHANNEL_AUX:
		return gpio_direction_input(g_ovp_chsw_en[channel_type]);
	case WIRED_CHANNEL_ALL:
		for (i = 0; i < g_ovp_chsw_count; i++) {
			if (gpio_direction_input(g_ovp_chsw_en[i]) < 0)
				return -1;
		}
		return 0;
	default:
		return 0;
	}
}

static int ovp_chsw_set_gpio_output(int channel_type, unsigned int value)
{
	int i;

	if ((g_ovp_chsw_count <= channel_type) && (channel_type != WIRED_CHANNEL_ALL))
		return 0;

	switch (channel_type) {
	case WIRED_CHANNEL_MAIN:
	case WIRED_CHANNEL_AUX:
		return gpio_direction_output(g_ovp_chsw_en[channel_type], value);
	case WIRED_CHANNEL_ALL:
		for (i = 0; i < g_ovp_chsw_count; i++) {
			if (gpio_direction_output(g_ovp_chsw_en[i], value) < 0)
				return -1;
		}
		return 0;
	default:
		return 0;
	}
}

static int ovp_chsw_set_gpio_low(int channel_type)
{
	if (g_gpio_low_by_set_input)
		return ovp_chsw_set_gpio_input(channel_type); /* restore */

	return ovp_chsw_set_gpio_output(channel_type, 0); /* restore */
}

static int ovp_chsw_set_wired_channel(int channel_type, int flag)
{
	int ret;

	if (!g_ovp_chsw_initialized) {
		hwlog_err("ovp channel switch not initialized\n");
		return -ENODEV;
	}

	if (flag == WIRED_CHANNEL_CUTOFF)
		ret = ovp_chsw_set_gpio_output(channel_type, 1); /* cutoff */
	else
		ret = ovp_chsw_set_gpio_low(channel_type); /* restore */

	hwlog_info("%d switch set en=%d\n", channel_type,
		(flag == WIRED_CHANNEL_CUTOFF) ? 1 : 0);

	ovp_chsw_set_wired_channel_status(channel_type, flag);

	return ret;
}

static struct wired_chsw_device_ops ovp_chsw_ops = {
	.get_wired_channel = ovp_chsw_get_wired_channel,
	.set_wired_channel = ovp_chsw_set_wired_channel,
};

static void ovp_chsw_parse_dts(struct device_node *np)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,wired_channel_switch", "use_ovp_cutoff_wired_channel",
		&g_chsw_by_ovp, 0);
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,wired_channel_switch", "wired_channel_count",
		&g_ovp_chsw_count, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_low_by_set_input", &g_gpio_low_by_set_input, 1);
}

static int ovp_chsw_gpio_init(struct device_node *np)
{
	int ret;

	if (power_gpio_request(np, "gpio_ovp_chsw_en", "gpio_ovp_chsw_en",
		&g_ovp_chsw_en[WIRED_CHANNEL_MAIN]))
		return -1;

	if ((g_ovp_chsw_count == WIRED_CHANNEL_ALL) &&
		(power_gpio_request(np, "gpio_ovp_chsw2_en", "gpio_ovp_chsw2_en",
		&g_ovp_chsw_en[WIRED_CHANNEL_AUX])))
		return -1;

	ret = ovp_chsw_set_gpio_low(WIRED_CHANNEL_MAIN);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		ovp_chsw_gpio_free();
		return -1;
	}

	g_ovp_chsw_initialized = 1;
	return 0;
}

static int ovp_chsw_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	np = pdev->dev.of_node;
	ovp_chsw_parse_dts(np);

	if (g_chsw_by_ovp) {
		ret = ovp_chsw_gpio_init(np);
		if (ret)
			return -1;

		ret = wired_chsw_ops_register(&ovp_chsw_ops);
		if (ret) {
			ovp_chsw_gpio_free();
			return -1;
		}
	}

	return 0;
}

static int ovp_chsw_remove(struct platform_device *pdev)
{
	ovp_chsw_gpio_free();
	return 0;
}

static const struct of_device_id ovp_chsw_match_table[] = {
	{
		.compatible = "huawei,ovp_channel_switch",
		.data = NULL,
	},
	{},
};

static struct platform_driver ovp_chsw_driver = {
	.probe = ovp_chsw_probe,
	.remove = ovp_chsw_remove,
	.driver = {
		.name = "huawei,ovp_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ovp_chsw_match_table),
	},
};

static int __init ovp_chsw_init(void)
{
	return platform_driver_register(&ovp_chsw_driver);
}

static void __exit ovp_chsw_exit(void)
{
	platform_driver_unregister(&ovp_chsw_driver);
}

fs_initcall_sync(ovp_chsw_init);
module_exit(ovp_chsw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ovp switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
