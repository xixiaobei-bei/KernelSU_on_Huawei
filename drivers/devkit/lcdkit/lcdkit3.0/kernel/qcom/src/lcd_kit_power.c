/*
 * lcd_kit_power.c
 *
 * lcdkit power function for lcd driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include "lcd_kit_common.h"
#include "lcd_kit_drm_panel.h"

/* variable */
/* global gpio */
uint32_t g_lcd_kit_gpio;

/* gpio power */
static struct gpio_desc gpio_req_cmds[] = {
	{
		DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 0
	},
};

static struct gpio_desc gpio_free_cmds[] = {
	{
		DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 0
	},
};

static struct gpio_desc gpio_high_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 1
	},
};

static struct gpio_desc gpio_low_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio, 0
	},
};

struct gpio_power_arra gpio_power[] = {
	{ GPIO_REQ, ARRAY_SIZE(gpio_req_cmds), gpio_req_cmds },
	{ GPIO_HIGH, ARRAY_SIZE(gpio_high_cmds), gpio_high_cmds },
	{ GPIO_LOW, ARRAY_SIZE(gpio_low_cmds), gpio_low_cmds },
	{ GPIO_FREE, ARRAY_SIZE(gpio_free_cmds), gpio_free_cmds },
};

int gpio_cmds_tx(struct gpio_desc *cmds, int cnt)
{
	int ret;
	struct gpio_desc *cm = NULL;
	int i;
	struct qcom_panel_info *plcd_kit_info = NULL;

	if (!cmds) {
		LCD_KIT_ERR("cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	cm = cmds;
	for (i = 0; i < cnt; i++) {
		if ((cm == NULL) || (cm->label == NULL)) {
			LCD_KIT_ERR("cm or cm->label is null! index=%d\n", i);
			ret = -1;
			goto error;
		}
		if (!gpio_is_valid(*(cm->gpio))) {
			LCD_KIT_ERR("gpio invalid, dtype=%d, lable=%s, gpio=%d!\n",
				cm->dtype, cm->label, *(cm->gpio));
			ret = LCD_KIT_FAIL;
			goto error;
		}
		plcd_kit_info = lcm_get_panel_info();
		if (plcd_kit_info != NULL)
			*(cm->gpio) += plcd_kit_info->gpio_offset;
		if (cm->dtype == DTYPE_GPIO_INPUT) {
			if (gpio_direction_input(*(cm->gpio)) != 0) {
				LCD_KIT_ERR("failed to gpio_direction_input, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = LCD_KIT_FAIL;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_OUTPUT) {
			if (gpio_direction_output(*(cm->gpio), cm->value) != 0) {
				LCD_KIT_ERR("failed to gpio_direction_output, label%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = LCD_KIT_FAIL;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_REQUEST) {
			if (gpio_request(*(cm->gpio), cm->label) != 0) {
				LCD_KIT_ERR("failed to gpio_request, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = LCD_KIT_FAIL;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_FREE) {
			gpio_free(*(cm->gpio));
		} else {
			LCD_KIT_ERR("dtype=%x NOT supported\n", cm->dtype);
			ret = LCD_KIT_FAIL;
			goto error;
		}
		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS)
				mdelay(cm->wait);
			else
				mdelay(cm->wait * 1000); // change s to ms
		}
		cm++;
	}
	return 0;

error:
	return ret;
}


void lcd_kit_gpio_tx(uint32_t type, uint32_t op)
{
	int i;
	struct gpio_power_arra *gpio_cm = NULL;

	switch (type) {
	case LCD_KIT_VCI:
		g_lcd_kit_gpio = power_hdl->lcd_vci.buf[1];
		break;
	case LCD_KIT_IOVCC:
		g_lcd_kit_gpio = power_hdl->lcd_iovcc.buf[1];
		break;
	case LCD_KIT_VSP:
		g_lcd_kit_gpio = power_hdl->lcd_vsp.buf[1];
		break;
	case LCD_KIT_VSN:
		g_lcd_kit_gpio = power_hdl->lcd_vsn.buf[1];
		break;
	case LCD_KIT_RST:
		g_lcd_kit_gpio = power_hdl->lcd_rst.buf[1];
		break;
	case LCD_KIT_BL:
		g_lcd_kit_gpio = power_hdl->lcd_backlight.buf[1];
		break;
	case LCD_KIT_VDD:
		g_lcd_kit_gpio = power_hdl->lcd_vdd.buf[1];
		break;
	case LCD_KIT_TP_RST:
		g_lcd_kit_gpio = power_hdl->tp_rst.buf[1];
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n", type);
		break;
	}

	for (i = 0; i < ARRAY_SIZE(gpio_power); i++) {
		if (gpio_power[i].oper == op) {
			gpio_cm = &gpio_power[i];
			break;
		}
	}
	if (i >= ARRAY_SIZE(gpio_power)) {
		LCD_KIT_ERR("not found cm from gpio_power\n");
		return;
	}
	gpio_cmds_tx(gpio_cm->cm, gpio_cm->num);
	LCD_KIT_INFO("gpio:%d ,op:%d\n", *gpio_cm->cm->gpio, op);
}

static int vci_process(struct dsi_panel *panel)
{
	int ret = LCD_KIT_OK;
	struct regulator *vreg = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (panel == NULL) {
		LCD_KIT_ERR("panel is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		vreg = devm_regulator_get(panel->parent,
			common_info->vci_vreg.name);
		ret = PTR_RET(vreg);
		if (ret) {
			LCD_KIT_ERR("failed to get %s regulator\n",
				common_info->vci_vreg.name);
			return LCD_KIT_FAIL;
		}
		common_info->vci_vreg.vreg = vreg;
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(LCD_KIT_VCI);
	}

	return ret;
}

static int iovcc_process(struct dsi_panel *panel)
{
	int ret = LCD_KIT_OK;
	struct regulator *vreg = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (panel == NULL) {
		LCD_KIT_ERR("panel is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		LCD_KIT_ERR("======%p  %sl\n", panel->parent, common_info->iovcc_vreg.name);
		vreg = devm_regulator_get(panel->parent,
			common_info->iovcc_vreg.name);
		ret = PTR_RET(vreg);
		if (ret) {
			LCD_KIT_ERR("failed to get %s regulator\n",
				common_info->iovcc_vreg.name);
			return LCD_KIT_FAIL;
		}
		common_info->iovcc_vreg.vreg = vreg;
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(LCD_KIT_IOVCC);
	}

	return ret;
}

static int vsp_process(struct dsi_panel *panel)
{
	int ret = LCD_KIT_OK;
	struct regulator *vreg = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (panel == NULL) {
		LCD_KIT_ERR("panel is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE) {
		vreg = devm_regulator_get(panel->parent,
			common_info->vsp_vreg.name);
		ret = PTR_RET(vreg);
		if (ret) {
			LCD_KIT_ERR("failed to get %s regulator\n",
				common_info->vsp_vreg.name);
			return LCD_KIT_FAIL;
		}
		common_info->vsp_vreg.vreg = vreg;
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(LCD_KIT_VSP);
	}

	return ret;
}

static int vsn_process(struct dsi_panel *panel)
{
	int ret = LCD_KIT_OK;
	struct regulator *vreg = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (panel == NULL) {
		LCD_KIT_ERR("panel is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_vsn.buf &&
		power_hdl->lcd_vsn.buf[POWER_MODE] == REGULATOR_MODE) {
		vreg = devm_regulator_get(panel->parent,
			common_info->vsn_vreg.name);
		ret = PTR_RET(vreg);
		if (ret) {
			LCD_KIT_ERR("failed to get %s regulator\n",
				common_info->vsn_vreg.name);
			return LCD_KIT_FAIL;
		}
		common_info->vsn_vreg.vreg = vreg;
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(LCD_KIT_VSN);
	}

	return ret;
}

void lcd_kit_power_init(struct dsi_panel *panel)
{
	int ret;

	if (!panel) {
		LCD_KIT_ERR("panel is null\n");
		return;
	}

	ret = vci_process(panel);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("vci_process is fail\n");
	ret = iovcc_process(panel);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("iovcc_process is fail\n");
	ret = vsp_process(panel);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("vsp_process is fail\n");
	ret = vsn_process(panel);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("vsn_process is fail\n");
}

