/*
 * hisi_scharger_v300_common.c
 *
 * Description: HI6523 charger common inner api
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "hisi_scharger_v300_common.h"
#include <hisi_scharger_v300.h>
#include <huawei_platform/power/huawei_charger.h>
#include <linux/i2c.h>

 /* record SchargerV300 i2c trans error, and when need, notify it */
static void scharger_i2c_err_monitor(void)
{
	static int scharger_i2c_err_cnt = RESET_VAL_ZERO;

	scharger_i2c_err_cnt++;
	if (scharger_i2c_err_cnt >= I2C_ERR_MAX_COUNT) {
		scharger_i2c_err_cnt = RESET_VAL_ZERO;
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_I2C_ERR, NULL);
	}
}

int hi6523_write_block(struct hi6523_device_info *di, u8 *value, u8 reg,
		       unsigned int num_bytes)
{
	struct i2c_msg msg[1]; /* 1: the length of msg is 1 */
	int ret;

	if (di == NULL || value == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}
	*value = reg;
	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = num_bytes + 1;
	/* 1: the length of msg is 2 */
	ret = i2c_transfer(di->client->adapter, msg, 1);
	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		scharger_err("i2c_write failed to transfer all messages\n");
		scharger_i2c_err_monitor();
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

int hi6523_read_block(struct hi6523_device_info *di, u8 *value, u8 reg,
		      unsigned int num_bytes)
{
	struct i2c_msg msg[2]; /* 2: the length of msg is 2 */
	u8 buf;
	int ret;

	if (di == NULL || value == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}
	buf = reg;
	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = num_bytes;
	/* 2: the length of msg is 2 */
	ret = i2c_transfer(di->client->adapter, msg, 2);
	/* i2c_transfer returns number of messages transferred */
	if (ret != 2) {
		scharger_err("i2c_write failed to transfer all messages\n");
		scharger_i2c_err_monitor();
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

int hi6523_write_byte(u8 reg, u8 value)
{
	struct hi6523_device_info *di = get_hi6523_device();
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[2] = { 0 };

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return hi6523_write_block(di, temp_buffer, reg, 1);
}

int hi6523_read_byte(u8 reg, u8 *value)
{
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL || value == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}

	return hi6523_read_block(di, value, reg, 1);
}

int hi6523_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hi6523_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = hi6523_write_byte(reg, val);

	return ret;
}

int hi6523_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	if (value == NULL) {
		scharger_err("%s value is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret = hi6523_read_byte(reg, &val);
	if (ret < 0)
		return ret;
	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

static int hi6523_set_adc_channel(u32 channel)
{
	return hi6523_write_mask(CHG_ADC_CTRL_REG, CHG_ADC_CH_MSK,
				 CHG_ADC_CH_SHIFT, (u8)channel);
}

/* value: 1(enable) or 0(disable) */
static int hi6523_adc_enable(u32 enable)
{
	return hi6523_write_mask(CHG_ADC_CTRL_REG, CHG_ADC_EN_MSK,
				 CHG_ADC_EN_SHIFT, (u8)enable);
}

static int hi6523_get_adc_conv_status(u8 *value)
{
	return hi6523_read_mask(CHG_ADC_CONV_STATUS_REG,
				CHG_ADC_CONV_STATUS_MSK,
				CHG_ADC_CONV_STATUS_SHIFT, value);
}

int hi6523_get_adc_value(u32 chan, u32 *data)
{
	int ret, ret0, ret1, ret2;
	u8 reg = 0;
	int i;
	u8 adc_data[2] = {0}; /* the length of acd is 2 */
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL || data == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}

	mutex_lock(&di->adc_conv_lock);
	ret0 = hi6523_set_adc_channel(chan);
	ret1 = hi6523_adc_enable(CHG_ADC_EN);
	ret2 = hi6523_write_mask(CHG_ADC_START_REG, CHG_ADC_START_MSK,
				 CHG_ADC_START_SHIFT, TRUE);
	ret = (ret0 || ret1 || ret2);
	if (ret) {
		scharger_err("set covn fail! ret =%d\n", ret);
		hi6523_adc_enable(CHG_ADC_DIS);
		mutex_unlock(&di->adc_conv_lock);
		return -1;
	}
	/* The conversion result is ready after tCONV, max 10ms */
	for (i = 0; i < 10; i++) {
		ret = hi6523_get_adc_conv_status(&reg);
		if (ret) {
			scharger_err("HI6523 read ADC CONV STAT fail!\n");
			continue;
		}
		/* if ADC Conversion finished, hkadc_valid bit will be 1 */
		if (reg == 1)
			break;
		msleep(1); /* 1: 1mS for chip read */
	}

	if (i == 10) { /* 10: the final trial times */
		scharger_err("Wait for ADC CONV timeout!\n");
		hi6523_adc_enable(CHG_ADC_DIS);
		mutex_unlock(&di->adc_conv_lock);
		return -1;
	}
	/* 2: adc len */
	ret0 = hi6523_read_block(di, adc_data, CHG_ADC_DATA_REG, 2);
	ret1 = hi6523_adc_enable(CHG_ADC_DIS);
	ret = (ret0 || ret1);
	if (ret) {
		scharger_err("[%s]get ibus_ref_data fail,ret:%d\n",
			     __func__, ret);
		hi6523_adc_enable(CHG_ADC_DIS);
		mutex_unlock(&di->adc_conv_lock);
		return -1;
	}

	*data = (u32)adc_data[0] * 256 + adc_data[1]; /* 256: scale of adc */
	mutex_unlock(&di->adc_conv_lock);
	return 0;
}

static int reg_judge_v300(u8 reg_v1, u8 reg_v2, u8 reg_v3, u8 reg_v4)
{
	if ((reg_v1 & CHG_FCP_ACK) &&
		(reg_v1 & CHG_FCP_CMDCPL) &&
		!(reg_v2 & (CHG_FCP_CRCRX | CHG_FCP_PARRX))) {
		return 0;
	} else if (((reg_v1 & CHG_FCP_CRCPAR) ||
		(reg_v3 & CHG_FCP_INIT_HAND_FAIL) ||
		(reg_v4 & CHG_FCP_ENABLE_HAND_FAIL)) &&
		(reg_v2 & CHG_FCP_PROTSTAT)) {
		scharger_inf("%s:FCP_TRANSFER_FAIL, slave status changed: ISR1=0x%x, ISR2=0x%x, ISR3=0x%x, ISR4=0x%x\n",
			     __func__, reg_v1, reg_v2, reg_v3, reg_v4);
		return -1;
	} else if (reg_v1 & CHG_FCP_NACK) {
		scharger_inf("%s:FCP_TRANSFER_FAIL, slave nack: ISR1=0x%x, ISR2=0x%x\n",
			     __func__, reg_v1, reg_v2);
		return -1;
	} else if ((reg_v2 & CHG_FCP_CRCRX) ||
		(reg_v2 & CHG_FCP_PARRX) ||
		(reg_v3 & CHG_FCP_TAIL_HAND_FAIL)) {
		scharger_inf("%s:FCP_TRANSFER_FAIL, CRCRX_PARRX_ERROR:ISR1=0x%x, ISR2=0x%x, ISR3=0x%x\n",
			     __func__, reg_v1, reg_v2, reg_v3);
		return -1;
	} else {
		return 1; /* 1: indicates failure of reg judge */
	}
}

/*
 * check cmd transfer success or fail
 * Return: 0: success, -1: fail
 */
static int hi6523_fcp_cmd_transfer_check(void)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	int i = 0;
	int reg_ret = 1; /* set is 1 to distinguish ret state in reg_judge */
	u8 reg_val3 = 0;
	u8 reg_val4 = 0;
	int ret, ret0, ret1, ret2, ret3;
	/* read accp interrupt registers until value is not zero */
	do {
		usleep_range(12000, 13000); /* (12000, 13000): sleep range */
		ret0 = hi6523_read_byte(CHG_FCP_ISR1_REG, &reg_val1);
		ret1 = hi6523_read_byte(CHG_FCP_ISR2_REG, &reg_val2);
		ret2 = hi6523_read_byte(CHG_FCP_IRQ3_REG, &reg_val3);
		ret3 = hi6523_read_byte(CHG_FCP_IRQ4_REG, &reg_val4);
		ret = (ret0 || ret1 || ret2 || ret3);
		if (ret) {
			scharger_err("%s:reg read failed!\n", __func__);
			break;
		}
		if (reg_val1 || reg_val2) {
			reg_ret = reg_judge_v300(reg_val1, reg_val2, reg_val3, reg_val4);
			if (reg_ret == -1 || reg_ret == 0)
				return reg_ret;
			else
				scharger_inf("%s:FCP_TRANSFER_FAIL, ISR1=0x%x, ISR2=0x%x, ISR3=0x%x, total time = %dms\n",
					     __func__, reg_val1, reg_val2,
					     reg_val3, i * 10); /* 10: scale transformation */
		}
		i++;
	} while (i < FCP_ACK_RETRY_CYCLE);

	scharger_inf("%s:fcp adapter transfer time out,total time %d ms\n",
		     __func__, i * 10); /* 10: scale transformation */
	return -1;
}

/*
 * Description: disable accp protocol and enable again
 * Return: 0: success, -1: fail
 */
static void hi6523_fcp_protocol_restart(void)
{
	u8 reg_val = 0;
	int ret, i;
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return;
	}

	/* disable accp protocol */
	mutex_lock(&di->fcp_detect_lock);
	(void)hi6523_write_mask(CHG_FCP_CTRL_REG, CHG_FCP_EN_MSK,
				CHG_FCP_EN_SHIFT, FALSE);
	usleep_range(9000, 10000); /* (9000, 10000): sleep range */
	(void)hi6523_write_mask(CHG_FCP_CTRL_REG, CHG_FCP_EN_MSK,
				CHG_FCP_EN_SHIFT, TRUE);
	/* detect hisi fcp charger, wait for ping succ */
	for (i = 0; i < HI6523_RESTART_TIME; i++) {
		usleep_range(9000, 10000); /* (9000, 10000): sleep range */
		ret = hi6523_read_byte(CHG_FCP_STATUS_REG, &reg_val);
		if (ret) {
			scharger_err("%s:read det attach err,ret:%d\n",
				     __func__, ret);
			continue;
		}

		if (((reg_val & (CHG_FCP_DVC_MSK | CHG_FCP_ATTATCH_MSK)) ==
			CHG_FCP_SLAVE_GOOD))
			break;
	}

	if (i == HI6523_RESTART_TIME) {
		scharger_err("%s:wait for slave fail\n", __func__);
		mutex_unlock(&di->fcp_detect_lock);
		return;
	}
	mutex_unlock(&di->fcp_detect_lock);
	scharger_err("%s:disable and enable fcp protocol accp status is 0x%x\n",
		     __func__, reg_val);
}

