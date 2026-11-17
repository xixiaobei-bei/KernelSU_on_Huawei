/*
 * lcd_kit_backlight.c
 *
 * lcdkit backlgiht function for lcd backlight ic driver
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

#include "lcd_kit_backlight.h"
#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_drm_panel.h"
#include "lcd_kit_bl.h"
#include "lcd_kit_bias.h"
#include "backlight_linear_to_exp.h"
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#define LCD_BACKLIGHT_BASE 10
#define LCD_BACKLIGHT_IC_COMP_LEN 128
#define BACKLIGHT_IC_CMD_LEN 5
#define BACKLIGHT_BL_CMD_LEN 4
#define DEFAULT_VOL_VAL 0x0F
#define BACKLIGHT_TRUE 1
#define BACKLIGHT_FALSE 0
#define BACKLIGHT_GPIO_DIR_OUT 1
#define BACKLIGHT_GPIO_OUT_ONE 1
#define BACKLIGHT_GPIO_OUT_ZERO 0

static struct lcd_kit_backlight_device *pbacklight_dev = NULL;
static char backlight_ic_name[LCD_BACKLIGHT_IC_NAME_LEN] = "default";
struct class *backlight_class = NULL;
static bool g_vol_mapped = false;
static int g_sysfs_created;
static bool backlight_init_status = false;
static unsigned int reg = 0x00;
static struct i2c_adapter *g_backlight_dual_ic_adap = NULL;
static struct backlight_bias_ic_voltage common_vol_table[] = {
	{ LCD_BACKLIGHT_VOL_400, COMMON_VOL_400 },
	{ LCD_BACKLIGHT_VOL_405, COMMON_VOL_405 },
	{ LCD_BACKLIGHT_VOL_410, COMMON_VOL_410 },
	{ LCD_BACKLIGHT_VOL_415, COMMON_VOL_415 },
	{ LCD_BACKLIGHT_VOL_420, COMMON_VOL_420 },
	{ LCD_BACKLIGHT_VOL_425, COMMON_VOL_425 },
	{ LCD_BACKLIGHT_VOL_430, COMMON_VOL_430 },
	{ LCD_BACKLIGHT_VOL_430, COMMON_VOL_435 },
	{ LCD_BACKLIGHT_VOL_440, COMMON_VOL_440 },
	{ LCD_BACKLIGHT_VOL_445, COMMON_VOL_445 },
	{ LCD_BACKLIGHT_VOL_450, COMMON_VOL_450 },
	{ LCD_BACKLIGHT_VOL_455, COMMON_VOL_455 },
	{ LCD_BACKLIGHT_VOL_460, COMMON_VOL_460 },
	{ LCD_BACKLIGHT_VOL_465, COMMON_VOL_465 },
	{ LCD_BACKLIGHT_VOL_470, COMMON_VOL_470 },
	{ LCD_BACKLIGHT_VOL_475, COMMON_VOL_475 },
	{ LCD_BACKLIGHT_VOL_480, COMMON_VOL_480 },
	{ LCD_BACKLIGHT_VOL_485, COMMON_VOL_485 },
	{ LCD_BACKLIGHT_VOL_490, COMMON_VOL_490 },
	{ LCD_BACKLIGHT_VOL_495, COMMON_VOL_495 },
	{ LCD_BACKLIGHT_VOL_500, COMMON_VOL_500 },
	{ LCD_BACKLIGHT_VOL_505, COMMON_VOL_505 },
	{ LCD_BACKLIGHT_VOL_510, COMMON_VOL_510 },
	{ LCD_BACKLIGHT_VOL_515, COMMON_VOL_515 },
	{ LCD_BACKLIGHT_VOL_520, COMMON_VOL_520 },
	{ LCD_BACKLIGHT_VOL_525, COMMON_VOL_525 },
	{ LCD_BACKLIGHT_VOL_530, COMMON_VOL_530 },
	{ LCD_BACKLIGHT_VOL_535, COMMON_VOL_535 },
	{ LCD_BACKLIGHT_VOL_540, COMMON_VOL_540 },
	{ LCD_BACKLIGHT_VOL_545, COMMON_VOL_545 },
	{ LCD_BACKLIGHT_VOL_550, COMMON_VOL_550 },
	{ LCD_BACKLIGHT_VOL_555, COMMON_VOL_555 },
	{ LCD_BACKLIGHT_VOL_560, COMMON_VOL_560 },
	{ LCD_BACKLIGHT_VOL_565, COMMON_VOL_565 },
	{ LCD_BACKLIGHT_VOL_570, COMMON_VOL_570 },
	{ LCD_BACKLIGHT_VOL_575, COMMON_VOL_575 },
	{ LCD_BACKLIGHT_VOL_580, COMMON_VOL_580 },
	{ LCD_BACKLIGHT_VOL_585, COMMON_VOL_585 },
	{ LCD_BACKLIGHT_VOL_590, COMMON_VOL_590 },
	{ LCD_BACKLIGHT_VOL_600, COMMON_VOL_600 },
	{ LCD_BACKLIGHT_VOL_605, COMMON_VOL_605 },
	{ LCD_BACKLIGHT_VOL_610, COMMON_VOL_610 },
	{ LCD_BACKLIGHT_VOL_615, COMMON_VOL_615 },
	{ LCD_BACKLIGHT_VOL_620, COMMON_VOL_620 },
	{ LCD_BACKLIGHT_VOL_625, COMMON_VOL_625 },
	{ LCD_BACKLIGHT_VOL_630, COMMON_VOL_630 },
	{ LCD_BACKLIGHT_VOL_635, COMMON_VOL_635 },
	{ LCD_BACKLIGHT_VOL_640, COMMON_VOL_640 },
	{ LCD_BACKLIGHT_VOL_645, COMMON_VOL_645 },
	{ LCD_BACKLIGHT_VOL_650, COMMON_VOL_650 },
	{ LCD_BACKLIGHT_VOL_655, COMMON_VOL_655 },
	{ LCD_BACKLIGHT_VOL_660, COMMON_VOL_660 },
	{ LCD_BACKLIGHT_VOL_665, COMMON_VOL_665 },
	{ LCD_BACKLIGHT_VOL_670, COMMON_VOL_670 },
	{ LCD_BACKLIGHT_VOL_675, COMMON_VOL_675 },
	{ LCD_BACKLIGHT_VOL_680, COMMON_VOL_680 },
	{ LCD_BACKLIGHT_VOL_685, COMMON_VOL_685 },
	{ LCD_BACKLIGHT_VOL_690, COMMON_VOL_690 },
	{ LCD_BACKLIGHT_VOL_695, COMMON_VOL_695 },
	{ LCD_BACKLIGHT_VOL_700, COMMON_VOL_700 },
	{ LCD_BACKLIGHT_VOL_705, COMMON_VOL_705 },
	{ LCD_BACKLIGHT_VOL_710, COMMON_VOL_710 },
	{ LCD_BACKLIGHT_VOL_715, COMMON_VOL_715 }
};

/* backlight bias ic: ktz8864 */
static struct backlight_bias_ic_voltage ktz8864_vol_table[] = {
	{ LCD_BACKLIGHT_VOL_400, KTZ8864_VOL_400 },
	{ LCD_BACKLIGHT_VOL_405, KTZ8864_VOL_405 },
	{ LCD_BACKLIGHT_VOL_410, KTZ8864_VOL_410 },
	{ LCD_BACKLIGHT_VOL_415, KTZ8864_VOL_415 },
	{ LCD_BACKLIGHT_VOL_420, KTZ8864_VOL_420 },
	{ LCD_BACKLIGHT_VOL_425, KTZ8864_VOL_425 },
	{ LCD_BACKLIGHT_VOL_430, KTZ8864_VOL_430 },
	{ LCD_BACKLIGHT_VOL_430, KTZ8864_VOL_435 },
	{ LCD_BACKLIGHT_VOL_440, KTZ8864_VOL_440 },
	{ LCD_BACKLIGHT_VOL_445, KTZ8864_VOL_445 },
	{ LCD_BACKLIGHT_VOL_450, KTZ8864_VOL_450 },
	{ LCD_BACKLIGHT_VOL_455, KTZ8864_VOL_455 },
	{ LCD_BACKLIGHT_VOL_460, KTZ8864_VOL_460 },
	{ LCD_BACKLIGHT_VOL_465, KTZ8864_VOL_465 },
	{ LCD_BACKLIGHT_VOL_470, KTZ8864_VOL_470 },
	{ LCD_BACKLIGHT_VOL_475, KTZ8864_VOL_475 },
	{ LCD_BACKLIGHT_VOL_480, KTZ8864_VOL_480 },
	{ LCD_BACKLIGHT_VOL_485, KTZ8864_VOL_485 },
	{ LCD_BACKLIGHT_VOL_490, KTZ8864_VOL_490 },
	{ LCD_BACKLIGHT_VOL_495, KTZ8864_VOL_495 },
	{ LCD_BACKLIGHT_VOL_500, KTZ8864_VOL_500 },
	{ LCD_BACKLIGHT_VOL_505, KTZ8864_VOL_505 },
	{ LCD_BACKLIGHT_VOL_510, KTZ8864_VOL_510 },
	{ LCD_BACKLIGHT_VOL_515, KTZ8864_VOL_515 },
	{ LCD_BACKLIGHT_VOL_520, KTZ8864_VOL_520 },
	{ LCD_BACKLIGHT_VOL_525, KTZ8864_VOL_525 },
	{ LCD_BACKLIGHT_VOL_530, KTZ8864_VOL_530 },
	{ LCD_BACKLIGHT_VOL_535, KTZ8864_VOL_535 },
	{ LCD_BACKLIGHT_VOL_540, KTZ8864_VOL_540 },
	{ LCD_BACKLIGHT_VOL_545, KTZ8864_VOL_545 },
	{ LCD_BACKLIGHT_VOL_550, KTZ8864_VOL_550 },
	{ LCD_BACKLIGHT_VOL_555, KTZ8864_VOL_555 },
	{ LCD_BACKLIGHT_VOL_560, KTZ8864_VOL_560 },
	{ LCD_BACKLIGHT_VOL_565, KTZ8864_VOL_565 },
	{ LCD_BACKLIGHT_VOL_570, KTZ8864_VOL_570 },
	{ LCD_BACKLIGHT_VOL_575, KTZ8864_VOL_575 },
	{ LCD_BACKLIGHT_VOL_580, KTZ8864_VOL_580 },
	{ LCD_BACKLIGHT_VOL_585, KTZ8864_VOL_585 },
	{ LCD_BACKLIGHT_VOL_590, KTZ8864_VOL_590 },
	{ LCD_BACKLIGHT_VOL_600, KTZ8864_VOL_600 },
	{ LCD_BACKLIGHT_VOL_605, KTZ8864_VOL_605 },
	{ LCD_BACKLIGHT_VOL_610, KTZ8864_VOL_610 },
	{ LCD_BACKLIGHT_VOL_615, KTZ8864_VOL_615 },
	{ LCD_BACKLIGHT_VOL_620, KTZ8864_VOL_620 },
	{ LCD_BACKLIGHT_VOL_625, KTZ8864_VOL_625 },
	{ LCD_BACKLIGHT_VOL_630, KTZ8864_VOL_630 },
	{ LCD_BACKLIGHT_VOL_635, KTZ8864_VOL_635 },
	{ LCD_BACKLIGHT_VOL_640, KTZ8864_VOL_640 },
	{ LCD_BACKLIGHT_VOL_645, KTZ8864_VOL_645 },
	{ LCD_BACKLIGHT_VOL_650, KTZ8864_VOL_650 },
	{ LCD_BACKLIGHT_VOL_655, KTZ8864_VOL_655 },
	{ LCD_BACKLIGHT_VOL_660, KTZ8864_VOL_660 }
};

