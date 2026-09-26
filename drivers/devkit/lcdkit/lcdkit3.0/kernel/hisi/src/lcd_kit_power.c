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
#include "hisi_fb.h"
#include "lcd_kit_common.h"

/*
 * variable
 */
/* scharg regulator */
static struct regulator *bl_vcc;
static struct regulator *bias_vcc;
static struct regulator *vsn_vcc;
static struct regulator *vsp_vcc;
/* ldo regulator */
static struct regulator *iovcc;
static struct regulator *vci;
static struct regulator *vdd;
/* global gpio */
uint32_t g_lcd_kit_gpio;
/* poweric detect gpio */
uint32_t poweric_gpio;

/*
 * power type
 */
/* scharger power */
static struct vcc_desc bias_get_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, BIAS_NAME, &bias_vcc, 0, 0, 0, 0 },
	{ DTYPE_VCC_GET, VSN_NAME, &vsn_vcc, 0, 0, 0, 0 },
	{ DTYPE_VCC_GET, VSP_NAME, &vsp_vcc, 0, 0, 0, 0 },
};

static struct vcc_desc bl_get_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, BACKLIGHT_NAME, &bl_vcc, 0, 0, 0, 0 },
};

static struct vcc_desc bias_set_cmds[] = {
	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, BIAS_NAME, &bias_vcc, 5400000, 5400000, 0, 0 },
	{ DTYPE_VCC_SET_VOLTAGE, VSP_NAME, &vsp_vcc,  5400000, 5400000, 0, 0 },
	{ DTYPE_VCC_SET_VOLTAGE, VSN_NAME, &vsn_vcc,  5400000, 5400000, 0, 0 },
};

static struct vcc_desc bias_put_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, BIAS_NAME, &bias_vcc, 0, 0, 0, 0 },
	{ DTYPE_VCC_PUT, VSN_NAME, &vsn_vcc, 0, 0, 0, 0 },
	{ DTYPE_VCC_PUT, VSP_NAME, &vsp_vcc, 0, 0, 0, 0 },
};

static struct vcc_desc bl_put_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, BACKLIGHT_NAME, &bl_vcc, 0, 0, 0, 0 },
};

static struct vcc_desc vsp_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, BIAS_NAME, &bias_vcc, 0, 0, WAIT_TYPE_MS, 0 },
	{ DTYPE_VCC_ENABLE, VSP_NAME, &vsp_vcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vsn_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VSN_NAME, &vsn_vcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vsn_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, VSN_NAME, &vsn_vcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vsp_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, VSP_NAME, &vsp_vcc, 0, 0, WAIT_TYPE_MS, 0 },
	{ DTYPE_VCC_DISABLE, BIAS_NAME, &bias_vcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc bl_enable_cmds[] = {
	/* backlight enable */
	{ DTYPE_VCC_ENABLE, BACKLIGHT_NAME, &bl_vcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc bl_disable_cmds[] = {
	/* backlight disable */
	{ DTYPE_VCC_DISABLE, BACKLIGHT_NAME, &bl_vcc, 0, 0, WAIT_TYPE_MS, 0 },
};
/* ldo power */
static struct vcc_desc iovcc_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, IOVCC_NAME, &iovcc, 0, 0, WAIT_TYPE_MS, 0 },
	/* io set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, IOVCC_NAME, &iovcc, 1800000, 1800000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, VCI_NAME, &vci, 0, 0, WAIT_TYPE_MS, 0 },
	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VCI_NAME, &vci, 2800000, 2800000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, VDD_NAME, &vdd, 0, 0, WAIT_TYPE_MS, 0 },
	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VDD_NAME, &vdd, 1250000, 1250000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, VCI_NAME, &vci, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc iovcc_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, IOVCC_NAME, &iovcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, VDD_NAME, &vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc iovcc_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, IOVCC_NAME, &iovcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VCI_NAME, &vci, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VDD_NAME, &vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc iovcc_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, IOVCC_NAME, &iovcc, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vci_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, VCI_NAME, &vci, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc vdd_disable_cmds[] = {
	/* vcc disable */
	{ DTYPE_VCC_DISABLE, VDD_NAME, &vdd, 0, 0, WAIT_TYPE_MS, 0 },
};

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

void lcd_poweric_gpio_operator(uint32_t gpio, uint32_t op)
{
	poweric_gpio = gpio;
	LCD_KIT_INFO("poweric_det gpio = %d", poweric_gpio);

	if (op == GPIO_HIGH) {
		LCD_KIT_INFO("gpio pull high %d", __LINE__);
		lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC, GPIO_HIGH);
	}
	if (op == GPIO_LOW) {
		LCD_KIT_INFO("gpio pull low %d", __LINE__);
		lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC, GPIO_LOW);
	}
	if (op == GPIO_REQ) {
		LCD_KIT_INFO("gpio request %d", __LINE__);
		lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC, GPIO_REQ);
	}
	if (op == GPIO_FREE) {
		LCD_KIT_INFO("gpio free %d", __LINE__);
		lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC, GPIO_FREE);
	}
}

