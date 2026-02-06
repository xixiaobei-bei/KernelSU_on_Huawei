/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "tp_color.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <huawei_platform/log/hw_log.h>
#include "tpkit_platform_adapter.h"
#ifdef CONFIG_SENSOR_PARAM_TO_SCP
#include "SCP_power_monitor.h"
#endif

#define HWLOG_TAG tp_color
HWLOG_REGIST();


char tp_color_buf[TP_COLOR_BUF_SIZE];
u8 cypress_ts_kit_color[TP_COLOR_SIZE];

static char g_tp_color_string[TP_COLOR_SIZE];

static struct work_struct nv_read_work;
static struct work_struct nv_write_work;
static struct completion nv_read_done;
static struct completion nv_write_done;


static int __init early_parse_tp_color_cmdline(char *arg)
{
	int len;

	memset(tp_color_buf, 0, sizeof(tp_color_buf));
	if (arg != NULL) {
		len = strlen(arg);
		if (len > sizeof(tp_color_buf))
			len = sizeof(tp_color_buf);
		memcpy(tp_color_buf, arg, len);
	} else {
		hwlog_info("%s: arg is NULL\n", __func__);
	}

	return 0;
}
early_param("TP_COLOR", early_parse_tp_color_cmdline);

static int read_tp_color(void)
{
	int tp_color = 0;

	hwlog_info("tp color is %s\n", tp_color_buf);
	if (kstrtoint(tp_color_buf, 0, &tp_color) != 0)
		hwlog_err("kstrtol failed\n");

	return tp_color;
}

static int is_color_correct(u8 color)
{
	if ((color & TP_COLOR_MASK) ==
		((~(color >> TP_COLOR_OFFSET_4BIT)) & TP_COLOR_MASK))
		return true;
	else
		return false;
}

static int read_tp_color_from_nv(void)
{
	int ret;
	char nv_data[NV_DATA_SIZE] = {0};

	ret = read_tp_color_adapter(nv_data, NV_DATA_SIZE);
	if (ret < 0) {
		hwlog_err("%s error %d\n", __func__, ret);
		return ret;
	}

	if ((!strncmp(nv_data, "white", strlen("white"))) ||
		(!strncmp(nv_data, "black", strlen("black"))) ||
		(!strncmp(nv_data, "silver", strlen("silver"))) ||
		(!strncmp(nv_data, "pink", strlen("pink"))) ||
		(!strncmp(nv_data, "red", strlen("red"))) ||
		(!strncmp(nv_data, "yellow", strlen("yellow"))) ||
		(!strncmp(nv_data, "blue", strlen("blue"))) ||
		(!strncmp(nv_data, "gold", strlen("gold"))) ||
		(!strncmp(nv_data, "gray", strlen("gray"))) ||
		(!strncmp(nv_data, "cafe", strlen("cafe"))) ||
		(!strncmp(nv_data, "pdblack", strlen("pdblack"))) ||
		(!strncmp(nv_data, "green", strlen("green"))) ||
		(!strncmp(nv_data, "pinkgold", strlen("pinkgold")))) {
		strncpy(g_tp_color_string, nv_data, strlen(nv_data));
		g_tp_color_string[strlen(nv_data)] = '\0';
		return 0;
	}
	hwlog_err("%s: read abnormal value\n", __func__);
	return -1;
}