static struct backlight_bias_ic_config backlight_bias_config[] = {
	{ "hw_lm36274",
	ARRAY_SIZE(common_vol_table),
	&common_vol_table[0] },
	{ "hw_nt50356",
	ARRAY_SIZE(common_vol_table),
	&common_vol_table[0] },
	{ "hw_rt4831",
	ARRAY_SIZE(common_vol_table),
	&common_vol_table[0] },
	{ "hw_ktz8864",
	ARRAY_SIZE(ktz8864_vol_table),
	&ktz8864_vol_table[0] }
};
static int lcd_kit_backlight_read_byte(struct lcd_kit_backlight_device *pchip,
	unsigned char reg, unsigned char *pdata)
{
	int ret;

	ret = i2c_smbus_read_byte_data(pchip->client, reg);
	if (ret < 0) {
		dev_err(&pchip->client->dev, "failed to read 0x%x\n", reg);
		return ret;
	}
	*pdata = (unsigned char)ret;

	return ret;
}

static int lcd_kit_backlight_write_byte(struct lcd_kit_backlight_device *pchip,
	unsigned char reg, unsigned char data)
{
	int ret;

	ret = i2c_smbus_write_byte_data(pchip->client, reg, data);
	if (ret < 0)
		dev_err(&pchip->client->dev, "failed to write 0x%.2x\n", reg);
	return ret;
}

static int lcd_kit_backlight_update_bit(struct lcd_kit_backlight_device *pchip,
	unsigned char reg, unsigned char mask, unsigned char data)
{
	int ret;
	unsigned char tmp = 0;

	ret = lcd_kit_backlight_read_byte(pchip, reg, &tmp);
	if (ret < 0) {
		dev_err(&pchip->client->dev, "failed to read 0x%.2x\n", reg);
		return ret;
	}

	tmp = (unsigned char)ret;
	tmp &= ~mask;
	tmp |= data & mask;

	return lcd_kit_backlight_write_byte(pchip, reg, tmp);
}

static ssize_t backlight_reg_addr_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	unsigned int reg_addr = 0;
	int ret;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return size;
	}
	ret = sscanf(buf, "reg=0x%x",&reg_addr);
	if (ret < 0) {
		LCD_KIT_ERR("check input fail\n");
		return -EINVAL;
	}
	reg = reg_addr;
	return size;
}

static DEVICE_ATTR(reg_addr, S_IRUGO|S_IWUSR, NULL, backlight_reg_addr_store);

static ssize_t backlight_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	int ret = 0;
	unsigned int val = 0;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL))
		return snprintf(buf, PAGE_SIZE, "input invalid\n");

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return snprintf(buf, PAGE_SIZE, "driver data is null\n");

	ret = i2c_smbus_read_byte_data(pchip->client, reg);
	if (ret < 0)
		goto i2c_error;
	val = ret;
	return snprintf(buf, PAGE_SIZE, "value = 0x%x\n", val);

i2c_error:
	return snprintf(buf, PAGE_SIZE, "backlight i2c read register error\n");
}

static ssize_t backlight_reg_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int val = 0;
	int ret = 0;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return size;
	}
	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		LCD_KIT_ERR("check input\n");
		return -EINVAL;
	}
	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return -EINVAL;

	ret = lcd_kit_backlight_update_bit(pchip, reg, mask, val);
	if (ret < 0) {
		LCD_KIT_ERR("backlight i2c update register error\n");
		return ret;
	}
	return size;
}

static DEVICE_ATTR(reg, S_IRUGO|S_IWUSR, backlight_reg_show, backlight_reg_store);

static ssize_t backlight_bl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	int ret;
	unsigned int bl_val;
	unsigned int bl_lsb;
	unsigned int bl_msb;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL))
		return snprintf(buf, PAGE_SIZE, "input invalid\n");

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return snprintf(buf, PAGE_SIZE, "driver data is null\n");

	ret = i2c_smbus_read_byte_data(pchip->client,
			pchip->bl_config.bl_lsb_reg_cmd.cmd_reg);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE,
			"backlight i2c read lsb register error\n");
	bl_lsb = (unsigned int)ret;

	ret = i2c_smbus_read_byte_data(pchip->client,
			pchip->bl_config.bl_msb_reg_cmd.cmd_reg);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE,
			"backlight i2c read msb register error\n");
	bl_msb = (unsigned int)ret;
	bl_val = (bl_msb << pchip->bl_config.bl_lsb_reg_cmd.val_bits) | bl_lsb;
	return snprintf(buf, PAGE_SIZE, "bl = 0x%x\n", bl_val);
}