int lcd_kit_gpio_tx(uint32_t type, uint32_t op)
{
	int i;
	int ret;
	struct gpio_power_arra *gpio_cm = NULL;

	if (lcd_kit_get_power_status() && (type != LCD_KIT_MIPI_SWITCH)) {
		LCD_KIT_INFO("panel is power on, not need operate gpio, type = %d, op = %d\n", type, op);
		return LCD_KIT_FAIL;
	}

	switch (type) {
	case LCD_KIT_VCI:
		g_lcd_kit_gpio = power_hdl->lcd_vci.buf[POWER_NUM];
		break;
	case LCD_KIT_IOVCC:
		g_lcd_kit_gpio = power_hdl->lcd_iovcc.buf[POWER_NUM];
		break;
	case LCD_KIT_VSP:
		g_lcd_kit_gpio = power_hdl->lcd_vsp.buf[POWER_NUM];
		break;
	case LCD_KIT_VSN:
		g_lcd_kit_gpio = power_hdl->lcd_vsn.buf[POWER_NUM];
		break;
	case LCD_KIT_RST:
		g_lcd_kit_gpio = power_hdl->lcd_rst.buf[POWER_NUM];
		break;
	case LCD_KIT_BL:
		g_lcd_kit_gpio = power_hdl->lcd_backlight.buf[POWER_NUM];
		break;
	case LCD_KIT_VDD:
		g_lcd_kit_gpio = power_hdl->lcd_vdd.buf[POWER_NUM];
		break;
	case LCD_KIT_AOD:
		g_lcd_kit_gpio = power_hdl->lcd_aod.buf[POWER_NUM];
		break;
	case LCD_KIT_POWERIC_DET_DBC:
		g_lcd_kit_gpio = poweric_gpio;
		break;
	case LCD_KIT_MIPI_SWITCH:
		g_lcd_kit_gpio = power_hdl->lcd_mipi_switch.buf[POWER_NUM];
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
int lcd_kit_pmu_ctrl(uint32_t type, uint32_t enable)
{
	int ret = LCD_KIT_OK;

	switch (type) {
	case LCD_KIT_VCI:
		if (power_hdl->lcd_vci.buf[POWER_NUM] <= 0)
			break;
		if (enable)
			ret = vcc_cmds_tx(NULL, vci_enable_cmds,
				ARRAY_SIZE(vci_enable_cmds));
		else
			ret = vcc_cmds_tx(NULL, vci_disable_cmds,
				ARRAY_SIZE(vci_disable_cmds));
		break;
	case LCD_KIT_IOVCC:
		if (power_hdl->lcd_iovcc.buf[POWER_NUM] <= 0)
			break;
		if (enable)
			ret = vcc_cmds_tx(NULL, iovcc_enable_cmds,
				ARRAY_SIZE(iovcc_enable_cmds));
		else
			ret = vcc_cmds_tx(NULL, iovcc_disable_cmds,
				ARRAY_SIZE(iovcc_disable_cmds));
		break;
	case LCD_KIT_VDD:
		if (power_hdl->lcd_vdd.buf[POWER_NUM] <= 0)
			break;
		if (enable)
			ret = vcc_cmds_tx(NULL, vdd_enable_cmds,
				ARRAY_SIZE(vdd_enable_cmds));
		else
			ret = vcc_cmds_tx(NULL, vdd_disable_cmds,
				ARRAY_SIZE(vdd_disable_cmds));
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("error type\n");
		break;
	}
	return ret;
}
int lcd_kit_pmu_ctrl_plugin(uint32_t type, uint32_t event_data)
{
	int ret = LCD_KIT_OK;

	if (event_data > 0) {
		LCD_KIT_ERR("do not handle power on event.\n");
		return LCD_KIT_FAIL;
	}

	switch (type) {
	case LCD_KIT_VCI:
		ret = vcc_cmds_tx(NULL, vci_disable_cmds,
			ARRAY_SIZE(vci_disable_cmds));
		break;
	case LCD_KIT_IOVCC:
		ret = vcc_cmds_tx(NULL, iovcc_disable_cmds,
			ARRAY_SIZE(iovcc_disable_cmds));
		break;
	case LCD_KIT_VDD:
		ret = vcc_cmds_tx(NULL, vdd_disable_cmds,
			ARRAY_SIZE(vdd_disable_cmds));
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("error type\n");
		break;
	}
	return ret;
}

int lcd_kit_charger_ctrl(uint32_t type, uint32_t enable)
{
	int ret;

	switch (type) {
	case LCD_KIT_VSP:
		if (enable)
			ret = vcc_cmds_tx(NULL, vsp_enable_cmds,
				ARRAY_SIZE(vsp_enable_cmds));
		else
			ret = vcc_cmds_tx(NULL, vsp_disable_cmds,
				ARRAY_SIZE(vsp_disable_cmds));
		break;
	case LCD_KIT_VSN:
		if (enable)
			ret = vcc_cmds_tx(NULL, vsn_enable_cmds,
				ARRAY_SIZE(vsn_enable_cmds));
		else
			ret = vcc_cmds_tx(NULL, vsn_disable_cmds,
				ARRAY_SIZE(vsn_disable_cmds));
		break;
	case LCD_KIT_BL:
		if (enable)
			ret = vcc_cmds_tx(NULL, bl_enable_cmds,
				ARRAY_SIZE(bl_enable_cmds));
		else
			ret = vcc_cmds_tx(NULL, bl_disable_cmds,
				ARRAY_SIZE(bl_disable_cmds));
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("error type\n");
		break;
	}
	return ret;
}

static void lcd_kit_power_set(struct vcc_desc *cmds, int cnt)
{
	struct vcc_desc *cm = NULL;
	int i;

	cm = cmds;
	for (i = 0; i < cnt; i++) {
		if (cm->dtype == DTYPE_VCC_SET_VOLTAGE) {
			if (!strncmp(cm->id, VCI_NAME, strlen(cm->id))) {
				cm->min_uV = power_hdl->lcd_vci.buf[POWER_VOL];
				cm->max_uV = power_hdl->lcd_vci.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, IOVCC_NAME, strlen(cm->id))) {
				cm->min_uV = power_hdl->lcd_iovcc.buf[POWER_VOL];
				cm->max_uV = power_hdl->lcd_iovcc.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, VDD_NAME, strlen(cm->id))) {
				cm->min_uV = power_hdl->lcd_vdd.buf[POWER_VOL];
				cm->max_uV = power_hdl->lcd_vdd.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, VSP_NAME, strlen(cm->id))) {
				cm->min_uV = power_hdl->lcd_vsp.buf[POWER_VOL];
				cm->max_uV = power_hdl->lcd_vsp.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, VSN_NAME, strlen(cm->id))) {
				cm->min_uV = power_hdl->lcd_vsn.buf[POWER_VOL];
				cm->max_uV = power_hdl->lcd_vsn.buf[POWER_VOL];
			}
			if (!strncmp(cm->id, BIAS_NAME, strlen(cm->id))) {
				cm->min_uV = power_hdl->lcd_vsp.buf[POWER_VOL];
				cm->max_uV = power_hdl->lcd_vsp.buf[POWER_VOL];
			}
		}
		cm++;
	}
}

