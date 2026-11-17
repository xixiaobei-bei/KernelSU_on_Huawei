/*
 * rt4539.c
 *
 * rt4539 driver for backlight
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

#include "rt4539.h"
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

static bool g_rt4539_used;
static bool g_init_status;
static struct class *g_rt_class;
static struct rt4539_chip_data *g_rtdev;
static struct rt4539_info g_bl_info;

static struct gpio_desc g_gpio_on_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_US,
		DELAY_0_US, GPIO_RT4539_EN_NAME,
		&g_bl_info.rt4539_hw_en_gpio, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS,
		DELAY_5_MS, GPIO_RT4539_EN_NAME,
		&g_bl_info.rt4539_hw_en_gpio, 1 },
};
static struct gpio_desc g_gpio_disable_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US,
		DELAY_0_US, GPIO_RT4539_EN_NAME,
		&g_bl_info.rt4539_hw_en_gpio, 0 },
	{ DTYPE_GPIO_INPUT, WAIT_TYPE_US,
		DELAY_10_US, GPIO_RT4539_EN_NAME,
		&g_bl_info.rt4539_hw_en_gpio, 0 },
};
static struct gpio_desc g_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_US,
		DELAY_50_US, GPIO_RT4539_EN_NAME,
		&g_bl_info.rt4539_hw_en_gpio, 0 },
};
static char *g_dts_str[RT4539_RW_REG_MAX] = {
	"rt4539_control_mode",           /* register 0x00 */
	"rt4539_current_protection",     /* register 0x01 */
	"rt4539_current_setting",        /* register 0x02 */
	"rt4539_voltage_setting",        /* register 0x03 */
	"rt4539_brightness_msb",         /* register 0x04 */
	"rt4539_brightness_lsb",         /* register 0x05 */
	"rt4539_slope_time_control",     /* register 0x06 */
	"rt4539_sample_rate_setting",    /* register 0x07 */
	"rt4539_mode_division",          /* register 0x08 */
	"rt4539_control_clk_pfm_func",   /* register 0x09 */
	"rt4539_reg_config_50",          /* register 0x0A */
	"rt4539_backlight_control",      /* register 0x0B */
	"rt4539_mtp_function",           /* register 0xFF */
};
static unsigned int g_reg_addr[RT4539_RW_REG_MAX] = {
	RT4539_CONTROL_MODE_ADDR,
	RT4539_CURRENT_PROTECTION_ADDR,
	RT4539_CURRENT_SETTING_ADDR,
	RT4539_VOLTAGE_SETTING_ADDR,
	RT4539_BRIGHTNESS_MSB_ADDR,
	RT4539_BRIGHTNESS_LSB_ADDR,
	RT4539_SLOPE_TIME_CONTROL_ADDR,
	RT4539_SAMPLE_RATE_SETTING_ADDR,
	RT4539_MODE_DIVISION_ADDR,
	RT4539_CONTROL_CLK_PFM_FUNC_ADDR,
	RT4539_REG_CONFIG_50,
	RT4539_BACKLIGHT_CONTROL,
	RT4539_MTP_FUNCTION,
};

unsigned int g_rt4539_msg_level = DEFAULT_MSG_LEVEL;
module_param_named(debug_rt4539_msg_level, g_rt4539_msg_level, int, 0640);
MODULE_PARM_DESC(debug_rt4539_msg_level, "backlight rt4539 msg level");