static ssize_t backlight_bl_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	unsigned int bl_val = 0;
	unsigned int bl_msb = 0;
	unsigned int bl_lsb = 0;
	int ret = 0;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return size;
	}
	ret = kstrtouint(buf, LCD_BACKLIGHT_BASE, &bl_val);
	if (ret) {
		LCD_KIT_ERR("check input fail\n");
		return -EINVAL;
	}
	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return -EINVAL;
	bl_lsb = bl_val & pchip->bl_config.bl_lsb_reg_cmd.cmd_mask;
	bl_msb = (bl_val >> pchip->bl_config.bl_lsb_reg_cmd.val_bits) &
		pchip->bl_config.bl_msb_reg_cmd.cmd_mask;

	if (pchip->bl_config.bl_lsb_reg_cmd.val_bits != 0) {
		ret = lcd_kit_backlight_write_byte(pchip,
			pchip->bl_config.bl_lsb_reg_cmd.cmd_reg, bl_lsb);
		if (ret < 0) {
			LCD_KIT_ERR("set backlight failed\n");
			return -EINVAL;
		}
	}
	ret = lcd_kit_backlight_write_byte(pchip,
		pchip->bl_config.bl_msb_reg_cmd.cmd_reg, bl_msb);
	if (ret < 0) {
		LCD_KIT_ERR("set backlight failed\n");
		return -EINVAL;
	}
	return size;
}

static DEVICE_ATTR(backlight, S_IRUGO|S_IWUSR, backlight_bl_show, backlight_bl_store);

static struct attribute *backlight_attributes[] = {
	&dev_attr_reg_addr.attr,
	&dev_attr_reg.attr,
	&dev_attr_backlight.attr,
	NULL,
};

static const struct attribute_group backlight_group = {
	.attrs = backlight_attributes,
};

static void lcd_kit_backlight_propname_cat(char *pdest, const char *psrc,
	int dest_len)
{
	if ((pdest == NULL) || (psrc == NULL) ||
		(dest_len == 0)) {
		LCD_KIT_ERR("input invalid\n");
		return;
	}
	memset(pdest, 0, dest_len);
	snprintf(pdest, dest_len, "%s,%s", backlight_ic_name, psrc);
}

void lcd_kit_parse_backlight_param(struct device_node *pnp,
	const char *node_str, unsigned int *pval)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	unsigned int tmp_val;
	int ret = 0;

	if ((node_str == NULL) ||(pval == NULL) || ( pnp == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return;
	}

	lcd_kit_backlight_propname_cat(tmp_buf, node_str, sizeof(tmp_buf));
	ret = of_property_read_u32(pnp, tmp_buf, &tmp_val);
	*pval = (!ret ? tmp_val : 0);
}

static int lcd_kit_parse_backlight_cmd(struct device_node *np,
	const char *propertyname, struct backlight_ic_cmd *pcmd)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	struct property *prop = NULL;
	unsigned int *buf = NULL;
	int ret = 0;

	if ((np == NULL) || (propertyname == NULL) || (pcmd == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_backlight_propname_cat(tmp_buf, propertyname, sizeof(tmp_buf));
	memset(pcmd, 0, sizeof(struct backlight_ic_cmd));
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("%s is not config\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	if (prop->length != BACKLIGHT_IC_CMD_LEN * sizeof(unsigned int)) {
		LCD_KIT_ERR("%s number is not right\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = of_property_read_u32_array(np, tmp_buf,
			buf, BACKLIGHT_IC_CMD_LEN);
	if (ret) {
		LCD_KIT_ERR("get %s config fail\n", tmp_buf);
		kfree(buf);
		buf = NULL;
		return LCD_KIT_FAIL;
	}
	/* 0:ops_type 1:cmd_reg  2:cmd_val  3:cmd_mask 4:delay */
	pcmd->ops_type = buf[0];
	pcmd->cmd_reg = buf[1];
	pcmd->cmd_val = buf[2];
	pcmd->cmd_mask = buf[3];
	pcmd->delay = buf[4];
	LCD_KIT_INFO("%s: type is 0x%x  reg is 0x%x  val is 0x%x   mask is 0x%x\n",
		propertyname, pcmd->ops_type, pcmd->cmd_reg,
		pcmd->cmd_val, pcmd->cmd_mask);
	kfree(buf);
	buf = NULL;

	return LCD_KIT_OK;
}

static int lcd_kit_backlight_parse_base_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	if ((pbl_config == NULL) || (np == NULL))
		return LCD_KIT_FAIL;
	lcd_kit_parse_backlight_param(np, "backlight_ic_backlight_level",
		&pbl_config->bl_level);
	lcd_kit_parse_backlight_param(np, "backlight_ic_kernel_ctrl_mode",
		&pbl_config->bl_ctrl_mod);
	lcd_kit_parse_backlight_param(np, "backlight_ic_only_backlight",
		&pbl_config->bl_only);
	lcd_kit_parse_backlight_param(np, "backlight_ic_level_10000",
		&pbl_config->bl_10000_support);
	lcd_kit_parse_backlight_param(np, "backlight_ic_bits_compatible",
		&pbl_config->bits_compatible);
	lcd_kit_parse_backlight_param(np, "backlight_ic_set_msb_before_lsb",
		&pbl_config->msb_before_lsb);
	lcd_kit_parse_backlight_param(np, "backlight_dual_ic",
		&pbl_config->dual_ic);
	if (pbl_config->dual_ic) {
		lcd_kit_parse_backlight_param(np, "backlight_dual_ic_i2c_bus_id",
			&pbl_config->dual_i2c_bus_id);
		LCD_KIT_INFO("dual_ic %u dual bus id %u\n", pbl_config->dual_ic,
			pbl_config->dual_i2c_bus_id);
	}
	lcd_kit_parse_backlight_param(np, "dimming_support",
		&pbl_config->bl_dimming_support);
	if (pbl_config->bl_dimming_support) {
		lcd_kit_parse_backlight_param(np, "dimming_config",
			&pbl_config->bl_dimming_config);
		lcd_kit_parse_backlight_param(np, "dimming_resume",
			&pbl_config->bl_dimming_resume);
		lcd_kit_parse_backlight_param(np, "dimming_config_reg",
			&pbl_config->dimming_config_reg);
	}

	return LCD_KIT_OK;
}

static void lcd_kit_backlight_parse_bias_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	lcd_kit_parse_backlight_param(np, "backlight_ic_init_vsp_index",
		&pbl_config->init_vsp_index);
	lcd_kit_parse_backlight_param(np, "backlight_ic_init_vsn_index",
		&pbl_config->init_vsn_index);
	lcd_kit_parse_backlight_param(np, "backlight_ic_pull_boost_support",
		&pbl_config->pull_down_boost_support);
	if (pbl_config->pull_down_boost_support) {
		lcd_kit_parse_backlight_param(np, "backlight_ic_pull_boost_delay",
			&pbl_config->pull_down_boost_delay);
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_pull_down_vsp_cmd",
			&pbl_config->pull_down_vsp_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down vsp cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_pull_down_vsn_cmd",
			&pbl_config->pull_down_vsn_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down vsn cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_pull_down_boost_cmd",
			&pbl_config->pull_down_boost_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down boost cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_bias_enable_cmd",
			&pbl_config->bias_enable_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down boost cmd fail\n");
	}
}

static void lcd_kit_backlight_parse_gpio_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	lcd_kit_parse_backlight_param(np, "hw_enable",
		&pbl_config->hw_en);
	if (pbl_config->hw_en) {
		lcd_kit_parse_backlight_param(np, "hw_en_gpio",
			&pbl_config->hw_en_gpio);
		lcd_kit_parse_backlight_param(np, "gpio_offset",
			&pbl_config->gpio_offset);
		pbl_config->hw_en_gpio += pbl_config->gpio_offset;
		LCD_KIT_INFO("hw_en_gpio is %u\n", pbl_config->hw_en_gpio);
		lcd_kit_parse_backlight_param(np, "bl_on_mdelay",
			&pbl_config->bl_on_mdelay);
		lcd_kit_parse_backlight_param(np, "before_bl_on_mdelay",
			&pbl_config->before_bl_on_mdelay);
		lcd_kit_parse_backlight_param(np, "hw_en_pull_low",
			&pbl_config->hw_en_pull_low);
		lcd_kit_parse_backlight_param(np, "ic_disable_mdelay",
			&pbl_config->ic_disable_mdelay);
		if (pbl_config->dual_ic) {
			lcd_kit_parse_backlight_param(np, "dual_hw_en_gpio",
				&pbl_config->dual_hw_en_gpio);
			pbl_config->dual_hw_en_gpio += pbl_config->gpio_offset;
			LCD_KIT_INFO("dual_hw_en_gpio is %u\n",
				pbl_config->dual_hw_en_gpio);
		}
	}
}

