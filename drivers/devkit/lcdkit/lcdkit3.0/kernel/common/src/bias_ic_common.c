/*
 * bias_ic_common.c
 *
 * bias_ic_common driver for lcd bias ic
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "bias_ic_common.h"
#include "bias_bl_common.h"
#include "lcd_kit_bias.h"
#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#define BIAS_RET_FAIL (-1)
#define BIAS_RET_OK    0
#define BIAS_TRUE 1
#define BIAS_FALSE 0

#define DMD_SET_VOLTAGE_VSP_FALI_EXPIRE_COUNT 6
#define DMD_SET_VOLTAGE_VSN_FALI_EXPIRE_COUNT 6
#define LCD_BIAS_IC_NAME_LEN 32

#define DTS_BIAS_IC_TYPE "bias_ic_type"
#define DTS_BIAS_IC_POS_REG "bias_ic_pos_reg"
#define DTS_BIAS_IC_POS_MASK "bias_ic_pos_mask"
#define DTS_BIAS_IC_NEG_REG "bias_ic_neg_reg"
#define DTS_BIAS_IC_NEG_MASK "bias_ic_neg_mask"
#define DTS_BIAS_IC_STATE_REG "bias_ic_state_reg"
#define DTS_BIAS_IC_STATE_VAL "bias_ic_state_val"
#define DTS_BIAS_IC_STATE_MASK "bias_ic_state_mask"

#define DTS_COMP_LCDKIT_PANEL_TYPE "huawei,lcd_panel_type"
#define LCD_BIAS_IC_NAME_LEN 32
#define LCD_BIAS_I2C_NAME_SIZE 20
#define CONFIG_DEFAULT_VALUE 0

#define DEFAULT_VOL_VAL 0x0F

static int g_sysfs_created;
static int g_vol_mapped = BIAS_FALSE;
struct class *bias_class;
static char chip_name[LCD_BIAS_IC_NAME_LEN] = "default";
static struct bias_ic_device *pbias_ic;

struct bias_ic_voltage {
	unsigned int voltage;
	unsigned char value;
};

struct bias_ic_config {
	char name[LCD_BIAS_IC_NAME_LEN];
	unsigned char len;
	struct bias_ic_voltage *vol_table;
};

/* bias ic: tps65132 */
static struct bias_ic_voltage tps65132_vol_table[] = {
	{ LCD_BAIC_VOL_40, TPS65132_VOL_40 },
	{ LCD_BAIC_VOL_41, TPS65132_VOL_41 },
	{ LCD_BAIC_VOL_42, TPS65132_VOL_42 },
	{ LCD_BAIC_VOL_43, TPS65132_VOL_43 },
	{ LCD_BAIC_VOL_44, TPS65132_VOL_44 },
	{ LCD_BAIC_VOL_45, TPS65132_VOL_45 },
	{ LCD_BAIC_VOL_46, TPS65132_VOL_46 },
	{ LCD_BAIC_VOL_47, TPS65132_VOL_47 },
	{ LCD_BAIC_VOL_48, TPS65132_VOL_48 },
	{ LCD_BAIC_VOL_49, TPS65132_VOL_49 },
	{ LCD_BAIC_VOL_50, TPS65132_VOL_50 },
	{ LCD_BAIC_VOL_51, TPS65132_VOL_51 },
	{ LCD_BAIC_VOL_52, TPS65132_VOL_52 },
	{ LCD_BAIC_VOL_53, TPS65132_VOL_53 },
	{ LCD_BAIC_VOL_54, TPS65132_VOL_54 },
	{ LCD_BAIC_VOL_55, TPS65132_VOL_55 },
	{ LCD_BAIC_VOL_56, TPS65132_VOL_56 },
	{ LCD_BAIC_VOL_57, TPS65132_VOL_57 },
	{ LCD_BAIC_VOL_58, TPS65132_VOL_58 },
	{ LCD_BAIC_VOL_59, TPS65132_VOL_59 },
	{ LCD_BAIC_VOL_60, TPS65132_VOL_60 }
};

