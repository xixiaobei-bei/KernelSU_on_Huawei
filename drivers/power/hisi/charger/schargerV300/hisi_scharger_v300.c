/*
 * hisi_scharger_v300.c
 *
 * HI6523 charger driver
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#include "hisi_scharger_v300.h"
#include "hisi_scharger_v300_common.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger/direct_charger.h>
#endif
#ifdef CONFIG_USB_AUDIO_POWER
#include <huawei_platform/audio/usb_audio_power.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <linux/hisi/hisi_adc.h>
#ifdef CONFIG_SWITCH_FSA9685
#include <huawei_platform/usb/switch/switch_fsa9685.h>
#endif
#ifdef CONFIG_BOOST_5V
#include <huawei_platform/power/boost_5v.h>
#endif
#include <securec.h>

#define MAX_RBOOST_CNT 300
#define ILIMIT_RBOOST_CNT 15

struct hi6523_device_info *g_hi6523_dev;
static int adc_channel_vbat_sys = -1;
static unsigned int is_board_type = 2; /* 0:sft 1:udp 2:asic */
unsigned short hi6523_version;
static int plugout_check_ibus_ma = 150;
static int plugout_check_delay_ms = 1000;
static unsigned int switch_id_flag; /* switch id status no need changed */
static int hi6523_force_set_term_flag = CHG_STAT_DISABLE;
static char g_batt_ovp_cnt_30s;
static char g_vbat_check_cycle;
static char g_batt_ovp_cnt_1s;
static int g_rboost_cnt;
static int i_bias_all;
static int dm_array_len;
static int dp_array_len;
static unsigned int g_single_phase_buck;

static int hi6523_set_charge_enable(int enable);

/* init vterm-vdpm table */
static struct charge_cv_vdpm_data vterm_vdpm[] = {
				{ 4150, 4460, 3 }, { 4400, 4675, 4 }, };
static int charger_is_fcp = FCP_FALSE;
static int first_insert_flag = FIRST_INSERT_TRUE;
static int is_weaksource = WEAKSOURCE_FALSE;
static int gain_cal;
static int iin_set = CHG_ILIMIT_100;

/*
 * chg_ir_comp[CHG_IR_COMP_NUM]: the resistance of charger, unit:mOhm
 * chg_lmt_set_v300[CHG_LMT_GEAR_NUM]: the current limit when setting
 *                                     charger value, unit: mA
 * chg_lmt_get_v300[CHG_LMT_GEAR_NUM]: the current limit when getting
 *                                     charger value, unit: mA
 * These arrays are set to replace the tedious "else if" statements,instead,
 * a concise "for" statement is used in this version, the items converted from
 * the original Macro definition like CHG_IR_COMP_15MOHM and CHG_ILIMIT_3200 in
 * the arrays are the values to be given to the variables in the "for" loop
 */
int chg_ir_comp[CHG_IR_COMP_NUM] = { 0, 15, 30, 45, 60, 75, 95, 110 };
int chg_lmt_set_v300[CHG_LMT_GEAR_NUM] = {
	100, 130, 699, 900, 1000, 1080, 1200, 1300, 1400, 1500, 1600, 1700,
	1800, 1900, 2000, 2100, 2200, 2280, 2400, 2480, 2600, 2680, 2800,
	2880, 3000, 3080, 3200 };
int chg_lmt_get_v300[CHG_LMT_GEAR_NUM] = {
	100, 130, 470, 900, 1000, 1080, 1200, 1300, 1400, 1500, 1600, 1700,
	1800, 1900, 2000, 2100, 2200, 2280, 2400, 2480, 2600, 2680, 2800, 2880,
	3000, 3080, 3200 };

struct hi6523_device_info *get_hi6523_device(void)
{
	return g_hi6523_dev;
}

static void set_boot_weaksource_flag(void)
{
#ifdef CONFIG_HISI_COUL_HI6421V700
	unsigned int reg_val;

	reg_val = hisi_pmic_reg_read(WEAKSOURCE_FLAG_REG);
	reg_val |= WAEKSOURCE_FLAG;
	hisi_pmic_reg_write(WEAKSOURCE_FLAG_REG, reg_val);
#endif
}

static void clr_boot_weaksource_flag(void)
{
#ifdef CONFIG_HISI_COUL_HI6421V700
	unsigned int reg_val;

	reg_val = hisi_pmic_reg_read(WEAKSOURCE_FLAG_REG);
	reg_val &= (~WAEKSOURCE_FLAG);
	hisi_pmic_reg_write(WEAKSOURCE_FLAG_REG, reg_val);
#endif
}

/* Description:    juege if do cv limit */
bool is_hi6523_cv_limit(void)
{
	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310))
		return TRUE;
	else
		return FALSE;
}
EXPORT_SYMBOL(is_hi6523_cv_limit);

/* check chip i2c communication */
static int hi6523_device_check(void)
{
	int ret;
	u8 reg_chip_id0 = 0xff;

	ret = hi6523_read_byte(CHIP_VERSION_4, &reg_chip_id0);
	if (ret) {
		scharger_err("[%s]:read chip_id0 fail\n", __func__);
		return CHARGE_IC_BAD;
	}

	if (reg_chip_id0 == CHIP_ID0 || reg_chip_id0 == CHIP_ID1 ||
		reg_chip_id0 == CHIP_ID2) {
		scharger_inf("hi6523 is good\n");
		return CHARGE_IC_GOOD;
	}
	scharger_err("hi6523 is bad\n");
	return CHARGE_IC_BAD;
}

/*
 * set the bat comp, schargerv100 can't set ir comp due to lx bug
 * Parameters: value:bat_comp mohm
 */
static int hi6523_set_bat_comp(int value)
{
	u8 i;
	u8 reg = 0;

	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310))
		return 0;
	/*
	 * This "for" statement takes place of the previous tedious
	 * "else if" statements. the items in the array chg_lmt_set_v300
	 * are the Macro defined values, like #define CHG_ILIMIT_3200 3200
	 * cin_limit and i_in_limit follow the subsequent ranges and values:
	 * (-∞, CHG_IR_COMP_MIN), reg is assigned to 0;
	 * [CHG_IR_COMP_0MOHM, CHG_IR_COMP_15MOHM), reg is assigned to 0;
	 * [CHG_IR_COMP_15MOHM, CHG_IR_COMP_30MOHM), reg is assigned to 1;
	 * [CHG_IR_COMP_30MOHM, CHG_IR_COMP_45MOHM), reg is assigned to 2;
	 * [CHG_IR_COMP_45MOHM, CHG_IR_COMP_60MOHM), reg is assigned to 3;
	 * [CHG_IR_COMP_60MOHM, CHG_IR_COMP_75MOHM), reg is assigned to 4;
	 * [CHG_IR_COMP_75MOHM, CHG_IR_COMP_95MOHM), reg is assigned to 5;
	 * [CHG_IR_COMP_95MOHM, CHG_IR_COMP_110MOHM), reg is assigned to 6;
	 * [CHG_IR_COMP_110MOHM, +∞), reg is assigned to 7;
	 */
	if (value < CHG_IR_COMP_MIN)
		reg = 0; /* 0: marking value less than CHG_IR_COMP_MIN */
	else if (value >= CHG_IR_COMP_110MOHM)
		reg = 7; /* 7: marking value larger than CHG_IR_COMP_110MOHM */
	for (i = 1; i < CHG_IR_COMP_NUM; i++)
		if (value >= chg_ir_comp[i - 1] && value < chg_ir_comp[i])
			reg = i - 1;

	return hi6523_write_mask(CHG_IR_COMP_REG, CHG_IR_COMP_MSK,
				 CHG_IR_COMP_SHIFT, reg);
}

/* Parameters: value:vclamp mV, schargerv100 can't set vclamp due to lx bug */
static int hi6523_set_vclamp(int value)
{
	u8 reg;

	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310))
		return 0;
	if (value < CHG_IR_VCLAMP_MIN)
		value = CHG_IR_VCLAMP_MIN;
	else if (value > CHG_IR_VCLAMP_MAX)
		value = CHG_IR_VCLAMP_MAX;

	reg = value / CHG_IR_VCLAMP_STEP;
	return hi6523_write_mask(CHG_IR_VCLAMP_REG, CHG_IR_VCLAMP_MSK,
				 CHG_IR_VCLAMP_SHIFT, reg);
}

static int hi6523_set_fast_safe_timer(u32 chg_fastchg_safe_timer)
{
	return hi6523_write_mask(CHG_FASTCHG_TIMER_REG, CHG_FASTCHG_TIMER_MSK,
			CHG_FASTCHG_TIMER_SHIFT, (u8)chg_fastchg_safe_timer);
}

static int hi6523_set_precharge_current(int precharge_current)
{
	u8 prechg_limit;

	if (precharge_current < CHG_PRG_ICHG_MIN)
		precharge_current = CHG_PRG_ICHG_MIN;
	else if (precharge_current > CHG_PRG_ICHG_MAX)
		precharge_current = CHG_PRG_ICHG_MAX;

	prechg_limit =
		(u8)((precharge_current - CHG_PRG_ICHG_MIN) / CHG_PRG_ICHG_STEP);
	return hi6523_write_mask(CHG_PRE_ICHG_REG, CHG_PRE_ICHG_MSK,
				 CHG_PRE_ICHG_SHIFT, prechg_limit);
}

static int hi6523_set_precharge_voltage(u32 pre_vchg)
{
	u8 vprechg;

	if (pre_vchg <= CHG_PRG_VCHG_2800)
		vprechg = 0; /* 0: precharge <=2800mV */
	else if (pre_vchg > CHG_PRG_VCHG_2800 && pre_vchg <= CHG_PRG_VCHG_3000)
		vprechg = 1; /* 1: precharge <=2800mV and >3000mV */
	else if (pre_vchg > CHG_PRG_VCHG_3000 && pre_vchg <= CHG_PRG_VCHG_3100)
		vprechg = 2; /* 2: precharge <=3100mV and >3000mV */
	else if (pre_vchg > CHG_PRG_VCHG_3100 && pre_vchg <= CHG_PRG_VCHG_3200)
		vprechg = 3; /* 3: precharge <=3200mV and >3100mV */
	else
		vprechg = 0; /* default 2.8V */
	return hi6523_write_mask(CHG_PRE_VCHG_REG, CHG_PRE_VCHG_MSK,
				 CHG_PRE_VCHG_SHIFT, vprechg);
}

/* config batfet status 1:enable, 0: disable */
static int hi6523_set_batfet_ctrl(u32 status)
{
	return hi6523_write_mask(BATFET_CTRL_CFG_REG, BATFET_CTRL_CFG_MSK,
				 BATFET_CTRL_CFG_SHIFT, status);
}

/* get the charge enable in charging process */
static bool hi6523_get_charge_enable(void)
{
	u8 charge_state = 0;

	(void)hi6523_read_mask(CHG_ENABLE_REG, CHG_EN_MSK,
			       CHG_EN_SHIFT, &charge_state);

	if (charge_state)
		return TRUE;
	else
		return FALSE;
}

static void vbus_5v_set(void)
{
	/* 0xa0: reg address, 0x00: mask value */
	(void)hi6523_write_mask(0xa0, VBUS_VSET_MSK, VBUS_VSET_SHIFT, 0x00);
	/*
	 * The following addresses and values are set according chip set and
	 * the concrete meaning can be checked in the nManager or in
	 * the header file <pmic_interface.h>
	 */
	(void)hi6523_write_byte(0x78, 0x0);
	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310)) {
		(void)hi6523_write_byte(0x8e, 0xc6);
		(void)hi6523_write_byte(0x62, 0x2c);
	} else {
		(void)hi6523_write_byte(0x8e, 0xc8);
		(void)hi6523_write_byte(0x62, 0x24);
	}
	(void)hi6523_write_byte(0x64, 0x28);
	(void)hi6523_write_byte(0x71, 0x54);
	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310))
		(void)hi6523_write_byte(0x95, 0x07);
	else
		(void)hi6523_write_mask(0x95, 0x07, 0x00, 0x00);
}

static void vbus_9v_set(void)
{
	/*
	 * The following addresses and values are set according chip set and
	 * the concrete meaning can be checked in the nManager or in
	 * the header file <pmic_interface.h>
	 */
	(void)hi6523_write_mask(0xa0, VBUS_VSET_MSK, VBUS_VSET_SHIFT, 0x01);
	(void)hi6523_write_byte(0x62, 0x2C);
	(void)hi6523_write_byte(0x64, 0x40);
	(void)hi6523_write_byte(0x71, 0x55);
	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310))
		(void)hi6523_write_byte(0x95, 0x07);
	else
		(void)hi6523_write_mask(0x95, 0x07, 0x00, 0x03);
}

