// SPDX-License-Identifier: GPL-2.0
/*
 * hl7139.c
 *
 * hl7139 driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "hl7139.h"
#include "hl7139_i2c.h"
#include "hl7139_protocol.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <chipset_common/hwpower/common_module/power_delay.h>

#define HWLOG_TAG hl7139_chg
HWLOG_REGIST();

#define BUF_LEN                             80
#define HL7139_PAGE0_NUM                    0x12
#define HL7139_PAGE1_NUM                    0x07
#define HL7139_PAGE0_BASE                   HL7139_DEVICE_ID_REG
#define HL7139_PAGE1_BASE                   HL7139_SCP_CTL_REG
#define HL7139_DBG_VAL_SIZE                 6

static int hl7139_reg_init(void *dev_data);
static void hl7139_lvc_opt_regs(void *dev_data);
static void hl7139_sc_opt_regs(void *dev_data);

static void hl7139_dump_register(void *dev_data)
{
	u8 i;
	int ret;
	u8 val = 0;
	u16 data = 0;
	struct hl7139_device_info *di = dev_data;

	for (i = HL7139_DEVICE_ID_REG; i <= HL7139_TRACK_OV_UV_REG; i++) {
		if ((i == HL7139_INT_REG) || (i == HL7139_STATUS_A_REG) ||
			(i == HL7139_STATUS_B_REG))
			continue;
		ret = hl7139_read_byte(di, i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");
		hwlog_info("reg [%x]=0x%x\n", i, val);
	}

	ret = hl7139_read_word(di, ADC_VIN_1_REG, &data);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("ADC_VIN=0x%x\n", data);
	ret = hl7139_read_word(di, ADC_IBAT_1_REG, &data);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("ADC_IBAT=0x%x\n", data);
	ret = hl7139_read_word(di, ADC_IIN_1_REG, &data);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("ADC_IIN=0x%x\n", data);
	ret = hl7139_read_word(di, ADC_VBAT_1_REG, &data);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("ADC_VBAT=0x%x\n", data);

	ret = hl7139_read_byte(di, ADC_TDIE_0_REG, &val);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("ADC_TDIE=0x%x\n", val);

	for (i = HL7139_SCP_CTL_REG; i <= HL7139_SCP_STIMER_REG; i++) {
		if ((i == HL7139_SCP_ISR1_REG) || (i == HL7139_SCP_ISR2_REG))
			continue;
		ret = hl7139_read_byte(di, i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");
		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}

static int hl7139_reg_reset(void *dev_data)
{
	int ret;
	u8 reg = 0;
	struct hl7139_device_info *di = dev_data;

	hl7139_write_mask(di, HL7139_CTRL2_REG, HL7139_CTRL2_SFT_RST_MASK,
		HL7139_CTRL2_SFT_RST_SHIFT, HL7139_CTRL2_SFT_RST_ENABLE);
	power_usleep(DT_USLEEP_10MS); /* wait soft reset ready */
	ret = hl7139_read_byte(di, HL7139_CTRL2_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("reg_reset [%x]=0x%x\n", HL7139_CTRL2_REG, reg);
	return 0;
}

static int hl7139_reg_reset_and_init(void *dev_data)
{
	int ret;

	if (!dev_data) {
		hwlog_err("dev_data is null\n");
		return -EPERM;
	}

	ret = hl7139_reg_reset(dev_data);
	ret += hl7139_reg_init(dev_data);

	return ret;
}

static int hl7139_lvc_charge_enable(int enable, void *dev_data)
{
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	hwlog_info("enter lvc_charge_enable\n");
	hl7139_lvc_opt_regs(di);

	ret = hl7139_write_mask(di, HL7139_CTRL3_REG, HL7139_CTRL3_DEV_MODE_MASK,
		HL7139_CTRL3_DEV_MODE_SHIFT, HL7139_CTRL3_BPMODE);
	if (ret)
		return -EPERM;

	ret = hl7139_write_mask(di, HL7139_CTRL0_REG, HL7139_CTRL0_CHG_EN_MASK,
		HL7139_CTRL0_CHG_EN_SHIFT, enable);
	if (ret)
		return -EPERM;

	return 0;
}

static int hl7139_sc_charge_enable(int enable, void *dev_data)
{
	u8 reg;
	int ret;
	struct hl7139_device_info *di = dev_data;
	u8 value = enable ? HL7139_CTRL0_CHG_EN : HL7139_CTRL0_CHG_OFF;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	hwlog_info("enter hl7139_sc_charge_enable\n");
	hl7139_sc_opt_regs(di);

	ret = hl7139_write_mask(di, HL7139_CTRL3_REG, HL7139_CTRL3_DEV_MODE_MASK,
		HL7139_CTRL3_DEV_MODE_SHIFT, HL7139_CTRL3_CPMODE);
	if (ret)
		return -EPERM;

	ret = hl7139_write_mask(di, HL7139_CTRL0_REG, HL7139_CTRL0_CHG_EN_MASK,
		HL7139_CTRL0_CHG_EN_SHIFT, value);
	if (ret)
		return -EPERM;

	ret = hl7139_read_byte(di, HL7139_CTRL0_REG, &reg);
	hwlog_info("charge_enable [%x]=0x%x\n", HL7139_CTRL0_REG, reg);

	return 0;
}