static int lcd_kit_backlight_parse_bl_cmd(struct device_node *np,
	const char *propertyname, struct backlight_reg_info *pcmd)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	struct property *prop = NULL;
	unsigned int *buf = NULL;
	int ret = 0;

	if ((np == NULL) || (propertyname == NULL) || (pcmd == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_backlight_propname_cat(tmp_buf, propertyname, sizeof(tmp_buf));
	memset(pcmd, 0, sizeof(struct backlight_reg_info));
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("%s is not config\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	if (prop->length != BACKLIGHT_BL_CMD_LEN * sizeof(unsigned int)) {
		LCD_KIT_ERR("%s number is not right\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = of_property_read_u32_array(np, tmp_buf,
		buf, BACKLIGHT_BL_CMD_LEN);
	if (ret) {
		LCD_KIT_ERR("get %s config fail\n", tmp_buf);
		kfree(buf);
		buf = NULL;
		return LCD_KIT_FAIL;
	}
	/* 0:val_bits 1:cmd_reg  2:cmd_val  3:cmd_mask */
	pcmd->val_bits = buf[0];
	pcmd->cmd_reg = buf[1];
	pcmd->cmd_val = buf[2];
	pcmd->cmd_mask = buf[3];
	LCD_KIT_INFO("%s: val_bits is 0x%x  reg is 0x%x  val is 0x%x   mask is 0x%x\n",
		tmp_buf, pcmd->val_bits, pcmd->cmd_reg, pcmd->cmd_val, pcmd->cmd_mask);
	kfree(buf);
	buf = NULL;

	return LCD_KIT_OK;
}

static void lcd_kit_backlight_parse_backlight_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	if ((np == NULL) || (pbl_config == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	ret = lcd_kit_backlight_parse_bl_cmd(np, "backlight_ic_level_lsb_cmd",
		&pbl_config->bl_lsb_reg_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse lsb cmd fail\n");
	ret = lcd_kit_backlight_parse_bl_cmd(np, "backlight_ic_level_msb_cmd",
		&pbl_config->bl_msb_reg_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse msb cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_bl_enable_cmd",
		&pbl_config->bl_enable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse bl enable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_bl_disable_cmd",
		&pbl_config->bl_disable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse bl disable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_dev_disable_cmd",
		&pbl_config->disable_dev_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse dev disable cmd fail\n");
}

static void lcd_kit_backlight_parse_fault_check_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	lcd_kit_parse_backlight_param(np, "backlight_ic_fault_check_support",
		&pbl_config->fault_check_enable);
	if (pbl_config->fault_check_enable) {
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_ovp_flag_cmd",
			&pbl_config->bl_ovp_flag_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse ovp flag cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_ocp_flag_cmd",
			&pbl_config->bl_ocp_flag_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse ocp flag cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_tsd_flag_cmd",
			&pbl_config->bl_tsd_flag_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse tsd flag cmd fail\n");
	}
}

static void lcd_kit_backlight_parse_init_cmds_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;
	unsigned int tmp_val;
	struct property *prop = NULL;
	int i;
	unsigned int *buf = NULL;
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};

	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	memset(&pbl_config->init_cmds[0], 0, LCD_BACKLIGHT_INIT_CMD_NUM);
	lcd_kit_parse_backlight_param(np, "backlight_ic_num_of_kernel_init_cmd",
		&pbl_config->num_of_init_cmds);
	if (!pbl_config->num_of_init_cmds) {
		LCD_KIT_ERR("init cmd number is 0\n");
		return;
	}
	lcd_kit_backlight_propname_cat(tmp_buf, "backlight_ic_kernel_init_cmd",
		sizeof(tmp_buf));
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("kernel init cmds is not config\n");
		return;
	}
	tmp_val = pbl_config->num_of_init_cmds;
	if (prop->length !=
		tmp_val * BACKLIGHT_IC_CMD_LEN * sizeof(unsigned int)) {
		LCD_KIT_ERR("init cmds number is not right\n");
		return;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return;
	}
	lcd_kit_backlight_propname_cat(tmp_buf, "backlight_ic_kernel_init_cmd",
		sizeof(tmp_buf));
	ret = of_property_read_u32_array(np, tmp_buf,
		buf, prop->length/sizeof(unsigned int));
	if (ret) {
		LCD_KIT_ERR("get kernel init cmds config fail\n");
		kfree(buf);
		buf = NULL;
		return;
	}
	for (i = 0; i < pbl_config->num_of_init_cmds; i++) {
		pbl_config->init_cmds[i].ops_type =
			buf[i * BACKLIGHT_IC_CMD_LEN];
		pbl_config->init_cmds[i].cmd_reg =
			buf[i * BACKLIGHT_IC_CMD_LEN + 1];
		pbl_config->init_cmds[i].cmd_val =
			buf[i * BACKLIGHT_IC_CMD_LEN + 2];
		pbl_config->init_cmds[i].cmd_mask =
			buf[i * BACKLIGHT_IC_CMD_LEN + 3];
		pbl_config->init_cmds[i].delay =
			buf[i * BACKLIGHT_IC_CMD_LEN + 4];
		LCD_KIT_INFO("init i is %d, tpye is 0x%x  reg is 0x%x  val is 0x%x   mask is 0x%x\n",
		 i, pbl_config->init_cmds[i].ops_type,
		 pbl_config->init_cmds[i].cmd_reg,
		 pbl_config->init_cmds[i].cmd_val,
		 pbl_config->init_cmds[i].cmd_mask);
	}
	kfree(buf);
	buf = NULL;
}

void lcd_kit_backlight_parse_dts(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	LCD_KIT_INFO("enter\n");
	if ((np == NULL) || (pbl_config == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	ret = lcd_kit_backlight_parse_base_config(np, pbl_config);
	if (ret < 0) {
		LCD_KIT_ERR("parse base config fail\n");
		return;
	}
	if (!pbl_config->bl_only)
		lcd_kit_backlight_parse_bias_config(np, pbl_config);
	lcd_kit_backlight_parse_gpio_config(np, pbl_config);
	lcd_kit_backlight_parse_backlight_config(np, pbl_config);
	lcd_kit_backlight_parse_fault_check_config(np, pbl_config);
	lcd_kit_backlight_parse_init_cmds_config(np, pbl_config);
}

static void lcd_kit_backlight_get_bias_value(int vsp, int vsn,
	unsigned char *pos_votage, unsigned char *neg_votage)
{
	int i;
	int j;

	if ((pos_votage == NULL) || (neg_votage == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(backlight_bias_config); i++) {
		if (strcmp(backlight_ic_name, backlight_bias_config[i].name))
			continue;
		for (j = 0; j < backlight_bias_config[i].len; j++) {
			if (backlight_bias_config[i].vol_table[j].voltage == vsp) {
				*pos_votage = backlight_bias_config[i].vol_table[j].value;
				break;
			}
		}
		if (j == backlight_bias_config[i].len) {
			LCD_KIT_INFO("not found vsp voltage, use default\n");
			*pos_votage = DEFAULT_VOL_VAL;
		}

		for (j = 0; j < backlight_bias_config[i].len; j++) {
			if (backlight_bias_config[i].vol_table[j].voltage == vsn) {
				*neg_votage  = backlight_bias_config[i].vol_table[j].value;
				break;
			}
		}

		if (j == backlight_bias_config[i].len) {
			LCD_KIT_INFO("not found neg voltage, use default voltage\n");
			*neg_votage = DEFAULT_VOL_VAL;
		}
		break;
	}

	if (i == ARRAY_SIZE(backlight_bias_config)) {
		LCD_KIT_INFO("not found right voltage, use default voltage\n");
		*pos_votage = DEFAULT_VOL_VAL;
		*neg_votage = DEFAULT_VOL_VAL;
	}
}

static int lcd_kit_backlight_dual_ic_i2c_adapter(void)
{
	struct i2c_adapter *adap = NULL;

	LCD_KIT_INFO("%s in", __func__);
	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	adap = i2c_get_adapter(pbacklight_dev->bl_config.dual_i2c_bus_id);
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}
	g_backlight_dual_ic_adap = adap;
	LCD_KIT_INFO("%s exit", __func__);
	return LCD_KIT_OK;
}

static int lcd_kit_backlight_dual_ic_config(void)
{
	struct i2c_adapter *adap = g_backlight_dual_ic_adap;
	struct i2c_msg msg = {0};
	char buf[2]; /* buf[0] is cmd reg buf[1] is cmd val */
	int ret;
	int i;

	LCD_KIT_INFO("%s in", __func__);
	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}

	msg.addr = pbacklight_dev->client->addr;
	msg.flags = pbacklight_dev->client->flags;
	msg.len = 2;
	msg.buf = buf;

	for (i = 0; i < pbacklight_dev->bl_config.num_of_init_cmds; i++) {
		buf[0] = pbacklight_dev->bl_config.init_cmds[i].cmd_reg;
		buf[1] = pbacklight_dev->bl_config.init_cmds[i].cmd_val;
		ret = i2c_transfer(adap, &msg, 1);
		LCD_KIT_INFO("%s reg = 0x%x, val = 0x%x\n",
			__func__, buf[0], buf[1]);
		if (pbacklight_dev->bl_config.init_cmds[i].delay)
			mdelay(pbacklight_dev->bl_config.init_cmds[i].delay);
	}

	return ret;
}

static int lcd_kit_setbacklight_dual_ic(int bl_lsb, int bl_msb)
{
	struct i2c_adapter *adap = g_backlight_dual_ic_adap;
	struct i2c_msg msg = {0};
	struct i2c_msg msg2 = {0};
	char buf[2];  /* buf[0] is cmd reg, buf[1] is cmd val */
	char buf2[2]; /* buf2[0] is cmd reg, buf2[1] is cmd val */
	int ret;

	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}

	msg.addr = pbacklight_dev->client->addr;
	msg.flags = pbacklight_dev->client->flags;
	msg.len = 2;
	msg2 = msg;
	buf[0] = pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg;
	buf[1] = bl_lsb;
	buf2[0] = pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_reg;
	buf2[1] = bl_msb;

	if (!pbacklight_dev->bl_config.msb_before_lsb) {
		msg.buf = buf;   /* lsb */
		msg2.buf = buf2; /* msb */
	} else {
		msg.buf = buf2;  /* msb */
		msg2.buf = buf;  /* lsb */
	}

	ret = i2c_transfer(adap, &msg, 1);
	ret = i2c_transfer(adap, &msg2, 1);
	return ret;
}

static int dync_dimming_config_dual_ic(unsigned char dim_reg, unsigned char dim_data)
{
	struct i2c_adapter *adap = g_backlight_dual_ic_adap;
	struct i2c_msg msg = {0};

	char buf[2];  /* buf[0] is cmd reg, buf[1] is cmd val */
	int ret;
	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}
	msg.addr = pbacklight_dev->client->addr;
	msg.flags = pbacklight_dev->client->flags;
	msg.len = 2;

	buf[0] = dim_reg;
	buf[1] = dim_data;
	msg.buf = buf;

	ret = i2c_transfer(adap, &msg, 1);
	LCD_KIT_INFO("%s reg = 0x%x, val = 0x%x\n", __func__, buf[0], buf[1]);

	return ret;
}

