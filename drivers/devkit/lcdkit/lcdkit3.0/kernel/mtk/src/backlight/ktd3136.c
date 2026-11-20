/*
 * ktd3136.c
 *
 * ktd3136 backlight driver
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

#include "ktd3136.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#include "backlight_linear_to_exp.h"
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#else
#include "lcm_drv.h"
#endif
#include "lcd_kit_disp.h"

static char *ktd3136_dts_string[KTD3136_RW_REG_MAX] = {
	"ktd3136_reg_ctrl_a_bl_cfg",
	"ktd3136_reg_ctrl_a_lsb",
	"ktd3136_reg_ctrl_a_msb",
	"ktd3136_reg_pwm_cfg",
	"ktd3136_reg_boost_ctrl",
	"ktd3136_reg_ctrl_a_current_set"
};

static unsigned int ktd3136_reg_addr[KTD3136_RW_REG_MAX] = {
	KTD3136_REG_CTRL_BL_CONFIG,
	KTD3136_REG_CTRL_BRIGHTNESS_LSB,
	KTD3136_REG_CTRL_BRIGHTNESS_MSB,
	KTD3136_REG_CTRL_PWM_CONFIG,
	KTD3136_REG_CTRL_BOOT_CTRL,
	KTD3136_REG_CTRL_CURRENT_SET
};

static struct ktd3136_backlight_information ktd3136_bl_info;
struct class *ktd3136_class = NULL;
struct ktd3136_chip_data *ktd3136_g_chip = NULL;
static bool ktd3136_init_status = true;
static int ktd3136_10000_bl;
static int ktd3136_fault_check_support;

static int ktd3136_parse_dts(struct device_node *np)
{
	int ret;
	int i;
	struct mtk_panel_info *panel_info = NULL;

	if (np == NULL) {
		LCD_KIT_ERR("np is null pointer\n");
		return KTD_FAIL;
	}

	for (i = 0; i < KTD3136_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, ktd3136_dts_string[i],
			&ktd3136_bl_info.ktd3136_reg[i]);
		if (ret < 0) {
			/* init to invalid data */
			ktd3136_bl_info.ktd3136_reg[i] =
				KTD3136_INVALID_VAL;
			LCD_KIT_INFO("can not find config:%s\n",
				ktd3136_dts_string[i]);
		}
	}

	ret = of_property_read_u32(np, KTD3136_HW_ENABLE,
		&ktd3136_bl_info.ktd3136_hw_en);
	if (ret < 0) {
		LCD_KIT_ERR("ktd3136_hw_enable use default value\n");
		ktd3136_bl_info.ktd3136_hw_en = 0;
	}
	if (ktd3136_bl_info.ktd3136_hw_en != 0) {
		ret = of_property_read_u32(np, KTD3136_HW_EN_GPIO,
			&ktd3136_bl_info.ktd3136_hw_en_gpio);
		if (ret < 0) {
			LCD_KIT_ERR("get ktd3136_hw_en_gpio fail\n");
		} else {
			/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
			panel_info = lcm_get_panel_info();
#else
			panel_info = lcdkit_mtk_common_panel.panel_info;
#endif
			if (panel_info == NULL) {
				LCD_KIT_ERR("panel_info is NULL\n");
				return KTD_FAIL;
			}
			ktd3136_bl_info.ktd3136_hw_en_gpio +=
				panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, KTD3136_HW_EN_PULL_LOW,
			&ktd3136_bl_info.ktd3136_hw_en_pull_low);
		if (ret < 0) {
			LCD_KIT_ERR("get ktd3136_hw_en_pull_low fail\n");
			ktd3136_bl_info.ktd3136_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, KTD3136_HW_EN_DELAY,
		&ktd3136_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		ktd3136_bl_info.bl_on_kernel_mdelay = 0;
		LCD_KIT_ERR("ktd3136 bl_on_kernel_mdelay use default value\n");
	}

	ret = of_property_read_u32(np, KTD3136_FAULT_CHECK_SUPPORT,
		&ktd3136_fault_check_support);
	if (ret < 0) {
		ktd3136_fault_check_support = 0;
		LCD_KIT_INFO("ktd3136 fault_check_support use default value\n");
	}

	ret = of_property_read_u32(np, KTD3136_10000_BACKLIGHT,
		&ktd3136_10000_bl);
	if (ret < 0) {
		LCD_KIT_INFO("ktd3136 is no support 10000 backlight!\n");
		ktd3136_10000_bl = 0;
	}

	return KTD_SUCC;
}

