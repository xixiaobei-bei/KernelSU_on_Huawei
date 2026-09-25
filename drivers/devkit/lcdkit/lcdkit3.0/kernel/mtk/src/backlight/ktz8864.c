/*
 * ktz8864.c
 *
 * bias+backlight driver of KTZ8864
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

#include "ktz8864.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#include "lcm_drv.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "backlight_linear_to_exp.h"

unsigned int ktz8864_msg_level = 7;

struct class *ktz8864_class = NULL;
struct ktz8864_chip_data *ktz8864_g_chip = NULL;
static bool ktz8864_init_status;
static int g_bl_level_enhance_mode;
static int g_hidden_reg_support;
static int g_only_bias;
struct ktz8864_backlight_information bl_info;
static int g_force_resume_bl_flag = RESUME_IDLE;
static int g_resume_bl_duration; /* default not support auto resume */
static int ktz8864_fault_check_support;
static struct backlight_work_mode_reg_info g_bl_work_mode_reg_indo;
extern struct LCM_DRIVER lcdkit_mtk_common_panel;

static enum hrtimer_restart ktz8864_bl_resume_hrtimer_fnc(struct hrtimer *timer);
static void ktz8864_bl_resume_workqueue_handler(struct work_struct *work);

module_param_named(debug_ktz8864_msg_level, ktz8864_msg_level, int, 0644);
MODULE_PARM_DESC(debug_ktz8864_msg_level, "backlight ktz8864 msg level");

int ktz8864_reg[KTZ8864_RW_REG_MAX] = { 0 };
static unsigned int g_reg_val[KTZ8864_RW_REG_MAX] = { 0 };

static struct ktz8864_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};

static char *ktz8864_dts_string[KTZ8864_RW_REG_MAX] = {
	"ktz8864_bl_config_1",
	"ktz8864_bl_config_2",
	"ktz8864_auto_freq_low",
	"ktz8864_auto_freq_high",
	"ktz8864_display_bias_config_2",
	"ktz8864_display_bias_config_3",
	"ktz8864_lcm_boost_bias",
	"ktz8864_vpos_bias",
	"ktz8864_vneg_bias",
	"ktz8864_bl_option_1",
	"ktz8864_bl_option_2",
	"ktz8864_display_bias_config_1",
	"ktz8864_bl_en",
};
static unsigned int ktz8864_reg_addr[KTZ8864_RW_REG_MAX] = {
	REG_BL_CONFIG_1,
	REG_BL_CONFIG_2,
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
	REG_BL_ENABLE,
};

static struct ktz8864_voltage voltage_table[] = {
	{ 4500000, KTZ8864_VOL_45 },
	{ 4600000, KTZ8864_VOL_46 },
	{ 4700000, KTZ8864_VOL_47 },
	{ 4800000, KTZ8864_VOL_48 },
	{ 4900000, KTZ8864_VOL_49 },
	{ 5000000, KTZ8864_VOL_50 },
	{ 5100000, KTZ8864_VOL_51 },
	{ 5200000, KTZ8864_VOL_52 },
	{ 5300000, KTZ8864_VOL_53 },
	{ 5400000, KTZ8864_VOL_54 },
	{ 5500000, KTZ8864_VOL_55 },
	{ 5600000, KTZ8864_VOL_56 },
	{ 5700000, KTZ8864_VOL_57 },
	{ 5750000, KTZ8864_VOL_575 },
	{ 5800000, KTZ8864_VOL_58 },
	{ 5900000, KTZ8864_VOL_59 },
	{ 6000000, KTZ8864_VOL_60 },
	{ 6100000, KTZ8864_VOL_61 },
	{ 6200000, KTZ8864_VOL_62 },
};

static int ktz8864_parse_dts(struct device_node *np)
{
	int ret;
	int i;

	if (np == NULL) {
		ktz8864_err("np is NULL pointer\n");
		return -1;
	}

	for (i = 0; i < KTZ8864_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, ktz8864_dts_string[i],
			&ktz8864_reg[i]);
		if (ret < 0) {
			ktz8864_err("get ktz8864 dts config failed\n");
		} else {
			ktz8864_info("get %s from dts value = 0x%x\n",
				ktz8864_dts_string[i], ktz8864_reg[i]);
		}
	}
	if (of_property_read_u32(np, "ktz8864_check_fault_support",
		&ktz8864_fault_check_support) < 0)
		ktz8864_info("No need to detect fault flags!\n");

	return ret;
}

static int ktz8864_config_write(struct ktz8864_chip_data *pchip,
	unsigned int reg[], unsigned int val[],
	unsigned int size)
{
	int ret = 0;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		ktz8864_err("pchip or reg or val is NULL pointer\n");
		return -1;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_write(pchip->regmap, reg[i], val[i]);
		if (ret < 0) {
			ktz8864_err("write ktz8864 backlight config register 0x%x failed\n",
				reg[i]);
			goto exit;
		} else {
			ktz8864_info("register 0x%x value = 0x%x\n", reg[i],
				val[i]);
		}
	}

exit:
	return ret;
}

static int ktz8864_config_read(struct ktz8864_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		ktz8864_err("pchip or reg or val is NULL pointer\n");
		return -1;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			ktz8864_err("read ktz8864 backlight config register 0x%x failed",
				reg[i]);
			goto exit;
		} else {
			ktz8864_info("read 0x%x value = 0x%x\n", reg[i],
				val[i]);
		}
	}

exit:
	return ret;
}

