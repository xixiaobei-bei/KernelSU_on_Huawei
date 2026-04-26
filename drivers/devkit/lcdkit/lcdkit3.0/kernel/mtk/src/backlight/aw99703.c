/*
 * aw99703.c
 *
 * aw99703 backlight driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "aw99703.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#include "backlight_linear_to_exp.h"
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#else
#include "lcm_drv.h"
#endif
#include "lcd_kit_disp.h"

static char *aw99703_dts_string[AW99703_RW_REG_MAX] = {
	"aw99703_reg_pfm_67",
	"aw99703_reg_boot_headroom_23",
	"aw99703_reg_pfm_threshold_25",
	"aw99703_reg_pfm_hys_24",
	"aw99703_reg_led_current",
	"aw99703_reg_boost_conctrol1",
	"aw99703_reg_af_high",
	"aw99703_reg_af_low",
	"aw99703_reg_soft_start_68",
	"aw99703_reg_boot_duty_31",
	"aw99703_reg_flash",
	"aw99703_reg_lsb",
	"aw99703_reg_msb",
	"aw99703_reg_boost_conctrol2",
	"aw99703_reg_mode",
	"aw99703_reg_boot_duty_31_1",
	"aw99703_reg_flash_1",
	"aw99703_reg_soft_start_68_1",
	"aw99703_reg_boost_conctrol2_1"
};

static unsigned int aw99703_reg_addr[AW99703_RW_REG_MAX] = {
	AW99703_REG_PFM_67,
	AW99703_REG_BOOT_HEADROOM_23,
	AW99703_REG_PFM_THRESHOLD_25,
	AW99703_REG_PFM_HYS_24,
	AW99703_REG_LED_CURRENT,
	AW99703_REG_BOOST_CONTROL1,
	AW99703_AUTO_FREQ_HIGH_THRESHOLD,
	AW99703_AUTO_FREQ_LOW_THRESHOLD,
	AW99703_REG_SOFT_START_68,
	AW99703_REG_BOOT_DUTY_31,
	AW99703_FLASH_SETTING,
	AW99703_REG_LED_BRIGHTNESS_LSB,
	AW99703_REG_LED_BRIGHTNESS_MSB,
	AW99703_REG_BOOST_CONTROL2,
	AW99703_REG_MODE,
	AW99703_REG_BOOT_DUTY_31_1,
	AW99703_FLASH_SETTING_1,
	AW99703_REG_SOFT_START_68_1,
	AW99703_REG_BOOST_CONTROL2_1
};

static struct aw99703_backlight_information aw99703_bl_info;
struct class *aw99703_class = NULL;
struct aw99703_chip_data *aw99703_g_chip = NULL;
static bool aw99703_init_status = true;
static int aw99703_10000_bl;
static int aw99703_fault_check_support;

static int aw99703_parse_dts(struct device_node *np)
{
	int ret;
	int i;
	struct mtk_panel_info *panel_info = NULL;

	if (np == NULL) {
		LCD_KIT_ERR("np is null pointer\n");
		return AW_FAIL;
	}

	for (i = 0; i < AW99703_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, aw99703_dts_string[i],
			&aw99703_bl_info.aw99703_reg[i]);
		if (ret < 0) {
			/* init to invalid data */
			aw99703_bl_info.aw99703_reg[i] =
				AW99703_INVALID_VAL;
			LCD_KIT_INFO("can not find config:%s\n",
				aw99703_dts_string[i]);
		}
	}

	ret = of_property_read_u32(np, AW99703_HW_ENABLE,
		&aw99703_bl_info.aw99703_hw_en);
	if (ret < 0) {
		LCD_KIT_ERR("aw99703_hw_enable use default value\n");
		aw99703_bl_info.aw99703_hw_en = 0;
	}
	if (aw99703_bl_info.aw99703_hw_en != 0) {
		ret = of_property_read_u32(np, AW99703_HW_EN_GPIO,
			&aw99703_bl_info.aw99703_hw_en_gpio);
		if (ret < 0) {
			LCD_KIT_ERR("get aw99703_hw_en_gpio fail\n");
		} else {
			/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
			panel_info = lcm_get_panel_info();
#else
			panel_info = lcdkit_mtk_common_panel.panel_info;
#endif
			if (panel_info == NULL) {
				LCD_KIT_ERR("panel_info is NULL\n");
				return AW_FAIL;
			}
			aw99703_bl_info.aw99703_hw_en_gpio +=
				panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, AW99703_HW_EN_PULL_LOW,
			&aw99703_bl_info.aw99703_hw_en_pull_low);
		if (ret < 0) {
			LCD_KIT_ERR("get aw99703_hw_en_pull_low fail\n");
			aw99703_bl_info.aw99703_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, AW99703_HW_EN_DELAY,
		&aw99703_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		aw99703_bl_info.bl_on_kernel_mdelay = 0;
		LCD_KIT_ERR("aw99703 bl_on_kernel_mdelay use default value\n");
	}
	ret = of_property_read_u32(np, AW99703_FAULT_CHECK_SUPPORT,
		&aw99703_fault_check_support);
	if (ret < 0) {
		aw99703_fault_check_support = 1;
		LCD_KIT_INFO("aw99703 fault_check_support use default value\n");
	}
	ret = of_property_read_u32(np, AW99703_10000_BACKLIGHT,
		&aw99703_10000_bl);
	if (ret < 0) {
		LCD_KIT_INFO("aw99703 is no support 10000 backlight!\n");
		aw99703_10000_bl = 0;
	}

	return AW_SUCC;
}