static void lcd_kit_set_scharger(void)
{
	/* set scharger vcc */
	vcc_cmds_tx(NULL, bias_set_cmds, ARRAY_SIZE(bias_set_cmds));
	/* scharger vcc enable */
	vcc_cmds_tx(NULL, vsp_enable_cmds, ARRAY_SIZE(vsp_enable_cmds));
	vcc_cmds_tx(NULL, vsn_enable_cmds, ARRAY_SIZE(vsn_enable_cmds));
}

static int vsp_process(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE) {
		/* lcd scharger vcc get */
		ret = vcc_cmds_tx(pdev, bias_get_cmds,
			ARRAY_SIZE(bias_get_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD scharger vcc get failed!\n");
			return ret;
		}
		/* init bias/vsp/vsn */
		lcd_kit_power_set(bias_set_cmds, ARRAY_SIZE(bias_set_cmds));
		lcd_kit_set_scharger();
	}
	return ret;
}

static int bl_process(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_backlight.buf &&
		power_hdl->lcd_backlight.buf[POWER_MODE] == REGULATOR_MODE) {
		/* lcd scharger vcc get */
		ret = vcc_cmds_tx(pdev, bl_get_cmds, ARRAY_SIZE(bl_get_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD scharger vcc get failed!\n");
			return ret;
		}
		/* scharger bl enable */
		vcc_cmds_tx(NULL, bl_enable_cmds, ARRAY_SIZE(bl_enable_cmds));
	}
	return ret;
}

