/*
* Simple driver
* Copyright (C)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*/
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/semaphore.h>
#include "lp8556.h"
#include "hisi_fb.h"
#include "../hisi_fb_panel.h"

static struct lp8556_backlight_information lp8556_bl_info;

static struct gpio_desc lp8556_hw_en_on_cmds[] = {
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_US, 0,
		GPIO_LP8556_EN_NAME, &lp8556_bl_info.lp8556_hw_en_gpio, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LP8556_EN_NAME, &lp8556_bl_info.lp8556_hw_en_gpio, 1},
};

static struct gpio_desc lp8556_hw_en_disable_cmds[] =
{
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 0,
		GPIO_LP8556_EN_NAME, &lp8556_bl_info.lp8556_hw_en_gpio, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 10,
		GPIO_LP8556_EN_NAME, &lp8556_bl_info.lp8556_hw_en_gpio, 0},
};

static struct gpio_desc lp8556_hw_en_free_cmds[] =
{
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 50,
		GPIO_LP8556_EN_NAME, &lp8556_bl_info.lp8556_hw_en_gpio, 0},
};

static char *lp8556_dts_string[LP8556_RW_REG_MAX] = {
	"lp8556_device_control",
	"lp8556_eprom_cfg0",
	"lp8556_eprom_cfg1",
	"lp8556_eprom_cfg2",
	"lp8556_eprom_cfg3",
	"lp8556_eprom_cfg4",
	"lp8556_eprom_cfg5",
	"lp8556_eprom_cfg6",
	"lp8556_eprom_cfg7",
	"lp8556_eprom_cfg9",
	"lp8556_eprom_cfgA",
	"lp8556_eprom_cfgE",
	"lp8556_eprom_cfg9E",
	"lp8556_led_enable",
	"lp8556_eprom_cfg98",
};

static unsigned int lp8556_reg_addr[LP8556_RW_REG_MAX] = {
	LP8556_DEVICE_CONTROL,
	LP8556_EPROM_CFG0,
	LP8556_EPROM_CFG1,
	LP8556_EPROM_CFG2,
	LP8556_EPROM_CFG3,
	LP8556_EPROM_CFG4,
	LP8556_EPROM_CFG5,
	LP8556_EPROM_CFG6,
	LP8556_EPROM_CFG7,
	LP8556_EPROM_CFG9,
	LP8556_EPROM_CFGA,
	LP8556_EPROM_CFGE,
	LP8556_EPROM_CFG9E,
	LP8556_LED_ENABLE,
	LP8556_EPROM_CFG98,
};

struct class *lp8556_class = NULL;
struct lp8556_chip_data *lp8556_g_chip = NULL;
static bool lp8556_init_status = false;

/*
** for debug, S_IRUGO
** /sys/module/hisifb/parameters
*/
unsigned lp8556_msg_level = 7;
module_param_named(debug_lp8556_msg_level, lp8556_msg_level, int, 0640);
MODULE_PARM_DESC(debug_lp8556_msg_level, "backlight lp8556 msg level");