static void ktz8864_bl_mode_reg_init(unsigned int reg[], unsigned int val[],
	unsigned int size)
{
	unsigned int i;
	unsigned int reg_element_num;

	if ((reg == NULL) || (val == NULL)) {
		ktz8864_err("reg or val is NULL pointer\n");
		return;
	}

	ktz8864_info("ktz8864_bl_mode_reg_init in\n");
	memset(&g_bl_work_mode_reg_indo, 0,
		sizeof(struct backlight_work_mode_reg_info));

	for (i = 0; i < size; i++) {
		switch (reg[i]) {
		case REG_BL_CONFIG_1:
		case REG_BL_CONFIG_2:
		case REG_BL_OPTION_2:
			reg_element_num = g_bl_work_mode_reg_indo.
				bl_mode_config_reg.reg_element_num;
			if (reg_element_num >= BL_MAX_CONFIG_REG_NUM)
				break;
			g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_addr[reg_element_num] = reg[i];
			g_bl_work_mode_reg_indo.bl_mode_config_reg.normal_reg_var[reg_element_num] = val[i];
			if (reg[i] == REG_BL_CONFIG_1)
				g_bl_work_mode_reg_indo.bl_mode_config_reg.enhance_reg_var[reg_element_num] =
					BL_OVP_29V;
			else if (reg[i] == REG_BL_CONFIG_2)
				g_bl_work_mode_reg_indo.bl_mode_config_reg.enhance_reg_var[reg_element_num] =
					CURRENT_RAMP_5MS;
			else
				g_bl_work_mode_reg_indo.bl_mode_config_reg.enhance_reg_var[reg_element_num] =
					BL_OCP_2;

			ktz8864_info("reg_addr:0x%x, normal_val=0x%x, enhance_val=0x%x\n",
				g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_addr[reg_element_num],
				g_bl_work_mode_reg_indo.bl_mode_config_reg.normal_reg_var[reg_element_num],
				g_bl_work_mode_reg_indo.bl_mode_config_reg.enhance_reg_var[reg_element_num]);

			g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_element_num++;
			break;
		case REG_BL_ENABLE:
			reg_element_num = g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_element_num;
			if (reg_element_num >= BL_MAX_CONFIG_REG_NUM)
				break;
			g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_addr[reg_element_num] = reg[i];
			g_bl_work_mode_reg_indo.bl_enable_config_reg.normal_reg_var[reg_element_num] = val[i];
			g_bl_work_mode_reg_indo.bl_enable_config_reg.enhance_reg_var[reg_element_num] =
				EN_4_SINK;

			ktz8864_info("reg_addr:0x%x, normal_val=0x%x, enhance_val=0x%x\n",
				g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_addr[reg_element_num],
				g_bl_work_mode_reg_indo.bl_enable_config_reg.normal_reg_var[reg_element_num],
				g_bl_work_mode_reg_indo.bl_enable_config_reg.enhance_reg_var[reg_element_num]);

			g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_element_num++;
			break;
		default:
			break;
		}
	}

	reg_element_num =
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num;
	g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr[reg_element_num] = REG_BL_BRIGHTNESS_LSB;
	g_bl_work_mode_reg_indo.bl_current_config_reg.normal_reg_var[reg_element_num] = LSB_10MA;
	g_bl_work_mode_reg_indo.bl_current_config_reg.enhance_reg_var[reg_element_num] = g_bl_level_enhance_mode & LSB;

	ktz8864_info("reg_addr:0x%x, normal_val=0x%x, enhance_val=0x%x\n",
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr[reg_element_num],
		g_bl_work_mode_reg_indo.bl_current_config_reg.normal_reg_var[reg_element_num],
		g_bl_work_mode_reg_indo.bl_current_config_reg.enhance_reg_var[reg_element_num]);

	g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num++;
	reg_element_num =
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num;
	g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr[reg_element_num] = REG_BL_BRIGHTNESS_MSB;
	g_bl_work_mode_reg_indo.bl_current_config_reg.normal_reg_var[reg_element_num] = MSB_10MA;
	g_bl_work_mode_reg_indo.bl_current_config_reg.enhance_reg_var[reg_element_num] =
		g_bl_level_enhance_mode >> MSB;

	ktz8864_info("reg_addr:0x%x, normal_val=0x%x, enhance_val=0x%x\n",
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr[reg_element_num],
		g_bl_work_mode_reg_indo.bl_current_config_reg.normal_reg_var[reg_element_num],
		g_bl_work_mode_reg_indo.bl_current_config_reg.enhance_reg_var[reg_element_num]);

	g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num++;

	ktz8864_info("ktz8864_bl_mode_reg_init success\n");
}
static int ktz8864_set_hidden_reg(struct ktz8864_chip_data *pchip)
{
	int ret;
	unsigned int val = 0;

	if (pchip == NULL) {
		ktz8864_err("pchip is NULL pointer\n");
		return -1;
	}

	ret = regmap_write(pchip->regmap, REG_SET_SECURITYBIT_ADDR,
		REG_SET_SECURITYBIT_VAL);
	if (ret < 0) {
		ktz8864_err("write ktz8864_set_hidden_reg register 0x%x failed\n",
			REG_SET_SECURITYBIT_ADDR);
		goto out;
	} else {
		ktz8864_info("register 0x%x value = 0x%x\n",
			REG_SET_SECURITYBIT_ADDR, REG_SET_SECURITYBIT_VAL);
	}

	ret = regmap_read(pchip->regmap, REG_HIDDEN_ADDR, &val);
	if (ret < 0) {
		ktz8864_err("read ktz8864_set_hidden_reg register 0x%x failed",
			REG_HIDDEN_ADDR);
		goto out;
	} else {
		ktz8864_info("read 0x%x value = 0x%x\n", REG_HIDDEN_ADDR, val);
	}

	if (BIAS_BOOST_TIME_3 != (val & BIAS_BOOST_TIME_3)) {
		val |= BIAS_BOOST_TIME_3;

		ret = regmap_write(pchip->regmap, REG_HIDDEN_ADDR, val);
		if (ret < 0) {
			ktz8864_err("write ktz8864_set_hidden_reg register 0x%x failed\n",
				REG_HIDDEN_ADDR);
			goto out;
		} else {
			ktz8864_info("register 0x%x value = 0x%x\n",
				REG_HIDDEN_ADDR, val);
		}

		ret = regmap_read(pchip->regmap, REG_HIDDEN_ADDR, &val);
		if (ret < 0) {
			ktz8864_err("read ktz8864_set_hidden_reg register 0x%x failed",
				REG_HIDDEN_ADDR);
			goto out;
		} else {
			ktz8864_info("read 0x%x value = 0x%x\n",
				REG_HIDDEN_ADDR, val);
		}
	}

out:
	ret = regmap_write(pchip->regmap, REG_SET_SECURITYBIT_ADDR,
		REG_CLEAR_SECURITYBIT_VAL);
	if (ret < 0) {
		ktz8864_err("write ktz8864_set_hidden_reg register 0x%x failed\n", REG_SET_SECURITYBIT_ADDR);
	} else {
		ktz8864_info("register 0x%x value = 0x%x\n",
			REG_SET_SECURITYBIT_ADDR, REG_CLEAR_SECURITYBIT_VAL);
	}

	ktz8864_info("ktz8864_set_hidden_reg exit\n");
	return ret;
}

