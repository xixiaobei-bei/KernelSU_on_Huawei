/*
 * lp8557.c
 *
 * lp8557 driver for backlight
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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
#include <securec.h>
#include "lp8557.h"
#include "hisi_fb.h"
#include "../hisi_fb_panel.h"

#define TEST_ERROR_CHIP_INIT     BIT(16)

static struct lp8557_backlight_information lp8557_bl_info;

static char *lp8557_dts_string[LP8557_RW_REG_MAX] = {
	"lp8557_config",
	"lp8557_current",
	"lp8557_pgen",
	"lp8557_boost",
	"lp8557_led_enable",
	"lp8557_step",
	"lp8557_command",
	"lp8557_brt_low",
	"lp8557_brt_high"
};

static unsigned int lp8557_reg_addr[LP8557_RW_REG_MAX] = {
	LP8557_CONFIG,
	LP8557_CURRENT,
	LP8557_PGEN,
	LP8557_BOOST,
	LP8557_LED_ENABLE,
	LP8557_STEP,
	LP8557_COMMAND,
	LP8557_BRT_LOW,
	LP8557_BRT_HIGH
};

struct class *lp8557_class = NULL;
struct lp8557_chip_data *lp8557_g_chip = NULL;
static bool lp8557_init_status = false;

/*
 * for debug, S_IRUGO
 * /sys/module/dpufb/parameters
*/
unsigned lp8557_msg_level = 7;
module_param_named(debug_lp8557_msg_level, lp8557_msg_level, int, 0640);
MODULE_PARM_DESC(debug_lp8557_msg_level, "backlight lp8557 msg level");

static int lp8557_parse_dts(const struct device_node *np)
{
	int ret;
	int i;

	if (np == NULL) {
		LP8557_ERR("np is null pointer\n");
		return -1;
	}

	for (i = 0; i < LP8557_RW_REG_MAX; i++ ) {
		ret = of_property_read_u32(np, lp8557_dts_string[i], &lp8557_bl_info.lp8557_reg[i]);
		if (ret < 0) {
			lp8557_bl_info.lp8557_reg[i] = 0xffff; // init to invalid data
			LP8557_INFO("can not find config:%s\n", lp8557_dts_string[i]);
		}
	}
	ret = of_property_read_u32(np, "dual_ic", &lp8557_bl_info.dual_ic);
	if (ret < 0) {
		LP8557_INFO("can not get dual_ic dts node\n");
	} else {
		ret = of_property_read_u32(np, "lp8557_i2c_bus_id", &lp8557_bl_info.lp8557_i2c_bus_id);
		if (ret < 0)
			LP8557_INFO("can not get lp8557_i2c_bus_id dts node\n");
	}
	ret = of_property_read_u32(np, "bl_on_kernel_mdelay", &lp8557_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		LP8557_ERR("get bl_on_kernel_mdelay dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_led_num", &lp8557_bl_info.bl_led_num);
	if (ret < 0) {
		LP8557_ERR("get bl_led_num dts config failed\n");
		return ret;
	}

	return ret;
}

static int lp8557_2_config_write(struct lp8557_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	struct i2c_adapter *adap = NULL;
	struct i2c_msg msg = {0};
	char buf[2];
	int ret;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL) || (pchip->client == NULL)) {
		LP8557_ERR("pchip or reg or val is null pointer\n");
		return -1;
	}
	LP8557_INFO("lp8557_2_config_write\n");
	/* get i2c adapter */
	adap = i2c_get_adapter(lp8557_bl_info.lp8557_i2c_bus_id);
	if (!adap) {
		LP8557_ERR("i2c device %d not found\n", lp8557_bl_info.lp8557_i2c_bus_id);
		ret = -ENODEV;
		goto out;
	}
	msg.addr = pchip->client->addr;
	msg.flags = pchip->client->flags;
	msg.len = 2; // 2: msg length is 2
	msg.buf = buf;
	for (i = 0; i < size; i++) {
		buf[0] = reg[i];
		buf[1] = val[i];
		if (val[i] != 0xffff) {
			ret = i2c_transfer(adap, &msg, 1);
			LP8557_INFO("lp8557_2_config_write reg=0x%x,val=0x%x\n", buf[0], buf[1]);
		}
	}
out:
	i2c_put_adapter(adap);
	return ret;
}

