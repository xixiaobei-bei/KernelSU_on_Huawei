/*
 * rt8555.c
 *
 * rt8555 driver for backlight
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include "rt8555.h"
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

struct class *g_rt_class = NULL;
static bool g_init_status;
static bool bl_set_slope_status;
static struct rt8555_chip_data *g_rtdev;
static struct rt8555_backlight_info g_bl_info;

static char *g_dts_str[RT8555_RW_REG_MAX] = {
	"rt8555_control_mode",       /* register 0x00 */
	"rt8555_current_protection", /* register 0x01 */
	"rt8555_current_setting",    /* register 0x02 */
	"rt8555_voltage_setting",    /* register 0x03 */
	"rt8555_brightness_setting", /* register 0x08 */
	"rt8555_time_control",       /* register 0x09 */
	"rt8555_mode_devision",      /* register 0x0A */
	"rt8555_compensation_duty",  /* register 0x0B */
	"rt8555_clk_pfm_enable",     /* register 0x0D */
	"rt8555_led_protection",     /* register 0x0E */
	"rt8555_reg_config_50",      /* register 0x50 */
};
static unsigned int g_reg_addr[RT8555_RW_REG_MAX] = {
	RT8555_CONTROL_MODE_ADDR,
	RT8555_CURRENT_PROTECTION_ADDR,
	RT8555_CURRENT_SETTING_ADDR,
	RT8555_VOLTAGE_SETTING_ADDR,
	RT8555_BRIGHTNESS_SETTING_ADDR,
	RT8555_TIME_CONTROL_ADDR,
	RT8555_MODE_DEVISION_ADDR,
	RT8555_COMPENSATION_DUTY_ADDR,
	RT8555_CLK_PFM_ENABLE_ADDR,
	RT8555_LED_PROTECTION_ADDR,
	RT8555_REG_CONFIG_50,
};

unsigned rt8555_msg_level = 7;
module_param_named(debug_rt8555_msg_level, rt8555_msg_level, int, 0640);
MODULE_PARM_DESC(debug_rt8555_msg_level, "backlight rt8555 msg level");

static int rt8555_parse_dts(struct device_node *np)
{
	int i;
	int ret;
	struct mtk_panel_info *plcd_kit_info = NULL;

	for (i = 0; i < RT8555_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, g_dts_str[i],
			&g_bl_info.reg[i]);
		if (ret < 0) {
			g_bl_info.reg[i] = PARSE_FAILED;
			rt8555_info("Not find :%s\n", g_dts_str[i]);
		}
	}
	ret = of_property_read_u32(np, "bl_set_long_slope",
		&g_bl_info.bl_set_long_slope);
	if (ret < 0)
		rt8555_info("bl_set_long_slope dts don't config\n");
	ret = of_property_read_u32(np, "dual_ic", &g_bl_info.dual_ic);
	if (ret < 0) {
		rt8555_info("can not get dual_ic dts node\n");
	}
	else {
		ret = of_property_read_u32(np, "rt8555_2_i2c_bus_id", &g_bl_info.rt8555_2_i2c_bus_id);
		if (ret < 0)
			rt8555_info("can not get rt8555_2_i2c_bus_id dts node\n");
	}
	ret = of_property_read_u32(np, "rt8555_hw_enable", &g_bl_info.rt8555_hw_en);
	if (ret < 0) {
		rt8555_err("get rt8555_hw_en dts config failed\n");
		g_bl_info.rt8555_hw_en = 0;
	}
	if (g_bl_info.rt8555_hw_en != 0) {
		ret = of_property_read_u32(np, "rt8555_hw_en_gpio", &g_bl_info.rt8555_hw_en_gpio);
		if (ret < 0) {
			rt8555_err("get rt8555_hw_en_gpio dts config failed\n");
			return ret;
		}
		if (g_bl_info.dual_ic) {
			ret = of_property_read_u32(np, "rt8555_2_hw_en_gpio",
				&g_bl_info.rt8555_2_hw_en_gpio);
			if (ret < 0) {
				rt8555_err("get rt8555_2_hw_en_gpio dts config failed\n");
				return ret;
			}
		}
	}
	/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
	plcd_kit_info = lcm_get_panel_info();
	if (plcd_kit_info != NULL) {
		g_bl_info.rt8555_hw_en_gpio += plcd_kit_info->gpio_offset;
		if (g_bl_info.dual_ic)
			g_bl_info.rt8555_2_hw_en_gpio += plcd_kit_info->gpio_offset;
	}