/* bias ic: rt4801h */
static struct bias_ic_voltage rt4801h_vol_table[] = {
	{ LCD_BAIC_VOL_40, RT4801H_VOL_40 },
	{ LCD_BAIC_VOL_41, RT4801H_VOL_41 },
	{ LCD_BAIC_VOL_42, RT4801H_VOL_42 },
	{ LCD_BAIC_VOL_43, RT4801H_VOL_43 },
	{ LCD_BAIC_VOL_44, RT4801H_VOL_44 },
	{ LCD_BAIC_VOL_45, RT4801H_VOL_45 },
	{ LCD_BAIC_VOL_46, RT4801H_VOL_46 },
	{ LCD_BAIC_VOL_47, RT4801H_VOL_47 },
	{ LCD_BAIC_VOL_48, RT4801H_VOL_48 },
	{ LCD_BAIC_VOL_49, RT4801H_VOL_49 },
	{ LCD_BAIC_VOL_50, RT4801H_VOL_50 },
	{ LCD_BAIC_VOL_51, RT4801H_VOL_51 },
	{ LCD_BAIC_VOL_52, RT4801H_VOL_52 },
	{ LCD_BAIC_VOL_53, RT4801H_VOL_53 },
	{ LCD_BAIC_VOL_54, RT4801H_VOL_54 },
	{ LCD_BAIC_VOL_55, RT4801H_VOL_55 },
	{ LCD_BAIC_VOL_56, RT4801H_VOL_56 },
	{ LCD_BAIC_VOL_57, RT4801H_VOL_57 },
	{ LCD_BAIC_VOL_58, RT4801H_VOL_58 },
	{ LCD_BAIC_VOL_59, RT4801H_VOL_59 },
	{ LCD_BAIC_VOL_60, RT4801H_VOL_60 }
};

/* bias ic: lv52134a */
static struct bias_ic_voltage lv52134a_vol_table[] = {
	{ LCD_BAIC_VOL_41, LV52134A_VOL_41 },
	{ LCD_BAIC_VOL_42, LV52134A_VOL_42 },
	{ LCD_BAIC_VOL_43, LV52134A_VOL_43 },
	{ LCD_BAIC_VOL_44, LV52134A_VOL_44 },
	{ LCD_BAIC_VOL_45, LV52134A_VOL_45 },
	{ LCD_BAIC_VOL_46, LV52134A_VOL_46 },
	{ LCD_BAIC_VOL_47, LV52134A_VOL_47 },
	{ LCD_BAIC_VOL_48, LV52134A_VOL_48 },
	{ LCD_BAIC_VOL_49, LV52134A_VOL_49 },
	{ LCD_BAIC_VOL_50, LV52134A_VOL_50 },
	{ LCD_BAIC_VOL_51, LV52134A_VOL_51 },
	{ LCD_BAIC_VOL_52, LV52134A_VOL_52 },
	{ LCD_BAIC_VOL_53, LV52134A_VOL_53 },
	{ LCD_BAIC_VOL_54, LV52134A_VOL_54 },
	{ LCD_BAIC_VOL_55, LV52134A_VOL_55 },
	{ LCD_BAIC_VOL_56, LV52134A_VOL_56 },
	{ LCD_BAIC_VOL_57, LV52134A_VOL_57 }
};

static struct bias_ic_config bias_config[] = {
	{ "huawei,hw_tps65132",
	ARRAY_SIZE(tps65132_vol_table),
	&tps65132_vol_table[0] },
	{ "huawei,hw_rt4801h",
	ARRAY_SIZE(rt4801h_vol_table),
	&rt4801h_vol_table[0] },
	{ "huawei,hw_lv52134a",
	ARRAY_SIZE(lv52134a_vol_table),
	&lv52134a_vol_table[0] }
};

static int bias_ic_write_byte(struct bias_ic_device *pbias_ic,
	unsigned char reg, unsigned char data)
{
	int ret;

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n", __func__);
		return BIAS_RET_FAIL;
	}
	printk(KERN_INFO "%s: reg is 0x%x   data is 0x%x\n",
		__func__, reg, data);
	ret = i2c_smbus_write_byte_data(pbias_ic->client, reg, data);
	if (ret < 0)
		dev_err(&pbias_ic->client->dev,
			"failed to write 0x%.2x\n", reg);

	return ret;
}