static int aw99703_config_write(
	struct aw99703_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = AW_FAIL;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LCD_KIT_ERR("pchip or reg or val is null pointer\n");
		return ret;
	}

	for (i = 0; i < size; i++) {
		/* judge reg is valid */
		if (val[i] != AW99703_INVALID_VAL) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			LCD_KIT_INFO("aw99703 write 0x%x = 0x%x",
				reg[i], val[i]);
			if (ret < 0) {
				LCD_KIT_ERR("write aw99703 0x%x fail\n",
					reg[i]);
				return ret;
			}
			if (reg[i] == AW99703_REG_MODE)
				mdelay(AW99703_REG_MODE_DELAY);
		}
	}

	return ret;
}

static int aw99703_config_read(
	struct aw99703_chip_data *pchip,
	unsigned int reg[],
	unsigned int val[],
	unsigned int size)
{
	int ret = 0;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LCD_KIT_ERR("pchip or reg or val is null pointer\n");
		return AW_FAIL;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			LCD_KIT_ERR("read aw99703 0x%x fail", reg[i]);
			return ret;
		} else {
			LCD_KIT_INFO("read 0x%x value 0x%x\n", reg[i], val[i]);
		}
	}

	return ret;
}

static ssize_t aw99703_reg_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct aw99703_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	unsigned int aw99703_reg_info[AW99703_RW_REG_MAX] = {0};
	ssize_t ret = AW_FAIL;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return ret;
	}

	if (dev == NULL) {
		ret = snprintf(buf, PAGE_SIZE, "dev is null\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL) {
		ret = snprintf(buf, PAGE_SIZE, "data is null\n");
		return ret;
	}

	client = pchip->client;
	if (client == NULL) {
		ret = snprintf(buf, PAGE_SIZE, "client is null\n");
		return ret;
	}

	ret = aw99703_config_read(pchip, aw99703_reg_addr,
		aw99703_reg_info, AW99703_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("aw99703 config read fail");
		ret = snprintf(buf, PAGE_SIZE,
			"%s: i2c access fail to register\n", __func__);
		return ret;
	}

	ret = snprintf(buf, PAGE_SIZE, "0x67 = 0x%x,0x23 = 0x%x,0x25 = 0x%x\n \
		\r0x24 = 0x%x,0x03 = 0x%x,0x04 = 0x%x,0x0c = 0x%x\n \
		\r0x0d = 0x%x,0x68 = 0x%x,0x31 = 0x%x\n \
		\r0x0b = 0x%x,0x06 = 0x%x,0x07 = 0x%x\n \
		\r0x05 = 0x%x,0x02 = 0x%x\n",
		aw99703_reg_info[0], aw99703_reg_info[1], aw99703_reg_info[2],
		aw99703_reg_info[3], aw99703_reg_info[4], aw99703_reg_info[5],
		aw99703_reg_info[6], aw99703_reg_info[7], aw99703_reg_info[8],
		aw99703_reg_info[9], aw99703_reg_info[10], aw99703_reg_info[11],
		aw99703_reg_info[12], aw99703_reg_info[13], aw99703_reg_info[14]);
	return ret;
}