#else
	g_bl_info.rt8555_hw_en_gpio += ((struct mtk_panel_info *)(lcdkit_mtk_common_panel.panel_info))->gpio_offset;
	if (g_bl_info.dual_ic)
		g_bl_info.rt8555_2_hw_en_gpio += ((struct mtk_panel_info *)(lcdkit_mtk_common_panel.panel_info))->gpio_offset;
#endif
	ret = of_property_read_u32(np, "bl_on_kernel_mdelay",
		&g_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		rt8555_err("get bl_on_kernel_mdelay dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_led_num",
		&g_bl_info.bl_led_num);
	if (ret < 0) {
		rt8555_err("get bl_led_num dts config failed\n");
		return ret;
	}
	return ret;
}

static int rt8555_2_config_write(struct rt8555_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	struct i2c_adapter *adap = NULL;
	struct i2c_msg msg = {0};
	char buf[2];
	int ret = 0;
	int i;

	if((pchip == NULL) || (reg == NULL) || (val == NULL) || (pchip->client == NULL)) {
		rt8555_err("pchip or reg or val or clinet is null pointer\n");
		return -1;
	}

	rt8555_info("rt8555_2_config_write\n");
	/* get i2c adapter */
	adap = i2c_get_adapter(g_bl_info.rt8555_2_i2c_bus_id);
	if (!adap) {
		rt8555_err("i2c device %d not found\n", g_bl_info.rt8555_2_i2c_bus_id);
		ret = -ENODEV;
		goto out;
	}
	msg.addr = pchip->client->addr;
	msg.flags = pchip->client->flags;
	msg.len = 2;
	msg.buf = buf;
	for(i = 0; i < size; i++) {
		buf[0] = reg[i];
		buf[1] = val[i];
		if (val[i] != PARSE_FAILED) {
			ret = i2c_transfer(adap, &msg, 1);
			rt8555_info("rt8555_2_config_write reg=0x%x,val=0x%x\n", buf[0], buf[1]);
		}
	}
out:
	i2c_put_adapter(adap);
	return ret;
}

static int rt8555_config_write(struct rt8555_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int i;
	int ret = 0;

	for (i = 0; i < size; i++) {
		if (val[i] != PARSE_FAILED) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			if (ret < 0) {
				rt8555_err("write reg 0x%x failed\n", reg[i]);
				goto exit;
			}
		}
	}
exit:
	return ret;
}

static int rt8555_config_read(struct rt8555_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = 0;
	unsigned int i;

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			rt8555_err("read rt8555 register 0x%x failed\n", reg[i]);
			goto exit;
		} else {
			rt8555_info("read 0x%x value = 0x%x\n", reg[i], val[i]);
		}
	}
exit:
	return ret;
}

