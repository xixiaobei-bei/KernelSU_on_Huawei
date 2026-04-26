/*
 * nt50356.c
 *
 * bias+backlight driver of NT50356
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

#include "nt50356.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#else
#include "lcm_drv.h"
#endif
#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "lcd_kit_bl.h"
#include "lcd_kit_bias.h"
#include "backlight_linear_to_exp.h"
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

static struct nt50356_voltage nt50356_vol_table[] = {
	{ 4000000, NT50356_VOL_400 },
	{ 4050000, NT50356_VOL_405 },
	{ 4100000, NT50356_VOL_410 },
	{ 4150000, NT50356_VOL_415 },
	{ 4200000, NT50356_VOL_420 },
	{ 4250000, NT50356_VOL_425 },
	{ 4300000, NT50356_VOL_430 },
	{ 4350000, NT50356_VOL_435 },
	{ 4400000, NT50356_VOL_440 },
	{ 4450000, NT50356_VOL_445 },
	{ 4500000, NT50356_VOL_450 },
	{ 4550000, NT50356_VOL_455 },
	{ 4600000, NT50356_VOL_460 },
	{ 4650000, NT50356_VOL_465 },
	{ 4700000, NT50356_VOL_470 },
	{ 4750000, NT50356_VOL_475 },
	{ 4800000, NT50356_VOL_480 },
	{ 4850000, NT50356_VOL_485 },
	{ 4900000, NT50356_VOL_490 },
	{ 4950000, NT50356_VOL_495 },
	{ 5000000, NT50356_VOL_500 },
	{ 5050000, NT50356_VOL_505 },
	{ 5100000, NT50356_VOL_510 },
	{ 5150000, NT50356_VOL_515 },
	{ 5200000, NT50356_VOL_520 },
	{ 5250000, NT50356_VOL_525 },
	{ 5300000, NT50356_VOL_530 },
	{ 5350000, NT50356_VOL_535 },
	{ 5400000, NT50356_VOL_540 },
	{ 5450000, NT50356_VOL_545 },
	{ 5500000, NT50356_VOL_550 },
	{ 5550000, NT50356_VOL_555 },
	{ 5600000, NT50356_VOL_560 },
	{ 5650000, NT50356_VOL_565 },
	{ 5700000, NT50356_VOL_570 },
	{ 5750000, NT50356_VOL_575 },
	{ 5800000, NT50356_VOL_580 },
	{ 5850000, NT50356_VOL_585 },
	{ 5900000, NT50356_VOL_590 },
	{ 5950000, NT50356_VOL_595 },
	{ 6000000, NT50356_VOL_600 },
	{ 6050000, NT50356_VOL_605 },
	{ 6400000, NT50356_VOL_640 },
	{ 6450000, NT50356_VOL_645 },
	{ 6500000, NT50356_VOL_650 },
	{ 6550000, NT50356_VOL_655 },
	{ 6600000, NT50356_VOL_660 },
	{ 6650000, NT50356_VOL_665 },
	{ 6700000, NT50356_VOL_670 },
	{ 6750000, NT50356_VOL_675 },
	{ 6800000, NT50356_VOL_680 },
	{ 6850000, NT50356_VOL_685 },
	{ 6900000, NT50356_VOL_690 },
	{ 6950000, NT50356_VOL_695 },
	{ 7000000, NT50356_VOL_700 },
};

static char *nt50356_dts_string[NT50356_RW_REG_MAX] = {
	"nt50356_bl_config_1",
	"nt50356_bl_config_2",
	"nt50356_bl_brightness_lsb",
	"nt50356_bl_brightness_msb",
	"nt50356_auto_freq_low",
	"nt50356_auto_freq_high",
	"nt50356_reg_config_e8",
	"nt50356_reg_config_e9",
	"nt50356_reg_config_a9",
	"nt50356_display_bias_config_2",
	"nt50356_display_bias_config_3",
	"nt50356_lcm_boost_bias",
	"nt50356_vpos_bias",
	"nt50356_vneg_bias",
	"nt50356_bl_option_1",
	"nt50356_bl_option_2",
	"nt50356_bl_current_config",
	"nt50356_display_bias_config_1",
	"nt50356_bl_en",
};

static unsigned int nt50356_reg_addr[NT50356_RW_REG_MAX] = {
	REG_BL_CONFIG_1,
	REG_BL_CONFIG_2,
	REG_BL_BRIGHTNESS_LSB,
	REG_BL_BRIGHTNESS_MSB,
	REG_AUTO_FREQ_LOW,
	REG_AUTO_FREQ_HIGH,
	NT50356_REG_CONFIG_E8,
	NT50356_REG_CONFIG_E9,
	NT50356_REG_CONFIG_A9,
	REG_DISPLAY_BIAS_CONFIG_2,
	REG_DISPLAY_BIAS_CONFIG_3,
	REG_LCM_BOOST_BIAS,
	REG_VPOS_BIAS,
	REG_VNEG_BIAS,
	REG_BL_OPTION_1,
	REG_BL_OPTION_2,
	REG_BL_CURRENT_CONFIG,
	REG_DISPLAY_BIAS_CONFIG_1,
	REG_BL_ENABLE,
};

struct class *nt50356_class = NULL;
struct nt50356_chip_data *nt50356_g_chip = NULL;
static bool nt50356_init_status;
static struct nt50356_backlight_information g_bl_info;
static int nt50356_fault_check_support;
static int nt50356_10000_bl;
unsigned int nt50356_msg_level = 7;
#ifndef CONFIG_DRM_MEDIATEK
extern struct LCM_DRIVER lcdkit_mtk_common_panel;
#endif
static int nt50356_reg[NT50356_RW_REG_MAX] = { 0 };
#if defined(CONFIG_HUAWEI_DSM)
static struct nt50356_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};
#endif
module_param_named(debug_nt50356_msg_level, nt50356_msg_level, int, 0644);
MODULE_PARM_DESC(debug_nt50356_msg_level, "backlight nt50356 msg level");

static int nt50356_parse_dts(struct device_node *np)
{
	int ret;
	int i;

	if (np == NULL) {
		nt50356_err("np is NULL pointer\n");
		return -1;
	}

	for (i = 0; i < NT50356_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, nt50356_dts_string[i],
			&nt50356_reg[i]);
		if (ret < 0)
			nt50356_err("get nt50356 dts config failed\n");
		else
			nt50356_info("get %s from dts value = 0x%x\n",
				nt50356_dts_string[i], nt50356_reg[i]);
	}
	if (of_property_read_u32(np, "nt50356_check_fault_support",
		&nt50356_fault_check_support) < 0)
		nt50356_info("No need to detect fault flags!\n");

	ret = of_property_read_u32(np, "nt50356_10000_bl", &nt50356_10000_bl);
	if (ret < 0)
		nt50356_10000_bl = 0;

	ret = of_property_read_u32(np, "nt50356_dimming_support",
		&g_bl_info.bl_dimming_support);
	if (ret < 0)
		nt50356_err("get bl_dimming_support dts config failed\n");
	ret = of_property_read_u32(np, "nt50356_dimming_config",
		&g_bl_info.bl_dimming_config);
	if (ret < 0)
		nt50356_err("get bl_dimming_config dts config failed\n");
	ret = of_property_read_u32(np, "nt50356_dimming_resume",
		&g_bl_info.bl_dimming_resume);
	if (ret < 0)
		nt50356_err("get bl_dimming_resume dts config failed\n");

	return LCD_KIT_OK;
}

static int nt50356_config_write(struct nt50356_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = -1;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		nt50356_err("pchip or reg or val is NULL pointer\n");
		return ret;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_write(pchip->regmap, reg[i], val[i]);
		if (ret < 0) {
			nt50356_err("write nt50356 bl config reg 0x%x failed\n",
				reg[i]);
			return ret;
		} else {
			nt50356_info("register 0x%x value = 0x%x\n", reg[i],
				val[i]);
		}
	}

	return ret;
}

static int nt50356_parsr_default_bias(struct device_node *np)
{
	int ret;

	if (np == NULL) {
		nt50356_err("np is NULL pointer\n");
		return -1;
	}

	ret = of_property_read_u32(np, NT50356_PULL_DOWN_BOOST,
		&g_bl_info.nt50356_pull_down_boost);
	if (ret) {
		nt50356_err("nt50356 parse default pull down boost failed!\n");
		return ret;
	}

	ret = of_property_read_u32(np, NT50356_PULL_UP_BOOST,
		&g_bl_info.nt50356_pull_up_boost);
	if (ret) {
		nt50356_err("nt50356 parse default pull up boost failed!\n");
		return ret;
	}

	ret = of_property_read_u32(np, NT50356_ENABLE_VSP_VSP,
		&g_bl_info.nt50356_enable_vsp_vsn);
	if (ret) {
		nt50356_err("nt50356 parse default enable vsp vsn failed!\n");
		return ret;
	}

	return ret;
}

static void nt50356_get_target_voltage(int *vpos, int *vneg)
{
	int i;
	int tal_size = sizeof(nt50356_vol_table) / sizeof(struct nt50356_voltage);

	if ((vpos == NULL) || (vneg == NULL)) {
		LCD_KIT_ERR("vpos or vneg is null\n");
		return;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (nt50356_vol_table[i].voltage == power_hdl->lcd_vsp.buf[2]) {
			*vpos = nt50356_vol_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsp voltage, use default voltage\n");
		*vpos = NT50356_VOL_600;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (nt50356_vol_table[i].voltage == power_hdl->lcd_vsn.buf[2]) {
			*vneg = nt50356_vol_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsn voltage, use default voltage\n");
		*vneg = NT50356_VOL_600;
	}

	LCD_KIT_INFO("*vpos_target=0x%x,*vneg_target=0x%x\n", *vpos, *vneg);

	return;
}

/* initialize chip */
static int nt50356_chip_init(struct nt50356_chip_data *pchip)
{
	int ret;
	struct device_node *np = NULL;
	struct mtk_panel_info *panel_info = NULL;
	int vpos_target = 0;
	int vneg_target = 0;

	nt50356_info("in!\n");

	if (pchip == NULL) {
		nt50356_err("pchip is NULL pointer\n");
		return -1;
	}

	memset(&g_bl_info, 0, sizeof(struct nt50356_backlight_information));

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_NT50356);
	if (!np) {
		nt50356_err("NOT FOUND device node %s!\n", DTS_COMP_NT50356);
		return -1;
	}

	ret = nt50356_parse_dts(np);
	if (ret < 0) {
		nt50356_err("parse nt50356 dts failed");
		return -1;
	}

	ret = of_property_read_u32(np, NT50356_ONLY_BACKLIGHT, &g_bl_info.bl_only);
	if (ret < 0) {
		nt50356_info("parse  only_backlight fail\n");
		g_bl_info.bl_only = 0;
	}

	if (g_bl_info.bl_only == 0) {
		nt50356_get_target_voltage(&vpos_target, &vneg_target);
		/* 12 is the position of vsp in nt50356_reg */
		/* 13 is the position of vsn in nt50356_reg */
		if (nt50356_reg[12] != vpos_target)
			nt50356_reg[12] = vpos_target;
		if (nt50356_reg[13] != vneg_target)
			nt50356_reg[13] = vneg_target;
	}

	ret = of_property_read_u32(np, NT50356_HW_ENABLE, &g_bl_info.nt50356_hw_en);
	if (ret < 0) {
		nt50356_err("get nt50356_hw_en dts config failed\n");
		g_bl_info.nt50356_hw_en = 0;
	}
	if (g_bl_info.nt50356_hw_en != 0) {
		ret = of_property_read_u32(np, NT50356_HW_EN_GPIO, &g_bl_info.nt50356_hw_en_gpio);
		if (ret < 0) {
			nt50356_err("get nt50356_hw_en_gpio dts config fail\n");
		} else {
			/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
			panel_info = lcm_get_panel_info();
#else
			panel_info = lcdkit_mtk_common_panel.panel_info;
#endif
			if (!panel_info) {
				nt50356_err("panel_info is NULL\n");
				return -1;
			}
			g_bl_info.nt50356_hw_en_gpio += panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, NT50356_HW_EN_PULL_LOW,
			&g_bl_info.nt50356_hw_en_pull_low);
		if (ret < 0) {
			nt50356_err("get nt50356_hw_en_pull_low dts config fail\n");
			g_bl_info.nt50356_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, NT50356_HW_EN_DELAY, &g_bl_info.bl_on_kernel_mdelay);
	if (ret < 0)
		nt50356_err("get bl_on_kernel_mdelay dts config failed\n");

	ret = of_property_read_u32(np, NT50356_PULL_BOOST_SUPPORT,
		&g_bl_info.nt50356_pull_boost_support);
	if (ret)
		g_bl_info.nt50356_pull_boost_support = 0;
	if (g_bl_info.nt50356_pull_boost_support) {
		ret = nt50356_parsr_default_bias(np);
		if (ret) {
			nt50356_err("parse default bias failed!\n");
			g_bl_info.nt50356_pull_boost_support = 0;
		}
	}
	nt50356_info("nt50356_pull_boost_support = %d\n",
		g_bl_info.nt50356_pull_boost_support);

	ret = of_property_read_u32(np, NT50356_VTC_LINE_BOOST,
		&g_bl_info.nt50356_vtc_line_boost);
	if (ret)
		nt50356_err("nt50356 parse vtc line boost failed!\n");
	ret = of_property_read_u32(np, NT50356_VTC_END_BOOST,
		&g_bl_info.nt50356_vtc_end_boost);
	if (ret)
		nt50356_err("nt50356 parse vtc end boost failed!\n");

	nt50356_info("ok!\n");

	return LCD_KIT_OK;
}

