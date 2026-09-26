/*
 * rt4831.c
 *
 * bias+backlight driver of RT4831
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

#include "rt4831.h"
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

static struct rt4831_voltage rt4831_vol_table[] = {
	{ 4000000, RT4831_VOL_400 },
	{ 4050000, RT4831_VOL_405 },
	{ 4100000, RT4831_VOL_410 },
	{ 4150000, RT4831_VOL_415 },
	{ 4200000, RT4831_VOL_420 },
	{ 4250000, RT4831_VOL_425 },
	{ 4300000, RT4831_VOL_430 },
	{ 4350000, RT4831_VOL_435 },
	{ 4400000, RT4831_VOL_440 },
	{ 4450000, RT4831_VOL_445 },
	{ 4500000, RT4831_VOL_450 },
	{ 4550000, RT4831_VOL_455 },
	{ 4600000, RT4831_VOL_460 },
	{ 4650000, RT4831_VOL_465 },
	{ 4700000, RT4831_VOL_470 },
	{ 4750000, RT4831_VOL_475 },
	{ 4800000, RT4831_VOL_480 },
	{ 4850000, RT4831_VOL_485 },
	{ 4900000, RT4831_VOL_490 },
	{ 4950000, RT4831_VOL_495 },
	{ 5000000, RT4831_VOL_500 },
	{ 5050000, RT4831_VOL_505 },
	{ 5100000, RT4831_VOL_510 },
	{ 5150000, RT4831_VOL_515 },
	{ 5200000, RT4831_VOL_520 },
	{ 5250000, RT4831_VOL_525 },
	{ 5300000, RT4831_VOL_530 },
	{ 5350000, RT4831_VOL_535 },
	{ 5400000, RT4831_VOL_540 },
	{ 5450000, RT4831_VOL_545 },
	{ 5500000, RT4831_VOL_550 },
	{ 5550000, RT4831_VOL_555 },
	{ 5600000, RT4831_VOL_560 },
	{ 5650000, RT4831_VOL_565 },
	{ 5700000, RT4831_VOL_570 },
	{ 5750000, RT4831_VOL_575 },
	{ 5800000, RT4831_VOL_580 },
	{ 5850000, RT4831_VOL_585 },
	{ 5900000, RT4831_VOL_590 },
	{ 5950000, RT4831_VOL_595 },
	{ 6000000, RT4831_VOL_600 },
	{ 6050000, RT4831_VOL_605 },
	{ 6400000, RT4831_VOL_640 },
	{ 6450000, RT4831_VOL_645 },
	{ 6500000, RT4831_VOL_650 },
};

static char *rt4831_dts_string[RT4831_RW_REG_MAX] = {
	"rt4831_bl_config_1",
	"rt4831_bl_config_2",
	"rt4831_bl_brightness_lsb",
	"rt4831_bl_brightness_msb",
	"rt4831_auto_freq_low",
	"rt4831_auto_freq_high",
	"rt4831_display_bias_config_2",
	"rt4831_display_bias_config_3",
	"rt4831_lcm_boost_bias",
	"rt4831_vpos_bias",
	"rt4831_vneg_bias",
	"rt4831_bl_option_1",
	"rt4831_bl_option_2",
	"rt4831_display_bias_config_1",
	"rt4831_reg_hidden_f0",
	"rt4831_reg_hidden_b1",
	"rt4831_bl_en",
};
static unsigned int rt4831_reg_addr[RT4831_RW_REG_MAX] = {
	REG_BL_CONFIG_1,
	REG_BL_CONFIG_2,
	REG_BL_BRIGHTNESS_LSB,
	REG_BL_BRIGHTNESS_MSB,
	REG_AUTO_FREQ_LOW,
	REG_AUTO_FREQ_HIGH,
	REG_DISPLAY_BIAS_CONFIG_2,
	REG_DISPLAY_BIAS_CONFIG_3,
	REG_LCM_BOOST_BIAS,
	REG_VPOS_BIAS,
	REG_VNEG_BIAS,
	REG_BL_OPTION_1,
	REG_BL_OPTION_2,
	REG_DISPLAY_BIAS_CONFIG_1,
	REG_RT4831_REG_HIDDEN_F0,
	REG_RT4831_REG_HIDDEN_B1,
	REG_BL_ENABLE,
};
int rt4831_reg[RT4831_RW_REG_MAX] = { 0 };
struct class *rt4831_class = NULL;
struct rt4831_chip_data *rt4831_g_chip = NULL;
static bool rt4831_init_status;
static struct rt4831_backlight_information g_bl_info;
static int rt4831_10000_bl;
static int rt4831_fault_check_support;
#ifndef CONFIG_DRM_MEDIATEK
extern struct LCM_DRIVER lcdkit_mtk_common_panel;
#endif
#if defined(CONFIG_HUAWEI_DSM)
static struct rt4831_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};
#endif

unsigned int rt4831_msg_level = 7;
extern int bl_lvl_map(int level);

module_param_named(debug_rt4831_msg_level, rt4831_msg_level, int, 0644);
MODULE_PARM_DESC(debug_rt4831_msg_level, "backlight rt4831 msg level");

static int rt4831_parse_dts(struct device_node *np)
{
	int ret;
	int i;

	if (np == NULL) {
		RT4831_ERR("np is NULL pointer\n");
		return -1;
	}

	for (i = 0; i < RT4831_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, rt4831_dts_string[i],
			&rt4831_reg[i]);
		if (ret < 0) {
			RT4831_ERR("get rt4831 dts config failed\n");
		} else {
			RT4831_INFO("get %s from dts value = 0x%x\n",
				rt4831_dts_string[i], rt4831_reg[i]);
		}
	}
	if (of_property_read_u32(np, "rt4831_check_fault_support",
		&rt4831_fault_check_support) < 0)
		RT4831_INFO("No need to detect fault flags!\n");

	ret = of_property_read_u32(np, RT4831_ONLY_BACKLIGHT, &g_bl_info.bl_only);
	if (ret < 0) {
		RT4831_INFO("No need to detect fault flags \n");
		g_bl_info.bl_only = 0;
	}

	ret = of_property_read_u32(np, "rt4831_10000_bl", &rt4831_10000_bl);
	if (ret < 0)
		rt4831_10000_bl = 0;

	ret = of_property_read_u32(np, "rt4831_dimming_support",
		&g_bl_info.bl_dimming_support);
	if (ret < 0)
		RT4831_ERR("get bl_dimming_support dts config failed\n");
	ret = of_property_read_u32(np, "rt4831_dimming_config",
		&g_bl_info.bl_dimming_config);
	if (ret < 0)
		RT4831_ERR("get bl_dimming_config dts config failed\n");
	ret = of_property_read_u32(np, "rt4831_dimming_resume",
		&g_bl_info.bl_dimming_resume);
	if (ret < 0)
		RT4831_ERR("get bl_dimming_resume dts config failed\n");

	return LCD_KIT_OK;
}

static int rt4831_config_write(struct rt4831_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = 0;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		RT4831_ERR("pchip or reg or val is NULL pointer\n");
		return -1;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_write(pchip->regmap, reg[i], val[i]);
		if (ret < 0) {
			RT4831_ERR("write rt4831 bl config reg 0x%x failed\n",
				reg[i]);
			return ret;
		} else {
			RT4831_INFO("register 0x%x value = 0x%x\n", reg[i],
				val[i]);
		}
	}

	return LCD_KIT_OK;
}

static int rt4831_parsr_default_bias(struct device_node *np)
{
	int ret;

	if (np == NULL) {
		RT4831_ERR("np is NULL pointer\n");
		return -1;
	}

	ret = of_property_read_u32(np, RT4831_PULL_DOWN_BOOST,
		&g_bl_info.rt4831_pull_down_boost);
	if (ret) {
		RT4831_ERR("rt4831 parse default pull down boost failed!\n");
		return ret;
	}

	ret = of_property_read_u32(np, RT4831_PULL_UP_BOOST,
		&g_bl_info.rt4831_pull_up_boost);
	if (ret) {
		RT4831_ERR("rt4831 parse default pull up boost failed!\n");
		return ret;
	}

	ret = of_property_read_u32(np, RT4831_ENABLE_VSP_VSP,
		&g_bl_info.rt4831_enable_vsp_vsn);
	if (ret) {
		RT4831_ERR("rt4831 parse default enable vsp vsn failed!\n");
		return ret;
	}

	return ret;
}

static void rt4831_get_target_voltage(int *vpos, int *vneg)
{
	int i;
	int tal_size = sizeof(rt4831_vol_table) / sizeof(struct rt4831_voltage);

	if((vpos == NULL) || (vneg == NULL)){
		LCD_KIT_ERR("vpos or vneg is null\n");
		return;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (rt4831_vol_table[i].voltage == power_hdl->lcd_vsp.buf[2]) {
			*vpos = rt4831_vol_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsp voltage, use default voltage\n");
		*vpos = RT4831_VOL_600;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (rt4831_vol_table[i].voltage == power_hdl->lcd_vsn.buf[2]) {
			*vneg = rt4831_vol_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsn voltage, use default voltage\n");
		*vneg = RT4831_VOL_600;
	}

	LCD_KIT_INFO("*vpos_target=0x%x,*vneg_target=0x%x\n", *vpos, *vneg);

	return;
}

/* initialize chip */
static int rt4831_chip_init(struct rt4831_chip_data *pchip)
{
	int ret;
	struct device_node *np = NULL;
	struct mtk_panel_info *panel_info = NULL;
	int vpos_target = 0;
	int vneg_target = 0;

	RT4831_INFO("in!\n");

	if (pchip == NULL) {
		RT4831_ERR("pchip is NULL pointer\n");
		return -1;
	}

	memset(&g_bl_info, 0, sizeof(struct rt4831_backlight_information));

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_RT4831);
	if (!np) {
		RT4831_ERR("NOT FOUND device node %s!\n", DTS_COMP_RT4831);
		return -1;
	}

	ret = rt4831_parse_dts(np);
	if (ret < 0) {
		RT4831_ERR("parse rt4831 dts failed");
		return ret;
	}

	if(!g_bl_info.bl_only) {
		rt4831_get_target_voltage(&vpos_target, &vneg_target);
		/* 9  is the position of vsp in rt4831_reg */
		/* 10 is the position of vsn in rt4831_reg */
		if (rt4831_reg[9] != vpos_target)
			rt4831_reg[9] = vpos_target;
		if (rt4831_reg[10] != vneg_target)
			rt4831_reg[10] = vneg_target;
	}

	ret = of_property_read_u32(np, RT4831_HW_ENABLE, &g_bl_info.rt4831_hw_en);
	if (ret < 0) {
		RT4831_ERR("get rt4831_hw_en dts config failed\n");
		g_bl_info.rt4831_hw_en = 0;
	}
	if (g_bl_info.rt4831_hw_en != 0) {
		ret = of_property_read_u32(np, RT4831_HW_EN_GPIO, &g_bl_info.rt4831_hw_en_gpio);
		if (ret < 0) {
			RT4831_ERR("get rt4831_hw_en_gpio dts config fail\n");
		} else {
			/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
			panel_info = lcm_get_panel_info();
#else
			panel_info = lcdkit_mtk_common_panel.panel_info;
#endif

			if (!panel_info) {
				RT4831_ERR("panel_info is NULL\n");
				return -1;
			}
			g_bl_info.rt4831_hw_en_gpio += panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, RT4831_HW_EN_PULL_LOW,
			&g_bl_info.rt4831_hw_en_pull_low);
		if (ret < 0) {
			RT4831_ERR("get rt4831_hw_en_pull_low dts config fail\n");
			g_bl_info.rt4831_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, RT4831_HW_EN_DELAY, &g_bl_info.bl_on_kernel_mdelay);
	if (ret < 0)
		RT4831_ERR("get bl_on_kernel_mdelay dts config failed\n");

	ret = of_property_read_u32(np, RT4831_PULL_BOOST_SUPPORT,
		&g_bl_info.rt4831_pull_boost_support);
	if (ret)
		g_bl_info.rt4831_pull_boost_support = 0;
	if (g_bl_info.rt4831_pull_boost_support) {
		ret = rt4831_parsr_default_bias(np);
		if (ret) {
			RT4831_ERR("parse default bias failed!\n");
			g_bl_info.rt4831_pull_boost_support = 0;
		}
	}
	RT4831_INFO("rt4831_pull_boost_support = %d\n",
		g_bl_info.rt4831_pull_boost_support);
	ret = of_property_read_u32(np, RT4831_VTC_LINE_BOOST,
		&g_bl_info.rt4831_vtc_line_boost);
	if (ret)
		RT4831_ERR("rt4831 parse vtc line boost failed!\n");
	ret = of_property_read_u32(np, RT4831_VTC_END_BOOST,
		&g_bl_info.rt4831_vtc_end_boost);
	if (ret)
		RT4831_ERR("nt50356 parse vtc end boost failed!\n");

	RT4831_INFO("ok!\n");

	return LCD_KIT_OK;
}