static void bias_ic_get_value(struct bias_ic_device *pbias_ic,
	const char *chip_name, int pos_votage, int neg_votage)
{
	int i;
	int j;

	if ((pbias_ic == NULL) || (chip_name == NULL))
		return;

	for (i = 0; i < ARRAY_SIZE(bias_config); i++) {
		if (strcmp(chip_name, bias_config[i].name))
			continue;
		for (j = 0; j < bias_config[i].len; j++) {
			if (bias_config[i].vol_table[j].voltage ==
				pos_votage) {
				pbias_ic->bias_config.vpos_val =
					bias_config[i].vol_table[j].value;
				break;
			}
		}
		if (j == bias_config[i].len) {
			printk(KERN_INFO "not found vsp voltage, use default\n");
			pbias_ic->bias_config.vpos_val = DEFAULT_VOL_VAL;
		}

		for (j = 0; j < bias_config[i].len; j++) {
			if (bias_config[i].vol_table[j].voltage ==
				neg_votage) {
				pbias_ic->bias_config.vneg_val =
					bias_config[i].vol_table[j].value;
				break;
			}
		}

		if (j == bias_config[i].len) {
			printk(KERN_INFO "not found neg voltage, use default voltage\n");
			pbias_ic->bias_config.vneg_val = DEFAULT_VOL_VAL;
		}
		break;
	}

	if (i == ARRAY_SIZE(bias_config)) {
		printk(KERN_INFO "not found right voltage, use default voltage\n");
		pbias_ic->bias_config.vpos_val = DEFAULT_VOL_VAL;
		pbias_ic->bias_config.vneg_val = DEFAULT_VOL_VAL;
	}
}

static int parse_bias_ic_config(struct device_node *np,
	struct bias_ic_device *pbias_ic)
{
	int ret;
	unsigned int tmp_val = 0;

	if (pbias_ic == NULL)
		return BIAS_RET_FAIL;

	ret = of_property_read_u32(np, DTS_BIAS_IC_TYPE, &tmp_val);
	pbias_ic->bias_config.ic_type =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_POS_REG, &tmp_val);
	pbias_ic->bias_config.vpos_reg =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_NEG_REG, &tmp_val);
	pbias_ic->bias_config.vneg_reg =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_POS_MASK, &tmp_val);
	pbias_ic->bias_config.vpos_mask =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_NEG_MASK, &tmp_val);
	pbias_ic->bias_config.vneg_mask =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_STATE_REG, &tmp_val);
	pbias_ic->bias_config.state_reg =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_STATE_VAL, &tmp_val);
	pbias_ic->bias_config.state_val =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	ret = of_property_read_u32(np, DTS_BIAS_IC_STATE_MASK, &tmp_val);
	pbias_ic->bias_config.state_mask =
		(!ret ? (unsigned char)tmp_val : CONFIG_DEFAULT_VALUE);

	printk(KERN_INFO "ic_type is 0x%x!\n",
		pbias_ic->bias_config.ic_type);
	printk(KERN_INFO "vpos_reg is 0x%x!\n",
		pbias_ic->bias_config.vpos_reg);
	printk(KERN_INFO "vpos_mask is 0x%x!\n",
		pbias_ic->bias_config.vpos_mask);
	printk(KERN_INFO "vneg_reg is 0x%x!\n",
		pbias_ic->bias_config.vneg_reg);
	printk(KERN_INFO "vneg_mask is 0x%x!\n",
		pbias_ic->bias_config.vneg_mask);
	printk(KERN_INFO "state_reg is 0x%x!\n",
		pbias_ic->bias_config.state_reg);
	printk(KERN_INFO "state_val is 0x%x!\n",
		pbias_ic->bias_config.state_val);
	printk(KERN_INFO "state_mask is 0x%x!\n",
		pbias_ic->bias_config.state_mask);

	return BIAS_RET_OK;
}

