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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/module.h>
#include "tpkit_platform_adapter.h"
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG tp_color
HWLOG_REGIST();

#define WHITE 0xE1
#define WHITE_OLD 0x02
#define BLACK 0xD2
#define BLACK_OLD 0x01
#define PINK 0xC3
#define RED 0xB4
#define YELLOW 0xA5
#define BLUE 0x96
#define GOLD 0x87
#define GOLD_OLD 0x04
#define SILVER 0x3C
#define GRAY 0x78
#define CAFE 0x69
#define CAFE2 0x5A
#define BLACK2 0x4B
#define GREEN 0x2D
#define PINKGOLD 0x1E
#define TP_COLOR_BUF_SIZE 20
#define TP_COLOR_SIZE 15
#define TP_COLOR_MASK 0x0f
#define TP_COLOR_OFFSET_4BIT 4
#define NV_DATA_SIZE 104
#define DEC_BASE_DATA 10

__attribute__((weak)) char tp_color_buf[TP_COLOR_BUF_SIZE];
__attribute__((weak)) u8 cypress_ts_kit_color[TP_COLOR_SIZE];

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

static int read_tp_color_from_nv(char *color_info)
{
	int ret;
	char nv_data[NV_DATA_SIZE] = {0};

	ret = read_tp_color_adapter(nv_data, NV_DATA_SIZE);
	if (ret < 0) {
		hwlog_err("nve_direct_access read error %d\n", ret);
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
		(!strncmp(nv_data, "green", strlen("green"))) ||
		(!strncmp(nv_data, "pinkgold", strlen("pinkgold")))) {
		strncpy(color_info, nv_data, strlen(nv_data));
		color_info[strlen(nv_data)] = '\0';
		return 0;
	}
	hwlog_err("%s: read abnormal value\n", __func__);
	return -1;
}

static int write_tp_color_to_nv(void)
{
	int ret;
	u8 lcd_id;
	u8 phone_color;
	char nv_data[NV_DATA_SIZE] = {0};

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
		return -1;
	}
	switch (phone_color) {
	case WHITE:
		strncpy(nv_data, "white", sizeof(nv_data));
		break;
	case BLACK:
		strncpy(nv_data, "black", sizeof(nv_data));
		break;
	case PINK:
	case PINKGOLD:
		strncpy(nv_data, "pink", sizeof(nv_data));
		break;
	case RED:
		strncpy(nv_data, "red", sizeof(nv_data));
		break;
	case YELLOW:
		strncpy(nv_data, "yellow", sizeof(nv_data));
		break;
	case BLUE:
		strncpy(nv_data, "blue", sizeof(nv_data));
		break;
	case GOLD:
		strncpy(nv_data, "gold", sizeof(nv_data));
		break;
	case SILVER:
		strncpy(nv_data, "silver", sizeof(nv_data));
		break;
	case GRAY:
		strncpy(nv_data, "gray", sizeof(nv_data));
		break;
	case CAFE:
	case CAFE2:
		strncpy(nv_data, "cafe", sizeof(nv_data));
		break;
	case BLACK2:
		strncpy(nv_data, "pdblack", sizeof(nv_data));
		break;
	case GREEN:
		strncpy(nv_data, "green", sizeof(nv_data));
		break;
	default:
		strncpy(nv_data, "", sizeof(nv_data));
		break;
	}
	ret = write_tp_color_adapter(nv_data);
	if (ret < 0)
		return ret;
	hwlog_info("%s: %s\n", __func__, nv_data);
	return ret;
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
		ret = read_tp_color_from_nv(buf);
		if (ret)
			strncpy(buf, "", sizeof(""));
		return strlen(buf);
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

static ssize_t attr_set_tp_color_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;

	if (kstrtoul(tp_color_buf, DEC_BASE_DATA, &val) != 0)
		hwlog_err("%s call kstrtol failed\n", __func__);
	hwlog_info("%s val=%lu\n", __func__, val);
	/* 1 means to write nv to the hal layer */
	if (val == 1)
		write_tp_color_to_nv();
	return count;
}

static struct device_attribute ts_kit_color_file =
	__ATTR(ts_kit_color_info, 0664, attr_get_tp_color_info,
	attr_set_tp_color_info);

static struct platform_device huawei_ts_kit_color = {
	.name = "huawei_ts_kit_color",
	.id = -1,
};

static int __init ts_color_info_init(void)
{
	int ret;

	hwlog_info("%s ++\n", __func__);

	ret = platform_device_register(&huawei_ts_kit_color);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret: %d\n",
			__func__, ret);
		goto register_err;
	}
	if (device_create_file(&huawei_ts_kit_color.dev, &ts_kit_color_file)) {
		hwlog_err("%s: unable to create interface\n", __func__);
		goto sysfs_create_file_err;
	}
	hwlog_info("%s --\n", __func__);
	return 0;

sysfs_create_file_err:
	platform_device_unregister(&huawei_ts_kit_color);
register_err:
	return ret;
}

device_initcall(ts_color_info_init);

MODULE_DESCRIPTION("ts color info");
MODULE_AUTHOR("huawei driver group of k3");
MODULE_LICENSE("GPL");
