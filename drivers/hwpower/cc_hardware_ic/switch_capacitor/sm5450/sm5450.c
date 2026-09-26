/*
 * sm5450.c
 *
 * sm5450 driver
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

#include "sm5450.h"
#include "sm5450_i2c.h"
#include "sm5450_protocol.h"
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
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>

#define HWLOG_TAG sm5450
HWLOG_REGIST();

#define BUF_LEN                             80
#define SM5450_PAGE0_NUM                    0x12
#define SM5450_PAGE1_NUM                    0x07
#define SM5450_PAGE0_BASE                   SM5450_CONTROL1_REG
#define SM5450_PAGE1_BASE                   SM5450_SCP_CTL_REG
#define SM5450_DBG_VAL_SIZE                 6

static int sm5450_reg_init(void *dev_data);
static void sm5450_common_opt_regs(void *dev_data);
static void sm5450_lvc_opt_regs(void *dev_data);
static void sm5450_sc_opt_regs(void *dev_data);

static void sm5450_dump_register(void *dev_data)
{
	u8 i;
	int ret;
	u8 val = 0;
	u8 val1[2] = {0}; /* read two bytes */
	struct sm5450_device_info *di = dev_data;

	for (i = SM5450_CONTROL1_REG; i <= SM5450_ADCCTRL_REG; i++) {
		if ((i == SM5450_FLAG1_REG) || (i == SM5450_FLAG2_REG) ||
			(i == SM5450_FLAG3_REG))
			continue;
		ret = sm5450_read_byte(di, i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");
		hwlog_info("reg [%x]=0x%x\n", i, val);
	}

	ret = sm5450_read_block(di, &val1[0], SM5450_VBUSADC_H_REG, 2);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("reg [%x]=0x%x, reg [%x]=0x%x\n",
			SM5450_VBUSADC_H_REG, val1[0], SM5450_VBUSADC_L_REG, val1[1]);
	ret = sm5450_read_block(di, &val1[0], SM5450_IBATADC_H_REG, 2);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("reg [%x]=0x%x, reg [%x]=0x%x\n",
			SM5450_IBATADC_H_REG, val1[0], SM5450_IBATADC_L_REG, val1[1]);
	ret = sm5450_read_block(di, &val1[0], SM5450_IBUSADC_H_REG, 2);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("reg [%x]=0x%x, reg [%x]=0x%x\n",
			SM5450_IBUSADC_H_REG, val1[0], SM5450_IBUSADC_L_REG, val1[1]);
	ret = sm5450_read_block(di, &val1[0], SM5450_VBATADC_H_REG, 2);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("reg [%x]=0x%x, reg [%x]=0x%x\n",
			SM5450_VBATADC_H_REG, val1[0], SM5450_VBATADC_L_REG, val1[1]);

	ret = sm5450_read_byte(di, SM5450_TDIEADC_REG, &val);
	if (ret)
		hwlog_err("dump_register read fail\n");
	else
		hwlog_info("reg [%x]=0x%x\n", SM5450_VBATADC_H_REG, val);

	for (i = SM5450_SCP_CTL_REG; i <= SM5450_SCP_STIMER_REG; i++) {
		if ((i == SM5450_SCP_ISR1_REG) || (i == SM5450_SCP_ISR2_REG))
			continue;
		ret = sm5450_read_byte(di, i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");
		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}

static int sm5450_reg_reset(void *dev_data)
{
	int ret;
	u8 reg = 0;
	struct sm5450_device_info *di = dev_data;

	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG,
		SM5450_CONTROL1_RESET_MASK, SM5450_CONTROL1_RESET_SHIFT,
		SM5450_CONTROL1_RST_ENABLE);
	if (ret)
		return -1;
	usleep_range(1000, 1100); /* wait soft reset ready, min:500us */
	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("reg_reset [%x]=0x%x\n", SM5450_CONTROL1_REG, reg);
	return 0;
}

static int sm5450_reg_reset_and_init(void *dev_data)
{
	int ret;

	if (!dev_data) {
		hwlog_err("dev_data is null\n");
		return -1;
	}

	ret = sm5450_reg_reset(dev_data);
	ret += sm5450_reg_init(dev_data);

	return ret;
}

static int sm5450_lvc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? SM5450_CONTROL1_FBYPASSMODE : SM5450_CONTROL1_OFFMODE;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (value == SM5450_CONTROL1_FBYPASSMODE)
		sm5450_lvc_opt_regs(di);

	/* 1. off mode */
	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_OPMODE_MASK,
		SM5450_CONTROL1_OPMODE_SHIFT, SM5450_CONTROL1_OFFMODE);
	if (ret)
		return -1;

	/* 2. enable sc mode */
	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_OPMODE_MASK,
		SM5450_CONTROL1_OPMODE_SHIFT, value);
	if (ret)
		return -1;

	if (value == SM5450_CONTROL1_OFFMODE)
		sm5450_common_opt_regs(di);

	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("charge_enable [%x]=0x%x\n", SM5450_CONTROL1_REG, reg);
	return 0;
}

static int sm5450_sc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? SM5450_CONTROL1_FCHGPUMPMODE : SM5450_CONTROL1_OFFMODE;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (value == SM5450_CONTROL1_FCHGPUMPMODE)
		sm5450_sc_opt_regs(di);

	/* 1. off mode */
	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_OPMODE_MASK,
		SM5450_CONTROL1_OPMODE_SHIFT, SM5450_CONTROL1_OFFMODE);
	if (ret)
		return -1;
	/* 2. enable sc mode */
	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_OPMODE_MASK,
		SM5450_CONTROL1_OPMODE_SHIFT, value);
	if (ret)
		return -1;

	if (value == SM5450_CONTROL1_OFFMODE)
		sm5450_common_opt_regs(di);

	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("ic_role = %d, charge_enable [%x]=0x%x\n", di->ic_role, SM5450_CONTROL1_REG, reg);
	return 0;
}

static int sm5450_discharge(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* VBUS_PD : 0(auto working), 1(manual pull down) */
	ret = sm5450_write_mask(di, SM5450_PULLDOWN_REG,
		SM5450_PULLDOWN_EN_VBUS_PD_MASK,
		SM5450_PULLDOWN_EN_VBUS_PD_SHIFT, value);
	if (ret)
		return -1;

	ret = sm5450_read_byte(di, SM5450_PULLDOWN_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("discharge [%x]=0x%x\n", SM5450_PULLDOWN_REG, reg);
	return 0;
}

static int sm5450_is_device_close(void *dev_data)
{
	u8 reg = 0;
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);
	if (ret)
		return 1;

	value = (reg & SM5450_CONTROL1_OPMODE_MASK) >> SM5450_CONTROL1_OPMODE_SHIFT;
	if ((value < SM5450_CONTROL1_FBYPASSMODE) || (value > SM5450_CONTROL1_FCHGPUMPMODE))
		return 1;

	return 0;
}

static int sm5450_get_device_id(void *dev_data)
{
	u8 part_info = 0;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->get_id_time == SM5450_USED)
		return di->device_id;

	di->get_id_time = SM5450_USED;
	ret = sm5450_read_byte(di, SM5450_DEVICE_INFO_REG, &part_info);
	if (ret) {
		di->get_id_time = SM5450_NOT_USED;
		hwlog_err("get_device_id read fail\n");
		return -1;
	}
	hwlog_info("get_device_id [%x]=0x%x\n", SM5450_DEVICE_INFO_REG, part_info);

	part_info = part_info & SM5450_DEVICE_INFO_DEVID_MASK;
	switch (part_info) {
	case SM5450_DEVICE_ID_SM5450:
	case SY69636_DEVICE_ID_SY69636:
		di->device_id = SWITCHCAP_SM_SM5450;
		break;
	default:
		di->device_id = -1;
		hwlog_err("switchcap get dev_id fail\n");
		break;
	}

	hwlog_info("device_id : 0x%x\n", di->device_id);

	return di->device_id;
}