static int vci_process(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vci is valid */
		if (power_hdl->lcd_vci.buf[POWER_NUM] > 0) {
			/* init vci */
			lcd_kit_power_set(vci_init_cmds,
				ARRAY_SIZE(vci_init_cmds));
			ret = vcc_cmds_tx(pdev, vci_init_cmds,
				ARRAY_SIZE(vci_init_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD vci init failed!\n");
				return ret;
			}
			ret = vcc_cmds_tx(pdev, vci_enable_cmds,
				ARRAY_SIZE(vci_enable_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD vci enable failed!\n");
				return ret;
			}
		}
	}
	return ret;
}

static int iovcc_process(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge iovcc is valid */
		if (power_hdl->lcd_iovcc.buf[POWER_NUM] > 0) {
			/* init iovcc */
			lcd_kit_power_set(iovcc_init_cmds,
				ARRAY_SIZE(iovcc_init_cmds));
			ret = vcc_cmds_tx(pdev, iovcc_init_cmds,
				ARRAY_SIZE(iovcc_init_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD iovcc init failed!\n");
				return ret;
			}
			ret = vcc_cmds_tx(pdev, iovcc_enable_cmds,
				ARRAY_SIZE(iovcc_enable_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD iovcc enable failed!\n");
				return ret;
			}
		}
	}
	return ret;
}

static int vdd_process(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vdd is valid */
		if (power_hdl->lcd_vdd.buf[POWER_NUM] > 0) {
			/* init vdd */
			lcd_kit_power_set(vdd_init_cmds,
				ARRAY_SIZE(vdd_init_cmds));
			ret = vcc_cmds_tx(pdev, vdd_init_cmds,
				ARRAY_SIZE(vdd_init_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD vdd init failed!\n");
				return ret;
			}
			ret = vcc_cmds_tx(pdev, vdd_enable_cmds,
				ARRAY_SIZE(vdd_enable_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD vdd enable failed!\n");
				return ret;
			}
		}
	}
	return ret;
}

int lcd_kit_power_init(struct platform_device *pdev)
{
	int ret;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	ret = vsp_process(pdev);
	if (ret != 0)
		return ret;
	ret = bl_process(pdev);
	if (ret != 0)
		return ret;
	ret = vci_process(pdev);
	if (ret != 0)
		return ret;
	ret = iovcc_process(pdev);
	if (ret != 0)
		return ret;
	ret = vdd_process(pdev);
	if (ret != 0)
		return ret;
	return ret;
}

int lcd_kit_power_finit(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE)
		ret = vcc_cmds_tx(pdev, vci_finit_cmds, ARRAY_SIZE(vci_finit_cmds));
	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE)
		ret = vcc_cmds_tx(pdev, iovcc_finit_cmds, ARRAY_SIZE(iovcc_finit_cmds));
	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE)
		ret = vcc_cmds_tx(pdev, vdd_finit_cmds, ARRAY_SIZE(vdd_finit_cmds));
	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE)
		ret = vcc_cmds_tx(pdev, bias_put_cmds, ARRAY_SIZE(bias_put_cmds));
	if (power_hdl->lcd_backlight.buf &&
		power_hdl->lcd_backlight.buf[POWER_MODE] == REGULATOR_MODE)
		ret = vcc_cmds_tx(pdev, bl_put_cmds, ARRAY_SIZE(bl_put_cmds));
	return ret;
}