static int ktz8864_parsr_default_bias(struct device_node *np)
{
	int ret;

	if (np == NULL) {
		ktz8864_err("np is NULL pointer\n");
		return -1;
	}

	ret = of_property_read_u32(np, KTZ8864_PULL_DOWN_BOOST,
		&bl_info.ktz8864_pull_down_boost);
	if (ret) {
		ktz8864_err("ktz8864 parse default pull down boost failed!\n");
		return ret;
	}

	ret = of_property_read_u32(np, KTZ8864_PULL_UP_BOOST,
		&bl_info.ktz8864_pull_up_boost);
	if (ret) {
		ktz8864_err("ktz8864 parse default pull up boost failed!\n");
		return ret;
	}

	ret = of_property_read_u32(np, KTZ8864_ENABLE_VSP_VSP,
		&bl_info.ktz8864_enable_vsp_vsn);
	if (ret) {
		ktz8864_err("ktz8864 parse default enable vsp vsn failed!\n");
		return ret;
	}

	return ret;
}

/* initialize chip */
static int ktz8864_chip_init(struct ktz8864_chip_data *pchip)
{
	int ret = -1;
	struct device_node *np = NULL;
	struct mtk_panel_info *panel_info = NULL;

	ktz8864_info("in!\n");

	if (pchip == NULL) {
		ktz8864_err("pchip is NULL pointer\n");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_KTZ8864);
	if (!np) {
		ktz8864_err("NOT FOUND device node %s!\n", DTS_COMP_KTZ8864);
		goto out;
	}

	ret = ktz8864_parse_dts(np);
	if (ret < 0) {
		ktz8864_err("parse ktz8864 dts failed");
		goto out;
	}

	ret = of_property_read_u32(np, KTZ8864_HW_ENABLE, &bl_info.ktz8864_hw_en);
	if (ret < 0) {
		ktz8864_debug("get ktz8864_hw_en dts config failed\n");
	}
	else {
		ret = of_property_read_u32(np, KTZ8864_HW_EN_GPIO, &bl_info.ktz8864_hw_en_gpio);
		if (ret < 0) {
			ktz8864_debug("get ktz8864_hw_en_gpio dts config fail\n");
		}
		else {
			/* gpio number offset */
			panel_info = lcdkit_mtk_common_panel.panel_info;
			if (!panel_info) {
				ktz8864_err("panel_info is NULL\n");
				return -1;
			}
			bl_info.ktz8864_hw_en_gpio += panel_info->gpio_offset;
		}
	}

	ret = of_property_read_u32(np, KTZ8864_HW_EN_DELAY, &bl_info.bl_on_kernel_mdelay);
	if (ret < 0)
		ktz8864_info("get bl_on_kernel_mdelay dts config failed\n");

	ret = of_property_read_u32(np, "ktz8864_bl_max_level",
		&g_bl_level_enhance_mode);
	if (ret)
		g_bl_level_enhance_mode = BL_MAX;
	ktz8864_info("g_bl_level_enhance_mode = %d\n",
		g_bl_level_enhance_mode);

	ret = of_property_read_u32(np, KTZ8864_HIDDEN_REG_SUPPORT,
		&g_hidden_reg_support);
	if (ret)
		g_hidden_reg_support = 0;
	ktz8864_info("g_hidden_reg_support = %d\n", g_hidden_reg_support);

	ret = of_property_read_u32(np, KTZ8864_ONLY_BIAS, &g_only_bias);
	if (ret)
		g_only_bias = 0;
	ktz8864_info("g_only_bias = %d\n", g_only_bias);

	ret = of_property_read_u32(np, KTZ8864_PULL_BOOST_SUPPORT,
		&bl_info.ktz8864_pull_boost_support);
	if (ret)
		bl_info.ktz8864_pull_boost_support = 0;
	if (bl_info.ktz8864_pull_boost_support) {
		ret = ktz8864_parsr_default_bias(np);
		if (ret) {
			ktz8864_err("parse default bias failed!\n");
			bl_info.ktz8864_pull_boost_support = 0;
		}
	}
	ktz8864_info("ktz8864_pull_boost_support = %d\n",
		bl_info.ktz8864_pull_boost_support);

	if (g_resume_bl_duration > MAX_BL_RESUME_TIMMER)
		g_resume_bl_duration = MAX_BL_RESUME_TIMMER;

	ktz8864_info("g_resume_bl_duration = %d\n", g_resume_bl_duration);

	if (g_hidden_reg_support) {
		ret = ktz8864_set_hidden_reg(pchip);
		if (ret < 0)
			ktz8864_info("ktz8864_set_hidden_reg failed");
	}

	ret = ktz8864_config_write(pchip, ktz8864_reg_addr,
		ktz8864_reg, KTZ8864_RW_REG_MAX);
	if (ret < 0) {
		ktz8864_err("ktz8864 config register failed");
		goto out;
	}

	ret = ktz8864_config_read(pchip, ktz8864_reg_addr, ktz8864_reg,
		KTZ8864_RW_REG_MAX);
	if (ret < 0) {
		ktz8864_err("ktz8864 config read failed");
		goto out;
	}

	ktz8864_bl_mode_reg_init(ktz8864_reg_addr, ktz8864_reg,
		KTZ8864_RW_REG_MAX);
	pchip->bl_resume_wq = create_singlethread_workqueue("bl_resume");
	if (!pchip->bl_resume_wq)
		ktz8864_err("create bl_resume_wq failed\n");

	INIT_WORK(&pchip->bl_resume_worker,
		ktz8864_bl_resume_workqueue_handler);
	hrtimer_init(&pchip->bl_resume_hrtimer,
		CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pchip->bl_resume_hrtimer.function = ktz8864_bl_resume_hrtimer_fnc;

	ktz8864_info("ok!\n");

	return ret;

out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

static void ktz8864_get_bias_config(int vsp, int vsn, int *outvsp, int *outvsn)
{
	int i;
	int vol_size = ARRAY_SIZE(voltage_table);

	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vsp) {
			*outvsp = voltage_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		ktz8864_err("not found vsn voltage, use default voltage\n");
		*outvsp = KTZ8864_VOL_55;
	}

	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vsn) {
			*outvsn = voltage_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		ktz8864_err("not found vsn voltage, use default voltage\n");
		*outvsn = KTZ8864_VOL_55;
	}
	ktz8864_info("vpos = 0x%x, vneg = 0x%x\n", *outvsp, *outvsn);
}

