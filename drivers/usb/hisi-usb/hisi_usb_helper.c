/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Description: driver for usb query info
 * Create: 2019-01-31
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>

static const char * const chip_usb_state_names[] = {
	[USB_STATE_UNKNOWN] = "USB_STATE_UNKNOWN",
	[USB_STATE_OFF] = "USB_STATE_OFF",
	[USB_STATE_DEVICE] = "USB_STATE_DEVICE",
	[USB_STATE_HOST] = "USB_STATE_HOST",
	[USB_STATE_HIFI_USB] = "USB_STATE_HIFI_USB",
	[USB_STATE_HIFI_USB_HIBERNATE] = "USB_STATE_HIFI_USB_HIBERNATE",
#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
	[USB_STATE_AP_USE_HIFIUSB] = "USB_STATE_AP_USE_HIFIUSB",
#endif
};

const char *chip_usb_state_string(enum usb_state state)
{
	if (state >= USB_STATE_ILLEGAL)
		return "illegal state";

	return chip_usb_state_names[state];
}

static const char * const charger_type_array[] = {
	[CHARGER_TYPE_SDP]     = "sdp",       /* Standard Downstreame Port */
	[CHARGER_TYPE_CDP]     = "cdp",       /* Charging Downstreame Port */
	[CHARGER_TYPE_DCP]     = "dcp",       /* Dedicate Charging Port */
	[CHARGER_TYPE_UNKNOWN] = "unknown",   /* non-standard */
	[CHARGER_TYPE_NONE]    = "none",      /* not connected */
	[PLEASE_PROVIDE_POWER] = "provide"   /* host mode, provide power */
};

const char *charger_type_string(enum hisi_charger_type type)
{
	if (type >= CHARGER_TYPE_ILLEGAL)
		return "illegal charger";

	return charger_type_array[type];
}

enum hisi_charger_type get_charger_type_from_str(const char *buf, size_t size)
{
	unsigned int i;
	enum hisi_charger_type ret = CHARGER_TYPE_NONE;

	if (!buf)
		return ret;

	for (i = 0; i < ARRAY_SIZE(charger_type_array); i++) {
		if (!strncmp(buf, charger_type_array[i], size - 1)) {
			ret = (enum hisi_charger_type)i;
			break;
		}
	}

	return ret;
}

static const char * const speed_names[] = {
	[USB_SPEED_UNKNOWN] = "UNKNOWN",
	[USB_SPEED_LOW] = "low-speed",
	[USB_SPEED_FULL] = "full-speed",
	[USB_SPEED_HIGH] = "high-speed",
	[USB_SPEED_WIRELESS] = "wireless",
	[USB_SPEED_SUPER] = "super-speed",
	[USB_SPEED_SUPER_PLUS] = "super-speed-plus",
};

enum usb_device_speed usb_speed_to_string(const char *maximum_speed,
	size_t len)
{
	/* default speed is SPEER_SPEED, if input is err, return max_speed */
	enum usb_device_speed speed = USB_SPEED_SUPER;
	unsigned int i;
	size_t actual;

	if (!maximum_speed || len == 0)
		return speed;

	for (i = 0; i < ARRAY_SIZE(speed_names); i++) {
		actual =
			strlen(speed_names[i]) < len ? strlen(speed_names[i]) : len;
		if (strncmp(maximum_speed, speed_names[i], actual) == 0) {
			speed = (enum usb_device_speed)i;
			break;
		}
	}

	return speed;
}

void __iomem *of_devm_ioremap(struct device *dev, const char *compatible)
{
	struct resource res;
	struct device_node *np = NULL;

	if (!dev || !compatible)
		return IOMEM_ERR_PTR(-EINVAL);

	np = of_find_compatible_node(NULL, NULL, compatible);
	if (!np) {
		pr_err("[CHIP_USB][%s]get %s failed!\n", __func__, compatible);
		return IOMEM_ERR_PTR(-EINVAL);
	}

	if (of_address_to_resource(np, 0, &res))
		return IOMEM_ERR_PTR(-EINVAL);

	return devm_ioremap_resource(dev, &res);
}

int chip_usb_get_clks(struct device_node *node, struct clk ***clks, int *num_clks)
{
	struct property *prop = NULL;
	const char *name = NULL;
	struct clk *clk = NULL;
	int i = 0;

	if (!node || !clks || !num_clks)
		return -EINVAL;

	*num_clks = of_property_count_strings(node, "clock-names");
	if (*num_clks < 1 || *num_clks > CHIP_USB_GET_CLKS_MAX_NUM) {
		*num_clks = 0;
		return -EINVAL;
	}

	/* clks is struct clk* point array */
	*clks = kcalloc(*num_clks, sizeof(struct clk *), GFP_KERNEL);
	if (!(*clks)) {
		*num_clks = 0;
		return -ENOMEM;
	}

	of_property_for_each_string(node, "clock-names", prop, name) {
		if (i == *num_clks) {
			pr_err("[CHIP_USB][%s]:clk number error?\n", __func__);
			break;
		}
		clk = of_clk_get_by_name(node, name);
		if (IS_ERR(clk)) {
			pr_err("[CHIP_USB][%s]:failed to get %s\n", __func__, name);
			return PTR_ERR(clk);
		}

		(*clks)[i] = clk;
		++i;
	}

	return 0;
}

int chip_usb_put_clks(struct clk **clks, int num_clks)
{
	if (!clks || num_clks == 0)
		return -EINVAL;

	kfree(clks);

	return 0;
}

int devm_chip_usb_get_clks(struct device *dev, struct clk ***clks, int *num_clks)
{
	struct device_node *np = NULL;
	struct property *prop = NULL;
	const char *name = NULL;
	struct clk *clk = NULL;
	int i;

	if (!dev || !clks || !num_clks)
		return -EINVAL;

	np = dev->of_node;

	*num_clks = of_property_count_strings(dev->of_node, "clock-names");
	if (*num_clks < 1 || *num_clks > CHIP_USB_GET_CLKS_MAX_NUM) {
		*num_clks = 0;
		return -EINVAL;
	}

	/* clks is struct clk* point array */
	*clks = devm_kcalloc(dev, *num_clks, sizeof(struct clk *), GFP_KERNEL);
	if (!(*clks)) {
		*num_clks = 0;
		return -ENOMEM;
	}

	i = 0;
	of_property_for_each_string(dev->of_node, "clock-names", prop, name) {
		if (i == *num_clks) {
			pr_err("[CHIP_USB][%s]:clk number error?\n", __func__);
			break;
		}
		clk = devm_clk_get(dev, name);
		if (IS_ERR(clk)) {
			pr_err("[CHIP_USB][%s]:failed to get %s\n", __func__, name);
			return PTR_ERR(clk);
		}

		(*clks)[i] = clk;
		++i;
	}

	return 0;
}

int chip_usb_init_clks(struct clk **clks, int num_clks)
{
	int i;
	int ret;

	if (!clks)
		return -EINVAL;

	for (i = 0; i < num_clks; i++) {
		ret = clk_prepare_enable(clks[i]);
		if (ret) {
			pr_err("[CHIP_USB][%s]enable clk failed", __func__);
			while (i--)
				clk_disable_unprepare(clks[i]);
			return ret;
		}
	}

	return 0;
}

void chip_usb_shutdown_clks(struct clk **clks, int num_clks)
{
	int i;

	if (!clks)
		return;

	for (i = 0; i < num_clks; i++)
		clk_disable_unprepare(clks[i]);
}