static int lcd_power_set_vdd_vol(void)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vdd.buf == NULL) {
		LCD_KIT_ERR("vdd buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE &&
		power_hdl->lcd_vdd.buf[POWER_NUM] > 0) {
		lcd_kit_power_set(vdd_init_cmds, ARRAY_SIZE(vdd_init_cmds));
		ret = vcc_cmds_tx(NULL, &vdd_init_cmds[1], 1);
		if (ret != 0) {
			LCD_KIT_ERR("LCD vdd init failed!\n");
			return LCD_KIT_FAIL;
		}
	}
	return ret;
}

int lcd_power_set_vol(uint32_t type)
{
	int ret;

	switch (type) {
	case LCD_KIT_VDD:
		ret = lcd_power_set_vdd_vol();
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n", type);
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

/* debug interface */
int lcd_kit_dbg_set_voltage(void)
{
	int ret = LCD_KIT_OK;

	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE) {
		/* init bias/vsp/vsn */
		lcd_kit_power_set(bias_set_cmds, ARRAY_SIZE(bias_set_cmds));
		lcd_kit_set_scharger();
	}
	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vci is valid */
		if (power_hdl->lcd_vci.buf[POWER_NUM] > 0) {
			/* init vci */
			lcd_kit_power_set(vci_init_cmds, ARRAY_SIZE(vci_init_cmds));
			ret = vcc_cmds_tx(NULL, &vci_init_cmds[1], 1);
			if (ret != 0) {
				LCD_KIT_ERR("LCD vci init failed!\n");
				return ret;
			}
		}
	}
	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge iovcc is valid */
		if (power_hdl->lcd_iovcc.buf[POWER_NUM] > 0) {
			/* init iovcc */
			lcd_kit_power_set(iovcc_init_cmds, ARRAY_SIZE(iovcc_init_cmds));
			ret = vcc_cmds_tx(NULL, &iovcc_init_cmds[1], 1);
			if (ret != 0) {
				LCD_KIT_ERR("LCD iovcc init failed!\n");
				return ret;
			}
		}
	}
	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vdd is valid */
		if (power_hdl->lcd_vdd.buf[POWER_NUM] > 0) {
			/* init vdd */
			lcd_kit_power_set(vdd_init_cmds, ARRAY_SIZE(vdd_init_cmds));
			ret = vcc_cmds_tx(NULL, &vdd_init_cmds[1], 1);
			if (ret != 0) {
				LCD_KIT_ERR("LCD vdd init failed!\n");
				return ret;
			}
		}
	}
	return ret;
}

/*
 * lcd dpd
 */