static int rt4539_parse_dts(struct device_node *np)
{
	int i;
	int ret;

	for (i = 0; i < RT4539_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, g_dts_str[i],
			&g_bl_info.reg[i]);
		if (ret < 0) {
			g_bl_info.reg[i] = PARSE_FAILED;
			rt4539_info(g_rtdev->dev, "Not find :%s\n",
				g_dts_str[i]);
		}
	}
	ret = of_property_read_u32(np, "bl_set_long_slope",
		&g_bl_info.bl_set_long_slope);
	if (ret < 0)
		rt4539_info(g_rtdev->dev,
			"bl_set_long_slope dts don't config\n");
	ret = of_property_read_u32(np, "dual_ic", &g_bl_info.dual_ic);
	if (ret < 0) {
		rt4539_info(g_rtdev->dev, "can not get dual_ic dts node\n");
	} else {
		ret = of_property_read_u32(np, "rt4539_i2c_bus_id", &g_bl_info.rt4539_i2c_bus_id);
		if (ret < 0)
			rt4539_info(g_rtdev->dev, "can not get rt4539_i2c_bus_id dts node\n");
	}
	ret = of_property_read_u32(np, "rt4539_hw_en_gpio",
		&g_bl_info.rt4539_hw_en_gpio);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev,
			"get rt4539_hw_en_gpio dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_on_kernel_mdelay",
		&g_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev,
			"get bl_on_kernel_mdelay dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_led_num",
		&g_bl_info.bl_led_num);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "get bl_led_num dts config failed\n");
		return ret;
	}
	return ret;
}

static int rt4539_2_config_write(struct rt4539_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	struct i2c_adapter *adap = NULL;
	struct i2c_msg msg = {0};
	char buf[2];
	int ret = 0;
	unsigned int i;

	if((pchip == NULL) || (reg == NULL) || (val == NULL) || (pchip->client == NULL)) {
		rt4539_err(g_rtdev->dev, "pchip or reg or val or clinet is null pointer\n");
		return -1;
	}

	rt4539_info(g_rtdev->dev, "rt4539_2_config_write\n");
	/* get i2c adapter */
	adap = i2c_get_adapter(g_bl_info.rt4539_i2c_bus_id);
	if (!adap) {
		rt4539_err(g_rtdev->dev, "i2c device %d not found\n", g_bl_info.rt4539_i2c_bus_id);
		ret = -ENODEV;
		goto out;
	}
	msg.addr = pchip->client->addr;
	msg.flags = pchip->client->flags;
	msg.len = 2;
	msg.buf = buf;
	for (i = 0; i < size; i++) {
		buf[0] = reg[i];
		buf[1] = val[i];
		if (val[i] != PARSE_FAILED) {
			ret = i2c_transfer(adap, &msg, 1);
			rt4539_info(g_rtdev->dev, "rt4539_2_config_write reg=0x%x,val=0x%x\n", buf[0], buf[1]);
		}
	}
out:
	i2c_put_adapter(adap);
	return ret;
}

static int rt4539_config_write(struct rt4539_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	unsigned int i;
	int ret = 0;

	for (i = 0; i < size; i++) {
		if (val[i] != PARSE_FAILED) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			if (ret < 0) {
				rt4539_err(g_rtdev->dev,
					"write reg 0x%x failed\n", reg[i]);
				goto exit;
			}
		}
	}
exit:
	return ret;
}

static int rt4539_config_read(struct rt4539_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = 0;
	unsigned int i;

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			rt4539_err(g_rtdev->dev,
				"read rt4539 register 0x%x failed\n", reg[i]);
			goto exit;
		} else {
			rt4539_info(g_rtdev->dev, "read 0x%x value = 0x%x\n",
				reg[i], val[i]);
		}
	}
exit:
	return ret;
}

static int rt4539_chip_init(struct rt4539_chip_data *pchip)
{
	int ret;

	rt4539_info(g_rtdev->dev, "in\n");
	if (g_bl_info.dual_ic) {
		ret = rt4539_2_config_write(pchip, g_reg_addr, g_bl_info.reg, RT4539_RW_REG_MAX);
		if (ret < 0) {
			rt4539_err(g_rtdev->dev, "rt4539 slave register failed\n");
			goto out;
		}
	}
	ret = rt4539_config_write(pchip, g_reg_addr,
		g_bl_info.reg, RT4539_RW_REG_MAX);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "rt4539 register failed\n");
		goto out;
	}
	rt4539_info(g_rtdev->dev, "OK\n");
	return ret;
