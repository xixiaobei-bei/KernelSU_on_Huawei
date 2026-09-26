/*
 * sm5350.c
 *
 * sm5350 backlight driver
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

#include "sm5350.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#include "backlight_linear_to_exp.h"
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#else
#include "lcm_drv.h"
#endif
#include "lcd_kit_disp.h"

static char *sm5350_dts_string[SM5350_RW_REG_MAX] = {
	"sm5350_reg_hvled_current_output_cfg",
	"sm5350_reg_ctrl_a_bl_cfg",
	"sm5350_reg_ctrl_a_current_set",
	"sm5350_reg_boost_ctrl",
	"sm5350_reg_pwm_cfg",
	"sm5350_reg_led_fault_enable",
	"sm5350_reg_ctrl_a_lsb",
	"sm5350_reg_ctrl_a_msb",
	"sm5350_reg_ctrl_back_enable"
};

static unsigned int sm5350_reg_addr[SM5350_RW_REG_MAX] = {
	SM5350_REG_HVLED_CURRENT_SINK_OUT_CONFIG,
	SM5350_REG_BL_CONFIG,
	SM5350_REG_BL_CTRL_A_FULL_SCALE_CURRENT_SETTING,
	SM5350_REG_BOOST_CTRL,
	SM5350_REG_PWM_CONFIG,
	SM5350_REG_LED_FAULT_ENABLE,
	SM5350_REG_CTRL_A_BRIGHTNESS_LSB,
	SM5350_REG_CTRL_A_BRIGHTNESS_MSB,
	SM5350_REG_CTRL_BANK_ENABLE
};

static struct sm5350_backlight_information sm5350_bl_info;
struct class *sm5350_class = NULL;
struct sm5350_chip_data *sm5350_g_chip = NULL;
static bool sm5350_init_status = true;
static int sm5350_10000_bl;

static int sm5350_parse_dts(struct device_node *np)
{
	int ret;
	int i;
	struct mtk_panel_info *panel_info = NULL;

	if (np == NULL) {
		LCD_KIT_ERR("np is null pointer\n");
		return AW_FAIL;
	}

	for (i = 0; i < SM5350_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, sm5350_dts_string[i],
			&sm5350_bl_info.sm5350_reg[i]);
		if (ret < 0) {
			/* init to invalid data */
			sm5350_bl_info.sm5350_reg[i] =
				SM5350_INVALID_VAL;
			LCD_KIT_INFO("can not find config:%s\n",
				sm5350_dts_string[i]);
		}
	}

	ret = of_property_read_u32(np, SM5350_HW_ENABLE,
		&sm5350_bl_info.sm5350_hw_en);
	if (ret < 0) {
		LCD_KIT_ERR("sm5350_hw_enable use default value\n");
		sm5350_bl_info.sm5350_hw_en = 0;
	}
	if (sm5350_bl_info.sm5350_hw_en != 0) {
		ret = of_property_read_u32(np, SM5350_HW_EN_GPIO,
			&sm5350_bl_info.sm5350_hw_en_gpio);
		if (ret < 0) {
			LCD_KIT_ERR("get sm5350_hw_en_gpio fail\n");
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
			sm5350_bl_info.sm5350_hw_en_gpio +=
				panel_info->gpio_offset;
		}
		ret = of_property_read_u32(np, SM5350_HW_EN_PULL_LOW,
			&sm5350_bl_info.sm5350_hw_en_pull_low);
		if (ret < 0) {
			LCD_KIT_ERR("get sm5350_hw_en_pull_low fail\n");
			sm5350_bl_info.sm5350_hw_en_pull_low = 0;
		}
	}

	ret = of_property_read_u32(np, SM5350_HW_EN_DELAY,
		&sm5350_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		sm5350_bl_info.bl_on_kernel_mdelay = 0;
		LCD_KIT_ERR("sm5350 bl_on_kernel_mdelay use default value\n");
	}

	ret = of_property_read_u32(np, SM5350_10000_BACKLIGHT,
		&sm5350_10000_bl);
	if (ret < 0) {
		LCD_KIT_INFO("sm5350 is no support 10000 backlight!\n");
		sm5350_10000_bl = 0;
	}

	return AW_SUCC;
}

static int sm5350_config_write(
	struct sm5350_chip_data *pchip,
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
		if (val[i] != SM5350_INVALID_VAL) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			LCD_KIT_INFO("sm5350 write 0x%x = 0x%x",
				reg[i], val[i]);
			if (ret < 0) {
				LCD_KIT_ERR("write sm5350 0x%x fail\n",
					reg[i]);
				return ret;
			}
		}
	}

	return ret;
}