static void vbus_12v_set(void)
{
	/*
	 * The following addresses and values are set according chip set and
	 * the concrete meaning can be checked in the nManager or in
	 * the header file <pmic_interface.h>
	 */
	(void)hi6523_write_mask(0xa0, VBUS_VSET_MSK, VBUS_VSET_SHIFT, 0x10);
	(void)hi6523_write_byte(0x62, 0x2C);
	(void)hi6523_write_byte(0x64, 0x40);
	(void)hi6523_write_byte(0x71, 0x55);
	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310))
		(void)hi6523_write_byte(0x95, 0x07);
	else
		(void)hi6523_write_mask(0x95, 0x07, 0x00, 0x03);
}

/*
 * Parameters: vbus_vol:VBUS_VSET_5V&VBUS_VSET_9V&VBUS_VSET_12V
 *			 charge_stop_flag:1 stop charge; 0 charge
 */
static void hi6523_vbus_init_set(int vbus_vol)
{
	if (vbus_vol == VBUS_VSET_5V)
		vbus_5v_set();
	else if (vbus_vol == VBUS_VSET_9V)
		vbus_9v_set();
	else if (vbus_vol == VBUS_VSET_12V)
		vbus_12v_set();

	/* for single-phase synchronous BUCK */
	if (g_single_phase_buck == 1) {
		(void)hi6523_write_mask(CHG_OTG_RESERVE2,
			CHG_BUCK_OCP_HALVED_MASK, CHG_BUCK_OCP_HALVED_SHIFT, 1);
		scharger_inf("%s: buck ocp is halved for single-phase synchronous BUCK!\n",
			     __func__);
	}
}

/* Parameters: vbus_set voltage: 5V/9V/12V */
int hi6523_set_vbus_vset(u32 value)
{
	u8 data;
	u32 charger_flag = 0;

	/* check charge state, if open, close charge. */
	if (hi6523_get_charge_enable() == TRUE) {
		hi6523_set_charge_enable(CHG_DISABLE);
		charger_flag = 1;
	}
	if (value < VBUS_VSET_9V) {
		hi6523_vbus_init_set(VBUS_VSET_5V);
		charger_is_fcp = FCP_FALSE;
		data = 0; /* 0: the 0th bit site */
	} else if (value < VBUS_VSET_12V) {
		hi6523_vbus_init_set(VBUS_VSET_9V);
		charger_is_fcp = FCP_TRUE;
		data = 1; /* 1: the 1st bit site */
	} else {
		data = 2; /* 2: the 2nd bit site */
	}

	/* resume charge state */
	if (charger_flag == 1)
		hi6523_set_charge_enable(CHG_ENABLE);
	return hi6523_write_mask(CHG_VBUS_VSET_REG, VBUS_VSET_MSK,
				 VBUS_VSET_SHIFT, data);
}

/*
 * Parameters: vbus_vol:5V/9V/12V
 * return value: 0-success or others-fail
 */
int hi6523_config_opt_param(int vbus_vol)
{
	/*
	 * The following addresses and values are set according chip set and
	 * the concrete meaning can be checked in the nManager or in
	 * the header file <pmic_interface.h>
	 */
	if (hi6523_version == CHG_VERSION_V320 ||
		hi6523_version >= CHG_VERSION_V200) {
		(void)hi6523_write_byte(0x78, 0x0);
		(void)hi6523_write_byte(0x8e, 0x48);
		(void)hi6523_write_byte(0x62, 0x24);
		(void)hi6523_write_byte(0x64, 0x28);
		(void)hi6523_write_byte(0x66, 0xf1);
		(void)hi6523_write_mask(0xE1, 0x03 << 0x05, 0x05, 0x01);
		(void)hi6523_write_mask(0xA0, 0x01 << 0x02, 0x02, 0x01);
		(void)hi6523_write_byte(0xE0, 0x87);
		if (g_single_phase_buck == 1) {
			/* for single-phase synchronous BUCK */
			(void)hi6523_write_byte(CHG_BUCK_REG5_REG, 0x00);
			scharger_inf("%s: single-phase synchronous BUCK!\n",
				     __func__);
		} else {
			/* for dual-phase synchronous BUCK */
			(void)hi6523_write_byte(CHG_BUCK_REG5_REG, 0x80);
		}
		(void)hi6523_write_byte(0x69, 0xd4);
		(void)hi6523_write_byte(0x6a, 0x1f);
		(void)hi6523_write_byte(0x6b, 0x1c);
		(void)hi6523_write_byte(0x72, 0x5d);
		(void)hi6523_write_byte(0x73, 0x18);
		(void)hi6523_write_byte(0x74, 0x01);
		(void)hi6523_write_byte(0x75, 0xd1);
		(void)hi6523_write_byte(0x71, 0x54);
		(void)hi6523_write_byte(0x76, 0x02);
		(void)hi6523_write_byte(0x79, 0x90);
		(void)hi6523_write_byte(0x84, 0x25);
		(void)hi6523_write_byte(0x87, 0x65);
		(void)hi6523_write_byte(0x96, 0x04);
		(void)hi6523_write_byte(0x97, 0xd4);
		(void)hi6523_write_byte(0x9e, 0x8a);
		(void)hi6523_write_byte(0xA3, 0x04);
	}

	hi6523_vbus_init_set(vbus_vol);
	return 0;
}

/* return value:  0-success or others-fail */
static int hi6523_set_input_current(int cin_limit)
{
	u8  i;
	u8 i_in_limit = 0;

	if (cin_limit <= CHG_ILIMIT_100)
		i_in_limit = 0; /* 0: marking state less than CHG_ILIMIT_100 */
	else if (cin_limit > CHG_ILIMIT_3200)
	/* 26: marking state greater than CHG_ILIMIT_3200 */
		i_in_limit = 26;
	/*
	 * This "for" statement takes place of the previous tedious
	 * "else if" statements. the items in the array chg_lmt_set_v300
	 * are the Macro defined values, like #define CHG_ILIMIT_3200 3200
	 * cin_limit and i_in_limit follow the subsequent ranges and values:
	 * (CHG_ILIMIT_100, CHG_ILIMIT_130], i_in_limit is assigned to 1;
	 * (CHG_ILIMIT_130, CHG_ILIMIT_699], i_in_limit is assigned to 2;
	 * (CHG_ILIMIT_699, CHG_ILIMIT_900], i_in_limit is assigned to 3;
	 * (CHG_ILIMIT_900, CHG_ILIMIT_1000], i_in_limit is assigned to 4;
	 * (CHG_ILIMIT_1000, CHG_ILIMIT_1080], i_in_limit is assigned to 5;
	 * (CHG_ILIMIT_1080, CHG_ILIMIT_1200], i_in_limit is assigned to 6;
	 * (CHG_ILIMIT_1200, CHG_ILIMIT_1300], i_in_limit is assigned to 7;
	 * (CHG_ILIMIT_1300, CHG_ILIMIT_1400], i_in_limit is assigned to 8;
	 * (CHG_ILIMIT_1400, CHG_ILIMIT_1500], i_in_limit is assigned to 9;
	 * (CHG_ILIMIT_1500, CHG_ILIMIT_1600], i_in_limit is assigned to 10;
	 * (CHG_ILIMIT_1600, CHG_ILIMIT_1700], i_in_limit is assigned to 11;
	 * (CHG_ILIMIT_1700, CHG_ILIMIT_1800], i_in_limit is assigned to 12;
	 * (CHG_ILIMIT_1800, CHG_ILIMIT_1900], i_in_limit is assigned to 13;
	 * (CHG_ILIMIT_1900, CHG_ILIMIT_2000], i_in_limit is assigned to 14;
	 * (CHG_ILIMIT_2000, CHG_ILIMIT_2100], i_in_limit is assigned to 15;
	 * (CHG_ILIMIT_2100, CHG_ILIMIT_2200], i_in_limit is assigned to 16;
	 * (CHG_ILIMIT_2200, CHG_ILIMIT_2280], i_in_limit is assigned to 17;
	 * (CHG_ILIMIT_2280, CHG_ILIMIT_2400], i_in_limit is assigned to 18;
	 * (CHG_ILIMIT_2400, CHG_ILIMIT_2480], i_in_limit is assigned to 19;
	 * (CHG_ILIMIT_2480, CHG_ILIMIT_2600], i_in_limit is assigned to 20;
	 * (CHG_ILIMIT_2600, CHG_ILIMIT_2680], i_in_limit is assigned to 21;
	 * (CHG_ILIMIT_2680, CHG_ILIMIT_2800], i_in_limit is assigned to 22;
	 * (CHG_ILIMIT_2800, CHG_ILIMIT_2880], i_in_limit is assigned to 23;
	 * (CHG_ILIMIT_2880, CHG_ILIMIT_3000], i_in_limit is assigned to 24;
	 * (CHG_ILIMIT_3000, CHG_ILIMIT_3080], i_in_limit is assigned to 25;
	 * (CHG_ILIMIT_3080, CHG_ILIMIT_3200], i_in_limit is assigned to 26;
	 */
	for (i = 1; i < 27; i++) /* 27: number of states */
		if (cin_limit > chg_lmt_set_v300[i - 1] &&
			cin_limit <= chg_lmt_set_v300[i])
			i_in_limit = i;

	iin_set = cin_limit;

	scharger_inf("input current reg is set 0x%x\n", i_in_limit);
	return hi6523_write_mask(CHG_INPUT_SOURCE_REG, CHG_ILIMIT_MSK,
				 CHG_ILIMIT_SHIFT, i_in_limit);
}

/*
 * Description:    get the input current limit
 * return value:  ilimit mA
 */
static int hi6523_get_input_current(void)
{
	int ret = 0;
	u8 i;
	u8 reg = 0;

	(void)hi6523_read_mask(CHG_INPUT_SOURCE_REG, CHG_ILIMIT_MSK,
			       CHG_ILIMIT_SHIFT, &reg);
	/*
	 * This "for" statement takes place of the previous tedious
	 * "switch and case" statements. the items in the array chg_lmt_get_v300
	 * are the Macro defined values, like #define CHG_ILIMIT_3200 3200
	 * reg and ret follow the subsequent values, when:
	 * reg equals to 0, ret is assigned to 100;
	 * reg equals to 1, ret is assigned to 130;
	 * reg equals to 2, ret is assigned to 470;
	 * reg equals to 3, ret is assigned to 900;
	 * reg equals to 4, ret is assigned to 1000;
	 * reg equals to 5, ret is assigned to 1080;
	 * reg equals to 6, ret is assigned to 1200;
	 * reg equals to 7, ret is assigned to 1300;
	 * reg equals to 8, ret is assigned to 1400;
	 * reg equals to 9, ret is assigned to 1500;
	 * reg equals to 10, ret is assigned to 1600;
	 * reg equals to 11, ret is assigned to 1700;
	 * reg equals to 12, ret is assigned to 1800;
	 * reg equals to 13, ret is assigned to 1900;
	 * reg equals to 14, ret is assigned to 2000;
	 * reg equals to 15, ret is assigned to 2100;
	 * reg equals to 16, ret is assigned to 2200;
	 * reg equals to 17, ret is assigned to 2280;
	 * reg equals to 18, ret is assigned to 2400;
	 * reg equals to 19, ret is assigned to 2480;
	 * reg equals to 20, ret is assigned to 2600;
	 * reg equals to 21, ret is assigned to 2680;
	 * reg equals to 22, ret is assigned to 2800;
	 * reg equals to 23, ret is assigned to 2880;
	 * reg equals to 24, ret is assigned to 3000;
	 * reg equals to 25, ret is assigned to 3080;
	 * reg equals to 26, ret is assigned to 3200;
	 */
	for (i = 0; i < 27; i++) /* 27: totally 27 gears */
		if (reg == i)
			ret = chg_lmt_get_v300[i];
	return ret;
}

static int hi6523_get_input_current_set(void)
{
	return iin_set;
}