static ssize_t aw99703_reg_store(
	struct device *dev,
	struct device_attribute *dev_attr,
	const char *buf, size_t size)
{
	ssize_t ret = AW_FAIL;
	struct aw99703_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return ret;
	}

	if (dev == NULL) {
		LCD_KIT_ERR("dev is null\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is null\n");
		return ret;
	}

	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x",
		&reg, &mask, &val);
	if (ret < 0) {
		LCD_KIT_INFO("check your input!!!\n");
		goto out_input;
	}

	LCD_KIT_INFO("%s: reg=0x%x,mask=0x%x,val=0x%x\n",
		__func__, reg, mask, val);

	ret = regmap_update_bits(pchip->regmap, reg, mask, val);
	if (ret < 0)
		goto i2c_error;

	return size;

i2c_error:
	dev_err(pchip->dev, "%s:i2c access fail to register\n", __func__);
	ret = snprintf((char *)buf, PAGE_SIZE, "%s: i2c access reg fail\n",
		__func__);
	return ret;

out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	ret = snprintf((char *)buf, PAGE_SIZE, "%s: input conversion fail\n",
		__func__);
	return ret;
}

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), aw99703_reg_show,
	aw99703_reg_store);

/* pointers to created device attributes */
static struct attribute *aw99703_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group aw99703_group = {
	.attrs = aw99703_attributes,
};

/* set default reg value */
static const struct regmap_config aw99703_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

static int aw99703_chip_enable(struct aw99703_chip_data *pchip)
{
	int ret = AW_FAIL;

	LCD_KIT_INFO("chip enable in!\n");

	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is NULL pointer\n");
		return ret;
	}

	ret = aw99703_config_write(pchip, aw99703_reg_addr,
		aw99703_bl_info.aw99703_reg, AW99703_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("aw99703 i2c config register failed");
		return ret;
	}

	LCD_KIT_INFO("chip enable ok!\n");
	return ret;
}