static int sm5350_config_read(
	struct sm5350_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
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
			LCD_KIT_ERR("read sm5350 0x%x fail", reg[i]);
			return ret;
		} else {
			LCD_KIT_INFO("read 0x%x value 0x%x\n", reg[i], val[i]);
		}
	}

	return ret;
}

static ssize_t sm5350_reg_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct sm5350_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	unsigned int sm5350_reg_info[SM5350_RW_REG_MAX] = {0};
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

	ret = sm5350_config_read(pchip, sm5350_reg_addr,
		sm5350_reg_info, SM5350_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("sm5350 config read fail");
		ret = snprintf(buf, PAGE_SIZE,
			"%s: i2c access fail to register\n", __func__);
		return ret;
	}

	ret = snprintf(buf, PAGE_SIZE, "0x10 = 0x%x,0x16 = 0x%x,0x17 = 0x%x\n \
		\r0x1A = 0x%x,0x1C = 0x%x,0xB4 = 0x%x\n \
		\r0x20 = 0x%x,0x21 = 0x%x,0x24 = 0x%x\n",
		sm5350_reg_info[0], sm5350_reg_info[1], sm5350_reg_info[2],
		sm5350_reg_info[3], sm5350_reg_info[4], sm5350_reg_info[5],
		sm5350_reg_info[6], sm5350_reg_info[7], sm5350_reg_info[8]);
	return ret;
}

static ssize_t sm5350_reg_store(
	struct device *dev,
	struct device_attribute *dev_attr,
	const char *buf, size_t size)
{
	ssize_t ret = AW_FAIL;
	struct sm5350_chip_data *pchip = NULL;
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

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), sm5350_reg_show,
	sm5350_reg_store);

/* pointers to created device attributes */
static struct attribute *sm5350_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group sm5350_group = {
	.attrs = sm5350_attributes,
};

/* set default reg value */
static const struct regmap_config sm5350_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

static int sm5350_chip_enable(struct sm5350_chip_data *pchip)
{
	int ret = AW_FAIL;

	LCD_KIT_INFO("chip enable in!\n");

	if (pchip == NULL) {
		LCD_KIT_ERR("pchip is NULL pointer\n");
		return ret;
	}

	ret = sm5350_config_write(pchip, sm5350_reg_addr,
		sm5350_bl_info.sm5350_reg, SM5350_RW_REG_MAX);
	if (ret < 0) {
		LCD_KIT_ERR("sm5350 i2c config register failed");
		return ret;
	}

	LCD_KIT_INFO("chip enable ok!\n");
	return ret;
}

static void sm5350_enable(void)
{
	int ret;

	if (sm5350_bl_info.sm5350_hw_en != 0 &&
		sm5350_bl_info.sm5350_hw_en_pull_low != 0)  {
		ret = gpio_request(sm5350_bl_info.sm5350_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("sm5350 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(sm5350_bl_info.sm5350_hw_en_gpio,
			GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("sm5350 set gpio output not success\n");
		gpio_set_value(sm5350_bl_info.sm5350_hw_en_gpio,
			GPIO_OUT_ONE);
		if (sm5350_bl_info.bl_on_kernel_mdelay)
			mdelay(sm5350_bl_info.bl_on_kernel_mdelay);
	}
	/* chip initialize */
	ret = sm5350_chip_enable(sm5350_g_chip);
	if (ret < 0) {
		LCD_KIT_ERR("sm5350_chip_init fail!\n");
		return;
	}

	sm5350_init_status = true;
}

static void sm5350_disable(void)
{
	if (sm5350_bl_info.sm5350_hw_en != 0 &&
		sm5350_bl_info.sm5350_hw_en_pull_low != 0) {
		gpio_set_value(sm5350_bl_info.sm5350_hw_en_gpio,
			GPIO_OUT_ZERO);
		gpio_free(sm5350_bl_info.sm5350_hw_en_gpio);
	}

	sm5350_init_status = false;
}

int sm5350_set_backlight(unsigned int bl_level)
{
	int bl_msb;
	int bl_lsb;
	int ret;
	unsigned int level;

	if (sm5350_g_chip == NULL) {
		LCD_KIT_ERR("sm5350_g_chip is null\n");
		return AW_FAIL;
	}

	if (down_trylock(&(sm5350_g_chip->test_sem))) {
		LCD_KIT_INFO("Now in test mode\n");
		return AW_SUCC;
	}

	/* first set backlight, enable sm5350 */
	if ((sm5350_init_status == false) && (bl_level > 0))
		sm5350_enable();

	if (sm5350_10000_bl) {
		level = bl_lvl_map(bl_level);
	} else {
		level = bl_level;
		if (level > SM5350_BL_MAX)
			level = SM5350_BL_MAX;
	}

	/* set backlight level */
	bl_msb = (level >> SM5350_LSB_LEN) & SM5350_BL_MSB_MASK;
	bl_lsb = level & SM5350_BL_LSB_MASK;
	ret = regmap_write(sm5350_g_chip->regmap,
		SM5350_REG_CTRL_A_BRIGHTNESS_LSB, bl_lsb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write sm5350 backlight level lsb:0x%x fail\n",
			bl_lsb);
	}

	ret = regmap_write(sm5350_g_chip->regmap,
		SM5350_REG_CTRL_A_BRIGHTNESS_MSB, bl_msb);
	if (ret < 0) {
		lcd_backlight_i2c_dmd();
		LCD_KIT_ERR("write sm5350 backlight level msb:0x%x fail\n",
			bl_msb);
	}

	/* if set backlight level 0, disable sm5350 */
	if ((sm5350_init_status == true) && (level == 0))
		sm5350_disable();

	up(&(sm5350_g_chip->test_sem));
	LCD_KIT_INFO("write sm5350 backlight %u success\n", level);
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = sm5350_set_backlight,
	.name = SM5350_NAME,
};

static int sm5350_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct sm5350_chip_data *pchip = NULL;
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

	pchip = devm_kzalloc(&client->dev, sizeof(struct sm5350_chip_data),
		GFP_KERNEL);
	if (pchip == NULL) {
		LCD_KIT_ERR("devm_kzalloc fail\n");
		return -ENOMEM;
	}

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &sm5350_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		LCD_KIT_ERR("allocate register map fail: %d\n", ret);
		goto err_init;
	}