static int hl7139_discharge(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (di->ignore_discharge_flag) {
		hwlog_info("sc8545_discharge return\n");
		return 0;
	};

	/* VBUS_PD : 0(auto working), 1(manual pull down) */
	ret = hl7139_write_mask(di, HL7139_VBUS_OVP_REG,
		HL7139_VBUS_OVP_VBUS_PD_EN_MASK,
		HL7139_VBUS_OVP_VBUS_PD_EN_SHIFT, value);
	if (ret)
		return -EPERM;

	ret = hl7139_read_byte(di, HL7139_VBUS_OVP_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("discharge [%x]=0x%x\n", HL7139_VBUS_OVP_REG, reg);
	return 0;
}

static int hl7139_is_device_close(void *dev_data)
{
	u8 reg = 0;
	u8 value;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	ret = hl7139_read_byte(di, HL7139_CTRL3_REG, &reg);
	if (ret)
		return 1;

	value = (reg & HL7139_CTRL3_DEV_MODE_MASK) >> HL7139_CTRL3_DEV_MODE_SHIFT;
	if ((value < HL7139_CTRL3_CPMODE) || (value > HL7139_CTRL3_BPMODE))
		return 1;

	return 0;
}

static int hl7139_get_device_id(void *dev_data)
{
	u8 part_info = 0;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (di->get_id_time == HL7139_USED)
		return di->device_id;

	di->get_id_time = HL7139_USED;
	ret = hl7139_read_byte(di, HL7139_DEVICE_ID_REG, &part_info);
	if (ret) {
		di->get_id_time = HL7139_NOT_USED;
		hwlog_err("get_device_id read fail\n");
		return -EPERM;
	}
	hwlog_info("get_device_id [%x]=0x%x\n", HL7139_DEVICE_ID_REG, part_info);

	part_info = part_info & HL7139_DEVICE_ID_DEV_ID_MASK;
	switch (part_info) {
	case HL7139_DEVICE_ID_HL7139:
		di->device_id = SWITCHCAP_HL7139;
		break;
	default:
		di->device_id = -1;
		hwlog_err("switchcap get dev_id fail\n");
		break;
	}

	hwlog_info("device_id : 0x%x\n", di->device_id);

	return di->device_id;
}

static int hl7139_get_revision_id(void *dev_data)
{
	u8 rev_info = 0;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (di->get_rev_time == HL7139_USED)
		return di->rev_id;

	di->get_rev_time = HL7139_USED;
	ret = hl7139_read_byte(di, HL7139_DEVICE_ID_REG, &rev_info);
	if (ret) {
		di->get_rev_time = HL7139_NOT_USED;
		hwlog_err("get_revision_id read fail\n");
		return -EPERM;
	}
	hwlog_info("get_revision_id [%x]=0x%x\n", HL7139_DEVICE_ID_REG, rev_info);

	di->rev_id = (rev_info & HL7139_DEVICE_ID_DEV_REV_MASK) >> HL7139_DEVICE_ID_DEV_REV_SHIFT;

	hwlog_info("revision_id : 0x%x\n", di->rev_id);
	return di->rev_id;
}

static int hl7139_get_vbat_mv(void *dev_data)
{
	s16 voltage;
	u8 val[ADC_REG_NUM] = {0}; /* read two bytes */
	u8 reg_high;
	u8 reg_low;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	/* get the values of two registers: vbat_high and vbat_low */
	if (hl7139_read_block(di, val, ADC_VBAT_0_REG, ADC_REG_NUM))
		return -EPERM;
	reg_high = val[ADC_REG_HIGH];
	reg_low = val[ADC_REG_LOW] & ADC_GET_VALID_NUM;
	hwlog_info("vbat_high=0x%x, vbat_low=0x%x\n", reg_high, reg_low);

	/* VBAT ADC LBS: 1.25mV, vbat_low: bit4 to bit7 is null */
	voltage = ((reg_high << ADC_INVALID_BIT) + reg_low) * 125 / 100;

	return (int)(voltage);
}

static int hl7139_get_ibat_ma(int *ibat, void *dev_data)
{
	s16 curr;
	u8 val[ADC_REG_NUM] = {0}; /* read two bytes */
	u8 reg_high;
	u8 reg_low;
	struct hl7139_device_info *di = dev_data;

	if (!ibat || !di) {
		hwlog_err("ibat or di is null\n");
		return -EPERM;
	}

	/* get the values of two registers: ibat_high and ibat_low */
	if (hl7139_read_block(di, val, ADC_IBAT_0_REG, ADC_REG_NUM))
		return -EPERM;
	reg_high = val[ADC_REG_HIGH];
	reg_low = val[ADC_REG_LOW] & ADC_GET_VALID_NUM;
	hwlog_info("ibat_high=0x%x, ibat_low=0x%x\n", reg_high, reg_low);

	/* IBAT ADC LBS: 2.2mA, ibat_low: bit4 to bit7 is null */
	curr = ((reg_high << ADC_INVALID_BIT) + reg_low) * 220 / 100;
	*ibat = ((int)curr) * di->sense_r_config / di->sense_r_actual;

	return 0;
}

static int hl7139_get_ibus_ma(int *iin, void *dev_data)
{
	s16 curr;
	u8 val[ADC_REG_NUM] = {0}; /* read two bytes */
	u8 reg_high;
	u8 reg_low;
	u16 value = 0;
	int flag;
	struct hl7139_device_info *di = dev_data;

	if (!iin || !di) {
		hwlog_err("iin or di is null\n");
		return -EPERM;
	}

	/* get the values of two registers: ibus_high and ibus_low */
	if (hl7139_read_block(di, val, ADC_IIN_0_REG, ADC_REG_NUM))
		return -EPERM;
	reg_high = val[ADC_REG_HIGH];
	reg_low = val[ADC_REG_LOW] & ADC_GET_VALID_NUM;
	hwlog_info("ibus_high=0x%x, ibus_low=0x%x\n", reg_high, reg_low);

	/* ibus_low: bit4 to bit7 is null */
	curr = (reg_high << ADC_INVALID_BIT) + reg_low;
	hl7139_read_byte(di, HL7139_CTRL3_REG, &value);
	flag = (value & HL7139_CTRL3_DEV_MODE_MASK) >> HL7139_CTRL3_DEV_MODE_SHIFT;
	if (flag == HL7139_CTRL3_BPMODE)
		*iin = (int)curr * 220 / 100; /* IBUS ADC LBS: 2.2mA */
	if (flag == HL7139_CTRL3_CPMODE)
		*iin = (int)curr * 110 / 100; /* IBUS ADC LBS: 1.1mA */

	return 0;
}

int hl7139_get_vbus_mv(int *vin, void *dev_data)
{
	s16 voltage;
	u8 val[ADC_REG_NUM] = {0}; /* read two bytes */
	u8 reg_high;
	u8 reg_low;
	struct hl7139_device_info *di = dev_data;

	if (!vin || !di) {
		hwlog_err("vin or di is null\n");
		return -EPERM;
	}

	/* get the values of two registers: vbus_high and vbus_low */
	if (hl7139_read_block(di, val, ADC_VIN_0_REG, ADC_REG_NUM))
		return -EPERM;
	reg_high = val[ADC_REG_HIGH];
	reg_low = val[ADC_REG_LOW] & ADC_GET_VALID_NUM;
	hwlog_info("vbus_high=0x%x, vbus_low=0x%x\n", reg_high, reg_low);

	/* VBUS ADC LBS: 4mV, vbus_low: bit4 to bit7 is null */
	voltage = ((reg_high << ADC_INVALID_BIT) + reg_low) * HL7139_ADC_RESOLUTION_4;
	*vin = (int)(voltage);

	return 0;
}

int hl7139_get_vbus_uvp_status(void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct hl7139_device_info *di = dev_data;

	ret = hl7139_read_byte(di, HL7139_STATUS_C_REG, &reg);
	if (ret)
		return -EPERM;
	hwlog_info("STATUS_C [%x]=0x%x\n", HL7139_STATUS_C_REG, reg);

	return (reg & HL7139_STATUS_C_VBUS_GOOD_S_MASK) ? 0 : HL7139_STATUS_C_VBUS_UVP_MODE;
}

static int hl7139_get_device_temp(int *temp, void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (!temp || !di) {
		hwlog_err("temp or di is null\n");
		return -EPERM;
	}

	ret = hl7139_read_byte(di, ADC_TDIE_0_REG, &reg);
	if (ret)
		return -EPERM;
	hwlog_info("ADC_TDIE=0x%x\n", reg);

	/* TDIE_ADC LSB: 0.0625C */
	*temp = (int)reg * 625 / 1000;

	return 0;
}

static int hl7139_set_nwatchdog(int disable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = disable ? 0x1 : 0x0;
	struct hl7139_device_info *di = dev_data;

	if (di->rev_id == 0) {
		hwlog_info("force set watchdog to disable\n");
		value = 1;
	}

	ret = hl7139_write_mask(di, HL7139_CTRL2_REG, HL7139_CTRL2_WD_DIS_MASK,
		HL7139_CTRL2_WD_DIS_SHIFT, value);
	if (ret)
		return -EPERM;

	ret = hl7139_read_byte(di, HL7139_CTRL2_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("watchdog [%x]=0x%x\n", HL7139_CTRL2_REG, reg);
	return 0;
}

static int hl7139_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	int ret;
	u8 reg = 0;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (time >= HL7139_WD_TMR_40000MS)
		val = 7;
	else if (time >= HL7139_WD_TMR_20000MS)
		val = 6;
	else if (time >= HL7139_WD_TMR_10000MS)
		val = 5;
	else if (time >= HL7139_WD_TMR_5000MS)
		val = 4;
	else if (time >= HL7139_WD_TMR_2000MS)
		val = 3;
	else if (time >= HL7139_WD_TMR_1000MS)
		val = 2;
	else if (time >= HL7139_WD_TMR_500MS)
		val = 1;
	else
		val = 0;

	ret = hl7139_write_mask(di, HL7139_CTRL2_REG, HL7139_CTRL2_WD_TMR_MASK,
		HL7139_CTRL2_WD_TMR_SHIFT, val);
	if (ret)
		return -EPERM;

	ret = hl7139_read_byte(di, HL7139_CTRL2_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("config_watchdog_ms [%x]=0x%x\n", HL7139_CTRL2_REG, reg);
	return 0;
}

static int hl7139_config_rlt_uvp_ref(int rltuvp_rate, void *dev_data)
{
	u8 value;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (rltuvp_rate >= HL7139_TRACK_OV_UV_TRACK_UV_MAX)
		rltuvp_rate = HL7139_TRACK_OV_UV_TRACK_UV_MAX;

	value = (u8)((rltuvp_rate - HL7139_TRACK_OV_UV_TRACK_UV_MIN) /
		HL7139_TRACK_OV_UV_TRACK_UV_STEP);

	ret = hl7139_write_mask(di, HL7139_TRACK_OV_UV_REG, HL7139_TRACK_OV_UV_TRACK_UV_MASK,
		HL7139_TRACK_OV_UV_TRACK_UV_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_rlt_uvp_ref [%x]=0x%x\n", HL7139_TRACK_OV_UV_REG, value);
	return 0;
}

static int hl7139_config_rlt_ovp_ref(int rltovp_rate, void *dev_data)
{
	u8 value;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (rltovp_rate >= HL7139_TRACK_OV_UV_TRACK_OV_MAX)
		rltovp_rate = HL7139_TRACK_OV_UV_TRACK_OV_MAX;

	value = (u8)((rltovp_rate - HL7139_TRACK_OV_UV_TRACK_OV_MIN) /
		HL7139_TRACK_OV_UV_TRACK_OV_STEP);

	ret = hl7139_write_mask(di, HL7139_TRACK_OV_UV_REG, HL7139_TRACK_OV_UV_TRACK_OV_MASK,
		HL7139_TRACK_OV_UV_TRACK_OV_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_rlt_ovp_ref [%x]=0x%x\n", HL7139_TRACK_OV_UV_REG, value);
	return 0;
}

static int hl7139_config_vbat_ovp_mv(int ovp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	int vbat_regulation;
	struct hl7139_device_info *di = dev_data;

	/* config vbat ovp offset */
	ret = hl7139_write_mask(di, HL7139_REG_CTRL1_REG, HL7139_REG_CTRL1_VBAT_OVP_TH_MASK,
		HL7139_REG_CTRL1_VBAT_OVP_TH_SHIFT, 0); /* vbat_ovp_offset 80mv */
	if (ret)
		return -EPERM;
	hwlog_info("config_vbat_ovp_offset_mv [%x]=0x%x\n", HL7139_REG_CTRL1_REG, value);

	/* config vbat regulation */
	vbat_regulation = ovp_threshold - HL7139_VBAT_OVP_TH_ABOVE_80MV;
	if (vbat_regulation < HL7139_VBAT_REG_TH_MIN)
		vbat_regulation = HL7139_VBAT_REG_TH_MIN;

	if (vbat_regulation > HL7139_VBAT_REG_TH_MAX)
		vbat_regulation = HL7139_VBAT_REG_TH_MAX;

	value = (u8)((vbat_regulation - HL7139_VBAT_REG_TH_MIN) /
		HL7139_VBAT_REG_TH_STEP);

	ret = hl7139_write_mask(di, HL7139_VBAT_REG_REG, HL7139_VBAT_REG_VBAT_REG_TH_MASK,
		HL7139_VBAT_REG_VBAT_REG_TH_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_vbat_reg_ref_mv [%x]=0x%x\n", HL7139_VBAT_REG_REG, value);
	return 0;
}

static int hl7139_config_ibat_ocp_ma(int ocp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	int ibat_regulation;
	struct hl7139_device_info *di = dev_data;

	/* config ibat ocp offset */
	value = (HL7139_IBAT_OCP_TH_ABOVE_500MA - HL7139_IBAT_OCP_TH_ABOVE_200MA) /
		HL7139_IBAT_OCP_TH_ABOVE_STEP;
	ret = hl7139_write_mask(di, HL7139_REG_CTRL1_REG, HL7139_REG_CTRL1_IBAT_OCP_TH_MASK,
		HL7139_REG_CTRL1_IBAT_OCP_TH_SHIFT, value); /* ibat ocp offset 500ma */
	if (ret)
		return -EPERM;
	hwlog_info("config_ibat_ocp_offset [%x]=0x%x\n", HL7139_REG_CTRL1_REG, value);

	/* config ibat regulation */
	ibat_regulation = ocp_threshold - HL7139_IBAT_OCP_TH_ABOVE_500MA;
	if (ibat_regulation < HL7139_IBAT_REG_TH_MIN)
		ibat_regulation = HL7139_IBAT_REG_TH_MIN;

	if (ibat_regulation > HL7139_IBAT_REG_TH_MAX)
		ibat_regulation = HL7139_IBAT_REG_TH_MAX;

	value = (u8)((ibat_regulation - HL7139_IBAT_REG_TH_MIN) /
		HL7139_IBAT_REG_TH_STEP);
	ret = hl7139_write_mask(di, HL7139_IBAT_REG_REG, HL7139_IBAT_REG_IBAT_REG_TH_MASK,
		HL7139_IBAT_REG_IBAT_REG_TH_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_ibat_reg_ref_ma [%x]=0x%x\n", HL7139_IBAT_REG_REG, value);
	return 0;
}

int hl7139_config_vbuscon_ovp_ref_mv(int ovp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (ovp_threshold < HL7139_VGS_SEL_MIN)
		ovp_threshold = HL7139_VGS_SEL_MIN;

	if (ovp_threshold > HL7139_VGS_SEL_MAX)
		ovp_threshold = HL7139_VGS_SEL_MAX;

	value = (u8)((ovp_threshold - HL7139_VGS_SEL_MIN) /
		HL7139_VGS_SEL_BASE_STEP);
	ret = hl7139_write_mask(di, HL7139_VBUS_OVP_REG, HL7139_VBUS_OVP_VGS_SEL_MASK,
		HL7139_VBUS_OVP_VGS_SEL_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_ac_ovp_threshold_mv [%x]=0x%x\n", HL7139_VBUS_OVP_REG, value);
	return 0;
}

int hl7139_config_vbus_ovp_ref_mv(int ovp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (ovp_threshold < HL7139_VBUS_OVP_MIN)
		ovp_threshold = HL7139_VBUS_OVP_MIN;

	if (ovp_threshold > HL7139_VBUS_OVP_MAX)
		ovp_threshold = HL7139_VBUS_OVP_MAX;

	value = (u8)((ovp_threshold - HL7139_VBUS_OVP_MIN) /
		HL7139_VBUS_OVP_STEP);
	ret = hl7139_write_mask(di, HL7139_VBUS_OVP_REG, HL7139_VBUS_OVP_VBUS_OVP_TH_MASK,
		HL7139_VBUS_OVP_VBUS_OVP_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_vbus_ovp_ref_mv [%x]=0x%x\n", HL7139_VBUS_OVP_REG, value);
	return 0;
}

static int hl7139_config_ibus_ocp_ma(int ocp_threshold, int chg_mode, void *dev_data)
{
	u8 value;
	int ret;
	int iin_regulation;
	struct hl7139_device_info *di = dev_data;

	/* config iin ocp offset */
	value = (HL7139_IIN_OCP_TH_ABOVE_250MA - HL7139_IIN_OCP_TH_ABOVE_100MA) /
		HL7139_IIN_OCP_TH_ABOVE_STEP;
	ret = hl7139_write_mask(di, HL7139_IIN_OC_REG, HL7139_IIN_OC_IIN_OCP_TH_MASK,
		HL7139_IIN_OC_IIN_OCP_TH_SHIFT, value); /* ibus ocp offset 250ma */
	if (ret)
		return -EPERM;
	hwlog_info("config_iin_ocp_offset_mv [%x]=0x%x\n", HL7139_IIN_OC_REG, value);

	iin_regulation = ocp_threshold - HL7139_IIN_OCP_TH_ABOVE_250MA;
	if (chg_mode == LVC_MODE) {
		if (iin_regulation < HL7139_IIN_REG_TH_BP_MIN)
			iin_regulation = HL7139_IIN_REG_TH_BP_MIN;

		if (iin_regulation > HL7139_IIN_REG_TH_BP_MAX)
			iin_regulation = HL7139_IIN_REG_TH_BP_MAX;

		value = (u8)((iin_regulation - HL7139_IIN_REG_TH_BP_MIN) /
			HL7139_IIN_REG_TH_BP_STEP);
	} else if (chg_mode == SC_MODE) {
		if (iin_regulation < HL7139_IIN_REG_TH_CP_MIN)
			iin_regulation = HL7139_IIN_REG_TH_CP_MIN;

		if (iin_regulation > HL7139_IIN_REG_TH_CP_MAX)
			iin_regulation = HL7139_IIN_REG_TH_CP_MAX;
		value = (u8)((iin_regulation - HL7139_IIN_REG_TH_CP_MIN) /
			HL7139_IIN_REG_TH_CP_STEP);
	} else {
		hwlog_err("chg mode error:chg_mode=%d\n", chg_mode);
		return -EPERM;
	}

	ret = hl7139_write_mask(di, HL7139_IIN_REG_REG,
		HL7139_IIN_REG_IIN_REG_TH_MASK,
		HL7139_IIN_REG_IIN_REG_TH_SHIFT, value);
	if (ret)
		return -EPERM;

	hwlog_info("config_ibus_reg_threshold_ma [%x]=0x%x\n",
		HL7139_IIN_REG_REG, value);
	return 0;
}

static int hl7139_config_switching_frequency(int data, void *dev_data)
{
	int freq;
	int ret;
	struct hl7139_device_info *di = dev_data;

	switch (data) {
	case HL7139_SW_FREQ_500KHZ:
		freq = HL7139_FSW_SET_SW_FREQ_500KHZ;
		break;
	case HL7139_SW_FREQ_700KHZ:
		freq = HL7139_FSW_SET_SW_FREQ_700KHZ;
		break;
	case HL7139_SW_FREQ_900KHZ:
		freq = HL7139_FSW_SET_SW_FREQ_900KHZ;
		break;
	case HL7139_SW_FREQ_1100KHZ:
		freq = HL7139_FSW_SET_SW_FREQ_1100KHZ;
		break;
	case HL7139_SW_FREQ_1300KHZ:
		freq = HL7139_FSW_SET_SW_FREQ_1300KHZ;
		break;
	case HL7139_SW_FREQ_1600KHZ:
		freq = HL7139_FSW_SET_SW_FREQ_1600KHZ;
		break;
	default:
		freq = HL7139_FSW_SET_SW_FREQ_900KHZ;
		break;
	}

	if (di->rev_id == 0) {
		if (freq >= HL7139_SW_FREQ_1300KHZ) {
			freq = HL7139_FSW_SET_SW_FREQ_1000KHZ;
			hwlog_info("force set switching_frequency to [%x]=0x%x\n",
				HL7139_CTRL0_REG, freq);
		}
	}

	ret = hl7139_write_mask(di, HL7139_CTRL0_REG, HL7139_CTRL0_FSW_SET_MASK,
		HL7139_CTRL0_FSW_SET_SHIFT, freq);
	if (ret)
		return -EPERM;

	hwlog_info("config_switching_frequency [%x]=0x%x\n",
		HL7139_CTRL0_REG, freq);

	return 0;
}

static void hl7139_lvc_opt_regs(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	/* needed setting value */
	hl7139_config_rlt_ovp_ref(HL7139_TRACK_OV_UV_TRACK_OV_MAX, di);
	hl7139_config_rlt_uvp_ref(HL7139_TRACK_OV_UV_TRACK_UV_MIN, di);
	hl7139_config_vbuscon_ovp_ref_mv(HL7139_VGS_SEL_INIT, di);
	hl7139_config_vbus_ovp_ref_mv(HL7139_VBUS_OVP_INIT, di);
	hl7139_config_vbat_ovp_mv(di->vbat_ovp_para, di);
	hl7139_config_ibat_ocp_ma(di->ibat_ocp_para, di);
	hl7139_config_ibus_ocp_ma(HL7139_IIN_OCP_BP_INIT, LVC_MODE, di);
}

static void hl7139_sc_opt_regs(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	/* needed setting value */
	hl7139_config_rlt_ovp_ref(HL7139_TRACK_OV_UV_TRACK_OV_MAX, di);
	hl7139_config_rlt_uvp_ref(HL7139_TRACK_OV_UV_TRACK_UV_INIT, di);
	hl7139_config_vbuscon_ovp_ref_mv(HL7139_VGS_SEL_INIT, di);
	hl7139_config_vbus_ovp_ref_mv(HL7139_VBUS_OVP_INIT, di);
	hl7139_config_vbat_ovp_mv(di->vbat_ovp_para, di);
	hl7139_config_ibat_ocp_ma(di->ibat_ocp_para, di);
	hl7139_config_ibus_ocp_ma(di->sc_ibus_ocp, SC_MODE, di);
}

static int hl7139_chip_init(void *dev_data)
{
	return 0;
}

static void hl7139_close_ibat_ocp(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	hl7139_write_mask(di, HL7139_IBAT_REG_REG,
		HL7139_IBAT_REG_IBAT_OCP_DIS_MASK, HL7139_IBAT_REG_IBAT_OCP_DIS_SHIFT, 1);
	hl7139_write_mask(di, HL7139_REG_CTRL1_REG,
		HL7139_REG_CTRL1_IBAT_REG_DIS_MASK, HL7139_REG_CTRL1_IBAT_REG_DIS_SHIFT, 1);
	hl7139_write_mask(di, HL7139_INT_MSK_REG,
		HL7139_INT_MSK_REG_M_MASK, HL7139_INT_MSK_REG_M_SHIFT, 1);
}

static int hl7139_reg_init(void *dev_data)
{
	int ret = 0;
	struct hl7139_device_info *di = dev_data;

	/* enable watchdog */
	if (di->rev_id == 0) {
		ret += hl7139_set_nwatchdog(1, di);
	} else {
		ret += hl7139_config_watchdog_ms(HL7139_WD_TMR_1000MS, di);
		ret += hl7139_set_nwatchdog(0, di);
	}

	ret += hl7139_config_rlt_ovp_ref(HL7139_TRACK_OV_UV_TRACK_OV_MIN, di);
	ret += hl7139_config_rlt_uvp_ref(HL7139_TRACK_OV_UV_TRACK_UV_MAX, di);
	ret += hl7139_config_vbuscon_ovp_ref_mv(HL7139_VGS_SEL_INIT, di);
	ret += hl7139_config_vbus_ovp_ref_mv(HL7139_VBUS_OVP_INIT, di);
	ret += hl7139_config_switching_frequency(di->switching_frequency, di);
	ret += hl7139_config_vbat_ovp_mv(HL7139_VBAT_OVP_INIT, di);
	ret += hl7139_config_ibat_ocp_ma(HL7139_IBAT_OCP_INIT, di);
	ret += hl7139_config_ibus_ocp_ma(HL7139_IIN_OCP_BP_INIT, LVC_MODE, di);

	/* enable ENADC */
	ret += hl7139_write_mask(di, HL7139_ADC_CTRL0_REG,
		HL7139_ADC_CTRL0_ADC_READ_EN_MASK, HL7139_ADC_CTRL0_ADC_READ_EN_SHIFT, 1);

	/* enable automatic VBUS_PD */
	ret += hl7139_write_mask(di, HL7139_VBUS_OVP_REG,
		HL7139_VBUS_OVP_VBUS_PD_EN_MASK, HL7139_VBUS_OVP_VBUS_PD_EN_SHIFT, 1);

	/* disable regulation */
	ret += hl7139_write_mask(di, HL7139_REG_CTRL1_REG, HL7139_REG_CTRL1_VBAT_REG_DIS_MASK,
		HL7139_REG_CTRL1_VBAT_REG_DIS_SHIFT, HL7139_REG_CTRL1_VBAT_REGULATION_OFF);
	ret += hl7139_write_mask(di, HL7139_REG_CTRL1_REG, HL7139_REG_CTRL1_IBAT_REG_DIS_MASK,
		HL7139_REG_CTRL1_IBAT_REG_DIS_SHIFT, HL7139_REG_CTRL1_IBAT_REGULATION_OFF);
	ret += hl7139_write_mask(di, HL7139_REG_CTRL0_REG, HL7139_REG_CTRL0_IIN_REG_DIS_MASK,
		HL7139_REG_CTRL0_IIN_REG_DIS_SHIFT, HL7139_REG_CTRL0_IIN_REGULATION_OFF);

	/* scp interrupt mask init */
	ret += hl7139_write_byte(di, HL7139_STATUS_A_REG, 0xFF);
	ret += hl7139_write_byte(di, HL7139_STATUS_B_REG, 0xFF);
	if (ret) {
		hwlog_err("reg_init fail\n");
		return -EPERM;
	}

	if (di->close_ibat_ocp)
		hl7139_close_ibat_ocp(dev_data);

	return 0;
}

static int hl7139_charge_init(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	di->device_id = hl7139_get_device_id((void *)di);
	if (di->device_id == -1)
		return -EPERM;

	di->rev_id = hl7139_get_revision_id(di);
	if (di->rev_id == -1)
		return -EPERM;

	if (hl7139_reg_init(di))
		return -EPERM;

	hwlog_info("switchcap device id is 0x%x, revision id is 0x%x\n",
		di->device_id, di->rev_id);

	di->init_finish_flag = HL7139_INIT_FINISH;
	return 0;
}

static int hl7139_charge_exit(void *dev_data)
{
	int ret;
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	ret = hl7139_sc_charge_enable(HL7139_SWITCHCAP_DISABLE, di);
	di->init_finish_flag = HL7139_NOT_INIT;
	di->int_notify_enable_flag = HL7139_DISABLE_INT_NOTIFY;

	return ret;
}

static int hl7139_batinfo_exit(void *dev_data)
{
	return 0;
}

static int hl7139_batinfo_init(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (hl7139_chip_init(di)) {
		hwlog_err("batinfo init fail\n");
		return -EPERM;
	}

	return 0;
}

static int hl7139_is_support_scp(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	if (di->param_dts.scp_support != 0) {
		hwlog_info("support scp charge\n");
		return 0;
	}
	return 1;
}

int hl7139_is_support_fcp(void *dev_data)
{
	struct hl7139_device_info *di = dev_data;

	if (di->param_dts.fcp_support != 0) {
		hwlog_info("support fcp charge\n");
		return 0;
	}
	return 1;
}

static int hl7139_db_value_dump(struct hl7139_device_info *di,
	char *reg_value, int size)
{
	int vbus = 0;
	int ibat = 0;
	int temp = 0;
	int ibus = 0;
	char buff[BUF_LEN] = {0};
	int len = 0;
	u8 reg = 0;

	(void)hl7139_get_vbus_mv(&vbus, di);
	(void)hl7139_get_ibat_ma(&ibat, di);
	(void)hl7139_get_ibus_ma(&ibus, di);
	(void)hl7139_get_device_temp(&temp, di);
	(void)hl7139_read_byte(di, HL7139_CTRL3_REG, &reg);

	if (((reg & HL7139_CTRL3_DEV_MODE_MASK) >> HL7139_CTRL3_DEV_MODE_SHIFT) ==
		HL7139_CTRL3_BPMODE)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((reg & HL7139_CTRL3_DEV_MODE_MASK) >> HL7139_CTRL3_DEV_MODE_SHIFT) ==
		HL7139_CTRL3_CPMODE)
		snprintf(buff, sizeof(buff), "%s", "SC     ");
	else
		snprintf(buff, sizeof(buff), "%s", "BUCK   ");
	len += strlen(buff);
	if (len < size)
		strncat(reg_value, buff, strlen(buff));

	len += snprintf(buff, sizeof(buff), "%-7.2d%-7.2d%-7.2d%-7.2d%-7.2d",
		ibus, vbus, ibat, hl7139_get_vbat_mv(di), temp);
	strncat(reg_value, buff, strlen(buff));

	return len;
}

static int hl7139_dump_reg_value(char *reg_value, int size, void *dev_data)
{
	u8 reg_val = 0;
	int i;
	int ret = 0;
	int len;
	int tmp;
	char buff[BUF_LEN] = {0};
	struct hl7139_device_info *di = dev_data;

	if (!di || !reg_value) {
		hwlog_err("di or reg_value is null\n");
		return -EPERM;
	}

	len = snprintf(reg_value, size, "%s ", di->name);
	len += hl7139_db_value_dump(di, buff, BUF_LEN);
	if (len < size)
		strncat(reg_value, buff, strlen(buff));

	for (i = 0; i < HL7139_PAGE0_NUM; i++) {
		tmp = HL7139_PAGE0_BASE + i;
		if ((tmp == HL7139_INT_REG) || (tmp == HL7139_STATUS_A_REG) ||
			(tmp == HL7139_STATUS_B_REG))
			continue;
		ret = ret || hl7139_read_byte(di, tmp, &reg_val);
		snprintf(buff, sizeof(buff), "0x%-7x", reg_val);
		len += strlen(buff);
		if (len < size)
			strncat(reg_value, buff, strlen(buff));
	}
	memset(buff, 0, sizeof(buff));
	for (i = 0; i < HL7139_PAGE1_NUM; i++) {
		tmp = HL7139_PAGE1_BASE + i;
		if ((tmp == HL7139_SCP_ISR1_REG) || (tmp == HL7139_SCP_ISR2_REG))
			continue;
		ret = ret || hl7139_read_byte(di, tmp, &reg_val);
		snprintf(buff, sizeof(buff), "0x%-7x", reg_val);
		len += strlen(buff);
		if (len < size)
			strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int hl7139_reg_head(char *reg_head, int size, void *dev_data)
{
	int i;
	int tmp;
	int len = 0;
	char buff[BUF_LEN] = {0};
	const char *half_head = "     dev     mode   Ibus   Vbus   Ibat   Vbat   Temp   ";
	struct hl7139_device_info *di = dev_data;

	if (!di || !reg_head) {
		hwlog_err("di or reg_head is null\n");
		return -EPERM;
	}

	snprintf(reg_head, size, half_head);
	len += strlen(half_head);

	memset(buff, 0, sizeof(buff));
	for (i = 0; i < HL7139_PAGE0_NUM; i++) {
		tmp = HL7139_PAGE0_BASE + i;
		if ((tmp == HL7139_INT_REG) || (tmp == HL7139_STATUS_A_REG) ||
			(tmp == HL7139_STATUS_B_REG))
			continue;
		snprintf(buff, sizeof(buff), "R[0x%3x] ", tmp);
		len += strlen(buff);
		if (len < size)
			strncat(reg_head, buff, strlen(buff));
	}

	memset(buff, 0, sizeof(buff));
	for (i = 0; i < HL7139_PAGE1_NUM; i++) {
		tmp = HL7139_PAGE1_BASE + i;
		if ((tmp == HL7139_SCP_ISR1_REG) || (tmp == HL7139_SCP_ISR2_REG))
			continue;
		snprintf(buff, sizeof(buff), "R[0x%3x] ", tmp);
		len += strlen(buff);
		if (len < size)
			strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static struct dc_ic_ops hl7139_lvc_ops = {
	.dev_name = "hl7139",
	.ic_init = hl7139_charge_init,
	.ic_exit = hl7139_charge_exit,
	.ic_enable = hl7139_lvc_charge_enable,
	.ic_discharge = hl7139_discharge,
	.is_ic_close = hl7139_is_device_close,
	.get_ic_id = hl7139_get_device_id,
	.config_ic_watchdog = hl7139_config_watchdog_ms,
	.ic_reg_reset_and_init = hl7139_reg_reset_and_init,
};

static struct dc_ic_ops hl7139_sc_ops = {
	.dev_name = "hl7139",
	.ic_init = hl7139_charge_init,
	.ic_exit = hl7139_charge_exit,
	.ic_enable = hl7139_sc_charge_enable,
	.ic_discharge = hl7139_discharge,
	.is_ic_close = hl7139_is_device_close,
	.get_ic_id = hl7139_get_device_id,
	.config_ic_watchdog = hl7139_config_watchdog_ms,
	.ic_reg_reset_and_init = hl7139_reg_reset_and_init,
};

static struct dc_batinfo_ops hl7139_batinfo_ops = {
	.init = hl7139_batinfo_init,
	.exit = hl7139_batinfo_exit,
	.get_bat_btb_voltage = hl7139_get_vbat_mv,
	.get_bat_package_voltage = hl7139_get_vbat_mv,
	.get_vbus_voltage = hl7139_get_vbus_mv,
	.get_bat_current = hl7139_get_ibat_ma,
	.get_ic_ibus = hl7139_get_ibus_ma,
	.get_ic_temp = hl7139_get_device_temp,
};

static struct power_log_ops hl7139_log_ops = {
	.dev_name = "hl7139",
	.dump_log_head = hl7139_reg_head,
	.dump_log_content = hl7139_dump_reg_value,
};

static void hl7139_init_ops_dev_data(struct hl7139_device_info *di)
{
	memcpy(&di->lvc_ops, &hl7139_lvc_ops, sizeof(struct dc_ic_ops));
	di->lvc_ops.dev_data = (void *)di;
	memcpy(&di->sc_ops, &hl7139_sc_ops, sizeof(struct dc_ic_ops));
	di->sc_ops.dev_data = (void *)di;
	memcpy(&di->batinfo_ops, &hl7139_batinfo_ops, sizeof(struct dc_batinfo_ops));
	di->batinfo_ops.dev_data = (void *)di;
	memcpy(&di->log_ops, &hl7139_log_ops, sizeof(struct power_log_ops));
	di->log_ops.dev_data = (void *)di;

	if (!di->ic_role) {
		snprintf(di->name, HL7139_DEV_NAME_LEN, "hl7139");
	} else {
		snprintf(di->name, HL7139_DEV_NAME_LEN, "hl7139_%d", di->ic_role);
		di->lvc_ops.dev_name = di->name;
		di->sc_ops.dev_name = di->name;
		di->log_ops.dev_name = di->name;
	}
}

static void hl7139_ops_register(struct hl7139_device_info *di)
{
	int ret;

	hl7139_init_ops_dev_data(di);

	ret = dc_ic_ops_register(LVC_MODE, di->ic_role, &di->lvc_ops);
	ret += dc_ic_ops_register(SC_MODE, di->ic_role, &di->sc_ops);
	ret += dc_batinfo_ops_register(SC_MODE, di->ic_role, &di->batinfo_ops);
	ret += dc_batinfo_ops_register(LVC_MODE, di->ic_role, &di->batinfo_ops);
	if (ret)
		hwlog_err("sysinfo ops register fail\n");

	ret = 0;
	if (hl7139_is_support_scp(di) == 0)
		ret += hl7139_scp_protocol_register(di);
	if (hl7139_is_support_fcp(di) == 0)
		ret += hl7139_fcp_protocol_register(di);
	if (ret)
		hwlog_err("scp or fcp ops register fail\n");

	power_log_ops_register(&di->log_ops);
}

static void hl7139_fault_handle(struct hl7139_device_info *di,
	struct nty_data *data)
{
	int val = 0;
	u8 flag0 = data->event2;
	u8 flag1 = data->event3;

	if (flag0 & HL7139_STATUS_A_VBAT_OVP_STS_MASK) {
		hwlog_info("VBAT_OVP happened\n");
		val = hl7139_get_vbat_mv(di);
		if (val >= di->vbat_ovp_para) {
			hwlog_info("VBAT_OVP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_VBAT_OVP, data);
		}
	}
	if (flag1 & HL7139_STATUS_B_IBAT_OCP_STS_MASK) {
		hwlog_info("IBAT_OCP happened\n");
		hl7139_get_ibat_ma(&val, di);
		if (val >= di->ibat_ocp_para) {
			hwlog_info("IBAT_OCP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_IBAT_OCP, data);
		}
	}
	if (flag0 & HL7139_STATUS_A_VIN_OVP_STS_MASK) {
		hwlog_info("VIN_OVP happened\n");
		hl7139_get_vbus_mv(&val, di);
		if (val >= di->vin_ovp_para) {
			hwlog_info("VIN_OVP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_VBUS_OVP, data);
		}
	}
	if (flag1 & HL7139_STATUS_B_IIN_OCP_STS_MASK) {
		hwlog_info("IIN_OCP happened\n");
		hl7139_get_ibus_ma(&val, di);
		if (val >= di->iin_ocp_para) {
			hwlog_info("IIN_OCP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_IBUS_OCP, data);
		}
	}
	if (flag1 & HL7139_STATUS_B_THSD_STS_MASK)
		hwlog_info("STATUS_B_THSD_STS happened\n");
}

static void hl7139_interrupt_clear(struct hl7139_device_info *di)
{
	u8 flag[5] = {0}; /* 5:read 5 byte */

	hwlog_info("irq_clear start\n");

	/* to confirm the interrupt */
	(void)hl7139_read_byte(di, HL7139_INT_REG, &flag[0]);
	(void)hl7139_read_byte(di, HL7139_STATUS_A_REG, &flag[1]);
	(void)hl7139_read_byte(di, HL7139_STATUS_B_REG, &flag[2]);
	/* to confirm the scp interrupt */
	(void)hl7139_read_byte(di, HL7139_SCP_ISR1_REG, &flag[3]);
	(void)hl7139_read_byte(di, HL7139_SCP_ISR2_REG, &flag[4]);
	di->scp_isr_backup[0] |= flag[3];
	di->scp_isr_backup[1] |= flag[4];

	hwlog_info("INT_REG [%x]=0x%x, STATUS_A_REG [%x]=0x%x, STATUS_B_REG [%x]=0x%x\n",
		HL7139_INT_REG, flag[0], HL7139_STATUS_A_REG, flag[1], HL7139_STATUS_B_REG, flag[2]);
	hwlog_info("ISR1 [%x]=0x%x, ISR2 [%x]=0x%x\n",
		HL7139_SCP_ISR1_REG, flag[3], HL7139_SCP_ISR2_REG, flag[4]);

	hwlog_info("irq_clear end\n");
}

static void hl7139_interrupt_work(struct work_struct *work)
{
	u8 flag[5] = {0}; /* read 5 bytes */
	struct hl7139_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct hl7139_device_info, irq_work);
	if (!di || !di->client)
		return;

	/* to confirm the interrupt */
	(void)hl7139_read_byte(di, HL7139_INT_REG, &flag[0]);
	(void)hl7139_read_byte(di, HL7139_STATUS_A_REG, &flag[1]);
	(void)hl7139_read_byte(di, HL7139_STATUS_B_REG, &flag[2]);
	/* to confirm the scp interrupt */
	(void)hl7139_read_byte(di, HL7139_SCP_ISR1_REG, &flag[3]);
	(void)hl7139_read_byte(di, HL7139_SCP_ISR2_REG, &flag[4]);

	di->scp_isr_backup[0] |= flag[3];
	di->scp_isr_backup[1] |= flag[4];

	data = &(di->nty_data);
	data->event1 = flag[0];
	data->event2 = flag[1];
	data->event3 = flag[2];
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag == HL7139_ENABLE_INT_NOTIFY) {
		hl7139_fault_handle(di, data);
		hl7139_dump_register(di);
	}

	hwlog_info("INT_REG [%x]=0x%x, STATUS_A_REG [%x]=0x%x, STATUS_B_REG [%x]=0x%x\n",
		HL7139_INT_REG, flag[0], HL7139_STATUS_A_REG, flag[1], HL7139_STATUS_B_REG, flag[2]);
	hwlog_info("ISR1 [%x]=0x%x, ISR2 [%x]=0x%x\n",
		HL7139_SCP_ISR1_REG, flag[3], HL7139_SCP_ISR2_REG, flag[4]);

	/* clear irq */
	enable_irq(di->irq_int);
}

static irqreturn_t hl7139_interrupt(int irq, void *_di)
{
	struct hl7139_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	if (di->init_finish_flag == HL7139_INIT_FINISH)
		di->int_notify_enable_flag = HL7139_ENABLE_INT_NOTIFY;

	hwlog_info("int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static void hl7139_parse_dts(struct device_node *np, struct hl7139_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", &di->switching_frequency,
		HL7139_SW_FREQ_900KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "scp_support",
		(u32 *)&(di->param_dts.scp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "fcp_support",
		(u32 *)&(di->param_dts.fcp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		(u32 *)&(di->ic_role), IC_ONE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", &di->sense_r_config, SENSE_R_5_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", &di->sense_r_actual, SENSE_R_5_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"close_ibat_ocp", &di->close_ibat_ocp, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vbat_ovp_para", &di->vbat_ovp_para, HL7139_VBAT_OVP_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibat_ocp_para", &di->ibat_ocp_para, HL7139_IBAT_OCP_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vin_ovp_para", &di->vin_ovp_para, HL7139_VIN_OVP_CP_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"iin_ocp_para", &di->iin_ocp_para, HL7139_IIN_OCP_CP_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ignore_discharge_flag", &di->ignore_discharge_flag, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc_ibus_ocp", &di->sc_ibus_ocp, HL7139_IIN_OCP_CP_INIT);
}

static void hl7139_init_lock_mutex(struct hl7139_device_info *di)
{
	mutex_init(&di->scp_detect_lock);
	mutex_init(&di->accp_adapter_reg_lock);
}

static void hl7139_destroy_lock_mutex(struct hl7139_device_info *di)
{
	mutex_destroy(&di->scp_detect_lock);
	mutex_destroy(&di->accp_adapter_reg_lock);
}

static int hl7139_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = -EINVAL;
	struct hl7139_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	di->chip_already_init = 1;

	di->device_id = hl7139_get_device_id(di);
	if (di->device_id == -1)
		goto hl7139_fail_0;
	di->rev_id = hl7139_get_revision_id(di);
	if (di->rev_id == -1)
		goto hl7139_fail_0;

	hl7139_parse_dts(np, di);
	hl7139_init_lock_mutex(di);
	hl7139_interrupt_clear(di);

	power_gpio_config_interrupt(np, "intr_gpio", "hl7139_gpio_int",
		&(di->gpio_int), &(di->irq_int));
	ret = request_irq(di->irq_int, hl7139_interrupt,
		IRQF_TRIGGER_FALLING, "hl7139_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto hl7139_fail_1;
	}
	INIT_WORK(&di->irq_work, hl7139_interrupt_work);

	ret = hl7139_reg_reset(di);
	if (ret)
		goto hl7139_fail_2;

	ret = hl7139_reg_init(di);
	if (ret)
		goto hl7139_fail_2;

	hl7139_ops_register(di);
	i2c_set_clientdata(client, di);
	return 0;

hl7139_fail_2:
	free_irq(di->irq_int, di);
hl7139_fail_1:
	gpio_free(di->gpio_int);
	hl7139_destroy_lock_mutex(di);
hl7139_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int hl7139_remove(struct i2c_client *client)
{
	struct hl7139_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	hl7139_reg_reset(di);

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);
	hl7139_destroy_lock_mutex(di);

	return 0;
}

static void hl7139_shutdown(struct i2c_client *client)
{
	struct hl7139_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hl7139_reg_reset(di);
}

MODULE_DEVICE_TABLE(i2c, hl7139);
static const struct of_device_id hl7139_of_match[] = {
	{
		.compatible = "hl7139",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id hl7139_i2c_id[] = {
	{ "hl7139", 0 }, {}
};

static struct i2c_driver hl7139_driver = {
	.probe = hl7139_probe,
	.remove = hl7139_remove,
	.shutdown = hl7139_shutdown,
	.id_table = hl7139_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hl7139",
		.of_match_table = of_match_ptr(hl7139_of_match),
	},
};

static int __init hl7139_init(void)
{
	return i2c_add_driver(&hl7139_driver);
}

static void __exit hl7139_exit(void)
{
	i2c_del_driver(&hl7139_driver);
}

module_init(hl7139_init);
module_exit(hl7139_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl7139 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
