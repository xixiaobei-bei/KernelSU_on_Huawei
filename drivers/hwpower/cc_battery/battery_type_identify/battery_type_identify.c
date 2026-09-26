/*
 * battery_type_identify.c
 *
 * interfaces for switch mode between adc and
 * onewire communication to identify battery type.
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/battery/battery_type_identify.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/batt_info_pub.h>

#define HWLOG_TAG batt_type_identify
HWLOG_REGIST();

static struct bat_type_dev *g_bat_type_dev;

void bat_security_ic_ops_register(const struct bat_security_ic_ops *ops)
{
	if (!g_bat_type_dev)
		return;

	g_bat_type_dev->ops = ops;
}

void bat_security_ic_ops_unregister(const struct bat_security_ic_ops *ops)
{
	if (!g_bat_type_dev)
		return;

	if (g_bat_type_dev->ops == ops)
		g_bat_type_dev->ops = NULL;
}

static int bat_type_set_gpio(int gpio, int direction, int value)
{
	if (direction == BAT_TYPE_GPIO_IN)
		return gpio_direction_input(gpio);

	return gpio_direction_output(gpio, value);
}

static void bat_type_set_id_mos(struct bat_type_dev *l_dev, int mode)
{
	int ret = -1;

	switch (mode) {
	case BAT_ID_VOLTAGE:
		/* close mos */
		ret = bat_type_set_gpio(l_dev->gpio,
			l_dev->id_voltage.direction,
			l_dev->id_voltage.value);

		if (l_dev->ops && l_dev->ops->close_ic)
			l_dev->ops->close_ic();
		break;
	case BAT_ID_SN:
		/* open mos */
		ret = bat_type_set_gpio(l_dev->gpio,
			l_dev->id_sn.direction,
			l_dev->id_sn.value);

		if (l_dev->ops && l_dev->ops->open_ic)
			l_dev->ops->open_ic();
		break;
	default:
		break;
	}

	if (ret) {
		hwlog_err("switch mode to %d error\n", mode);
		return;
	}

	l_dev->cur_mode = mode;
}

void bat_type_apply_mode(int mode)
{
	struct bat_type_dev *l_dev = g_bat_type_dev;

	if (!l_dev)
		return;

	hwlog_info("apply_mode cur_mode=%d mode=%d\n", l_dev->cur_mode, mode);

	mutex_lock(&l_dev->lock);
	if (l_dev->cur_mode == mode)
		return;

	bat_type_set_id_mos(l_dev, mode);
}

void bat_type_release_mode(bool flag)
{
	struct bat_type_dev *l_dev = g_bat_type_dev;

	if (!l_dev)
		return;

	hwlog_info("release_mode flag=%d\n", flag);

	if (flag)
		bat_type_set_id_mos(l_dev, BAT_ID_VOLTAGE);
	mutex_unlock(&l_dev->lock);
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t bat_type_dbg_identify_mode_show(void *dev_data,
	char *buf, size_t size)
{
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev)
		return snprintf(buf, size, "driver init error\n");

	return snprintf(buf, size, "current mode is %d\n", l_dev->cur_mode);
}

static ssize_t bat_type_dbg_id_voltage_show(void *dev_data,
	char *buf, size_t size)
{
	int voltage;
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev)
		return snprintf(buf, size, "driver init error\n");

	voltage = power_platform_get_battery_id_voltage();
	return snprintf(buf, size, "read id voltage is %d\n", voltage);
}

static ssize_t bat_type_dbg_id_sn_show(void *dev_data,
	char *buf, size_t size)
{
	unsigned char id_sn[BAT_TYPE_ID_SN_SIZE] = { 0 };
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev)
		return snprintf(buf, size, "driver init error\n");

	if (get_battery_type(id_sn, BAT_TYPE_ID_SN_SIZE))
		return snprintf(buf, size, "get id sn error\n");

	return snprintf(buf, size, "read battery type is %s\n", id_sn);
}

static ssize_t bat_type_dbg_id_sn_voltage_show(void *dev_data,
	char *buf, size_t size)
{
	int i;
	int len = 0;
	int voltage;
	unsigned char id_sn[BAT_TYPE_ID_SN_SIZE] = { 0 };
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev)
		return snprintf(buf, size, "driver init error\n");

	for (i = 0; i < BAT_TYPE_TEST_TIMES; i++) {
		len += snprintf(buf + len, size, "%d--", i);
		if (get_battery_type(id_sn, BAT_TYPE_ID_SN_SIZE)) {
			len += snprintf(buf + len, size, "read error\n");
			return len;
		}

		len += snprintf(buf + len, size, "%s--", id_sn);
		voltage = power_platform_get_battery_id_voltage();
		len += snprintf(buf + len, size, "%d++\n", voltage);
	}

	len += snprintf(buf + len, size, "read fine\n");
	return len;
}

