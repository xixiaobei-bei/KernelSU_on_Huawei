/*
 * lcd_kit_ext_power.c
 *
 * lcdkit power function for lcd driver
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

#include "lcd_kit_power.h"
#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_ext_disp.h"

/* global gpio */
uint32_t g_lcd_kit_gpio_plugin;

/* gpio power */
static struct gpio_desc gpio_req_cmds[] = {
	{
		DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 0
	},
};

static struct gpio_desc gpio_free_cmds[] = {
	{
		DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 0
	},
};

static struct gpio_desc gpio_high_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 1
	},
};

static struct gpio_desc gpio_low_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 0
	},
};

static struct gpio_power_arra gpio_power_plugin[] = {
	{ GPIO_REQ, ARRAY_SIZE(gpio_req_cmds), gpio_req_cmds },
	{ GPIO_HIGH, ARRAY_SIZE(gpio_high_cmds), gpio_high_cmds },
	{ GPIO_LOW, ARRAY_SIZE(gpio_low_cmds), gpio_low_cmds },
	{ GPIO_FREE, ARRAY_SIZE(gpio_free_cmds), gpio_free_cmds },
};

int lcd_kit_gpio_tx_plugin(uint32_t type, uint32_t op)
{
	uint32_t i;
	int ret;
	struct gpio_power_arra *gpio_cm = NULL;

	if (lcd_kit_get_power_status()) {
		LCD_KIT_INFO("not need operate gpio, type = %d, op = %d\n", type, op);
		return LCD_KIT_FAIL;
	}

	switch (type) {
	case LCD_KIT_VCI:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_vci->buf[POWER_NUM];
		break;
	case LCD_KIT_IOVCC:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_iovcc->buf[POWER_NUM];
		break;
	case LCD_KIT_RST:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_rst->buf[POWER_NUM];
		break;
	case LCD_KIT_VDD:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_vdd->buf[POWER_NUM];
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n", type);
		break;
	}

	for (i = 0; i < ARRAY_SIZE(gpio_power_plugin); i++) {
		if (gpio_power_plugin[i].oper == op) {
			gpio_cm = &gpio_power_plugin[i];
			break;
		}
	}
	if (i >= ARRAY_SIZE(gpio_power_plugin)) {
		LCD_KIT_ERR("not found cm from gpio_power\n");
		return LCD_KIT_FAIL;
	}
	if (!gpio_cm) {
		LCD_KIT_ERR("gpio_cm is null!\n");
		return LCD_KIT_FAIL;
	}
	if (gpio_cm->num > 0) {
		ret = gpio_cmds_tx(gpio_cm->cm, gpio_cm->num);
		LCD_KIT_INFO("gpio:%d ,op:%d\n", *gpio_cm->cm->gpio, op);
		return ret;
	}
	LCD_KIT_INFO("gpio is not bigger than 0\n");
	return LCD_KIT_FAIL;
}

static int lcd_kit_reset_power_off_plugin(void)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!g_pwr_off_optimize_cfg.lcd_rst->buf) {
		LCD_KIT_ERR("can not get lcd reset!\n");
		return LCD_KIT_FAIL;
	}
	switch (g_pwr_off_optimize_cfg.lcd_rst->buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable_plugin)
			ret = adapt_ops->gpio_disable_plugin(LCD_KIT_RST);
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n",
			g_pwr_off_optimize_cfg.lcd_rst->buf[POWER_MODE]);
		break;
	}
	return ret;
}

static int lcd_kit_reset_power_ctrl_plugin(int event_data)
{
	if (event_data == 0)
		return lcd_kit_reset_power_off_plugin();
	return LCD_KIT_OK;
}

static int lcd_kit_iovcc_power_ctrl_plugin(int event_data)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (event_data > 0) {
		LCD_KIT_ERR("do not handle power on event!\n");
		return LCD_KIT_FAIL;
	}
	if (!g_pwr_off_optimize_cfg.lcd_iovcc->buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}
	switch (g_pwr_off_optimize_cfg.lcd_iovcc->buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable_plugin)
			ret = adapt_ops->gpio_disable_plugin(LCD_KIT_IOVCC);
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_disable_plugin)
			ret = adapt_ops->regulator_disable_plugin(LCD_KIT_IOVCC);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd iovcc mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd iovcc mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_vci_power_ctrl_plugin(int event_data)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (event_data > 0) {
		LCD_KIT_ERR("do not handle power on event!\n");
		return LCD_KIT_FAIL;
	}
	if (!g_pwr_off_optimize_cfg.lcd_vci->buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}
	switch (g_pwr_off_optimize_cfg.lcd_vci->buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable_plugin)
			ret = adapt_ops->gpio_disable_plugin(LCD_KIT_VCI);
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_disable_plugin)
			ret = adapt_ops->regulator_disable_plugin(LCD_KIT_VCI);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd iovcc mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd iovcc mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_vdd_power_ctrl_plugin(int event_data)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (event_data > 0) {
		LCD_KIT_ERR("do not handle power on event!\n");
		return LCD_KIT_FAIL;
	}
	if (!g_pwr_off_optimize_cfg.lcd_vdd->buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}
	switch (g_pwr_off_optimize_cfg.lcd_vdd->buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable_plugin)
			ret = adapt_ops->gpio_disable_plugin(LCD_KIT_VDD);
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_disable_plugin)
			ret = adapt_ops->regulator_disable_plugin(LCD_KIT_VDD);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vdd mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vdd mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}
int lcd_kit_event_handler_plugin(void *hld, uint32_t event,
	uint32_t data, uint32_t delay)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("event = %u, data = %u, delay = %u\n", event, data, delay);
	switch (event) {
	case EVENT_VCI:
		ret = lcd_kit_vci_power_ctrl_plugin(data);
		break;
	case EVENT_IOVCC:
		ret = lcd_kit_iovcc_power_ctrl_plugin(data);
		break;
	case EVENT_RESET:
		ret = lcd_kit_reset_power_ctrl_plugin(data);
		break;
	case EVENT_VDD:
		ret = lcd_kit_vdd_power_ctrl_plugin(data);
		break;
	case EVENT_NONE:
		LCD_KIT_INFO("none event\n");
		break;
	default:
		LCD_KIT_INFO("event not exist\n");
		break;
	}
	lcd_kit_delay(delay, LCD_KIT_WAIT_MS, true);
	return ret;
}