static int ktz8864_set_ic_disable(void)
{
	int ret;

	if (!ktz8864_g_chip)
		return -1;

	/* reset backlight ic */
	ret = regmap_write(ktz8864_g_chip->regmap, REG_BL_ENABLE, BL_RESET);
	if (ret < 0)
		ktz8864_err("regmap_write REG_BL_ENABLE fail\n");

	/* clean up bl val register */
	ret = regmap_update_bits(ktz8864_g_chip->regmap,
		REG_BL_BRIGHTNESS_LSB, MASK_BL_LSB, BL_DISABLE);
	if (ret < 0)
		ktz8864_err("regmap_update_bits REG_BL_BRIGHTNESS_LSB fail\n");

	ret = regmap_write(ktz8864_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
		BL_DISABLE);
	if (ret < 0)
		ktz8864_err("regmap_write REG_BL_BRIGHTNESS_MSB fail!\n");
	return ret;
}

static int ktz8864_set_bias_power_down(int vpos, int vneg)
{
	int ret;
	int vsp;
	int vsn;

	if (!ktz8864_g_chip)
		return -1;

	if (!bl_info.ktz8864_pull_boost_support) {
		ktz8864_info("No need to pull down BOOST!\n");
		return 0;
	}

	ktz8864_get_bias_config(vpos, vneg, &vsp, &vsn);
	ret = regmap_write(ktz8864_g_chip->regmap, REG_VPOS_BIAS, vsp);
	if (ret < 0) {
		ktz8864_err("write pull_down_vsp failed!\n");
		return ret;
	}

	ret = regmap_write(ktz8864_g_chip->regmap, REG_VNEG_BIAS, vsn);
	if (ret < 0) {
		ktz8864_err("write pull_down_vsn failed!\n");
		return ret;
	}

	ret = regmap_write(ktz8864_g_chip->regmap,
		REG_DISPLAY_BIAS_CONFIG_1,
		bl_info.ktz8864_enable_vsp_vsn);
	if (ret < 0) {
		ktz8864_err("write enable_vsp_vsn failed!\n");
		return ret;
	}

	ret = regmap_write(ktz8864_g_chip->regmap,
		REG_LCM_BOOST_BIAS, bl_info.ktz8864_pull_down_boost);
	if (ret < 0) {
		ktz8864_err("write enable_vsp_vsn failed!\n");
		return ret;
	}
	ktz8864_info("lcd_kit_pull_boost_for_ktz8864 is successful\n");
	return ret;
}

static int ktz8864_set_bias(int vpos, int vneg)
{
	int ret;

	if (vpos < 0 || vneg < 0) {
		ktz8864_err("vpos or vneg is error\n");
		return -1;
	}

	if (!ktz8864_g_chip)
		return -1;

	ret = ktz8864_config_write(ktz8864_g_chip, ktz8864_reg_addr,
		ktz8864_reg, KTZ8864_RW_REG_MAX);
	if (ret < 0) {
		ktz8864_err("ktz8864 config register failed");
		return ret;
	}
	ktz8864_info("ktz8864_set_bias is successful\n");
	return ret;
}

