/* Copyright (c) 2019-2019, Huawei terminal Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include "sgm37603a.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#else
#include "lcm_drv.h"
#endif
#include "lcd_kit_disp.h"

static char *sgm37603a_dts_string[SGM37603A_RW_REG_MAX] = {
	"sgm37603a_reg_leds_config",
	"sgm37603a_reg_brightness_conctrol",
	"sgm37603a_reg_lsb",
	"sgm37603a_reg_msb"
};

static unsigned int sgm37603a_reg_addr[SGM37603A_RW_REG_MAX] = {
	SGM37603A_REG_LEDS_CONFIG,
	SGM37603A_REG_BRIGHTNESS_CONTROL,
	SGM37603A_REG_RATIO_LSB,
	SGM37603A_REG_RATIO_MSB
};

static struct sgm37603a_backlight_information sgm37603a_bl_info;
struct class *sgm37603a_class = NULL;
struct sgm37603a_chip_data *sgm37603a_g_chip = NULL;
static bool sgm37603a_init_status = true;
#ifndef CONFIG_DRM_MEDIATEK
extern struct LCM_DRIVER lcdkit_mtk_common_panel;
#endif
#if defined(CONFIG_HUAWEI_DSM)
struct sgm37603a_check_flag err_table[] = {
	{ SGM37603A_OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO },
	{ SGM37603A_OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO },
};
#endif
static int sgm37603a_parse_dts(struct device_node *np)
{
	int ret;
	int i;
	struct mtk_panel_info *panel_info = NULL;

	if (np == NULL) {
		LCD_KIT_ERR("np is null pointer\n");
		return SGM_FAIL;
	}

	for (i = 0; i < SGM37603A_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, sgm37603a_dts_string[i],
			&sgm37603a_bl_info.sgm37603a_reg[i]);
		if (ret < 0) {
			/* init to invalid data */
			sgm37603a_bl_info.sgm37603a_reg[i] =
				SGM37603A_INVALID_VAL;
			LCD_KIT_INFO("can not find config:%s\n",
				sgm37603a_dts_string[i]);
		}
	}

	ret = of_property_read_u32(np, SGM37603A_HW_ENABLE,
		&sgm37603a_bl_info.sgm37603a_hw_en);
	if (ret < 0) {
		LCD_KIT_ERR("sgm37603a_hw_enable use default value\n");
		sgm37603a_bl_info.sgm37603a_hw_en = 0;
	}

	ret = of_property_read_u32(np, SGM37603A_HW_EN_GPIO,
		&sgm37603a_bl_info.sgm37603a_hw_en_gpio);
	if (ret < 0) {
		LCD_KIT_ERR("get sgm37603a_hw_en_gpio dts config fail\n");
		return ret;
	}

	/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
#else
	panel_info = lcdkit_mtk_common_panel.panel_info;
#endif
	if (!panel_info) {
		LCD_KIT_ERR("panel_info is NULL\n");
		return SGM_FAIL;
	}

	sgm37603a_bl_info.sgm37603a_hw_en_gpio += panel_info->gpio_offset;
	ret = of_property_read_u32(np, SGM37603A_HW_EN_DELAY,
		&sgm37603a_bl_info.bl_on_lk_mdelay);
	if (ret < 0) {
		LCD_KIT_ERR("get bl_on_kernel_mdelay dts config fail\n");
		return ret;
	}

	ret = of_property_read_u32(np, SGM37603A_BL_LEVEL,
		&sgm37603a_bl_info.bl_level);
	if (ret < 0) {
		LCD_KIT_ERR("get bl_level dts config fail\n");
		return ret;
	}

	return ret;
}

static int sgm37603a_config_write(
	struct sgm37603a_chip_data *pchip,
	unsigned int reg[],
	unsigned int val[],
	unsigned int size)
{
	int ret = SGM_FAIL;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LCD_KIT_ERR("pchip or reg or val is null pointer\n");
		return ret;
	}

	for (i = 0; i < size; i++) {
		/* judge reg is valid */
		if (val[i] != SGM37603A_INVALID_VAL) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			LCD_KIT_INFO("sgm37603a write 0x%x = 0x%x", reg[i], val[i]);
			if (ret < 0) {
				LCD_KIT_ERR("write sgm37603a 0x%x fail\n",
					reg[i]);
				return ret;
			}
		}
	}

	return ret;
}

