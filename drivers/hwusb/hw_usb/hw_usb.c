/*
 * hw_usb.c
 *
 * usb driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwusb/hw_usb.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/usb.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_vote.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG hw_usb
HWLOG_REGIST();

#define HW_USB_EVENT_SIZE 32

static struct hw_usb_device *g_hw_usb_di;

static int hw_usb_vote_callback(struct power_vote_object *obj,
	void *data, int result, const char *client_str)
{
	struct hw_usb_device *l_dev = (struct hw_usb_device *)data;

	if (!client_str) {
		hwlog_err("client_str is null\n");
		return -EINVAL;
	}

	if (!l_dev || !l_dev->usb_phy_ldo) {
		hwlog_err("l_dev or usb_phy_ldo is null\n");
		return -ENODEV;
	}

	hwlog_info("result=%d client_str=%s\n", result, client_str);

	if (result)
		regulator_enable(g_hw_usb_di->usb_phy_ldo);
	else
		regulator_disable(g_hw_usb_di->usb_phy_ldo);

	return 0;
}

int hw_usb_ldo_supply_enable(const char *client_name)
{
	if (!client_name)
		return -EINVAL;

	if (!g_hw_usb_di || !g_hw_usb_di->usb_phy_ldo) {
		hwlog_err("g_hw_usb_di or usb_phy_ldo is null\n");
		return -ENODEV;
	}

	return power_vote_set(HW_USB_VOTE_OBJECT, client_name, true, true);
}
EXPORT_SYMBOL_GPL(hw_usb_ldo_supply_enable);

int hw_usb_ldo_supply_disable(const char *client_name)
{
	if (!client_name)
		return -EINVAL;

	if (!g_hw_usb_di || !g_hw_usb_di->usb_phy_ldo) {
		hwlog_err("g_hw_usb_di or usb_phy_ldo is null\n");
		return -ENODEV;
	}

	return power_vote_set(HW_USB_VOTE_OBJECT, client_name, false, false);
}
EXPORT_SYMBOL_GPL(hw_usb_ldo_supply_disable);

void hw_usb_send_event(enum hw_usb_cable_event_type event)
{
	char event_buf[HW_USB_EVENT_SIZE] = {0};
	char *envp[] = { event_buf, NULL };
	int ret;

	if (!g_hw_usb_di || !g_hw_usb_di->child_usb_dev) {
		hwlog_err("g_hw_usb_di or child_usb_dev is null\n");
		return;
	}

	switch (event) {
	case USB31_CABLE_IN_EVENT:
		snprintf(event_buf, sizeof(event_buf), "USB3_STATE=ON");
		break;
	case USB31_CABLE_OUT_EVENT:
		snprintf(event_buf, sizeof(event_buf), "USB3_STATE=OFF");
		break;
	case DP_CABLE_IN_EVENT:
		snprintf(event_buf, sizeof(event_buf), "DP_STATE=ON");
		break;
	case DP_CABLE_OUT_EVENT:
		snprintf(event_buf, sizeof(event_buf), "DP_STATE=OFF");
		break;
	case ANA_AUDIO_IN_EVENT:
		snprintf(event_buf, sizeof(event_buf), "ANA_AUDIO_STATE=ON");
		break;
	case ANA_AUDIO_OUT_EVENT:
		snprintf(event_buf, sizeof(event_buf), "ANA_AUDIO_STATE=OFF");
		break;
	default:
		return;
	}

	ret = kobject_uevent_env(&g_hw_usb_di->child_usb_dev->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_err("uevent sending failed ret=%d\n", ret);
	else
		hwlog_info("sent uevent %s\n", envp[0]);
}

void hw_usb_set_usb_speed(unsigned int usb_speed)
{
	if (!g_hw_usb_di)
		return;

	g_hw_usb_di->speed = usb_speed;
	hwlog_info("usb_speed=%u\n", usb_speed);

	switch (usb_speed) {
	case USB_SPEED_UNKNOWN:
		hw_usb_send_event(USB31_CABLE_OUT_EVENT);
		break;
	case USB_SPEED_SUPER:
	case USB_SPEED_SUPER_PLUS:
		hw_usb_send_event(USB31_CABLE_IN_EVENT);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL_GPL(hw_usb_set_usb_speed);

static unsigned int hw_usb_get_usb_speed(void)
{
	hwlog_info("usb_speed=%u\n", g_hw_usb_di->speed);

	return g_hw_usb_di->speed;
}

static ssize_t hw_usb_speed_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!g_hw_usb_di)
		return  -ENODEV;

	switch (hw_usb_get_usb_speed()) {
	case USB_SPEED_UNKNOWN:
		return scnprintf(buf, PAGE_SIZE, "%s", "unknown");
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
		return scnprintf(buf, PAGE_SIZE, "%s", "full-speed");
	case USB_SPEED_HIGH:
		return scnprintf(buf, PAGE_SIZE, "%s", "high-speed");
	case USB_SPEED_WIRELESS:
		return scnprintf(buf, PAGE_SIZE, "%s", "wireless-speed");
	case USB_SPEED_SUPER:
		return scnprintf(buf, PAGE_SIZE, "%s", "super-speed");
	case USB_SPEED_SUPER_PLUS:
		return scnprintf(buf, PAGE_SIZE, "%s", "super-speed-plus");
	default:
		return scnprintf(buf, PAGE_SIZE, "%s", "unknown");
	}
}

void hw_usb_host_abnormal_event_notify(unsigned int event)
{
	if (!g_hw_usb_di)
		return;

	hwlog_info("event=%u\n", event);
	if ((g_hw_usb_di->abnormal_event == USB_HOST_EVENT_HUB_TOO_DEEP) &&
		(event == USB_HOST_EVENT_UNKNOW_DEVICE))
		g_hw_usb_di->abnormal_event = USB_HOST_EVENT_HUB_TOO_DEEP;
	else
		g_hw_usb_di->abnormal_event = event;
}
EXPORT_SYMBOL_GPL(hw_usb_host_abnormal_event_notify);

static unsigned int usb_host_get_abnormal_event(void)
{
	hwlog_info("abnormal_event=%u\n", g_hw_usb_di->abnormal_event);

	return g_hw_usb_di->abnormal_event;
}

static ssize_t hw_usb_host_abnormal_event_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!g_hw_usb_di)
		return -ENODEV;

	switch (usb_host_get_abnormal_event()) {
	case USB_HOST_EVENT_NORMAL:
		return scnprintf(buf, PAGE_SIZE, "%s", "normal");
	case USB_HOST_EVENT_POWER_INSUFFICIENT:
		return scnprintf(buf, PAGE_SIZE, "%s", "power_insufficient");
	case USB_HOST_EVENT_HUB_TOO_DEEP:
		return scnprintf(buf, PAGE_SIZE, "%s", "hub_too_deep");
	case USB_HOST_EVENT_UNKNOW_DEVICE:
		return scnprintf(buf, PAGE_SIZE, "%s", "unknown_device");
	default:
		return scnprintf(buf, PAGE_SIZE, "%s", "invalid");
	}
}

static DEVICE_ATTR(usb_speed, 0440, hw_usb_speed_show, NULL);
static DEVICE_ATTR(usb_event, 0440, hw_usb_host_abnormal_event_show, NULL);

static struct attribute *hw_usb_ctrl_attributes[] = {
	&dev_attr_usb_speed.attr,
	&dev_attr_usb_event.attr,
	NULL,
};

static const struct attribute_group hw_usb_attr_group = {
	.attrs = hw_usb_ctrl_attributes,
};

static struct device *hw_usb_create_group(void)
{
	return power_sysfs_create_group("hw_usb", "usb", &hw_usb_attr_group);
}

static void hw_usb_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &hw_usb_attr_group);
}

static void hw_usb_parse_dts(struct hw_usb_device *di)
{
	int volt;
	const char *speed = NULL;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG),
		di->dev->of_node, "maximum-speed", &speed))
		return;

	di->usb_phy_ldo = devm_regulator_get(di->dev, "usb_phy_ldo_33v");
	if (IS_ERR(di->usb_phy_ldo)) {
		hwlog_err("usb_phy_ldo_33v regulator dts read failed\n");
		return;
	}

	volt = regulator_get_voltage(di->usb_phy_ldo);
	hwlog_info("usb_phy_ldo_33v=%d\n", volt);
}

static int hw_usb_probe(struct platform_device *pdev)
{
	struct hw_usb_device *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hw_usb_di = di;
	di->pdev = pdev;
	di->dev = &pdev->dev;

	hw_usb_parse_dts(di);
	g_hw_usb_di->child_usb_dev = hw_usb_create_group();
	power_vote_create_object(HW_USB_VOTE_OBJECT, POWER_VOTE_SET_ANY,
		hw_usb_vote_callback, g_hw_usb_di);
	platform_set_drvdata(pdev, di);

	return 0;
}

static int hw_usb_remove(struct platform_device *pdev)
{
	struct hw_usb_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	if (!IS_ERR(di->usb_phy_ldo))
		regulator_put(di->usb_phy_ldo);

	power_vote_destroy_object(HW_USB_VOTE_OBJECT);
	hw_usb_remove_group(g_hw_usb_di->child_usb_dev);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_hw_usb_di->child_usb_dev = NULL;
	g_hw_usb_di = NULL;

	return 0;
}

static const struct of_device_id hw_usb_match_table[] = {
	{
		.compatible = "huawei,huawei_usb",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_usb_driver = {
	.probe = hw_usb_probe,
	.remove = hw_usb_remove,
	.driver = {
		.name = "huawei_usb",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_usb_match_table),
	},
};

static int __init hw_usb_init(void)
{
	return platform_driver_register(&hw_usb_driver);
}

static void __exit hw_usb_exit(void)
{
	platform_driver_unregister(&hw_usb_driver);
}

fs_initcall_sync(hw_usb_init);
module_exit(hw_usb_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei usb module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