static void ktz8864_check_fault(struct ktz8864_chip_data *pchip,
	int last_level, int level)
{
	unsigned int val = 0;
	int ret;
	int i;

	ktz8864_info("backlight check FAULT_FLAG!\n");

	ret = regmap_read(pchip->regmap, REG_FLAGS, &val);
	if (ret < 0) {
		ktz8864_info("read ktz8864 FAULT_FLAG failed!\n");
		return;
	}

	for (i = 0; i < FLAG_CHECK_NUM; i++) {
		if (!(err_table[i].flag & val))
			continue;
		ktz8864_err("last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
	}
}

static int ktz8864_chip_enable(struct ktz8864_chip_data *pchip)
{
	int ret = -1;

	ktz8864_info("chip enable in!\n");

	if (pchip == NULL) {
		ktz8864_err("pchip is NULL pointer\n");
		return ret;
	}

	ret =  ktz8864_config_write(pchip, ktz8864_reg_addr,
		ktz8864_reg, KTZ8864_RW_REG_MAX);
	if (ret < 0) {
		ktz8864_err("ktz8864 i2c config register failed");
		return ret;
	}

	ktz8864_info("chip enable ok!\n");
	return ret;
}
static void ktz8864_enable(void)
{
	int ret;

	if (bl_info.ktz8864_hw_en) {
		ret = gpio_request(bl_info.ktz8864_hw_en_gpio, NULL);
		if (ret)
			ktz8864_err("ktz8864 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(bl_info.ktz8864_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			ktz8864_err("ktz8864 set gpio output not success\n");
		gpio_set_value(bl_info.ktz8864_hw_en_gpio, GPIO_OUT_ONE);
		if (bl_info.bl_on_kernel_mdelay)
			mdelay(bl_info.bl_on_kernel_mdelay);
	}

	/* chip initialize */
	ret = ktz8864_chip_enable(ktz8864_g_chip);
	if (ret < 0) {
		ktz8864_err("ktz8864_chip_init fail!\n");
		return;
	}
	ktz8864_init_status = true;
}

static void ktz8864_disable(void)
{
	if (bl_info.ktz8864_hw_en) {
		gpio_set_value(bl_info.ktz8864_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(bl_info.ktz8864_hw_en_gpio);
	}
	ktz8864_init_status = false;
}
/*
 * ktz8864_set_backlight_reg(): Set Backlight working mode
 *
 * @bl_level: value for backlight ,range from 0 to 2047
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
int ktz8864_set_backlight_reg(uint32_t bl_level)
{
	ssize_t ret = -1;
	uint32_t level;
	int bl_msb;
	int bl_lsb;
	static int last_level = -1;
	static int enable_flag;
	static int disable_flag;

	if (!ktz8864_g_chip) {
		ktz8864_err("init fail, return.\n");
		return ret;
	}

	if (down_trylock(&(ktz8864_g_chip->test_sem))) {
		ktz8864_info("Now in test mode\n");
		return 0;
	}

	/* first set backlight, enable ktz8864 */
	if ((ktz8864_init_status == false) && (bl_level > 0))
		ktz8864_enable();

	level = bl_level;
	if (level > BL_MAX)
		level = BL_MAX;

	/* 11-bit brightness code */
	bl_msb = level >> MSB;
	bl_lsb = level & LSB;

	ktz8864_info("level = %d, bl_msb = %d, bl_lsb = %d\n", level, bl_msb, bl_lsb);

	ret = regmap_update_bits(ktz8864_g_chip->regmap, REG_BL_BRIGHTNESS_LSB,
		MASK_BL_LSB, bl_lsb);
	if (ret < 0)
		goto i2c_error;

	ret = regmap_write(ktz8864_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
		bl_msb);
	if (ret < 0)
		goto i2c_error;

	/*if set backlight level 0, disable ktz8864*/
	if (true == ktz8864_init_status && 0 == bl_level)
		ktz8864_disable();

	/* Judge power on or power off */
	if (ktz8864_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		ktz8864_check_fault(ktz8864_g_chip, last_level, level);

	last_level = level;
	up(&(ktz8864_g_chip->test_sem));
	ktz8864_info("ktz8864_set_backlight_reg exit succ\n");
	return ret;

i2c_error:
	up(&(ktz8864_g_chip->test_sem));
	dev_err(ktz8864_g_chip->dev, "%s:i2c access fail to register\n",
		__func__);
	ktz8864_info("ktz8864_set_backlight_reg exit fail\n");
	return ret;
}

/*
 * ktz8864_set_reg(): Set ktz8864 reg
 *
 * @bl_reg: which reg want to write
 * @bl_mask: which bits of reg want to change
 * @bl_val: what value want to write to the reg
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t ktz8864_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val)
{
	ssize_t ret = -1;
	u8 reg = bl_reg;
	u8 mask = bl_mask;
	u8 val = bl_val;

	if (!ktz8864_init_status) {
		ktz8864_err("init fail, return.\n");
		return ret;
	}

	if (reg < REG_MAX) {
		ktz8864_err("Invalid argument!!!\n");
		return ret;
	}

	ktz8864_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask,
		val);

	ret = regmap_update_bits(ktz8864_g_chip->regmap, reg, mask, val);
	if (ret < 0) {
		ktz8864_err("i2c access fail to register\n");
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(ktz8864_set_reg);

static ssize_t ktz8864_reg_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ktz8864_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret;
	int bl_lsb = 0;
	int bl_msb = 0;
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
		return snprintf(buf, PAGE_SIZE, "KTZ8864 I2C read error\n");

	ret = regmap_read(pchip->regmap, REG_BL_BRIGHTNESS_LSB, &bl_lsb);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "KTZ8864 I2C read error\n");

	bl_level = (bl_msb << MSB) | bl_lsb;

	return snprintf(buf, PAGE_SIZE, "KTZ8864 bl_level:%d\n", bl_level);
}

static ssize_t ktz8864_reg_bl_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t size)
{
	ssize_t ret;
	struct ktz8864_chip_data *pchip = NULL;
	unsigned int bl_level = 0;
	unsigned int bl_msb;
	unsigned int bl_lsb;

	if (!dev) {
		ktz8864_err("dev is null\n");
		return -1;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		ktz8864_err("data is null\n");
		return -1;
	}

	ret = kstrtouint(buf, 10, &bl_level); // 10 means decimal
	if (ret)
		goto out_input;

	ktz8864_info("%s:buf=%s,state=%d\n", __func__, buf, bl_level);

	if (bl_level > BL_MAX)
		bl_level = BL_MAX;

	/* 11-bit brightness code */
	bl_msb = bl_level >> MSB;
	bl_lsb = bl_level & LSB;

	ktz8864_info("bl_level = %d, bl_msb = %d, bl_lsb = %d\n", bl_level,
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
	return -1;

out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	return -1;
}

static DEVICE_ATTR(reg_bl, 0644, ktz8864_reg_bl_show,
	ktz8864_reg_bl_store);

static ssize_t ktz8864_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ktz8864_chip_data *pchip = NULL;
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

static ssize_t ktz8864_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	struct ktz8864_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!dev) {
		ktz8864_err("dev is null\n");
		return -1;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		ktz8864_err("data is null\n");
		return -1;
	}

	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		printk("check your input!!!\n");
		goto out_input;
	}

	if (reg > REG_MAX) {
		printk("Invalid argument!!!\n");
		goto out_input;
	}

	ktz8864_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg,
		mask, val);

	ret = regmap_update_bits(pchip->regmap, reg, mask, val);
	if (ret < 0)
		goto i2c_error;

	return size;