static void aw99703_enable(void)
{
	int ret;

	if (aw99703_bl_info.aw99703_hw_en != 0 &&
		aw99703_bl_info.aw99703_hw_en_pull_low != 0)  {
		ret = gpio_request(aw99703_bl_info.aw99703_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("aw99703 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(aw99703_bl_info.aw99703_hw_en_gpio,
			GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("aw99703 set gpio output not success\n");
		gpio_set_value(aw99703_bl_info.aw99703_hw_en_gpio,
			GPIO_OUT_ONE);
		if (aw99703_bl_info.bl_on_kernel_mdelay)
			mdelay(aw99703_bl_info.bl_on_kernel_mdelay);
	}
	/* chip initialize */
	ret = aw99703_chip_enable(aw99703_g_chip);
	if (ret < 0) {
		LCD_KIT_ERR("aw99703_chip_init fail!\n");
		return;
	}

	aw99703_init_status = true;
}

static void aw99703_disable(void)
{
	int ret;

	ret = regmap_write(aw99703_g_chip->regmap, AW99703_REG_RESET, AW99703_REG_RESET);
	if (ret < 0)
		LCD_KIT_ERR("aw99703 write reset %d fail\n", ret);
	if (aw99703_bl_info.aw99703_hw_en != 0 &&
		aw99703_bl_info.aw99703_hw_en_pull_low != 0) {
		gpio_set_value(aw99703_bl_info.aw99703_hw_en_gpio,
			GPIO_OUT_ZERO);
		gpio_free(aw99703_bl_info.aw99703_hw_en_gpio);
	}

	aw99703_init_status = false;
}

static void aw99703_check_fault(struct aw99703_chip_data *pchip,
	int last_level, int level)
{
	unsigned int val1 = 0;
	unsigned int val2 = 0;
	int ret;

	LCD_KIT_INFO("check REG_FAULT_FLAG!\n");
	ret = regmap_read(pchip->regmap, AW99703_FLAGS1_SETTING,
		&val1);
	if (ret) {
		LCD_KIT_ERR("aw99703 read REG_STATUS_FLAG1 fail !\n");
		return;
	}
	if (val1)
		LCD_KIT_ERR("check FAULT_FLAG error: val1=0x%x\n", val1);
	ret = regmap_read(pchip->regmap, AW99703_FLAGS2_SETTING,
		&val2);
	if (ret) {
		LCD_KIT_ERR("aw99703 read REG_STATUS_FLAG2 fail !\n");
		return;
	}
	if (val2)
		LCD_KIT_ERR("check FAULT_FLAG error: val2=0x%x\n", val2);
#if defined(CONFIG_HUAWEI_DSM)
	if(val1 & 1 << AW99703_OCP_BIT) {
		LCD_KIT_ERR("last_bkl:%d, cur_bkl:%d, FAULT_FLAG1:0x%x!\n",
			last_level, level, val1);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (!ret) {
			dsm_client_record(lcd_dclient,
				"aw99703 last_bkl:%d, cur_bkl:%d\n"
				"REG_STATUS_FLAG1:0x%x!\n",
				last_level, level, val1);
			dsm_client_notify(lcd_dclient,
				DSM_LCD_BACKLIGHT_OCP_ERROR_NO);
		} else {
			dev_err(&pchip->client->dev,
				"dsm_client_ocuppy flag1 fail: ret=%d!\n", ret);
		}
	}

	if(val2 & 1 << AW99703_OVP_BIT) {
		LCD_KIT_ERR("last_bkl:%d, cur_bkl:%d, FAULT_FLAG2:0x%x!\n",
			last_level, level, val2);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (!ret) {
			dsm_client_record(lcd_dclient,
				"aw99703 last_bkl:%d, cur_bkl:%d\n"
				"REG_STATUS_FLAG2:0x%x!\n",
				last_level, level, val2);
			dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
		} else {
			dev_err(&pchip->client->dev,
				"dsm_client_ocuppy flag2 fail: ret=%d!\n", ret);
		}
	}
#endif
}

static void aw99703_backlight_ic_check_fault(struct aw99703_chip_data *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is NULL\n");
		return;
	}
	/* Judge power on or power off */
	if (aw99703_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		aw99703_check_fault(pchip, last_level, level);
	last_level = level;
}

int aw99703_set_backlight(unsigned int bl_level)
{
	int bl_msb;
	int bl_lsb;
	int ret;
	unsigned int level;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *panel_info = NULL;
#endif

	if (aw99703_g_chip == NULL) {
		LCD_KIT_ERR("aw99703_g_chip is null\n");
		return AW_FAIL;
	}

	if (down_trylock(&(aw99703_g_chip->test_sem))) {
		LCD_KIT_INFO("Now in test mode\n");
		return AW_SUCC;
	}

	/* first set backlight, enable aw99703 */
	if ((aw99703_init_status == false) && (bl_level > 0))
		aw99703_enable();

	if (aw99703_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > AW99703_BL_MAX)
			level = AW99703_BL_MAX;
	}
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		LCD_KIT_ERR("panel_info is NULL\n");
		up(&(aw99703_g_chip->test_sem));
		return AW_FAIL;
	}
	if (panel_info->bl_ic_ctrl_mode == MTK_MIPI_BL_IC_PWM_MODE) {
		LCD_KIT_INFO("panel backlight is mipi pwm mode\n");
		aw99703_backlight_ic_check_fault(aw99703_g_chip, level);
		up(&(aw99703_g_chip->test_sem));
		return AW_SUCC;
	}
#endif
	/* set backlight level */
	bl_msb = (level >> AW99703_LSB_LEN) & AW99703_BL_MSB_MASK;
	bl_lsb = level & AW99703_BL_LSB_MASK;
	ret = regmap_write(aw99703_g_chip->regmap,
		AW99703_REG_LED_BRIGHTNESS_LSB, bl_lsb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write aw99703 backlight level lsb:0x%x fail\n",
			bl_lsb);
	}

	ret = regmap_write(aw99703_g_chip->regmap,
		AW99703_REG_LED_BRIGHTNESS_MSB, bl_msb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write aw99703 backlight level msb:0x%x fail\n",
			bl_msb);
	}

	aw99703_backlight_ic_check_fault(aw99703_g_chip, level);
	/* if set backlight level 0, disable aw99703 */
	if ((aw99703_init_status == true) && (level == 0))
		aw99703_disable();

	up(&(aw99703_g_chip->test_sem));
	LCD_KIT_INFO("write aw99703 backlight %u success\n", level);
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = aw99703_set_backlight,
	.name = "aw9973",
};