static int lp8556_parse_dts(struct device_node *np)
{
	int ret = 0;
	int i = 0;

	for (i = 0;i < LP8556_RW_REG_MAX;i++ ) {
		ret = of_property_read_u32(np, lp8556_dts_string[i], &lp8556_bl_info.lp8556_reg[i]);
		if (ret < 0) {
			lp8556_bl_info.lp8556_reg[i] = 0xffff;
			LP8556_INFO("can not find config:%s\n", lp8556_dts_string[i]);
		}
	}

	ret = of_property_read_u32(np, "lp8556_hw_en_gpio", &lp8556_bl_info.lp8556_hw_en_gpio);
	if (ret < 0) {
		LP8556_ERR("get lp8556_hw_en_gpio dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_on_kernel_mdelay", &lp8556_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		LP8556_ERR("get bl_on_kernel_mdelay dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_led_num", &lp8556_bl_info.bl_led_num);
	if (ret < 0) {
		LP8556_ERR("get bl_led_num dts config failed\n");
		return ret;
	}

	return ret;
}

static int lp8556_config_write(struct lp8556_chip_data *pchip,
			unsigned int reg[],unsigned int val[],unsigned int size)
{
	int ret = 0;
	unsigned int i = 0;

	for(i = 0;i < size;i++) {
		if (val[i] != 0xffff) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			if (ret < 0) {
				LP8556_ERR("write lp8556 backlight config register 0x%x failed\n",reg[i]);
				goto exit;
			}
		}
	}

exit:
	return ret;
}

static int lp8556_config_read(struct lp8556_chip_data *pchip,
			unsigned int reg[],unsigned int val[],unsigned int size)
{
	int ret = 0;
	unsigned int i = 0;

	for(i = 0;i < size;i++) {
		ret = regmap_read(pchip->regmap, reg[i],&val[i]);
		if (ret < 0) {
			LP8556_ERR("read lp8556 backlight config register 0x%x failed",reg[i]);
			goto exit;
		} else {
			LP8556_INFO("read 0x%x value = 0x%x\n", reg[i], val[i]);
		}
	}

exit:
	return ret;
}

/* initialize chip */
static int lp8556_chip_init(struct lp8556_chip_data *pchip)
{
	int ret = -1;

	LP8556_INFO("in!\n");

	ret = lp8556_config_write(pchip, lp8556_reg_addr, lp8556_bl_info.lp8556_reg, LP8556_RW_REG_MAX);
	if (ret < 0) {
		LP8556_ERR("lp8556 config register failed");
		goto out;
	}
	LP8556_INFO("ok!\n");
	return ret;

out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

/**
 * lp8556_set_backlight_init(): initial ic working mode
 *
 * @bl_level: value for backlight ,range from 0 to ~
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t lp8556_set_backlight_init(uint32_t bl_level)
{
	int ret = 0;

	if (g_fake_lcd_flag) {
		LP8556_INFO("is fake lcd!\n");
		return ret;
	}

	if (down_trylock(&(lp8556_g_chip->test_sem))) {
		LP8556_INFO("Now in test mode\n");
		return 0;
	}

	if (false == lp8556_init_status && bl_level > 0) {
		mdelay(lp8556_bl_info.bl_on_kernel_mdelay);
		gpio_cmds_tx(lp8556_hw_en_on_cmds, ARRAY_SIZE(lp8556_hw_en_on_cmds));
		/* chip initialize */
		ret = lp8556_chip_init(lp8556_g_chip);
		if (ret < 0) {
			LP8556_ERR("lp8556_chip_init fail!\n");
			goto out;
		}
		lp8556_init_status = true;
	} else {
		LP8556_DEBUG("lp8556_chip_init %u, 0: already off; else : already init!\n", bl_level);
	}

out:
	up(&(lp8556_g_chip->test_sem));
	return ret;
}
/* EXPORT_SYMBOL(lp8556_set_backlight_init); */

static ssize_t lp8556_reg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lp8556_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret = -1;

	if (!buf) {
		LP8556_ERR("buf is null\n");
		return ret;
	}

	if (!dev) {
		ret =  snprintf(buf, PAGE_SIZE, "dev is null\n");
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

	ret = lp8556_config_read(pchip, lp8556_reg_addr, lp8556_bl_info.lp8556_reg, LP8556_RW_REG_MAX);
	if (ret < 0) {
		LP8556_ERR("lp8556 config read failed");
		goto i2c_error;
	}

	ret = snprintf(buf, PAGE_SIZE, "Device control(0x01)= 0x%x\nEprom Configuration0(0xA0) = 0x%x\n \
			\rEprom Configuration1(0xA1) = 0x%x\nEprom Configuration2(0xA2) = 0x%x\n \
			\rEprom Configuration3(0xA3) = 0x%x\nEprom Configuration4(0xA4) = 0x%x\n \
			\rEprom Configuration5(0xA5) = 0x%x\nEprom Configuration6(0xA6) = 0x%x\n \
			\rEprom Configuration7(0xA7)  = 0x%x\nEprom Configuration9(0xA9)  = 0x%x\n \
			\rEprom ConfigurationA(0xAA) = 0x%x\nEprom ConfigurationE(0xAE) = 0x%x\n \
			\rEprom Configuration9E(0x9E) = 0x%x\nLed enable(0x16) = 0x%x\n",
			lp8556_bl_info.lp8556_reg[0], lp8556_bl_info.lp8556_reg[1], lp8556_bl_info.lp8556_reg[2], lp8556_bl_info.lp8556_reg[3], lp8556_bl_info.lp8556_reg[4], lp8556_bl_info.lp8556_reg[5],lp8556_bl_info.lp8556_reg[6], lp8556_bl_info.lp8556_reg[7],
			lp8556_bl_info.lp8556_reg[8], lp8556_bl_info.lp8556_reg[9], lp8556_bl_info.lp8556_reg[10], lp8556_bl_info.lp8556_reg[11], lp8556_bl_info.lp8556_reg[12], lp8556_bl_info.lp8556_reg[13]);
	return ret;

i2c_error:
	ret = snprintf(buf, PAGE_SIZE,"%s: i2c access fail to register\n", __func__);
	return ret;
}