static int sgm37603a_config_read(
	struct sgm37603a_chip_data *pchip,
	unsigned int reg[],
	unsigned int val[],
	unsigned int size)
{
	int ret = 0;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LCD_KIT_ERR("pchip or reg or val is null pointer\n");
		return SGM_FAIL;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			LCD_KIT_ERR("read sgm37603a 0x%x fail", reg[i]);
			return ret;
		} else {
			LCD_KIT_INFO("read 0x%x value 0x%x\n", reg[i], val[i]);
		}
	}

	return ret;
}

/* initialize chip */
static int sgm37603a_chip_init(struct sgm37603a_chip_data *pchip)
{
	int ret = SGM_FAIL;

	LCD_KIT_INFO("%s in!\n", __func__);
	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is null pointer\n");
		return ret;
	}

	ret = sgm37603a_config_write(pchip, sgm37603a_reg_addr,
		sgm37603a_bl_info.sgm37603a_reg, SGM37603A_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("sgm37603a config register fail");
		return ret;
	}
	LCD_KIT_INFO("%s ok!\n", __func__);
	return ret;
}

static ssize_t sgm37603a_reg_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct sgm37603a_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	unsigned int sgm37603a_reg_info[SGM37603A_RW_REG_MAX] = {0};
	ssize_t ret = SGM_FAIL;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return ret;
	}

	if (!dev) {
		ret = snprintf(buf, PAGE_SIZE, "dev is null\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		ret = snprintf(buf, PAGE_SIZE, "data is null\n");
		return ret;
	}

	client = pchip->client;
	if (!client) {
		ret = snprintf(buf, PAGE_SIZE, "client is null\n");
		return ret;
	}

	ret = sgm37603a_config_read(pchip, sgm37603a_reg_addr,
		sgm37603a_reg_info, SGM37603A_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("sgm37603a config read fail");
		ret = snprintf(buf, PAGE_SIZE,
			"%s: i2c access fail to register\n", __func__);
		return ret;
	}

	ret = snprintf(buf, PAGE_SIZE, "0x10 = 0x%x 0x11 = 0x%x\n \
		\r0x1A = 0x%x 0x19 = 0x%x\n ",
		sgm37603a_reg_info[0],
		sgm37603a_reg_info[1],
		sgm37603a_reg_info[2],
		sgm37603a_reg_info[3]);
	return ret;
}

static ssize_t sgm37603a_reg_store(
	struct device *dev,
	struct device_attribute *dev_attr,
	const char *buf, size_t size)
{
	ssize_t ret = SGM_FAIL;
	struct sgm37603a_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return ret;
	}

	if (!dev) {
		LCD_KIT_ERR("dev is null\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
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

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), sgm37603a_reg_show,
	sgm37603a_reg_store);

/* pointers to created device attributes */
static struct attribute *sgm37603a_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group sgm37603a_group = {
	.attrs = sgm37603a_attributes,
};

/* set default reg value */
static const struct regmap_config sgm37603a_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

static void sgm37603a_check_fault(int last_level, int level)
{
	unsigned int val = 0;
	int ret;
	int i;

	LCD_KIT_INFO("check REG_FAULT_FLAG!\n");
	ret = regmap_read(sgm37603a_g_chip->regmap, SGM37603A_REG_FAULT_FLAG,
		&val);
	if (ret) {
		LCD_KIT_ERR("sgm37603a read REG_FAULT_FLAG fail !\n");
		return;
	}
	if (val)
		LCD_KIT_ERR("check FAULT_FLAG error: val=0x%x\n", val);
#if defined(CONFIG_HUAWEI_DSM)
	for (i = 0; i < SGM37603A_FLAG_CHECK_NUM; i++) {
		if (!(err_table[i].flag & val))
			continue;
		LCD_KIT_ERR("last_bkl:%d, cur_bkl:%d, FAULT_FLAG:0x%x!\n",
			last_level, level, val);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (!ret) {
			dsm_client_record(lcd_dclient,
				"sgm37603a last_bkl:%d, cur_bkl:%d\n"
				"FAULT_FLAG:0x%x!\n",
				last_level, level, err_table[i].flag);
			dsm_client_notify(lcd_dclient, err_table[i].err_no);
		} else {
			dev_err(&sgm37603a_g_chip->client->dev,
				"dsm_client_ocuppy fail: ret=%d!\n", ret);
		}
	}
#endif
}