static int lp8557_config_write(struct lp8557_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = 0;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LP8557_ERR("pchip or reg or val is null pointer\n");
		return -1;
	}

	for (i = 0; i < size; i++) {
		/* judge reg is invalid */
		if (val[i] != 0xffff) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			if (ret < 0) {
				LP8557_ERR("write lp8557 backlight config register 0x%x failed\n", reg[i]);
				goto exit;
			}
		}
	}

exit:
	return ret;
}

static int lp8557_config_read(struct lp8557_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret;
	unsigned int i;

	if ((pchip == NULL) || (reg == NULL) || (val == NULL)) {
		LP8557_ERR("pchip or reg or val is null pointer\n");
		return -1;
	}

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			LP8557_ERR("read lp8557 backlight config register 0x%x failed", reg[i]);
			goto exit;
		} else {
			LP8557_INFO("read 0x%x value = 0x%x\n", reg[i], val[i]);
		}
	}

exit:
	return ret;
}

/* initialize chip */
static int lp8557_chip_init(struct lp8557_chip_data *pchip)
{
	int ret = -1;

	LP8557_INFO("in!\n");

	if (pchip == NULL) {
		LP8557_ERR("pchip is null pointer\n");
		return -1;
	}
	if (lp8557_bl_info.dual_ic) {
		ret = lp8557_2_config_write(pchip, lp8557_reg_addr, lp8557_bl_info.lp8557_reg,
			LP8557_RW_REG_MAX);
		if (ret < 0) {
			LP8557_ERR("lp8557 slave config register failed\n");
		goto out;
		}
	}
	ret = lp8557_config_write(pchip, lp8557_reg_addr, lp8557_bl_info.lp8557_reg,
		LP8557_RW_REG_MAX);
	if (ret < 0) {
		LP8557_ERR("lp8557 config register failed");
		goto out;
	}
	LP8557_INFO("ok!\n");
	return ret;

out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

/**
 * lp8557_set_backlight_init(): initial ic working mode
 *
 * @bl_level: value for backlight ,range from 0 to ~
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t lp8557_set_backlight_init(uint32_t bl_level)
{
	int ret = 0;

	if (g_fake_lcd_flag) {
		LP8557_INFO("is fake lcd!\n");
		return ret;
	}

	if (down_trylock(&(lp8557_g_chip->test_sem))) {
		LP8557_INFO("Now in test mode\n");
		return 0;
	}

	if (false == lp8557_init_status && bl_level > 0) {
		mdelay(lp8557_bl_info.bl_on_kernel_mdelay);
		/* chip initialize */
		ret = lp8557_chip_init(lp8557_g_chip);
		if (ret < 0) {
			LP8557_ERR("lp8557_chip_init fail!\n");
			goto out;
		}
		lp8557_init_status = true;
	} else {
		LP8557_DEBUG("lp8557_chip_init %u, 0: already off; else : already init!\n", bl_level);
	}

out:
	up(&(lp8557_g_chip->test_sem));
	return ret;
}

static ssize_t lp8557_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lp8557_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret = -1;
	int val[LP8557_RW_REG_MAX] = { 0 };

	if (!buf) {
		LP8557_ERR("buf is null\n");
		return ret;
	}

	if (!dev) {
		ret =  snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s", "dev is null\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s", "data is null\n");
		return ret;
	}

	client = pchip->client;
	if (!client) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s", "client is null\n");
		return ret;
	}

	ret = lp8557_config_read(pchip, lp8557_reg_addr, val, LP8557_RW_REG_MAX);
	if (ret < 0) {
		LP8557_ERR("lp8557 config read failed");
		goto i2c_error;
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "Config(0x10) = 0x%x\n \
		\rEprom Current(0x11) = 0x%x\nPGEN(0x12) = 0x%x\n \
		\rBoost(0x13) = 0x%x\nLED enable(0x14) = 0x%x\n \
		\rStep(0x15) = 0x%x\nCommand(0x00) = 0x%x\n \
		\rBrightness low(0x03) 0x%x\nBrightness high(0x04) = 0x%x\n",
		val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8]);
	return ret;

i2c_error:
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s: i2c access fail to register\n", __func__);
	return ret;
}