static int sm5450_get_revision_id(void *dev_data)
{
	u8 rev_info = 0;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (di->get_rev_time == SM5450_USED)
		return di->rev_id;

	di->get_rev_time = SM5450_USED;
	ret = sm5450_read_byte(di, SM5450_DEVICE_INFO_REG, &rev_info);
	if (ret) {
		di->get_rev_time = SM5450_NOT_USED;
		hwlog_err("get_revision_id read fail\n");
		return -1;
	}
	hwlog_info("get_revision_id [%x]=0x%x\n", SM5450_DEVICE_INFO_REG, rev_info);

	di->rev_id = (rev_info & SM5450_DEVICE_INFO_REVID_MASK) >> SM5450_DEVICE_INFO_REVID_SHIFT;

	hwlog_info("revision_id : 0x%x\n", di->rev_id);
	return di->rev_id;
}

static int sm5450_get_vbat_mv(void *dev_data)
{
	u8 reg_high;
	u8 reg_low;
	s16 voltage;
	u8 val[2] = {0}; /* read two bytes */
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (sm5450_read_block(di, &val[0], SM5450_VBATADC_H_REG, 2))
		return -1;

	reg_high = val[0];
	reg_low = val[1];
	hwlog_info("VBAT_ADC1=0x%x, VBAT_ADC0=0x%x\n", reg_high, reg_low);

	voltage = ((reg_high << SM5450_LENGTH_OF_BYTE) + reg_low) * SM5450_ADC_RESOLUTION_2;

	return (int)(voltage);
}

static int sm5450_get_ibat_ma(int *ibat, void *dev_data)
{
	u8 reg_high;
	u8 reg_low;
	s16 curr;
	u8 val[2] = {0}; /* read two bytes */
	struct sm5450_device_info *di = dev_data;

	if (!ibat || !di) {
		hwlog_err("ibat or di is null\n");
		return -1;
	}

	if (sm5450_read_block(di, &val[0], SM5450_IBATADC_H_REG, 2))
		return -1;

	reg_high = val[0];
	reg_low = val[1];
	hwlog_info("IBAT_ADC1=0x%x, IBAT_ADC0=0x%x\n", reg_high, reg_low);

	curr = ((reg_high << SM5450_LENGTH_OF_BYTE) + reg_low) * SM5450_ADC_RESOLUTION_2;
	*ibat = ((int)curr) * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	return 0;
}

static int sm5450_get_ibus_ma(int *ibus, void *dev_data)
{
	u8 reg_high;
	u8 reg_low;
	s16 curr;
	u8 val[2] = {0}; /* read two bytes */
	struct sm5450_device_info *di = dev_data;

	if (!ibus || !di) {
		hwlog_err("ibus or di is null\n");
		return -1;
	}

	if (sm5450_read_block(di, &val[0], SM5450_IBUSADC_H_REG, 2))
		return -1;

	reg_high = val[0];
	reg_low = val[1];
	hwlog_info("IBUS_ADC1=0x%x, IBUS_ADC0=0x%x\n", reg_high, reg_low);

	curr = ((reg_high << SM5450_LENGTH_OF_BYTE) + reg_low) * SM5450_ADC_RESOLUTION_2;
	*ibus = (int)(curr);

	return 0;
}

int sm5450_get_vbus_mv(int *vbus, void *dev_data)
{
	u8 reg_high;
	u8 reg_low;
	s16 voltage;
	u8 val[2] = {0}; /* read two bytes */
	struct sm5450_device_info *di = dev_data;

	if (!vbus || !di) {
		hwlog_err("vbus or di is null\n");
		return -1;
	}

	if (sm5450_read_block(di, &val[0], SM5450_VBUSADC_H_REG, 2))
		return -1;

	reg_high = val[0];
	reg_low = val[1];
	hwlog_info("VBUS_ADC1=0x%x, VBUS_ADC0=0x%x\n", reg_high, reg_low);

	voltage = ((reg_high << SM5450_LENGTH_OF_BYTE) + reg_low) * SM5450_ADC_RESOLUTION_4;
	*vbus = (int)(voltage);

	return 0;
}

static int sm5450_get_device_temp(int *temp, void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (!temp || !di) {
		hwlog_err("temp or di is null\n");
		return -1;
	}

	ret = sm5450_read_byte(di, SM5450_TDIEADC_REG, &reg);
	if (ret)
		return -1;
	hwlog_info("TDIE_ADC=0x%x\n", reg);

	*temp = reg - SM5450_ADC_STANDARD_TDIE;

	return 0;
}

static int sm5450_set_nwatchdog(int disable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = disable ? 0x1 : 0x0;
	struct sm5450_device_info *di = dev_data;

	if (di->rev_id == 0) {
		hwlog_info("force set watchdog to disable\n");
		value = 1;
	}

	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_NEN_WATCHDOG_MASK,
		SM5450_CONTROL1_NEN_WATCHDOG_SHIFT, value);
	if (ret)
		return -1;

	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("watchdog [%x]=0x%x\n", SM5450_CONTROL1_REG, reg);
	return 0;
}

static int sm5450_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	int ret;
	u8 reg = 0;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (time >= SM5450_WTD_CONFIG_TIMING_80000MS)
		val = 7;
	else if (time >= SM5450_WTD_CONFIG_TIMING_40000MS)
		val = 6;
	else if (time >= SM5450_WTD_CONFIG_TIMING_20000MS)
		val = 5;
	else if (time >= SM5450_WTD_CONFIG_TIMING_10000MS)
		val = 4;
	else if (time >= SM5450_WTD_CONFIG_TIMING_5000MS)
		val = 3;
	else if (time >= SM5450_WTD_CONFIG_TIMING_2000MS)
		val = 2;
	else if (time >= SM5450_WTD_CONFIG_TIMING_1000MS)
		val = 1;
	else
		val = 0;

	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_WATCHDOG_REF_MASK,
		SM5450_CONTROL1_WATCHDOG_REF_SHIFT, val);
	if (ret)
		return -1;

	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("config_watchdog_ms [%x]=0x%x\n", SM5450_CONTROL1_REG, reg);
	return 0;
}