static int aw99703_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct aw99703_chip_data *pchip = NULL;
	struct device_node *np = NULL;
	int ret = AW_FAIL;

	LCD_KIT_INFO("in %s!\n", __func__);
	if (client == NULL) {
		LCD_KIT_ERR("client is null pointer\n");
		return ret;
	}
	adapter = client->adapter;
	if (adapter == NULL) {
		LCD_KIT_ERR("adapter is null pointer\n");
		return ret;
	}
	np = client->dev.of_node;
	if (np == NULL) {
		LCD_KIT_ERR("np is null pointer\n");
		return ret;
	}
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		LCD_KIT_ERR("i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev, sizeof(struct aw99703_chip_data),
		GFP_KERNEL);
	if (pchip == NULL) {
		LCD_KIT_ERR("devm_kzalloc fail\n");
		return -ENOMEM;
	}

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &aw99703_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		LCD_KIT_ERR("allocate register map fail: %d\n", ret);
		goto err_init;
	}
#endif
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(aw99703_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		LCD_KIT_ERR("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &aw99703_group);
		if (ret)
			goto err_sysfs;
	}
	aw99703_g_chip = pchip;
	ret = aw99703_parse_dts(np);
	if (ret < 0) {
		LCD_KIT_ERR("parse aw99703 dts fail");
		goto err_sysfs;
	}
	if (aw99703_bl_info.aw99703_hw_en) {
		ret = gpio_request(aw99703_bl_info.aw99703_hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("aw99703 Could not request hw_en_gpio\n");
			goto err_sysfs;
		}
	}
	lcd_kit_bl_register(&bl_ops);
	return AW_SUCC;

err_sysfs:
	LCD_KIT_ERR("sysfs error!\n");
	device_destroy(aw99703_class, 0);
	aw99703_class = NULL;
err_init:
	if (pchip != NULL) {
		devm_kfree(&client->dev, pchip);
		aw99703_g_chip = NULL;
	}
	return ret;
}

static int aw99703_remove(struct i2c_client *client)
{
	if (client == NULL) {
		LCD_KIT_ERR("client is null pointer\n");
		return AW_FAIL;
	}
	if (aw99703_class != NULL)
		device_destroy(aw99703_class, 0);

	sysfs_remove_group(&client->dev.kobj, &aw99703_group);
	if (aw99703_g_chip != NULL)
		devm_kfree(&client->dev, aw99703_g_chip);

	return AW_SUCC;
}


static const struct i2c_device_id aw99703_id[] = {
	{ AW99703_NAME, 0 },
	{},
};

static const struct of_device_id aw99703_of_id_table[] = {
	{ .compatible = DTS_COMP_AW99703 },
	{},
};

MODULE_DEVICE_TABLE(i2c, aw99703_id);
static struct i2c_driver aw99703_i2c_driver = {
	.driver = {
		.name = AW99703_NAME,
		.owner = THIS_MODULE,
		.of_match_table = aw99703_of_id_table,
	},
	.probe = aw99703_probe,
	.remove = aw99703_remove,
	.id_table = aw99703_id,
};

static int __init aw99703_init(void)
{
	int ret;

	LCD_KIT_INFO("%s init\n", __func__);
	aw99703_class = class_create(THIS_MODULE, AW99703_NAME);
	if (IS_ERR(aw99703_class)) {
		ret = PTR_ERR(aw99703_class);
		LCD_KIT_ERR("Unable to create aw99703 class; errno = %ld\n",
			ret);
		aw99703_class = NULL;
		return ret;
	}

	ret = i2c_add_driver(&aw99703_i2c_driver);
	if (ret)
		LCD_KIT_ERR("Unable to register aw99703 driver\n");

	LCD_KIT_INFO("%s ok\n", __func__);
	return ret;
}

static void __exit aw99703_exit(void)
{
	i2c_del_driver(&aw99703_i2c_driver);
}

late_initcall(aw99703_init);
module_exit(aw99703_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");