static int get_bias_ic_name(char *buf, int len)
{
	struct device_node *np = NULL;
	char *tmp_name = NULL;
	int name_len;

	if (buf == NULL)
		return BIAS_RET_FAIL;

	np = of_find_compatible_node(NULL, NULL,
		DTS_COMP_LCDKIT_PANEL_TYPE);
	if (np == NULL) {
		printk(KERN_ERR "%s: not found device node %s!\n",
			__func__, DTS_COMP_LCDKIT_PANEL_TYPE);
		return BIAS_RET_FAIL;
	}

	tmp_name = (char *)of_get_property(np, "lcd_bias_ic_name", NULL);
	if (tmp_name == NULL) {
		printk(KERN_ERR "%s: parse lcd-bias-ic-name node fail!\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	name_len = strlen(tmp_name);
	if (!name_len) {
		printk(KERN_ERR "%s: tmp_name len is 0!\n", __func__);
		return BIAS_RET_FAIL;
	}

	if (name_len < len)
		memcpy(buf, tmp_name, name_len + 1);
	else
		memcpy(buf, tmp_name, len - 1);

	buf[len - 1] = 0;

	return BIAS_RET_OK;
}

static ssize_t bias_vsp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bias_ic_device *pchip = NULL;
	int ret;
	unsigned char val;

	if (buf == NULL)
		return -EINVAL;

	if (dev == NULL)
		return snprintf(buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return snprintf(buf, PAGE_SIZE, "data is null\n");

	if (pchip->bias_config.ic_type & BIAS_IC_READ_INHIBITION)
		return snprintf(buf, PAGE_SIZE, "read inhibition\n");

	ret = i2c_smbus_read_byte_data(pchip->client,
		pchip->bias_config.vpos_reg);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "bias i2c read vsp error\n");

	val = (unsigned char)ret;
	return snprintf(buf, PAGE_SIZE, "vsp = 0x%x\n", val);
}

static ssize_t bias_vsp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct bias_ic_device *pchip = NULL;
	unsigned char val;
	unsigned int input_val;
	int ret;

	if (buf == NULL) {
		printk(KERN_ERR "%s: buf is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = sscanf(buf, "vsp=0x%x", &input_val);
	if (ret < 0) {
		printk(KERN_ERR "%s: check input!\n", __func__);
		return -EINVAL;
	}

	val = (unsigned char)input_val;

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return -EINVAL;

	ret = bias_ic_write_byte(pchip, pchip->bias_config.vpos_reg, val);
	if (ret < 0) {
		printk(KERN_ERR "%s:i2c write vsp register error\n",
			__func__);
		return -EINVAL;
	}
	return size;
}

static ssize_t bias_vsn_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bias_ic_device *pchip = NULL;
	int ret;
	unsigned char val;

	if (buf == NULL)
		return -EINVAL;

	if (dev == NULL)
		return snprintf(buf, PAGE_SIZE, "dev is null\n");

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return snprintf(buf, PAGE_SIZE, "data is null\n");

	if (pchip->bias_config.ic_type & BIAS_IC_READ_INHIBITION)
		return snprintf(buf, PAGE_SIZE, "read inhibition\n");

	ret = i2c_smbus_read_byte_data(pchip->client,
		pchip->bias_config.vneg_reg);
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "bias i2c read vsn error\n");

	val = (unsigned char)ret;
	return snprintf(buf, PAGE_SIZE, "vsn = 0x%x\n", val);
}

static ssize_t bias_vsn_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct bias_ic_device *pchip = NULL;
	unsigned int input_val;
	unsigned char val;
	int ret;

	if (buf == NULL) {
		printk(KERN_ERR "%s: buf is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = sscanf(buf, "vsn=0x%x", &input_val);
	if (ret < 0) {
		printk(KERN_ERR "%s: check input!\n", __func__);
		return -EINVAL;
	}

	val = (unsigned char)input_val;

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL) {
		printk(KERN_ERR "%s: data is null!\n", __func__);
		return -EINVAL;
	}
	ret = bias_ic_write_byte(pchip, pchip->bias_config.vneg_reg, val);
	if (ret < 0) {
		printk(KERN_ERR "%s:i2c write vsn register error\n",
			__func__);
		return -EINVAL;
	}
	return size;
}