int hi6523_fcp_adapter_reg_read(u8 *val, u8 reg)
{
	int i;
	int ret2 = 0;
	int ret3 = 0;
	int ret, ret0, ret1, ret4, ret5, ret6;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL || val == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}
	mutex_lock(&di->accp_adapter_reg_lock);
	for (i = 0; i < FCP_RETRY_TIME; i++) {
		/* before send cmd, clear accp interrupt registers */
		ret0 = hi6523_read_byte(CHG_FCP_ISR1_REG, &reg_val1);
		ret1 = hi6523_read_byte(CHG_FCP_ISR2_REG, &reg_val2);
		if (reg_val1 != 0)
			ret2 = hi6523_write_byte(CHG_FCP_ISR1_REG, reg_val1);

		if (reg_val2 != 0)
			ret3 = hi6523_write_byte(CHG_FCP_ISR2_REG, reg_val2);

		ret4 = hi6523_write_byte(CHG_FCP_CMD_REG, CHG_FCP_CMD_SBRRD);
		ret5 = hi6523_write_byte(CHG_FCP_ADDR_REG, reg);
		ret6 = hi6523_write_mask(CHG_FCP_CTRL_REG, CHG_FCP_SNDCMD_MSK,
					 CHG_FCP_SNDCMD_SHIFT, CHG_FCP_EN);
		ret = (ret0 || ret1 || ret2 || ret3 || ret4 || ret5 || ret6);
		if (ret) {
			scharger_err("%s:write error, ret:%d\n", __func__, ret);
			mutex_unlock(&di->accp_adapter_reg_lock);
			return HI6523_FAIL;
		}

		/* check cmd transfer success or fail */
		if (hi6523_fcp_cmd_transfer_check() == 0) {
			/* recived data from adapter */
			ret = hi6523_read_byte(CHG_FCP_RDATA_REG, val);
			break;
		}
		hi6523_fcp_protocol_restart();
	}
	if (i == FCP_RETRY_TIME) {
		scharger_err("%s:ack error,retry %d times\n", __func__, i);
		ret = HI6523_FAIL;
	}
	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

