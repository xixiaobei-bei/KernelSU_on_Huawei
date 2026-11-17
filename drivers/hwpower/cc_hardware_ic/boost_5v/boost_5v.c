/*
 * boost_5v.c
 *
 * boost with 5v driver
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

#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_vote.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG boost_5v
HWLOG_REGIST();

static struct boost_5v_dev *g_boost_5v_dev;

static int boost_5v_output(struct boost_5v_dev *l_dev, int value)
{
	int ret;

	if (l_dev->use_pmic) {
		ret = power_platform_pmic_enable_boost(value);
		if (ret) {
			hwlog_err("pmic enable boost fail\n");
			return -1;
		}
	} else if (l_dev->use_buckboost) {
		if (value == BOOST_5V_ENABLE) {
			buck_boost_set_enable(value);
			buck_boost_set_vout(l_dev->buckboost_vol);
		} else {
			buck_boost_set_enable(value);
		}
	} else {
		gpio_set_value(l_dev->gpio_no, value);
	}

	return 0;
}

/*lint -e580*/
int boost_5v_enable(bool enable, const char *client_name)
{
	struct boost_5v_dev *l_dev = g_boost_5v_dev;

	if (!l_dev || !client_name)
		return -EINVAL;

	if (!(l_dev->use_gpio || l_dev->use_pmic || l_dev->use_buckboost)) {
		hwlog_err("boost_5v not initialized\n");
		return -EINVAL;
	}

	return power_vote_set(BOOST_5V_VOTE_OBJECT, client_name, enable, enable);
}
EXPORT_SYMBOL(boost_5v_enable);

bool boost_5v_status(const char *client_name)
{
	struct boost_5v_dev *l_dev = g_boost_5v_dev;

	if (!l_dev || !client_name)
		return false;

	if (!(l_dev->use_gpio || l_dev->use_pmic || l_dev->use_buckboost)) {
		hwlog_err("boost_5v not initialized\n");
		return false;
	}

	return power_vote_is_client_enabled_locked(BOOST_5V_VOTE_OBJECT, client_name, true);
}
EXPORT_SYMBOL(boost_5v_status);
/*lint +e580*/

static int boost_5v_vote_callback(struct power_vote_object *obj,
	void *data, int result, const char *client_str)
{
	struct boost_5v_dev *l_dev = (struct boost_5v_dev *)data;

	if (!l_dev || !client_str) {
		hwlog_err("l_dev or client_str is null\n");
		return -EINVAL;
	}

	hwlog_info("result=%d client_str=%s\n", result, client_str);

	if (result)
		boost_5v_output(l_dev, BOOST_5V_ENABLE);
	else
		boost_5v_output(l_dev, BOOST_5V_DISABLE);

	return 0;
}

static int boost_5v_parse_dts(struct boost_5v_dev *l_dev, struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_5v_use_common_pmic", &l_dev->use_pmic, 0);
	if (l_dev->use_pmic) {
		l_dev->use_gpio = 0;
		return 0;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_5v_use_buckboost", &l_dev->use_buckboost, 0);
	if (l_dev->use_buckboost) {
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"boost_5v_buckboost_vol", &l_dev->buckboost_vol, BBST_DEFAULT_VOUT);
		l_dev->use_gpio = 0;
		return 0;
	}

	if (power_gpio_config_output(np,
		"gpio_5v_boost", "gpio_5v_boost", &l_dev->gpio_no, 0))
		return -1;
	l_dev->use_gpio = 1;
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t boost_5v_enable_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct boost_5v_dev *l_dev = g_boost_5v_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return -EINVAL;
	}

	if (!power_cmdline_is_factory_mode()) {
		hwlog_info("only factory version can ctrl boost_5v\n");
		return count;
	}

	/* 0: disable, 1: enable, others: invaid */
	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1)) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (val)
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_AT_CMD);
	else
		boost_5v_output(l_dev, BOOST_5V_DISABLE);

	hwlog_info("ctrl boost_5v by sys class\n");
	return count;
}

static DEVICE_ATTR(enable, 0200, NULL, boost_5v_enable_sysfs_store);

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
	struct boost_5v_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_boost_5v_dev = l_dev;
	np = pdev->dev.of_node;
	if (boost_5v_parse_dts(l_dev, np))
		goto fail_free_mem;

	l_dev->dev = boost_5v_sysfs_create_group();
	power_vote_create_object(BOOST_5V_VOTE_OBJECT, POWER_VOTE_SET_ANY,
		boost_5v_vote_callback, l_dev);
	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_boost_5v_dev = NULL;
	return -1;
}

static int boost_5v_remove(struct platform_device *pdev)
{
	struct boost_5v_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	if (l_dev->use_gpio) {
		if (gpio_is_valid(l_dev->gpio_no))
			gpio_free(l_dev->gpio_no);
	}

	power_vote_destroy_object(BOOST_5V_VOTE_OBJECT);
	boost_5v_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
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
