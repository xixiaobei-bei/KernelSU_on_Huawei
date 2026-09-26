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
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#else
#include "lcm_drv.h"
#endif
#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "backlight_linear_to_exp.h"
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

unsigned int ktz8864_msg_level = 7;

struct class *ktz8864_class = NULL;
struct ktz8864_chip_data *ktz8864_g_chip = NULL;
static bool ktz8864_init_status;
struct ktz8864_backlight_information bl_info;
static int ktz8864_fault_check_support;
static int ktz8864_10000_bl;
#ifndef CONFIG_DRM_MEDIATEK
extern struct LCM_DRIVER lcdkit_mtk_common_panel;
#endif

module_param_named(debug_ktz8864_msg_level, ktz8864_msg_level, int, 0644);
MODULE_PARM_DESC(debug_ktz8864_msg_level, "backlight ktz8864 msg level");

int ktz8864_reg[KTZ8864_RW_REG_MAX] = { 0 };
#if defined(CONFIG_HUAWEI_DSM)
static struct ktz8864_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};
#endif
static char *ktz8864_dts_string[KTZ8864_RW_REG_MAX] = {
	"ktz8864_bl_config_1",
	"ktz8864_bl_config_2",
	"ktz8864_bl_brightness_lsb",
	"ktz8864_bl_brightness_msb",
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

	ret = of_property_read_u32(np, "ktz8864_10000_bl", &ktz8864_10000_bl);
	if (ret < 0)
		ktz8864_10000_bl = 0;

	ret = of_property_read_u32(np, "ktz8864_dimming_support",
		&bl_info.bl_dimming_support);
	if (ret < 0)
		ktz8864_err("get bl_dimming_support dts config failed\n");
	ret = of_property_read_u32(np, "ktz8864_dimming_config",
		&bl_info.bl_dimming_config);
	if (ret < 0)
		ktz8864_err("get bl_dimming_config dts config failed\n");
	ret = of_property_read_u32(np, "ktz8864_dimming_resume",
		&bl_info.bl_dimming_resume);
	if (ret < 0)
		ktz8864_err("get bl_dimming_resume dts config failed\n");

	return LCD_KIT_OK;
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

static void ktz8864_get_target_voltage(int *vpos, int *vneg)
{
	int i;
	int tal_size = sizeof(voltage_table) / sizeof(struct ktz8864_voltage);

	if ((vpos == NULL) || (vneg == NULL)) {
		LCD_KIT_ERR("vpos or vneg is null\n");
		return;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (voltage_table[i].voltage == power_hdl->lcd_vsp.buf[2]) {
			*vpos = voltage_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsp voltage, use default voltage\n");
		*vpos = KTZ8864_VOL_60;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (voltage_table[i].voltage == power_hdl->lcd_vsn.buf[2]) {
			*vneg = voltage_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsn voltage, use default voltage\n");
		*vneg = KTZ8864_VOL_60;
	}

	LCD_KIT_INFO("*vpos_target=0x%x,*vneg_target=0x%x\n", *vpos, *vneg);

	return;
}


/* initialize chip */
static int ktz8864_chip_init(struct ktz8864_chip_data *pchip)
{
	int ret = -1;
	struct device_node *np = NULL;
	struct mtk_panel_info *panel_info = NULL;
	int vpos_target = 0;
	int vneg_target = 0;

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

	ret = of_property_read_u32(np, KTZ8864_ONLY_BACKLIGHT, &bl_info.bl_only);
	if (ret < 0) {
		ktz8864_info("Is Not Only backlight\n");
		bl_info.bl_only = 0;
	}

	ret = of_property_read_u32(np, KTZ8864_HW_ENABLE, &bl_info.ktz8864_hw_en);
	if (ret < 0) {
		ktz8864_debug("get ktz8864_hw_en dts config failed\n");
		bl_info.ktz8864_hw_en = 0;
	}
	if (bl_info.ktz8864_hw_en != 0) {
		ret = of_property_read_u32(np, KTZ8864_HW_EN_GPIO, &bl_info.ktz8864_hw_en_gpio);
		if (ret < 0) {
			ktz8864_debug("get ktz8864_hw_en_gpio dts config fail\n");
		} else {
			/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
			panel_info = lcm_get_panel_info();
#else
			panel_info = lcdkit_mtk_common_panel.panel_info;
#endif
			if (!panel_info) {
				ktz8864_err("panel_info is NULL\n");
				return -1;
			}
			bl_info.ktz8864_hw_en_gpio += panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, KTZ8864_HW_EN_PULL_LOW,
			&bl_info.ktz8864_hw_en_pull_low);
		if (ret < 0) {
			ktz8864_err("get ktz8864_hw_en_pull_low dts config fail\n");
			bl_info.ktz8864_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, KTZ8864_HW_EN_DELAY, &bl_info.bl_on_kernel_mdelay);
	if (ret < 0)
		ktz8864_info("get bl_on_kernel_mdelay dts config failed\n");

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

	if (bl_info.bl_only == 0) {
		ktz8864_get_target_voltage(&vpos_target, &vneg_target);
		/* 9  is the position of vsp in ktz8864_reg */
		/* 10 is the position of vsn in ktz8864_reg */
		if (ktz8864_reg[9] != vpos_target)
			ktz8864_reg[9] = vpos_target;
		if (ktz8864_reg[10] != vneg_target)
			ktz8864_reg[10] = vneg_target;
	}

	ktz8864_info("ok!\n");

	return LCD_KIT_OK;

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

	if (bl_info.ktz8864_hw_en != 0 &&
		bl_info.ktz8864_hw_en_pull_low != 0) {
		gpio_set_value(bl_info.ktz8864_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(bl_info.ktz8864_hw_en_gpio);
	}
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

	if (bl_info.ktz8864_hw_en != 0 &&
		bl_info.ktz8864_hw_en_pull_low != 0) {
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
	if (val)
		LCD_KIT_ERR("check FAULT_FLAG error: val=0x%x\n", val);

#if defined CONFIG_HUAWEI_DSM
	for (i = 0; i < FLAG_CHECK_NUM; i++) {
		if (!(err_table[i].flag & val))
			continue;
		ktz8864_err("last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret) {
			ktz8864_err("dsm_client_ocuppy fail: ret=%d!\n", ret);
			continue;
		}
		dsm_client_record(lcd_dclient,
			"ktz8864 last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		dsm_client_notify(lcd_dclient, err_table[i].err_no);
	}
#endif
}

static void backlight_ic_check_fault(struct ktz8864_chip_data *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		ktz8864_err("pchip is NULL\n");
		return;
	}
	/* Judge power on or power off */
	if (ktz8864_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		ktz8864_check_fault(pchip, last_level, level);
	last_level = level;
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

	if (bl_info.ktz8864_hw_en != 0 &&
		bl_info.ktz8864_hw_en_pull_low != 0) {
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
	if (bl_info.ktz8864_hw_en != 0 &&
		bl_info.ktz8864_hw_en_pull_low != 0) {
		gpio_set_value(bl_info.ktz8864_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(bl_info.ktz8864_hw_en_gpio);
	}
	ktz8864_init_status = false;
}

static void ktz8864_dync_dimming_config(unsigned int bl_level)
{
	int ret = -1;
	static int bl_work_state = BL_POWER_ON;

	if (bl_info.bl_dimming_support != BL_DIMMING_SUPPORT)
		return;
	if (bl_level == 0) {
		if (bl_info.bl_dimming_resume != 0) {
			ret = regmap_write(ktz8864_g_chip->regmap,
				REG_BL_CONFIG_2,
				bl_info.bl_dimming_resume);
			if (ret < 0)
				ktz8864_err("dync dimming config error\n");
		}
		bl_work_state = BL_POWER_OFF;
		return;
	}
	if (bl_work_state == BL_POWER_OFF) {
		bl_work_state = BL_POWER_ON;
	} else if (bl_work_state == BL_POWER_ON) {
		if (bl_info.bl_dimming_config != 0) {
			ret = regmap_write(ktz8864_g_chip->regmap,
				REG_BL_CONFIG_2,
				bl_info.bl_dimming_config);
			if (ret < 0)
				ktz8864_err("dync dimming config error\n");
			ktz8864_info("config bl dimming reg[%d]=0x%x\n",
				REG_BL_CONFIG_2, bl_info.bl_dimming_config);
		}
		bl_work_state = BL_CONFIG_LEVEL_POWER_ON;
	}
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
	unsigned int bl_msb;
	unsigned int bl_lsb;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *panel_info = NULL;
#endif

	if (!ktz8864_g_chip) {
		ktz8864_err("init fail, return.\n");
		return ret;
	}

	if (down_trylock(&(ktz8864_g_chip->test_sem))) {
		ktz8864_info("Now in test mode\n");
		return 0;
	}

	/* first set backlight, enable ktz8864 */
	if (bl_info.bl_only != 0) {
		if ((ktz8864_init_status == false) && (bl_level > 0))
			ktz8864_enable();
	}

	if (ktz8864_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > BL_MAX)
			level = BL_MAX;
	}
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		ktz8864_err("panel_info is NULL\n");
		up(&(ktz8864_g_chip->test_sem));
		return ret;
	}
	if (panel_info->bl_ic_ctrl_mode == MTK_MIPI_BL_IC_PWM_MODE) {
		ktz8864_info("ktz8864 mipi pwm\n");
		backlight_ic_check_fault(ktz8864_g_chip, level);
		up(&(ktz8864_g_chip->test_sem));
		return LCD_KIT_OK;
	}
#endif

	ktz8864_dync_dimming_config(bl_level);
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
	backlight_ic_check_fault(ktz8864_g_chip, level);
	/* if set backlight level 0, disable ktz8864 */
	if (bl_info.bl_only != 0) {
		if (ktz8864_init_status == true && bl_level == 0)
			ktz8864_disable();
	}
	up(&(ktz8864_g_chip->test_sem));
	ktz8864_info("ktz8864_set_backlight_reg exit succ\n");
	return ret;

i2c_error:
	up(&(ktz8864_g_chip->test_sem));
	lcd_backlight_i2c_dmd();
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
	if (bl_info.ktz8864_hw_en) {
		ret = gpio_request(bl_info.ktz8864_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("ktz8864 Could not request hw_en_gpio\n");
	}
	if (bl_info.bl_only != 0) {
		lcd_kit_bl_register(&bl_ops);
	} else {
		lcd_kit_bias_register(&bias_ops);
		lcd_kit_bl_register(&bl_ops);
	}

	ktz8864_init_status = true;

	return ret;

err_sysfs:
	device_destroy(ktz8864_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

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

late_initcall(ktz8864_module_init);
module_exit(ktz8864_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Backlight driver for ktz8864");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