static int lcd_kit_backlight_ic_config(void)
{
	int ret;
	int i;

	LCD_KIT_INFO("backlight ic config\n");
	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < pbacklight_dev->bl_config.num_of_init_cmds; i++) {
		switch (pbacklight_dev->bl_config.init_cmds[i].ops_type) {
		case REG_READ_MODE:
			ret = lcd_kit_backlight_read_byte(pbacklight_dev,
				pbacklight_dev->bl_config.init_cmds[i].cmd_reg,
				&(pbacklight_dev->bl_config.init_cmds[i].cmd_val));
			break;
		case REG_WRITE_MODE:
			ret = lcd_kit_backlight_write_byte(pbacklight_dev,
				pbacklight_dev->bl_config.init_cmds[i].cmd_reg,
				pbacklight_dev->bl_config.init_cmds[i].cmd_val);
			break;
		case REG_UPDATE_MODE:
			ret = lcd_kit_backlight_update_bit(pbacklight_dev,
				pbacklight_dev->bl_config.init_cmds[i].cmd_reg,
				pbacklight_dev->bl_config.init_cmds[i].cmd_mask,
				pbacklight_dev->bl_config.init_cmds[i].cmd_val);
			break;
		default:
			break;
		}
		if (ret < 0) {
			LCD_KIT_ERR("operation  reg 0x%x fail\n",
				pbacklight_dev->bl_config.init_cmds[i].cmd_reg);
			return ret;
		}
		if (pbacklight_dev->bl_config.init_cmds[i].delay)
			mdelay(pbacklight_dev->bl_config.init_cmds[i].delay);
	}

	return ret;
}