static int hi6523_get_charge_current(void)
{
	u8 reg = 0;
	int ret;
	int charge_current;

	ret = hi6523_read_mask(CHG_FAST_CURRENT_REG, CHG_FAST_ICHG_MSK,
			       CHG_FAST_ICHG_SHIFT, &reg);
	if (ret) {
		scharger_inf("HI6523 read mask fail\n");
		return CHG_FAST_ICHG_00MA;
	}
	if (reg == CURRENT_STEP_0)
		charge_current = CHG_FAST_ICHG_100MA;
	else if (reg <= CURRENT_STEP_9)
		charge_current = CHG_FAST_ICHG_STEP_100 * reg +
				CHG_FAST_ICHG_100MA;
	else if (reg <= CURRENT_STEP_14)
		charge_current =
			(reg - CURRENT_STEP_9) * CHG_FAST_ICHG_STEP_200 +
				CHG_FAST_ICHG_1000MA;
	else if (reg <= CURRENT_STEP_24)
		charge_current =
			(reg - CURRENT_STEP_14) * CHG_FAST_ICHG_STEP_100 +
				CHG_FAST_ICHG_2000MA;
	else if (reg <= CURRENT_STEP_30)
		charge_current =
			(reg - CURRENT_STEP_24) * CHG_FAST_ICHG_STEP_200 +
				CHG_FAST_ICHG_3000MA;
	else if (reg == CURRENT_STEP_31)
		charge_current = CHG_FAST_ICHG_4500MA;
	else
		charge_current = CHG_FAST_ICHG_00MA;

	scharger_inf("charge current is set %d %u\n", charge_current, reg);
	return charge_current;
}

static int hi6523_calc_charge_current_bias(int charge_current)
{
	int i_bias;
	int i_coul_ma;
	int i_delta_ma;
	static int last_ichg;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	mutex_lock(&di->ibias_calc_lock);
	/*
	 * if target is less than 1A,need not to calc ibias,need not to minus
	 * bias calculated before
	 */
	if (charge_current < CHG_FAST_ICHG_1000MA) {
		last_ichg = charge_current;
		mutex_unlock(&di->ibias_calc_lock);
		return charge_current;
	}
	/*
	 * if target charge current changed, no need to calc ibias,just use
	 * bias calculated before
	 */
	if (last_ichg != charge_current) {
		last_ichg = charge_current;
		charge_current -= i_bias_all;
		mutex_unlock(&di->ibias_calc_lock);
		return charge_current;
	}

	/*
	 * calculate bias with difference between I_coul and last target charge
	 * current
	 */
	i_coul_ma = hisi_battery_current();
	/*
	 * current from hisi_battery_current is negative while charging,change
	 * to positive to calc with charge_current
	 */
	i_coul_ma = -i_coul_ma;
	i_delta_ma = i_coul_ma - charge_current;
	/*
	 * if I_coul is less than last target charge current for more than
	 * 100ma, bias should minus 100ma
	 */
	if (i_delta_ma < -CHG_FAST_ICHG_100MA)
		i_bias = -CHG_FAST_ICHG_100MA;
	/*
	 * if difference between I_coul and last target charge current is
	 * less than 100ma, no need to add bias
	 */
	else if (i_delta_ma < CHG_FAST_ICHG_100MA)
		i_bias = 0;
	else
		/*
		 * if difference between I_coul and last target charge current
		 * is more than 100ma, calc bias with 100 rounding down
		 */
		i_bias = (i_delta_ma / 100) * 100;

	/* update i_bias_all within [0,400] mA */
	i_bias_all += i_bias;
	if (i_bias_all <= 0)
		i_bias_all = 0;
	if (i_bias_all >= CHG_FAST_ICHG_400MA)
		i_bias_all = CHG_FAST_ICHG_400MA;

	/* update charge current */
	charge_current -= i_bias_all;
	scharger_inf("%s:Ichg:%d, i_coul_ma:%d, Ibias:%d, i_bias_all:%d\n",
		     __func__, charge_current, i_coul_ma, i_bias, i_bias_all);
	mutex_unlock(&di->ibias_calc_lock);
	return charge_current;
}

static int hi6523_set_charge_current(int charge_current)
{
	u8 i_chg_limit;

	charge_current = hi6523_calc_charge_current_bias(charge_current);
	if (charge_current <= CHG_FAST_ICHG_100MA)
		i_chg_limit = 0;
	else if (charge_current > CHG_FAST_ICHG_100MA &&
			charge_current <= CHG_FAST_ICHG_1000MA)
		i_chg_limit = (charge_current - CHG_FAST_ICHG_100MA) /
				CHG_FAST_ICHG_STEP_100;
	else if (charge_current > CHG_FAST_ICHG_1000MA &&
			charge_current <= CHG_FAST_ICHG_2000MA)
		i_chg_limit = (charge_current - CHG_FAST_ICHG_1000MA) /
				CHG_FAST_ICHG_STEP_200 + 9; /* 9: 9mA */
	else if (charge_current > CHG_FAST_ICHG_2000MA &&
			charge_current <= CHG_FAST_ICHG_3000MA)
		i_chg_limit = (charge_current - CHG_FAST_ICHG_2000MA) /
				CHG_FAST_ICHG_STEP_100 + 14; /* 14: 14mA */
	else if (charge_current > CHG_FAST_ICHG_3000MA &&
			charge_current <= CHG_FAST_ICHG_4200MA)
		i_chg_limit = (charge_current - CHG_FAST_ICHG_3000MA) /
				CHG_FAST_ICHG_STEP_200 + 24; /* 24: 24mA */
	else if (charge_current > CHG_FAST_ICHG_4200MA &&
			charge_current < CHG_FAST_ICHG_4500MA)
		i_chg_limit = 30; /* 30: 30mA */
	else
		i_chg_limit = 31; /* 31: 31mA */

	scharger_inf("charge current reg is set 0x%x\n", i_chg_limit);
	return hi6523_write_mask(CHG_FAST_CURRENT_REG, CHG_FAST_ICHG_MSK,
				 CHG_FAST_ICHG_SHIFT, i_chg_limit);
}

/*
 * set the terminal voltage in charging process
 *	 (v300&v310 scharger's max cv is 4.25V due to lx bug)
 * return value: 0-success or others-fail
 */
static int hi6523_set_terminal_voltage(int charge_voltage)
{
	u8 data;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (charge_voltage < CHG_FAST_VCHG_MIN)
		charge_voltage = CHG_FAST_VCHG_MIN;
	else if (charge_voltage > CHG_FAST_VCHG_MAX)
		charge_voltage = CHG_FAST_VCHG_MAX;

	if (is_hi6523_cv_limit() == TRUE)
		if (charge_voltage > CHG_FAST_VCHG_4250)
			charge_voltage = CHG_FAST_VCHG_4250;

	di->term_vol_mv = charge_voltage;

	/* need not do dpm, set vterm directly */
	if (di->param_dts.dpm_en == DPM_ENABLE) {
		if (is_weaksource == WEAKSOURCE_TRUE) {
			scharger_err("weaksource true, do not set CV\n");
			return 0;
		}
		scharger_inf("no need do vdpm, just set cv!\n");
	}

	data = (u8)((charge_voltage - CHG_FAST_VCHG_MIN) /
						CHG_FAST_VCHG_STEP_50);
	return hi6523_write_mask(CHG_FAST_VCHG_REG, CHG_FAST_VCHG_MSK,
				 CHG_FAST_VCHG_SHIFT, data);
}

static int hi6523_get_terminal_voltage(void)
{
	u8 data = 0;

	(void)hi6523_read_mask(CHG_FAST_VCHG_REG, CHG_FAST_VCHG_MSK,
			       CHG_FAST_VCHG_SHIFT, &data);
	return (int)(data * CHG_FAST_VCHG_STEP_50 + CHG_FAST_VCHG_MIN);
}

/*
 * Description: check whether VINDPM or IINDPM
 * return value: TRUE means VINDPM or IINDPM
 *               FALSE means NoT DPM
 */
static int hi6523_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = hi6523_read_byte(CHG_STATUS0_REG, &reg);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return ret;
	}

	if ((reg & CHG_IN_DPM_STATE) == CHG_IN_DPM_STATE) {
		scharger_inf("CHG_STATUS0_REG:0x%2x in dpm state\n", reg);
		return TRUE;
	}
	return FALSE;
}

/*
 * Description:  check whether ACL
 * return value: TRUE means ACL
 *               FALSE means NoT ACL
 */
static int hi6523_check_input_acl_state(void)
{
	u8 reg = 0;
	int ret;

	ret = hi6523_read_byte(CHG_STATUS0_REG, &reg);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return ret;
	}
	if ((reg & CHG_IN_ACL_STATE) == CHG_IN_ACL_STATE) {
		scharger_inf("CHG_STATUS0_REG:0x%2x in acl state\n", reg);
		return TRUE;
	}
	return FALSE;
}

static int hi6523_get_charge_state(unsigned int *state)
{
	u8 reg1 = 0;
	u8 reg2 = 0;
	int ret0, ret1;

	ret0 = hi6523_read_byte(CHG_STATUS0_REG, &reg1);
	ret1 = hi6523_read_byte(CHG_STATUS1_REG, &reg2);
	if (ret0 || ret1) {
		scharger_err("[%s]read charge status reg fail\n", __func__);
		return -1;
	}

	if ((reg1 & HI6523_CHG_BUCK_OK) != HI6523_CHG_BUCK_OK)
		*state |= CHAGRE_STATE_NOT_PG;
	if ((reg1 & HI6523_CHG_STAT_CHARGE_DONE) == HI6523_CHG_STAT_CHARGE_DONE)
		*state |= CHAGRE_STATE_CHRG_DONE;
	if ((reg2 & HI6523_WATCHDOG_OK) != HI6523_WATCHDOG_OK)
		*state |= CHAGRE_STATE_WDT_FAULT;
	return 0;
}

static bool hi6523_get_anti_reverbst_enabled(void)
{
	u8 reg_val = 0;

	(void)hi6523_read_mask(CHG_ANTI_REVERBST_REG, CHG_ANTI_REVERBST_EN_MSK,
			       CHG_ANTI_REVERBST_EN_SHIFT, &reg_val);

	if (reg_val != CHG_ANTI_REVERBST_EN)
		return FALSE;
	return TRUE;
}

/*
 * check the charger and the line is weak source
 * return value: TRUE: weak source FALSE: not weak_source
 */
static int check_weaksource(void)
{
	/* init return val, test result, and temp val to 0 */
	int ret, result;
	int orig_iin, current_iin;

	orig_iin = hi6523_get_input_current();
	ret = hi6523_set_input_current(WEAK_IIN_THRESHOLD);
	if (ret) {
		scharger_err("%s set inputcurrent fail!\n", __func__);
		return WEAK_TEMP_FALSE;
	}
	msleep(WEAK_CHECK_WAIT_100);
	current_iin = hi6523_get_input_current();
	if (current_iin != WEAK_IIN_THRESHOLD) {
		scharger_err("during weak check,iin changed,return false!\n");
		return WEAK_TEMP_FALSE;
	}
	if (hi6523_check_input_dpm_state()) {
		result = WEAK_TEMP_TRUE;
		scharger_err("%s weak source\n", __func__);
	} else {
		result = WEAK_TEMP_FALSE;
		scharger_err("%s not weak source\n", __func__);
	}
	ret = hi6523_set_input_current(orig_iin);
	if (ret)
		scharger_err("%s set inputcurrent fail!\n", __func__);

	return result;
}

static int hi6523_apply_set_vdpm_cv(int *vterm, int vterm_vdpm_tbl_len,
		int *vdpm_set_val, u8 *vterm_set, int *ret_vdpm, int *ret_vterm,
		struct hi6523_device_info *di, int *last_vdpm)
{
	int tmp_index, vterm_tmp;

	scharger_err("Can't find Vdpm-Vterm, apply original setting\n");
	/* set Vdpm to 4.675V; CV to 4.4V */
	tmp_index = vterm_vdpm_tbl_len - 1;
	vterm_tmp = vterm_vdpm[tmp_index].cv;
	*vdpm_set_val = vterm_vdpm[tmp_index].vdpm_set_val;
	vterm_tmp = di->term_vol_mv < vterm_tmp ? di->term_vol_mv : vterm_tmp;
	*vterm_set = (u8)((vterm_tmp - CHG_FAST_VCHG_MIN) / CHG_FAST_VCHG_STEP_50);
	*vterm = vterm_tmp;
	*ret_vdpm = hi6523_write_mask(CHG_OTG_RESERVE2,
			CHG_5V_DPM_SHIFT_MSK, CHG_5V_DPM_SHIFT, *vdpm_set_val);
	if (*ret_vdpm == 0)
		*ret_vterm = hi6523_write_mask(CHG_FAST_VCHG_REG,
				CHG_FAST_VCHG_MSK, CHG_FAST_VCHG_SHIFT,
				*vterm_set);
	*last_vdpm = VINDPM_4700;