static void nt50356_get_bias_config(int vsp, int vsn,
	int *outvsp, int *outvsn)
{
	int i;
	int vol_size = ARRAY_SIZE(nt50356_vol_table);

	for (i = 0; i < vol_size; i++) {
		if (nt50356_vol_table[i].voltage == vsp) {
			*outvsp = nt50356_vol_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		nt50356_err("not found vsn voltage, use default voltage\n");
		*outvsp = NT50356_VOL_550;
	}

	for (i = 0; i < vol_size; i++) {
		if (nt50356_vol_table[i].voltage == vsn) {
			*outvsn = nt50356_vol_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		nt50356_err("not found vsn voltage, use default voltage\n");
		*outvsn = NT50356_VOL_550;
	}
	nt50356_info("vpos = 0x%x, vneg = 0x%x\n", *outvsp, *outvsn);
}

static int nt50356_set_ic_disable(void)
{
	int ret;

	if (!nt50356_g_chip)
		return -1;

	/* reset backlight ic */
	ret = regmap_write(nt50356_g_chip->regmap, REG_BL_ENABLE, BL_RESET);
	if (ret < 0)
		nt50356_err("regmap_write REG_BL_ENABLE fail\n");
	/* set brightness need 1ms delay after config reset reg */
	mdelay(1);
	/* clean up bl val register */
	ret = regmap_update_bits(nt50356_g_chip->regmap,
			REG_BL_BRIGHTNESS_LSB, MASK_BL_LSB, BL_DISABLE);
	if (ret < 0)
		nt50356_err("regmap_update_bits REG_BL_BRIGHTNESS_LSB fail\n");

	ret = regmap_write(nt50356_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
			BL_DISABLE);
	if (ret < 0)
		nt50356_err("regmap_write REG_BL_BRIGHTNESS_MSB fail!\n");
	if (!ret)
		nt50356_info("nt50356_set_ic_disable successful!\n");
	if (g_bl_info.nt50356_hw_en != 0 &&
		g_bl_info.nt50356_hw_en_pull_low != 0) {
		gpio_set_value(g_bl_info.nt50356_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(g_bl_info.nt50356_hw_en_gpio);
	}
	return ret;
}

static int nt50356_set_bias_power_down(int vpos, int vneg)
{
	int ret = 0;
	int vsp;
	int vsn;

	if (!nt50356_g_chip)
		return -1;

	if (!g_bl_info.nt50356_pull_boost_support) {
		nt50356_info("No need to pull down BOOST!\n");
		return ret;
	}

	nt50356_get_bias_config(vpos, vneg, &vsp, &vsn);

	if (!nt50356_g_chip)
		return -1;

	ret = regmap_write(nt50356_g_chip->regmap, REG_VPOS_BIAS, vsp);
	if (ret < 0) {
		nt50356_err("write pull_down_vsp failed!\n");
		return ret;
	}

	ret = regmap_write(nt50356_g_chip->regmap, REG_VNEG_BIAS, vsn);
	if (ret < 0) {
		nt50356_err("write pull_down_vsn failed!\n");
		return ret;
	}

	ret = regmap_write(nt50356_g_chip->regmap,
		REG_DISPLAY_BIAS_CONFIG_1,
		g_bl_info.nt50356_enable_vsp_vsn);
	if (ret < 0) {
		nt50356_err("write enable_vsp_vsn failed!\n");
		return ret;
	}

	ret = regmap_write(nt50356_g_chip->regmap,
		REG_LCM_BOOST_BIAS, g_bl_info.nt50356_pull_down_boost);
	if (ret < 0) {
		nt50356_err("write enable_vsp_vsn failed!\n");
		return ret;
	}
	nt50356_info("lcd_kit_pull_boost_for_nt50356 is successful\n");

	return ret;
}

static int nt50356_set_bias(int vpos, int vneg)
{
	int ret;

	if ((vpos < 0) || (vneg < 0)) {
		nt50356_err("vpos or vneg is error\n");
		return -1;
	}

	if (!nt50356_g_chip)
		return -1;
	if (g_bl_info.nt50356_hw_en != 0 &&
		g_bl_info.nt50356_hw_en_pull_low != 0) {
		ret = gpio_request(g_bl_info.nt50356_hw_en_gpio, NULL);
		if (ret)
			nt50356_err("nt50356 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(g_bl_info.nt50356_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			nt50356_err("nt50356 set gpio output not success\n");
		gpio_set_value(g_bl_info.nt50356_hw_en_gpio, GPIO_OUT_ONE);
		if (g_bl_info.bl_on_kernel_mdelay)
			mdelay(g_bl_info.bl_on_kernel_mdelay);
	}
	ret = nt50356_config_write(nt50356_g_chip, nt50356_reg_addr,
			nt50356_reg, NT50356_RW_REG_MAX);
	if (ret < 0) {
		nt50356_err("nt50356 config register failed");
		return ret;
	}
	nt50356_info("nt50356_set_bias is successful\n");
	return ret;
}

static void nt50356_check_fault(struct nt50356_chip_data *pchip,
	int last_level, int level)
{
	unsigned int val = 0;
	int ret;
	int i;

	nt50356_info("backlight check FAULT_FLAG!\n");

	ret = regmap_read(pchip->regmap, REG_FLAGS, &val);
	if (ret < 0) {
		nt50356_info("read NT50356 FAULT_FLAG failed!\n");
		return;
	}
	if (val)
		LCD_KIT_ERR("check FAULT_FLAG error: val=0x%x\n", val);

#if defined CONFIG_HUAWEI_DSM
	for (i = 0; i < FLAG_CHECK_NUM; i++) {
		if (!(err_table[i].flag & val))
			continue;
		nt50356_err("last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret) {
			nt50356_err("dsm_client_ocuppy fail: ret=%d!\n", ret);
			continue;
		}
		dsm_client_record(lcd_dclient,
			"nt50356 last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		dsm_client_notify(lcd_dclient, err_table[i].err_no);
	}
#endif
}

static void backlight_ic_check_fault(struct nt50356_chip_data *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		nt50356_err("pchip is NULL\n");
		return;
	}
	/* Judge power on or power off */
	if (nt50356_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		nt50356_check_fault(pchip, last_level, level);
	last_level = level;
}

static int nt50356_chip_enable(struct nt50356_chip_data *pchip)
{
	int ret = -1;

	nt50356_info("chip enable in!\n");

	if (pchip == NULL) {
		nt50356_err("pchip is NULL pointer\n");
		return ret;
	}

	ret =  nt50356_config_write(pchip, nt50356_reg_addr,
		nt50356_reg, NT50356_RW_REG_MAX);
	if (ret < 0) {
		nt50356_err("nt50356 i2c config register failed");
		return ret;
	}

	nt50356_info("chip enable ok!\n");
	return ret;
}
static void nt50356_enable(void)
{
	int ret;

	if (g_bl_info.nt50356_hw_en != 0 &&
		g_bl_info.nt50356_hw_en_pull_low != 0) {
		ret = gpio_request(g_bl_info.nt50356_hw_en_gpio, NULL);
		if (ret)
			nt50356_err("nt50356 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(g_bl_info.nt50356_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			nt50356_err("nt50356 set gpio output not success\n");
		gpio_set_value(g_bl_info.nt50356_hw_en_gpio, GPIO_OUT_ONE);
		if (g_bl_info.bl_on_kernel_mdelay)
			mdelay(g_bl_info.bl_on_kernel_mdelay);
	}

	/* chip initialize */
	ret = nt50356_chip_enable(nt50356_g_chip);
	if (ret < 0) {
		nt50356_err("nt50356_chip_init fail!\n");
		return;
	}
	nt50356_init_status = true;
}

static void nt50356_disable(void)
{
	if (g_bl_info.nt50356_hw_en != 0 &&
		g_bl_info.nt50356_hw_en_pull_low != 0) {
		gpio_set_value(g_bl_info.nt50356_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(g_bl_info.nt50356_hw_en_gpio);
	}
	nt50356_init_status = false;
}

static void nt50356_dync_dimming_config(unsigned int bl_level)
{
	int ret = -1;
	static int bl_work_state = BL_POWER_ON;

	if (g_bl_info.bl_dimming_support != BL_DIMMING_SUPPORT)
		return;
	if (bl_level == 0) {
		if (g_bl_info.bl_dimming_resume != 0) {
			ret = regmap_write(nt50356_g_chip->regmap,
				REG_BL_CONFIG_2,
				g_bl_info.bl_dimming_resume);
			if (ret < 0)
				nt50356_err("dync dimming config error\n");
		}
		bl_work_state = BL_POWER_OFF;
		return;
	}
	if (bl_work_state == BL_POWER_OFF) {
		bl_work_state = BL_POWER_ON;
	} else if (bl_work_state == BL_POWER_ON) {
		if (g_bl_info.bl_dimming_config != 0) {
			ret = regmap_write(nt50356_g_chip->regmap,
				REG_BL_CONFIG_2,
				g_bl_info.bl_dimming_config);
			if (ret < 0)
				nt50356_err("dync dimming config error\n");
			nt50356_info("config bl dimming reg[%d]=0x%x\n",
				REG_BL_CONFIG_2, g_bl_info.bl_dimming_config);
		}
		bl_work_state = BL_CONFIG_LEVEL_POWER_ON;
	}
}

/*
 * nt50356_set_backlight: Set Backlight working mode
 *
 * @bl_level: value for backlight ,range from 0 to 2047
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
int nt50356_set_backlight(uint32_t bl_level)
{
	ssize_t ret = -1;
	uint32_t level;
	unsigned int bl_msb;
	unsigned int bl_lsb;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *panel_info = NULL;
#endif

	if (!nt50356_g_chip) {
		nt50356_err("init fail, return.\n");
		return ret;
	}
	if (down_trylock(&(nt50356_g_chip->test_sem))) {
		nt50356_info("Now in test mode\n");
		return 0;
	}

	/* first set backlight, enable nt50356 */
	if (g_bl_info.bl_only != 0) {
		if ((nt50356_init_status == false) && (bl_level > 0))
			nt50356_enable();
	}

	nt50356_info("nt50356_set_backlight bl_level = %u\n", bl_level);

	if (nt50356_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > BL_MAX)
			level = BL_MAX;
	}
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		nt50356_err("panel_info is NULL\n");
		up(&(nt50356_g_chip->test_sem));
		return ret;
	}
	if (panel_info->bl_ic_ctrl_mode == MTK_MIPI_BL_IC_PWM_MODE) {
		nt50356_info("nt50356 mipi pwm\n");
		backlight_ic_check_fault(nt50356_g_chip, level);
		up(&(nt50356_g_chip->test_sem));
		return LCD_KIT_OK;
	}
#endif

	nt50356_dync_dimming_config(bl_level);
	/* 11-bit brightness code */
	bl_msb = level >> MSB;
	bl_lsb = level & LSB;

	ret = regmap_update_bits(nt50356_g_chip->regmap, REG_BL_BRIGHTNESS_LSB,
		MASK_BL_LSB, bl_lsb);
	if (ret < 0)
		goto i2c_error;

	ret = regmap_write(nt50356_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
		bl_msb);
	if (ret < 0)
		goto i2c_error;

	/* if set backlight level 0, disable nt50356 */
	if (g_bl_info.bl_only != 0) {
		if ( nt50356_init_status == true && bl_level == 0)
			nt50356_disable();
	}
	backlight_ic_check_fault(nt50356_g_chip, level);
	up(&(nt50356_g_chip->test_sem));
	nt50356_info("nt50356_set_backlight exit succ\n");
	return ret;

i2c_error:
	up(&(nt50356_g_chip->test_sem));
	lcd_backlight_i2c_dmd();
	dev_err(nt50356_g_chip->dev, "%s:i2c access fail to register\n",
		__func__);
	nt50356_info("nt50356_set_backlight exit fail\n");
	return ret;
}

/*
 * nt50356_set_reg(): Set nt50356 reg
 *
 * @bl_reg: which reg want to write
 * @bl_mask: which bits of reg want to change
 * @bl_val: what value want to write to the reg
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t nt50356_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val)
{
	ssize_t ret = -1;
	u8 reg = bl_reg;
	u8 mask = bl_mask;
	u8 val = bl_val;

	if (!nt50356_init_status) {
		nt50356_err("init fail, return.\n");
		return ret;
	}

	if (reg < REG_MAX) {
		nt50356_err("Invalid argument!!!\n");
		return ret;
	}

	nt50356_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask,
		val);

	ret = regmap_update_bits(nt50356_g_chip->regmap, reg, mask, val);
	if (ret < 0) {
		nt50356_err("i2c access fail to register\n");
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(nt50356_set_reg);

static ssize_t nt50356_reg_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct nt50356_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret;
	unsigned int bl_lsb = 0;
	unsigned int bl_msb = 0;
	int bl_level;

	if (!dev)
		return snprintf(buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (!pchip)
		return snprintf(buf, PAGE_SIZE, "data is null\n");

	client = pchip->client;
	if (!client)
		return snprintf(buf, PAGE_SIZE, "client is null\n");

	ret = regmap_read(pchip->regmap, REG_BL_BRIGHTNESS_MSB, &bl_msb);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "NT50356 I2C read error\n");

	ret = regmap_read(pchip->regmap, REG_BL_BRIGHTNESS_LSB, &bl_lsb);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "NT50356 I2C read error\n");

	bl_level = (bl_msb << MSB) | bl_lsb;

	return snprintf(buf, PAGE_SIZE, "NT50356 bl_level:%d\n", bl_level);
}

static ssize_t nt50356_reg_bl_store(struct device *dev,
	struct device_attribute *dev_attr, const char *buf, size_t size)
{
	ssize_t ret;
	struct nt50356_chip_data *pchip = NULL;
	unsigned int bl_level = 0;
	unsigned int bl_msb;
	unsigned int bl_lsb;

	if (!dev)
		return snprintf((char *)buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (!pchip)
		return snprintf((char *)buf, PAGE_SIZE, "data is null\n");

	ret = kstrtouint(buf, 10, &bl_level); // 10 means decimal
	if (ret)
		goto out_input;

	nt50356_info("%s:buf=%s,state=%d\n", __func__, buf, bl_level);

	if (bl_level > BL_MAX)
		bl_level = BL_MAX;

	/* 11-bit brightness code */
	bl_msb = bl_level >> MSB;
	bl_lsb = bl_level & LSB;

	nt50356_info("bl_level = %d, bl_msb = %d, bl_lsb = %d\n", bl_level,
		bl_msb, bl_lsb);

	ret = regmap_update_bits(pchip->regmap, REG_BL_BRIGHTNESS_LSB,
		MASK_BL_LSB, bl_lsb);
	if (ret < 0)
		goto i2c_error;

	ret = regmap_write(pchip->regmap, REG_BL_BRIGHTNESS_MSB, bl_msb);
	if (ret < 0)
		goto i2c_error;

	return size;

i2c_error:
	dev_err(pchip->dev, "%s:i2c access fail to register\n", __func__);
	return snprintf((char *)buf, PAGE_SIZE,
		"%s: i2c access fail to register\n", __func__);

out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	return snprintf((char *)buf, PAGE_SIZE,
		"%s: input conversion fail\n", __func__);
}

static DEVICE_ATTR(reg_bl, 0644, nt50356_reg_bl_show,
	nt50356_reg_bl_store);

static ssize_t nt50356_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct nt50356_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret;
	unsigned char val[REG_MAX] = { 0 };

	if (!dev)
		return snprintf(buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (!pchip)
		return snprintf(buf, PAGE_SIZE, "data is null\n");

	client = pchip->client;
	if (!client)
		return snprintf(buf, PAGE_SIZE, "client is null\n");

	ret = regmap_bulk_read(pchip->regmap, REG_REVISION, &val[0], REG_MAX);
	if (ret < 0)
		goto i2c_error;

	return snprintf(buf, PAGE_SIZE, "Revision(0x01)= 0x%x\nBacklight Configuration1(0x02)= 0x%x\n"
			"\rBacklight Configuration2(0x03) = 0x%x\nBacklight Brightness LSB(0x04) = 0x%x\n"
			"\rBacklight Brightness MSB(0x05) = 0x%x\nBacklight Auto-Frequency Low(0x06) = 0x%x\n"
			"\rBacklight Auto-Frequency High(0x07) = 0x%x\nBacklight Enable(0x08) = 0x%x\n"
			"\rDisplay Bias Configuration 1(0x09)  = 0x%x\nDisplay Bias Configuration 2(0x0A)  = 0x%x\n"
			"\rDisplay Bias Configuration 3(0x0B) = 0x%x\nLCM Boost Bias Register(0x0C) = 0x%x\n"
			"\rVPOS Bias Register(0x0D) = 0x%x\nVNEG Bias Register(0x0E) = 0x%x\n"
			"\rFlags Register(0x0F) = 0x%x\nBacklight Option 1 Register(0x10) = 0x%x\n"
			"\rBacklight Option 2 Register(0x11) = 0x%x\nBL CURRENT CONFIG(0x20) = 0x%x\n",
			val[0], val[1], val[2], val[3], val[4], val[5], val[6],
			val[7], val[8], val[9], val[10], val[11], val[12],
			val[13], val[14], val[15], val[16], val[31]);
			/* 0~18 means number of registers */

i2c_error:
	return snprintf(buf, PAGE_SIZE, "%s: i2c access fail to register\n",
		__func__);
}

static ssize_t nt50356_reg_store(struct device *dev,
	struct device_attribute *dev_attr, const char *buf, size_t size)
{
	ssize_t ret;
	struct nt50356_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!dev)
		return snprintf((char *)buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (!pchip)
		return snprintf((char *)buf, PAGE_SIZE, "data is null\n");

	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		pr_err("check your input!!!\n");
		goto out_input;
	}

	if (reg > REG_MAX) {
		pr_err("Invalid argument!!!\n");
		goto out_input;
	}

	nt50356_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg,
		mask, val);

	ret = regmap_update_bits(pchip->regmap, reg, mask, val);
	if (ret < 0)
		goto i2c_error;

	return size;

i2c_error:
	dev_err(pchip->dev, "%s:i2c access fail to register\n", __func__);
	return snprintf((char *)buf, PAGE_SIZE,
		"%s: i2c access fail to register\n", __func__);

out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	return snprintf((char *)buf, PAGE_SIZE, "%s: input conversion fail\n",
		__func__);
}

static DEVICE_ATTR(reg, 0644, nt50356_reg_show,
	nt50356_reg_store);

static const struct regmap_config nt50356_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

/* pointers to created device attributes */
static struct attribute *nt50356_attributes[] = {
	&dev_attr_reg_bl.attr,
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group nt50356_group = {
	.attrs = nt50356_attributes,
};

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = nt50356_set_bias,
	.set_bias_power_down = nt50356_set_bias_power_down,
	.set_ic_disable = nt50356_set_ic_disable,
};

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = nt50356_set_backlight,
	.name = "nt50356",
};