#endif
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(sm5350_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		LCD_KIT_ERR("Unable to create device; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &sm5350_group);
		if (ret)
			goto err_sysfs;
	}
	sm5350_g_chip = pchip;
	ret = sm5350_parse_dts(np);
	if (ret < 0) {
		LCD_KIT_ERR("parse sm5350 dts fail");
		goto err_sysfs;
	}
	if (sm5350_bl_info.sm5350_hw_en) {
		ret = gpio_request(sm5350_bl_info.sm5350_hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("sm5350 Could not request hw_en_gpio\n");
			goto err_sysfs;
		}
	}
	lcd_kit_bl_register(&bl_ops);
	return AW_SUCC;

err_sysfs:
	LCD_KIT_ERR("sysfs error!\n");
	device_destroy(sm5350_class, 0);
	sm5350_class = NULL;
err_init:
	if (pchip != NULL) {
		devm_kfree(&client->dev, pchip);
		sm5350_g_chip = NULL;
	}
	return ret;
}

static int sm5350_remove(struct i2c_client *client)
{
	if (client == NULL) {
		LCD_KIT_ERR("client is null pointer\n");
		return AW_FAIL;
	}
	if (sm5350_class != NULL)
		device_destroy(sm5350_class, 0);

	sysfs_remove_group(&client->dev.kobj, &sm5350_group);
	if (sm5350_g_chip != NULL)
		devm_kfree(&client->dev, sm5350_g_chip);

	return AW_SUCC;
}


static const struct i2c_device_id sm5350_id[] = {
	{ SM5350_NAME, 0 },
	{},
};

static const struct of_device_id sm5350_of_id_table[] = {
	{ .compatible = DTS_COMP_SM5350 },
	{},
};

MODULE_DEVICE_TABLE(i2c, sm5350_id);
static struct i2c_driver sm5350_i2c_driver = {
	.driver = {
		.name = SM5350_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sm5350_of_id_table,
	},
	.probe = sm5350_probe,
	.remove = sm5350_remove,
	.id_table = sm5350_id,
};

static int __init sm5350_init(void)
{
	int ret;

	LCD_KIT_INFO("%s init\n", __func__);
	sm5350_class = class_create(THIS_MODULE, SM5350_NAME);
	if (IS_ERR(sm5350_class)) {
		ret = PTR_ERR(sm5350_class);
		LCD_KIT_ERR("Unable to create sm5350 class; errno = %ld\n",
			ret);
		sm5350_class = NULL;
		return ret;
	}

	ret = i2c_add_driver(&sm5350_i2c_driver);
	if (ret)
		LCD_KIT_ERR("Unable to register sm5350 driver\n");

	LCD_KIT_INFO("%s ok\n", __func__);
	return ret;
}

static void __exit sm5350_exit(void)
{
	i2c_del_driver(&sm5350_i2c_driver);
}

late_initcall(sm5350_init);
module_exit(sm5350_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");