static void write_tp_color_to_nv(void)
{
	u8 lcd_id;
	u8 phone_color;

	memset(g_tp_color_string, 0, sizeof(g_tp_color_string));
	lcd_id = read_tp_color();
	/* 0xff: an invalid tp_color value */
	if (lcd_id != 0xff)
		hwlog_info("lcd id is %u from read tp color\n", lcd_id);
	if (is_color_correct(cypress_ts_kit_color[0])) {
		phone_color = cypress_ts_kit_color[0];
	} else if (is_color_correct(lcd_id)) {
		phone_color = lcd_id;
	} else {
		hwlog_err("LCD/TP ID both error!\n");
		return;
	}
	switch (phone_color) {
	case WHITE:
		strncpy(g_tp_color_string, "white", TP_COLOR_SIZE - 1);
		break;
	case BLACK:
		strncpy(g_tp_color_string, "black", TP_COLOR_SIZE - 1);
		break;
	case PINK:
	case PINKGOLD:
		strncpy(g_tp_color_string, "pink", TP_COLOR_SIZE - 1);
		break;
	case RED:
		strncpy(g_tp_color_string, "red", TP_COLOR_SIZE - 1);
		break;
	case YELLOW:
		strncpy(g_tp_color_string, "yellow", TP_COLOR_SIZE - 1);
		break;
	case BLUE:
		strncpy(g_tp_color_string, "blue", TP_COLOR_SIZE - 1);
		break;
	case GOLD:
		strncpy(g_tp_color_string, "gold", TP_COLOR_SIZE - 1);
		break;
	case SILVER:
		strncpy(g_tp_color_string, "silver", TP_COLOR_SIZE - 1);
		break;
	case GRAY:
		strncpy(g_tp_color_string, "gray", TP_COLOR_SIZE - 1);
		break;
	case CAFE:
	case CAFE2:
		strncpy(g_tp_color_string, "cafe", TP_COLOR_SIZE - 1);
		break;
	case BLACK2:
		strncpy(g_tp_color_string, "pdblack", TP_COLOR_SIZE - 1);
		break;
	case GREEN:
		strncpy(g_tp_color_string, "green", TP_COLOR_SIZE - 1);
		break;
	default:
		strncpy(g_tp_color_string, "", TP_COLOR_SIZE - 1);
		break;
	}
	schedule_work(&nv_write_work);
	if (!wait_for_completion_interruptible_timeout(&nv_write_done,
		msecs_to_jiffies(NV_WRITE_TIMEOUT)))
		hwlog_info("nv_write_work time out\n");
	hwlog_info("%s: %s\n", __func__, g_tp_color_string);
}

static void tp_nv_read_work_fn(struct work_struct *work)
{
	int ret;

	reinit_completion(&nv_write_done);
	ret = read_tp_color_from_nv();
	if (ret)
		strncpy(g_tp_color_string, "", sizeof(""));
	complete_all(&nv_read_done);
}

static void read_tp_color_from_system(void)
{
	reinit_completion(&nv_read_done);
	schedule_work(&nv_read_work);
	if (!wait_for_completion_interruptible_timeout(&nv_read_done,
		msecs_to_jiffies(NV_READ_TIMEOUT))) {
		snprintf(g_tp_color_string, TP_COLOR_SIZE, "%s", "");
		hwlog_info("nv_read_work time out\n");
	}
	hwlog_info("%s:tp color is %s\n", __func__, g_tp_color_string);
}

static ssize_t attr_get_tp_color_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	u8 lcd_id;
	u8 phone_color;

	lcd_id = read_tp_color();
	/* 0xff: an invalid tp_color value */
	if (lcd_id != 0xff)
		hwlog_info("lcd id is %u from read tp color\n", lcd_id);
	hwlog_info("%s: tp id=%x, lcd id=%x\n", __func__,
		cypress_ts_kit_color[0], lcd_id);
	if (is_color_correct(cypress_ts_kit_color[0])) {
		phone_color = cypress_ts_kit_color[0];
	} else if (is_color_correct(lcd_id)) {
		phone_color = lcd_id;
	} else {
		read_tp_color_from_system();
		ret = snprintf(buf, TP_COLOR_SIZE, "%s", g_tp_color_string);
		hwlog_info("tp_color %s\n", buf);
		return ret;
	}
	switch (phone_color) {
	case WHITE:
	case WHITE_OLD:
		strncpy(buf, "white", sizeof("white"));
		break;
	case BLACK:
	case BLACK_OLD:
		strncpy(buf, "black", sizeof("black"));
		break;
	case PINK:
	case PINKGOLD:
		strncpy(buf, "pink", sizeof("pink"));
		break;
	case RED:
		strncpy(buf, "red", sizeof("red"));
		break;
	case YELLOW:
		strncpy(buf, "yellow", sizeof("yellow"));
		break;
	case BLUE:
		strncpy(buf, "blue", sizeof("blue"));
		break;
	case GOLD:
	case GOLD_OLD:
		strncpy(buf, "gold", sizeof("gold"));
		break;
	case SILVER:
		strncpy(buf, "silver", sizeof("silver"));
		break;
	case GRAY:
		strncpy(buf, "gray", sizeof("gray"));
		break;
	case CAFE:
	case CAFE2:
		strncpy(buf, "cafe", sizeof("cafe"));
		break;
	case BLACK2:
		strncpy(buf, "pdblack", sizeof("pdblack"));
		break;
	case GREEN:
		strncpy(buf, "green", sizeof("green"));
		break;
	default:
		strncpy(buf, "", sizeof(""));
		break;
	}
	hwlog_info("%s: TP color is %s\n", __func__, buf);
	return strlen(buf);
}