i2c_error:
	dev_err(pchip->dev, "%s:i2c access fail to register\n", __func__);
	return -1;

out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	return -1;
}

static DEVICE_ATTR(reg, 0644, ktz8864_reg_show,
	ktz8864_reg_store);

static enum hrtimer_restart ktz8864_bl_resume_hrtimer_fnc(struct hrtimer *timer)
{
	struct ktz8864_chip_data *bl_chip_ctrl = NULL;

	bl_chip_ctrl = container_of(timer, struct ktz8864_chip_data,
		bl_resume_hrtimer);
	if (bl_chip_ctrl == NULL) {
		ktz8864_info("bl_chip_ctrl is NULL, return\n");
		return HRTIMER_NORESTART;
	}

	if (bl_chip_ctrl->bl_resume_wq)
		queue_work(bl_chip_ctrl->bl_resume_wq,
			&(bl_chip_ctrl->bl_resume_worker));

	return HRTIMER_NORESTART;
}
static void ktz8864_bl_resume_workqueue_handler(struct work_struct *work)
{
	ssize_t error;
	struct ktz8864_chip_data *bl_chip_ctrl = NULL;

	ktz8864_info("ktz8864_bl_resume_workqueue_handler in\n");
	if (work == NULL) {
		ktz8864_err("work is NULL, return\n");
		return;
	}
	bl_chip_ctrl = container_of(work, struct ktz8864_chip_data,
		bl_resume_worker);
	if (bl_chip_ctrl == NULL) {
		ktz8864_err("bl_chip_ctrl is NULL, return\n");
		return;
	}

	if (down_interruptible(&(ktz8864_g_chip->test_sem))) {
		if (bl_chip_ctrl->bl_resume_wq)
			queue_work(bl_chip_ctrl->bl_resume_wq,
			&(bl_chip_ctrl->bl_resume_worker));
		ktz8864_info("down_trylock get sem fail return\n");
		return;
	}

	if (g_force_resume_bl_flag != RESUME_2_SINK &&
		g_force_resume_bl_flag != RESUME_REMP_OVP_OCP) {
		ktz8864_err("g_force_resume_bl_flag = 0x%x, return\n",
			g_force_resume_bl_flag);
		up(&(ktz8864_g_chip->test_sem));
		return;
	}

	if (g_force_resume_bl_flag == RESUME_2_SINK) {
		ktz8864_info("resume RESUME_LINK\n");
		/* set 4 link to 2 link */
		error = ktz8864_config_write(ktz8864_g_chip,
			g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_addr,
			g_bl_work_mode_reg_indo.bl_enable_config_reg.normal_reg_var,
			g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_element_num);
		if (error) {
			ktz8864_info("resume 2 sink fail return\n");
			if (bl_chip_ctrl->bl_resume_wq)
				queue_work(bl_chip_ctrl->bl_resume_wq,
				&(bl_chip_ctrl->bl_resume_worker));

			up(&(ktz8864_g_chip->test_sem));
			return;
		}

		error = ktz8864_config_write(ktz8864_g_chip,
			g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr,
			g_bl_work_mode_reg_indo.bl_current_config_reg.normal_reg_var,
			g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num);
		if (error)
			ktz8864_err("set bl_current_config_reg fail\n");

		g_force_resume_bl_flag = RESUME_REMP_OVP_OCP;

		/* 6ms */
		mdelay(BL_LOWER_POW_DELAY);
	}

	if (g_force_resume_bl_flag == RESUME_REMP_OVP_OCP) {
		ktz8864_info("resume RESUME_REMP_OVP_OCP\n");
		error = ktz8864_config_write(ktz8864_g_chip,
			g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_addr,
			g_bl_work_mode_reg_indo.bl_mode_config_reg.normal_reg_var,
			g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_element_num);
		if (error) {
			ktz8864_info("resume OVP fail return\n");
			if (bl_chip_ctrl->bl_resume_wq)
				queue_work(bl_chip_ctrl->bl_resume_wq,
				&(bl_chip_ctrl->bl_resume_worker));

			up(&(ktz8864_g_chip->test_sem));
			return;
		}

		g_force_resume_bl_flag = RESUME_IDLE;
	}

	up(&(ktz8864_g_chip->test_sem));
	ktz8864_info("ktz8864_bl_resume_workqueue_handler exit\n");
}
int ktz8864_get_bl_resume_timmer(int *timmer)
{
	int ret = -1;

	if (timmer == NULL)
		return ret;

	*timmer = g_resume_bl_duration;
	ret = 0;
	ktz8864_info("timmer %d!\n", *timmer);
	return ret;
}