static void sgm37603a_enable(void)
{
	int ret;

	/* chip initialize */
	ret = sgm37603a_chip_init(sgm37603a_g_chip);
	if (ret < 0) {
		LCD_KIT_ERR("sgm37603a_chip_init fail!\n");
		return ;
	}
	sgm37603a_init_status = true;
}

static void sgm37603a_disable(void)
{
	int ret;
	ret = regmap_write(sgm37603a_g_chip->regmap,
		SGM37603A_REG_LEDS_CONFIG, SGM37603A_REG_LEDS_CONFIG_DISABLE);
	if (ret < 0)
		LCD_KIT_ERR("write sgm37603a 0x%x fail\n", SGM37603A_REG_LEDS_CONFIG);

	sgm37603a_init_status = false;
}

int sgm37603a_set_backlight(unsigned int bl_level)
{
	static int last_bl_level;
	int bl_msb;
	int bl_lsb;
	int ret;
	unsigned int bl_max_level = lcm_get_panel_backlight_max_level();

	if (!sgm37603a_g_chip) {
		LCD_KIT_ERR("sgm37603a_g_chip is null\n");
		return SGM_FAIL;
	}
	if (down_trylock(&(sgm37603a_g_chip->test_sem))) {
		LCD_KIT_INFO("Now in test mode\n");
		return SGM_SUCC;
	}
	/* first set backlight, enable sgm37603a */
	if ((sgm37603a_init_status == false) && (bl_level > 0)) {
		sgm37603a_enable();
		sgm37603a_check_fault(last_bl_level, bl_level);
	}

	if (!bl_max_level)
		bl_max_level = SGM37603A_BL_DEFAULT_LEVEL;

	if (bl_level > bl_max_level)
		bl_level = bl_max_level;

	bl_level = bl_level * sgm37603a_bl_info.bl_level / bl_max_level;
	/* set backlight level */
	bl_msb = (bl_level >> SGM37603A_MSB_LEN) & 0xFF;
	bl_lsb = bl_level & 0x0F;
	ret = regmap_write(sgm37603a_g_chip->regmap, SGM37603A_REG_RATIO_LSB,
		bl_lsb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write sgm37603a backlight level lsb:0x%x fail\n",
			bl_lsb);
	}

	ret = regmap_write(sgm37603a_g_chip->regmap, SGM37603A_REG_RATIO_MSB,
		bl_msb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write sgm37603a backlight level msb:0x%x fail\n",
			bl_msb);
	}

	LCD_KIT_INFO("write sgm37603a backlight %u success\n", bl_level);

	/* if set backlight level 0, disable sgm37603a */
	if ((sgm37603a_init_status == true) && (bl_level == 0)) {
		sgm37603a_check_fault(last_bl_level, bl_level);
		sgm37603a_disable();
	}

	up(&(sgm37603a_g_chip->test_sem));

	last_bl_level = bl_level;
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = sgm37603a_set_backlight,
};

static int sgm37603a_probe(
	struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct sgm37603a_chip_data *pchip = NULL;
	int ret = SGM_FAIL;
	struct device_node *np = NULL;
#if defined(CONFIG_HUAWEI_DSM)
	unsigned int val = 0;
#endif

	LCD_KIT_INFO("in %s!\n", __func__);
	if (!client) {
		LCD_KIT_ERR("client is null pointer\n");
		return ret;
	}
	adapter = client->adapter;
	if (!adapter) {
		LCD_KIT_ERR("adapter is null pointer\n");
		return ret;
	}

	np = client->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("np is null pointer\n");
		return ret;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		LCD_KIT_ERR("i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev, sizeof(struct sgm37603a_chip_data),
		GFP_KERNEL);
	if (!pchip) {
		LCD_KIT_ERR("devm_kzalloc fail\n");
		return -ENOMEM;
	}

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &sgm37603a_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		LCD_KIT_ERR("allocate register map fail: %d\n", ret);
		goto err_out;
	}