int hi6523_fcp_adapter_reg_write(u8 val, u8 reg)
{
	int retval = 0;
#define RET_SIZE_8 8
	int ret[RET_SIZE_8] = {0};
	int i, j;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	mutex_lock(&di->accp_adapter_reg_lock);
	for (i = 0; i < FCP_RETRY_TIME; i++) {
		/* before send cmd, clear accp interrupt registers */
		ret[0] = hi6523_read_byte(CHG_FCP_ISR1_REG, &reg_val1);
		ret[1] = hi6523_read_byte(CHG_FCP_ISR2_REG, &reg_val2);
		if (reg_val1 != 0)
			ret[2] = hi6523_write_byte(CHG_FCP_ISR1_REG, reg_val1);

		if (reg_val2 != 0)
			ret[3] = hi6523_write_byte(CHG_FCP_ISR2_REG, reg_val2);

		ret[4] = hi6523_write_byte(CHG_FCP_CMD_REG, CHG_FCP_CMD_SBRWR);
		ret[5] = hi6523_write_byte(CHG_FCP_ADDR_REG, reg);
		ret[6] = hi6523_write_byte(CHG_FCP_WDATA_REG, val);
		ret[7] = hi6523_write_mask(CHG_FCP_CTRL_REG,
					   CHG_FCP_SNDCMD_MSK,
					   CHG_FCP_SNDCMD_SHIFT,
					   CHG_FCP_EN);

		for (j = 0; j < RET_SIZE_8; j++) {
			if (ret[j]) {
				scharger_err("%s: write error ret is %d\n",
					     __func__, retval);
				mutex_unlock(&di->accp_adapter_reg_lock);
				return HI6523_FAIL;
			}
		}

		/* check cmd transfer success or fail */
		if (hi6523_fcp_cmd_transfer_check() == 0)
			break;

		hi6523_fcp_protocol_restart();
	}
	if (i == FCP_RETRY_TIME) {
		scharger_err("%s:ack error,retry %d times\n", __func__, i);
		retval = HI6523_FAIL;
	}
	mutex_unlock(&di->accp_adapter_reg_lock);
	return retval;
}