	return (*ret_vdpm || *ret_vterm);
}

static void hi6523_apply_judge_vindpm(
	int vindpm, int *last_vdpm, int *ret_vdpm, int *ret_vterm,
	int vdpm_set_val, u8 vterm_set)
{
	if (vindpm >= *last_vdpm) {
		/* first set Vdpm. then set Vterm */
		*ret_vdpm = hi6523_write_mask(CHG_OTG_RESERVE2,
				CHG_5V_DPM_SHIFT_MSK, CHG_5V_DPM_SHIFT,
				vdpm_set_val);
		if (*ret_vdpm == 0) {
			*last_vdpm = vindpm;
			*ret_vterm = hi6523_write_mask(CHG_FAST_VCHG_REG,
					CHG_FAST_VCHG_MSK, CHG_FAST_VCHG_SHIFT,
					vterm_set);
		}
	} else {
		/* first set Vterm, then set Vdpm */
		*ret_vterm = hi6523_write_mask(CHG_FAST_VCHG_REG,
			CHG_FAST_VCHG_MSK, CHG_FAST_VCHG_SHIFT, vterm_set);
		if (*ret_vterm == 0) {
			*ret_vdpm = hi6523_write_mask(CHG_OTG_RESERVE2,
					CHG_5V_DPM_SHIFT_MSK, CHG_5V_DPM_SHIFT,
					vdpm_set_val);
			if (*ret_vdpm == 0)
				*last_vdpm = vindpm;
		}
	}
}

/*
 * Parameters: vindpm-current Vdpm selected by Vbat trend
 * return value: 0: not need do dpm or Vdpm-Vterm set ok
 *               1: Vdpm-Vterm not set ok
 */
static int hi6523_apply_dpm_policy(int vindpm)
{
	/* init return val and temp val to 0 */
	int i;
	int ret = 10; /* set to 10 to avoid duplication and misleading */
	int vterm = 0;
	int vdpm_set_val = 0;
	u8 vterm_set = 0;
	u8 vterm_set_tmp;
	int ret_vterm = 0;
	int ret_vdpm = 0;
	int vterm_vdpm_tbl_len;
	static int last_vdpm = VINDPM_4700; /* init last_vdpm to max */
	static int weak_check_cnt;
	static int weak_sum;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return 0; /* not do dpm */
	}

	if (di->charger_type != CHARGER_TYPE_DCP || charger_is_fcp == FCP_TRUE) {
		scharger_err("charger_type is %d, do not modify dpm\n",
			     di->charger_type);
		return 0; /* not do dpm */
	}

	/* charger type is DCP, first insert, do nothing */
	if (first_insert_flag == FIRST_INSERT_TRUE) {
		first_insert_flag = FIRST_INSERT_FALSE;
		is_weaksource = WEAKSOURCE_FALSE;
		weak_check_cnt = 0;
		weak_sum = 0;
		last_vdpm = VINDPM_4700;
		return 0; /* not do dpm */
	}

	/* start check weaksource, check two times */
	if (weak_check_cnt < WEAKSOURCE_CHECK_MAX_CNT) {
		if (check_weaksource() == WEAK_TEMP_TRUE)
			weak_sum++;

		weak_check_cnt++;
		if (weak_check_cnt == WEAKSOURCE_CHECK_MAX_CNT) {
			if (weak_sum >= WEAKSOURCE_CONFIRM_THRES) {
				is_weaksource = WEAKSOURCE_TRUE;
				atomic_notifier_call_chain(&fault_notifier_list,
					CHARGE_FAULT_WEAKSOURCE, NULL);
			} else {
				is_weaksource = WEAKSOURCE_FALSE;
			}
		} else {
			return 0; /* check times is less than 3,just return */
		}
	}

	if (is_weaksource == WEAKSOURCE_FALSE) {
		scharger_inf("weaksource false, not do dpm\n");
		return 0; /* not do dpm */
	}
	scharger_inf("weaksource true, do dpm and cv\n");

	/* select vterm according to Vdpm_set */
	vterm_vdpm_tbl_len = sizeof(vterm_vdpm) / (sizeof(vterm_vdpm[0]));
	scharger_inf("vterm_vdpm_tbl_len is %d\n", vterm_vdpm_tbl_len);
	for (i = 0; i < vterm_vdpm_tbl_len; i++) {
		if (vindpm == vterm_vdpm[i].vdpm) {
			vterm = vterm_vdpm[i].cv;
			vdpm_set_val = vterm_vdpm[i].vdpm_set_val;
			break;
		}
	}

	if (i == vterm_vdpm_tbl_len) {
		ret = hi6523_apply_set_vdpm_cv(&vterm, vterm_vdpm_tbl_len,
			&vdpm_set_val, &vterm_set, &ret_vdpm, &ret_vterm, di,
			&last_vdpm);
		return ret;
	}

	vterm = di->term_vol_mv < vterm ? di->term_vol_mv : vterm;
	vterm_set_tmp = (u8)((vterm - CHG_FAST_VCHG_MIN) / CHG_FAST_VCHG_STEP_50);
	if (vterm_set_tmp > HI6523_MAX_U8) {
		scharger_err("%s vterm_set = %d, out of u8 range, is now set to 0\n",
			     __func__, vterm_set_tmp);
		vterm_set = 0;
	} else {
		vterm_set = vterm_set_tmp;
	}
	scharger_inf("Do dpm_set, Vdpm: %d, Vterm: %d\n", vindpm, vterm);
	hi6523_apply_judge_vindpm(vindpm, &last_vdpm, &ret_vdpm, &ret_vterm,
				  vdpm_set_val, vterm_set);
	return (ret_vdpm || ret_vterm);
}

static int hi6523_set_dpm_voltage(int vindpm)
{
	int ret = 0;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return ret;
	}

	if (di->param_dts.dpm_en == DPM_ENABLE)
		ret = hi6523_apply_dpm_policy(vindpm);

	return ret;
}

/*
 * (min value is 400ma for scharger ic bug)
 * return value: 0-success or others-fail
 */
static int hi6523_set_terminal_current(int term_current)
{
	u8 i_term;

	if (term_current <= CHG_TERM_ICHG_400MA)
	/* 5: reg val for marking CHG_TERM_ICHG_400MA */
		i_term = 5;
	else if (term_current > CHG_TERM_ICHG_400MA &&
		term_current <= CHG_TERM_ICHG_500MA)
	/* 6: reg val for marking (CHG_TERM_ICHG_400MA, CHG_TERM_ICHG_500MA] */
		i_term = 6;
	else if (term_current > CHG_TERM_ICHG_500MA &&
		term_current <= CHG_TERM_ICHG_600MA)
	/* 7: reg val for marking (CHG_TERM_ICHG_500MA, CHG_TERM_ICHG_600MA] */
		i_term = 7;
	else
		i_term = 5; /* default 400mA */

	scharger_inf("term current reg is set 0x%x\n", i_term);
	return hi6523_write_mask(CHG_TERM_ICHG_REG, CHG_TERM_ICHG_MSK,
				 CHG_TERM_ICHG_SHIFT, i_term);
}

static int hi6523_set_charge_enable(int enable)
{
	/* invalidate charge enable on udp board */
	if ((is_board_type == BAT_BOARD_UDP) && (enable == CHG_ENABLE))
		return 0;
	return hi6523_write_mask(CHG_ENABLE_REG, CHG_EN_MSK,
				 CHG_EN_SHIFT, enable);
}

/*
 * Description: config hi6523 opt params for otg
 * return value: 0-success or others-fail
 */
static int hi6523_config_otg_opt_param(void)
{
	u8 otg_lim_set = 0;
	int ret, ret0, ret1, ret2, ret3, ret4, ret5;

	(void)hi6523_read_mask(CHG_OTG_REG0, CHG_OTG_LIM_MSK,
			       CHG_OTG_LIM_SHIFT, &otg_lim_set);
	/*
	 * The following addresses and values are set according chip set and
	 * the concrete meaning can be checked in the nManager or in
	 * the header file <pmic_interface.h>
	 */
	ret0 = hi6523_write_byte(0x6a, 0x3f);
	ret1 = hi6523_write_byte(0x6b, 0x5c);
	ret2 = hi6523_write_byte(0x81, 0x5c);
	ret3 = hi6523_write_byte(0x86, 0x39);
	/* need to config regs according to otg current */
	if (otg_lim_set != 3) {
		ret4 = hi6523_write_byte(0x8a, 0x10);
		ret5 = hi6523_write_byte(0x88, 0xbf);
	} else {
		ret4 = hi6523_write_byte(0x8a, 0x18);
		ret5 = hi6523_write_byte(0x88, 0x9f);
	}
	ret = (ret0 || ret1 || ret2 || ret3 || ret4 || ret5);
	return ret;
}

static int hi6523_set_otg_enable(int enable)
{
	if (enable) {
		(void)hi6523_set_charge_enable(CHG_DISABLE);
		(void)hi6523_config_otg_opt_param();
	}
	return hi6523_write_mask(CHG_OTG_REG0, CHG_OTG_EN_MSK,
				 CHG_OTG_EN_SHIFT, enable);
}

static int hi6523_set_otg_current(int value)
{
	u8 reg = 0;
	unsigned int temp_current_ma;

	temp_current_ma = (unsigned int) value;

	if (temp_current_ma < BOOST_LIM_MIN || temp_current_ma > BOOST_LIM_MAX)
		scharger_inf("set otg current %dmA is out of range!\n", value);
	if (temp_current_ma < BOOST_LIM_500)
		reg = 3; /* 3: cboost current >= 500mA */
	else if (temp_current_ma >= BOOST_LIM_500 &&
					temp_current_ma < BOOST_LIM_1000)
		reg = 3; /* 3: cboost current >=500mA and <1000mA */
	else if (temp_current_ma >= BOOST_LIM_1000 &&
					temp_current_ma < BOOST_LIM_1500)
		reg = 1; /* 1: cboost current >=1000mA and <1500mA */
	else if (temp_current_ma >= BOOST_LIM_1500 &&
					temp_current_ma < BOOST_LIM_2000)
		reg = 0; /* 0: cboost current >=1500mA and <2000mA */
	else
		reg = 0; /* other current values */

	scharger_inf("otg current reg is set 0x%x\n", reg);
	return hi6523_write_mask(CHG_OTG_REG0, CHG_OTG_LIM_MSK,
				 CHG_OTG_LIM_SHIFT, reg);
}

/*
 * judge if in dpm by vbus vol
 * Parameters: vbus mV
 */
static bool hi6523_vbus_in_dpm(int vbus)
{
	if ((hi6523_version == CHG_VERSION_V300) ||
		(hi6523_version == CHG_VERSION_V310)) {
		if (vbus <= CHG_DPM_VOL_4855_MV)
			return TRUE;
		else
			return FALSE;
	} else {
		if (vbus <= CHG_DPM_VOL_4835_MV)
			return TRUE;
		else
			return FALSE;
	}
}

/* return value:  0-success or others-fail */
static int hi6523_set_term_enable(int enable)
{
	int ret;
	int vbatt_mv;
	int term_mv;
	int vbus = 0;
	u8 state = 0;

	if (hi6523_force_set_term_flag == CHG_STAT_ENABLE) {
		scharger_inf("Charger is in the production line test phase!\n");
		return 0;
	}

	if (enable == 0)
		return hi6523_write_mask(CHG_EN_TERM_REG, CHG_EN_TERM_MSK,
					 CHG_EN_TERM_SHIFT, (u8)enable);
	ret = hi6523_read_byte(CHG_STATUS0_REG, &state);
	if (ret != 0)
		scharger_err("hi6523_read_byte err\n");
	vbatt_mv = hisi_battery_voltage();
	term_mv = hi6523_get_terminal_voltage();
	ret = hi6523_get_vbus_mv((unsigned int *)&vbus); /* declared in comm.h */
	if (ret)
		return ret;
	if (((CHG_TERM_ABLE_STATE & state) == 0) &&
		((hi6523_vbus_in_dpm(vbus) == FALSE) ||
		((CHG_IN_DPM_STATE & state) == 0)) &&
		(vbatt_mv > (term_mv - 100))) { /* 100: reference voltage */
		return hi6523_write_mask(CHG_EN_TERM_REG, CHG_EN_TERM_MSK,
					 CHG_EN_TERM_SHIFT, (u8)enable);
	} else {
		scharger_inf("cancel term_en state:0x%x,vbatt_mv:%d, term_mv:%d\n",
			     state, vbatt_mv, term_mv);
		/* close EOC */
		(void)hi6523_write_mask(CHG_EN_TERM_REG, CHG_EN_TERM_MSK,
					CHG_EN_TERM_SHIFT, (u8)CHG_TERM_DIS);
		return -1;
	}
}