static int lcd_kit_gpio_dual_request()
{
	int ret;

	if (pbacklight_dev->bl_config.dual_hw_en_gpio) {
		ret = gpio_request(pbacklight_dev->bl_config.dual_hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("can not request  dual_hw_en_gpio\n");
			return LCD_KIT_FAIL;
		}
		ret = gpio_direction_output(pbacklight_dev->bl_config.dual_hw_en_gpio,
			BACKLIGHT_GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("set gpio output not success\n");
		gpio_set_value(pbacklight_dev->bl_config.dual_hw_en_gpio,
			BACKLIGHT_GPIO_OUT_ONE);
		LCD_KIT_INFO("lcd_kit_backlight_enable dual_hw_en_gpio = %d suc\n",
			pbacklight_dev->bl_config.dual_hw_en_gpio);
	}

	return LCD_KIT_OK;
}

static int lcd_kit_backlight_set_bias(int vpos, int vneg)
{
	int ret;
	unsigned int vsp_index;
	unsigned int vsn_index;
	unsigned char vsp_vol = 0;
	unsigned char vsn_vol = 0;

	if (vpos < 0 || vneg < 0) {
		LCD_KIT_ERR("vpos or vneg is error\n");
		return LCD_KIT_FAIL;
	}

	if (pbacklight_dev == NULL)
		return LCD_KIT_FAIL;

	if (pbacklight_dev->bl_config.hw_en != 0 &&
		pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		ret = gpio_request(pbacklight_dev->bl_config.hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("can not request  hw_en_gpio\n");
			return LCD_KIT_FAIL;
		}
		ret = gpio_direction_output(pbacklight_dev->bl_config.hw_en_gpio,
			BACKLIGHT_GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("set gpio output not success\n");
		gpio_set_value(pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ONE);
		if (pbacklight_dev->bl_config.bl_on_mdelay)
			mdelay(pbacklight_dev->bl_config.bl_on_mdelay);
	}
	if (g_vol_mapped == BACKLIGHT_FALSE) {
		vsp_index = pbacklight_dev->bl_config.init_vsp_index;
		vsn_index = pbacklight_dev->bl_config.init_vsn_index;
		lcd_kit_backlight_get_bias_value(vpos, vneg, &vsp_vol, &vsn_vol);
		pbacklight_dev->bl_config.init_cmds[vsp_index].cmd_val = vsp_vol;
		pbacklight_dev->bl_config.init_cmds[vsn_index].cmd_val = vsn_vol;
		g_vol_mapped = BACKLIGHT_TRUE;
	}
	ret = lcd_kit_backlight_ic_config();
	if (ret < 0) {
		LCD_KIT_ERR("set config register failed");
		return ret;
	}
	backlight_init_status = true;
	LCD_KIT_INFO("set_bias is successful\n");
	return ret;
}

static void lcd_kit_backlight_enable(void)
{
	int ret;

	if (pbacklight_dev->bl_config.hw_en != 0 &&
		pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		if (pbacklight_dev->bl_config.before_bl_on_mdelay)
			mdelay(pbacklight_dev->bl_config.before_bl_on_mdelay);
		if (pbacklight_dev->bl_config.dual_ic) {
			ret = lcd_kit_gpio_dual_request();
			if (ret) {
				LCD_KIT_ERR("can not request gpio_dual\n");
				return;
			}
		}
		ret = gpio_request(pbacklight_dev->bl_config.hw_en_gpio, NULL);
		if (ret) {
			LCD_KIT_ERR("can not request  hw_en_gpio\n");
			return;
		}
		ret = gpio_direction_output(pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("set gpio output not success\n");
		gpio_set_value(pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ONE);
		LCD_KIT_INFO("backlight ic enable enter\n");
		if (pbacklight_dev->bl_config.bl_on_mdelay)
			mdelay(pbacklight_dev->bl_config.bl_on_mdelay);
	}

	/* chip initialize */
	ret = lcd_kit_backlight_ic_config();
	if (ret < 0) {
		LCD_KIT_ERR("backlight ic init fail!\n");
		return;
	}
	if (pbacklight_dev->bl_config.dual_ic) {
		ret = lcd_kit_backlight_dual_ic_config();
		if (ret < 0) {
			LCD_KIT_ERR("dual bl_ic not used\n");
			return;
		}
	}
	backlight_init_status = true;
}

static void lcd_kit_backlight_disable(void)
{
	if (pbacklight_dev->bl_config.hw_en != 0 &&
		pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		if (pbacklight_dev->bl_config.dual_ic &&
			pbacklight_dev->bl_config.dual_hw_en_gpio) {
			LCD_KIT_INFO("backlight dual ic disable enter\n");
			gpio_set_value(pbacklight_dev->bl_config.dual_hw_en_gpio,
				BACKLIGHT_GPIO_OUT_ZERO);
			gpio_free(pbacklight_dev->bl_config.dual_hw_en_gpio);
		}
		LCD_KIT_INFO("backlight ic disable enter\n");
		gpio_set_value(pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ZERO);
		gpio_free(pbacklight_dev->bl_config.hw_en_gpio);
	}
	backlight_init_status = false;
}

static int lcd_kit_backlight_set_ic_disable(void)
{
	int ret;

	if (pbacklight_dev == NULL)
		return LCD_KIT_FAIL;

	/* reset backlight ic */
	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.disable_dev_cmd.cmd_reg,
		pbacklight_dev->bl_config.disable_dev_cmd.cmd_val);
	if (ret < 0)
		LCD_KIT_ERR("write REG_BL_ENABLE fail\n");

	/* clean up bl val register */
	ret = lcd_kit_backlight_update_bit(pbacklight_dev,
		pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg,
		pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_mask, 0);
	if (ret < 0)
		LCD_KIT_ERR("update_bits REG_BL_BRIGHTNESS_LSB fail\n");

	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_reg, 0);
	if (ret < 0)
		LCD_KIT_ERR("write REG_BL_BRIGHTNESS_MSB fail!\n");

	if (pbacklight_dev->bl_config.ic_disable_mdelay)
		mdelay(pbacklight_dev->bl_config.ic_disable_mdelay);
	if (pbacklight_dev->bl_config.hw_en != 0 &&
		pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		if (pbacklight_dev->bl_config.dual_ic &&
			pbacklight_dev->bl_config.dual_hw_en_gpio) {
			gpio_set_value(pbacklight_dev->bl_config.dual_hw_en_gpio,
				BACKLIGHT_GPIO_OUT_ZERO);
			gpio_free(pbacklight_dev->bl_config.dual_hw_en_gpio);
		}
		gpio_set_value(pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ZERO);
		gpio_free(pbacklight_dev->bl_config.hw_en_gpio);
	}
	backlight_init_status = false;
	return ret;
}

static int lcd_kit_backlight_set_bias_power_down(int vpos, int vneg)
{
	int ret;
	unsigned char vsp = 0;
	unsigned char vsn = 0;

	if (pbacklight_dev == NULL)
		return LCD_KIT_FAIL;

	if (!pbacklight_dev->bl_config.pull_down_boost_support) {
		LCD_KIT_INFO("No need to pull down BOOST\n");
		return 0;
	}

	lcd_kit_backlight_get_bias_value(vpos, vneg, &vsp, &vsn);
	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.pull_down_vsp_cmd.cmd_reg, vsp);
	if (ret < 0) {
		LCD_KIT_ERR("write pull_down_vsp failed\n");
		return ret;
	}
	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.pull_down_vsn_cmd.cmd_reg, vsn);
	if (ret < 0) {
		LCD_KIT_ERR("write pull_down_vsn failed\n");
		return ret;
	}
	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.bias_enable_cmd.cmd_reg,
		pbacklight_dev->bl_config.bias_enable_cmd.cmd_val);
	if (ret < 0) {
		LCD_KIT_ERR("write enable_vsp_vsn failed\n");
		return ret;
	}
	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.pull_down_boost_cmd.cmd_reg,
		pbacklight_dev->bl_config.pull_down_boost_cmd.cmd_val);
	if (ret < 0) {
		LCD_KIT_ERR("write boost_vsp_vsn failed\n");
		return ret;
	}
	LCD_KIT_INFO("lcd_kit_pull_boost is successful\n");
	return ret;
}

#if defined CONFIG_HUAWEI_DSM
static void lcd_kit_backlight_ic_ovp_check(struct lcd_kit_backlight_device *pchip,
	int last_level, int level)
{
	unsigned char val = 0;
	int ret;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	if (!pchip->bl_config.bl_ovp_flag_cmd.cmd_mask)
		return;
	ret = lcd_kit_backlight_read_byte(pchip,
		pchip->bl_config.bl_ovp_flag_cmd.cmd_reg, &val);
	if (ret < 0) {
		LCD_KIT_ERR("read backlight ic ovp reg fail\n");
		return;
	}
	if ((val & pchip->bl_config.bl_ovp_flag_cmd.cmd_mask) ==
		pchip->bl_config.bl_ovp_flag_cmd.cmd_val) {
		LCD_KIT_INFO("backlight ic check ovp fault val is 0x%x\n", val);
		if (lcd_dclient == NULL)
			return;
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret != 0) {
			LCD_KIT_ERR("dsm_client_ocuppy fail, ret=%d\n", ret);
			return;
		}
		dsm_client_record(lcd_dclient,
			"%s : last_bkl:%d, cur_bkl:%d, reg val 0x%x=0x%x\n",
			backlight_ic_name, last_level, level,
			pchip->bl_config.bl_ovp_flag_cmd.cmd_reg, val);
		dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
	}
}

static void lcd_kit_backlight_ic_ocp_check(struct lcd_kit_backlight_device *pchip,
	int last_level, int level)
{
	unsigned char val = 0;
	int ret;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	if (!pchip->bl_config.bl_ocp_flag_cmd.cmd_mask)
		return;
	ret = lcd_kit_backlight_read_byte(pchip,
		pchip->bl_config.bl_ocp_flag_cmd.cmd_reg, &val);
	if (ret < 0) {
		LCD_KIT_ERR("read backlight ic ocp reg fail\n");
		return;
	}
	if ((val & pchip->bl_config.bl_ocp_flag_cmd.cmd_mask) ==
		pchip->bl_config.bl_ocp_flag_cmd.cmd_val) {
		LCD_KIT_INFO("backlight ic check ocp fault val is 0x%x\n", val);
		if (lcd_dclient == NULL)
			return;
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret != 0) {
			LCD_KIT_ERR("dsm_client_ocuppy fail, ret=%d\n", ret);
			return;
		}
		dsm_client_record(lcd_dclient,
			"%s : last_bkl:%d, cur_bkl:%d, reg val 0x%x=0x%x\n",
			backlight_ic_name, last_level, level,
			pchip->bl_config.bl_ocp_flag_cmd.cmd_reg, val);
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_OCP_ERROR_NO);
	}
}