static int sm5450_config_rlt_uvp_ref(int rltuvp_rate, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (rltuvp_rate >= SM5450_RLTVUVP_REF_1P04)
		rltuvp_rate = SM5450_RLTVUVP_REF_1P04;
	value = rltuvp_rate;

	ret = sm5450_write_mask(di, SM5450_CONTROL3_REG, SM5450_CONTROL3_RLTVUVP_REF_MASK,
		SM5450_CONTROL3_RLTVUVP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_rlt_uvp_ref [%x]=0x%x\n", SM5450_CONTROL3_REG, value);
	return 0;
}

static int sm5450_config_rlt_ovp_ref(int rltovp_rate, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (rltovp_rate >= SM5450_RLTVOVP_REF_1P25)
		rltovp_rate = SM5450_RLTVOVP_REF_1P25;
	value = rltovp_rate;

	ret = sm5450_write_mask(di, SM5450_CONTROL3_REG, SM5450_CONTROL3_RLTVOVP_REF_MASK,
		SM5450_CONTROL3_RLTVOVP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_rlt_ovp_ref [%x]=0x%x\n", SM5450_CONTROL3_REG, value);
	return 0;
}

static int sm5450_config_vbat_ovp_ref_mv(int ovp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (ovp_threshold < SM5450_BAT_OVP_BASE_4000MV)
		ovp_threshold = SM5450_BAT_OVP_BASE_4000MV;

	if (ovp_threshold > SM5450_BAT_OVP_BASE_5575MV)
		ovp_threshold = SM5450_BAT_OVP_BASE_5575MV;

	value = (u8)((ovp_threshold - SM5450_BAT_OVP_BASE_4000MV) /
		SM5450_BAT_OVP_STEP);
	ret = sm5450_write_mask(di, SM5450_VBATOVP_REG, SM5450_VBATOVP_VBATOVP_REF_MASK,
		SM5450_VBATOVP_VBATOVP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_vbat_ovp_ref_mv [%x]=0x%x\n", SM5450_VBATOVP_REG, value);
	return 0;
}

static int sm5450_config_vbat_reg_ref_mv(int vbat_regulation, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (vbat_regulation <= SM5450_VBATREG_REF_BELOW_50MV)
		value = ((SM5450_VBATREG_REF_BELOW_50MV - SM5450_VBATREG_REF_BELOW_50MV) /
			SM5450_VBATREG_REF_BELOW_STEP);
	else if (vbat_regulation <= SM5450_VBATREG_REF_BELOW_100MV)
		value = ((SM5450_VBATREG_REF_BELOW_100MV - SM5450_VBATREG_REF_BELOW_50MV) /
			SM5450_VBATREG_REF_BELOW_STEP);
	else if (vbat_regulation <= SM5450_VBATREG_REF_BELOW_150MV)
		value = ((SM5450_VBATREG_REF_BELOW_150MV - SM5450_VBATREG_REF_BELOW_50MV) /
			SM5450_VBATREG_REF_BELOW_STEP);
	else if (vbat_regulation <= SM5450_VBATREG_REF_BELOW_200MV)
		value = ((SM5450_VBATREG_REF_BELOW_200MV - SM5450_VBATREG_REF_BELOW_50MV) /
			SM5450_VBATREG_REF_BELOW_STEP);
	else
		value = (SM5450_VBATREG_REF_BELOW_50MV /
			SM5450_VBATREG_REF_BELOW_STEP) - 1;

	ret = sm5450_write_mask(di, SM5450_REGULATION_REG, SM5450_REGULATION_VBATREG_REF_MASK,
		SM5450_REGULATION_VBATREG_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_vbat_reg_ref_mv [%x]=0x%x\n", SM5450_REGULATION_REG, value);
	return 0;
}

static int sm5450_config_ibat_ocp_ref_ma(int ocp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (ocp_threshold < SM5450_BAT_OCP_BASE_2000MA)
		ocp_threshold = SM5450_BAT_OCP_BASE_2000MA;

	if (ocp_threshold > SM5450_BAT_OCP_BASE_8300MA)
		ocp_threshold = SM5450_BAT_OCP_BASE_8300MA;

	value = (u8)((ocp_threshold - SM5450_BAT_OCP_BASE_2000MA) /
		SM5450_BAT_OCP_STEP);
	ret = sm5450_write_mask(di, SM5450_IBATOCP_REG, SM5450_IBATOCP_IBATOCP_REF_MASK,
		SM5450_IBATOCP_IBATOCP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_ibat_ocp_ref_ma [%x]=0x%x\n", SM5450_IBATOCP_REG, value);
	return 0;
}

static int sm5450_config_ibat_reg_ref_ma(int ibat_regulation, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (ibat_regulation <= SM5450_IBATREG_REF_BELOW_200MA)
		value = (SM5450_IBATREG_REF_BELOW_200MA - SM5450_IBATREG_REF_BELOW_200MA) /
			SM5450_IBATREG_REF_BELOW_STEP;
	else if (ibat_regulation <= SM5450_IBATREG_REF_BELOW_300MA)
		value = (SM5450_IBATREG_REF_BELOW_300MA - SM5450_IBATREG_REF_BELOW_200MA) /
			SM5450_IBATREG_REF_BELOW_STEP;
	else if (ibat_regulation <= SM5450_IBATREG_REF_BELOW_400MA)
		value = (SM5450_IBATREG_REF_BELOW_400MA - SM5450_IBATREG_REF_BELOW_200MA) /
			SM5450_IBATREG_REF_BELOW_STEP;
	else if (ibat_regulation <= SM5450_IBATREG_REF_BELOW_500MA)
		value = (SM5450_IBATREG_REF_BELOW_500MA - SM5450_IBATREG_REF_BELOW_200MA) /
			SM5450_IBATREG_REF_BELOW_STEP;
	else
		value = (SM5450_IBATREG_REF_BELOW_200MA - SM5450_IBATREG_REF_BELOW_200MA) /
			SM5450_IBATREG_REF_BELOW_STEP;

	ret = sm5450_write_mask(di, SM5450_REGULATION_REG, SM5450_REGULATION_IBATREG_REF_MASK,
		SM5450_REGULATION_IBATREG_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_ibat_reg_regulation_mv [%x]=0x%x\n", SM5450_REGULATION_REG, value);
	return 0;
}

int sm5450_config_vbuscon_ovp_ref_mv(int ovp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (ovp_threshold < SM5450_VBUSCON_OVP_BASE_4000MV)
		ovp_threshold = SM5450_VBUSCON_OVP_BASE_4000MV;

	if (ovp_threshold > SM5450_VBUSCON_OVP_MAX_19000MV)
		ovp_threshold = SM5450_VBUSCON_OVP_MAX_19000MV;

	value = (u8)((ovp_threshold - SM5450_VBUSCON_OVP_BASE_4000MV) /
		SM5450_VBUSCON_OVP_STEP);
	ret = sm5450_write_mask(di, SM5450_VBUSCON_OVP_REG, SM5450_VBUSCON_OVP_VBUSCON_OVP_REF_MASK,
		SM5450_VBUSCON_OVP_VBUSCON_OVP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_ac_ovp_threshold_mv [%x]=0x%x\n", SM5450_VBUSCON_OVP_REG, value);
	return 0;
}

int sm5450_config_vbus_ovp_ref_mv(int ovp_threshold, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (ovp_threshold < SM5450_VBUS_OVP_BASE_4000MV)
		ovp_threshold = SM5450_VBUS_OVP_BASE_4000MV;

	if (ovp_threshold > SM5450_VBUS_OVP_MAX_14000MV)
		ovp_threshold = SM5450_VBUS_OVP_MAX_14000MV;

	value = (u8)((ovp_threshold - SM5450_VBUS_OVP_BASE_4000MV) /
		SM5450_VBUS_OVP_STEP);
	ret = sm5450_write_mask(di, SM5450_VBUSOVP_REG, SM5450_VBUSOVP_VBUSOVP_REF_MASK,
		SM5450_VBUSOVP_VBUSOVP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_vbus_ovp_ref_mv [%x]=0x%x\n", SM5450_VBUSOVP_REG, value);
	return 0;
}

static int sm5450_config_ibus_ucp_ref_ma(int ucp_threshold, void *dev_data)
{
	u8 value = 0;
	u8 ucp_th_value;
	int ret;
	u8 pre_opmode;
	struct sm5450_device_info *di = dev_data;

	if (ucp_threshold <= SM5450_BUS_UCP_BASE_300MA_150MA) {
		ucp_threshold = SM5450_BUS_UCP_BASE_300MA_150MA;
		ucp_th_value = 0;
	} else if (ucp_threshold <= SM5450_BUS_UCP_BASE_500MA_250MA) {
		ucp_threshold = SM5450_BUS_UCP_BASE_500MA_250MA;
		ucp_th_value = 1;
	} else {
		ucp_threshold = SM5450_BUS_UCP_BASE_300MA_150MA;
		ucp_th_value = 0;
	}

	/* 1. save previous op mode */
	ret = sm5450_read_byte(di, SM5450_CONTROL1_REG, &value);
	if (ret)
		return -1;
	pre_opmode = (value & SM5450_CONTROL1_OPMODE_MASK) >> SM5450_CONTROL1_OPMODE_SHIFT;

	/* 2. set op mode to init mode */
	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG,
		SM5450_CONTROL1_OPMODE_MASK, SM5450_CONTROL1_OPMODE_SHIFT,
		SM5450_CONTROL1_OFFMODE);
	if (ret)
		return -1;

	/* 3. set IBUS_UCP reg */
	ret = sm5450_write_mask(di, SM5450_IBUS_OCP_UCP_REG, SM5450_IBUS_OCP_UCP_IBUSUCP_REF_MASK,
		SM5450_IBUS_OCP_UCP_IBUSUCP_REF_SHIFT, ucp_th_value);
	if (ret)
		return -1;

	/* 4. set op mode to pre mode */
	ret = sm5450_write_mask(di, SM5450_CONTROL1_REG, SM5450_CONTROL1_OPMODE_MASK,
		SM5450_CONTROL1_OPMODE_SHIFT, pre_opmode);
	if (ret)
		return -1;

	hwlog_info("config_ibus_ucp_threshold_ma [%x]=0x%x\n", SM5450_IBUS_OCP_UCP_REG, ucp_th_value);
	return 0;
}

static int sm5450_config_ibus_ocp_ref_ma(int ocp_threshold, int chg_mode, void *dev_data)
{
	u8 value;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (chg_mode == LVC_MODE) {
		if (ocp_threshold < SM5450_IBUS_OCP_BP_BASE_2500MA)
			ocp_threshold = SM5450_IBUS_OCP_BP_BASE_2500MA;

		if (ocp_threshold > SM5450_IBUS_OCP_BP_MAX_5600MA)
			ocp_threshold = SM5450_IBUS_OCP_BP_MAX_5600MA;

		value = (u8)((ocp_threshold - SM5450_IBUS_OCP_BP_BASE_2500MA) /
			SM5450_IBUS_OCP_STEP);
	} else if (chg_mode == SC_MODE) {
		if (ocp_threshold < SM5450_IBUS_OCP_CP_BASE_500MA)
			ocp_threshold = SM5450_IBUS_OCP_CP_BASE_500MA;

		if (ocp_threshold > SM5450_IBUS_OCP_CP_MAX_3600MA)
			ocp_threshold = SM5450_IBUS_OCP_CP_MAX_3600MA;

		value = (u8)((ocp_threshold - SM5450_IBUS_OCP_CP_BASE_500MA) /
			SM5450_IBUS_OCP_STEP);
	} else {
		hwlog_err("chg mode error:chg_mode=%d\n", chg_mode);
		return -1;
	}

	ret = sm5450_write_mask(di, SM5450_IBUS_OCP_UCP_REG,
		SM5450_IBUS_OCP_UCP_IBUSOCP_REF_MASK,
		SM5450_IBUS_OCP_UCP_IBUSOCP_REF_SHIFT, value);
	if (ret)
		return -1;

	hwlog_info("config_ibus_ocp_threshold_ma [%x]=0x%x\n",
		SM5450_IBUS_OCP_UCP_REG, value);
	return 0;
}

static int sm5450_config_switching_frequency(int data, void *dev_data)
{
	int freq;
	int freq_shift;
	int ret;
	struct sm5450_device_info *di = dev_data;

	switch (data) {
	case SM5450_SW_FREQ_450KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_M_P10;
		break;
	case SM5450_SW_FREQ_500KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_NORMAL;
		break;
	case SM5450_SW_FREQ_550KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_P_P10;
		break;
	case SM5450_SW_FREQ_675KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_M_P10;
		break;
	case SM5450_SW_FREQ_750KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_NORMAL;
		break;
	case SM5450_SW_FREQ_825KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_P_P10;
		break;
	case SM5450_SW_FREQ_1000KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_1000KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_NORMAL;
		break;
	case SM5450_SW_FREQ_1250KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_1250KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_NORMAL;
		break;
	case SM5450_SW_FREQ_1500KHZ:
		freq = SM5450_FSW_SET_SW_FREQ_1500KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_NORMAL;
		break;
	default:
		freq = SM5450_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SM5450_SW_FREQ_SHIFT_NORMAL;
		break;
	}

	if (di->rev_id == 0) {
		if (freq >= SM5450_FSW_SET_SW_FREQ_1250KHZ) {
			freq = SM5450_FSW_SET_SW_FREQ_1000KHZ;
			hwlog_info("force set switching_frequency to [%x]=0x%x\n",
				SM5450_CONTROL2_REG, freq);
		}
	}

	ret = sm5450_write_mask(di, SM5450_CONTROL2_REG, SM5450_CONTROL2_FREQ_MASK,
		SM5450_CONTROL2_FREQ_SHIFT, freq);
	if (ret)
		return -1;

	ret = sm5450_write_mask(di, SM5450_CONTROL2_REG, SM5450_CONTROL2_ADJUST_FREQ_MASK,
		SM5450_CONTROL2_ADJUST_FREQ_SHIFT, freq_shift);
	if (ret)
		return -1;

	hwlog_info("config_switching_frequency [%x]=0x%x\n",
		SM5450_CONTROL2_REG, freq);
	hwlog_info("config_adjustable_switching_frequency [%x]=0x%x\n",
		SM5450_CONTROL2_REG, freq_shift);

	return 0;
}

static void sm5450_common_opt_regs(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	/* needed setting value */
	sm5450_config_rlt_ovp_ref(SM5450_RLTVOVP_REF_1P25, di);
	sm5450_config_rlt_uvp_ref(SM5450_RLTVUVP_REF_1P01, di);
	sm5450_config_vbuscon_ovp_ref_mv(SM5450_VBUSCON_OVP_REF_INIT, di);
	sm5450_config_vbus_ovp_ref_mv(SM5450_VBUS_OVP_REF_INIT, di);
	sm5450_config_ibus_ocp_ref_ma(SM5450_IBUS_OCP_REF_INIT, SC_MODE, di);
	sm5450_config_ibus_ucp_ref_ma(SM5450_BUS_UCP_BASE_300MA_150MA, di);
	sm5450_config_vbat_ovp_ref_mv(SM5450_VBAT_OVP_REF_INIT, di);
	sm5450_config_ibat_ocp_ref_ma(SM5450_IBAT_OCP_REF_INIT, di);
	sm5450_config_ibat_reg_ref_ma(SM5450_IBATREG_REF_BELOW_200MA, di);
	sm5450_config_vbat_reg_ref_mv(SM5450_VBATREG_REF_BELOW_50MV, di);
}

static void sm5450_lvc_opt_regs(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	/* needed setting value */
	sm5450_config_rlt_ovp_ref(SM5450_RLTVOVP_REF_1P25, di);
	sm5450_config_rlt_uvp_ref(SM5450_RLTVUVP_REF_1P01, di);
	sm5450_config_vbuscon_ovp_ref_mv(12000, di); /* 12V */
	sm5450_config_vbus_ovp_ref_mv(11500, di); /* 11.5V */
	sm5450_config_ibus_ocp_ref_ma(5000, LVC_MODE, di); /* 5.0A */
	sm5450_config_ibus_ucp_ref_ma(SM5450_BUS_UCP_BASE_300MA_150MA, di); /* 300/150mA */
	sm5450_config_vbat_ovp_ref_mv(4500, di); /* 4.5V */
	sm5450_config_ibat_ocp_ref_ma(6000, di); /* 6A  */
	sm5450_config_ibat_reg_ref_ma(SM5450_IBATREG_REF_BELOW_500MA, di); /* ibat_ocp_ref - 0.5 = 5.5A */
	sm5450_config_vbat_reg_ref_mv(SM5450_VBATREG_REF_BELOW_50MV, di); /* vbat_ovp_ref - 0.05 = 4.45V */
}

static void sm5450_sc_opt_regs(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	/* needed setting value */
	sm5450_config_rlt_ovp_ref(SM5450_RLTVOVP_REF_1P25, di);
	sm5450_config_rlt_uvp_ref(SM5450_RLTVUVP_REF_1P01, di);
	sm5450_config_vbuscon_ovp_ref_mv(12000, di); /* 12V */
	sm5450_config_vbus_ovp_ref_mv(11500, di); /* 11.5V */
	sm5450_config_ibus_ocp_ref_ma(2800, SC_MODE, di); /* 2.8A */
	sm5450_config_ibus_ucp_ref_ma(SM5450_BUS_UCP_BASE_300MA_150MA, di); /* 300/150mA */
	sm5450_config_vbat_ovp_ref_mv(4500, di); /* 4.5V */
	sm5450_config_ibat_ocp_ref_ma(6000, di); /* 6A  */
	sm5450_config_ibat_reg_ref_ma(SM5450_IBATREG_REF_BELOW_500MA, di); /* ibat_ocp_ref - 0.5 = 5.5A */
	sm5450_config_vbat_reg_ref_mv(SM5450_VBATREG_REF_BELOW_50MV, di); /* vbat_ovp_ref - 0.05 = 4.45V */
}

static int sm5450_chip_init(void *dev_data)
{
	return 0;
}

static int sm5450_reg_init(void *dev_data)
{
	int ret = 0;
	struct sm5450_device_info *di = dev_data;
	u8 reg_val1 = 0;

	/* enable watchdog */
	if (di->rev_id == 0) {
		ret += sm5450_set_nwatchdog(1, di);
	} else {
		ret += sm5450_config_watchdog_ms(SM5450_WTD_CONFIG_TIMING_2000MS, di);
		ret += sm5450_set_nwatchdog(0, di);
	}

	/* enable EN_COMP */
	if (di->rev_id == 0)
		ret += sm5450_write_mask(di, SM5450_CONTROL3_REG,
			SM5450_CONTROL3_EN_COMP_MASK, SM5450_CONTROL3_EN_COMP_SHIFT, 1);
	else
		ret += sm5450_write_mask(di, SM5450_CONTROL3_REG,
			SM5450_CONTROL3_EN_COMP_MASK, SM5450_CONTROL3_EN_COMP_SHIFT, 0);
	(void)sm5450_read_byte(di, SM5450_CONTROL3_REG, &reg_val1);
	hwlog_info("di->rev_id=0x%x, reg:0x[%x]=0x%x\n", di->rev_id, SM5450_CONTROL3_REG, reg_val1);

	ret += sm5450_config_rlt_ovp_ref(SM5450_RLTVOVP_REF_1P10, di);
	ret += sm5450_config_rlt_uvp_ref(SM5450_RLTVUVP_REF_1P04, di);
	ret += sm5450_config_vbat_ovp_ref_mv(SM5450_VBAT_OVP_REF_INIT, di); /* VBAT_OVP 4.35V */
	ret += sm5450_config_ibat_ocp_ref_ma(SM5450_IBAT_OCP_REF_INIT, di); /* IBAT_OCP 7.2A */
	ret += sm5450_config_vbuscon_ovp_ref_mv(SM5450_VBUSCON_OVP_REF_INIT, di); /* VBUSCON_OVP 12V */
	ret += sm5450_config_vbus_ovp_ref_mv(SM5450_VBUS_OVP_REF_INIT, di); /* VBUS_OVP 11.5V */
	ret += sm5450_config_ibus_ocp_ref_ma(SM5450_IBUS_OCP_REF_INIT, SC_MODE, di); /* IBUS_OCP 3A */
	ret += sm5450_config_ibus_ocp_ref_ma(SM5450_IBUS_OCP_REF_INIT, LVC_MODE, di); /* IBUS_OCP 3A */
	ret += sm5450_config_ibus_ucp_ref_ma(SM5450_BUS_UCP_BASE_300MA_150MA, di); /* 300/150mA */
	ret += sm5450_config_ibat_reg_ref_ma(SM5450_IBATREG_REF_BELOW_500MA, di);
	ret += sm5450_config_vbat_reg_ref_mv(SM5450_VBATREG_REF_BELOW_50MV, di);
	ret += sm5450_config_switching_frequency(di->switching_frequency, di);
	ret += sm5450_write_byte(di, SM5450_FLAG_MASK1_REG, SM5450_FLAG_MASK1_INIT_REG);
	ret += sm5450_write_byte(di, SM5450_FLAG_MASK2_REG, SM5450_FLAG_MASK2_INIT_REG);
	ret += sm5450_write_byte(di, SM5450_FLAG_MASK3_REG, SM5450_FLAG_MASK3_INIT_REG);
	ret += sm5450_write_mask(di, SM5450_SCP_PING_REG,
		SM5450_SCP_PING_WAIT_TIME_MASK, SM5450_SCP_PING_WAIT_TIME_SHIFT, 1);

	/* enable ENADC */
	ret += sm5450_write_mask(di, SM5450_ADCCTRL_REG,
		SM5450_ADCCTRL_ENADC_MASK, SM5450_ADCCTRL_ENADC_SHIFT, 1);

	/* continuous mode for rev_id '0' */
	if (di->rev_id == 0)
		ret += sm5450_write_mask(di, SM5450_ADCCTRL_REG,
			SM5450_ADCCTRL_ADCMODE_MASK, SM5450_ADCCTRL_ADCMODE_SHIFT, 1);

	/* enable automatic VBUS_PD */
	ret += sm5450_write_mask(di, SM5450_PULLDOWN_REG,
		SM5450_PULLDOWN_EN_VBUS_PD_MASK, SM5450_PULLDOWN_EN_VBUS_PD_SHIFT, 0);

	/* scp interrupt mask init */
	ret += sm5450_write_byte(di, SM5450_SCP_MASK1_REG, 0xFF);
	ret += sm5450_write_byte(di, SM5450_SCP_MASK2_REG, 0xFF);
	if (ret) {
		hwlog_err("reg_init fail\n");
		return -1;
	}

	return 0;
}

static int sm5450_charge_init(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->device_id = sm5450_get_device_id((void *)di);
	if (di->device_id == -1)
		return -1;

	di->rev_id = sm5450_get_revision_id(di);
	if (di->rev_id == -1)
		return -1;

	if (sm5450_reg_init(di))
		return -1;

	hwlog_info("switchcap device id is 0x%x, revision id is 0x%x\n",
		di->device_id, di->rev_id);

	di->init_finish_flag = SM5450_INIT_FINISH;
	return 0;
}

static int sm5450_charge_exit(void *dev_data)
{
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = sm5450_sc_charge_enable(SM5450_SWITCHCAP_DISABLE, di);
	di->init_finish_flag = SM5450_NOT_INIT;
	di->int_notify_enable_flag = SM5450_DISABLE_INT_NOTIFY;

	return ret;
}

static int sm5450_batinfo_exit(void *dev_data)
{
	return 0;
}

static int sm5450_batinfo_init(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (sm5450_chip_init(di)) {
		hwlog_err("batinfo init fail\n");
		return -1;
	}

	return 0;
}

static int sm5450_is_support_scp(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (di->param_dts.scp_support != 0) {
		hwlog_info("support scp charge\n");
		return 0;
	}
	return 1;
}

int sm5450_is_support_fcp(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (di->param_dts.fcp_support != 0) {
		hwlog_info("support fcp charge\n");
		return 0;
	}
	return 1;
}

static int sm5450_db_value_dump(struct sm5450_device_info *di,
	char *reg_value, int size)
{
	int vbus = 0;
	int ibat = 0;
	int temp = 0;
	int ibus = 0;
	char buff[BUF_LEN] = {0};
	int len = 0;
	u8 reg = 0;

	(void)sm5450_get_vbus_mv(&vbus, di);
	(void)sm5450_get_ibat_ma(&ibat, di);
	(void)sm5450_get_ibus_ma(&ibus, di);
	(void)sm5450_get_device_temp(&temp, di);
	(void)sm5450_read_byte(di, SM5450_CONTROL1_REG, &reg);

	if (((reg & SM5450_CONTROL1_OPMODE_MASK) >> SM5450_CONTROL1_OPMODE_SHIFT) ==
		SM5450_CONTROL1_FBYPASSMODE)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((reg & SM5450_CONTROL1_OPMODE_MASK) >> SM5450_CONTROL1_OPMODE_SHIFT) ==
		SM5450_CONTROL1_FCHGPUMPMODE)
		snprintf(buff, sizeof(buff), "%s", "SC     ");
	else
		snprintf(buff, sizeof(buff), "%s", "BUCK   ");
	len += strlen(buff);
	if (len < size)
		strncat(reg_value, buff, strlen(buff));

	len += snprintf(buff, sizeof(buff), "%-7.2d%-7.2d%-7.2d%-7.2d%-7.2d",
		ibus, vbus, ibat, sm5450_get_vbat_mv(di), temp);
	strncat(reg_value, buff, strlen(buff));

	return len;
}

static int sm5450_dump_reg_value(char *reg_value, int size, void *dev_data)
{
	u8 reg_val = 0;
	int i;
	int ret = 0;
	int len = 0;
	int tmp;
	char buff[BUF_LEN] = {0};
	struct sm5450_device_info *di = dev_data;

	if (!di || !reg_value) {
		hwlog_err("di or reg_value is null\n");
		return -1;
	}

	if (di->ic_role == CHARGE_IC_TYPE_AUX) {
		snprintf(buff, sizeof(buff), "   ");
		len += strlen(buff);
		if (len < size)
			strncat(reg_value, buff, strlen(buff));
		memset(buff, 0, sizeof(buff));
	}

	len = sm5450_db_value_dump(di, reg_value, size);

	for (i = 0; i < SM5450_PAGE0_NUM; i++) {
		tmp = SM5450_PAGE0_BASE + i;
		if ((tmp == SM5450_FLAG1_REG) || (tmp == SM5450_FLAG2_REG) ||
			(tmp == SM5450_FLAG3_REG))
			continue;
		ret = ret || sm5450_read_byte(di, tmp, &reg_val);
		snprintf(buff, sizeof(buff), "0x%-7x", reg_val);
		len += strlen(buff);
		if (len < size)
			strncat(reg_value, buff, strlen(buff));
	}
	memset(buff, 0, sizeof(buff));
	for (i = 0; i < SM5450_PAGE1_NUM; i++) {
		tmp = SM5450_PAGE1_BASE + i;
		if ((tmp == SM5450_SCP_ISR1_REG) || (tmp == SM5450_SCP_ISR2_REG))
			continue;
		ret = ret || sm5450_read_byte(di, tmp, &reg_val);
		snprintf(buff, sizeof(buff), "0x%-7x", reg_val);
		len += strlen(buff);
		if (len < size)
			strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int sm5450_reg_head(char *reg_head, int size, void *dev_data)
{
	int i;
	int tmp;
	int len = 0;
	char buff[BUF_LEN] = {0};
	const char *half_head = "mode   Ibus   Vbus   Ibat   Vbat   Temp   ";
	const char *half_head1 = "   mode1   Ibus1   Vbus1   Ibat1   Vbat1   Temp1   ";
	struct sm5450_device_info *di = dev_data;

	if (!di || !reg_head) {
		hwlog_err("di or reg_head is null\n");
		return -1;
	}

	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		snprintf(reg_head, size, half_head);
		len += strlen(half_head);
	} else {
		snprintf(reg_head, size, half_head1);
		len += strlen(half_head1);
	}

	memset(buff, 0, sizeof(buff));
	for (i = 0; i < SM5450_PAGE0_NUM; i++) {
		tmp = SM5450_PAGE0_BASE + i;
		if ((tmp == SM5450_FLAG1_REG) || (tmp == SM5450_FLAG2_REG) ||
			(tmp == SM5450_FLAG3_REG))
			continue;
		snprintf(buff, sizeof(buff), "R[0x%3x] ", tmp);
		len += strlen(buff);
		if (len < size)
			strncat(reg_head, buff, strlen(buff));
	}

	memset(buff, 0, sizeof(buff));
	for (i = 0; i < SM5450_PAGE1_NUM; i++) {
		tmp = SM5450_PAGE1_BASE + i;
		if ((tmp == SM5450_SCP_ISR1_REG) || (tmp == SM5450_SCP_ISR2_REG))
			continue;
		snprintf(buff, sizeof(buff), "R[0x%3x] ", tmp);
		len += strlen(buff);
		if (len < size)
			strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static struct dc_ic_ops sm5450_lvc_ops = {
	.dev_name = "sm5450",
	.ic_init = sm5450_charge_init,
	.ic_exit = sm5450_charge_exit,
	.ic_enable = sm5450_lvc_charge_enable,
	.ic_discharge = sm5450_discharge,
	.is_ic_close = sm5450_is_device_close,
	.get_ic_id = sm5450_get_device_id,
	.config_ic_watchdog = sm5450_config_watchdog_ms,
	.ic_reg_reset_and_init = sm5450_reg_reset_and_init,
};

static struct dc_ic_ops sm5450_sc_ops = {
	.dev_name = "sm5450",
	.ic_init = sm5450_charge_init,
	.ic_exit = sm5450_charge_exit,
	.ic_enable = sm5450_sc_charge_enable,
	.ic_discharge = sm5450_discharge,
	.is_ic_close = sm5450_is_device_close,
	.get_ic_id = sm5450_get_device_id,
	.config_ic_watchdog = sm5450_config_watchdog_ms,
	.ic_reg_reset_and_init = sm5450_reg_reset_and_init,
};

static struct dc_batinfo_ops sm5450_batinfo_ops = {
	.init = sm5450_batinfo_init,
	.exit = sm5450_batinfo_exit,
	.get_bat_btb_voltage = sm5450_get_vbat_mv,
	.get_bat_package_voltage = sm5450_get_vbat_mv,
	.get_vbus_voltage = sm5450_get_vbus_mv,
	.get_bat_current = sm5450_get_ibat_ma,
	.get_ic_ibus = sm5450_get_ibus_ma,
	.get_ic_temp = sm5450_get_device_temp,
};

static struct power_log_ops sm5450_log_ops = {
	.dev_name = "sm5450",
	.dump_log_head = sm5450_reg_head,
	.dump_log_content = sm5450_dump_reg_value,
};

static struct dc_ic_ops sm5450_aux_lvc_ops = {
	.dev_name = "sm5450_aux",
	.ic_init = sm5450_charge_init,
	.ic_exit = sm5450_charge_exit,
	.ic_enable = sm5450_lvc_charge_enable,
	.ic_discharge = sm5450_discharge,
	.is_ic_close = sm5450_is_device_close,
	.get_ic_id = sm5450_get_device_id,
	.config_ic_watchdog = sm5450_config_watchdog_ms,
	.ic_reg_reset_and_init = sm5450_reg_reset_and_init,
};

static struct dc_ic_ops sm5450_aux_sc_ops = {
	.dev_name = "sm5450_aux",
	.ic_init = sm5450_charge_init,
	.ic_exit = sm5450_charge_exit,
	.ic_enable = sm5450_sc_charge_enable,
	.ic_discharge = sm5450_discharge,
	.is_ic_close = sm5450_is_device_close,
	.get_ic_id = sm5450_get_device_id,
	.config_ic_watchdog = sm5450_config_watchdog_ms,
	.ic_reg_reset_and_init = sm5450_reg_reset_and_init,
};

static struct dc_batinfo_ops sm5450_aux_batinfo_ops = {
	.init = sm5450_batinfo_init,
	.exit = sm5450_batinfo_exit,
	.get_bat_btb_voltage = sm5450_get_vbat_mv,
	.get_bat_package_voltage = sm5450_get_vbat_mv,
	.get_vbus_voltage = sm5450_get_vbus_mv,
	.get_bat_current = sm5450_get_ibat_ma,
	.get_ic_ibus = sm5450_get_ibus_ma,
	.get_ic_temp = sm5450_get_device_temp,
};

static struct power_log_ops sm5450_aux_log_ops = {
	.dev_name = "sm5450_aux",
	.dump_log_head = sm5450_reg_head,
	.dump_log_content = sm5450_dump_reg_value,
};

static void sm5450_init_ops_dev_data(struct sm5450_device_info *di)
{
	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		sm5450_lvc_ops.dev_data = (void *)di;
		sm5450_sc_ops.dev_data = (void *)di;
		sm5450_batinfo_ops.dev_data = (void *)di;
		sm5450_log_ops.dev_data = (void *)di;
	} else {
		sm5450_aux_lvc_ops.dev_data = (void *)di;
		sm5450_aux_sc_ops.dev_data = (void *)di;
		sm5450_aux_batinfo_ops.dev_data = (void *)di;
		sm5450_aux_log_ops.dev_data = (void *)di;
	}
}

static int sm5450_ops_register(struct sm5450_device_info *di)
{
	int ret;

	sm5450_init_ops_dev_data(di);

	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		ret = dc_ic_ops_register(LVC_MODE, CHARGE_IC_TYPE_MAIN, &sm5450_lvc_ops);
		ret += dc_ic_ops_register(SC_MODE, CHARGE_IC_TYPE_MAIN, &sm5450_sc_ops);
		ret += dc_batinfo_ops_register(SC_MODE, CHARGE_IC_TYPE_MAIN, &sm5450_batinfo_ops);
		ret += dc_batinfo_ops_register(LVC_MODE, CHARGE_IC_TYPE_MAIN, &sm5450_batinfo_ops);
		if (sm5450_is_support_scp(di) == 0)
			ret += sm5450_scp_protocol_register(di);
		if (sm5450_is_support_fcp(di) == 0)
			ret += sm5450_fcp_protocol_register(di);

		ret += power_log_ops_register(&sm5450_log_ops);
	} else {
		ret = dc_ic_ops_register(LVC_MODE, CHARGE_IC_TYPE_AUX, &sm5450_aux_lvc_ops);
		ret += dc_ic_ops_register(SC_MODE, CHARGE_IC_TYPE_AUX, &sm5450_aux_sc_ops);
		ret += dc_batinfo_ops_register(SC_MODE, CHARGE_IC_TYPE_AUX,
			&sm5450_aux_batinfo_ops);
		ret += dc_batinfo_ops_register(LVC_MODE, CHARGE_IC_TYPE_AUX,
			&sm5450_aux_batinfo_ops);
		if (sm5450_is_support_scp(di) == 0)
			ret += sm5450_scp_protocol_register(di);
		if (sm5450_is_support_fcp(di) == 0)
			ret += sm5450_fcp_protocol_register(di);

		ret += power_log_ops_register(&sm5450_aux_log_ops);
	}

	if (ret)
		hwlog_err("ops_register fail\n");

	return ret;
}

static void sm5450_fault_handle(struct sm5450_device_info *di,
	struct nty_data *data)
{
	int val = 0;
	u8 flag0 = data->event1;
	u8 flag1 = data->event2;

	if (flag0 & SM5450_FLAG1_VBUSCON_OVP_FLAG_MASK) {
		hwlog_info("AC OVP happened\n");
		power_event_anc_notify(POWER_ANT_SC_FAULT,
			POWER_NE_DC_FAULT_AC_OVP, data);
	}
	if (flag1 & SM5450_FLAG2_VBATOVP_FLAG_MASK) {
		hwlog_info("BAT OVP happened\n");
		val = sm5450_get_vbat_mv(di);
		if (val >= SM5450_VBAT_OVP_REF_INIT) {
			hwlog_info("BAT OVP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_VBAT_OVP, data);
		}
	}
	if (flag1 & SM5450_FLAG2_IBATOCP_FLAG_MASK) {
		hwlog_info("BAT OCP happened\n");
		sm5450_get_ibat_ma(&val, di);
		if (val >= SM5450_IBAT_OCP_REF_INIT) {
			hwlog_info("BAT OCP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_IBAT_OCP, data);
		}
	}
	if (flag0 & SM5450_FLAG1_VBUSOVP_FLAG_MASK) {
		hwlog_info("BUS OVP happened\n");
		sm5450_get_vbus_mv(&val, di);
		if (val >= SM5450_VBUS_OVP_REF_INIT) {
			hwlog_info("BUS OVP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_VBUS_OVP, data);
		}
	}
	if (flag0 & SM5450_FLAG1_IBUSOCP_FLAG_MASK) {
		hwlog_info("BUS OCP happened\n");
		sm5450_get_ibus_ma(&val, di);
		if (val >= SM5450_IBUS_OCP_REF_INIT) {
			hwlog_info("BUS OCP happened [%d]\n", val);
			power_event_anc_notify(POWER_ANT_SC_FAULT,
				POWER_NE_DC_FAULT_IBUS_OCP, data);
		}
	}
	if (flag1 & SM5450_FLAG2_TSD_FLAG_MASK)
		hwlog_info("DIE TEMP OTP happened\n");
}

static void sm5450_interrupt_clear(struct sm5450_device_info *di)
{
	u8 flag[5] = {0}; /* 5:read 5 byte */

	hwlog_info("irq_clear start\n");

	/* to confirm the interrupt */
	(void)sm5450_read_byte(di, SM5450_FLAG1_REG, &flag[0]);
	(void)sm5450_read_byte(di, SM5450_FLAG2_REG, &flag[1]);
	(void)sm5450_read_byte(di, SM5450_FLAG3_REG, &flag[2]);
	/* to confirm the latest status */
	(void)sm5450_read_byte(di, SM5450_FLAG1_REG, &flag[0]);
	(void)sm5450_read_byte(di, SM5450_FLAG2_REG, &flag[1]);
	(void)sm5450_read_byte(di, SM5450_FLAG3_REG, &flag[2]);
	/* to confirm the scp interrupt */
	(void)sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &flag[3]);
	(void)sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &flag[4]);
	di->scp_isr_backup[0] |= flag[3];
	di->scp_isr_backup[1] |= flag[4];

	hwlog_info("FLAG1 [%x]=0x%x, FLAG2 [%x]=0x%x, FLAG3 [%x]=0x%x\n",
		SM5450_FLAG1_REG, flag[0], SM5450_FLAG2_REG, flag[1], SM5450_FLAG3_REG, flag[2]);
	hwlog_info("ISR1 [%x]=0x%x, ISR2 [%x]=0x%x\n",
		SM5450_SCP_ISR1_REG, flag[3], SM5450_SCP_ISR2_REG, flag[4]);

	hwlog_info("irq_clear end\n");
}

static void sm5450_interrupt_work(struct work_struct *work)
{
	u8 flag[5] = {0}; /* read 5 bytes */
	struct sm5450_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct sm5450_device_info, irq_work);
	if (!di || !di->client)
		return;

	/* to confirm the interrupt */
	(void)sm5450_read_byte(di, SM5450_FLAG1_REG, &flag[0]);
	(void)sm5450_read_byte(di, SM5450_FLAG2_REG, &flag[1]);
	(void)sm5450_read_byte(di, SM5450_FLAG3_REG, &flag[2]);
	/* to confirm the latest status */
	(void)sm5450_read_byte(di, SM5450_FLAG1_REG, &flag[0]);
	(void)sm5450_read_byte(di, SM5450_FLAG2_REG, &flag[1]);
	(void)sm5450_read_byte(di, SM5450_FLAG3_REG, &flag[2]);
	/* to confirm the scp interrupt */
	(void)sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &flag[3]);
	(void)sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &flag[4]);

	di->scp_isr_backup[0] |= flag[3];
	di->scp_isr_backup[1] |= flag[4];

	data = &(di->nty_data);
	data->event1 = flag[0];
	data->event2 = flag[1];
	data->event3 = flag[2];
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag == SM5450_ENABLE_INT_NOTIFY) {
		sm5450_fault_handle(di, data);
		sm5450_dump_register(di);
	}

	hwlog_info("FLAG1 [%x]=0x%x, FLAG2 [%x]=0x%x, FLAG3 [%x]=0x%x\n",
		SM5450_FLAG1_REG, flag[0], SM5450_FLAG2_REG, flag[1], SM5450_FLAG3_REG, flag[2]);
	hwlog_info("ISR1 [%x]=0x%x, ISR2 [%x]=0x%x\n",
		SM5450_SCP_ISR1_REG, flag[3], SM5450_SCP_ISR2_REG, flag[4]);

	/* clear irq */
	enable_irq(di->irq_int);
}

static irqreturn_t sm5450_interrupt(int irq, void *_di)
{
	struct sm5450_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	if (di->init_finish_flag == SM5450_INIT_FINISH)
		di->int_notify_enable_flag = SM5450_ENABLE_INT_NOTIFY;

	hwlog_info("int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static void sm5450_parse_dts(struct device_node *np, struct sm5450_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", &di->switching_frequency,
		SM5450_SW_FREQ_550KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "scp_support",
		(u32 *)&(di->param_dts.scp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "fcp_support",
		(u32 *)&(di->param_dts.fcp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		(u32 *)&(di->ic_role), CHARGE_IC_TYPE_MAIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", &di->sense_r_config, SENSE_R_5_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", &di->sense_r_actual, SENSE_R_5_MOHM);
}

static void sm5450_init_lock_mutex(struct sm5450_device_info *di)
{
	mutex_init(&di->scp_detect_lock);
	mutex_init(&di->accp_adapter_reg_lock);
}

static void sm5450_destroy_lock_mutex(struct sm5450_device_info *di)
{
	mutex_destroy(&di->scp_detect_lock);
	mutex_destroy(&di->accp_adapter_reg_lock);
}

static int sm5450_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = -EINVAL;
	struct sm5450_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	di->chip_already_init = 1;

	di->device_id = sm5450_get_device_id(di);
	if (di->device_id == -1)
		goto sm5450_fail_0;
	di->rev_id = sm5450_get_revision_id(di);
	if (di->rev_id == -1)
		goto sm5450_fail_0;

	sm5450_parse_dts(np, di);
	sm5450_init_lock_mutex(di);
	sm5450_interrupt_clear(di);

	power_gpio_config_interrupt(np, "intr_gpio", "sm5450_gpio_int",
		&(di->gpio_int), &(di->irq_int));
	ret = request_irq(di->irq_int, sm5450_interrupt,
		IRQF_TRIGGER_FALLING, "sm5450_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto sm5450_fail_1;
	}
	INIT_WORK(&di->irq_work, sm5450_interrupt_work);

	ret = sm5450_ops_register(di);
	if (ret)
		goto sm5450_fail_2;

	ret = sm5450_reg_reset(di);
	if (ret)
		goto sm5450_fail_2;

	ret = sm5450_reg_init(di);
	if (ret)
		goto sm5450_fail_2;

	return 0;

sm5450_fail_2:
	free_irq(di->irq_int, di);
sm5450_fail_1:
	gpio_free(di->gpio_int);
	sm5450_destroy_lock_mutex(di);
sm5450_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int sm5450_remove(struct i2c_client *client)
{
	struct sm5450_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	sm5450_reg_reset(di);

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);
	sm5450_destroy_lock_mutex(di);

	return 0;
}

static void sm5450_shutdown(struct i2c_client *client)
{
	struct sm5450_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	sm5450_reg_reset(di);
}

MODULE_DEVICE_TABLE(i2c, sm5450);
static const struct of_device_id sm5450_of_match[] = {
	{
		.compatible = "sm5450",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sm5450_i2c_id[] = {
	{ "sm5450", 0 }, {}
};

static struct i2c_driver sm5450_driver = {
	.probe = sm5450_probe,
	.remove = sm5450_remove,
	.shutdown = sm5450_shutdown,
	.id_table = sm5450_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sm5450",
		.of_match_table = of_match_ptr(sm5450_of_match),
	},
};

static int __init sm5450_init(void)
{
	return i2c_add_driver(&sm5450_driver);
}

static void __exit sm5450_exit(void)
{
	i2c_del_driver(&sm5450_driver);
}

module_init(sm5450_init);
module_exit(sm5450_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sm5450 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