static ssize_t lp8556_reg_store(struct device *dev,
					struct device_attribute *devAttr,
					const char *buf, size_t size)
{
	ssize_t ret = -1;
	struct lp8556_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!buf) {
		LP8556_ERR("buf is null\n");
		return ret;
	}

	if (!dev) {
		LP8556_ERR("dev is null\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);

	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		LP8556_INFO("check your input!!!\n");
		goto out_input;
	}

	LP8556_INFO("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask, val);

	ret = regmap_update_bits(pchip->regmap, reg, mask, val);
	if (ret < 0)
		goto i2c_error;

	return size;

i2c_error:
	dev_err(pchip->dev, "%s:i2c access fail to register\n", __func__);
	ret = snprintf((char *)buf, PAGE_SIZE, "%s: i2c access fail to register\n", __func__);
	return ret;

out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	ret = snprintf((char *)buf, PAGE_SIZE, "%s: input conversion fail\n", __func__);
	return ret;
}

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), lp8556_reg_show, lp8556_reg_store);

/* pointers to created device attributes */
static struct attribute *lp8556_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group lp8556_group = {
	.attrs = lp8556_attributes,
};

static const struct regmap_config lp8556_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

#include "lcd_kit_common.h"

static void lp8556_enable(void)
{
	int ret = 0;

	mdelay(lp8556_bl_info.bl_on_kernel_mdelay);
	gpio_cmds_tx(lp8556_hw_en_on_cmds, ARRAY_SIZE(lp8556_hw_en_on_cmds));
	/* chip initialize */
	ret = lp8556_chip_init(lp8556_g_chip);
	if (ret < 0) {
		LP8556_ERR("lp8556_chip_init fail!\n");
		return ;
	}
	lp8556_init_status = true;
}

static void lp8556_disable(void)
{
	gpio_cmds_tx(lp8556_hw_en_disable_cmds, ARRAY_SIZE(lp8556_hw_en_disable_cmds));
	gpio_cmds_tx(lp8556_hw_en_free_cmds, ARRAY_SIZE(lp8556_hw_en_free_cmds));
	lp8556_init_status = false;
}