static int rt8555_chip_init(struct rt8555_chip_data *pchip)
{
	int ret;

	rt8555_info("in\n");
	if (g_bl_info.dual_ic) {
		ret = rt8555_2_config_write(pchip, g_reg_addr, g_bl_info.reg, RT8555_RW_REG_MAX);
		if (ret < 0) {
			rt8555_err("rt8555 slave register failed\n");
		goto out;
		}
	}
	ret = rt8555_config_write(pchip, g_reg_addr,
		g_bl_info.reg, RT8555_RW_REG_MAX);
	if (ret < 0) {
		rt8555_err("rt8555 register failed\n");
		goto out;
	}
	rt8555_info("OK\n");
	return ret;
out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

static ssize_t rt8555_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct i2c_client *client = NULL;
	struct rt8555_chip_data *pchip = NULL;

	if (!buf) {
		rt8555_err("buf is null\n");
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
	ret = rt8555_config_read(pchip,
		g_reg_addr, g_bl_info.reg, RT8555_RW_REG_MAX);
	if (ret < 0) {
		rt8555_err("rt8555 config read failed\n");
		goto i2c_error;
	}
	/* transfer rt8555 regiesters config information to Application layer */
	ret = snprintf(buf, PAGE_SIZE,
		"Control_mode_0x00= 0x%x\n"
		"Current_protection_0x01 = 0x%x\n"
		"\rCurrent_setting_0x02 = 0x%x\n"
		"Voltage_setting_0x03 = 0x%x\n"
		"\rBrightness_setting_0x08 = 0x%x\n"
		"Time_control_0x09 = 0x%x\n"
		"\rMode_devision_0x0A = 0x%x\n"
		"Compensation_duty_0x0B = 0x%x\n"
		"\rClk_pfm_enable_0x0D = 0x%x\n"
		"Led_protection_0x0E = 0x%x\n"
		"\rConfig_reg_50 = 0x%x\n",
		g_bl_info.reg[0], g_bl_info.reg[1],
		g_bl_info.reg[2], g_bl_info.reg[3],
		g_bl_info.reg[4], g_bl_info.reg[5],
		g_bl_info.reg[6], g_bl_info.reg[7],
		g_bl_info.reg[8], g_bl_info.reg[9],
		g_bl_info.reg[10]);
	return ret;
i2c_error:
	ret = snprintf(buf, PAGE_SIZE, "%s: i2c access failed\n", __func__);
	return ret;
}

static ssize_t rt8555_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val;
	unsigned int reg;
	unsigned int mask;
	struct rt8555_chip_data *pchip = NULL;

	if (!buf) {
		rt8555_err("buf is null\n");
		return -1;
	}
	if (!dev) {
		rt8555_err("dev is null\n");
		return -1;
	}
	pchip = dev_get_drvdata(dev);
	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		rt8555_info("check your input\n");
		goto out_input;
	}
	rt8555_info("reg=0x%x,mask=0x%x,val=0x%x\n", reg, mask, val);
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

static DEVICE_ATTR(reg, 0644, rt8555_reg_show, rt8555_reg_store);

static struct attribute *g_rt8555_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group g_rt8555_group = {
	.attrs = g_rt8555_attributes,
};

static const struct regmap_config rt8555_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