/*
 * Parameters:   enable:terminal enable or not
 *               0&1:dbc control. 2:original charger procedure
 * return value:  0-success or others-fail
 */
static int hi6523_force_set_term_enable(int enable)
{
	if ((enable == 0) || (enable == 1)) {
		hi6523_force_set_term_flag = CHG_STAT_ENABLE;
	} else {
		hi6523_force_set_term_flag = CHG_STAT_DISABLE;
		return 0;
	}
	return hi6523_write_mask(CHG_EN_TERM_REG, CHG_EN_TERM_MSK,
				 CHG_EN_TERM_SHIFT, (u8)enable);
}

/*
 * return value: charger state
 *               1:Charge Termination Done
 *               0:Not Charging and buck is closed;Pre-charge;Fast-charg;
 *              -1:error
 */
static int hi6523_get_charger_state(void)
{
	u8 data = 0;
	int ret;

	ret = hi6523_read_byte(CHG_STATUS0_REG, &data);
	if (ret) {
		scharger_err("[%s]:read charge status reg fail\n", __func__);
		return -1;
	}

	/* Charge buck was not work */
	if ((HI6523_CHG_BUCK_OK & data) == 0)
		return 0;

	data &= HI6523_CHG_STAT_MASK;
	data = data >> HI6523_CHG_STAT_SHIFT;
	switch (data) {
	case 0: /* charger not onsite */
	case 1: /* charger not onsite */
	case 2: /* charger not onsite */
		ret = 0; /* charger not onsite */
		break;
	case 3:
		ret = 1; /* charger onb site */
		break;
	default:
		scharger_err("get charger state fail\n");
		break;
	}

	return ret;
}

static bool hi6523_is_vbus_adc_ready(void)
{
	u8 reg_val = 0;

	(void)hi6523_read_mask(CHG_ADC_VBUS_RDY_REG, CHG_ADC_VBUS_RDY_MSK,
			       CHG_ADC_VBUS_RDY_SHIFT, &reg_val);
	if (reg_val == CHG_ADC_VBUS_RDY)
		return TRUE;

	return FALSE;
}

/* Parameters: gain_cal-from efuse value */
static int hi6523_get_vbus_gain_cal(int *vol)
{
	int ret;
	int reg;
	int value;
	u8 reg_val = 0;

	ret = hi6523_read_byte(CHG_EFUSE_WE7_REG, &reg_val);
	if (ret) {
		scharger_err("[%s]get efuse fail,ret:%d\n", __func__, ret);
		return ret;
	}
	/* 1: 1 bit right moving; 4: 4 bits right moving */
	value = (reg_val & CHG_GAIN_CAL_VALUE_MASK) >> 1;
	reg = (reg_val & CHG_GAIN_CAL_REG_MASK) >> 4;
	if (reg == 0x1)
		*vol = value * (-1);
	else
		*vol = value;
	return 0;
}

int hi6523_get_vbus_mv(unsigned int *vbus_mv)
{
	int ret;
	u32 result = 0;
	const u32 vref = 2000; /* 2000mV */
	u8 data = 0;
	unsigned int tmp;

	if (hi6523_is_vbus_adc_ready() == FALSE) {
		(void)hi6523_read_byte(CHG_ADC_VBUS_RDY_REG, &data);
		*vbus_mv = 0;
		return 0;
	}
	ret = hi6523_get_adc_value(CHG_ADC_CH_VBUS, &result);
	if (ret) {
		scharger_err("[%s]get vbus_mv fail,ret:%d\n", __func__, ret);
		return -1;
	}
	/*
	 * transfer from code value to actual value
	 * 800: the gain of voltage
	 * 7: 7 gears;
	 * 4096: 12 bits to record temperature, 2^12 = 4096;
	 * 100000: base value;
	 * 125: 7 bits for storing the value;
	 */
	if (gain_cal != 800) {
		tmp = result * vref * 7 / 4096 * (100000 - 125 * gain_cal) / 100000;
	} else {
		scharger_err("[%s]zero divisor occurs, tmp is now set to 0\n");
		tmp = 0; /* this condition deals with zero divisor */
	}

	if (tmp > HI6523_MAX_UINT) {
		scharger_err("[%s]vbus_mv = :%u, out of unsigned int range, is now set to 0\n",
			     __func__, tmp);
		*vbus_mv = 0;
	} else {
		*vbus_mv = tmp;
	}

	return ret;
}

static int hi6523_reset_watchdog_timer(void)
{
	return hi6523_write_mask(WATCHDOG_SOFT_RST_REG, WD_RST_N_MSK,
				 WATCHDOG_TIMER_SHIFT, WATCHDOG_TIMER_RST);
}

/* get average value for ilim */
static int hi6523_get_ibus_ma(void)
{
	int ret;
	int ilimit;
	u32 ibus_ref_data = 0;
	u32 ibus_data = 0;
	u32 ibus, ibus_tmp;
	u32 state = 0;

	ret = hi6523_get_charge_state(&state);
	if (ret) {
		scharger_err("[%s] get_chg_state fail,ret:%d\n", __func__, ret);
		return -1;
	}
	if (CHAGRE_STATE_NOT_PG & state)
		return 0;

	ret = hi6523_get_adc_value(CHG_ADC_CH_IBUS_REF, &ibus_ref_data);
	if (ret) {
		scharger_err("[%s]get ibus_ref_data err, ret:%d\n",
			     __func__, ret);
		return -1;
	}

	ret = hi6523_get_adc_value(CHG_ADC_CH_IBUS, &ibus_data);
	if (ret) {
		scharger_err("[%s]get ibus_data fail, ret:%d\n", __func__, ret);
		return -1;
	}

	ilimit = hi6523_get_input_current();
	if (ibus_ref_data == 0)
		return -1;

	ibus_tmp = ibus_data * ilimit / ibus_ref_data;

	if (ibus_tmp > HI6523_MAX_U32) {
		scharger_err("[%s]ibus = %u, out of u32 range, is now set to 0\n",
			     __func__, ibus_tmp);
		ibus = 0;
	} else {
		ibus = ibus_tmp;
	}

	return ibus;
}

/* get vsys sample */
static int hi6523_vbat_sys(void)
{
	int i = 0;
	int v_sample = -1;
	const int retry_times = 3;

	if (adc_channel_vbat_sys < 0)
		return v_sample;

	for (i = 0; i < retry_times; ++i) {
		v_sample = hisi_adc_get_value(adc_channel_vbat_sys);
		if (v_sample < 0)
			scharger_err("adc read channel 15 fail!\n");
		else
			break;
	}
	return v_sample;
}

/* get vsys voltage */
static int hi6523_get_vbat_sys(void)
{
	int i;
	int v_temp;
	int cnt = 0;
	int sum = 0;
	const int sample_num = 5; /* use 5 samples to get an average value */
	const int delay_times = 100;

	for (i = 0; i < sample_num; ++i) {
		v_temp = hi6523_vbat_sys();
		if (v_temp >= 0) {
			sum += v_temp;
			++cnt;
		} else {
			scharger_err("hi6523 get v_temp fail!\n");
		}
		msleep(delay_times);
	}
	if (cnt > 0)
		return (3 * 1000 * sum / cnt); /* average battery vol value */

	scharger_err("use 0 as default Vvlim!\n");
	return 0;
}

/* print the register value in charging process */
static int hi6523_dump_register(char *reg_value, int size)
{
	u8 reg[HI6523_REG_TOTAL_NUM] = {0};
	char buff[26] = {0}; /* 26: the length of register */
	int i;
	int ret;
	int vbus = 0;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	memset(reg_value, 0, size);
	ret = hi6523_read_block(di, &reg[0], 0, HI6523_REG_TOTAL_NUM);
	if (ret != 0)
		scharger_err("%s hi6523_read_block failed!\n", __func__);
	ret = hi6523_get_vbus_mv((unsigned int *)&vbus);
	if (ret != 0)
		scharger_err("%s hi6523_get_vbus_mv failed!\n", __func__);

	snprintf(buff, 26, "%-8.2d", hi6523_get_ibus_ma());
	strncat(reg_value, buff, strlen(buff));
	snprintf(buff, 26UL, "%-8.2d", vbus);
	strncat(reg_value, buff, strlen(buff));
	snprintf(buff, 26UL, "%-8.2d", i_bias_all);
	strncat(reg_value, buff, strlen(buff));
	for (i = 0; i < HI6523_REG_TOTAL_NUM; i++) {
		snprintf(buff, 26, "0x%-8x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}
	return 0;
}

/* print the register head in charging process */
static int hi6523_get_register_head(char *reg_head, int size)
{
	char buff[26] = {0}; /* 26: the length of register */
	int i;

	memset(reg_head, 0, size);
	snprintf(buff, 26, "Ibus    ");
	strncat(reg_head, buff, strlen(buff));
	snprintf(buff, 26UL, "Vbus    ");
	strncat(reg_head, buff, strlen(buff));
	snprintf(buff, 26UL, "Ibias   ");
	strncat(reg_head, buff, strlen(buff));
	for (i = 0; i < HI6523_REG_TOTAL_NUM; i++) {
		snprintf(buff, 26, "Reg[0x%2x] ", i);
		strncat(reg_head, buff, strlen(buff));
	}
	return 0;
}

static int hi6523_set_batfet_disable(int disable)
{
	return hi6523_set_batfet_ctrl(!disable);
}

/* return value:  0-success or others-fail */
static int hi6523_set_watchdog_timer(int value)
{
	u8 val;
	u8 dog_time = (u8)value;

	if (dog_time >= WATCHDOG_TIMER_40_S)
		val = 3; /* for 40S timer */
	else if (dog_time >= WATCHDOG_TIMER_20_S)
		val = 2; /* for 20S timer */
	else if (dog_time >= WATCHDOG_TIMER_10_S)
		val = 1; /* for 10S timer */
	else
		val = 0; /* for other timer */

	scharger_inf("watch dog timer is %u,the register value is set %u\n",
		     dog_time, val);
	return hi6523_write_mask(WATCHDOG_CTRL_REG, WATCHDOG_TIMER_MSK,
				 WATCHDOG_TIMER_SHIFT, val);
}

/* return value: 0-success or others-fail */
static int hi6523_set_charger_hiz(int enable)
{
	return hi6523_write_mask(CHG_HIZ_CTRL_REG, CHG_HIZ_ENABLE_MSK,
				 CHG_HIZ_ENABLE_SHIFT, enable);
}

/* judge is charger in hiz or not */
static bool hi6523_is_charger_hiz(void)
{
	u8 hiz_state = 0;

	(void)hi6523_read_mask(CHG_HIZ_CTRL_REG, CHG_HIZ_ENABLE_MSK,
			       CHG_HIZ_ENABLE_SHIFT, &hiz_state);
	if (hiz_state)
		return TRUE;
	return FALSE;
}

/* return value: 0: success, -1: other fail */
int hi6523_apple_adapter_detect(int enable)
{
	return hi6523_write_mask(CHG_ADC_APPDET_REG, APPLE_DETECT_MASK,
				 APPLE_DETECT_SHIFT, enable);
}

/* Description:   check voltage of DM */
static bool is_dm_water_intrused(u32 dm_vol)
{
	int i;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL)
		return FALSE;

	if (dm_array_len == 0) {
		if (dm_vol > HI6523_DPDM_WATER_THRESH_1460MV &&
				dm_vol < HI6523_DPDM_WATER_THRESH_1490MV)
			return TRUE;
		else if (dm_vol > HI6523_DPDM_WATER_THRESH_1560MV &&
				dm_vol < HI6523_DPDM_WATER_THRESH_1590MV)
			return TRUE;
		else
			return FALSE;
	} else {
		for (i = 0; i < dm_array_len / WATER_VOLT_PARA; i++) {
			if (dm_vol > di->param_dts.scharger_check_vol.dm_vol_data[i].vol_min &&
				dm_vol < di->param_dts.scharger_check_vol.dm_vol_data[i].vol_max)
				return TRUE;
		}
		return FALSE;
	}
}

/* Description:   check voltage of DM */
static bool is_dp_water_intrused(u32 dp_vol)
{
	int i;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL)
		return FALSE;

	if (dp_array_len == 0) {
		if (dp_vol > HI6523_DPDM_WATER_THRESH_1460MV &&
		dp_vol < HI6523_DPDM_WATER_THRESH_1490MV)
			return TRUE;
		else if (dp_vol > HI6523_DPDM_WATER_THRESH_1560MV &&
			dp_vol < HI6523_DPDM_WATER_THRESH_1590MV)
			return TRUE;
		else
			return FALSE;
	} else {
		for (i = 0; i < dp_array_len / WATER_VOLT_PARA; i++) {
			if (dp_vol > di->param_dts.scharger_check_vol.dp_vol_data[i].vol_min &&
				dp_vol < di->param_dts.scharger_check_vol.dp_vol_data[i].vol_max)
				return TRUE;
		}
		return FALSE;
	}
}