static int ktd3136_config_write(
	struct ktd3136_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = KTD_FAIL;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LCD_KIT_ERR("pchip or reg or val is null pointer\n");
		return ret;
	}

	for (i = 0; i < size; i++) {
		/* judge reg is valid */
		if (val[i] != KTD3136_INVALID_VAL) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			LCD_KIT_INFO("ktd3136 write 0x%x = 0x%x",
				reg[i], val[i]);
			if (ret < 0) {
				LCD_KIT_ERR("write ktd3136 0x%x fail\n",
					reg[i]);
				return ret;
			}
		}
	}

	return ret;
}

static int ktd3136_config_read(
	struct ktd3136_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = 0;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LCD_KIT_ERR("pchip or reg or val is null pointer\n");
		return KTD_FAIL;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			LCD_KIT_ERR("read ktd3136 0x%x fail", reg[i]);
			return ret;
		} else {
			LCD_KIT_INFO("read 0x%x value 0x%x\n", reg[i], val[i]);
		}
	}

	return ret;
}

static ssize_t ktd3136_reg_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct ktd3136_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	unsigned int ktd3136_reg_info[KTD3136_RW_REG_MAX] = {0};
	ssize_t ret = KTD_FAIL;

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

	ret = ktd3136_config_read(pchip, ktd3136_reg_addr,
		ktd3136_reg_info, KTD3136_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("ktd3136 config read fail");
		ret = snprintf(buf, PAGE_SIZE,
			"%s: i2c access fail to register\n", __func__);
		return ret;
	}
	/* 0-5:index for init registers */
	ret = snprintf(buf, PAGE_SIZE, "0x03 = 0x%x,0x04 = 0x%x,0x05 = 0x%x\n \
		\r0x06 = 0x%x,0x08 = 0x%x,0x02 = 0x%x\n",
		ktd3136_reg_info[0], ktd3136_reg_info[1], ktd3136_reg_info[2],
		ktd3136_reg_info[3], ktd3136_reg_info[4], ktd3136_reg_info[5]);
	return ret;
}

static ssize_t ktd3136_reg_store(
	struct device *dev,
	struct device_attribute *dev_attr,
	const char *buf, size_t size)
{
	ssize_t ret = KTD_FAIL;
	struct ktd3136_chip_data *pchip = NULL;
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

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), ktd3136_reg_show,
	ktd3136_reg_store);

/* pointers to created device attributes */
static struct attribute *ktd3136_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group ktd3136_group = {
	.attrs = ktd3136_attributes,
};

/* set default reg value */
static const struct regmap_config ktd3136_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

static int ktd3136_chip_enable(struct ktd3136_chip_data *pchip)
{
	int ret = KTD_FAIL;

	LCD_KIT_INFO("chip enable in!\n");

	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is NULL pointer\n");
		return ret;
	}

	ret = ktd3136_config_write(pchip, ktd3136_reg_addr,
		ktd3136_bl_info.ktd3136_reg, KTD3136_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("ktd3136 i2c config register failed");
		return ret;
	}

	LCD_KIT_INFO("chip enable ok!\n");
	return ret;
}

