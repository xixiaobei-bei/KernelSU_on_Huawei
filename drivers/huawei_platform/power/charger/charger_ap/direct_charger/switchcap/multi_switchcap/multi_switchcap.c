/*
 * multi_switchcap.c
 *
 * multi switchcap driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <huawei_platform/power/multi_switchcap.h>

#define HWLOG_TAG multi_switchcap
HWLOG_REGIST();

static struct multi_sc_chip_info *g_info[MULTI_SWITCHCAP_NUM];
static struct multi_sc_info *g_multi_sc_dev;
static struct ovp_ops *g_ovp_ops;
static int g_current_sc_num;

int multi_switchcap_chip_register(struct multi_sc_chip_info *info)
{
	if (!info) {
		hwlog_err("info[%d] is null\n", g_current_sc_num);
		return -1;
	}

	if (g_current_sc_num >= MULTI_SWITCHCAP_NUM) {
		hwlog_err("g_current_sc_num out of range\n");
		return -1;
	}

	g_info[g_current_sc_num] = info;

	hwlog_info("dev_id=%d register success\n",
		g_info[g_current_sc_num]->dev_id);

	g_current_sc_num++;

	return 0;
}

void multi_switchcap_chip_unregister(void)
{
	int i;

	for (i = 0; i < g_current_sc_num; i++)
		g_info[g_current_sc_num] = NULL;
}

int ovp_for_sc_ops_register(struct ovp_ops *ops)
{
	if (!ops) {
		hwlog_err("g_multi_sc or ops is null\n");
		return -1;
	}

	g_ovp_ops = ops;
	hwlog_info("ovp ops register success\n");

	return 0;
}

static int multi_switchcap_sc_power(bool en)
{
	int i;

	for (i = 0; i < g_current_sc_num; i++) {
		if (g_info[i]->ops && g_info[i]->data &&
			g_info[i]->ops->sc_chip_power) {
			if (!g_info[i]->ops->sc_chip_power(g_info[i]->data, en))
				return 0;
		}
	}

	return -1;
}

static int multi_switchcap_chip_enable(int enable)
{
	int i;
	int ret = 0;

	if (g_ovp_ops && g_ovp_ops->ovp_enable)
		g_ovp_ops->ovp_enable(enable);

	if (enable)
		msleep(200); /* wait OVP ready and SC precharge */

	for (i = 0; i < g_current_sc_num; i++) {
		if (g_info[i]->ops && g_info[i]->data &&
			g_info[i]->ops->sc_enable)
			ret |= g_info[i]->ops->sc_enable(g_info[i]->data,
				enable);
	}

	return ret;
}

static int multi_switchcap_enable_prepare(void *dev_data)
{
	msleep(120); /* sleep 120ms wait for vbus ready */
	return multi_switchcap_chip_enable(1);
}

static int multi_switchcap_charge_init(void *dev_data)
{
	int i;
	int ret = 0;

	multi_switchcap_sc_power(true);
	/* sleep 10ms for chip ready */
	usleep_range(10000, 11000);

	for (i = 0; i < g_current_sc_num; i++) {
		if (g_info[i]->ops && g_info[i]->data &&
			g_info[i]->ops->sc_init)
			ret |= g_info[i]->ops->sc_init(g_info[i]->data);
	}

	return ret;
}

static int multi_switchcap_charge_exit(void *dev_data)
{
	int i;
	int ret = 0;

	multi_switchcap_chip_enable(0);

	for (i = 0; i < g_current_sc_num; i++) {
		if (g_info[i]->ops && g_info[i]->data &&
			g_info[i]->ops->sc_exit)
			ret |= g_info[i]->ops->sc_exit(g_info[i]->data);
	}

	multi_switchcap_sc_power(false);

	return ret;
}

static int multi_switchcap_charge_enable(int enable, void *dev_data)
{
	int value;

	if (!g_multi_sc_dev) {
		hwlog_err("g_multi_sc_dev is null\n");
		return -1;
	}

	value = g_multi_sc_dev->gpio_high_effect ? enable : !enable;

	gpio_set_value(g_multi_sc_dev->gpio_btst_delay, value);
	hwlog_info("set gpio_btst_delay=%d\n", value);

	return 0;
}