static int lp8556_set_backlight(uint32_t bl_level)
{
	static int last_bl_level = 0;
	int bl_msb = 0;
	int bl_lsb = 0;
	int ret = 0;

	if (!lp8556_g_chip) {
		LP8556_ERR("lp8556_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(lp8556_g_chip->test_sem))) {
		LP8556_INFO("Now in test mode\n");
		return 0;
	}
	/*first set backlight, enable lp8556*/
	if (false == lp8556_init_status && bl_level > 0) {
		lp8556_enable();
	}

	/*set backlight level*/
	bl_msb = (bl_level >> 8) & 0x0F;
	bl_lsb = bl_level & 0xFF;
	ret = regmap_write(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_lsb, bl_lsb);
	if (ret < 0) {
		LP8556_DEBUG("write lp8556 backlight level lsb:0x%x failed\n", bl_lsb);
	}
	ret = regmap_write(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_msb, bl_msb);
	if (ret < 0) {
		LP8556_DEBUG("write lp8556 backlight level msb:0x%x failed\n", bl_msb);
	}

	/*if set backlight level 0, disable lp8556*/
	if (true == lp8556_init_status && 0 == bl_level) {
		lp8556_disable();
	}
	up(&(lp8556_g_chip->test_sem));

	/*when power on backlight, schedule backlight check work*/
	if (last_bl_level == 0 && bl_level > 0) {
		if (common_info->check_thread.check_bl_support) {
			/*delay 500ms schedule work*/
			schedule_delayed_work(&common_info->check_thread.check_work, (HZ / 5));
		}
		LP8556_INFO("level = %d, bl_msb = %d, bl_lsb = %d", bl_level, bl_msb, bl_lsb);
	}
	last_bl_level = bl_level;
	return ret;
}

static int lp8556_backlight_save_restore(int save_enable)
{
	static int bl_msb = 0;
	static int bl_lsb = 0;
	int ret = 0;
	int val = 0;


	if (save_enable) {
		ret = regmap_read(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_lsb, &val);
		if (ret < 0) {
			LP8556_ERR("write lp8556 backlight level msb:0x%x failed\n", bl_msb);
			return -1;
		}
		bl_lsb = val & 0xFF;
		ret = regmap_read(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_msb, &val);
		if (ret < 0) {
			LP8556_ERR("write lp8556 backlight level msb:0x%x failed\n", bl_msb);
			return -1;
		}
		bl_msb = val & 0x0F;
	} else {
		ret = regmap_write(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_lsb, bl_lsb);
		if (ret < 0) {
			LP8556_ERR("write lp8556 backlight level lsb:0x%x failed\n", bl_lsb);
			return -1;
		}
		ret = regmap_write(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_msb, bl_msb);
		if (ret < 0) {
			LP8556_ERR("write lp8556 backlight level msb:0x%x failed\n", bl_msb);
			return -1;
		}
	}
	return ret;
}


static int lp8556_test_led_open(struct lp8556_chip_data *pchip, int led_num)
{
	int ret;
	int i;
	int result = TEST_OK;
	unsigned int val = 0;
	unsigned int enable_leds = LP8556_ENABLE_ALL_LEDS;
	int bl_led_num = lp8556_bl_info.bl_led_num;

	for (i = bl_led_num; i < LP8556_LED_NUM; i++) {
		enable_leds &= ~(1 << i);
	}
	/* Enable all LED strings */
	ret = regmap_write(pchip->regmap, LP8556_LED_ENABLE, enable_leds);
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* Set maximum brightness */
	lp8556_set_backlight(LP8556_BL_MAX);


	/* Open LEDx string. */
	ret = regmap_write(pchip->regmap, LP8556_LED_ENABLE, (~(1<<(unsigned int)led_num)) & enable_leds);
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* Wait 4 msec. */
	msleep(4);

	/* Read LED open fault*/
	ret = regmap_read(pchip->regmap, LP8556_FUALT_FLAG, &val);
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* If then a LED open fault condition has been detected. */
	if (val & (1 << LP8556_FAULT_OPEN_BIT)) {
		result |= (1<<(LP8556_LED1_OPEN_ERR_BIT + led_num));
	}

	/* Connect LEDx string. */
	ret = regmap_write(pchip->regmap, LP8556_LED_ENABLE, enable_leds);
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return result|TEST_ERROR_I2C;
	}

	/* Repeat the procedure for the other LED strings. */
	msleep(1000);
	return result;
}