#define CONFIG_SYSFS_SCHG
#ifdef CONFIG_SYSFS_SCHG
/*
 * There are a numerous options that are configurable on the HI6523
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
static ssize_t hi6523_sysfs_show(struct device *dev,
				 struct device_attribute *attr, char *buf);
static ssize_t hi6523_sysfs_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count);
#define hi6523_sysfs_field(_name, r, f, m, store) \
{							\
	.attr = __ATTR(_name, m, hi6523_sysfs_show, store), \
	.reg = CHG_##r##_REG, .mask = CHG_##f##_MSK, .shift = CHG_##f##_SHIFT, \
}

#define hi6523_sysfs_field_rw(_name, r, f) \
	hi6523_sysfs_field(_name, r, f, 0640, hi6523_sysfs_store)

struct hi6523_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
	u8 mask;
	u8 shift;
};

static struct hi6523_sysfs_field_info hi6523_sysfs_field_tbl[] = {
	/* sysfs name reg field in reg */
	hi6523_sysfs_field_rw(en_hiz, HIZ_CTRL, HIZ_ENABLE),
	hi6523_sysfs_field_rw(iinlim, INPUT_SOURCE, ILIMIT),
	hi6523_sysfs_field_rw(reg_addr, NONE, NONE),
	hi6523_sysfs_field_rw(reg_value, NONE, NONE),
	hi6523_sysfs_field_rw(adapter_reg, NONE, NONE),
	hi6523_sysfs_field_rw(adapter_val, NONE, NONE),
};

static struct attribute *hi6523_sysfs_attrs[ARRAY_SIZE(hi6523_sysfs_field_tbl) + 1];

static const struct attribute_group hi6523_sysfs_attr_group = {
	.attrs = hi6523_sysfs_attrs,
};

static void hi6523_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(hi6523_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		hi6523_sysfs_attrs[i] = &hi6523_sysfs_field_tbl[i].attr.attr;

	hi6523_sysfs_attrs[limit] = NULL; /* Has additional entry for this */
}