/*
 * check voltage of DN/DP
 * return value: 1:water intrused/ 0:water not intrused/ -ENOMEM:hi6523 is not
 * initializied
 */
int hi6523_is_water_intrused(void)
{
	u32 dm_data = 0;
	u32 dp_data = 0;
	u32 dm_vol = 0;
	u32 dp_vol;
	int i, ret;

	if (g_hi6523_dev == NULL)
		return -ENOMEM;

	(void)hi6523_write_mask(CHG_FCP_CTRL_REG, CHG_FCP_EN_MSK,
				CHG_FCP_EN_SHIFT, FALSE);
	(void)hi6523_apple_adapter_detect(CHG_ADC_APPDET_EN);
	usleep_range(9000, 10000); /* (9000, 10000): sleep range */
	(void)hi6523_write_mask(CHG_ADC_APPDET_REG, CHG_ADC_APPDET_CHSEL_MSK,
			   CHG_ADC_APPDET_CHSEL_SHIFT, CHG_ADC_APPDET_DMINUS);
	for (i = 0; i < HI6523_WATER_CHECKDPDN_NUM; i++) {
		ret = hi6523_get_adc_value(CHG_ADC_CH_DET, &dm_data);
		if (ret != 0)
			scharger_err("%s hi6523_get_adc_value err\n", __func__);
		dm_vol = HI6523_DPDM_CALC_MV(dm_data);
		scharger_inf("%s:DM_VOL:%u mv, data:0x%x\n",
			     __func__, dm_vol, dm_data);
		if (is_dm_water_intrused(dm_vol) == FALSE)
			break;
	}
	if (i == HI6523_WATER_CHECKDPDN_NUM) {
		scharger_inf("D- water intrused\n");
		return 1;
	}
	(void)hi6523_write_mask(CHG_ADC_APPDET_REG, CHG_ADC_APPDET_CHSEL_MSK,
			    CHG_ADC_APPDET_CHSEL_SHIFT, CHG_ADC_APPDET_DPLUS);
	for (i = 0; i < HI6523_WATER_CHECKDPDN_NUM; i++) {
		ret = hi6523_get_adc_value(CHG_ADC_CH_DET, &dp_data);
		if (ret != 0)
			scharger_err("%s hi6523_get_adc_value err\n", __func__);
		dp_vol = HI6523_DPDM_CALC_MV(dp_data);
		scharger_inf("%s:DP_VOL:%u mV, data:0x%x\n",
			     __func__, dp_vol, dp_data);
		if (is_dp_water_intrused(dp_vol) == FALSE)
			return 0;
	}
	scharger_inf("D+ water intrused\n");
	return 1;
}

static int hi6523_soft_vbatt_ovp_protect(void)
{
	struct hi6523_device_info *di = g_hi6523_dev;
	int vbatt_mv, vbatt_max;

	if (di == NULL)
		return -1;
	vbatt_mv = hisi_battery_voltage();
	vbatt_max = hisi_battery_vbat_max();
	if (vbatt_mv >= MIN(CHG_VBATT_SOFT_OVP_MAX, CHG_VBATT_CV_103(vbatt_max))) {
		g_batt_ovp_cnt_30s++;
		if (g_batt_ovp_cnt_30s == CHG_VBATT_SOFT_OVP_CNT) {
			(void)hi6523_set_charger_hiz(TRUE);
			scharger_err("%s:vbat:%d, cv_mv:%d, ovp_cnt:%d, shutdown buck\n",
				     __func__, vbatt_mv, di->term_vol_mv,
				     g_batt_ovp_cnt_30s);
			g_batt_ovp_cnt_30s = 0;
		}
	} else {
		g_batt_ovp_cnt_30s = 0;
	}
	return 0;
}

/*
 * limit buck current to 470mA according to rboost count
 * Return: 0: do nothing; 1:limit buck current 470ma
 */
static int hi6523_rboost_buck_limit(void)
{
	if (g_rboost_cnt > ILIMIT_RBOOST_CNT) {
		scharger_inf("%s:rboost cnt:%d\n", __func__, g_rboost_cnt);
		set_boot_weaksource_flag();
		return 1;
	}
	g_rboost_cnt = 0;

	return 0;
}

static int hi6523_chip_init(struct chip_init_crit *init_crit)
{
#define RET_SIZE 16
	int ret[RET_SIZE] = {0};
	int i;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL || init_crit == NULL) {
		scharger_err("%s hi6523_device_info or init_crit is NULL!\n",
			     __func__);
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret[0] = hi6523_config_opt_param(VBUS_VSET_5V);
		ret[1] = hi6523_set_vbus_vset(VBUS_VSET_5V);
		charger_is_fcp = FCP_FALSE;
		first_insert_flag = FIRST_INSERT_TRUE;
		break;
	case ADAPTER_9V:
		ret[0] = hi6523_config_opt_param(VBUS_VSET_9V);
		ret[1] = hi6523_set_vclamp(di->param_dts.vclamp);
		break;
	case ADAPTER_12V:
		ret[0] = hi6523_config_opt_param(VBUS_VSET_12V);
		ret[1] = hi6523_set_vclamp(di->param_dts.vclamp);
		break;
	default:
		scharger_err("%s: init mode err\n", __func__);
		return -EINVAL;
	}
	/* ret[]: return value of initial function is atored in this array */
	ret[2] = hi6523_set_charge_enable(CHG_DISABLE);
	ret[3] = hi6523_set_fast_safe_timer(CHG_FASTCHG_TIMER_20H);
	ret[4] = hi6523_set_term_enable(CHG_TERM_DIS);
	ret[5] = hi6523_set_input_current(CHG_ILIMIT_470);
	ret[6] = hi6523_set_charge_current(CHG_FAST_ICHG_500MA);
	ret[7] = hi6523_set_terminal_voltage(CHG_FAST_VCHG_4400);
	ret[8] = hi6523_set_terminal_current(CHG_TERM_ICHG_150MA);
	ret[9] = hi6523_set_watchdog_timer(WATCHDOG_TIMER_40_S);
	ret[10] = hi6523_set_precharge_current(CHG_PRG_ICHG_200MA);
	ret[11] = hi6523_set_precharge_voltage(CHG_PRG_VCHG_2800);
	ret[12] = hi6523_set_batfet_ctrl(CHG_BATFET_EN);
	ret[13] = hi6523_set_bat_comp(di->param_dts.bat_comp);
	ret[14] = hi6523_set_otg_current(BOOST_LIM_1000);
	ret[15] = hi6523_set_otg_enable(OTG_DISABLE);

	for (i = 0; i < RET_SIZE; i++) {
		if (ret[i])
			return -1;
	}
	return 0;
}

static int hi6523_set_uvp_ovp(void)
{
	int ret;

	ret = hi6523_write_mask(CHG_UVP_OVP_VOLTAGE_REG,
		CHG_UVP_OVP_VOLTAGE_MSK, CHG_UVP_OVP_VOLTAGE_SHIFT,
		CHG_UVP_OVP_VOLTAGE_MAX);
	if (ret) {
		scharger_err("%s:uvp&ovp vol set err,ret=%d\n", __func__, ret);
		return -1;
	}
	return ret;
}

static int hi6523_stop_charge_config(void)
{
	int ret;

	ret = hi6523_set_vbus_vset(VBUS_VSET_5V);

	is_weaksource = WEAKSOURCE_FALSE;

	return ret;
}

struct charge_device_ops hi6523_ops = {
	.chip_init = hi6523_chip_init,
	.dev_check = hi6523_device_check,
	.set_input_current = hi6523_set_input_current,
	.set_charge_current = hi6523_set_charge_current,
	.set_terminal_voltage = hi6523_set_terminal_voltage,
	.set_dpm_voltage = hi6523_set_dpm_voltage,
	.set_terminal_current = hi6523_set_terminal_current,
	.set_charge_enable = hi6523_set_charge_enable,
	.set_otg_enable = hi6523_set_otg_enable,
	.set_term_enable = hi6523_set_term_enable,
	.get_charge_state = hi6523_get_charge_state,
	.reset_watchdog_timer = hi6523_reset_watchdog_timer,
	.dump_register = hi6523_dump_register,
	.get_register_head = hi6523_get_register_head,
	.set_watchdog_timer = hi6523_set_watchdog_timer,
	.set_batfet_disable = hi6523_set_batfet_disable,
	.get_ibus = hi6523_get_ibus_ma,
	.get_vbus = hi6523_get_vbus_mv,
	.get_vbat_sys = hi6523_get_vbat_sys,
	.set_charger_hiz = hi6523_set_charger_hiz,
	.check_input_dpm_state = hi6523_check_input_dpm_state,
	.check_input_vdpm_state = hi6523_check_input_dpm_state,
	.check_input_idpm_state = hi6523_check_input_acl_state,
	.set_otg_current = hi6523_set_otg_current,
	.stop_charge_config = hi6523_stop_charge_config,
	.set_vbus_vset = hi6523_set_vbus_vset,
	.set_uvp_ovp = hi6523_set_uvp_ovp,
	.set_force_term_enable = hi6523_force_set_term_enable,
	.get_charger_state = hi6523_get_charger_state,
	.soft_vbatt_ovp_protect = hi6523_soft_vbatt_ovp_protect,
	.rboost_buck_limit = hi6523_rboost_buck_limit,
	.get_charge_current = hi6523_get_charge_current,
	.get_iin_set = hi6523_get_input_current_set,
};

struct charger_otg_device_ops hi6523_otg_ops = {
	.chip_name = "scharger_v300",
	.otg_set_charger_enable = hi6523_set_charge_enable,
	.otg_set_enable = hi6523_set_otg_enable,
	.otg_set_current = hi6523_set_otg_current,
	.otg_set_watchdog_timer = hi6523_set_watchdog_timer,
	.otg_reset_watchdog_timer = hi6523_reset_watchdog_timer,
};

static struct water_detect_ops hi6523_water_detect_ops = {
	.type_name = "usb_dp_dn",
	.is_water_intruded = hi6523_is_water_intrused,
};

/* schedule or cancel check work based on charger type
 * USB/NON_STD/BC_USB: schedule work, REMOVED: cancel work
 */
static void hi6523_plugout_check_process(enum hisi_charger_type type)
{
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL)
		return;
	switch (type) {
	case CHARGER_TYPE_SDP:
	case CHARGER_TYPE_DCP:
	case CHARGER_TYPE_CDP:
	case CHARGER_TYPE_UNKNOWN:
		queue_delayed_work(system_power_efficient_wq,
			&di->plugout_check_work, msecs_to_jiffies(0));
		break;
	case CHARGER_TYPE_NONE:
		cancel_delayed_work_sync(&di->plugout_check_work);
		g_vbat_check_cycle = 0;
		g_batt_ovp_cnt_1s = 0;
		g_batt_ovp_cnt_30s = 0;
		g_rboost_cnt = 0;
		mutex_lock(&di->ibias_calc_lock);
		i_bias_all = 0;
		mutex_unlock(&di->ibias_calc_lock);
		clr_boot_weaksource_flag();
		break;
	default:
		break;
	}
}

/* respond the charger_type events from USB PHY */
static int hi6523_usb_notifier_call(struct notifier_block *usb_nb,
				    unsigned long event, void *data)
{
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL) {
		scharger_err("%s:di is NULL!\n", __func__);
		return NOTIFY_OK;
	}

	di->charger_type = (enum hisi_charger_type)event;
	hi6523_plugout_check_process(di->charger_type);
	return NOTIFY_OK;
}