static void ktd3136_enable(void)
{
	int ret;

	if (ktd3136_bl_info.ktd3136_hw_en != 0 &&
		ktd3136_bl_info.ktd3136_hw_en_pull_low != 0)  {
		ret = gpio_request(ktd3136_bl_info.ktd3136_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("ktd3136 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(ktd3136_bl_info.ktd3136_hw_en_gpio,
			GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("ktd3136 set gpio output not success\n");
		gpio_set_value(ktd3136_bl_info.ktd3136_hw_en_gpio,
			GPIO_OUT_ONE);
		if (ktd3136_bl_info.bl_on_kernel_mdelay)
			mdelay(ktd3136_bl_info.bl_on_kernel_mdelay);
	}
	/* chip initialize */
	ret = ktd3136_chip_enable(ktd3136_g_chip);
	if (ret < 0) {
		LCD_KIT_ERR("ktd3136_chip_init fail!\n");
		return;
	}

	ktd3136_init_status = true;
}

static void ktd3136_disable(void)
{
	int ret;

	ret = regmap_write(ktd3136_g_chip->regmap, KTD3136_REG_CTRL_CURRENT_SET,
		KTD3136_BL_DISABLE);
	if (ret < 0)
		LCD_KIT_ERR("ktd3136 write bl disable %d fail\n", ret);
	if (ktd3136_bl_info.ktd3136_hw_en != 0 &&
		ktd3136_bl_info.ktd3136_hw_en_pull_low != 0) {
		gpio_set_value(ktd3136_bl_info.ktd3136_hw_en_gpio,
			GPIO_OUT_ZERO);
		gpio_free(ktd3136_bl_info.ktd3136_hw_en_gpio);
	}

	ktd3136_init_status = false;
}

static void ktd3136_check_fault(struct ktd3136_chip_data *pchip,
	int last_level, int level)
{
	unsigned int val = 0;
	int ret;

	LCD_KIT_INFO("check REG_FAULT_FLAG!\n");
	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is NULL\n");
		return;
	}
	ret = regmap_read(pchip->regmap, KTD3136_FLAGS_SETTING,
		&val);
	if (ret) {
		LCD_KIT_ERR("ktd3136 read REG_STATUS_FLAG1 fail !\n");
		return;
	}
	if (val)
		LCD_KIT_ERR("check FAULT_FLAG error: val=0x%x\n", val);
#if defined(CONFIG_HUAWEI_DSM)
	if(val & 1 << KTD3136_OCP_BIT) {
		LCD_KIT_ERR("last_bkl:%d, cur_bkl:%d, FAULT_FLAG1:0x%x!\n",
			last_level, level, val);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (!ret) {
			dsm_client_record(lcd_dclient,
				"ktd3136 last_bkl:%d, cur_bkl:%d\n"
				"REG_STATUS_FLAG1:0x%x!\n",
				last_level, level, val);
			dsm_client_notify(lcd_dclient,
				DSM_LCD_BACKLIGHT_OCP_ERROR_NO);
		} else {
			dev_err(&pchip->client->dev,
				"dsm_client_ocuppy flag1 fail: ret=%d!\n", ret);
		}
	}

	if(val & 1 << KTD3136_OVP_BIT) {
		LCD_KIT_ERR("last_bkl:%d, cur_bkl:%d, FAULT_FLAG2:0x%x!\n",
			last_level, level, val);
		ret = dsm_client_ocuppy(lcd_dclient);
		if (!ret) {
			dsm_client_record(lcd_dclient,
				"ktd3136 last_bkl:%d, cur_bkl:%d\n"
				"REG_STATUS_FLAG2:0x%x!\n",
				last_level, level, val);
			dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
		} else {
			dev_err(&pchip->client->dev,
				"dsm_client_ocuppy flag2 fail: ret=%d!\n", ret);
		}
	}
#endif
}

static void ktd3136_backlight_ic_check_fault(struct ktd3136_chip_data *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is NULL\n");
		return;
	}
	/* Judge power on or power off */
	if (ktd3136_fault_check_support &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0)))
		ktd3136_check_fault(pchip, last_level, level);
	last_level = level;
}

int ktd3136_set_backlight(unsigned int bl_level)
{
	int bl_msb;
	int bl_lsb;
	int ret;
	unsigned int level;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *panel_info = NULL;
#endif

	if (ktd3136_g_chip == NULL) {
		LCD_KIT_ERR("ktd3136_g_chip is null\n");
		return KTD_FAIL;
	}

	if (down_trylock(&(ktd3136_g_chip->test_sem))) {
		LCD_KIT_INFO("Now in test mode\n");
		return KTD_SUCC;
	}

	/* first set backlight, enable ktd3136 */
	if ((ktd3136_init_status == false) && (bl_level > 0))
		ktd3136_enable();

	if (ktd3136_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > KTD3136_BL_MAX)
			level = KTD3136_BL_MAX;
	}
#ifdef CONFIG_DRM_MEDIATEK
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		LCD_KIT_ERR("panel_info is NULL\n");
		up(&(ktd3136_g_chip->test_sem));
		return KTD_FAIL;
	}
	if (panel_info->bl_ic_ctrl_mode == MTK_MIPI_BL_IC_PWM_MODE) {
		LCD_KIT_INFO("panel backlight is mipi pwm mode\n");
		ktd3136_backlight_ic_check_fault(ktd3136_g_chip, level);
		up(&(ktd3136_g_chip->test_sem));
		return KTD_SUCC;
	}