static int lp8556_test_led_short(struct lp8556_chip_data *pchip, int led_num)
{
	unsigned int val = 0;
	int ret;
	int result = TEST_OK;

	/* Enable only LEDx string. */
	ret = regmap_write(pchip->regmap, LP8556_LED_ENABLE, (1<<(unsigned int)led_num));
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* Set maximum brightness. */
	lp8556_set_backlight(LP8556_BL_MAX);

	/* Wait 4 msec. */
	msleep(4);

	/* Read LED short fault */
	ret = regmap_read(pchip->regmap, LP8556_FUALT_FLAG, &val);
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/*A LED short fault condition has been detected. */
	if (val & (1 << LP8556_FAULT_SHORT_BIT)) {
		result |= (1<<(LP8556_LED1_SHORT_ERR_BIT + led_num));
	}

	/* Set chip enable and LED string enable low. */
	ret = regmap_write(pchip->regmap, LP8556_LED_ENABLE, LP8556_DISABLE_ALL_LEDS);
	if (ret < 0) {
		LP8556_ERR("TEST_ERROR_I2C\n");
		return result|TEST_ERROR_I2C;
	}

	/* Repeat the procedure for the other LED Strings */
	msleep(1000);
	return result;
}

static ssize_t lp8556_self_test(void)
{
	struct lp8556_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret = -1;
	int result = 0;
	int lp8556_regs[LP8556_RW_REG_MAX] = {0};
	int led_num = lp8556_bl_info.bl_led_num;
	int i;

	pchip = lp8556_g_chip;
	client = pchip->client;
	if (!client) {
		LP8556_ERR("client is null\n");
	}

	down(&(pchip->test_sem));
	ret = lp8556_config_read(pchip, lp8556_reg_addr, lp8556_regs, LP8556_RW_REG_MAX);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8556_test_failed;
	}
	ret = lp8556_backlight_save_restore(1);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8556_test_failed;
	}

	for (i = 0; i < led_num; i++) {
		result |= lp8556_test_led_open(pchip, i);
	}

	for (i = 0; i < led_num; i++) {
		result |= lp8556_test_led_short(pchip, i);
	}

	ret = lp8556_chip_init(pchip);
	if (ret < 0) {
		result |= TEST_ERROR_CHIP_INIT;
		goto lp8556_test_failed;
	}

	ret = lp8556_config_write(pchip, lp8556_reg_addr, lp8556_regs, LP8556_RW_REG_MAX);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8556_test_failed;
	}
	ret = lp8556_backlight_save_restore(0);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8556_test_failed;
	}

	up(&(pchip->test_sem));
	LP8556_INFO("self test out:%d\n", result);
	return result;
lp8556_test_failed:
	up(&(pchip->test_sem));
	LP8556_INFO("self test out:%d\n", result);
	return result;
}

static void lp8556_dsm_notify(int val)
{
	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, "lp8556 happen short, reg:0x02 value is:0x%x\n", val);
		dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
	} else {
		LP8556_ERR("dsm_client_ocuppy fail!\n");
	}
}

static int lp8556_check_backlight(void)
{
	#define CHECK_COUNT	3
	#define CHECK_REG	0x02
	#define CHECK_VAL	0x04
	#define CHECK_VAL_FACTORY	0xC4
	int count = 0;
	int val = 0;
	int ret = 0;
	int err_cnt = 0;

	if (!lp8556_g_chip) {
		LP8556_ERR("lp8556_g_chip is null\n");
		return -1;
	}
	/*judge lp8556 is enable, if not return*/
	if (lp8556_init_status == false) {
		LP8556_ERR("lp8556 not enable\n");
		ret = -1;
		goto error;
	}

	while (count < CHECK_COUNT) {
		ret = regmap_read(lp8556_g_chip->regmap, CHECK_REG, &val);
		if (ret < 0) {
			LP8556_INFO("read lp8556 fail!\n");
			goto error;
		}
		if (runmode_is_factory()) {
			if (val & CHECK_VAL_FACTORY) {
				err_cnt++;
				LP8556_ERR("check val:0x%x, backlight maybe short!\n", val);
			}
		} else {
			if (val & CHECK_VAL) {
				err_cnt++;
				LP8556_ERR("check val:0x%x, backlight maybe short!\n", val);
			}
		}
		count++;
		mdelay(3);
	}
	if (err_cnt == CHECK_COUNT) {
		/*backlight short, shutdown backlight*/
		LP8556_ERR("backlight short, shutdown backlight!\n");
		gpio_cmds_tx(lp8556_hw_en_disable_cmds, ARRAY_SIZE(lp8556_hw_en_disable_cmds));
		gpio_cmds_tx(lp8556_hw_en_free_cmds, ARRAY_SIZE(lp8556_hw_en_free_cmds));
		lp8556_dsm_notify(val);
		ret = -1;
		goto error;
	}
error:
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = lp8556_set_backlight,
	.bl_self_test = lp8556_self_test,
	.check_backlight = lp8556_check_backlight,
};