static struct hi6523_sysfs_field_info *hi6523_sysfs_field_lookup(
							const char *name)
{
	int i;
	int limit = ARRAY_SIZE(hi6523_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		if (!strncmp(name, hi6523_sysfs_field_tbl[i].attr.attr.name,
							strlen(name)))
			break;

	if (i >= limit)
		return NULL;

	return &hi6523_sysfs_field_tbl[i];
}

static ssize_t hi6523_sysfs_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct hi6523_sysfs_field_info *info = NULL;
	struct hi6523_sysfs_field_info *info2 = NULL;
#ifdef CONFIG_HISI_DEBUG_FS
	int ret;
#endif
	u8 v;
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	info = hi6523_sysfs_field_lookup(attr->attr.name);
	if (info == NULL)
		return -EINVAL;

	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr")))
		return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", info->reg);

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = hi6523_sysfs_field_lookup("reg_addr");
		if (info2 == NULL)
			return -EINVAL;
		info->reg = info2->reg;
	}

	if (!strncmp("adapter_reg", attr->attr.name, strlen("adapter_reg")))
		return scnprintf(buf, PAGE_SIZE, "0x%x\n",
				 di->sysfs_fcp_reg_addr);

	if (!strncmp("adapter_val", attr->attr.name, strlen("adapter_val"))) {
#ifdef CONFIG_HISI_DEBUG_FS
		ret = hi6523_fcp_adapter_reg_read(&v, di->sysfs_fcp_reg_addr);
		scharger_inf("sys read fcp adapter reg 0x%x, v 0x%x\n",
			     di->sysfs_fcp_reg_addr, v);
		if (ret)
			return ret;
#else
		v = 0;
#endif
		return scnprintf(buf, PAGE_SIZE, "0x%x\n", v);
	}

#ifdef CONFIG_HISI_DEBUG_FS
	ret = hi6523_read_mask(info->reg, info->mask, info->shift, &v);
	if (ret)
		return ret;
#else
	v = 0;
#endif

	return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", v);
}

static ssize_t hi6523_sysfs_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct hi6523_sysfs_field_info *info = NULL;
	struct hi6523_sysfs_field_info *info2 = NULL;
	int ret;
	u8 v;
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	info = hi6523_sysfs_field_lookup(attr->attr.name);
	if (info == NULL)
		return -EINVAL;

	ret = kstrtou8(buf, 0, &v);
	if (ret < 0)
		return ret;

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = hi6523_sysfs_field_lookup("reg_addr");
		if (info2 == NULL)
			return -EINVAL;
		info->reg = info2->reg;
	}
	if (!strncmp(("reg_addr"), attr->attr.name, strlen("reg_addr"))) {
		if (v < (u8)HI6523_REG_TOTAL_NUM) {
			info->reg = v;
			return count;
		} else {
			return -EINVAL;
		}
	}

	if (!strncmp(("adapter_reg"), attr->attr.name, strlen("adapter_reg"))) {
		di->sysfs_fcp_reg_addr = (u8)v;
		return count;
	}
	if (!strncmp(("adapter_val"), attr->attr.name, strlen("adapter_val"))) {
		di->sysfs_fcp_reg_val = (u8)v;
#ifdef CONFIG_HISI_DEBUG_FS
		ret = hi6523_fcp_adapter_reg_write(di->sysfs_fcp_reg_val,
						   di->sysfs_fcp_reg_addr);
		scharger_inf("sys write fcp adapter reg 0x%x, v 0x%x\n",
			     di->sysfs_fcp_reg_addr, di->sysfs_fcp_reg_val);

		if (ret)
			return ret;
#endif
		return count;
	}

#ifdef CONFIG_HISI_DEBUG_FS
	ret = hi6523_write_mask(info->reg, info->mask, info->shift, v);
	if (ret)
		return ret;
#endif
	return count;
}

int hi6523_sysfs_create_group(struct hi6523_device_info *di)
{
	hi6523_sysfs_init_attrs();
	if (!power_sysfs_create_link_group("hw_power", "charger", "HI6523",
					   di->dev, &hi6523_sysfs_attr_group))
		return -1;

	return 0;
}

void hi6523_sysfs_remove_group(struct hi6523_device_info *di)
{
	(void)power_sysfs_remove_link_group("hw_power", "charger", "HI6523",
					    di->dev, &hi6523_sysfs_attr_group);
}
#else

int hi6523_sysfs_create_group(struct hi6523_device_info *di)
{
	return 0;
}

inline void hi6523_sysfs_remove_group(struct hi6523_device_info *di)
{
	return;
}
#endif
