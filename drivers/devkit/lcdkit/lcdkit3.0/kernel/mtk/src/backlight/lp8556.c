/*
* Simple driver
* Copyright (C)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*/

#include "lp8556.h"
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

static struct lp8556_backlight_information lp8556_bl_info;

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
static bool lp8556_init_status = true;
#ifndef CONFIG_DRM_MEDIATEK
extern struct LCM_DRIVER lcdkit_mtk_common_panel;
#endif

/*
** for debug, S_IRUGO
** /sys/module/hisifb/parameters
*/
unsigned lp8556_msg_level = 7;
module_param_named(debug_lp8556_msg_level, lp8556_msg_level, int, 0640);
MODULE_PARM_DESC(debug_lp8556_msg_level, "backlight lp8556 msg level");

static int lp8556_parse_dts(struct device_node *np)
{
	int ret;
	int i;
	struct mtk_panel_info *plcd_kit_info = NULL;

	if(np == NULL){
		lp8556_err("np is null pointer\n");
		return -1;
	}

	for (i = 0;i < LP8556_RW_REG_MAX;i++ ) {
		ret = of_property_read_u32(np, lp8556_dts_string[i],
			&lp8556_bl_info.lp8556_reg[i]);
		if (ret < 0) {
			//init to invalid data
			lp8556_bl_info.lp8556_reg[i] = 0xffff;
			lp8556_info("can not find config:%s\n", lp8556_dts_string[i]);
		}
	}
	ret = of_property_read_u32(np, "dual_ic", &lp8556_bl_info.dual_ic);
	if (ret < 0) {
		lp8556_info("can not get dual_ic dts node\n");
	}
	else {
		ret = of_property_read_u32(np, "lp8556_2_i2c_bus_id",
			&lp8556_bl_info.lp8556_2_i2c_bus_id);
		if (ret < 0)
			lp8556_info("can not get lp8556_2_i2c_bus_id dts node\n");
	}
	ret = of_property_read_u32(np, "lp8556_hw_enable",
		&lp8556_bl_info.lp8556_hw_en);
	if (ret < 0) {
		lp8556_err("get lp8556_hw_en dts config failed\n");
		lp8556_bl_info.lp8556_hw_en = 0;
	}
	if (lp8556_bl_info.lp8556_hw_en != 0) {
		ret = of_property_read_u32(np, "lp8556_hw_en_gpio",
			&lp8556_bl_info.lp8556_hw_en_gpio);
		if (ret < 0) {
			lp8556_err("get lp8556_hw_en_gpio dts config failed\n");
			return ret;
		}
		if (lp8556_bl_info.dual_ic) {
			ret = of_property_read_u32(np, "lp8556_2_hw_en_gpio",
				&lp8556_bl_info.lp8556_2_hw_en_gpio);
			if (ret < 0) {
				lp8556_err("get lp8556_2_hw_en_gpio dts config failed\n");
				return ret;
			}
		}
	}
	/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
	plcd_kit_info = lcm_get_panel_info();
	if (plcd_kit_info != NULL) {
		lp8556_bl_info.lp8556_hw_en_gpio += plcd_kit_info->gpio_offset;
		if (lp8556_bl_info.dual_ic)
			lp8556_bl_info.lp8556_2_hw_en_gpio += plcd_kit_info->gpio_offset;
	}
#else
	lp8556_bl_info.lp8556_hw_en_gpio += ((struct mtk_panel_info *)(lcdkit_mtk_common_panel.panel_info))->gpio_offset;
	if (lp8556_bl_info.dual_ic)
		lp8556_bl_info.lp8556_2_hw_en_gpio += ((struct mtk_panel_info *)(lcdkit_mtk_common_panel.panel_info))->gpio_offset;
#endif
	ret = of_property_read_u32(np, "bl_on_kernel_mdelay",
		&lp8556_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		lp8556_err("get bl_on_kernel_mdelay dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_led_num",
		&lp8556_bl_info.bl_led_num);
	if (ret < 0) {
		lp8556_err("get bl_led_num dts config failed\n");
		return ret;
	}

	return ret;
}

static int lp8556_2_config_write(struct lp8556_chip_data *pchip,
			unsigned int reg[], unsigned int val[], unsigned int size)
{
	struct i2c_adapter *adap = NULL;
	struct i2c_msg msg = {0};
	char buf[2];
	int ret;
	int i;

	if((pchip == NULL) || (reg == NULL) || (val == NULL) || (pchip->client == NULL)) {
		lp8556_err("pchip or reg or val is null pointer\n");
		return -1;
	}
	lp8556_info("lp8556_2_config_write\n");
	/* get i2c adapter */
	adap = i2c_get_adapter(lp8556_bl_info.lp8556_2_i2c_bus_id);
	if (!adap) {
		lp8556_err("i2c device %d not found\n", lp8556_bl_info.lp8556_2_i2c_bus_id);
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
		if (val[i] != 0xffff) {
			ret = i2c_transfer(adap, &msg, 1);
			lp8556_info("lp8556_2_config_write reg=0x%x,val=0x%x\n", buf[0], buf[1]);
		}
	}
out:
	i2c_put_adapter(adap);
	return ret;
}

static int lp8556_config_write(struct lp8556_chip_data *pchip,
			unsigned int reg[],unsigned int val[],unsigned int size)
{
	int ret = 0;
	unsigned int i = 0;

	if((pchip == NULL) || (reg == NULL) || (val == NULL)){
		lp8556_err("pchip or reg or val is null pointer\n");
		return -1;
	}
	for(i = 0;i < size;i++) {
		/*judge reg is invalid*/
		if (val[i] != 0xffff) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			if (ret < 0) {
				lp8556_err("write lp8556 backlight config register 0x%x failed\n",reg[i]);
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

	if((pchip == NULL) || (reg == NULL) || (val == NULL)){
		lp8556_err("pchip or reg or val is null pointer\n");
		return -1;
	}

	for(i = 0;i < size;i++) {
		ret = regmap_read(pchip->regmap, reg[i],&val[i]);
		if (ret < 0) {
			lp8556_err("read lp8556 backlight config register 0x%x failed",reg[i]);
			goto exit;
		} else {
			lp8556_info("read 0x%x value = 0x%x\n", reg[i], val[i]);
		}
	}

exit:
	return ret;
}

/* initialize chip */
static int lp8556_chip_init(struct lp8556_chip_data *pchip)
{
	int ret = -1;

	lp8556_info("in!\n");

	if(pchip == NULL){
		lp8556_err("pchip is null pointer\n");
		return -1;
	}
	if (lp8556_bl_info.dual_ic) {
		ret = lp8556_2_config_write(pchip, lp8556_reg_addr, lp8556_bl_info.lp8556_reg, LP8556_RW_REG_MAX);
		if (ret < 0) {
			lp8556_err("lp8556 slave config register failed\n");
		goto out;
		}
	}
	ret = lp8556_config_write(pchip, lp8556_reg_addr, lp8556_bl_info.lp8556_reg, LP8556_RW_REG_MAX);
	if (ret < 0) {
		lp8556_err("lp8556 config register failed");
		goto out;
	}
	lp8556_info("ok!\n");
	return ret;

out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

/*
 * lp8556_set_reg(): Set lp8556 reg
 *
 * @bl_reg: which reg want to write
 * @bl_mask: which bits of reg want to change
 * @bl_val: what value want to write to the reg
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
ssize_t lp8556_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val)
{
	ssize_t ret = -1;
	u8 reg = bl_reg;
	u8 mask = bl_mask;
	u8 val = bl_val;

	if (!lp8556_init_status) {
		lp8556_err("init fail, return.\n");
		return ret;
	}

	if (reg < REG_MAX) {
		lp8556_err("Invalid argument!!!\n");
		return ret;
	}

	lp8556_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask,
		val);

	ret = regmap_update_bits(lp8556_g_chip->regmap, reg, mask, val);
	if (ret < 0) {
		lp8556_err("i2c access fail to register\n");
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(lp8556_set_reg);

static ssize_t lp8556_reg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lp8556_chip_data *pchip = NULL;
	struct i2c_client *client = NULL;
	ssize_t ret = -1;

	if (!buf) {
		lp8556_err("buf is null\n");
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
		lp8556_err("lp8556 config read failed");
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
					struct device_attribute *dev_attr,
					const char *buf, size_t size)
{
	ssize_t ret;
	struct lp8556_chip_data *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;

	if (!buf) {
		lp8556_err("buf is null\n");
		return -1;
	}

	if (!dev) {
		lp8556_err("dev is null\n");
		return -1;
	}

	pchip = dev_get_drvdata(dev);
	if(!pchip){
		lp8556_err("pchip is null\n");
		return -1;
	}

	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		lp8556_info("check your input!!!\n");
		goto out_input;
	}

	lp8556_info("%s:reg=0x%x,mask=0x%x,val=0x%x\n", __func__, reg, mask, val);

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

static void lp8556_enable(void)
{
	int ret;

	if (lp8556_bl_info.lp8556_hw_en) {
		ret = gpio_request(lp8556_bl_info.lp8556_hw_en_gpio, NULL);
		if (ret)
			lp8556_err("lp8556 Could not request  hw_en_gpio\n");
		ret = gpio_direction_output(lp8556_bl_info.lp8556_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			lp8556_err("lp8556 set gpio output not success\n");
		gpio_set_value(lp8556_bl_info.lp8556_hw_en_gpio, GPIO_OUT_ONE);
		if (lp8556_bl_info.dual_ic) {
			ret = gpio_request(lp8556_bl_info.lp8556_2_hw_en_gpio, NULL);
			if (ret)
				lp8556_err("lp8556 Could not request  hw_en2_gpio\n");
			ret = gpio_direction_output(lp8556_bl_info.lp8556_2_hw_en_gpio, GPIO_DIR_OUT);
			if (ret)
				lp8556_err("lp8556 set gpio output not success\n");
			gpio_set_value(lp8556_bl_info.lp8556_2_hw_en_gpio, GPIO_OUT_ONE);
		if (lp8556_bl_info.bl_on_kernel_mdelay)
			mdelay(lp8556_bl_info.bl_on_kernel_mdelay);
		}
	}
	/* chip initialize */
	ret = lp8556_chip_init(lp8556_g_chip);
	if (ret < 0) {
		lp8556_err("lp8556_chip_init fail!\n");
		return;
	}
	lp8556_init_status = true;
}

static void lp8556_disable(void)
{
	if (lp8556_bl_info.lp8556_hw_en) {
		gpio_set_value(lp8556_bl_info.lp8556_hw_en_gpio, GPIO_OUT_ZERO);
		gpio_free(lp8556_bl_info.lp8556_hw_en_gpio);
		if (lp8556_bl_info.dual_ic) {
			gpio_set_value(lp8556_bl_info.lp8556_2_hw_en_gpio, GPIO_OUT_ZERO);
			gpio_free(lp8556_bl_info.lp8556_2_hw_en_gpio);
		}
	}
	lp8556_init_status = false;
}

static int lp8556_set_backlight(uint32_t bl_level)
{
	static int last_bl_level = 0;
	int bl_msb = 0;
	int bl_lsb = 0;
	int ret = 0;

	if (!lp8556_g_chip) {
		lp8556_err("lp8556_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(lp8556_g_chip->test_sem))) {
		lp8556_info("Now in test mode\n");
		return 0;
	}
	/*first set backlight, enable lp8556*/
	if (false == lp8556_init_status && bl_level > 0)
		lp8556_enable();

	/*set backlight level*/
	bl_msb = (bl_level >> 8) & 0x0F;
	bl_lsb = bl_level & 0xFF;
	ret = regmap_write(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_lsb, bl_lsb);
	if (ret < 0)
		lp8556_debug("write lp8556 backlight level lsb:0x%x failed\n", bl_lsb);
	ret = regmap_write(lp8556_g_chip->regmap, lp8556_bl_info.lp8556_level_msb, bl_msb);
	if (ret < 0)
		lp8556_debug("write lp8556 backlight level msb:0x%x failed\n", bl_msb);

	/*if set backlight level 0, disable lp8556*/
	if (true == lp8556_init_status && 0 == bl_level)
		lp8556_disable();
	up(&(lp8556_g_chip->test_sem));
	last_bl_level = bl_level;
	return ret;
}

static int lp8556_en_backlight(uint32_t bl_level)
{
	static int last_bl_level = 0;
	int ret = 0;

	if (!lp8556_g_chip) {
		lp8556_err("lp8556_g_chip is null\n");
		return -1;
	}
	if (down_trylock(&(lp8556_g_chip->test_sem))) {
		lp8556_info("Now in test mode\n");
		return 0;
	}
	lp8556_info("lp8556_en_backlight bl_level=%d\n", bl_level);
	/*first set backlight, enable lp8556*/
	if (false == lp8556_init_status && bl_level > 0)
		lp8556_enable();

	/*if set backlight level 0, disable lp8556*/
	if (true == lp8556_init_status && 0 == bl_level)
		lp8556_disable();
	up(&(lp8556_g_chip->test_sem));
	last_bl_level = bl_level;
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = lp8556_set_backlight,
	.en_backlight = lp8556_en_backlight,
	.name = "8556",
};

static int lp8556_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct lp8556_chip_data *pchip = NULL;
	int ret = -1;
	struct device_node *np = NULL;

	lp8556_info("in!\n");

	if(!client){
		lp8556_err("client is null pointer\n");
		return -1;
	}
	adapter = client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail.\n");
		return -EOPNOTSUPP;
	}

	pchip = devm_kzalloc(&client->dev,
				sizeof(struct lp8556_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_I2C
	pchip->regmap = devm_regmap_init_i2c(client, &lp8556_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "fail : allocate register map: %d\n", ret);
		goto err_out;
	}
#endif

	lp8556_g_chip = pchip;
	pchip->client = client;
	i2c_set_clientdata(client, pchip);

	sema_init(&(pchip->test_sem), 1);

	pchip->dev = device_create(lp8556_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		lp8556_err("Unable to create device; errno = %ld\n", PTR_ERR(pchip->dev));
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
		lp8556_err("NOT FOUND device node %s!\n", DTS_COMP_LP8556);
		goto err_sysfs;
	}

	ret = lp8556_parse_dts(np);
	if (ret < 0) {
		lp8556_err("parse lp8556 dts failed");
		goto err_sysfs;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LP8556);
	if (!np) {
		lp8556_err("NOT FOUND device node %s!\n", DTS_COMP_LP8556);
		goto err_sysfs;
	}
	/* Only testing lp8556 used */
	ret = regmap_read(pchip->regmap,
		lp8556_reg_addr[0], &lp8556_bl_info.lp8556_reg[0]);
	if (ret < 0) {
		lp8556_err("lp8556 not used\n");
		goto err_sysfs;
	}
	/* Testing lp8556-2 used */
	if (lp8556_bl_info.dual_ic) {
		ret = lp8556_2_config_write(pchip, lp8556_reg_addr, lp8556_bl_info.lp8556_reg, 1);
		if (ret < 0) {
			lp8556_err("lp8556 slave not used\n");
			goto err_sysfs;
		}
	}
	ret = of_property_read_u32(np, "lp8556_level_lsb", &lp8556_bl_info.lp8556_level_lsb);
	if (ret < 0) {
		lp8556_err("get lp8556_level_lsb failed\n");
		goto err_sysfs;
	}

	ret = of_property_read_u32(np, "lp8556_level_msb", &lp8556_bl_info.lp8556_level_msb);
	if (ret < 0) {
		lp8556_err("get lp8556_level_msb failed\n");
		goto err_sysfs;
	}
	lcd_kit_bl_register(&bl_ops);

	return ret;

err_sysfs:
	lp8556_debug("sysfs error!\n");
	device_destroy(lp8556_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int lp8556_remove(struct i2c_client *client)
{
	if(!client){
		lp8556_err("client is null pointer\n");
		return -1;
	}

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

	lp8556_info("in!\n");

	lp8556_class = class_create(THIS_MODULE, "lp8556");
	if (IS_ERR(lp8556_class)) {
		lp8556_err("Unable to create lp8556 class; errno = %ld\n", PTR_ERR(lp8556_class));
		lp8556_class = NULL;
	}

	ret = i2c_add_driver(&lp8556_i2c_driver);
	if (ret)
		lp8556_err("Unable to register lp8556 driver\n");

	lp8556_info("ok!\n");

	return ret;
}
static void __exit lp8556_module_exit(void)
{
	i2c_del_driver(&lp8556_i2c_driver);
}

late_initcall(lp8556_module_init);
module_exit(lp8556_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Backlight driver for lp8556");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