static DEVICE_ATTR(vsp, 0644, bias_vsp_show, bias_vsp_store);
static DEVICE_ATTR(vsn, 0644, bias_vsn_show, bias_vsn_store);

static struct attribute *bias_attributes[] = {
	&dev_attr_vsp.attr,
	&dev_attr_vsn.attr,
	NULL,
};

static const struct attribute_group bias_group = {
	.attrs = bias_attributes,
};

#if defined CONFIG_HUAWEI_DSM
static int bias_vpos_dsm(void)
{
	struct dsm_client *lcd_dclient = NULL;
	struct bias_bl_common_ops *bias_ops = NULL;

	bias_ops = bias_bl_get_ops();
	if (bias_ops == NULL) {
		printk(KERN_ERR "%s: can not get bias_ops!!\n", __func__);
		return BIAS_RET_FAIL;
	}

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n", __func__);
		return BIAS_RET_FAIL;
	}

	if (!bias_ops->get_lcd_dsm_client) {
		printk(KERN_ERR "%s: can not get get_lcd_dsm_client!\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	lcd_dclient = bias_ops->get_lcd_dsm_client();
	if (lcd_dclient) {
		if (!dsm_client_ocuppy(lcd_dclient)) {
			dsm_client_record(lcd_dclient,
				"set vsp failed! preg=0x%x, pval=0x%x, chip=%s.\n",
				pbias_ic->bias_config.vpos_reg,
				pbias_ic->bias_config.vpos_val,
				chip_name);
			dsm_client_notify(lcd_dclient,
				DSM_LCD_BIAS_I2C_ERROR_NO);
			return BIAS_RET_OK;
		}
	}
	return BIAS_RET_FAIL;
}

static int bias_vneg_dsm(void)
{
	struct dsm_client *lcd_dclient  = NULL;
	struct bias_bl_common_ops *bias_ops = NULL;

	bias_ops = bias_bl_get_ops();
	if (bias_ops == NULL) {
		printk(KERN_ERR "%s: can not get bias_ops!!\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	if (!bias_ops->get_lcd_dsm_client) {
		printk(KERN_ERR "%s: not get get_lcd_dsm_client func!!\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	lcd_dclient = bias_ops->get_lcd_dsm_client();
	if (lcd_dclient) {
		if (!dsm_client_ocuppy(lcd_dclient)) {
			dsm_client_record(lcd_dclient,
				"set neg failed! nreg=0x%x, nval=0x%x, chip=%s.\n",
				pbias_ic->bias_config.vneg_reg,
				pbias_ic->bias_config.vneg_val,
				chip_name);
			dsm_client_notify(lcd_dclient,
				DSM_LCD_BIAS_I2C_ERROR_NO);
			return BIAS_RET_OK;
		}
	}
	return BIAS_RET_FAIL;
}
#endif

static int bias_ic_set_vpos(void)
{
	int ret;
	struct bias_bl_common_ops *bias_ops = NULL;
#if defined CONFIG_HUAWEI_DSM
	static uint32_t s_set_voltage_vsp_count;
#endif

	bias_ops = bias_bl_get_ops();
	if (bias_ops == NULL) {
		printk(KERN_ERR "%s: can not get bias_ops!!\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n", __func__);
		return BIAS_RET_FAIL;
	}

	ret = bias_ic_write_byte(pbias_ic, pbias_ic->bias_config.vpos_reg,
		pbias_ic->bias_config.vpos_val);
	if (ret < 0) {
		printk(KERN_ERR "%s: set bias ic vsp failed!\n", __func__);
#if defined CONFIG_HUAWEI_DSM
		s_set_voltage_vsp_count++;
		if (s_set_voltage_vsp_count >=
			DMD_SET_VOLTAGE_VSP_FALI_EXPIRE_COUNT) {
			ret = bias_vpos_dsm();
			if (ret == BIAS_RET_OK)
				s_set_voltage_vsp_count = 0;
		}
#endif
		return BIAS_RET_FAIL;
	}
#if defined CONFIG_HUAWEI_DSM
	s_set_voltage_vsp_count = 0;
#endif

	return BIAS_RET_OK;
}

static int bias_ic_set_vneg(void)
{
	int ret;
	struct bias_bl_common_ops *bias_ops = NULL;
#if defined CONFIG_HUAWEI_DSM
	static uint32_t s_set_voltage_vsn_count;
#endif

	bias_ops = bias_bl_get_ops();
	if (bias_ops == NULL) {
		printk(KERN_ERR "%s: can not get bias_ops!!\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	ret = bias_ic_write_byte(pbias_ic,
		pbias_ic->bias_config.vneg_reg,
		pbias_ic->bias_config.vneg_val);
	if (ret < 0) {
		printk(KERN_ERR "%s: set bias ic vsn failed!\n",
			__func__);
#if defined CONFIG_HUAWEI_DSM
		s_set_voltage_vsn_count++;
		if (s_set_voltage_vsn_count >=
			DMD_SET_VOLTAGE_VSN_FALI_EXPIRE_COUNT) {
			ret = bias_vneg_dsm();
			if (ret == BIAS_RET_OK)
				s_set_voltage_vsn_count = 0;
		}
#endif
		return BIAS_RET_FAIL;
	}
#if defined CONFIG_HUAWEI_DSM
	s_set_voltage_vsn_count = 0;
#endif

	return BIAS_RET_OK;
}

static int bias_ic_set_state(void)
{
	int ret;
	unsigned char val;
	unsigned char value;

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n",
			__func__);
		return BIAS_RET_FAIL;
	}

	if (pbias_ic->bias_config.state_mask != 0) {
		ret = i2c_smbus_read_byte_data(pbias_ic->client,
			pbias_ic->bias_config.state_reg);
		if (ret < 0) {
			printk(KERN_ERR "%s: read bias ic state fail\n",
				__func__);
			return BIAS_RET_FAIL;
		}
		val = (unsigned char)ret;
		value = val & (~pbias_ic->bias_config.state_mask);
		value |= (pbias_ic->bias_config.state_val
			& pbias_ic->bias_config.state_mask);
		ret = bias_ic_write_byte(pbias_ic,
			pbias_ic->bias_config.state_reg, value);
		if (ret < 0) {
			printk(KERN_ERR "%s: set state register fail\n",
				__func__);
			return BIAS_RET_FAIL;
		}
	}

	return BIAS_RET_OK;
}

static int bias_ic_set_voltage(int vpos, int vneg)
{
	int ret = BIAS_RET_OK;

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n", __func__);
		return BIAS_RET_FAIL;
	}

	if (pbias_ic->bias_config.ic_type & BIAS_IC_RESUME_NEED_CONFIG) {
		if (g_vol_mapped == BIAS_FALSE) {
			bias_ic_get_value(pbias_ic, chip_name, vpos, vneg);
			g_vol_mapped = BIAS_TRUE;
		}
		ret = bias_ic_set_vpos();
		if (ret < 0)
			printk(KERN_ERR "%s: set bias ic vsp fail\n",
				__func__);
		ret = bias_ic_set_vneg();
		if (ret < 0)
			printk(KERN_ERR "%s: set bias ic vsn fail\n",
				__func__);
		ret = bias_ic_set_state();
		if (ret < 0)
			printk(KERN_ERR "%s: read bias ic state fail\n",
				__func__);

		ret = BIAS_RET_OK;
	}
	return ret;
}

static struct lcd_kit_bias_ops bias_ic_ops = {
	.set_bias_voltage = bias_ic_set_voltage,
};

static int bias_ic_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct bias_bl_common_ops *bias_ops = NULL;

	printk(KERN_INFO "enter %s\n", __func__);
	bias_ops = bias_bl_get_ops();
	if (bias_ops == NULL) {
		printk(KERN_ERR "%s: can not get bias_ops!!\n",
			__func__);
		return -EINVAL;
	}

	if (client == NULL) {
		printk(KERN_ERR "%s: client is NULL\n", __func__);
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, chip_name);
	if (np == NULL) {
		printk(KERN_ERR "%s: not found device node %s!\n",
			__func__, chip_name);
		return -ENODEV;
	}

	pbias_ic = devm_kzalloc(&client->dev,
		sizeof(struct bias_ic_device), GFP_KERNEL);

	if (pbias_ic == NULL) {
		printk(KERN_ERR "%s: pbias_ic is NULL\n", __func__);
		return -EINVAL;
	}

	pbias_ic->client = client;
	parse_bias_ic_config(np, pbias_ic);

	if (bias_class != NULL) {
		pbias_ic->dev = device_create(bias_class, NULL,
			0, "%s", "lcd_bias");
		if (IS_ERR(pbias_ic->dev)) {
			printk(KERN_ERR "%s: Unable to create device; errno = %ld\n",
				__func__, PTR_ERR(pbias_ic->dev));
			pbias_ic->dev = NULL;
			g_sysfs_created = BIAS_FALSE;
		} else {
			dev_set_drvdata(pbias_ic->dev, pbias_ic);
			g_sysfs_created = BIAS_TRUE;
			ret = sysfs_create_group(&pbias_ic->dev->kobj,
				&bias_group);
			if (ret) {
				printk(KERN_ERR "%s: Create bias sysfs group node failed!\n",
					__func__);
				device_destroy(bias_class, 0);
				g_sysfs_created = BIAS_FALSE;
			}
		}
	}

	lcd_kit_bias_register(&bias_ic_ops);

	if (bias_ops->set_hw_dev_detect)
		bias_ops->set_hw_dev_detect();

	return BIAS_RET_OK;
}

static int bias_ic_remove(struct i2c_client *client)
{
	if (g_sysfs_created)
		sysfs_remove_group(&pbias_ic->dev->kobj, &bias_group);

	device_destroy(bias_class, 0);
	if (client != NULL) {
		devm_kfree(&client->dev, pbias_ic);
		pbias_ic = NULL;
	}

	return BIAS_RET_OK;
}

static struct i2c_device_id bias_ic_common_id[] = {
	{ "bias_ic_common", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, bias_ic_common_id);

static struct of_device_id bias_ic_match_table[] = {
	{ .compatible = "bias_ic_common", },
	{},
};

static struct i2c_driver bias_ic_driver = {
	.probe = bias_ic_probe,
	.remove = bias_ic_remove,
	.driver = {
		.name = "bias_ic_common",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bias_ic_match_table),
	},
	.id_table = bias_ic_common_id,
};

static int __init bias_ic_module_init(void)
{
	int ret;
	char tmp_name[LCD_BIAS_IC_NAME_LEN] = {0};

	ret = get_bias_ic_name(tmp_name, sizeof(tmp_name));
	if (ret != BIAS_RET_OK)
		return BIAS_RET_OK;

	printk(KERN_INFO "%s: bias ic chip_name is %s\n", __func__, tmp_name);
	if (!strcmp(tmp_name, "default"))
		return BIAS_RET_OK;

	memset(chip_name, 0, LCD_BIAS_IC_NAME_LEN);
	memcpy(chip_name, tmp_name, LCD_BIAS_IC_NAME_LEN);
	chip_name[LCD_BIAS_IC_NAME_LEN-1] = 0;
	memcpy(bias_ic_common_id[0].name, chip_name, LCD_BIAS_I2C_NAME_SIZE - 1);
	bias_ic_common_id[0].name[LCD_BIAS_I2C_NAME_SIZE - 1] = 0;
	memcpy(bias_ic_match_table[0].compatible, chip_name,
		LCD_BIAS_IC_NAME_LEN);
	bias_ic_match_table[0].compatible[LCD_BIAS_IC_NAME_LEN - 1] = 0;
	bias_ic_driver.driver.name = chip_name;

	bias_class = class_create(THIS_MODULE, "lcd_bias");
	if (IS_ERR(bias_class)) {
		printk(KERN_ERR "%s: Unable to create bias class; errno = %ld\n",
			__func__, PTR_ERR(bias_class));
		bias_class = NULL;
	}

	return i2c_add_driver(&bias_ic_driver);
}

static void __exit bias_ic_module_exit(void)
{
	if (bias_class != NULL)
		class_destroy(bias_class);

	i2c_del_driver(&bias_ic_driver);
}

late_initcall(bias_ic_module_init);
module_exit(bias_ic_module_exit);

MODULE_DESCRIPTION("bias ic common driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