ssize_t ktz8864_blkit_set_normal_work_mode(void)
{
	ssize_t error = -1;

	if (!ktz8864_init_status) {
		ktz8864_err("ktz8864_init fail, return.\n");
		return error;
	}

	ktz8864_info("ktz8864_blkit_set_normal_work_mode in\n");
	if (down_interruptible(&ktz8864_g_chip->test_sem)) {
		ktz8864_err("down_trylock fail return\n");
		return error;
	}

	error = ktz8864_config_write(ktz8864_g_chip,
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr,
		g_bl_work_mode_reg_indo.bl_current_config_reg.normal_reg_var,
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num);
	if (error)
		ktz8864_err("set bl_current_config_reg fail\n");
	else
		mdelay(BL_LOWER_POW_DELAY);  // 6ms

	/* set 4  to 2 sink */
	error = ktz8864_config_write(ktz8864_g_chip,
		g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_addr,
		g_bl_work_mode_reg_indo.bl_enable_config_reg.normal_reg_var,
		g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_element_num);
	if (error) {
		ktz8864_err("set bl_enable_config_reg fail\n");
		g_force_resume_bl_flag = RESUME_2_SINK;
		goto out;
	}

	/* 6ms */
	mdelay(BL_LOWER_POW_DELAY);

	g_force_resume_bl_flag = RESUME_REMP_OVP_OCP;
	error = ktz8864_config_write(ktz8864_g_chip,
		g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_addr,
		g_bl_work_mode_reg_indo.bl_mode_config_reg.normal_reg_var,
		g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_element_num);
	if (error) {
		ktz8864_err("set bl_mode_config_reg fail\n");
		goto out;
	}

	if (g_resume_bl_duration != 0)
		hrtimer_cancel(&ktz8864_g_chip->bl_resume_hrtimer);

	g_force_resume_bl_flag = RESUME_IDLE;

	up(&(ktz8864_g_chip->test_sem));
	ktz8864_info("ktz8864_blkit_set_normal_work_mode exit!\n");
	return error;

out:
	up(&(ktz8864_g_chip->test_sem));
	ktz8864_info("ktz8864_blkit_set_normal_work_mode failed\n");
	return error;
}

ssize_t ktzblkit_set_enhance_work_mode(void)
{
	ssize_t error = -1;

	if (!ktz8864_init_status) {
		ktz8864_err("ktz8864_init fail, return.\n");
		return error;
	}

	ktz8864_info("ktzblkit_set_enhance_work_mode in!\n");
	if (down_interruptible(&ktz8864_g_chip->test_sem)) {
		ktz8864_err("down_trylock fail return\n");
		return error;
	}

	g_force_resume_bl_flag = RESUME_IDLE;
	error = ktz8864_config_write(ktz8864_g_chip,
		g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_addr,
		g_bl_work_mode_reg_indo.bl_mode_config_reg.enhance_reg_var,
		g_bl_work_mode_reg_indo.bl_mode_config_reg.reg_element_num);
	if (error) {
		ktz8864_err("set bl_mode_config_reg fail\n");
		goto out;
	}

	g_force_resume_bl_flag = RESUME_REMP_OVP_OCP;
	error = ktz8864_config_write(ktz8864_g_chip,
		g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_addr,
		g_bl_work_mode_reg_indo.bl_enable_config_reg.enhance_reg_var,
		g_bl_work_mode_reg_indo.bl_enable_config_reg.reg_element_num);
	if (error) {
		ktz8864_err("set bl_enable_config_reg fail\n");
		goto out;
	}

	g_force_resume_bl_flag = RESUME_2_SINK;
	error = ktz8864_config_write(ktz8864_g_chip,
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_addr,
		g_bl_work_mode_reg_indo.bl_current_config_reg.enhance_reg_var,
		g_bl_work_mode_reg_indo.bl_current_config_reg.reg_element_num);
	if (error)
		ktz8864_err("set bl_current_config_reg fail\n");

	if (g_resume_bl_duration != 0) {
		hrtimer_cancel(&ktz8864_g_chip->bl_resume_hrtimer);
		hrtimer_start(&ktz8864_g_chip->bl_resume_hrtimer,
			ktime_set((g_resume_bl_duration +
			PROTECT_BL_RESUME_TIMMER) / 1000,
			((g_resume_bl_duration + PROTECT_BL_RESUME_TIMMER) %
			1000) * 1000000), HRTIMER_MODE_REL);
	}  /* the values of 1000 and 1000000 is to change time to second */

	up(&(ktz8864_g_chip->test_sem));
	ktz8864_info("ktzblkit_set_enhance_work_mode exit!\n");
	return error;

out:
	up(&(ktz8864_g_chip->test_sem));
	return error;
}

static const struct regmap_config ktz8864_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