static void lcd_kit_backlight_ic_tsd_check(struct lcd_kit_backlight_device *pchip,
	int last_level, int level)
{
	unsigned char val = 0;
	int ret;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	if (!pchip->bl_config.bl_tsd_flag_cmd.cmd_mask)
		return;
	ret = lcd_kit_backlight_read_byte(pchip,
		pchip->bl_config.bl_tsd_flag_cmd.cmd_reg, &val);
	if (ret < 0) {
		LCD_KIT_ERR("read backlight ic tsd reg fail\n");
		return;
	}
	if ((val & pchip->bl_config.bl_tsd_flag_cmd.cmd_mask) ==
		pchip->bl_config.bl_tsd_flag_cmd.cmd_val) {
		LCD_KIT_INFO("backlight ic check tsd fault val is 0x%x\n", val);
		if (lcd_dclient == NULL)
			return;
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret != 0) {
			LCD_KIT_ERR("dsm_client_ocuppy fail, ret=%d\n", ret);
			return;
		}
		dsm_client_record(lcd_dclient,
			"%s : last_bkl:%d, cur_bkl:%d, reg val 0x%x=0x%x\n",
			backlight_ic_name, last_level, level,
			pchip->bl_config.bl_tsd_flag_cmd.cmd_reg, val);
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_TSD_ERROR_NO);
	}
}

static void dync_dimming_config(unsigned int bl_level)
{
	int ret = -1;
	static int bl_work_state = LCD_BACKLIGHT_POWER_ON;
	if (pbacklight_dev->bl_config.bl_dimming_support != LCD_BACKLIGHT_DIMMING_SUPPORT)
		return;
	if (bl_level == 0) {
		if (pbacklight_dev->bl_config.bl_dimming_resume != 0) {
			LCD_KIT_INFO("config bl dimming reg[%d]=0x%x\n",
				pbacklight_dev->bl_config.dimming_config_reg,
				pbacklight_dev->bl_config.bl_dimming_resume);

			ret = lcd_kit_backlight_write_byte(pbacklight_dev,
				pbacklight_dev->bl_config.dimming_config_reg,
				pbacklight_dev->bl_config.bl_dimming_resume);
			if (ret < 0)
				LCD_KIT_ERR("dync dimming config error\n");
			if (pbacklight_dev->bl_config.dual_ic) {
				ret = dync_dimming_config_dual_ic(pbacklight_dev->bl_config.dimming_config_reg,
					pbacklight_dev->bl_config.bl_dimming_resume);
				if (ret < 0)
					LCD_KIT_ERR("dync dual dimming config error\n");
				LCD_KIT_INFO("backlight icdync dual dimming config val is reg = 0x%x, avl = 0x%x\n",
					pbacklight_dev->bl_config.dimming_config_reg, pbacklight_dev->bl_config.bl_dimming_resume);
			}
		}
		bl_work_state = LCD_BACKLIGHT_POWER_OFF;
		return;
	}
	if (bl_work_state == LCD_BACKLIGHT_POWER_OFF) {
		bl_work_state = LCD_BACKLIGHT_POWER_ON;
	} else if (bl_work_state == LCD_BACKLIGHT_POWER_ON) {
		if (pbacklight_dev->bl_config.bl_dimming_config != 0) {
			LCD_KIT_INFO("config bl dimming reg[%d]=0x%x\n",
				pbacklight_dev->bl_config.dimming_config_reg,
				pbacklight_dev->bl_config.bl_dimming_config);

			ret = lcd_kit_backlight_write_byte(pbacklight_dev,
				pbacklight_dev->bl_config.dimming_config_reg,
				pbacklight_dev->bl_config.bl_dimming_config);
			if (ret < 0)
				LCD_KIT_ERR("dync dimming config error\n");
			if (pbacklight_dev->bl_config.dual_ic) {
				ret = dync_dimming_config_dual_ic(pbacklight_dev->bl_config.dimming_config_reg,
					pbacklight_dev->bl_config.bl_dimming_config);
				if (ret < 0)
					LCD_KIT_ERR("dync dual dimming config error\n");
				LCD_KIT_INFO("backlight icdync dual dimming config val is reg = 0x%x, avl = 0x%x\n",
					pbacklight_dev->bl_config.dimming_config_reg, pbacklight_dev->bl_config.bl_dimming_config);
			}
		}
		bl_work_state = LCD_BACKLIGHT_LEVEL_POWER_ON;
	}
}

static void lcd_kit_backlight_ic_fault_check(struct lcd_kit_backlight_device *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	if (pchip->bl_config.fault_check_enable &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0))) {
		LCD_KIT_INFO("start backlight ic fault check\n");
		lcd_kit_backlight_ic_ovp_check(pchip, last_level, level);
		lcd_kit_backlight_ic_ocp_check(pchip, last_level, level);
		lcd_kit_backlight_ic_tsd_check(pchip, last_level, level);
	}
	last_level = level;
}
#endif

int lcd_kit_set_backlight(unsigned int bl_level)
{
	int ret = LCD_KIT_FAIL;
	unsigned int level;
	unsigned char bl_msb;
	unsigned char bl_lsb;
	unsigned int bl_max_level = lcm_get_panel_backlight_max_level();

	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR("init fail, return\n");
		return ret;
	}
	if (down_trylock(&(pbacklight_dev->sem))) {
		LCD_KIT_INFO("Now in test mode\n");
		return ret;
	}
	LCD_KIT_INFO("bl_level = %d\n", bl_level);
	/* first set backlight, enable ktz8864 */
	if (pbacklight_dev->bl_config.bl_only != 0) {
		if ((backlight_init_status == false) && (bl_level > 0))
			lcd_kit_backlight_enable();
	}

	if (pbacklight_dev->bl_config.bl_10000_support) {
		level = bl_lvl_map(bl_level);
	} else {
		if (pbacklight_dev->bl_config.bits_compatible && bl_max_level > 0) {
			bl_level = bl_level * pbacklight_dev->bl_config.bl_level / bl_max_level;
		}
		level = bl_level;
		if (level > pbacklight_dev->bl_config.bl_level)
			level = pbacklight_dev->bl_config.bl_level;
	}
	if (pbacklight_dev->bl_config.bl_ctrl_mod == BL_PWM_I2C_MODE) {
		LCD_KIT_INFO("backlight ic ctrl by mipi pwm, level is %u\n", level);
#ifdef CONFIG_HUAWEI_DSM
		lcd_kit_backlight_ic_fault_check(pbacklight_dev, level);
#endif
		up(&(pbacklight_dev->sem));
		return LCD_KIT_OK;
	}
	dync_dimming_config(bl_level);

	/*
	* This is a rt8555 IC bug, when bl level is 0 or 0x0FF,
	* bl level must add or sub 1, or flickering
	*/
	if (!strcmp(backlight_ic_name, "hw_rt8555")) {
		if ((level != 0) && ((level & 0xF) == 0))
			level += 1;
		if ((level & 0xFF) == 0xFF)
			level -= 1;
	} else if (!strcmp(backlight_ic_name, "hw_mp3314")) {
		/*
		* This is a mp3314 IC bug, when bl level more than 4000,
		* bl level must refit the dimming curve.
		* 0.9929:Straight line slope. 446:Line intercept.
		*/
		if (level > 4000)
			level = (unsigned int)(level * 9929  / 10000) + 446;
	}
	bl_lsb = level & pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_mask;
	bl_msb = (level >> pbacklight_dev->bl_config.bl_lsb_reg_cmd.val_bits) &
		pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_mask;

	LCD_KIT_INFO("level = %d, bl_msb = 0x%x, bl_lsb = 0x%x\n", level, bl_msb, bl_lsb);
	if ((pbacklight_dev->bl_config.bl_lsb_reg_cmd.val_bits != 0) &&
		(!pbacklight_dev->bl_config.msb_before_lsb)) {
		ret = lcd_kit_backlight_write_byte(pbacklight_dev,
			pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg, bl_lsb);
		if (ret < 0) {
			LCD_KIT_ERR("set backlight ic brightness lsb failed!\n");
			goto i2c_error;
		}
	}
	ret = lcd_kit_backlight_write_byte(pbacklight_dev,
		pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_reg, bl_msb);
	if (ret < 0) {
		LCD_KIT_ERR("set backlight ic brightness msb failed!\n");
		goto i2c_error;
	}
	if (pbacklight_dev->bl_config.msb_before_lsb) {
		ret = lcd_kit_backlight_write_byte(pbacklight_dev,
			pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg, bl_lsb);
		if (ret < 0) {
			LCD_KIT_ERR("set backlight ic brightness lsb failed!\n");
			goto i2c_error;
		}
	}

	if (pbacklight_dev->bl_config.dual_ic)
		lcd_kit_setbacklight_dual_ic(bl_lsb, bl_msb);