static ssize_t lp8557_reg_store(struct device *dev,
	struct device_attribute *devAttr, const char *buf, size_t size)
{
	ssize_t ret;
	struct lp8557_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!buf) {
		LP8557_ERR("buf is null\n");
		return -1;
	}

	if (!dev) {
		LP8557_ERR("dev is null\n");
		return -1;
	}

	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		LP8557_ERR("pchip is null\n");
		return -1;
	}

	ret = sscanf_s(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		LP8557_INFO("check your input!!!\n");
		goto out_input;
	}

	LP8557_INFO("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask, val);

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

static DEVICE_ATTR(reg, (S_IRUGO|S_IWUSR), lp8557_reg_show, lp8557_reg_store);

/* pointers to created device attributes */
static struct attribute *lp8557_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group lp8557_group = {
	.attrs = lp8557_attributes,
};

static const struct regmap_config lp8557_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_common.h"

static void lp8557_enable(void)
{
	int ret;

	mdelay(lp8557_bl_info.bl_on_kernel_mdelay);
	/* chip initialize */
	ret = lp8557_chip_init(lp8557_g_chip);
	if (ret < 0) {
		LP8557_ERR("lp8557_chip_init fail!\n");
		return;
	}
	lp8557_init_status = true;
}

static void lp8557_disable(void)
{
	int ret;

	ret = regmap_write(lp8557_g_chip->regmap, LP8557_COMMAND, LP8557_DISABLE_VAL);
	if (ret < 0) {
		LP8557_ERR("[lp8557_disable] write LP8557_COMMAND = 0x00 failed\n");
		return;
	}
	lp8557_init_status = false;
}

static int lp8557_set_backlight(uint32_t bl_level)
{
	static int last_bl_level = 0;
	int bl_msb;
	int bl_lsb;
	int ret;

	if (!lp8557_g_chip) {
		LP8557_ERR("lp8557_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(lp8557_g_chip->test_sem))) {
		LP8557_INFO("Now in test mode\n");
		return 0;
	}
	/* first set backlight, enable lp8557 */
	if (lp8557_init_status == false && bl_level > 0)
		lp8557_enable();

	/* set backlight level */
	bl_msb = (bl_level >> 4) & 0xFF; // move 4 bits for 8 msb bits
	bl_lsb = (bl_level & 0x0F) << 4; // 4 lsb bits
	ret = regmap_write(lp8557_g_chip->regmap, lp8557_bl_info.lp8557_level_lsb, bl_lsb);
	if (ret < 0)
		LP8557_DEBUG("write lp8557 backlight level lsb:0x%x failed\n", bl_lsb);

	ret = regmap_write(lp8557_g_chip->regmap, lp8557_bl_info.lp8557_level_msb, bl_msb);
	if (ret < 0)
		LP8557_DEBUG("write lp8557 backlight level msb:0x%x failed\n", bl_msb);

	/* if set backlight level 0, disable lp8557 */
	if (lp8557_init_status == true && bl_level == 0)
		lp8557_disable();

	up(&(lp8557_g_chip->test_sem));

	/* when power on backlight, schedule backlight check work */
	if (last_bl_level == 0 && bl_level > 0) {
		if (common_info->check_thread.check_bl_support)
			/* delay 500ms schedule work */
			schedule_delayed_work(&common_info->check_thread.check_work, (HZ / 5));
		LP8557_INFO("level = %d, bl_msb = %d, bl_lsb = %d", bl_level, bl_msb, bl_lsb);
	}
	last_bl_level = bl_level;
	return ret;
}

static int lp8557_en_backlight(uint32_t bl_level)
{
	static int last_bl_level = 0;
	int ret = 0;

	if (!lp8557_g_chip) {
		LP8557_ERR("lp8557_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(lp8557_g_chip->test_sem))) {
		LP8557_INFO("Now in test mode\n");
		return 0;
	}
	LP8557_INFO("lp8557_en_backlight bl_level=%d\n", bl_level);
	/* first set backlight, enable lp8557 */
	if (lp8557_init_status == false && bl_level > 0)
		lp8557_enable();

	/* if set backlight level 0, disable lp8557 */
	if (lp8557_init_status == true && bl_level == 0)
		lp8557_disable();

	up(&(lp8557_g_chip->test_sem));

	/* when power on backlight, schedule backlight check work */
	if (last_bl_level == 0 && bl_level > 0) {
		if (common_info->check_thread.check_bl_support)
			/* delay 500ms schedule work */
			schedule_delayed_work(&common_info->check_thread.check_work, (HZ / 5));
	}
	last_bl_level = bl_level;
	return ret;
}


static int lp8557_backlight_save_restore(int save_enable)
{
	static int bl_msb = 0;
	static int bl_lsb = 0;
	int val = 0;
	int ret;

	if (save_enable) {
		ret = regmap_read(lp8557_g_chip->regmap, lp8557_bl_info.lp8557_level_lsb, &val);
		if (ret < 0) {
			LP8557_ERR("write lp8557 backlight level lsb:0x%x failed\n", bl_lsb);
			return -1;
		}
		bl_lsb = val & 0xF0;
		ret = regmap_read(lp8557_g_chip->regmap, lp8557_bl_info.lp8557_level_msb, &val);
		if (ret < 0) {
			LP8557_ERR("write lp8557 backlight level msb:0x%x failed\n", bl_msb);
			return -1;
		}
		bl_msb = val & 0xFF;
	} else {
		ret = regmap_write(lp8557_g_chip->regmap, lp8557_bl_info.lp8557_level_lsb, bl_lsb);
		if (ret < 0) {
			LP8557_ERR("write lp8557 backlight level lsb:0x%x failed\n", bl_lsb);
			return -1;
		}
		ret = regmap_write(lp8557_g_chip->regmap, lp8557_bl_info.lp8557_level_msb, bl_msb);
		if (ret < 0) {
			LP8557_ERR("write lp8557 backlight level msb:0x%x failed\n", bl_msb);
			return -1;
		}
	}
	return ret;
}


static int lp8557_test_led_open(struct lp8557_chip_data *pchip, int led_num)
{
	int ret;
	int i;
	int result = TEST_OK;
	unsigned int val = 0;
	unsigned int enable_leds = LP8557_ENABLE_ALL_LEDS;
	int bl_led_num = lp8557_bl_info.bl_led_num;

	if (pchip == NULL) {
		LP8557_ERR("pchip is null pointer\n");
		return -1;
	}

	for (i = bl_led_num; i < LP8557_LED_NUM; i++)
		enable_leds &= ~(unsigned int)(0x01 << (unsigned int)i);
	/* Enable all LED strings */
	ret = regmap_write(pchip->regmap, LP8557_LED_ENABLE, enable_leds);
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* Set maximum brightness */
	lp8557_set_backlight(LP8557_BL_MAX);

	/* Open LEDx string. */
	ret = regmap_write(pchip->regmap, LP8557_LED_ENABLE,
		(~(unsigned int)(0x01 << (unsigned int)led_num)) & enable_leds);
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* Wait 4 msec. */
	msleep(4);

	/* Read LED open fault */
	ret = regmap_read(pchip->regmap, LP8557_FUALT_FLAG, &val);
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* If then a LED open fault condition has been detected. */
	if (val & (1 << LP8557_FAULT_OPEN_BIT))
		result |= (1<<(LP8557_LED1_OPEN_ERR_BIT + led_num));

	/* Connect LEDx string. */
	ret = regmap_write(pchip->regmap, LP8557_LED_ENABLE, enable_leds);
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return result | TEST_ERROR_I2C;
	}

	/* Repeat the procedure for the other LED strings. */
	msleep(1000); // Wait 1000 msec
	return result;
}

static int lp8557_test_led_short(struct lp8557_chip_data *pchip, int led_num)
{
	unsigned int val = 0;
	int ret;
	int result = TEST_OK;

	if (pchip == NULL) {
		LP8557_ERR("pchip is null pointer\n");
		return -1;
	}

	/* Enable only LEDx string. */
	ret = regmap_write(pchip->regmap, LP8557_LED_ENABLE, (1<<(unsigned int)led_num));
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* Set maximum brightness. */
	lp8557_set_backlight(LP8557_BL_MAX);

	/* Wait 4 msec. */
	msleep(4);

	/* Read LED short fault */
	ret = regmap_read(pchip->regmap, LP8557_FUALT_FLAG, &val);
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return TEST_ERROR_I2C;
	}

	/* A LED short fault condition has been detected. */
	if (val & (1 << LP8557_FAULT_SHORT_BIT))
		result |= (1<<(LP8557_LED1_SHORT_ERR_BIT + led_num));

	/* Set chip enable and LED string enable low. */
	ret = regmap_write(pchip->regmap, LP8557_LED_ENABLE, LP8557_DISABLE_ALL_LEDS);
	if (ret < 0) {
		LP8557_ERR("TEST_ERROR_I2C\n");
		return result | TEST_ERROR_I2C;
	}

	/* Repeat the procedure for the other LED Strings */
	msleep(1000); // Wait 1000 msec
	return result;
}

static ssize_t lp8557_self_test(void)
{
	struct lp8557_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret;
	int result = 0;
	int lp8557_regs[LP8557_RW_REG_MAX] = {0};
	int led_num = lp8557_bl_info.bl_led_num;
	int i;

	pchip = lp8557_g_chip;
	if (!pchip) {
		LP8557_ERR("pchip is null\n");
		return -1;
	}

	client = pchip->client;
	if (!client)
		LP8557_ERR("client is null\n");

	down(&(pchip->test_sem));
	ret = lp8557_config_read(pchip, lp8557_reg_addr, lp8557_regs, LP8557_RW_REG_MAX);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8557_test_failed;
	}
	ret = lp8557_backlight_save_restore(1);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8557_test_failed;
	}

	for (i = 0; i < led_num; i++)
		result |= lp8557_test_led_open(pchip, i);

	for (i = 0; i < led_num; i++)
		result |= lp8557_test_led_short(pchip, i);

	ret = lp8557_chip_init(pchip);
	if (ret < 0) {
		result |= TEST_ERROR_CHIP_INIT;
		goto lp8557_test_failed;
	}

	ret = lp8557_config_write(pchip, lp8557_reg_addr, lp8557_regs, LP8557_RW_REG_MAX);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8557_test_failed;
	}
	ret = lp8557_backlight_save_restore(0);
	if (ret) {
		result |= TEST_ERROR_I2C;
		goto lp8557_test_failed;
	}

	up(&(pchip->test_sem));
	LP8557_INFO("self test out:%d\n", result);
	return result;