out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

ssize_t rt4539_set_backlight_init(uint32_t bl_level)
{
	int ret = 0;

	if (g_fake_lcd_flag) {
		rt4539_info(g_rtdev->dev, "is fake lcd\n");
		return ret;
	}
	if (down_trylock(&(g_rtdev->test_sem))) {
		rt4539_info(g_rtdev->dev, "Now in test mode\n");
		return 0;
	}
	if ((g_init_status == false) && (bl_level > 0)) {
		mdelay(g_bl_info.bl_on_kernel_mdelay);
		gpio_cmds_tx(g_gpio_on_cmds,
			ARRAY_SIZE(g_gpio_on_cmds));
		/* chip initialize */
		ret = rt4539_chip_init(g_rtdev);
		if (ret < 0) {
			rt4539_err(g_rtdev->dev, "rt4539 init fail\n");
			goto out;
		}
		g_init_status = true;
	} else {
		rt4539_debug(g_rtdev->dev,
			"rt4539 %u, 0: off; else inited\n", bl_level);
	}
out:
	up(&(g_rtdev->test_sem));
	return ret;
}

bool is_rt4539_used(void)
{
	return g_rt4539_used;
}

static ssize_t rt4539_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct i2c_client *client = NULL;
	struct rt4539_chip_data *pchip = NULL;

	if (!buf) {
		rt4539_err(g_rtdev->dev, "buf is null\n");
		return ret;
	}
	if (!dev) {
		ret =  snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "dev is null\n");
		return ret;
	}
	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "data is null\n");
		return ret;
	}
	client = pchip->client;
	if (!client) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "client is null\n");
		return ret;
	}
	ret = rt4539_config_read(pchip,
		g_reg_addr, g_bl_info.reg, RT4539_RW_REG_MAX);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "rt4539 config read failed\n");
		goto i2c_error;
	}
	/* transfer rt4539 regiesters config information to Application layer */
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"Control_mode_0x00= 0x%x\n"
		"Current_protection_0x01 = 0x%x\n"
		"\rCurrent_setting_0x02 = 0x%x\n"
		"Voltage_setting_0x03 = 0x%x\n"
		"\rBrightness_msb_0x04 = 0x%x\n"
		"Brightness_lsb_0x05 = 0x%x\n"
		"\rSlope_time_control_0x06 = 0x%x\n"
		"Sample_rate_setting_0x07 = 0x%x\n"
		"\rMode_division_0x08 = 0x%x\n"
		"Control_clk_pfm_func_0x09 = 0x%x\n"
		"\rConfig_reg_50_0x0A = 0x%x\n"
		"Backlight_control_0x0B = 0x%x\n"
		"\rMtp_function_0xFE = 0x%x\n",
		g_bl_info.reg[0], g_bl_info.reg[1],
		g_bl_info.reg[2], g_bl_info.reg[3],
		g_bl_info.reg[4], g_bl_info.reg[5],
		g_bl_info.reg[6], g_bl_info.reg[7],
		g_bl_info.reg[8], g_bl_info.reg[9],
		g_bl_info.reg[10],g_bl_info.reg[11],
		g_bl_info.reg[12]);
	return ret;
i2c_error:
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s: i2c access failed\n", __func__);
	return ret;
}

static ssize_t rt4539_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val;
	unsigned int reg;
	unsigned int mask;
	struct rt4539_chip_data *pchip = NULL;

	if (!buf) {
		rt4539_err(g_rtdev->dev, "buf is null\n");
		return -1;
	}
	if (!dev) {
		rt4539_err(g_rtdev->dev, "dev is null\n");
		return -1;
	}
	pchip = dev_get_drvdata(dev);
	ret = sscanf_s(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		rt4539_info(g_rtdev->dev, "check your input\n");
		goto out_input;
	}
	rt4539_info(g_rtdev->dev,
		"reg=0x%x,mask=0x%x,val=0x%x\n", reg, mask, val);
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