static int nt50356_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct nt50356_chip_data *pchip = NULL;
	int ret;

	nt50356_info("in!\n");
	if (client == NULL) {
		nt50356_err("client is NULL pointer\n");
		return -1;
	}

	adapter = client->adapter;
	if (adapter == NULL) {
		nt50356_err("adapter is NULL pointer\n");
		return -1;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev, sizeof(struct nt50356_chip_data),
		GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &nt50356_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "fail : allocate register map: %d\n",
			ret);
		goto err_out;
	}
#endif

	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	/* chip initialize */
	ret = nt50356_chip_init(pchip);
	if (ret < 0) {
		dev_err(&client->dev, "fail : chip init\n");
		goto err_out;
	}

	pchip->dev = device_create(nt50356_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		nt50356_err("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &nt50356_group);
		if (ret)
			goto err_sysfs;
	}

	nt50356_g_chip = pchip;
	if (g_bl_info.nt50356_hw_en) {
		ret = gpio_request(g_bl_info.nt50356_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("nt50356 Could not request hw_en_gpio\n");
	}
	if (g_bl_info.bl_only != 0) {
		lcd_kit_bl_register(&bl_ops);
	} else {
		lcd_kit_bias_register(&bias_ops);
		lcd_kit_bl_register(&bl_ops);
	}

	nt50356_init_status = true;

	return ret;

err_sysfs:
	device_destroy(nt50356_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int nt50356_remove(struct i2c_client *client)
{
	struct nt50356_chip_data *pchip = NULL;

	if (client == NULL) {
		nt50356_err("client is NULL pointer\n");
		return -1;
	}
	pchip = i2c_get_clientdata(client);

	regmap_write(pchip->regmap, REG_BL_ENABLE, BL_DISABLE);

	sysfs_remove_group(&client->dev.kobj, &nt50356_group);

	return 0;
}

static const struct i2c_device_id nt50356_id[] = {
	{ NT50356_NAME, 0 },
	{}
};

static const struct of_device_id nt50356_of_id_table[] = {
	{ .compatible = "nt,nt50356" },
	{},
};

MODULE_DEVICE_TABLE(i2c, nt50356_id);
static struct i2c_driver nt50356_i2c_driver = {
	.driver = {
		.name = "nt50356",
		.owner = THIS_MODULE,
		.of_match_table = nt50356_of_id_table,
	},
	.probe = nt50356_probe,
	.remove = nt50356_remove,
	.id_table = nt50356_id,
};

static int __init nt50356_module_init(void)
{
	int ret;

	nt50356_info("in!\n");

	nt50356_class = class_create(THIS_MODULE, "nt50356");
	if (IS_ERR(nt50356_class)) {
		ret = PTR_ERR(nt50356_class);
		nt50356_err("Unable to create nt50356 class; errno = %d\n",
			ret);
		nt50356_class = NULL;
		return ret;
	}

	ret = i2c_add_driver(&nt50356_i2c_driver);
	if (ret) {
		nt50356_err("Unable to register nt50356 driver\n");
		return ret;
	}

	nt50356_info("ok!\n");

	return ret;
}
static void __exit nt50356_module_exit(void)
{
	i2c_del_driver(&nt50356_i2c_driver);
}

late_initcall(nt50356_module_init);
module_exit(nt50356_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Backlight driver for nt50356");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