static void rt4831_get_bias_config(int vsp, int vsn, int *outvsp, int *outvsn)
{
	int i;
	int vol_size = ARRAY_SIZE(rt4831_vol_table);

	for (i = 0; i < vol_size; i++) {
		if (rt4831_vol_table[i].voltage == vsp) {
			*outvsp = rt4831_vol_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		RT4831_ERR("not found vsn voltage, use default voltage\n");
		*outvsp = RT4831_VOL_550;
	}

	for (i = 0; i < vol_size; i++) {
		if (rt4831_vol_table[i].voltage == vsn) {
			*outvsn = rt4831_vol_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		RT4831_ERR("not found vsn voltage, use default voltage\n");
		*outvsn = RT4831_VOL_550;
	}
	RT4831_INFO("vpos = 0x%x, vneg = 0x%x\n", *outvsp, *outvsn);
}

static int rt4831_set_ic_disable(void)
{
	int ret;

	if (!rt4831_g_chip)
		return -1;

	/* reset backlight ic */
	ret = regmap_write(rt4831_g_chip->regmap, REG_BL_ENABLE, BL_RESET);
	if (ret < 0)
		RT4831_ERR("regmap_write REG_BL_ENABLE fail\n");

	/* clean up bl val register */
	ret = regmap_update_bits(rt4831_g_chip->regmap,
			REG_BL_BRIGHTNESS_LSB, MASK_BL_LSB, BL_DISABLE);
	if (ret < 0)
		RT4831_ERR("regmap_update_bits REG_BL_BRIGHTNESS_LSB fail\n");

	ret = regmap_write(rt4831_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
			BL_DISABLE);
	if (ret < 0)
		RT4831_ERR("regmap_write REG_BL_BRIGHTNESS_MSB fail!\n");
	if (!ret)
		RT4831_INFO("rt4831_set_ic_disable successful!\n");
	if (g_bl_info.rt4831_hw_en != 0 &&
		g_bl_info.rt4831_hw_en_pull_low != 0) {
		gpio_set_value(g_bl_info.rt4831_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(g_bl_info.rt4831_hw_en_gpio);
	}
	return ret;
}

static int rt4831_set_bias_power_down(int vpos, int vneg)
{
	int ret = 0;
	int vsp;
	int vsn;

	if (!rt4831_g_chip)
		return -1;

	if (!g_bl_info.rt4831_pull_boost_support) {
		RT4831_INFO("No need to pull down BOOST!\n");
		return ret;
	}

	rt4831_get_bias_config(vpos, vneg, &vsp, &vsn);

	if (!rt4831_g_chip)
		return -1;

	ret = regmap_write(rt4831_g_chip->regmap, REG_VPOS_BIAS, vsp);
	if (ret < 0) {
		RT4831_ERR("write pull_down_vsp failed!\n");
		return ret;
	}

	ret = regmap_write(rt4831_g_chip->regmap, REG_VNEG_BIAS, vsn);
	if (ret < 0) {
		RT4831_ERR("write pull_down_vsn failed!\n");
		return ret;
	}

	ret = regmap_write(rt4831_g_chip->regmap,
		REG_DISPLAY_BIAS_CONFIG_1,
		g_bl_info.rt4831_enable_vsp_vsn);
	if (ret < 0) {
		RT4831_ERR("write enable_vsp_vsn failed!\n");
		return ret;
	}

	ret = regmap_write(rt4831_g_chip->regmap,
		REG_LCM_BOOST_BIAS, g_bl_info.rt4831_pull_down_boost);
	if (ret < 0) {
		RT4831_ERR("write enable_vsp_vsn failed!\n");
		return ret;
	}
	RT4831_INFO("lcd_kit_pull_boost_for_rt4831 is successful\n");

	return ret;
}

static int rt4831_set_bias(int vpos, int vneg)
{
	int ret;

	if ((vpos < 0) || (vneg < 0)) {
		RT4831_ERR("vpos or vneg is error\n");
		return -1;
	}

	if (!rt4831_g_chip)
		return -1;
	if (g_bl_info.rt4831_hw_en != 0 &&
		g_bl_info.rt4831_hw_en_pull_low != 0) {
		ret = gpio_request(g_bl_info.rt4831_hw_en_gpio, NULL);
		if (ret)
			RT4831_ERR("rt4831 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(g_bl_info.rt4831_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			RT4831_ERR("rt4831 set gpio output not success\n");
		gpio_set_value(g_bl_info.rt4831_hw_en_gpio, GPIO_OUT_ONE);
		if (g_bl_info.bl_on_kernel_mdelay)
			mdelay(g_bl_info.bl_on_kernel_mdelay);
	}
	ret = rt4831_config_write(rt4831_g_chip, rt4831_reg_addr,
			rt4831_reg, RT4831_RW_REG_MAX);
	if (ret < 0) {
		RT4831_ERR("rt4831 config register failed");
		return ret;
	}
	RT4831_INFO("rt4831_set_bias is successful\n");
	return ret;

}

static void rt4831_check_fault(struct rt4831_chip_data *pchip,
	int last_level, int level)
{
	unsigned int val = 0;
	int ret;
	int i;

	RT4831_INFO("backlight check FAULT_FLAG!\n");

	ret = regmap_read(pchip->regmap, REG_FLAGS, &val);
	if (ret < 0) {
		RT4831_INFO("read rt4831 FAULT_FLAG failed!\n");
		return;
	}
	if (val)
		LCD_KIT_ERR("check FAULT_FLAG error: val=0x%x\n", val);

#if defined CONFIG_HUAWEI_DSM
	for (i = 0; i < FLAG_CHECK_NUM; i++) {
		if (!(err_table[i].flag & val))
			continue;
		RT4831_ERR("last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret) {
			RT4831_ERR("dsm_client_ocuppy fail: ret=%d!\n", ret);
			continue;
		}
		dsm_client_record(lcd_dclient,
			"rt4831 last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		dsm_client_notify(lcd_dclient, err_table[i].err_no);
	}
#endif
}

static void backlight_ic_check_fault(struct rt4831_chip_data *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		RT4831_ERR("pchip is NULL\n");
		return;
	}
	/* Judge power on or power off */
	if (rt4831_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		rt4831_check_fault(pchip, last_level, level);
	last_level = level;
}

static int rt4831_chip_enable(struct rt4831_chip_data *pchip)
{
	int ret = -1;

	RT4831_INFO("chip enable in!\n");

	if (pchip == NULL) {
		RT4831_ERR("pchip is NULL pointer\n");
		return ret;
	}

	ret =  rt4831_config_write(pchip, rt4831_reg_addr,
		rt4831_reg, RT4831_RW_REG_MAX);
	if (ret < 0) {
		RT4831_ERR("rt4831 i2c config register failed");
		return ret;
	}

	RT4831_INFO("chip enable ok!\n");
	return ret;
}
static void rt4831_enable(void)
{
	int ret;

	if (g_bl_info.rt4831_hw_en != 0 &&
		g_bl_info.rt4831_hw_en_pull_low != 0) {
		ret = gpio_request(g_bl_info.rt4831_hw_en_gpio, NULL);
		if (ret)
			RT4831_ERR("rt4831 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(g_bl_info.rt4831_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			RT4831_ERR("rt4831 set gpio output not success\n");
		gpio_set_value(g_bl_info.rt4831_hw_en_gpio, GPIO_OUT_ONE);
		if (g_bl_info.bl_on_kernel_mdelay)
			mdelay(g_bl_info.bl_on_kernel_mdelay);
	}

	/* chip initialize */
	ret = rt4831_chip_enable(rt4831_g_chip);
	if (ret < 0) {
		RT4831_ERR("rt4831_chip_init fail!\n");
		return;
	}
	rt4831_init_status = true;
}

static void rt4831_disable(void)
{
	if (g_bl_info.rt4831_hw_en != 0 &&
		g_bl_info.rt4831_hw_en_pull_low != 0) {
		gpio_set_value(g_bl_info.rt4831_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(g_bl_info.rt4831_hw_en_gpio);
	}
	rt4831_init_status = false;
}

static void rt4831_dync_dimming_config(unsigned int bl_level)
{
	int ret = -1;
	static int bl_work_state = BL_POWER_ON;

	if (g_bl_info.bl_dimming_support != BL_DIMMING_SUPPORT)
		return;
	if (bl_level == 0) {
		if (g_bl_info.bl_dimming_resume != 0) {
			ret = regmap_write(rt4831_g_chip->regmap,
				REG_BL_CONFIG_2,
				g_bl_info.bl_dimming_resume);
			if (ret < 0)
				RT4831_ERR("dync dimming config error\n");
		}
		bl_work_state = BL_POWER_OFF;
		return;
	}
	if (bl_work_state == BL_POWER_OFF) {
		bl_work_state = BL_POWER_ON;
	} else if (bl_work_state == BL_POWER_ON) {
		if (g_bl_info.bl_dimming_config != 0) {
			ret = regmap_write(rt4831_g_chip->regmap,
				REG_BL_CONFIG_2,
				g_bl_info.bl_dimming_config);
			if (ret < 0)
				RT4831_ERR("dync dimming config error\n");
			RT4831_INFO("config bl dimming reg[%d]=0x%x\n",
				REG_BL_CONFIG_2, g_bl_info.bl_dimming_config);
		}
		bl_work_state = BL_CONFIG_LEVEL_POWER_ON;
	}
}

/*
 * rt4831_set_backlight(): Set Backlight working mode
 *
 * @bl_level: value for backlight ,range from 0 to 2047
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
int rt4831_set_backlight(uint32_t bl_level)
{
	ssize_t ret = -1;
	uint32_t level;
	unsigned int bl_msb;
	unsigned int bl_lsb;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *panel_info = NULL;
#endif

	if (!rt4831_init_status) {
		RT4831_ERR("init fail, return.\n");
		return ret;
	}
	if (down_trylock(&(rt4831_g_chip->test_sem))) {
		RT4831_INFO("Now in test mode\n");
		return 0;
	}

	/* first set backlight, enable rt4831 */
	if (g_bl_info.bl_only != 0) {
		if ((rt4831_init_status == false) && (bl_level > 0))
			rt4831_enable();
	}

	RT4831_INFO("rt4831_set_backlight bl_level = %u\n", bl_level);

	if (rt4831_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > BL_MAX)
			level = BL_MAX;
	}
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		RT4831_ERR("panel_info is NULL\n");
		up(&(rt4831_g_chip->test_sem));
		return ret;
	}
	if (panel_info->bl_ic_ctrl_mode == MTK_MIPI_BL_IC_PWM_MODE) {
		RT4831_INFO("rt4831 mipi pwm\n");
		backlight_ic_check_fault(rt4831_g_chip, level);
		up(&(rt4831_g_chip->test_sem));
		return LCD_KIT_OK;
	}
#endif

	rt4831_dync_dimming_config(bl_level);
	/* 11-bit brightness code */
	bl_msb = level >> MSB;
	bl_lsb = level & LSB;

	ret = regmap_update_bits(rt4831_g_chip->regmap, REG_BL_BRIGHTNESS_LSB,
		MASK_BL_LSB, bl_lsb);
	if (ret < 0)
		goto i2c_error;

	ret = regmap_write(rt4831_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
		bl_msb);
	if (ret < 0)
		goto i2c_error;
	backlight_ic_check_fault(rt4831_g_chip, level);
	/* if set backlight level 0, disable rt4831 */
	if (g_bl_info.bl_only != 0) {
		if (rt4831_init_status == true && bl_level == 0)
			rt4831_disable();
	}
	up(&(rt4831_g_chip->test_sem));
	RT4831_INFO("rt4831_set_backlight exit succ\n");
	return ret;

i2c_error:
	up(&(rt4831_g_chip->test_sem));
	lcd_backlight_i2c_dmd();
	dev_err(rt4831_g_chip->dev, "%s:i2c access fail to register\n",
		__func__);
	RT4831_INFO("rt4831_set_backlight exit fail\n");
	return ret;
}

/*
 * rt4831_set_reg(): Set rt4831 reg
 *
 * @bl_reg: which reg want to write
 * @bl_mask: which bits of reg want to change
 * @bl_val: what value want to write to the reg
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t rt4831_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val)
{
	ssize_t ret = -1;
	u8 reg = bl_reg;
	u8 mask = bl_mask;
	u8 val = bl_val;

	if (!rt4831_init_status) {
		RT4831_ERR("init fail, return.\n");
		return ret;
	}

	if (reg < REG_MAX) {
		RT4831_ERR("Invalid argument!!!\n");
		return ret;
	}

	RT4831_INFO("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask,
		val);

	ret = regmap_update_bits(rt4831_g_chip->regmap, reg, mask, val);
	if (ret < 0) {
		RT4831_ERR("i2c access fail to register\n");
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(rt4831_set_reg);

static ssize_t rt4831_reg_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct rt4831_chip_data *pchip = NULL;
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
		return snprintf(buf, PAGE_SIZE, "RT4831 I2C read error\n");

	ret = regmap_read(pchip->regmap, REG_BL_BRIGHTNESS_LSB, &bl_lsb);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "RT4831 I2C read error\n");

	bl_level = (bl_msb << MSB) | bl_lsb;

	return snprintf(buf, PAGE_SIZE, "RT4831 bl_level:%d\n", bl_level);
}

static ssize_t rt4831_reg_bl_store(struct device *dev,
	struct device_attribute *dev_attr,
	const char *buf, size_t size)
{
	ssize_t ret;
	struct rt4831_chip_data *pchip = NULL;
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

	RT4831_INFO("%s:buf=%s,state=%d\n", __func__, buf, bl_level);

	if (bl_level > BL_MAX)
		bl_level = BL_MAX;

	/* 11-bit brightness code */
	bl_msb = bl_level >> MSB;
	bl_lsb = bl_level & LSB;

	RT4831_INFO("bl_level = %d, bl_msb = %d, bl_lsb = %d\n", bl_level,
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

static DEVICE_ATTR(reg_bl, 0644, rt4831_reg_bl_show,
	rt4831_reg_bl_store);

static ssize_t rt4831_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct rt4831_chip_data *pchip = NULL;
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
			"\rBacklight Option 2 Register(0x11) = 0x%x\nPWM-to-Digital Code Readback LSB(0x12) = 0x%x\n"
			"\rPWM-to-Digital Code Readback MSB(0x13) = 0x%x\n",
			val[0], val[1], val[2], val[3], val[4], val[5], val[6],
			val[7], val[8], val[9], val[10], val[11], val[12],
			val[13], val[14], val[15], val[16], val[17], val[18]);
			/* 0~18 means number of registers */

i2c_error:
	return snprintf(buf, PAGE_SIZE, "%s: i2c access fail to register\n",
		__func__);
}

static ssize_t rt4831_reg_store(struct device *dev,
	struct device_attribute *dev_attr, const char *buf, size_t size)
{
	ssize_t ret;
	struct rt4831_chip_data *pchip = NULL;
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

	RT4831_INFO("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg,
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

static DEVICE_ATTR(reg, 0644, rt4831_reg_show,
	rt4831_reg_store);

static const struct regmap_config rt4831_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

/* pointers to created device attributes */
static struct attribute *rt4831_attributes[] = {
	&dev_attr_reg_bl.attr,
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group rt4831_group = {
	.attrs = rt4831_attributes,
};

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = rt4831_set_backlight,
	.name = "rt4831",
};
static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = rt4831_set_bias,
	.set_bias_power_down = rt4831_set_bias_power_down,
	.set_ic_disable = rt4831_set_ic_disable,
};


static int rt4831_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct rt4831_chip_data *pchip = NULL;
	int ret;

	RT4831_INFO("in!\n");
	if (client == NULL) {
		RT4831_ERR("client is NULL pointer\n");
		return -1;
	}

	adapter = client->adapter;
	if (adapter == NULL) {
		RT4831_ERR("adapter is NULL pointer\n");
		return -1;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev, sizeof(struct rt4831_chip_data),
		GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &rt4831_regmap);
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
	ret = rt4831_chip_init(pchip);
	if (ret < 0) {
		dev_err(&client->dev, "fail : chip init\n");
		goto err_out;
	}

	pchip->dev = device_create(rt4831_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		RT4831_ERR("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &rt4831_group);
		if (ret)
			goto err_sysfs;
	}

	rt4831_g_chip = pchip;
	if (g_bl_info.rt4831_hw_en) {
		ret = gpio_request(g_bl_info.rt4831_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("rt4831 Could not request hw_en_gpio\n");
	}
	if(g_bl_info.bl_only) {
		lcd_kit_bl_register(&bl_ops);
	} else {
		lcd_kit_bias_register(&bias_ops);
		lcd_kit_bl_register(&bl_ops);
	}

	rt4831_init_status = true;

	return ret;

err_sysfs:
	device_destroy(rt4831_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int rt4831_remove(struct i2c_client *client)
{
	struct rt4831_chip_data *pchip = NULL;

	if (client == NULL) {
		RT4831_ERR("client is NULL pointer\n");
		return -1;
	}
	pchip = i2c_get_clientdata(client);

	regmap_write(pchip->regmap, REG_BL_ENABLE, BL_DISABLE);

	sysfs_remove_group(&client->dev.kobj, &rt4831_group);

	return 0;
}

static const struct i2c_device_id rt4831_id[] = {
	{ RT4831_NAME, 0 },
	{}
};

static const struct of_device_id rt4831_of_id_table[] = {
	{ .compatible = "rt,rt4831" },
	{},
};

MODULE_DEVICE_TABLE(i2c, rt4831_id);
static struct i2c_driver rt4831_i2c_driver = {
	.driver = {
		.name = "rt4831",
		.owner = THIS_MODULE,
		.of_match_table = rt4831_of_id_table,
	},
	.probe = rt4831_probe,
	.remove = rt4831_remove,
	.id_table = rt4831_id,
};

static int __init rt4831_module_init(void)
{
	int ret;

	RT4831_INFO("in!\n");

	rt4831_class = class_create(THIS_MODULE, "rt4831");
	if (IS_ERR(rt4831_class)) {
		ret = PTR_ERR(rt4831_class);
		RT4831_ERR("Unable to create rt4831 class; errno = %d\n",
			ret);
		rt4831_class = NULL;
		return ret;
	}

	ret = i2c_add_driver(&rt4831_i2c_driver);
	if (ret) {
		RT4831_ERR("Unable to register rt4831 driver\n");
		return ret;
	}

	RT4831_INFO("ok!\n");

	return ret;
}
static void __exit rt4831_module_exit(void)
{
	i2c_del_driver(&rt4831_i2c_driver);
}

late_initcall(rt4831_module_init);
module_exit(rt4831_module_exit);


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Backlight driver for rt4831");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