static ssize_t bat_type_dbg_open_ic_show(void *dev_data,
	char *buf, size_t size)
{
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev || !l_dev->ops || !l_dev->ops->open_ic)
		return snprintf(buf, size, "driver init error\n");

	l_dev->ops->open_ic();
	return snprintf(buf, size, "open ic pass\n");
}

static ssize_t bat_type_dbg_close_ic_show(void *dev_data,
	char *buf, size_t size)
{
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev || !l_dev->ops || !l_dev->ops->close_ic)
		return snprintf(buf, size, "driver init error\n");

	l_dev->ops->close_ic();
	return snprintf(buf, size, "close ic pass\n");
}

static ssize_t bat_type_dbg_open_mos_show(void *dev_data,
	char *buf, size_t size)
{
	int ret;
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev)
		return snprintf(buf, size, "driver init error\n");

	ret = gpio_direction_output(l_dev->gpio, 0);
	return snprintf(buf, size, "open mos ret %d\n", ret);
}

static ssize_t bat_type_dbg_close_mos_show(void *dev_data,
	char *buf, size_t size)
{
	int ret;
	struct bat_type_dev *l_dev = dev_data;

	if (!l_dev)
		return snprintf(buf, size, "driver init error\n");

	ret = gpio_direction_output(l_dev->gpio, 1);
	return snprintf(buf, size, "close mos ret %d\n", ret);
}

static void bat_type_debug_register(struct bat_type_dev *l_dev)
{
	power_dbg_ops_register("identify_mode", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_identify_mode_show, NULL);
	power_dbg_ops_register("id_voltage", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_id_voltage_show, NULL);
	power_dbg_ops_register("id_sn", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_id_sn_show, NULL);
	power_dbg_ops_register("id_sn_voltage", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_id_sn_voltage_show, NULL);
	power_dbg_ops_register("open_ic", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_open_ic_show, NULL);
	power_dbg_ops_register("close_ic", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_close_ic_show, NULL);
	power_dbg_ops_register("open_mos", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_open_mos_show, NULL);
	power_dbg_ops_register("close_mos", (void *)l_dev,
		(power_dbg_show)bat_type_dbg_close_mos_show, NULL);
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int bat_type_parse_gpio(struct device_node *np)
{
	int gpio;

	if (power_gpio_request(np, "gpios", "battery_type_gpio", &gpio))
		return -1;

	return gpio;
}

static int bat_type_parse_channel(struct device_node *np,
	const char *prop, struct bat_type_gpio_state *states)
{
	if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
		np, prop, 0, &states->direction))
		return -1;

	if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
		np, prop, 1, &states->value))
		return -1;

	return 0;
}

static int bat_type_parse_dts(struct device_node *np,
	struct bat_type_dev *l_dev)
{
	l_dev->gpio = bat_type_parse_gpio(np);
	if (l_dev->gpio < 0)
		return -1;

	if (bat_type_parse_channel(np, "id_voltage_gpiov", &l_dev->id_voltage) ||
		bat_type_parse_channel(np, "id_sn_gpiov", &l_dev->id_sn))
		return -1;

	return 0;
}

static int bat_type_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct bat_type_dev *l_dev = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = devm_kzalloc(&pdev->dev, sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	np = pdev->dev.of_node;
	l_dev->cur_mode = BAT_INVALID_MODE;

	if (bat_type_parse_dts(np, l_dev))
		goto free_mem;

	mutex_init(&l_dev->lock);
	g_bat_type_dev = l_dev;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	bat_type_debug_register(l_dev);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

	return 0;

free_mem:
	devm_kfree(&pdev->dev, l_dev);
	return -1;
}

static int bat_type_remove(struct platform_device *pdev)
{
	if (!g_bat_type_dev)
		return -ENODEV;

	mutex_destroy(&g_bat_type_dev->lock);
	gpio_free(g_bat_type_dev->gpio);
	return 0;
}

static const struct of_device_id bat_type_match_table[] = {
	{
		.compatible = "huawei,battery-identify",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_type_driver = {
	.probe = bat_type_probe,
	.remove = bat_type_remove,
	.driver = {
		.name = "battery-type-identify",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_type_match_table),
	},
};

static int __init bat_type_init(void)
{
	return platform_driver_register(&bat_type_driver);
}

static void __exit bat_type_exit(void)
{
	platform_driver_unregister(&bat_type_driver);
}

subsys_initcall(bat_type_init);
module_exit(bat_type_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery type identify driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
