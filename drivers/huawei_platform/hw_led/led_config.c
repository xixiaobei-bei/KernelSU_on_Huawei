/*
 * led_config.c
 *
 * led_config driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include "led_config.h"

#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/hisi/leds_pmic.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/touthscreen/huawei_tp_color.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG led_config
HWLOG_REGIST();

static struct led_config_data *g_data;

extern int tp_color_provider(void);
extern int tp_project_id_provider(char *name, uint8_t len);
static int led_config_parse_current_setting(struct led_config_data *data);
static int get_led_iset_immediately(void);

void led_config_get_current_setting(struct led_spmi_platform_data *hisi_leds)
{
	int ret;

	if (hisi_leds == NULL || g_data == NULL) {
		hwlog_err("%s null pointer error\n", __func__);
		return;
	}

	if (!(g_data->tp_color_flag || g_data->tp_module_flag ||
		g_data->tp_ic_flag))
		return;

	if (!g_data->curr.got_flag) {
		ret = get_led_iset_immediately();
		if (ret != 0) {
			hwlog_err("%s can not get led iset\n", __func__);
			return;
		}
	}

	hwlog_info("%s, get led iset: red = %u, green = %u, blue = %u",
		__func__, g_data->curr.red_curr, g_data->curr.green_curr,
		g_data->curr.blue_curr);
	hisi_leds->leds[0].each_maxdr_iset = g_data->curr.red_curr;
	hisi_leds->leds[1].each_maxdr_iset = g_data->curr.green_curr;
	hisi_leds->leds[2].each_maxdr_iset = g_data->curr.blue_curr;
}
EXPORT_SYMBOL(led_config_get_current_setting);

static int get_led_iset_immediately(void)
{
	if (g_data->retry_times <= 0)
		return -1;

	g_data->retry_times--;
	if (g_data->work_flag) {
		cancel_delayed_work(&g_data->work);
		g_data->work_flag = false;
	}
	return led_config_parse_current_setting(g_data);
}

static int led_config_get_tp_color_name(struct led_config_data *data,
	char *color_name, uint8_t len)
{
	int tp_color;
	char *tmp_str = NULL;

	if (data == NULL || color_name == NULL) {
		hwlog_err("%s null pointer error\n", __func__);
		return -1;
	}

	tp_color = tp_color_provider();
	hwlog_info("%s tp_color:0x%x\n", __func__, tp_color);
	switch (tp_color) {
	case WHITE:
		tmp_str = "white";
		break;
	case BLACK:
		tmp_str = "black";
		break;
	case BLUE:
		tmp_str = "blue";
		break;
	default:
		hwlog_err("%s tp_color_id error\n", __func__);
		return -1;
	}

	if (len <= ((uint8_t)strlen(tmp_str) + 1)) {
		hwlog_err("%s color_name len is too small, tmp_str len %d\n",
			__func__, (int)strlen(tmp_str));
		return -1;
	}
	memcpy(color_name, tmp_str, ((int)strlen(tmp_str) + 1));
	hwlog_info("%s out, color_name %s\n", __func__, color_name);
	return 0;
}

static int led_config_get_tp_module_name(
	uint8_t module_id, char *module_name, uint8_t len)
{
	const char *tmp_str = NULL;
	static const char tp_ic_name_map[][MAX_TP_INFO_LEN] = {
		"O-Film",
		"ECW",
		"Truly",
		"Mutto",
		"GIS",
		"JUNDA",
		"LENSONE",
		"YASSY",
		"JDI",
		"Samsung",
		"LG",
		"TIANMA",
		"CMI",
		"BOE",
		"CTC",
		"EDO",
		"Sharp",
		"Auo",
		"TopTouch",
		"BOE",
		"CTC",
		"BIEL",
		"KD",
		"EACH",
		"DJN",
		"TXD",
		"HLT",
	};

	if (module_id >= ARRAY_SIZE(tp_ic_name_map)) {
		hwlog_err("%s invalid data\n", __func__);
		return -1;
	}

	tmp_str = tp_ic_name_map[module_id];

	if (len <= ((uint8_t)strlen(tmp_str) + 1)) {
		hwlog_err("%s module_name len is too small, tmp_str len %d\n",
			__func__, (int)strlen(tmp_str));
		return -1;
	}
	memcpy(module_name, tmp_str, ((int)strlen(tmp_str) + 1));
	hwlog_info("%s module_name %s\n", __func__, module_name);
	return 0;
}

static int led_config_get_tp_ic_name(uint8_t ic_id,
	char *ic_name, uint8_t len)
{
	char *tmp_str = "default";

	if (len <= ((uint8_t)strlen(tmp_str) + 1)) {
		hwlog_err("%s ic_name len is too small, tmp_str len %d\n",
			__func__, (int)strlen(tmp_str));
		return -1;
	}

	memcpy(ic_name, tmp_str, ((int)strlen(tmp_str) + 1));
	hwlog_info("%s ic_name %s\n", __func__, ic_name);
	return 0;
}

static int led_config_get_tp_project_info(struct led_config_data *data,
					  char *module_name,
					  uint8_t module_len,
					  char *ic_name,
					  uint8_t ic_len)
{
	char tp_project_info[MAX_TP_INFO_LEN] = {0};
	int ret;
	uint8_t module_id;
	uint8_t ic_id;

	if (data == NULL || module_name == NULL || ic_name == NULL) {
		hwlog_err("%s null pointer error\n", __func__);
		return -1;
	}

	ret = tp_project_id_provider(tp_project_info, MAX_TP_INFO_LEN);
	if (ret != 0) {
		hwlog_err("%s get tp_project_info fail\n", __func__);
		return -1;
	}
	hwlog_info("%s tp_module_info %s\n", __func__, tp_project_info);

	/* tp_project_info [4]~[5] is the ic info, [6]~[8] is the module info */
	module_id = (uint8_t)turn_to_num(tp_project_info[6], tp_project_info[7]);
	ic_id = (uint8_t)turn_to_num(tp_project_info[4], tp_project_info[5]);

	if (data->tp_module_flag) {
		ret = led_config_get_tp_module_name(module_id,
			module_name, module_len);
		if (ret != 0) {
			hwlog_err("%s get module name fail\n", __func__);
			return -1;
		}
	}

	if (data->tp_ic_flag) {
		ret = led_config_get_tp_ic_name(ic_id,
			ic_name, ic_len);
		if (ret != 0) {
			hwlog_err("%s get module name fail\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int led_config_iset(struct led_config_data *data,
	const struct tp_info *tp, const char *led_name, uint8_t *led_iset)
{
	char config_dt_name[MAX_DT_NAME_LEN] = {0};
	int name_len;
	int ret;
	uint32_t tmp = 0;

	name_len = strlen(tp->module) + strlen("_module_") +
		strlen(tp->ic) + strlen("_ic_") +
		strlen(tp->color) + strlen("_color_tp_") +
		strlen(led_name) + 1;
	if (name_len >= MAX_DT_NAME_LEN) {
		hwlog_err("%s config_name buffer len too small\n", __func__);
		return -1;
	}

	if ((data->tp_module_flag) && (!data->tp_ic_flag) &&
		(!data->tp_color_flag)) {
		(void)sprintf(config_dt_name, "%s_module_tp_%s",
			tp->module, led_name);
	} else if ((data->tp_color_flag) && (!data->tp_ic_flag) &&
		(!data->tp_module_flag)) {
		(void)sprintf(config_dt_name, "%s_color_tp_%s",
			tp->color, led_name);
	} else {
		hwlog_err("%s now not supprot this", __func__);
		return -1;
	}

	ret = of_property_read_u32(data->node, config_dt_name, &tmp);
	if (ret != 0) {
		hwlog_err("%s:read %s fail\n", __func__, config_dt_name);
		return -1;
	}
	*led_iset = (uint8_t)tmp;
	return 0;
}

static int led_config_parse_current_setting(struct led_config_data *data)
{
	struct tp_info tp;
	int ret;

	memset(&tp, 0, sizeof(tp));
	if (data->tp_color_flag) {
		ret = led_config_get_tp_color_name(data, tp.color,
			sizeof(tp.color));
		if (ret != 0) {
			hwlog_err("%s get tp_color fail\n", __func__);
			return -1;
		}
	}

	if (data->tp_ic_flag || data->tp_module_flag) {
		ret = led_config_get_tp_project_info(data, tp.module,
			sizeof(tp.module), tp.ic, sizeof(tp.ic));
		if (ret != 0) {
			hwlog_err("%s get tp_module_name fail\n", __func__);
			return -1;
		}
	}

	ret = led_config_iset(data, &tp, "led_red_maxdr_iset",
		&(data->curr.red_curr));
	if (ret != 0) {
		hwlog_err("%s get led iset fail\n", __func__);
		return -1;
	}

	ret = led_config_iset(data, &tp, "led_green_maxdr_iset",
		&(data->curr.green_curr));
	if (ret != 0) {
		hwlog_err("%s get led iset fail\n", __func__);
		return -1;
	}

	ret = led_config_iset(data, &tp, "led_blue_maxdr_iset",
		&(data->curr.blue_curr));
	if (ret != 0) {
		hwlog_err("%s get led iset fail\n", __func__);
		return -1;
	}

	data->curr.got_flag = true;
	return 0;
}

static void led_config_work_func(struct work_struct *work)
{
	struct led_config_data *data = NULL;

	data = container_of(work, struct led_config_data, work.work);
	if (led_config_parse_current_setting(data))
		hwlog_info("%s get current fail\n", __func__);
	g_data->work_flag = false;
}

static const struct of_device_id led_config_match_table[] = {
	{ .compatible = "huawei,led_config", },
	{},
};
MODULE_DEVICE_TABLE(of, led_config_match_table);

static void get_led_curret_use_config(struct led_config_data *data)
{
	int ret;
	uint32_t tmp = 0;

	ret = of_property_read_u32(data->node,
		"led_current_using_tp_color_setting", &tmp);
	if (ret != 0) {
		hwlog_err("%s:read tp_color fail, using default\n",
			__func__);
		data->tp_color_flag = false;
	}
	data->tp_color_flag = (bool)tmp;

	ret = of_property_read_u32(data->node,
		"led_current_using_tp_module_setting", &tmp);
	if (ret != 0) {
		hwlog_err("%s:read tp_module fail, using default\n",
			__func__);
		data->tp_module_flag = false;
	}
	data->tp_module_flag = (bool)tmp;

	ret = of_property_read_u32(data->node,
		"led_current_using_tp_ic_setting", &tmp);
	if (ret != 0) {
		hwlog_err("%s:read tp_ic fail, using default\n",
			__func__);
		data->tp_ic_flag = false;
	}
	data->tp_ic_flag = (bool)tmp;

	hwlog_info("%s: tp_flag = %d, %d, %d", __func__,
		(int)data->tp_color_flag,
		(int)data->tp_module_flag,
		(int)data->tp_ic_flag);
}

static int led_config_probe(struct platform_device *pdev)
{
	struct led_config_data *data = NULL;

	if (pdev == NULL) {
		hwlog_err("%s null pointer error\n", __func__);
		return -ENODEV;
	}

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;

	data->node = pdev->dev.of_node;
	if (data->node == NULL) {
		hwlog_err("%s failed to find dts node led_alwayson\n",
			__func__);
		devm_kfree(&pdev->dev, data);
		data = NULL;
		return -ENODEV;
	}

	get_led_curret_use_config(data);

	INIT_DELAYED_WORK(&data->work, led_config_work_func);
	if (data->tp_color_flag || data->tp_module_flag || data->tp_ic_flag) {
		schedule_delayed_work(&data->work, 3 * HZ); // delay 3s
		data->work_flag = true;
	}

	data->retry_times = 10; // retry 10 times
	g_data = data;
	platform_set_drvdata(pdev, data);

	hwlog_info("%s succ\n", __func__);
	return 0;
}

static int led_config_remove(struct platform_device *pdev)
{
	struct led_config_data *data = NULL;

	if (pdev == NULL) {
		hwlog_err("%s null pointer error\n", __func__);
		return -ENODEV;
	}
	data = platform_get_drvdata(pdev);

	devm_kfree(&pdev->dev, data);
	data = NULL;
	g_data = NULL;
	platform_set_drvdata(pdev, NULL);

	return 0;
}

struct platform_driver led_config_driver = {
	.probe = led_config_probe,
	.remove = led_config_remove,
	.driver = {
		.name = LED_CONFIG,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(led_config_match_table),
	},
};

static int __init led_config_init(void)
{
	hwlog_info("init\n");
	return platform_driver_register(&led_config_driver);
}

static void __exit led_config_exit(void)
{
	platform_driver_unregister(&led_config_driver);
}

module_init(led_config_init);
module_exit(led_config_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Led config driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
