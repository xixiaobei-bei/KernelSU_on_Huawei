/*
 * vbus_channel_pogopin_boost.c
 *
 * pogopin boost for vbus channel driver
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
#include <linux/of_gpio.h>
#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/vbus_channel/vbus_channel.h>
#include <huawei_platform/power/vbus_channel/vbus_channel_pogopin_boost.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG vbus_ch_pogopin
HWLOG_REGIST();

static struct pogopin_boost_dev *g_pogopin_boost_dev;

static struct pogopin_boost_dev *pogopin_boost_get_dev(void)
{
	if (!g_pogopin_boost_dev) {
		hwlog_err("g_pogopin_boost_dev is null\n");
		return NULL;
	}

	return g_pogopin_boost_dev;
}

static int pogopin_boost_start_config(void)
{
	struct pogopin_boost_dev *l_dev = NULL;

	l_dev = pogopin_boost_get_dev();
	if (!l_dev)
		return -EINVAL;

	/* open mos between charger and buckboost */
	gpio_set_value(l_dev->mos_en, MOS_GPIO_SWITCH_ENABLE);
	/* open buckboost */
	gpio_set_value(l_dev->buckboost_en, BUCKBOOST_GPIO_SWITCH_ENABLE);

	return 0;
}

static int pogopin_boost_stop_config(void)
{
	struct pogopin_boost_dev *l_dev = NULL;

	l_dev = pogopin_boost_get_dev();
	if (!l_dev)
		return -EINVAL;

	/* close mos between charger and buckboost */
	gpio_set_value(l_dev->mos_en, MOS_GPIO_SWITCH_DISABLE);
	/* close buckboost */
	gpio_set_value(l_dev->buckboost_en, BUCKBOOST_GPIO_SWITCH_DISABLE);

	return 0;
}

static int pogopin_boost_open(unsigned int user, int flag)
{
	struct pogopin_boost_dev *l_dev = NULL;

	l_dev = pogopin_boost_get_dev();
	if (!l_dev)
		return -EINVAL;

	if (pogopin_boost_start_config())
		return -EINVAL;

	l_dev->user |= (1 << user);

	hwlog_info("pogopin_boost(%x) open ok\n", l_dev->user);
	return 0;
}

static int pogopin_boost_close(unsigned int user, int flag, int force)
{
	struct pogopin_boost_dev *l_dev = NULL;

	l_dev = pogopin_boost_get_dev();
	if (!l_dev)
		return -EINVAL;

	l_dev->user &= ~(1 << user);

	if (l_dev->user == VBUS_CH_NO_OP_USER) {
		if (pogopin_boost_stop_config())
			return -EINVAL;
	}

	hwlog_info("pogopin_boost(%x) close ok\n", l_dev->user);
	return 0;
}

static int pogopin_boost_get_state(unsigned int user, int *state)
{
	struct pogopin_boost_dev *l_dev = NULL;

	l_dev = pogopin_boost_get_dev();
	if (!l_dev || !state)
		return -EINVAL;

	if (l_dev->user == VBUS_CH_NO_OP_USER)
		*state = VBUS_CH_STATE_CLOSE;
	else
		*state = VBUS_CH_STATE_OPEN;

	return 0;
}

struct vbus_ch_ops pogopin_boost_ops = {
	.type_name = "pogopin_boost",
	.open = pogopin_boost_open,
	.close = pogopin_boost_close,
	.get_state = pogopin_boost_get_state,
	.get_mode = NULL,
	.set_switch_mode = NULL,
	.set_voltage = NULL,
	.get_voltage = NULL,
};

static int pogopin_boost_parse_dts(struct device_node *np,
	struct pogopin_boost_dev *l_dev)
{
	int ret;

	l_dev->mos_en = of_get_named_gpio(np, "vbus_mos_ctrl_gpio", 0);
	hwlog_info("vbus_mos_ctrl_gpio=%d\n", l_dev->mos_en);

	if (!gpio_is_valid(l_dev->mos_en)) {
		hwlog_err("gpio is not valid\n");
		return -EINVAL;
	}

	ret = gpio_request(l_dev->mos_en, "vbus_mos_ctrl_gpio");
	if (ret) {
		hwlog_err("gpio request fail\n");
		return ret;
	}

	ret = gpio_direction_output(l_dev->mos_en, MOS_GPIO_SWITCH_DISABLE);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		gpio_free(l_dev->mos_en);
		return ret;
	}

	l_dev->buckboost_en = of_get_named_gpio(np, "buckboost_gpio", 0);
	hwlog_info("buckboost_gpio=%d\n", l_dev->buckboost_en);

	if (!gpio_is_valid(l_dev->buckboost_en)) {
		hwlog_err("gpio is not valid\n");
		gpio_free(l_dev->mos_en);
		return -EINVAL;
	}

	ret = gpio_request(l_dev->buckboost_en, "buckboost_gpio");
	if (ret) {
		hwlog_err("gpio request fail\n");
		gpio_free(l_dev->mos_en);
		return ret;
	}

	ret = gpio_direction_output(l_dev->buckboost_en,
		BUCKBOOST_GPIO_SWITCH_DISABLE);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		gpio_free(l_dev->mos_en);
		gpio_free(l_dev->buckboost_en);
		return ret;
	}

	return 0;
}

static int pogopin_boost_probe(struct platform_device *pdev)
{
	int ret;
	struct pogopin_boost_dev *l_dev = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = devm_kzalloc(&pdev->dev, sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_pogopin_boost_dev = l_dev;
	l_dev->pdev = pdev;
	l_dev->dev = &pdev->dev;

	ret = pogopin_boost_parse_dts(l_dev->dev->of_node, l_dev);
	if (ret)
		goto fail_parse_dts;

	ret = vbus_ch_ops_register(&pogopin_boost_ops);
	if (ret)
		goto fail_register_ops;

	platform_set_drvdata(pdev, l_dev);

	hwlog_info("probe end\n");
	return 0;

fail_register_ops:
	gpio_free(l_dev->mos_en);
	gpio_free(l_dev->buckboost_en);
fail_parse_dts:
	devm_kfree(&pdev->dev, l_dev);
	g_pogopin_boost_dev = NULL;

	return ret;
}

static int pogopin_boost_remove(struct platform_device *pdev)
{
	struct pogopin_boost_dev *l_dev = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!l_dev)
		return -EINVAL;

	if (l_dev->mos_en)
		gpio_free(l_dev->mos_en);

	if (l_dev->buckboost_en)
		gpio_free(l_dev->buckboost_en);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, l_dev);
	g_pogopin_boost_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id pogopin_boost_match_table[] = {
	{
		.compatible = "huawei,vbus_channel_pogopin_boost",
		.data = NULL,
	},
	{},
};

static struct platform_driver pogopin_boost_driver = {
	.probe = pogopin_boost_probe,
	.remove = pogopin_boost_remove,
	.driver = {
		.name = "huawei,vbus_channel_pogopin_boost",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pogopin_boost_match_table),
	},
};

static int __init pogopin_boost_init(void)
{
	return platform_driver_register(&pogopin_boost_driver);
}

static void __exit pogopin_boost_exit(void)
{
	platform_driver_unregister(&pogopin_boost_driver);
}

fs_initcall(pogopin_boost_init);
module_exit(pogopin_boost_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pogopin boost for vbus channel driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
