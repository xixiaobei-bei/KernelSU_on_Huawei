/*
* Simple driver for Texas Instruments LM3639 Backlight + Flash LED driver chip
* Copyright (C) 2012 Texas Instruments
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*/
#include "lm36274.h"
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

static struct lm36274_voltage lm36274_vol_table[] = {
	{ 4000000, LM36274_VOL_400 },
	{ 4050000, LM36274_VOL_405 },
	{ 4100000, LM36274_VOL_410 },
	{ 4150000, LM36274_VOL_415 },
	{ 4200000, LM36274_VOL_420 },
	{ 4250000, LM36274_VOL_425 },
	{ 4300000, LM36274_VOL_430 },
	{ 4350000, LM36274_VOL_435 },
	{ 4400000, LM36274_VOL_440 },
	{ 4450000, LM36274_VOL_445 },
	{ 4500000, LM36274_VOL_450 },
	{ 4550000, LM36274_VOL_455 },
	{ 4600000, LM36274_VOL_460 },
	{ 4650000, LM36274_VOL_465 },
	{ 4700000, LM36274_VOL_470 },
	{ 4750000, LM36274_VOL_475 },
	{ 4800000, LM36274_VOL_480 },
	{ 4850000, LM36274_VOL_485 },
	{ 4900000, LM36274_VOL_490 },
	{ 4950000, LM36274_VOL_495 },
	{ 5000000, LM36274_VOL_500 },
	{ 5050000, LM36274_VOL_505 },
	{ 5100000, LM36274_VOL_510 },
	{ 5150000, LM36274_VOL_515 },
	{ 5200000, LM36274_VOL_520 },
	{ 5250000, LM36274_VOL_525 },
	{ 5300000, LM36274_VOL_530 },
	{ 5350000, LM36274_VOL_535 },
	{ 5400000, LM36274_VOL_540 },
	{ 5450000, LM36274_VOL_545 },
	{ 5500000, LM36274_VOL_550 },
	{ 5550000, LM36274_VOL_555 },
	{ 5600000, LM36274_VOL_560 },
	{ 5650000, LM36274_VOL_565 },
	{ 5700000, LM36274_VOL_570 },
	{ 5750000, LM36274_VOL_575 },
	{ 5800000, LM36274_VOL_580 },
	{ 5850000, LM36274_VOL_585 },
	{ 5900000, LM36274_VOL_590 },
	{ 5950000, LM36274_VOL_595 },
	{ 6000000, LM36274_VOL_600 },
	{ 6050000, LM36274_VOL_605 },
	{ 6400000, LM36274_VOL_640 },
	{ 6450000, LM36274_VOL_645 },
	{ 6500000, LM36274_VOL_650 },
};
struct class *lm36274_class = NULL;
struct lm36274_chip_data *lm36274_g_chip = NULL;
static bool lm36274_init_status = false;
static int lm36274_fault_check_support;
static int lm36274_10000_bl;
static int lm36274_reg[LM36274_RW_REG_MAX] = { 0 };
unsigned lm36274_msg_level = 7;

static struct backlight_information bl_info;
#if defined(CONFIG_HUAWEI_DSM)
static struct lm36274_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};
#endif
/*
 * for debug, S_IRUGO
 * /sys/module/hisifb/parameters
 */
module_param_named(debug_lm36274_msg_level, lm36274_msg_level, int, 0644);
MODULE_PARM_DESC(debug_lm36274_msg_level, "backlight lm36274 msg level");

static int lm36274_config_write(struct lm36274_chip_data *pchip,
			unsigned int reg[],unsigned int val[],unsigned int size)
{
	int ret;
	unsigned int i;

	if((pchip == NULL) || (reg == NULL) || (val == NULL)){
		lm36274_err("pchip or reg or val is NULL pointer \n");
		return -1;
	}

	for(i = 0;i < size;i++) {
		ret = regmap_write(pchip->regmap, reg[i], val[i]);
		if (ret < 0) {
			lm36274_err("write lm36274 backlight config register 0x%x failed\n",reg[i]);
			goto exit;
		} else {
			lm36274_info("register 0x%x value = 0x%x\n", reg[i], val[i]);
		}
	}

exit:
	return ret;
}