static int lp8556_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = client->adapter;
	struct lp8556_chip_data *pchip = NULL;
	int ret = -1;
	struct device_node *np = NULL;

	LP8556_INFO("in!\n");

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev,
				sizeof(struct lp8556_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

	pchip->regmap = devm_regmap_init_i2c(client, &lp8556_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "fail : allocate register map: %d\n", ret);
		goto err_out;
	}

	lp8556_g_chip = pchip;
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(lp8556_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		LP8556_ERR("Unable to create device; errno = %ld\n", PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &lp8556_group);
		if (ret)
			goto err_sysfs;
	}

	memset(&lp8556_bl_info, 0, sizeof(struct lp8556_backlight_information));

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LP8556);
	if (!np) {
		LP8556_ERR("NOT FOUND device node %s!\n", DTS_COMP_LP8556);
		goto err_sysfs;
	}

	ret = lp8556_parse_dts(np);
	if (ret < 0) {
		LP8556_ERR("parse lp8556 dts failed");
		goto err_sysfs;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LP8556);
	if (!np) {
		LP8556_ERR("NOT FOUND device node %s!\n", DTS_COMP_LP8556);
		goto err_sysfs;
	}
	ret = of_property_read_u32(np, "lp8556_level_lsb", &lp8556_bl_info.lp8556_level_lsb);
	if (ret < 0) {
		LP8556_ERR("get lp8556_level_lsb failed\n");
		goto err_sysfs;
	}

	ret = of_property_read_u32(np, "lp8556_level_msb", &lp8556_bl_info.lp8556_level_msb);
	if (ret < 0) {
		LP8556_ERR("get lp8556_level_msb failed\n");
		goto err_sysfs;
	}
	lcd_kit_bl_register(&bl_ops);
	return ret;

err_sysfs:
	LP8556_DEBUG("sysfs error!\n");
	device_destroy(lp8556_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int lp8556_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &lp8556_group);

	return 0;
}

static const struct i2c_device_id lp8556_id[] = {
	{LP8556_NAME, 0},
	{},
};

static const struct of_device_id lp8556_of_id_table[] = {
	{.compatible = "ti,lp8556"},
	{},
};

MODULE_DEVICE_TABLE(i2c, lp8556_id);
static struct i2c_driver lp8556_i2c_driver = {
		.driver = {
			.name = "lp8556",
			.owner = THIS_MODULE,
			.of_match_table = lp8556_of_id_table,
		},
		.probe = lp8556_probe,
		.remove = lp8556_remove,
		.id_table = lp8556_id,
};

static int __init lp8556_module_init(void)
{
	int ret = -1;

	LP8556_INFO("in!\n");

	lp8556_class = class_create(THIS_MODULE, "lp8556");
	if (IS_ERR(lp8556_class)) {
		LP8556_ERR("Unable to create lp8556 class; errno = %ld\n", PTR_ERR(lp8556_class));
		lp8556_class = NULL;
	}

	ret = i2c_add_driver(&lp8556_i2c_driver);
	if (ret)
		LP8556_ERR("Unable to register lp8556 driver\n");

	LP8556_INFO("ok!\n");

	return ret;
}
late_initcall(lp8556_module_init);

MODULE_DESCRIPTION("Texas Instruments Backlight driver for LP8556");
MODULE_AUTHOR("Daniel Jeong <daniel.jeong@ti.com>");
MODULE_AUTHOR("G.Shark Jeong <gshark.jeong@gmail.com>");
MODULE_LICENSE("GPL v2");