#ifdef CONFIG_HUAWEI_DSM
	lcd_kit_backlight_ic_fault_check(pbacklight_dev, level);
#endif
	/* if set backlight level 0, disable ic */
	if (pbacklight_dev->bl_config.bl_only != 0) {
		if (backlight_init_status == true && bl_level == 0)
			lcd_kit_backlight_disable();
	}
	up(&(pbacklight_dev->sem));
	return ret;

i2c_error:
	up(&(pbacklight_dev->sem));
	LCD_KIT_ERR("lcd kit set backlight exit fail\n");
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = lcd_kit_set_backlight,
	.name = "bl_common",
};

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = lcd_kit_backlight_set_bias,
	.set_bias_power_down = lcd_kit_backlight_set_bias_power_down,
	.set_ic_disable = lcd_kit_backlight_set_ic_disable,
};

static int lcd_kit_backlight_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct i2c_adapter *adapter = NULL;

	LCD_KIT_INFO("enter\n");
	if (client == NULL) {
		LCD_KIT_ERR("client is null\n");
		return -EINVAL;
	}
	adapter = client->adapter;
	if (adapter == NULL) {
		LCD_KIT_ERR("adapter is NULL pointer\n");
		return -EINVAL;
	}
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}
	pbacklight_dev = devm_kzalloc(&client->dev, sizeof(struct lcd_kit_backlight_device) , GFP_KERNEL);
	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR("Failed to allocate memory\n");
		return -ENOMEM;
	}
	pbacklight_dev->client = client;
	if (client->dev.of_node == NULL) {
		LCD_KIT_ERR("of_node is NULL\n");
		devm_kfree(&client->dev, pbacklight_dev);
		pbacklight_dev = NULL;
		return -EINVAL;
	}

	if (client->dev.of_node != NULL)
		bl_ops.name = (char *)of_get_property(client->dev.of_node, "bl_type_string", NULL);

	lcd_kit_backlight_parse_dts(client->dev.of_node, &pbacklight_dev->bl_config);
	sema_init(&(pbacklight_dev->sem), 1);

	if (backlight_class != NULL) {
		pbacklight_dev->dev = device_create(backlight_class,
			NULL, 0, "%s", backlight_ic_name);
		if (IS_ERR(pbacklight_dev->dev)) {
			LCD_KIT_ERR("Unable to create device; errno = %ld\n", PTR_ERR(pbacklight_dev->dev));
			pbacklight_dev->dev = NULL;
			devm_kfree(&client->dev, pbacklight_dev);
			g_sysfs_created = BACKLIGHT_FALSE;
			return -EINVAL;
		} else {
			dev_set_drvdata(pbacklight_dev->dev, pbacklight_dev);
			ret = sysfs_create_group(&pbacklight_dev->dev->kobj, &backlight_group);
			if (ret) {
				LCD_KIT_ERR("Create backlight sysfs group node failed!\n");
				device_destroy(backlight_class, 0);
				devm_kfree(&client->dev, pbacklight_dev);
				g_sysfs_created = BACKLIGHT_FALSE;
				return ret;
			}
			g_sysfs_created = BACKLIGHT_TRUE;
		}
	}
	if (pbacklight_dev->bl_config.dual_ic) {
		ret = lcd_kit_backlight_dual_ic_i2c_adapter();
		if (ret)
			LCD_KIT_ERR("%s dual_ic_i2c_adapter fail", __func__);
	}
	if (pbacklight_dev->bl_config.hw_en) {
		ret = gpio_request(pbacklight_dev->bl_config.hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("can not request hw_en_gpio\n");
	}
	if (pbacklight_dev->bl_config.bl_only != 0) {
		lcd_kit_bl_register(&bl_ops);
	} else {
		lcd_kit_bias_register(&bias_ops);
		lcd_kit_bl_register(&bl_ops);
	}
	backlight_init_status = true;
	LCD_KIT_INFO("exit\n");
	return LCD_KIT_OK;
}

static int lcd_kit_backlight_remove(struct i2c_client *client)
{
	if (pbacklight_dev == NULL) {
		LCD_KIT_ERR("pbacklight_dev is null\n");
		return -EINVAL;
	}
	if (g_sysfs_created)
		sysfs_remove_group(&client->dev.kobj, &backlight_group);
	if (pbacklight_dev->dev != NULL)
		device_destroy(backlight_class, 0);
	if (client != NULL) {
		devm_kfree(&client->dev, pbacklight_dev);
		pbacklight_dev = NULL;
	}
	if (backlight_class != NULL) {
		class_destroy(backlight_class);
		backlight_class = NULL;
	}
	return LCD_KIT_OK;
}

static int __init early_parse_backlight_ic_cmdline(char *arg)
{
	int len;

	if (arg == NULL) {
		LCD_KIT_ERR("input is null\n");
		return -EINVAL;
	}
	memset(backlight_ic_name, 0, LCD_BACKLIGHT_IC_NAME_LEN);
	len = strlen(arg);
	if (len >= LCD_BACKLIGHT_IC_NAME_LEN)
		len = LCD_BACKLIGHT_IC_NAME_LEN - 1;
	memcpy(backlight_ic_name, arg, len);
	backlight_ic_name[LCD_BACKLIGHT_IC_NAME_LEN - 1] = 0;

	return LCD_KIT_OK;
}

early_param("hw_backlight", early_parse_backlight_ic_cmdline);

static struct i2c_device_id backlight_ic_id[] = {
	{ "bl_ic_common", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, backlight_ic_id);

static struct of_device_id lcd_kit_backlight_match_table[] = {
	{ .compatible = "bl_ic_common", },
	{},
};

static struct i2c_driver lcd_kit_backlight_driver = {
	.probe = lcd_kit_backlight_probe,
	.remove = lcd_kit_backlight_remove,
	.driver = {
		.name = "bl_ic_common",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lcd_kit_backlight_match_table),
	},
	.id_table = backlight_ic_id,
};

static int __init lcd_kit_backlight_init(void)
{
	char compatible_name[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	int len;

	LCD_KIT_INFO("backlight_ic_name is %s\n", backlight_ic_name);
	if (!strcmp(backlight_ic_name, "default"))
		return LCD_KIT_OK;

	memcpy(backlight_ic_id[0].name, backlight_ic_name, LCD_BACKLIGHT_IC_NAME_LEN);
	lcd_kit_backlight_driver.driver.name = backlight_ic_name;
	snprintf(compatible_name, sizeof(compatible_name), "huawei,%s", backlight_ic_name);
	len = strlen(compatible_name);
	if (len >= LCD_BACKLIGHT_IC_COMP_LEN)
		len = LCD_BACKLIGHT_IC_COMP_LEN - 1;
	memcpy(lcd_kit_backlight_match_table[0].compatible, compatible_name, len);
	lcd_kit_backlight_match_table[0].compatible[LCD_BACKLIGHT_IC_COMP_LEN - 1] = 0;

	backlight_class = class_create(THIS_MODULE, "lcd_backlight");
	if (IS_ERR(backlight_class)) {
		LCD_KIT_ERR("Unable to create backlight class, errno = %ld\n", PTR_ERR(backlight_class));
		backlight_class = NULL;
	}

	return i2c_add_driver(&lcd_kit_backlight_driver);
}

static void __exit lcd_kit_backlight_exit(void)
{
	if (backlight_class != NULL) {
		class_destroy(backlight_class);
		backlight_class = NULL;
	}
	if (g_backlight_dual_ic_adap != NULL) {
		i2c_put_adapter(g_backlight_dual_ic_adap);
		g_backlight_dual_ic_adap = NULL;
	}

	i2c_del_driver(&lcd_kit_backlight_driver);
}

late_initcall(lcd_kit_backlight_init);
module_exit(lcd_kit_backlight_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");