static int lm36274_parse_dts(struct device_node *np)
{
	int ret;
	int i;
	struct mtk_panel_info *panel_info = NULL;

	if(np == NULL){
		lm36274_err("np is NULL pointer \n");
		return -1;
	}

	for (i = 0;i < LM36274_RW_REG_MAX;i++ ) {
		ret = of_property_read_u32(np, lm36274_dts_string[i], &lm36274_reg[i]);
		if (ret < 0) {
			lm36274_err("get lm36274 dts config failed\n");
		} else {
			lm36274_info("get %s from dts value = 0x%x\n", lm36274_dts_string[i],
				lm36274_reg[i]);
		}
	}
	ret = of_property_read_u32(np, "lm36274_check_fault_support", &lm36274_fault_check_support);
	if (ret < 0)
		lm36274_info("No need to detect fault flags \n");

	ret = of_property_read_u32(np, "lm36274_10000_bl", &lm36274_10000_bl);
	if (ret < 0)
		lm36274_10000_bl = 0;

	ret = of_property_read_u32(np, LM36274_ONLY_BACKLIGHT, &bl_info.bl_only);
	if (ret < 0) {
		lm36274_info("parse bl_only fail, use default value\n");
		bl_info.bl_only = 0;
	}

	ret = of_property_read_u32(np, LM36274_HW_ENABLE, &bl_info.lm36274_hw_en);
	if (ret < 0) {
		lm36274_err("get lm36274_hw_en dts config failed\n");
		bl_info.lm36274_hw_en = 0;
	}
	if (bl_info.lm36274_hw_en != 0) {
		ret = of_property_read_u32(np, LM36274_HW_EN_GPIO, &bl_info.lm36274_hw_en_gpio);
		if (ret < 0) {
			lm36274_err("get lm36274_hw_en_gpio dts config fail\n");
			return ret;
		} else {
			/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
			panel_info = lcm_get_panel_info();
#else
			panel_info = lcdkit_mtk_common_panel.panel_info;
#endif
			if (!panel_info) {
				lm36274_err("panel_info is NULL\n");
				return -1;
			}
			bl_info.lm36274_hw_en_gpio += panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, LM36274_HW_EN_PULL_LOW,
			&bl_info.lm36274_hw_en_pull_low);
		if (ret < 0) {
			lm36274_err("get lm36274_hw_en_pull_low dts config fail\n");
			bl_info.lm36274_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, LM36274_HW_EN_DELAY, &bl_info.bl_on_kernel_mdelay);
	if (ret < 0)
		lm36274_err("get bl_on_kernel_mdelay dts config failed\n");
	ret = of_property_read_u32(np, "lm36274_dimming_support",
		&bl_info.bl_dimming_support);
	if (ret < 0)
		lm36274_err("get bl_dimming_support dts config failed\n");
	ret = of_property_read_u32(np, "lm36274_dimming_config",
		&bl_info.bl_dimming_config);
	if (ret < 0)
		lm36274_err("get bl_dimming_config dts config failed\n");
	ret = of_property_read_u32(np, "lm36274_dimming_resume",
		&bl_info.bl_dimming_resume);
	if (ret < 0)
		lm36274_err("get bl_dimming_resume dts config failed\n");
	return LCD_KIT_OK;
}

static void lm36274_get_target_voltage(int *vpos, int *vneg)
{
	int i;
	int tal_size = sizeof(lm36274_vol_table) / sizeof(struct lm36274_voltage);

	if ((vpos == NULL) || (vneg == NULL)) {
		LCD_KIT_ERR("vpos or vneg is null\n");
		return;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (lm36274_vol_table[i].voltage == power_hdl->lcd_vsp.buf[2]) {
			*vpos = lm36274_vol_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsp voltage, use default voltage\n");
		*vpos = LM36274_VOL_600;
	}

	for (i = 0; i < tal_size; i++) {
		/* set bias voltage buf[2] means bias value */
		if (lm36274_vol_table[i].voltage == power_hdl->lcd_vsn.buf[2]) {
			*vneg = lm36274_vol_table[i].value;
			break;
		}
	}

	if (i >= tal_size) {
		LCD_KIT_INFO("not found vsn voltage, use default voltage\n");
		*vneg = LM36274_VOL_600;
	}

	LCD_KIT_INFO("*vpos_target=0x%x,*vneg_target=0x%x\n", *vpos, *vneg);

	return;
}

/* initialize chip */
static int lm36274_chip_init(struct lm36274_chip_data *pchip)
{
	int ret = -1;
	struct device_node *np = NULL;
	int vpos_target = 0;
	int vneg_target = 0;

	lm36274_info("in!\n");

	if (pchip == NULL) {
		lm36274_err("pchip is NULL pointer\n");
		return ret;
	}

	memset(&bl_info, 0, sizeof(struct backlight_information));

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LM36274);
	if (!np) {
		lm36274_err("NOT FOUND device node %s!\n", DTS_COMP_LM36274);
		goto out;
	}

	ret = lm36274_parse_dts(np);
	if (ret < 0) {
		lm36274_err("parse lm36274 dts failed");
		goto out;
	}

	if (bl_info.bl_only == 0) {
		lm36274_get_target_voltage(&vpos_target, &vneg_target);
		/* 9  is the position of vsp in lm36274_reg */
		/* 10 is the position of vsn in lm36274_reg */
		if (lm36274_reg[9] != vpos_target)
			lm36274_reg[9] = vpos_target;
		if (lm36274_reg[10] != vneg_target)
			lm36274_reg[10] = vneg_target;
	}

	lm36274_info("ok!\n");

	return ret;

out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

static void lm36274_check_fault(struct lm36274_chip_data *pchip,
	int last_level, int level)
{
	unsigned int val = 0;
	int ret;
	int i;

	lm36274_info("backlight check FAULT_FLAG!\n");

	ret = regmap_read(pchip->regmap, REG_FLAGS, &val);
	if (ret < 0) {
		lm36274_err("read lm36274 FAULT_FLAG failed!\n");
		return;
	}
	if (val)
		LCD_KIT_ERR("check FAULT_FLAG error: val=0x%x\n", val);

#if defined CONFIG_HUAWEI_DSM
	for (i = 0; i < FLAG_CHECK_NUM; i++) {
		if (!(err_table[i].flag & val))
			continue;
		lm36274_err("last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret) {
			lm36274_err("dsm_client_ocuppy fail: ret=%d!\n", ret);
			continue;
		}
		dsm_client_record(lcd_dclient,
			"lm36274 last_bkl:%d, cur_bkl:%d\n FAULT_FLAG:0x%x!\n",
			last_level, level, err_table[i].flag);
		dsm_client_notify(lcd_dclient, err_table[i].err_no);
	}
#endif
}

static void backlight_ic_check_fault(struct lm36274_chip_data *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		lm36274_err("pchip is NULL\n");
		return;
	}
	/* Judge power on or power off */
	if (lm36274_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		lm36274_check_fault(pchip, last_level, level);
	last_level = level;
}

static int lm36274_chip_enable(struct lm36274_chip_data *pchip)
{
	int ret = -1;

	if (pchip == NULL) {
		lm36274_err("pchip is NULL pointer\n");
		return ret;
	}

	ret =  lm36274_config_write(pchip, lm36274_reg_addr,
		lm36274_reg, LM36274_RW_REG_MAX);
	if (ret < 0) {
		lm36274_err("lm36274 config register failed");
		goto out;
	}

	lm36274_info("chip enable ok!\n");

	return ret;

out:
	lm36274_err("i2c failed to access register\n");
	return ret;
}

static void lm36274_enable(void)
{
	int ret;

	if (bl_info.lm36274_hw_en != 0 &&
		bl_info.lm36274_hw_en_pull_low != 0) {
		ret = gpio_request(bl_info.lm36274_hw_en_gpio, NULL);
		if (ret)
			lm36274_err("lm36274 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(bl_info.lm36274_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			lm36274_err("lm36274 set gpio output not success\n");
		gpio_set_value(bl_info.lm36274_hw_en_gpio, GPIO_OUT_ONE);
		if (bl_info.bl_on_kernel_mdelay)
			mdelay(bl_info.bl_on_kernel_mdelay);
	}

	/* chip initialize */
	ret = lm36274_chip_enable(lm36274_g_chip);
	if (ret < 0) {
		lm36274_err("lm36274_chip_init fail!\n");
		return;
	}
	lm36274_init_status = true;
}

static void lm36274_disable(void)
{
	lm36274_err("lm36274_enable in\n");
	if (bl_info.lm36274_hw_en != 0 &&
		bl_info.lm36274_hw_en_pull_low != 0) {
		gpio_set_value(bl_info.lm36274_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(bl_info.lm36274_hw_en_gpio);
	}
	lm36274_init_status = false;
}

static void lm36274_dync_dimming_config(unsigned int bl_level)
{
	int ret = -1;
	static int bl_work_state = BL_POWER_ON;

	if (bl_info.bl_dimming_support != BL_DIMMING_SUPPORT)
		return;
	if (bl_level == 0) {
		if (bl_info.bl_dimming_resume != 0) {
			ret = regmap_write(lm36274_g_chip->regmap,
				REG_BL_CONFIG_2,
				bl_info.bl_dimming_resume);
			if (ret < 0)
				lm36274_err("dync dimming config error\n");
		}
		bl_work_state = BL_POWER_OFF;
		return;
	}
	if (bl_work_state == BL_POWER_OFF) {
		bl_work_state = BL_POWER_ON;
	} else if (bl_work_state == BL_POWER_ON) {
		if (bl_info.bl_dimming_config != 0) {
			ret = regmap_write(lm36274_g_chip->regmap,
				REG_BL_CONFIG_2,
				bl_info.bl_dimming_config);
			if (ret < 0)
				lm36274_err("dync dimming config error\n");
			lm36274_info("config bl dimming reg[%d]=0x%x\n",
				REG_BL_CONFIG_2, bl_info.bl_dimming_config);
		}
		bl_work_state = BL_CONFIG_LEVEL_POWER_ON;
	}
}

/**
 * lm36274_set_backlight(): Set Backlight working mode
 *
 * @bl_level: value for backlight ,range from 0 to 2047
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
int lm36274_set_backlight(unsigned int bl_level)
{
	int ret = -1;
	uint32_t level;
	unsigned int bl_msb;
	unsigned int bl_lsb;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *panel_info = NULL;
#endif

	if (!lm36274_g_chip) {
		lm36274_err("init fail, return.\n");
		return ret;
	}

	if (down_trylock(&(lm36274_g_chip->test_sem))) {
		lm36274_info("Now in test mode\n");
		return 0;
	}

	/* first set backlight, enable lm36274 */
	if (bl_info.bl_only != 0) {
		if ((lm36274_init_status == false) && (bl_level > 0))
			lm36274_enable();
	}

	lm36274_info("lm36274_set_backlight bl_level = %u \n", bl_level);

	if (lm36274_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > BL_MAX)
			level = BL_MAX;
	}
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		lm36274_err("panel_info is NULL\n");
		up(&(lm36274_g_chip->test_sem));
		return ret;
	}
	if (panel_info->bl_ic_ctrl_mode == MTK_MIPI_BL_IC_PWM_MODE) {
		backlight_ic_check_fault(lm36274_g_chip, level);
		up(&(lm36274_g_chip->test_sem));
		return LCD_KIT_OK;
	}
#endif
	lm36274_dync_dimming_config(bl_level);

	/* 11-bit brightness code */
	bl_msb = level >> MSB;
	bl_lsb = level & LSB;

	ret = regmap_update_bits(lm36274_g_chip->regmap, REG_BL_BRIGHTNESS_LSB, MASK_BL_LSB,bl_lsb);
	if (ret < 0) {
		goto i2c_error;
	}

	ret = regmap_write(lm36274_g_chip->regmap, REG_BL_BRIGHTNESS_MSB, bl_msb);
	if (ret < 0) {
		goto i2c_error;
	}
	backlight_ic_check_fault(lm36274_g_chip, level);
	/* if set backlight level 0, disable lm36274 */
	if (bl_info.bl_only != 0) {
		if (lm36274_init_status == true && bl_level == 0)
			lm36274_disable();
	}

	up(&(lm36274_g_chip->test_sem));
	lm36274_info("lm36274_set_backlight exit succ \n");
	return ret;

i2c_error:
	up(&(lm36274_g_chip->test_sem));
	lcd_backlight_i2c_dmd();
	dev_err(lm36274_g_chip->dev, "%s:i2c access fail to register\n", __func__);
	lm36274_info("lm36274_set_backlight exit fail \n");
	return ret;
}

/**
 * lm36274_set_reg(): Set lm36274 reg
 *
 * @bl_reg: which reg want to write
 * @bl_mask: which bits of reg want to change
 * @bl_val: what value want to write to the reg
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t lm36274_set_reg(u8 bl_reg,u8 bl_mask,u8 bl_val)
{
	ssize_t ret = -1;
	u8 reg = bl_reg;
	u8 mask = bl_mask;
	u8 val = bl_val;

	if (!lm36274_init_status) {
		lm36274_err("init fail, return.\n");
		return ret;
	}

	if (REG_MAX < reg) {
		lm36274_err("Invalid argument!!!\n");
		return ret;
	}

	lm36274_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask, val);

	ret = regmap_update_bits(lm36274_g_chip->regmap, reg, mask, val);
	if (ret < 0) {
		lm36274_err("i2c access fail to register\n");
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(lm36274_set_reg);

static ssize_t lm36274_reg_bl_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lm36274_chip_data *pchip = NULL;
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
	if(!client)
		return snprintf(buf, PAGE_SIZE, "client is null\n");

	ret = regmap_read(pchip->regmap, REG_BL_BRIGHTNESS_MSB, &bl_msb);
	if(ret < 0)
		return snprintf(buf, PAGE_SIZE, "LM36274 I2C read error\n");

	ret = regmap_read(pchip->regmap, REG_BL_BRIGHTNESS_LSB, &bl_lsb);
	if(ret < 0)
		return snprintf(buf, PAGE_SIZE, "LM36274 I2C read error\n");

	bl_level = (bl_msb << MSB) | bl_lsb;

	return snprintf(buf, PAGE_SIZE, "LM36274 bl_level:%d\n", bl_level);
}

static ssize_t lm36274_reg_bl_store(struct device *dev,
					struct device_attribute *devattr,
					const char *buf, size_t size)
{
	ssize_t ret;
	struct lm36274_chip_data *pchip = NULL;
	unsigned int bl_level = 0;
	unsigned int bl_msb;
	unsigned int bl_lsb;

	if (!dev) {
		lm36274_err("dev is null\n");
		return -1;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		lm36274_err("data is null\n");
		return -1;
	}

	ret = kstrtouint(buf, 10, &bl_level);
	if (ret) {
		goto out_input;
	}

	lm36274_info("%s:buf=%s,state=%d\n", __func__, buf, bl_level);

	if (bl_level > BL_MAX)
		bl_level = BL_MAX;

	/* 11-bit brightness code */
	bl_msb = bl_level >> MSB;
	bl_lsb = bl_level & LSB;

	lm36274_info("bl_level = %d, bl_msb = %d, bl_lsb = %d\n", bl_level, bl_msb, bl_lsb);

	ret = regmap_update_bits(pchip->regmap, REG_BL_BRIGHTNESS_LSB, MASK_BL_LSB, bl_lsb);
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

static DEVICE_ATTR(reg_bl, (S_IRUGO|S_IWUSR), lm36274_reg_bl_show, lm36274_reg_bl_store);

static ssize_t lm36274_reg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lm36274_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret;
	unsigned char val[REG_MAX] = {0};

	if (!dev)
		return snprintf(buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (!pchip)
		return snprintf(buf, PAGE_SIZE, "data is null\n");

	client = pchip->client;
	if(!client)
		return snprintf(buf, PAGE_SIZE, "client is null\n");

	ret = regmap_bulk_read(pchip->regmap, REG_REVISION, &val[0], REG_MAX);
	if (ret < 0)
		goto i2c_error;

	return snprintf(buf, PAGE_SIZE, "Revision(0x01)= 0x%x\nBacklight Configuration1(0x02)= 0x%x\n \
			\rBacklight Configuration2(0x03) = 0x%x\nBacklight Brightness LSB(0x04) = 0x%x\n \
			\rBacklight Brightness MSB(0x05) = 0x%x\nBacklight Auto-Frequency Low(0x06) = 0x%x\n \
			\rBacklight Auto-Frequency High(0x07) = 0x%x\nBacklight Enable(0x08) = 0x%x\n \
			\rDisplay Bias Configuration 1(0x09)  = 0x%x\nDisplay Bias Configuration 2(0x0A)  = 0x%x\n \
			\rDisplay Bias Configuration 3(0x0B) = 0x%x\nLCM Boost Bias Register(0x0C) = 0x%x\n \
			\rVPOS Bias Register(0x0D) = 0x%x\nVNEG Bias Register(0x0E) = 0x%x\n \
			\rFlags Register(0x0F) = 0x%x\nBacklight Option 1 Register(0x10) = 0x%x\n \
			\rBacklight Option 2 Register(0x11) = 0x%x\nPWM-to-Digital Code Readback LSB(0x12) = 0x%x\n \
			\rPWM-to-Digital Code Readback MSB(0x13) = 0x%x\n",
			val[0],val[1],val[2],val[3],val[4],val[5],val[6],val[7],
			val[8],val[9],val[10],val[11],val[12],val[13],val[14],val[15],
			val[16],val[17],val[18]);

i2c_error:
	return snprintf(buf, PAGE_SIZE,"%s: i2c access fail to register\n", __func__);
}

static ssize_t lm36274_reg_store(struct device *dev,
					struct device_attribute *devattr,
					const char *buf, size_t size)
{
	ssize_t ret;
	struct lm36274_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!dev) {
		lm36274_err("dev is null\n");
		return -1;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		lm36274_err("data is null\n");
		return -1;
	}

	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x",&reg,&mask,&val);
	if (ret < 0) {
		printk("check your input!!!\n");
		goto out_input;
	}

	if (reg > REG_MAX) {
		printk("Invalid argument!!!\n");
		goto out_input;
	}

	lm36274_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask, val);

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

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), lm36274_reg_show, lm36274_reg_store);