lp8557_test_failed:
	up(&(pchip->test_sem));
	LP8557_INFO("self test out:%d\n", result);
	return result;
}

static void lp8557_dsm_notify(int val)
{
#if defined(CONFIG_HUAWEI_DSM)
	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, "lp8557 happen short, reg:0x01 value is:0x%x\n", val);
		dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
	} else {
		LP8557_ERR("dsm_client_ocuppy fail!\n");
	}
#endif
}

static int lp8557_check_backlight(void)
{
#define CHECK_COUNT	3
#define CHECK_REG	0x01
#define CHECK_VAL	0x04
#define CHECK_VAL_FACTORY	0xC4
	int count = 0;
	int val = 0;
	int ret = 0;
	int err_cnt = 0;

	if (!lp8557_g_chip) {
		LP8557_ERR("lp8557_g_chip is null\n");
		return -1;
	}
	/* judge lp8557 is enable, if not return */
	if (lp8557_init_status == false) {
		LP8557_ERR("lp8557 not enable\n");
		ret = -1;
		goto error;
	}

	while (count < CHECK_COUNT) {
		ret = regmap_read(lp8557_g_chip->regmap, CHECK_REG, &val);
		if (ret < 0) {
			LP8557_INFO("read lp8557 fail!\n");
			goto error;
		}
		if (runmode_is_factory()) {
			if (val & CHECK_VAL_FACTORY) {
				err_cnt++;
				LP8557_ERR("check val:0x%x, backlight maybe short!\n", val);
			}
		} else {
			if (val & CHECK_VAL) {
				err_cnt++;
				LP8557_ERR("check val:0x%x, backlight maybe short!\n", val);
			}
		}
		count++;
		mdelay(3); /* delay 3ms */
	}
	if (err_cnt == CHECK_COUNT) {
		/* backlight short, shutdown backlight */
		LP8557_ERR("backlight short, shutdown backlight!\n");

		ret = regmap_write(lp8557_g_chip->regmap, lp8557_reg_addr[0], LP8557_DISABLE_VAL);
		if (ret < 0)
			LP8557_ERR("[lp8557_disable] write lp8557_reg_addr[0] = 0x00 failed\n");
		lp8557_dsm_notify(val);
		ret = -1;
		goto error;
	}
error:
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = lp8557_set_backlight,
	.en_backlight = lp8557_en_backlight,
	.bl_self_test = lp8557_self_test,
	.check_backlight = lp8557_check_backlight,
	.name = "8557",
};
#endif