static void tp_nv_write_work_fn(struct work_struct *work)
{
	int ret;

	ret = write_tp_color_adapter(g_tp_color_string);
	if (ret < 0)
		hwlog_info("%s:write tp color failed\n", __func__);
	complete_all(&nv_write_done);
}

static ssize_t attr_set_tp_color_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;

	if (kstrtoul(buf, DEC_BASE_DATA, &val) != 0)
		hwlog_err("kstrtol failed\n");
	hwlog_info("[%s] val=%lu\n", __func__, val);
	if (val == 1)
		write_tp_color_to_nv();
	return count;
}

static struct device_attribute ts_kit_color_file =
__ATTR(ts_kit_color_info, 0664, attr_get_tp_color_info, attr_set_tp_color_info);

static struct platform_device huawei_ts_kit_color = {
	.name = "huawei_ts_kit_color",
	.id = -1,
};

#if (defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_DEVKIT_MTK_3_0) || \
	(defined CONFIG_HUAWEI_THP_QCOM) || \
	(defined CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
static struct device_attribute tp_color_file = __ATTR(tp_color_info,
	0660, attr_get_tp_color_info, attr_set_tp_color_info);

static struct platform_device huawei_tp_color = {
	.name = "huawei_tp_color",
	.id = -1,
};
#endif

static int __init ts_color_info_init(void)
{
	int ret;
#ifdef CONFIG_SENSOR_PARAM_TO_SCP
	uint8_t tp_color;
#endif

	hwlog_info("%s ++\n", __func__);

	INIT_WORK(&nv_read_work, tp_nv_read_work_fn);
	INIT_WORK(&nv_write_work, tp_nv_write_work_fn);
	init_completion(&nv_read_done);
	init_completion(&nv_write_done);
	ret = platform_device_register(&huawei_ts_kit_color);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret:%d.\n",
			__func__, ret);
		goto REGISTER_ERR;
	}
	if (device_create_file(&huawei_ts_kit_color.dev, &ts_kit_color_file)) {
		hwlog_err("%s:Unable to create interface\n", __func__);
		goto SYSFS_CREATE_FILE_ERR;
	}

#if (defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_DEVKIT_MTK_3_0) || \
	(defined CONFIG_HUAWEI_THP_QCOM) || \
	(defined CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
	ret = platform_device_register(&huawei_tp_color);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret: %d\n",
			__func__, ret);
		goto register_dev_err;
	}
	if (device_create_file(&huawei_tp_color.dev, &tp_color_file)) {
		hwlog_err("%s: Unable to create interface\n", __func__);
		goto sysfs_create_tpcolor_file_err;
	}
#endif
#ifdef CONFIG_SENSOR_PARAM_TO_SCP
	hwlog_info("%s:tp id=%x\n", __func__, cypress_ts_kit_color[0]);
	if (is_color_correct(cypress_ts_kit_color[0])) {
		tp_color = cypress_ts_kit_color[0];
		scp_power_monitor_notify(SENSOR_TP_COLOR, &tp_color);
	}
#endif

	hwlog_info("%s --\n", __func__);
	return 0;

SYSFS_CREATE_FILE_ERR:
	platform_device_unregister(&huawei_ts_kit_color);
REGISTER_ERR:
	return ret;

#if (defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_DEVKIT_MTK_3_0) || \
	(defined CONFIG_HUAWEI_THP_QCOM) || \
	(defined CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
sysfs_create_tpcolor_file_err:
	platform_device_unregister(&huawei_tp_color);
register_dev_err:
	return ret;
#endif
}

device_initcall(ts_color_info_init);
MODULE_DESCRIPTION("ts color info");
MODULE_AUTHOR("huawei driver group of k3");
MODULE_LICENSE("GPL");
