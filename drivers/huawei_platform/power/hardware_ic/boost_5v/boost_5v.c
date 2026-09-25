/*
 * boost_5v.c
 *
 * boost with 5v driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <huawei_platform/power/power_sysfs.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/power/power_gpio.h>
#include <huawei_platform/power/power_cmdline.h>
#include <media/huawei/camera/pmic/hw_pmic.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG boost_5v
HWLOG_REGIST();

static struct device *g_boost_5v_dev;
static int gpio_5v_boost;
static unsigned int boost_status;
static DEFINE_MUTEX(boost_op_mutex);
static int boost_initialized;
static u32 boost_5v_use_common_pmic;

static const char * const g_boost_ctrl_source_table[BOOST_CTRL_END] = {
	[BOOST_CTRL_BOOST_GPIO_OTG] = "BOOST_GPIO_OTG",
	[BOOST_CTRL_PD_VCONN] = "PD",
	[BOOST_CTRL_DC] = "DC",
	[BOOST_CTRL_MOTOER] = "MOTOR",
	[BOOST_CTRL_AUDIO] = "AUDIO",
	[BOOST_CTRL_AT_CMD] = "AT_CMD",
	[BOOST_CTRL_FCP] = "FCP",
	[BOOST_CTRL_WLDC] = "WLDC",
	[BOOST_CTRL_WLTX] = "WLTX",
	[BOOST_CTRL_WLC] = "WIRELESS_CHARGE",
	[BOOST_CTRL_CAMERA] = "CAMERA",
	[BOOST_CTRL_NFC] = "NFC",
	[BOOST_CTRL_SC_CHIP] = "SC_CHIP",
};

static const char *boost_5v_get_ctrl_source(unsigned int type)
{
	if ((type >= BOOST_CTRL_BEGIN) && (type < BOOST_CTRL_END))
		return g_boost_ctrl_source_table[type];

	return "illegal type";
}

static int boost_5v_output(int value)
{
	int ret;

	if (boost_5v_use_common_pmic) {
		ret = pmic_enable_boost(value);
		if (ret) {
			hwlog_err("pmic enable boost fail\n");
			return -1;
		}
	} else {
		gpio_set_value(gpio_5v_boost, value);
	}

	return 0;
}

static int boost_set_enable(unsigned int type)
{
	if (boost_status == 0)
		boost_5v_output(BOOST_5V_ENABLE);

	boost_status = boost_status | (1 << type);

	hwlog_info("%s enable boost_5v success\n",
		boost_5v_get_ctrl_source(type));
	return 0;
}

static int boost_set_disable(unsigned int type)
{
	if (boost_status != 0) {
		boost_status = boost_status & (~(1 << type));

		if (boost_status == 0)
			boost_5v_output(BOOST_5V_DISABLE);
	}

	hwlog_info("%s disable boost_5v success\n",
		boost_5v_get_ctrl_source(type));
	return 0;
}

int boost_5v_enable(bool enable, unsigned int type)
{
	hwlog_info("module=%s set boost_5v to enable=%d\n",
		boost_5v_get_ctrl_source(type), enable);

	if (!(boost_initialized || boost_5v_use_common_pmic)) {
		hwlog_err("boost 5v not initialized\n");
		return -EINVAL;
	}

	if (type >= BOOST_CTRL_END) {
		hwlog_err("invalid type=%d\n", type);
		return -EINVAL;
	}

	mutex_lock(&boost_op_mutex);

	if (enable)
		boost_set_enable(type);
	else
		boost_set_disable(type);

	mutex_unlock(&boost_op_mutex);

	return 0;
}

unsigned int boost_5v_status(void)
{
	return boost_status;
}

static int boost_5v_gpio_init(struct device_node *np)
{
	if (power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_5v_boost", "gpio_5v_boost", &gpio_5v_boost, 0))
		return -1;

	boost_initialized = 1;
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t boost_5v_enable_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", boost_status);
}

static ssize_t boost_5v_enable_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;

	if (!power_cmdline_is_factory_mode()) {
		hwlog_info("only factory version can ctrl boost 5v\n");
		return count;
	}

	/*
	 * 10: decimal
	 * 0: disable; 1: enable; others: invaid
	 */
	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1)) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (val) {
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_AT_CMD);
	} else {
		mutex_lock(&boost_op_mutex);
		boost_5v_output(BOOST_5V_DISABLE);
		boost_status = 0;
		mutex_unlock(&boost_op_mutex);
	}

	hwlog_info("ctrl boost 5v by sys class\n");
	return count;
}

static DEVICE_ATTR(enable, 0640,
	boost_5v_enable_sysfs_show, boost_5v_enable_sysfs_store);

static struct attribute *boost_5v_attributes[] = {
	&dev_attr_enable.attr,
	NULL,
};

static const struct attribute_group boost_5v_attr_group = {
	.attrs = boost_5v_attributes,
};

static struct device *boost_5v_sysfs_create_group(void)
{
	return power_sysfs_create_group("hw_power", "boost_5v",
		&boost_5v_attr_group);
}

static void boost_5v_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &boost_5v_attr_group);
}
#else
static inline struct device *boost_5v_sysfs_create_group(void)
{
	return 0;
}

static inline void boost_5v_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int boost_5v_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	np = pdev->dev.of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_5v_use_common_pmic", &boost_5v_use_common_pmic, 0);
	if (!boost_5v_use_common_pmic) {
		ret = boost_5v_gpio_init(np);
		if (ret)
			return -1;
	}

	g_boost_5v_dev = boost_5v_sysfs_create_group();

	return 0;
}

static int boost_5v_remove(struct platform_device *pdev)
{
	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	if (!gpio_is_valid(gpio_5v_boost))
		gpio_free(gpio_5v_boost);

	mutex_destroy(&boost_op_mutex);
	boost_5v_sysfs_remove_group(g_boost_5v_dev);
	g_boost_5v_dev = NULL;

	return 0;
}

static const struct of_device_id boost_5v_match_table[] = {
	{
		.compatible = "huawei,boost_5v",
		.data = NULL,
	},
	{},
};

static struct platform_driver boost_5v_driver = {
	.probe = boost_5v_probe,
	.remove = boost_5v_remove,
	.driver = {
		.name = "huawei,boost_5v",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(boost_5v_match_table),
	},
};

static int __init boost_5v_init(void)
{
	return platform_driver_register(&boost_5v_driver);
}

static void __exit boost_5v_exit(void)
{
	platform_driver_unregister(&boost_5v_driver);
}

fs_initcall(boost_5v_init);
module_exit(boost_5v_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("boost with 5v driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