#ifdef LCD_KIT_DEBUG_ENABLE
static int dpd_vsp_init(struct platform_device *pdev)
{
	int ret;

	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE) {
		/* lcd scharger vcc get */
		ret = vcc_cmds_tx(pdev, bias_get_cmds,
			ARRAY_SIZE(bias_get_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD scharger vcc get failed!\n");
			return ret;
		}
		/* init bias/vsp/vsn */
		lcd_kit_power_set(bias_set_cmds, ARRAY_SIZE(bias_set_cmds));
		/* set scharger vcc */
		ret = vcc_cmds_tx(NULL, bias_set_cmds,
			ARRAY_SIZE(bias_set_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD scharger vcc set failed!\n");
			return ret;
		}
	}
	return LCD_KIT_OK;
}

static int dpd_bl_init(struct platform_device *pdev)
{
	int ret;

	if (power_hdl->lcd_backlight.buf &&
		power_hdl->lcd_backlight.buf[POWER_MODE] == REGULATOR_MODE) {
		/* lcd scharger vcc get */
		ret = vcc_cmds_tx(pdev, bl_get_cmds, ARRAY_SIZE(bl_get_cmds));
		if (ret != 0) {
			LCD_KIT_ERR("LCD scharger vcc get failed!\n");
			return ret;
		}
	}
	return LCD_KIT_OK;
}

static int dpd_vdd_init(struct platform_device *pdev)
{
	int ret;

	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vdd is valid */
		if (power_hdl->lcd_vdd.buf[POWER_NUM] > 0) {
			lcd_kit_power_set(vdd_init_cmds,
				ARRAY_SIZE(vdd_init_cmds));
			ret = vcc_cmds_tx(pdev, vdd_init_cmds,
				ARRAY_SIZE(vdd_init_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD vdd init failed!\n");
				return ret;
			}
		}
	}
	return LCD_KIT_OK;
}

static int dpd_iovcc_init(struct platform_device *pdev)
{
	int ret;

	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge iovcc is valid */
		if (power_hdl->lcd_iovcc.buf[POWER_NUM] > 0) {
			lcd_kit_power_set(iovcc_init_cmds,
				ARRAY_SIZE(iovcc_init_cmds));
			ret = vcc_cmds_tx(pdev, iovcc_init_cmds,
				ARRAY_SIZE(iovcc_init_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD iovcc init failed!\n");
				return ret;
			}
		}
	}
	return LCD_KIT_OK;
}

static int dpd_vci_init(struct platform_device *pdev)
{
	int ret;

	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		/* judge vci is valid */
		if (power_hdl->lcd_vci.buf[POWER_NUM] > 0) {
			/* init vci */
			lcd_kit_power_set(vci_init_cmds,
				ARRAY_SIZE(vci_init_cmds));
			ret = vcc_cmds_tx(pdev, vci_init_cmds,
				ARRAY_SIZE(vci_init_cmds));
			if (ret != 0) {
				LCD_KIT_ERR("LCD vci init failed!\n");
				return ret;
			}
		}
	}
	return LCD_KIT_OK;
}

int dpd_regu_init(struct platform_device *pdev)
{
	int ret;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	ret = dpd_vsp_init(pdev);
	if (ret) {
		LCD_KIT_ERR("dpd vsp init fail\n");
		return LCD_KIT_FAIL;
	}
	ret = dpd_bl_init(pdev);
	if (ret) {
		LCD_KIT_ERR("dpd bl init fail\n");
		return LCD_KIT_FAIL;
	}
	ret = dpd_vdd_init(pdev);
	if (ret) {
		LCD_KIT_ERR("dpd vdd init fail\n");
		return LCD_KIT_FAIL;
	}
	ret = dpd_iovcc_init(pdev);
	if (ret) {
		LCD_KIT_ERR("dpd iovcc init fail\n");
		return LCD_KIT_FAIL;
	}
	ret = dpd_vci_init(pdev);
	if (ret) {
		LCD_KIT_ERR("dpd vci init fail\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
#endif