#endif

	sgm37603a_g_chip = pchip;
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(sgm37603a_class, NULL, 0, "%s",
		client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		LCD_KIT_ERR("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &sgm37603a_group);
		if (ret)
			goto err_sysfs;
	}

	ret = sgm37603a_parse_dts(np);
	if (ret < 0) {
		LCD_KIT_ERR("parse sgm37603a dts fail");
		goto err_sysfs;
	}

	if (sgm37603a_bl_info.sgm37603a_hw_en) {
		ret = gpio_request(sgm37603a_bl_info.sgm37603a_hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("sgm37603a Could not request hw_en_gpio\n");
			goto err_sysfs;
		}
	}
	lcd_kit_bl_register(&bl_ops);
#if defined(CONFIG_HUAWEI_DSM)
	ret = regmap_read(sgm37603a_g_chip->regmap, SGM37603A_REG_FAULT_FLAG,
		&val);
	if (ret) {
		LCD_KIT_ERR("sgm37603a read REG_FAULT_FLAG fail !\n");
		goto err_sysfs;
	}

	if (val != SGM37603A_DEVICE_FAULT_OCCUR) {
		ret = dsm_client_ocuppy(lcd_dclient);
		if (!ret) {
			dev_err(&sgm37603a_g_chip->client->dev,
				"fail : REG_FAULT_FLAG statues 0X1F=%u!\n",
				val);
			dsm_client_record(lcd_dclient,
				"REG_FAULT_FLAG statues error 0X1F=%u!\n", val);
			dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
		} else {
			dev_err(&sgm37603a_g_chip->client->dev,
				"dsm_client_ocuppy fail: ret=%d!\n", ret);
		}
	}
#endif
	return ret;

err_sysfs:
	LCD_KIT_ERR("sysfs error!\n");
	device_destroy(sgm37603a_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int sgm37603a_remove(struct i2c_client *client)
{
	if (!client) {
		LCD_KIT_ERR("client is null pointer\n");
		return SGM_FAIL;
	}

	sysfs_remove_group(&client->dev.kobj, &sgm37603a_group);
	devm_kfree(&client->dev, sgm37603a_g_chip);

	return SGM_SUCC;
}


static const struct i2c_device_id sgm37603a_id[] = {
	{ SGM37603A_NAME, 0 },
	{},
};

static const struct of_device_id sgm37603a_of_id_table[] = {
	{ .compatible = "sgm,sgm37603a" },
	{},
};

MODULE_DEVICE_TABLE(i2c, sgm37603a_id);
static struct i2c_driver sgm37603a_i2c_driver = {
	.driver = {
		.name = "sgm37603a",
		.owner = THIS_MODULE,
		.of_match_table = sgm37603a_of_id_table,
	},
	.probe = sgm37603a_probe,
	.remove = sgm37603a_remove,
	.id_table = sgm37603a_id,
};

static int __init sgm37603a_init(void)
{
	int ret;

	LCD_KIT_INFO("%s init\n", __func__);
	sgm37603a_class = class_create(THIS_MODULE, "sgm37603a");
	if (IS_ERR(sgm37603a_class)) {
		LCD_KIT_ERR("Unable to create sgm37603a class; errno = %ld\n",
			PTR_ERR(sgm37603a_class));
		sgm37603a_class = NULL;
	}

	ret = i2c_add_driver(&sgm37603a_i2c_driver);
	if (ret)
		LCD_KIT_ERR("Unable to register sgm37603a driver\n");

	LCD_KIT_INFO("%s ok\n", __func__);
	return ret;
}

static void __exit sgm37603a_exit(void)
{
	i2c_del_driver(&sgm37603a_i2c_driver);
}

module_init(sgm37603a_init);
module_exit(sgm37603a_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");