/* pointers to created device attributes */
static struct attribute *ktz8864_attributes[] = {
	&dev_attr_reg_bl.attr,
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group ktz8864_group = {
	.attrs = ktz8864_attributes,
};

#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_bl.h"

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = ktz8864_set_backlight_reg,
	.name = "8864",
};
static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = ktz8864_set_bias,
	.set_bias_power_down = ktz8864_set_bias_power_down,
	.set_ic_disable = ktz8864_set_ic_disable,
};
#endif

static int ktz8864_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct ktz8864_chip_data *pchip = NULL;
	int ret;

	ktz8864_info("in!\n");
	if (client == NULL) {
		ktz8864_err("client is NULL pointer\n");
		return -1;
	}

	adapter = client->adapter;
	if (adapter == NULL) {
		ktz8864_err("adapter is NULL pointer\n");
		return -1;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev, sizeof(struct ktz8864_chip_data),
		GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &ktz8864_regmap);
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
	ret = ktz8864_chip_init(pchip);
	if (ret < 0) {
		dev_err(&client->dev, "fail : chip init\n");
		goto err_out;
	}

	pchip->dev = device_create(ktz8864_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		ktz8864_err("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &ktz8864_group);
		if (ret)
			goto err_sysfs;
	}

	ktz8864_g_chip = pchip;

	ktz8864_info("name: %s, address: (0x%x) ok!\n",
		client->name, client->addr);
#ifdef CONFIG_LCD_KIT_DRIVER
	if(!g_only_bias)
		lcd_kit_bl_register(&bl_ops);
	lcd_kit_bias_register(&bias_ops);
#endif
	ktz8864_init_status = true;

	return ret;

err_sysfs:
	device_destroy(ktz8864_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int ktz8864_suspend(struct device *dev)
{
	int ret;

	/* store reg val before suspend */
	ktz8864_config_read(ktz8864_g_chip, ktz8864_reg_addr, g_reg_val,
		KTZ8864_RW_REG_MAX);
	/* reset backlight ic */
	ret = regmap_write(ktz8864_g_chip->regmap, REG_BL_ENABLE, BL_RESET);
	if (ret < 0)
		dev_err(ktz8864_g_chip->dev,
			"%s:i2c access fail to register\n", __func__);


	/* clean up bl val register */
	ret = regmap_update_bits(ktz8864_g_chip->regmap,
		REG_BL_BRIGHTNESS_LSB, MASK_BL_LSB, BL_DISABLE);
	if (ret < 0)
		dev_err(ktz8864_g_chip->dev,
			"%s:i2c access fail to register\n", __func__);

	ret = regmap_write(ktz8864_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
		BL_DISABLE);
	if (ret < 0)
		dev_err(ktz8864_g_chip->dev,
			"%s:i2c access fail to register\n", __func__);


	return ret;
}

static int ktz8864_resume(struct device *dev)
{
	int ret;

	ktz8864_init_status = true;
	if (g_hidden_reg_support) {
		ret = ktz8864_set_hidden_reg(ktz8864_g_chip);
		if (ret < 0)
			dev_err(ktz8864_g_chip->dev,
				"i2c access fail to ktz8864 hidden register");

	}

	ret = ktz8864_config_write(ktz8864_g_chip, ktz8864_reg_addr, g_reg_val,
		KTZ8864_RW_REG_MAX);
	if (ret < 0)
		dev_err(ktz8864_g_chip->dev,
			"%s:i2c access fail to register\n", __func__);


	return ret;
}

static SIMPLE_DEV_PM_OPS(ktz8864_pm_ops, ktz8864_suspend, ktz8864_resume);
#define KTZ8864_PM_OPS    (&ktz8864_pm_ops)

static int ktz8864_remove(struct i2c_client *client)
{
	struct ktz8864_chip_data *pchip = NULL;

	if (client == NULL) {
		ktz8864_err("client is NULL pointer\n");
		return -1;
	}
	pchip = i2c_get_clientdata(client);

	regmap_write(pchip->regmap, REG_BL_ENABLE, BL_DISABLE);

	sysfs_remove_group(&client->dev.kobj, &ktz8864_group);

	if (g_resume_bl_duration != 0)
		hrtimer_cancel(&ktz8864_g_chip->bl_resume_hrtimer);

	return 0;
}

static const struct i2c_device_id ktz8864_id[] = {
	{ KTZ8864_NAME, 0 },
	{}
};

static const struct of_device_id ktz8864_of_id_table[] = {
	{ .compatible = "ktz,ktz8864"},
	{},
};

MODULE_DEVICE_TABLE(i2c, ktz8864_id);
static struct i2c_driver ktz8864_i2c_driver = {
	.driver = {
		.name = "ktz8864",
		.owner = THIS_MODULE,
		.of_match_table = ktz8864_of_id_table,
#ifndef CONFIG_LCD_KIT_DRIVER
		.pm = KTZ8864_PM_OPS,
#endif
	},
	.probe = ktz8864_probe,
	.remove = ktz8864_remove,
	.id_table = ktz8864_id,
};

static int __init ktz8864_module_init(void)
{
	int ret;

	ktz8864_info("in!\n");

	ktz8864_class = class_create(THIS_MODULE, "ktz8864");
	if (IS_ERR(ktz8864_class)) {
		ktz8864_err("Unable to create ktz8864 class; errno = %ld\n",
			PTR_ERR(ktz8864_class));
		ktz8864_class = NULL;
	}

	ret = i2c_add_driver(&ktz8864_i2c_driver);
	if (ret)
		ktz8864_err("Unable to register ktz8864 driver\n");

	ktz8864_info("ok!\n");

	return ret;
}
static void __exit ktz8864_module_exit(void)
{
	i2c_del_driver(&ktz8864_i2c_driver);
}

module_init(ktz8864_module_init);
module_exit(ktz8864_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Backlight driver for ktz8864");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