static int lp8557_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct lp8557_chip_data *pchip = NULL;
	int ret = -1;
	struct device_node *np = NULL;

	LP8557_INFO("in!\n");

	if (!client) {
		LP8557_ERR("client is null pointer\n");
		return -1;
	}
	adapter = client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev,
		sizeof(struct lp8557_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &lp8557_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "fail : allocate register map: %d\n", ret);
		goto err_out;
	}
#endif

	lp8557_g_chip = pchip;
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(lp8557_class, NULL, 0, NULL, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		LP8557_ERR("Unable to create device; errno = %ld\n", PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &lp8557_group);
		if (ret)
			goto err_sysfs;
	}

	ret = memset_s(&lp8557_bl_info, sizeof(struct lp8557_backlight_information),
		0, sizeof(struct lp8557_backlight_information));
	if (ret != 0) {
		LP8557_ERR("memset bl_info failed!\n");
		goto err_sysfs;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LP8557);
	if (!np) {
		LP8557_ERR("NOT FOUND device node %s!\n", DTS_COMP_LP8557);
		goto err_sysfs;
	}

	ret = lp8557_parse_dts(np);
	if (ret < 0) {
		LP8557_ERR("parse lp8557 dts failed");
		goto err_sysfs;
	}

#ifdef CONFIG_LCD_KIT_DRIVER
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LP8557);
	if (!np) {
		LP8557_ERR("NOT FOUND device node %s!\n", DTS_COMP_LP8557);
		goto err_sysfs;
	}
	/* Only testing lp8557 used */
	ret = regmap_read(pchip->regmap,
		lp8557_reg_addr[0], &lp8557_bl_info.lp8557_reg[0]);
	if (ret < 0) {
		LP8557_ERR("lp8557 not used\n");
		goto err_sysfs;
	}
	/* Testing lp8557-2 used */
	if (lp8557_bl_info.dual_ic) {
		ret = lp8557_2_config_write(pchip, lp8557_reg_addr, lp8557_bl_info.lp8557_reg, 1);
		if (ret < 0) {
			LP8557_ERR("lp8557 slave not used\n");
			goto err_sysfs;
		}
	}
	ret = of_property_read_u32(np, "lp8557_level_lsb", &lp8557_bl_info.lp8557_level_lsb);
	if (ret < 0) {
		LP8557_ERR("get lp8557_level_lsb failed\n");
		goto err_sysfs;
	}

	ret = of_property_read_u32(np, "lp8557_level_msb", &lp8557_bl_info.lp8557_level_msb);
	if (ret < 0) {
		LP8557_ERR("get lp8557_level_msb failed\n");
		goto err_sysfs;
	}
	lcd_kit_bl_register(&bl_ops);