static void ibus_vbus_operate(int ibus, int vbus, int vbatt_mv)
{
	if (hi6523_vbus_in_dpm(vbus) == TRUE) {
		ibus = hi6523_get_ibus_ma();
		if ((ibus < plugout_check_ibus_ma) &&
			hi6523_is_charger_hiz() == FALSE) {
			if (g_rboost_cnt < MAX_RBOOST_CNT) {
				(void)hi6523_set_charger_hiz(TRUE);
				/* 50: need 50mS after set TRUE before FALSE */
				msleep(50);
				(void)hi6523_set_charger_hiz(FALSE);
				g_rboost_cnt++;
				scharger_inf("%s:ibus=%dma, vbus=%dmv, vbat=%dmv, r-boost cnt:%d\n",
					     __func__, ibus, vbus,
					     vbatt_mv, g_rboost_cnt);
			} else {
				scharger_err("%s:ibus=%dma, vbus=%dmv, vbat=%dmv, r-boost cnt:%d, shutdown buck\n",
					     __func__, ibus, vbus,
					     vbatt_mv, g_rboost_cnt);
				(void)hi6523_set_charger_hiz(TRUE);
				set_boot_weaksource_flag();
			}
		}
	}
}

/* handler for chargerIC plugout detect failure check */
static void hi6523_plugout_check_work(struct work_struct *work)
{
	int ret;
	int ibus = 0;
	int vbatt_mv, vbatt_max;
	int vbus = 0;
	unsigned char rboost_irq_reg = 0;
	struct hi6523_device_info *di = g_hi6523_dev;

	if (di == NULL)
		return;
	(void)hi6523_read_mask(CHG_IRQ2, CHG_RBOOST_IRQ_MSK,
			       CHG_RBOOST_IRQ_SHIFT, &rboost_irq_reg);
	if (rboost_irq_reg == CHG_RBOOST_IRQ) {
		g_rboost_cnt++;
		(void)hi6523_write_mask(CHG_IRQ2, CHG_RBOOST_IRQ_MSK,
					CHG_RBOOST_IRQ_SHIFT, CHG_RBOOST_IRQ);
	}
	vbatt_mv = hisi_battery_voltage();
	vbatt_max = hisi_battery_vbat_max();
	g_vbat_check_cycle++;
	if ((g_vbat_check_cycle % CHG_VBATT_CHK_CNT) == 0) {
		g_vbat_check_cycle = 0;
		if (vbatt_mv >=
			MIN(CHG_VBATT_SOFT_OVP_MAX, CHG_VBATT_CV_103(vbatt_max))) {
			g_batt_ovp_cnt_1s++;
			if (g_batt_ovp_cnt_1s >= CHG_VBATT_SOFT_OVP_CNT) {
				(void)hi6523_set_charger_hiz(TRUE);
				scharger_err("%s:vbat:%d, cv_mv:%d, ovp_cnt:%d, shutdown buck\n",
					__func__, vbatt_mv, di->term_vol_mv,
					g_batt_ovp_cnt_1s);
				g_batt_ovp_cnt_1s = 0;
			}
		} else {
			g_batt_ovp_cnt_1s = 0;
		}
	}
	if (!((hi6523_get_anti_reverbst_enabled() == TRUE) &&
		(vbatt_mv < CHG_DPM_VOL_4000_MV))) {
		ret = hi6523_get_vbus_mv((unsigned int *)&vbus);
		if (ret == 0)
			ibus_vbus_operate(ibus, vbus, vbatt_mv);
	}
	queue_delayed_work(system_power_efficient_wq,
			   &di->plugout_check_work,
			   msecs_to_jiffies(plugout_check_delay_ms));
}

/* mask chargerIC fault irq in irq bottom process */
static void hi6523_mask_irq(void)
{
	int ret;

	ret = hi6523_write_byte(CHG_IRQ0_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ0 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ1_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ1 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ2_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ2 err\n", __func__);
	ret = hi6523_write_byte(CHG_WDT_IRQ_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ err\n", __func__);
}

/* unmask chargerIC irq in irq bottom process */
static void hi6523_unmask_irq(void)
{
	int ret;

	ret = hi6523_write_byte(CHG_IRQ0_MASK_ADDR, CHG_IRQ0_MASK_DEFAULT);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ0 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ1_MASK_ADDR, CHG_IRQ1_MASK_DEFAULT);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ1 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ2_MASK_ADDR, CHG_IRQ2_MASK_DEFAULT);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ2 err\n", __func__);
	ret = hi6523_write_byte(CHG_WDT_IRQ_MASK_ADDR, CHG_WDT_IRQ_DEFAULT);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ err\n", __func__);
}

static void hi6523_init_irq_status(void)
{
	u8 reg0 = 0;
	u8 reg1 = 0;
	u8 reg2 = 0;
	u8 reg_wdt = 0;
	int ret;
	/* FCP_IRQ3 FCP_IRQ4 are default to be disabled */
	ret = hi6523_write_byte(CHG_FCP_ISR1_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte ISR1_MASK err\n", __func__);
	ret = hi6523_write_byte(CHG_FCP_ISR2_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte ISR2_MASK err\n", __func__);
	ret = hi6523_write_byte(CHG_FCP_IRQ5_MASK_ADDR, CHG_IRQ_MASK_ALL);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte ISR5_MASK err\n", __func__);
	/* get irq status from boot */
	ret = hi6523_read_byte(CHG_IRQ0, &reg0);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte IRQ0 err\n", __func__);
	ret = hi6523_read_byte(CHG_IRQ1, &reg1);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte IRQ1 err\n", __func__);
	ret = hi6523_read_byte(CHG_IRQ2, &reg2);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte IRQ2 err\n", __func__);
	ret = hi6523_read_byte(CHG_WDT_IRQ, &reg_wdt);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte WDT_IRQ err\n", __func__);
	scharger_inf(
		"%s:CHG_IRQ0:0x%x, CHG_IRQ1:0x%x, CHG_IRQ2:0x%x, CHG_WDT_IRQ:0x%x\n",
		__func__, reg0, reg1, reg2, reg_wdt);
	/* clear irqs on boot after we got them */
	ret = hi6523_write_byte(CHG_IRQ0, 0xff);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ0 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ1, 0xff);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ1 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ2, 0xff);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ2 err\n", __func__);
	ret = hi6523_write_byte(CHG_WDT_IRQ, 0xff);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte WDT_IRQ err\n", __func__);
}

static void hi6523_otg_irq_handle(u8 reg0, u8 *otg_scp_cnt, u8 *otg_uvp_cnt)
{
	u8 _reg_read = 0;

	if ((reg0 & CHG_OTG_OCP) == CHG_OTG_OCP)
		scharger_err("%s:CHARGE_FAULT_OTG_OCP\n", __func__);
	if ((reg0 & CHG_OTG_SCP) == CHG_OTG_SCP) {
		(*otg_scp_cnt)++;
		(void)hi6523_write_byte(CHG_IRQ0, CHG_OTG_SCP);
		msleep(600); /* 600: need 600mS for next read */
		(void)hi6523_read_byte(CHG_STATUS0_REG, &_reg_read);
		if ((_reg_read & HI6523_CHG_OTG_ON) == HI6523_CHG_OTG_ON)
			*otg_scp_cnt = 0;
		if (*otg_scp_cnt > 3) { /* 3: total trial time 3 */
			*otg_scp_cnt = 0;
			scharger_err("%s:CHARGE_FAULT_OTG_SCP\n", __func__);
			atomic_notifier_call_chain(&fault_notifier_list,
				CHARGE_FAULT_BOOST_OCP, NULL);
		}
	}
	if ((reg0 & CHG_OTG_UVP) == CHG_OTG_UVP) {
		(*otg_uvp_cnt)++;
		(void)hi6523_write_byte(CHG_IRQ0, CHG_OTG_UVP);
		msleep(600); /* 600: need 600mS for next read */
		(void)hi6523_read_byte(CHG_STATUS0_REG, &_reg_read);
		if ((_reg_read & HI6523_CHG_OTG_ON) == HI6523_CHG_OTG_ON)
			*otg_uvp_cnt = 0;
		if (*otg_uvp_cnt > 3) { /* 3: total trial time 3 */
			*otg_uvp_cnt = 0;
			scharger_err("%s:CHARGE_FAULT_OTG_UVP\n", __func__);
			atomic_notifier_call_chain(&fault_notifier_list,
				CHARGE_FAULT_BOOST_OCP, NULL);
		}
	}
}

static void hi6523_bat_irq_handle(u8 reg1)
{
	int i;
	u8 _reg_s1 = 0;
	u8 vbat_ovp_cnt = 0;

	if ((reg1 & CHG_VBAT_OVP) == CHG_VBAT_OVP) {
		for (i = 0; i < 5; i++) { /* 5: the times of reading regs */
			(void)hi6523_read_byte(CHG_IRQ_STATUS1, &_reg_s1);
			if ((_reg_s1 & CHG_VBAT_OVP) == CHG_VBAT_OVP) {
				vbat_ovp_cnt++;
				msleep(5); /* 5: need 5 mS for next read */
			} else {
				vbat_ovp_cnt = 0;
				break;
			}
		}
		(void)hi6523_write_byte(CHG_IRQ1, CHG_VBAT_OVP);
		if (vbat_ovp_cnt >= 5) { /* 5: need 5 mS for next read */
			scharger_err("%s:CHARGE_FAULT_VBAT_OVP\n", __func__);
			atomic_notifier_call_chain(&fault_notifier_list,
				CHARGE_FAULT_VBAT_OVP, NULL);
		}
	}
}

/* handler for chargerIC fault irq in charging process */
static void hi6523_irq_work(struct work_struct *work)
{
	struct hi6523_device_info *di =
		container_of(work, struct hi6523_device_info, irq_work);
	u8 reg0 = 0;
	u8 reg1 = 0;
	u8 reg2 = 0;
	u8 reg_wdt = 0;
	static u8 otg_scp_cnt, otg_uvp_cnt;
	int ret;

	hi6523_mask_irq();
	ret = hi6523_read_byte(CHG_IRQ0, &reg0);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte IRQ0 err\n", __func__);
	ret = hi6523_read_byte(CHG_IRQ1, &reg1);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte IRQ1 err\n", __func__);
	ret = hi6523_read_byte(CHG_IRQ2, &reg2);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte IRQ2 err\n", __func__);
	ret = hi6523_read_byte(CHG_WDT_IRQ, &reg_wdt);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte WDT_IRQ err\n", __func__);

	if (reg0) {
		scharger_inf("%s:CHG_IRQ0, 0x%x\n", __func__, reg0);
		hi6523_otg_irq_handle(reg0, &otg_scp_cnt, &otg_uvp_cnt);
	}

	if (reg1) {
		scharger_inf("%s:CHG_IRQ1, 0x%x\n", __func__, reg1);
		hi6523_bat_irq_handle(reg1);
	}

	if (reg2)
		scharger_inf("%s:CHG_IRQ2, 0x%x\n", __func__, reg2);

	if (reg_wdt)
		scharger_inf("%s:CHG_WDT_IRQ, 0x%x\n", __func__, reg_wdt);

	ret = hi6523_write_byte(CHG_IRQ0, reg0);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ0 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ1, reg1);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ1 err\n", __func__);
	ret = hi6523_write_byte(CHG_IRQ2, reg2);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte IRQ2 err\n", __func__);
	ret = hi6523_write_byte(CHG_WDT_IRQ, reg_wdt);
	if (ret != 0)
		scharger_err("%s hi6523_write_byte WDT_IRQ err\n", __func__);
	hi6523_unmask_irq();
	(void)enable_irq(di->irq_int);
}

/* callback function for chargerIC fault irq in charging process */
static irqreturn_t hi6523_interrupt(int irq, void *_di)
{
	struct hi6523_device_info *di = _di;

	disable_irq_nosync(di->irq_int);
	queue_work(system_power_efficient_wq, &di->irq_work);
	return IRQ_HANDLED;
}

static void parse_water_dm_strtol(int dm_ary_idx, const char *chrg_vol_string,
			long temp, struct scharger_check_voltage *water)
{
	long tmp = temp;
	const unsigned int base = 10;

	if (dm_ary_idx % WATER_VOLT_PARA) {
		if (kstrtol(chrg_vol_string, base, &tmp) == 0) {
			water->dm_vol_data[dm_ary_idx / WATER_VOLT_PARA].vol_max = tmp;
			scharger_inf("dm_vol[%d]max:%d\n",
				     dm_ary_idx / WATER_VOLT_PARA, tmp);
		}
	} else {
		if (kstrtol(chrg_vol_string, base, &tmp) == 0) {
			water->dm_vol_data[dm_ary_idx / WATER_VOLT_PARA].vol_min = tmp;
			scharger_inf("dm_vol[%d]min:%d\n",
				     dm_ary_idx / WATER_VOLT_PARA, tmp);
		}
	}
}