static int multi_switchcap_is_device_close(void *dev_data)
{
	int i;
	int ret = 0;

	for (i = 0; i < g_current_sc_num; i++) {
		if (g_info[i]->ops && g_info[i]->data &&
			g_info[i]->ops->is_sc_close)
			ret |= g_info[i]->ops->is_sc_close(g_info[i]->data);
	}

	return ret;
}

static int multi_switchcap_get_device_id(void *dev_data)
{
	int i;
	int ret = 0;

	for (i = 0; i < g_current_sc_num; i++) {
		if (g_info[i]->ops && g_info[i]->data &&
			g_info[i]->ops->get_sc_id)
			ret |= g_info[i]->ops->get_sc_id(g_info[i]->data);
	}

	return ret;
}

static int multi_switchcap_discharge(int enable, void *dev_data)
{
	return 0;
}

static int multi_switchcap_config_watchdog_ms(int time, void *dev_data)
{
	return 0;
}

static int multi_switchcap_status(void *dev_data)
{
	return 0;
}

static struct dc_ic_ops multi_switchcap_ops = {
	.dev_name = "multi_switchcap",
	.ic_init = multi_switchcap_charge_init,
	.ic_exit = multi_switchcap_charge_exit,
	.ic_enable = multi_switchcap_charge_enable,
	.ic_discharge = multi_switchcap_discharge,
	.is_ic_close = multi_switchcap_is_device_close,
	.get_ic_id = multi_switchcap_get_device_id,
	.config_ic_watchdog = multi_switchcap_config_watchdog_ms,
	.get_ic_status = multi_switchcap_status,
	.ic_enable_prepare = multi_switchcap_enable_prepare,
};

static int multi_switchcap_prase_dts(struct multi_sc_info *di,
	struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "sc_num",
		&di->sc_num, MULTI_SWITCHCAP_NUM))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_high_effect", &di->gpio_high_effect, 0);

	if (power_gpio_config_output(np,
		"gpio_btst_delay", "multi_sc_delay",
		&di->gpio_btst_delay, 1))
		return -EINVAL;

	return 0;
}

static int multi_switchcap_probe(struct platform_device *pdev)
{
	struct multi_sc_info *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->pdev = pdev;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	g_multi_sc_dev = di;

	ret = multi_switchcap_prase_dts(di, np);
	if (ret)
		return -EINVAL;

	hwlog_info("current_sc_num=%d\n", g_current_sc_num);
	if (g_current_sc_num < di->sc_num) {
		ret = -EINVAL;
		goto prase_dts_fail;
	}

	ret = multi_switchcap_charge_enable(0, di);
	if (ret)
		goto prase_dts_fail;

	ret = dc_ic_ops_register(SC_MODE, CHARGE_IC_TYPE_MAIN, &multi_switchcap_ops);
	if (ret) {
		hwlog_err("switchcap ops register failed\n");
		goto prase_dts_fail;
	}

	platform_set_drvdata(pdev, di);
	return 0;

prase_dts_fail:
	gpio_free(di->gpio_btst_delay);
	return ret;
}

static int multi_switchcap_remove(struct platform_device *pdev)
{
	struct multi_sc_info *info = platform_get_drvdata(pdev);

	if (!info)
		return -ENODEV;

	if (info->gpio_btst_delay)
		gpio_free(info->gpio_btst_delay);

	multi_switchcap_chip_unregister();
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);

	return 0;
}

static const struct of_device_id multi_switchcap_match_table[] = {
	{
		.compatible = "huawei,multi_switchcap",
		.data = NULL,
	},
	{},
};

static struct platform_driver multi_switchcap_driver = {
	.probe = multi_switchcap_probe,
	.remove = multi_switchcap_remove,
	.driver = {
		.name = "multi_switchcap",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(multi_switchcap_match_table),
	},
};

static int __init multi_switchcap_init(void)
{
	return platform_driver_register(&multi_switchcap_driver);
}

static void __exit multi_switchcap_exit(void)
{
	platform_driver_unregister(&multi_switchcap_driver);
}

device_initcall_sync(multi_switchcap_init);
module_exit(multi_switchcap_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("multi switchcap module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