#endif
	return ret;

err_sysfs:
	LP8557_DEBUG("sysfs error!\n");
	device_destroy(lp8557_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int lp8557_remove(struct i2c_client *client)
{
	if (!client) {
		LP8557_ERR("client is null pointer\n");
		return -1;
	}

	sysfs_remove_group(&client->dev.kobj, &lp8557_group);

	return 0;
}

static const struct i2c_device_id lp8557_id[] = {
	{LP8557_NAME, 0},
	{},
};

static const struct of_device_id lp8557_of_id_table[] = {
	{.compatible = "ti,lp8557"},
	{},
};

MODULE_DEVICE_TABLE(i2c, lp8557_id);
static struct i2c_driver lp8557_i2c_driver = {
		.driver = {
			.name = "lp8557",
			.owner = THIS_MODULE,
			.of_match_table = lp8557_of_id_table,
		},
		.probe = lp8557_probe,
		.remove = lp8557_remove,
		.id_table = lp8557_id,
};

static int __init lp8557_module_init(void)
{
	int ret = -1;

	LP8557_INFO("in!\n");

	lp8557_class = class_create(THIS_MODULE, "lp8557");
	if (IS_ERR(lp8557_class)) {
		LP8557_ERR("Unable to create lp8557 class; errno = %ld\n", PTR_ERR(lp8557_class));
		lp8557_class = NULL;
	}

	ret = i2c_add_driver(&lp8557_i2c_driver);
	if (ret)
		LP8557_ERR("Unable to register lp8557 driver\n");

	LP8557_INFO("ok!\n");

	return ret;
}
late_initcall(lp8557_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HW lp8557 LED driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