#endif

	/* set backlight level */
	bl_msb = (level >> KTD3136_LSB_LEN) & KTD3136_BL_MSB_MASK;
	bl_lsb = level & KTD3136_BL_LSB_MASK;
	ret = regmap_write(ktd3136_g_chip->regmap,
		KTD3136_REG_CTRL_BRIGHTNESS_LSB, bl_lsb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write ktd3136 backlight level lsb:0x%x fail\n",
			bl_lsb);
	}

	ret = regmap_write(ktd3136_g_chip->regmap,
		KTD3136_REG_CTRL_BRIGHTNESS_MSB, bl_msb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write ktd3136 backlight level msb:0x%x fail\n",
			bl_msb);
	}

	ktd3136_backlight_ic_check_fault(ktd3136_g_chip, level);
	/* if set backlight level 0, disable ktd3136 */
	if ((ktd3136_init_status == true) && (level == 0))
		ktd3136_disable();

	up(&(ktd3136_g_chip->test_sem));
	LCD_KIT_INFO("write ktd3136 backlight %u success\n", level);
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = ktd3136_set_backlight,
	.name = KTD3136_NAME,
};

static int ktd3136_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct ktd3136_chip_data *pchip = NULL;
	struct device_node *np = NULL;
	int ret = KTD_FAIL;

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

	pchip = devm_kzalloc(&client->dev, sizeof(struct ktd3136_chip_data),
		GFP_KERNEL);
	if (pchip == NULL) {
		LCD_KIT_ERR("devm_kzalloc fail\n");
		return -ENOMEM;
	}

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &ktd3136_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		LCD_KIT_ERR("allocate register map fail: %d\n", ret);
		goto err_init;
	}
#endif
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(ktd3136_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		LCD_KIT_ERR("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &ktd3136_group);
		if (ret)
			goto err_sysfs;
	}
	ktd3136_g_chip = pchip;
	ret = ktd3136_parse_dts(np);
	if (ret < 0) {
		LCD_KIT_ERR("parse ktd3136 dts fail");
		goto err_sysfs;
	}
	if (ktd3136_bl_info.ktd3136_hw_en) {
		ret = gpio_request(ktd3136_bl_info.ktd3136_hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("ktd3136 Could not request hw_en_gpio\n");
			goto err_sysfs;
		}
	}
	lcd_kit_bl_register(&bl_ops);
	return KTD_SUCC;

err_sysfs:
	LCD_KIT_ERR("sysfs error!\n");
	device_destroy(ktd3136_class, 0);
	ktd3136_class = NULL;
err_init:
	if (pchip != NULL) {
		devm_kfree(&client->dev, pchip);
		ktd3136_g_chip = NULL;
	}
	return ret;
}

static int ktd3136_remove(struct i2c_client *client)
{
	if (client == NULL) {
		LCD_KIT_ERR("client is null pointer\n");
		return KTD_FAIL;
	}
	if (ktd3136_class != NULL)
		device_destroy(ktd3136_class, 0);

	sysfs_remove_group(&client->dev.kobj, &ktd3136_group);
	if (ktd3136_g_chip != NULL)
		devm_kfree(&client->dev, ktd3136_g_chip);

	return KTD_SUCC;
}


static const struct i2c_device_id ktd3136_id[] = {
	{ KTD3136_NAME, 0 },
	{},
};

static const struct of_device_id ktd3136_of_id_table[] = {
	{ .compatible = DTS_COMP_KTD3136 },
	{},
};

MODULE_DEVICE_TABLE(i2c, ktd3136_id);
static struct i2c_driver ktd3136_i2c_driver = {
	.driver = {
		.name = KTD3136_NAME,
		.owner = THIS_MODULE,
		.of_match_table = ktd3136_of_id_table,
	},
	.probe = ktd3136_probe,
	.remove = ktd3136_remove,
	.id_table = ktd3136_id,
};

static int __init ktd3136_init(void)
{
	int ret;

	LCD_KIT_INFO("%s init\n", __func__);
	ktd3136_class = class_create(THIS_MODULE, KTD3136_NAME);
	if (IS_ERR(ktd3136_class)) {
		ret = PTR_ERR(ktd3136_class);
		LCD_KIT_ERR("Unable to create ktd3136 class; errno = %ld\n",
			ret);
		ktd3136_class = NULL;
		return ret;
	}

	ret = i2c_add_driver(&ktd3136_i2c_driver);
	if (ret)
		LCD_KIT_ERR("Unable to register ktd3136 driver\n");

	LCD_KIT_INFO("%s ok\n", __func__);
	return ret;
}

static void __exit ktd3136_exit(void)
{
	i2c_del_driver(&ktd3136_i2c_driver);
}

late_initcall(ktd3136_init);
module_exit(ktd3136_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");