static void rt8555_enable(void)
{
	struct i2c_adapter *adap = g_rtdev->client->adapter;
	int ret;

	mdelay(g_bl_info.bl_on_kernel_mdelay);

	/* lock at rt8555-2 */
	if (g_bl_info.dual_ic) {
		adap = i2c_get_adapter(g_bl_info.rt8555_2_i2c_bus_id);
		rt8555_debug("lock i2c-%d 100us in enable\n", g_bl_info.rt8555_2_i2c_bus_id);
	}
	if (adap != NULL)
		i2c_lock_adapter(adap);
	if (g_bl_info.rt8555_hw_en) {
		ret = gpio_request(g_bl_info.rt8555_hw_en_gpio, NULL);
		if (ret)
			rt8555_err("rt8555 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(g_bl_info.rt8555_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			rt8555_err("rt8555 set gpio output not success\n");
		gpio_set_value(g_bl_info.rt8555_hw_en_gpio, GPIO_OUT_ONE);;
		if (g_bl_info.dual_ic) {
			ret = gpio_request(g_bl_info.rt8555_2_hw_en_gpio, NULL);
			if (ret)
				rt8555_err("rt8555 Could not request  rt8555_2_hw_en_gpio\n");
			ret = gpio_direction_output(g_bl_info.rt8555_2_hw_en_gpio, GPIO_DIR_OUT);
			if (ret)
				rt8555_err("rt8555 set gpio output not success\n");
			gpio_set_value(g_bl_info.rt8555_2_hw_en_gpio, GPIO_OUT_ONE);
		}
	}
	udelay(RT8555_ENABLE_DELAY);
	if (adap != NULL)
		i2c_unlock_adapter(adap);

	/* chip initialize */
	ret = rt8555_chip_init(g_rtdev);
	if (ret < 0) {
		rt8555_err("rt8555_chip_init fail\n");
		return;
	}
	g_init_status = true;
}

static void rt8555_disable(void)
{
	struct i2c_adapter *adap = g_rtdev->client->adapter;

	/* lock at rt8555-2 */
	if (g_bl_info.dual_ic) {
		adap = i2c_get_adapter(g_bl_info.rt8555_2_i2c_bus_id);
		rt8555_debug("lock i2c-%d\n", g_bl_info.rt8555_2_i2c_bus_id);
	}
	if (adap != NULL)
		i2c_lock_adapter(adap);
	if (g_bl_info.rt8555_hw_en) {
		gpio_set_value(g_bl_info.rt8555_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(g_bl_info.rt8555_hw_en_gpio);
		if (g_bl_info.dual_ic) {
			gpio_set_value(g_bl_info.rt8555_2_hw_en_gpio, GPIO_OUT_ZERO);
			gpio_free(g_bl_info.rt8555_2_hw_en_gpio);
		}
	}
	msleep(RT8555_DISABLE_DELAY);
	g_init_status = false;
	if (adap != NULL)
		i2c_unlock_adapter(adap);
}

static int rt8555_set_backlight(uint32_t bl_level)
{
	int ret = 0;
	int slope_addr;

	if (!g_rtdev) {
		rt8555_err("rt8555_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(g_rtdev->test_sem))) {
		rt8555_info("Now in test mode\n");
		return 0;
	}
	rt8555_info("rt8555_en_backlight bl_level=%d\n", bl_level);

	if ((g_bl_info.bl_set_long_slope) && (bl_set_slope_status == true)) {
		ret = regmap_write(g_rtdev->regmap,
			RT8555_BRIGHTNESS_SETTING_ADDR, g_bl_info.bl_set_long_slope);
		/* communicate with rt8555-2 */
		if (g_bl_info.dual_ic) {
			slope_addr = RT8555_BRIGHTNESS_SETTING_ADDR;
			ret = rt8555_2_config_write(g_rtdev, &slope_addr, &g_bl_info.bl_set_long_slope, 1);
			if (ret < 0)
				rt8555_err("rt8555-2 set slope time failed\n");
		}
		rt8555_info("set slope time to long 0x%x\n", g_bl_info.bl_set_long_slope);
		bl_set_slope_status = false;
	}

	/* first set backlight, enable rt8555 */
	if ((g_init_status == false) && (bl_level > 0)) {
		rt8555_enable();
		bl_set_slope_status = true;
	}

	/* if set backlight level 0, disable rt8555 */
	if ((g_init_status == true) && (bl_level == 0)) {
		rt8555_disable();
		bl_set_slope_status = false;
	}
	up(&(g_rtdev->test_sem));
	return ret;
}

static int rt8555_en_backlight(uint32_t bl_level)
{
	int ret = 0;
	int slope_addr;

	if (!g_rtdev) {
		rt8555_err("rt8555_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(g_rtdev->test_sem))) {
		rt8555_info("Now in test mode\n");
		return 0;
	}
	rt8555_info("rt8555_en_backlight bl_level=%d\n", bl_level);

	if ((g_bl_info.bl_set_long_slope) && (bl_set_slope_status == true)) {
		ret = regmap_write(g_rtdev->regmap,
			RT8555_BRIGHTNESS_SETTING_ADDR, g_bl_info.bl_set_long_slope);
		/* communicate with rt8555-2 */
		if (g_bl_info.dual_ic) {
			slope_addr = RT8555_BRIGHTNESS_SETTING_ADDR;
			ret = rt8555_2_config_write(g_rtdev, &slope_addr, &g_bl_info.bl_set_long_slope, 1);
			if (ret < 0)
				rt8555_err("rt8555-2 set slope time failed\n");
		}
		rt8555_info("set slope time to long 0x%x\n", g_bl_info.bl_set_long_slope);
		bl_set_slope_status = false;
	}

	/* first set backlight, enable rt8555 */
	if ((g_init_status == false) && (bl_level > 0)) {
		rt8555_enable();
		bl_set_slope_status = true;
	}

	/* if set backlight level 0, disable rt8555 */
	if ((g_init_status == true) && (bl_level == 0)) {
		rt8555_disable();
		bl_set_slope_status = false;
	}
	up(&(g_rtdev->test_sem));
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = rt8555_set_backlight,
	.en_backlight = rt8555_en_backlight,
	.name = "8555",
};

static int rt8555_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct rt8555_chip_data *pchip = NULL;
	struct i2c_adapter *adapter = client->adapter;

	rt8555_info("in\n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}
	pchip = devm_kzalloc(&client->dev,
		sizeof(struct rt8555_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;
#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &rt8555_regmap);
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
	g_rt_class = class_create(THIS_MODULE, "rt8555");
	if (IS_ERR(g_rt_class)) {
		rt8555_err("Create rt8555 class failed; errno = %ld\n",
			PTR_ERR(g_rt_class));
		g_rt_class = NULL;
	}
	pchip->dev = device_create(g_rt_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		rt8555_err("Create device failed; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &g_rt8555_group);
		if (ret)
			goto err_sysfs;
	}
	memset(&g_bl_info, 0, sizeof(struct rt8555_backlight_info));
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_RT8555);
	if (!np) {
		rt8555_err("NOT FOUND device node %s\n", DTS_COMP_RT8555);
		goto err_sysfs;
	}
	ret = rt8555_parse_dts(np);
	if (ret < 0) {
		rt8555_err("parse rt8555 dts failed\n");
		goto err_sysfs;
	}
#ifdef CONFIG_LCD_KIT_DRIVER
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_RT8555);
	if (!np) {
		rt8555_err("NOT FOUND device node %s\n", DTS_COMP_RT8555);
		goto err_sysfs;
	}
	/* Only testing rt8555 used */
	ret = regmap_read(pchip->regmap, g_reg_addr[0],
		&g_bl_info.reg[0]);
	if (ret < 0) {
		rt8555_err("rt8555 not used\n");
		goto err_sysfs;
	}
	/* Testing rt8555-2 used */
	if (g_bl_info.dual_ic) {
		ret = rt8555_2_config_write(pchip, g_reg_addr, g_bl_info.reg, 1);
		if (ret < 0) {
			rt8555_err("rt8555 slave not used\n");
			goto err_sysfs;
		}
	}
	ret = of_property_read_u32(np, "rt8555_level_lsb",
		&g_bl_info.rt8555_level_lsb);
	if (ret < 0) {
		rt8555_err("get rt8555_level_lsb failed\n");
		goto err_sysfs;
	}

	ret = of_property_read_u32(np, "rt8555_level_msb",
		&g_bl_info.rt8555_level_msb);
	if (ret < 0) {
		rt8555_err("get rt8555_level_msb failed\n");
		goto err_sysfs;
	}
	lcd_kit_bl_register(&bl_ops);
#endif
	return ret;
err_sysfs:
	rt8555_err("sysfs error\n");
	device_destroy(g_rt_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int rt8555_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &g_rt8555_group);
	return 0;
}

static const struct i2c_device_id rt8555_id[] = {
	{ RT8555_NAME, 0 },
	{},
};

static const struct of_device_id rt8555_of_id_table[] = {
	{ .compatible = "realtek,rt8555" },
	{},
};

MODULE_DEVICE_TABLE(i2c, rt8555_id);
static struct i2c_driver rt8555_i2c_driver = {
	.driver = {
		.name = "rt8555",
		.owner = THIS_MODULE,
		.of_match_table = rt8555_of_id_table,
	},
	.probe = rt8555_probe,
	.remove = rt8555_remove,
	.id_table = rt8555_id,
};

static int __init rt8555_module_init(void)
{
	return i2c_add_driver(&rt8555_i2c_driver);
}
static void __exit rt8555_module_exit(void)
{
	i2c_del_driver(&rt8555_i2c_driver);
}

late_initcall(rt8555_module_init);
module_exit(rt8555_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Backlight driver for rt8555");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