static const struct regmap_config lm36274_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

/* pointers to created device attributes */
static struct attribute *lm36274_attributes[] = {
	&dev_attr_reg_bl.attr,
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group lm36274_group = {
	.attrs = lm36274_attributes,
};

static int lm36274_set_bias_voltage(int vpos, int vneg)
{
	int ret;

	if (vpos < 0 || vneg < 0) {
		lm36274_err("vpos or vneg is error\n");
		return -1;
	}
	if (bl_info.lm36274_hw_en != 0 &&
		bl_info.lm36274_hw_en_pull_low != 0) {
		ret = gpio_request(bl_info.lm36274_hw_en_gpio, NULL);
		if (ret)
			lm36274_err("lm36274 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(bl_info.lm36274_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			lm36274_err("lm36274 set gpio output not success\n");
		gpio_set_value(bl_info.lm36274_hw_en_gpio, GPIO_OUT_ONE);
		if (bl_info.bl_on_kernel_mdelay)
			mdelay(bl_info.bl_on_kernel_mdelay);
	}
	ret = lm36274_config_write(lm36274_g_chip, lm36274_reg_addr,
		lm36274_reg, LM36274_RW_REG_MAX);
	if (ret < 0)
		lm36274_err("i2c access fail to register\n");
	lm36274_init_status = true;

	return ret;
}

static int lm36274_set_ic_disable(void)
{
	int ret;

	if (!lm36274_g_chip)
		return -1;

	/* reset backlight ic */
	ret = regmap_write(lm36274_g_chip->regmap, REG_BL_ENABLE, BL_RESET);
	if (ret < 0)
		lm36274_err("i2c access fail to register\n");

	/* clean up bl val register */
	ret = regmap_update_bits(lm36274_g_chip->regmap, REG_BL_BRIGHTNESS_LSB,
		MASK_BL_LSB,BL_DISABLE);
	if (ret < 0)
		lm36274_err("i2c access fail to register\n");

	ret = regmap_write(lm36274_g_chip->regmap, REG_BL_BRIGHTNESS_MSB,
		BL_DISABLE);
	if (ret < 0)
		lm36274_err("i2c access fail to register\n");
	if (bl_info.lm36274_hw_en != 0 &&
		bl_info.lm36274_hw_en_pull_low != 0) {
		gpio_set_value(bl_info.lm36274_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(bl_info.lm36274_hw_en_gpio);
	}
	lm36274_init_status = false;

	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = lm36274_set_backlight,
	.name = "lm36274",
};

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = lm36274_set_bias_voltage,
	.set_ic_disable = lm36274_set_ic_disable,
};

static int lm36274_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct lm36274_chip_data *pchip = NULL;
	int ret = -1;

	lm36274_info("in!\n");

	if(client == NULL){
		lm36274_err("client is NULL pointer\n");
		return ret;
	}

	adapter = client->adapter;
	if(adapter == NULL){
		lm36274_err("adapter is NULL pointer\n");
		return ret;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev,
				sizeof(struct lm36274_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &lm36274_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "fail : allocate register map: %d\n", ret);
		goto err_out;
	}
#endif

	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	/* chip initialize */
	ret = lm36274_chip_init(pchip);
	if (ret < 0) {
		dev_err(&client->dev, "fail : chip init\n");
		goto err_out;
	}

	pchip->dev = device_create(lm36274_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		lm36274_err("Unable to create device; errno = %ld\n", PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &lm36274_group);
		if (ret)
			goto err_sysfs;
	}

	lm36274_g_chip = pchip;
	if (bl_info.lm36274_hw_en) {
		ret = gpio_request(bl_info.lm36274_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("lm36274 Could not request hw_en_gpio\n");
	}
	if (bl_info.bl_only != 0) {
		lcd_kit_bl_register(&bl_ops);
	} else {
		lcd_kit_bias_register(&bias_ops);
		lcd_kit_bl_register(&bl_ops);
	}
	lm36274_init_status = true;

	return ret;

err_sysfs:
	device_destroy(lm36274_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int lm36274_remove(struct i2c_client *client)
{
	struct lm36274_chip_data *pchip = i2c_get_clientdata(client);

	if (regmap_write(pchip->regmap, REG_BL_ENABLE, BL_DISABLE) < 0)
		lm36274_err("regmap_write REG_BL_ENABLE err\n");

	sysfs_remove_group(&client->dev.kobj, &lm36274_group);

	return 0;
}

static const struct i2c_device_id lm36274_id[] = {
	{LM36274_NAME, 0},
	{}
};

static const struct of_device_id lm36274_of_id_table[] = {
	{.compatible = "ti,lm36274"},
	{ },
};

MODULE_DEVICE_TABLE(i2c, lm36274_id);
static struct i2c_driver lm36274_i2c_driver = {
		.driver = {
			.name = "lm36274",
			.owner = THIS_MODULE,
			.of_match_table = lm36274_of_id_table,
		},
		.probe = lm36274_probe,
		.remove = lm36274_remove,
		.id_table = lm36274_id,
};

static int __init lm36274_module_init(void)
{
	int ret = -1;

	lm36274_info("in!\n");

	lm36274_class = class_create(THIS_MODULE, "lm36274");
	if (IS_ERR(lm36274_class)) {
		lm36274_err("Unable to create lm36274 class; errno = %ld\n", PTR_ERR(lm36274_class));
		lm36274_class = NULL;
	}

	ret = i2c_add_driver(&lm36274_i2c_driver);
	if (ret)
		lm36274_err("Unable to register lm36274 driver\n");

	lm36274_info("ok!\n");

	return ret;
}

static void __exit lm36274_module_exit(void)
{
	i2c_del_driver(&lm36274_i2c_driver);
}


late_initcall(lm36274_module_init);
module_exit(lm36274_module_exit);

MODULE_DESCRIPTION("Texas Instruments Backlight driver for LM36274");
MODULE_AUTHOR("Daniel Jeong <daniel.jeong@ti.com>");
MODULE_AUTHOR("G.Shark Jeong <gshark.jeong@gmail.com>");
MODULE_LICENSE("GPL v2");