static DEVICE_ATTR(reg, 0644, rt4539_reg_show, rt4539_reg_store);

static struct attribute *g_rt4539_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group g_rt4539_group = {
	.attrs = g_rt4539_attributes,
};

static const struct regmap_config rt4539_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_common.h"

static void rt4539_enable(void)
{
	int ret;

	mdelay(g_bl_info.bl_on_kernel_mdelay);
	gpio_cmds_tx(g_gpio_on_cmds, ARRAY_SIZE(g_gpio_on_cmds));
	/* chip initialize */
	ret = rt4539_chip_init(g_rtdev);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "rt4539_chip_init fail\n");
		return;
	}
	g_init_status = true;
}

static void rt4539_disable(void)
{
	gpio_cmds_tx(g_gpio_disable_cmds,
		ARRAY_SIZE(g_gpio_disable_cmds));
	gpio_cmds_tx(g_gpio_free_cmds,
		ARRAY_SIZE(g_gpio_free_cmds));
	msleep(RT4539_DISABLE_DELAY);
	g_init_status = false;
}

static int rt4539_set_backlight(uint32_t bl_value)
{
	int ret;
	int32_t bl_max;
	unsigned int bl_msb;
	unsigned int bl_lsb;
	uint32_t bl_level = bl_value;

	if (!g_rtdev) {
		printk(KERN_ERR "rt4539_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(g_rtdev->test_sem))) {
		rt4539_info(g_rtdev->dev, "Now in test mode\n");
		return 0;
	}

	/* first set backlight, enable rt4539 */
	if ((g_init_status == false) && (bl_level > 0))
		rt4539_enable();

	bl_max = bl_config_max_value();
	if (bl_max > 0)
		bl_level = bl_level * RT4539_BL_MAX / bl_max;
	if (bl_level > RT4539_BL_MAX)
		bl_level = RT4539_BL_MAX;

	/* set backlight level rt4539 backlight level 12bits */
	bl_msb = (bl_level >> 8) & 0x0F; /* 4 msb bits */
	bl_lsb = bl_level & 0xFF; /* 8 lsb bits */

	ret = regmap_write(g_rtdev->regmap,
		g_bl_info.rt4539_level_msb, bl_msb);
	if (ret < 0)
		rt4539_err(g_rtdev->dev,
			"write bl level msb:0x%x failed\n", bl_msb);

	ret = regmap_write(g_rtdev->regmap,
		g_bl_info.rt4539_level_lsb, bl_lsb);
	if (ret < 0)
		rt4539_err(g_rtdev->dev,
			"write bl level lsb:0x%x failed\n", bl_lsb);

	/* if set backlight level 0, disable rt4539 */
	if ((g_init_status == true) && (bl_level == 0))
		rt4539_disable();
	up(&(g_rtdev->test_sem));
	return ret;
}

static int rt4539_en_backlight(uint32_t bl_value)
{
	int ret = 0;

	if (!g_rtdev) {
		printk(KERN_ERR "rt4539_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(g_rtdev->test_sem))) {
		rt4539_info(g_rtdev->dev, "Now in test mode\n");
		return 0;
	}
	rt4539_info(g_rtdev->dev, "rt4539_en_backlight bl_value=%d\n", bl_value);

	/* first set backlight, enable rt4539 */
	if ((g_init_status == false) && (bl_value > 0))
		rt4539_enable();

	/* if set backlight level 0, disable rt4539 */
	if ((g_init_status == true) && (bl_value == 0))
		rt4539_disable();

	up(&(g_rtdev->test_sem));
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = rt4539_set_backlight,
	.en_backlight = rt4539_en_backlight,
	.name = "4539",
};
#endif

static int rt4539_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct rt4539_chip_data *pchip = NULL;
	struct i2c_adapter *adapter = client->adapter;

	rt4539_info(&client->dev, "in\n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}
	pchip = devm_kzalloc(&client->dev,
		sizeof(struct rt4539_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;
#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &rt4539_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "allocate regmap failed: %d\n", ret);
		goto err_out;
	}
#endif
	g_rtdev = pchip;
	pchip->client = client;
	i2c_set_clientdata(client, pchip);
	sema_init(&(pchip->test_sem), 1);
	g_rt_class = class_create(THIS_MODULE, "rt4539");
	if (IS_ERR(g_rt_class)) {
		rt4539_err(g_rtdev->dev,
			"Create rt4539 class failed; errno = %ld\n",
			PTR_ERR(g_rt_class));
		g_rt_class = NULL;
	}
	pchip->dev = device_create(g_rt_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		rt4539_err(g_rtdev->dev,
			"Create device failed; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &g_rt4539_group);
		if (ret)
			goto err_sysfs;
	}
	ret = memset_s(&g_bl_info, sizeof(struct rt4539_info),
			0, sizeof(struct rt4539_info));
	if (ret != 0) {
		rt4539_err(g_rtdev->dev, "memset bl_info failed!\n");
		goto err_sysfs;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_RT4539);
	if (!np) {
		rt4539_err(g_rtdev->dev, "NOT FOUND device node %s\n", DTS_COMP_RT4539);
		goto err_sysfs;
	}
	ret = rt4539_parse_dts(np);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "parse rt4539 dts failed\n");
		goto err_sysfs;
	}
#ifdef CONFIG_LCD_KIT_DRIVER
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_RT4539);
	if (!np) {
		rt4539_err(g_rtdev->dev, "NOT FOUND device node %s\n", DTS_COMP_RT4539);
		goto err_sysfs;
	}
	/* Only testing rt4539 used */
	ret = regmap_read(pchip->regmap, g_reg_addr[0],
		&g_bl_info.reg[0]);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "rt4539 not used\n");
		goto err_sysfs;
	}
	/* Testing rt4539-2 used */
	if (g_bl_info.dual_ic) {
		ret = rt4539_2_config_write(pchip, g_reg_addr, g_bl_info.reg, 1);
		if (ret < 0) {
			rt4539_err(g_rtdev->dev, "rt4539 slave not used\n");
			goto err_sysfs;
		}
	}
	ret = of_property_read_u32(np, "rt4539_level_lsb",
		&g_bl_info.rt4539_level_lsb);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "get rt4539_level_lsb failed\n");
		goto err_sysfs;
	}

	ret = of_property_read_u32(np, "rt4539_level_msb",
		&g_bl_info.rt4539_level_msb);
	if (ret < 0) {
		rt4539_err(g_rtdev->dev, "get rt4539_level_msb failed\n");
		goto err_sysfs;
	}
	g_rt4539_used = true;
	lcd_kit_bl_register(&bl_ops);
#endif
	return ret;
err_sysfs:
	rt4539_err(g_rtdev->dev, "sysfs error\n");
	device_destroy(g_rt_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int rt4539_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &g_rt4539_group);
	return 0;
}

static const struct i2c_device_id rt4539_id[] = {
	{ RT4539_NAME, 0 },
	{},
};

static const struct of_device_id rt4539_of_id_table[] = {
	{ .compatible = "realtek,rt4539" },
	{},
};

MODULE_DEVICE_TABLE(i2c, rt4539_id);
static struct i2c_driver rt4539_i2c_driver = {
	.driver = {
		.name = "rt4539",
		.owner = THIS_MODULE,
		.of_match_table = rt4539_of_id_table,
	},
	.probe = rt4539_probe,
	.remove = rt4539_remove,
	.id_table = rt4539_id,
};

static int __init rt4539_module_init(void)
{
	return i2c_add_driver(&rt4539_i2c_driver);
}

late_initcall(rt4539_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HW rt4539 LED driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