static void parse_water_dm_dts(const struct device_node *np,
			       struct scharger_check_voltage *water)
{
	int i, ret;
	const long temp = 0;
	const char *chrg_vol_string = NULL;

	dm_array_len = of_property_count_strings(np, "dm_water_vol");
	if ((dm_array_len <= 0) || (dm_array_len % WATER_VOLT_PARA != 0) ||
		(dm_array_len > WATER_VOLT_PARA_LEVEL * WATER_VOLT_PARA)) {
		dm_array_len = 0;
		scharger_inf("dm_water_vol is invaild\n");
	} else {
		for (i = 0; i < dm_array_len; i++) {
			ret = of_property_read_string_index(
				np, "dm_water_vol", i, &chrg_vol_string);
			if (ret) {
				scharger_err("get dm_water_vol failed\n");
				dm_array_len = 0;
				break;
			}
			parse_water_dm_strtol(i, chrg_vol_string, temp, water);
		}
	}
}

static void parse_water_dp_strtol(int dp_ary_idx, const char *chrg_vol_string,
			long temp, struct scharger_check_voltage *water)
{
	long tmp = temp;
	const unsigned int base = 10;

	if (dp_ary_idx % WATER_VOLT_PARA) {
		if (kstrtol(chrg_vol_string, base, &tmp) == 0) {
			water->dp_vol_data[dp_ary_idx / WATER_VOLT_PARA].vol_max = tmp;
			scharger_inf("dp_vol[%d]max = %d\n",
				     dp_ary_idx / WATER_VOLT_PARA, tmp);
		}
	} else {
		if (kstrtol(chrg_vol_string, base, &tmp) == 0) {
			water->dp_vol_data[dp_ary_idx / WATER_VOLT_PARA].vol_min = tmp;
			scharger_inf("dp_vol[%d]min = %d\n",
				     dp_ary_idx / WATER_VOLT_PARA, tmp);
		}
	}
}

static void parse_water_dp_dts(const struct device_node *np,
			       struct scharger_check_voltage *water)
{
	int i, ret;
	const long temp = 0;
	const char *chrg_vol_string = NULL;

	dp_array_len = of_property_count_strings(np, "dp_water_vol");
	if ((dp_array_len <= 0) || (dp_array_len % WATER_VOLT_PARA != 0) ||
		(dp_array_len > WATER_VOLT_PARA_LEVEL * WATER_VOLT_PARA)) {
		dp_array_len = 0;
		scharger_inf("dp_water_vol is invaild\n");
	} else {
		for (i = 0; i < dp_array_len; i++) {
			ret = of_property_read_string_index(
				np, "dp_water_vol", i, &chrg_vol_string);
			if (ret) {
				scharger_err("get dp_arrary_len failed\n");
				dp_array_len = 0;
				break;
			}
			parse_water_dp_strtol(i, chrg_vol_string, temp, water);
		}
	}
}

static void parse_water_dts(const struct device_node *np,
			    struct scharger_check_voltage *water)
{
	if (water == NULL)
		return;
	parse_water_dm_dts(np, water);
	parse_water_dp_dts(np, water);
}

static void parse_dts(const struct device_node *np,
		      struct hi6523_device_info *di)
{
	int ret;
	struct device_node *batt_node = NULL;

	if (np == NULL) {
		scharger_err("%s device_node is NULL!\n", __func__);
		return;
	}
	/* 80,224: default value for bat_comp & vclamp */
	di->param_dts.bat_comp = 80;
	di->param_dts.vclamp = 224;
	ret = of_property_read_u32(
		np, "g_single_phase_buck", (u32 *)&g_single_phase_buck);
	if (ret) {
		scharger_inf("get g_single_phase_buck failed\n");
		g_single_phase_buck = 0;
	}
	scharger_inf("prase_dts g_single_phase_buck = %u\n", g_single_phase_buck);
	/* parse Vdpm_en */
	ret = of_property_read_u32(
		np, "dpm_en", (u32 *)&(di->param_dts.dpm_en));
	if (ret) {
		scharger_inf("dpm_en config not found,No need do dpm\n");
		di->param_dts.dpm_en = DPM_DISABLE;
	}
	scharger_inf("prase_dts dpm_en = %d\n", di->param_dts.dpm_en);

	/* scharger water check */
	parse_water_dts(np, &di->param_dts.scharger_check_vol);

	ret = of_property_read_u32(
		np, "bat_comp", (u32 *)&(di->param_dts.bat_comp));
	if (ret) {
		scharger_err("get bat_comp failed\n");
		return;
	}
	scharger_inf("prase_dts bat_comp = %d\n", di->param_dts.bat_comp);
	ret = of_property_read_u32(
		np, "vclamp", (u32 *)&(di->param_dts.vclamp));
	if (ret) {
		scharger_err("get vclamp failed\n");
		return;
	}
	scharger_inf("prase_dts vclamp = %d\n", di->param_dts.vclamp);
	ret = of_property_read_u32(
		np, "adc_channel_vbat_sys", (u32 *)&adc_channel_vbat_sys);
	if (ret) {
		scharger_inf("get adc_channel_vbat_sys failed!\n");
		adc_channel_vbat_sys = -1;
	}
	ret = of_property_read_u32(
		np, "fcp_support", (u32 *)&(di->param_dts.fcp_support));
	if (ret) {
		scharger_err("get fcp_support failed\n");
		return;
	}
	scharger_inf("prase_dts fcp_support = %d\n", di->param_dts.fcp_support);

	ret = of_property_read_u32(
		np, "scp_support", (u32 *)&(di->param_dts.scp_support));
	if (ret) {
		scharger_err("get scp_support failed\n");
		return;
	}
	scharger_inf("prase_dts scp_support = %d\n", di->param_dts.scp_support);
	batt_node =
		of_find_compatible_node(NULL, NULL, "huawei,hisi_bci_battery");
	if (batt_node != NULL) {
		if (of_property_read_u32(
			batt_node, "battery_board_type", &is_board_type)) {
			scharger_err("get battery_board_type fail!\n");
			is_board_type = BAT_BOARD_ASIC;
		}
	} else {
		scharger_err("get hisi_bci_battery fail!\n");
		is_board_type = BAT_BOARD_ASIC;
	}
	ret = of_property_read_u32(np, "switch_id_change", &switch_id_flag);
	if (ret)
		scharger_err("get switch id change fail!\n");
}

static void hi6523_probe_mutex_init(struct hi6523_device_info *di)
{
	int ret;

	di->term_vol_mv = hi6523_get_terminal_voltage();
	/* enable scharger apple detect */
	(void)hi6523_apple_adapter_detect(APPLE_DETECT_ENABLE);
	mutex_init(&di->fcp_detect_lock);
	mutex_init(&di->adc_conv_lock);
	mutex_init(&di->accp_adapter_reg_lock);
	mutex_init(&di->ibias_calc_lock);
	ret = water_detect_ops_register(&hi6523_water_detect_ops);
	if (ret)
		scharger_err("register water detect ops failed!\n");
}

static void hi6523_probe_version_chk(unsigned char *hi6523_version_high,
		unsigned char *hi6523_version_low, struct hi6523_device_info *di)
{
	int ret;

	ret = hi6523_read_byte(CHIP_VERSION_4, hi6523_version_high);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte VERSION_4 err!\n", __func__);
	ret = hi6523_read_byte(CHIP_VERSION_5, hi6523_version_low);
	if (ret != 0)
		scharger_err("%s hi6523_read_byte VERSION_5 err!\n", __func__);
	hi6523_version = *hi6523_version_high;
	/* 8: bit width of register */
	hi6523_version = ((hi6523_version << 8) | *hi6523_version_low);
	scharger_inf("%s:hi6523_version is 0x%x\n", __func__, hi6523_version);
	(void)hi6523_plugout_check_process(di->charger_type);

	ret = hi6523_get_vbus_gain_cal(&gain_cal);
	if (ret)
		scharger_err("[%s]get Gain fail,ret:%d\n", __func__, ret);
}

/*
 * Parameters: client:i2c_client
 *             id:i2c_device_id
 * return value: 0-success or others-fail
 */
static int hi6523_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hi6523_device_info *di = NULL;
	struct device_node *np = NULL;
	unsigned char hi6523_version_high = 0;
	unsigned char hi6523_version_low = 0;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	g_hi6523_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;

	i2c_set_clientdata(client, di);
	parse_dts(np, di);
	INIT_WORK(&di->irq_work, hi6523_irq_work);
	INIT_DELAYED_WORK(&di->plugout_check_work, hi6523_plugout_check_work);
	hi6523_init_irq_status();
	scharger_err("%s np before is %pK NULL!\n", __func__, np);
	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	scharger_err("%s np after is %pK NULL!\n", __func__, np);
	if (!gpio_is_valid(di->gpio_int)) {
		scharger_err("gpio_int is not valid\n");
		ret = -EINVAL;
		goto hi6523_gpio_init_fail;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		scharger_err("could not request gpio_int\n");
		goto hi6523_gpio_reqst_fail;
	}
	ret = gpio_direction_input(di->gpio_int);
	if (ret < 0) {
		scharger_err("Could not set gpio direction\n");
		goto hi6523_gpio_direct_map_fail;
	}
	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		scharger_err("could not map gpio_int to irq\n");
		goto hi6523_gpio_direct_map_fail;
	}
	ret = request_irq(di->irq_int, hi6523_interrupt, IRQF_TRIGGER_FALLING,
			  "charger_int_irq", di);
	if (ret) {
		scharger_err("could not request irq_int\n");
		di->irq_int = -1;
		goto hi6523_irq_init_reqst_fail;
	}

	di->charger_type = hisi_get_charger_type();
	di->usb_nb.notifier_call = hi6523_usb_notifier_call;
	ret = hisi_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0) {
		scharger_err("hisi_charger_type_notifier_register failed\n");
		goto hi6523_irq_init_reqst_fail;
	}

	hi6523_probe_mutex_init(di);
	ret = charge_ops_register(&hi6523_ops);
	if (ret) {
		scharger_err("register charge ops failed!\n");
		goto hi6523_irq_init_reqst_fail;
	}

	ret = charger_otg_ops_register(&hi6523_otg_ops);
	if (ret) {
		scharger_err("register charger_otg ops failed\n");
		goto hi6523_irq_init_reqst_fail;
	}

	hi6523_fcp_scp_ops_register();
	ret = hi6523_sysfs_create_group(di);
	if (ret) {
		scharger_err("create sysfs entries failed!\n");
		goto hi6523_create_sysfs_fail;
	}
	hi6523_probe_version_chk(&hi6523_version_high, &hi6523_version_low, di);

	scharger_inf("HI6523 probe ok!\n");
	return 0;

hi6523_create_sysfs_fail:
	hi6523_sysfs_remove_group(di);
hi6523_irq_init_reqst_fail:
	free_irq(di->irq_int, di);
hi6523_gpio_direct_map_fail:
	gpio_free(di->gpio_int);
hi6523_gpio_reqst_fail:
hi6523_gpio_init_fail:
	g_hi6523_dev = NULL;

	return ret;
}

static int hi6523_remove(struct i2c_client *client)
{
	struct hi6523_device_info *di = i2c_get_clientdata(client);

	if (di == NULL)
		return -1;

	hi6523_sysfs_remove_group(di);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	g_hi6523_dev = NULL;
	return 0;
}

MODULE_DEVICE_TABLE(i2c, HI6523);
const static struct of_device_id hi6523_of_match[] = {
	{
		.compatible = "huawei,hi6523_charger",
		.data = NULL,
	}, {

	},
};

static const struct i2c_device_id hi6523_i2c_id[] = {
	{"hi6523_charger", 0}, {

	}
};

static struct i2c_driver hi6523_driver = {
	.probe = hi6523_probe,
	.remove = hi6523_remove,
	.id_table = hi6523_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hi6523_charger",
		.of_match_table = of_match_ptr(hi6523_of_match),
	},
};

static int __init hi6523_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&hi6523_driver);
	if (ret)
		scharger_err("%s: i2c_add_driver error!!!\n", __func__);

	return ret;
}

static void __exit hi6523_exit(void)
{
	i2c_del_driver(&hi6523_driver);
}

module_init(hi6523_init);
module_exit(hi6523_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HI6523 charger driver");
MODULE_AUTHOR("Jack Lu <jack.lujianglin@hisilicon.com>");
