/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_pmic_eis.h
 *
 * eis pmic driver header
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

#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <uapi/linux/time.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include "../hisi_eis_core_freq.h"
#include "hisi_pmic_eis.h"

struct eis_pmic_device {
	int eis_support;
	int eis_time_support;
	int eis_wavelet_support;
	int eis_freq_support;
	struct device *dev;
};

/* the gear of vol gain 12 */
static unsigned int v_bat_gain[] = { 1, 2, 3, 4, 6, 8, 10 };
/* unit: mV, this table is the relative voltahe change */
static unsigned int v_bat_gain_range[][RANGE_ELEMENT_COUNT] = {
	{ 600, 1200 }, { 400, 600 }, { 300, 400 }, { 200, 300 },
	{ 150, 200 }, { 120, 150 }, { 0, 120 }
};
/* the compensatory capacitor gear corresponding to vol gain */
static unsigned int v_bat_gain_cap[] = { 7, 5, 3, 3, 2, 2, 2, 3 };
/* the compensatory resistance gear corresponding to vol gain */
static unsigned int v_bat_gain_res[] = { 6, 4, 5, 5, 4, 4, 4, 5 };
/* the gear of cur gain when external resistance is 2 mohm */
static unsigned int i_bat_gain_2_mohm[] = { 32, 64, 256, 512 };
/* unit: mA , 198 modified  to 0 */
static unsigned int i_bat_gain_range_2_mohm[][RANGE_ELEMENT_COUNT] = {
	{ 6000, 8000 }, { 2500, 6000 }, { 600, 1200 }, { 0, 600 }
};
/* the gear of cur gain when external resistance is 5 mohm */
static unsigned int i_bat_gain_5_mohm[LEN_I_BAT_GAIN_5M] = { 16, 32, 64, 256, 512 };
/* unit: mA 198 modified  to 0 */
static unsigned int i_bat_gain_range_5_mohm[][RANGE_ELEMENT_COUNT] = {
	{ 4000, 8000 }, { 2000, 4000 }, { 1000, 2000 }, { 250, 450 },
	{ 0, 250 }
};
/* the compensatory capacitor gear corresponding to cur gain */
static unsigned int i_bat_gain_cap[] = { 2, 2, 0, 2, 1 };
/* the compensatory resistance gear corresponding to cur gain */
static unsigned int i_bat_gain_res[] = { 1, 1, 1, 1, 0 };
/* unit:mS, do not change the array order */
static unsigned int _t_freq[] = {
	1, 2, 4, 5, 10, 16, 20, 50, 100, 200,
	500, 1000, 2000, 5000, 10000, 20000
};

static unsigned int i_bat_timer_k[] = { 0, 1, 2, 3, 4, 5, 6 };
static unsigned int i_bat_timer_j[] = { 1, 2, 3, 4 };
static unsigned int res_external[] = { 2, 5 }; /* unit:mOhm */
static unsigned int curt_pull_ampltd[] = { 200, 150, 100, 50 }; /* unit:mA */

/* 0xA012: config wait time for adc sampling */
static unsigned int t_ana_adc[] = { 20, 50 }; /* unit: uS */
static unsigned int adc_avg_times[] = { 4, 8, 16, 32 };
static unsigned int t_chanl2start[] = { 5, 10, 20, 30 }; /* unit: uS */
static unsigned int t_w_unit[] = { 0, 20, 100, 200 }; /* unit: mS */

static int locate_in_array(const unsigned int *array, int len, unsigned int val)
{
	int i;
	int index = -1;

	if (array == NULL || len <= 0)
		return -1;
	for (i = 0; i < len; i++) {
		if (array[i] == val) {
			index = i;
			break;
		}
	}
	return index;
}

unsigned int eis_pmic_get_periods_order_now(void)
{
	u16 reg_val_l;
	u16 reg_val_h;
	unsigned int tmp_addr_l;
	unsigned int tmp_addr_h;
	unsigned int order;

	tmp_addr_l = EIS_DET_T_NUM_L;
	tmp_addr_h = EIS_DET_T_NUM_H;

	reg_val_l = eis_reg_read(tmp_addr_l);
	/* 0, 7: get the reg value of <7:0> */
	reg_val_l = eis_reg_getbits(reg_val_l, 0, 7);
	reg_val_h = eis_reg_read(tmp_addr_h);
	/* 0, 2: get the reg value of <2:0> */
	reg_val_h = eis_reg_getbits(reg_val_h, 0, 2);
	/* the order would automatically add 1 based on the reg val in 0xA023 */
	order = (unsigned int)((reg_val_h << V_BAT_H_SHIFT) | reg_val_l);
	eis_debug("EIS_DET_T_NUM is: %u\n", order);

	return order;
}

unsigned int eis_pmic_get_v_fifo_order_now(void)
{
	u8 reg_val;
	unsigned int order;

	reg_val = eis_reg_read(EIS_VBAT_DET_NUM);
	/* 0, 3: get the reg value of <3:0> */
	reg_val = eis_reg_getbits(reg_val, 0, 3);
	/*
	 * 1: add 1 to make fifo order in the range of [1, 16], otherwise,
	 * it is in the range of [0, 15]
	 */
	order = (unsigned int)(reg_val) + 1;
	eis_debug("EIS_VBAT_DET_NUM is: %u\n", order);

	return order;
}

unsigned int eis_pmic_get_v_bat_fifo_data(unsigned int fifo_order)
{
	u8 reg_val_l;
	u16 reg_val_h;
	int tmp_addr_l;
	int tmp_addr_h;
	unsigned int v_data;

	if (fifo_order > V_BAT_FIFO_DEPTH || fifo_order == 0) {
		eis_err("fifo_order = %u,overflow!\n", fifo_order);
		return 0;
	}

	/*
	 * 2: the tmp_addr_l and tmp_addr_h are continuous with interval of 1;
	 * 1: minus 1 to convert fifo_order from [1, 16] to [0, 15]
	 */
	tmp_addr_l = EIS_VBAT_D1_L + 2 * (fifo_order - 1);
	tmp_addr_h = EIS_VBAT_D1_H + 2 * (fifo_order - 1);

	reg_val_l = eis_reg_read(tmp_addr_l);
	/* 0, 7: get the reg value of <7:0> */
	reg_val_l = eis_reg_getbits(reg_val_l, 0, 7);
	reg_val_h = eis_reg_read(tmp_addr_h);
	/* the current overflow status is in <4>, <3:0> is pure voltage data */
	reg_val_h = eis_reg_getbits(reg_val_h, 0, 4);
	v_data = ((reg_val_h << V_BAT_H_SHIFT) | reg_val_l);
	eis_debug("v_data  = %u\n", v_data);

	return v_data;
}

unsigned int eis_pmic_get_i_bat_fifo_data(unsigned int fifo_order)
{
	u8 reg_val_l;
	u16 reg_val_h;
	int tmp_addr_l;
	int tmp_addr_h;
	unsigned int i_data;

	if (fifo_order > I_BAT_FIFO_DEPTH || fifo_order == 0) {
		eis_err("fifo_order = %u,overflow!\n", fifo_order);
		return 0;
	}

	/*
	 * 2: the tmp_addr_l and tmp_addr_h are continuous with interval of 1;
	 * 1: minus 1 to convert fifo_order from [1, 4] to [0, 3]
	 */
	tmp_addr_l = EIS_IBAT_P_D4_L + 2 * (fifo_order - 1);
	tmp_addr_h = EIS_IBAT_P_D4_H + 2 * (fifo_order - 1);

	reg_val_l = eis_reg_read(tmp_addr_l);
	/* 0, 7: get the reg value of <7:0> */
	reg_val_l = eis_reg_getbits(reg_val_l, 0, 7);
	reg_val_h = eis_reg_read(tmp_addr_h);
	/* 0, 3: get the reg value of <3:0> */
	reg_val_h = eis_reg_getbits(reg_val_h, 0, 3);
	i_data = ((reg_val_h << I_BAT_H_SHIFT) | reg_val_l);
	eis_debug("i_data  = %u\n", i_data);
	return i_data;
}

void eis_pmic_set_t_n_m(unsigned int t, unsigned int n, unsigned int m)
{
	u16 reg_val, reg_tmp;
	int index;

	if (m < 1)
		return; /* m: according to nManager, m at least 1 */

	index = locate_in_array(_t_freq, LEN_T_FREQ, t);
	if (index == -1) {
		eis_err("t = %u, invalid input!\n", t);
		return;
	}

	/* set <3:0> of EIS_DET_TIMER as t */
	reg_val = eis_reg_read(EIS_DET_TIMER);
	eis_reg_setbits(reg_val, 0, 4, index);
	eis_reg_write(EIS_DET_TIMER, reg_val);

	reg_val = eis_reg_read(EIS_DET_TIMER);
	/* get <3:0> of EIS_DET_TIMER */
	reg_tmp = eis_reg_getbits(reg_val, 0, 3);
	eis_debug("EIS_DET_TIMER after set is: 0x%x\n", reg_tmp);

	/* set <3:0> of EIS_DET_TIMER_N as n */
	reg_val = eis_reg_read(EIS_DET_TIMER_N);
	eis_reg_setbits(reg_val, 0, 4, n);
	eis_reg_write(EIS_DET_TIMER_N, reg_val);

	/* get <3:0> of EIS_DET_TIMER_N */
	reg_val = eis_reg_read(EIS_DET_TIMER_N);
	reg_tmp = eis_reg_getbits(reg_val, 0, 3);
	eis_debug("EIS_DET_TIMER_N after set is: 0x%x\n", reg_tmp);

	/* set <7:0> of EIS_DET_TIMER_M_L as m lower part */
	reg_val = eis_reg_read(EIS_DET_TIMER_M_L);
	reg_val = eis_reg_getbits((m - 1), 0, 7);
	eis_reg_write(EIS_DET_TIMER_M_L, reg_val);

	/* get <7:0> of EIS_DET_TIMER_M_L */
	reg_tmp = eis_reg_read(EIS_DET_TIMER_M_L);
	reg_tmp = eis_reg_getbits(reg_tmp, 0, 7);
	/* set <1:0> of EIS_DET_TIMER_M_H as m higher part */
	reg_val = eis_reg_read(EIS_DET_TIMER_M_H);
	eis_reg_setbits(reg_val, 0, 2, eis_reg_getbits((m - 1), 8, 9));
	eis_reg_write(EIS_DET_TIMER_M_H, reg_val);

	/* get <1:0> of EIS_DET_TIMER_M_H */
	reg_val = eis_reg_read(EIS_DET_TIMER_M_H);
	reg_val = eis_reg_getbits(reg_val, 0, 1);
	reg_tmp = ((reg_val << BITS_PER_BYTE) | reg_tmp);
	eis_debug("EIS_DET_TIMER_M after set is: 0x%x\n", reg_tmp);
}

static void eis_pmic_get_t_n_m(unsigned int *t, unsigned int *n, unsigned int *m)
{
	unsigned int m_tmp;
	u8 reg_val;
	u8 reg_val_h;

	if (t == NULL || n == NULL || m == NULL) {
		eis_err("the input params are incorrect\n");
		return;
	}
	reg_val = eis_reg_read(EIS_DET_TIMER);
	/* get <3:0> of EIS_DET_TIMER */
	reg_val = eis_reg_getbits(reg_val, 0, 3);
	*t = _t_freq[reg_val];

	reg_val = eis_reg_read(EIS_DET_TIMER_N);
	/* get <3:0> of EIS_DET_TIMER_N */
	reg_val = eis_reg_getbits(reg_val, 0, 3);
	*n = reg_val;

	reg_val_h = eis_reg_read(EIS_DET_TIMER_M_H);
	reg_val_h = eis_reg_getbits(reg_val_h, 0, 1);
	reg_val = eis_reg_read(EIS_DET_TIMER_M_L);
	/* get <7:0> of EIS_DET_TIMER_L */
	reg_val = eis_reg_getbits(reg_val, 0, 7);
	m_tmp = reg_val_h;
	m_tmp = (m_tmp << V_BAT_H_SHIFT) | reg_val;
	*m = m_tmp;
}

int eis_pmic_set_curt_sample_interval(unsigned int k, unsigned int j)
{
	u8 reg_val = eis_reg_read(EIS_IBAT_DET_CTRL);
	int idx_k, idx_j;

	if (locate_in_array(i_bat_timer_k, LEN_I_BAT_TIMER_K, k) == -1
	    || locate_in_array(i_bat_timer_j, LEN_I_BAT_TIMER_J, j) == -1) {
		eis_err("input t, k or j is not eis allowed value\n");
		return _ERROR;
	}
	/*
	 * According to HI6421v900_nManager, both 0b111 and 0b110 stand for
	 * k is 6, so only the <2:1> is enough to describe state 6
	 */
	idx_k = locate_in_array(i_bat_timer_k, LEN_I_BAT_TIMER_K, k);
	idx_j = locate_in_array(i_bat_timer_j, LEN_I_BAT_TIMER_J, j);
	if (k == 6) {
		/*
	 	* 1: starting from <1>, 2: length including starting bit
	 	* 1, 2: <2:1> bit of idx_k
		* 0, 2: <2:0> bit of reg_val
	 	*/
		eis_reg_setbits(reg_val, 1, 2, eis_reg_getbits(idx_k, 1, 2));
		eis_reg_write(EIS_IBAT_DET_CTRL, reg_val);
		reg_val = eis_reg_read(EIS_IBAT_DET_CTRL);
		reg_val = eis_reg_getbits(reg_val, 0, 2);
		eis_debug("k = %u, EIS_IBAT_DET_CTRL <2:0> after set is: 0x%x\n",
			 k, reg_val);
	} else {
		/*
	 	 * 0: starting from <0>, 3: length including starting bit
		 * 0, 2: <2:0> bit of reg_val
	 	 */
		eis_reg_setbits(reg_val, 0, 3, idx_k);
		eis_reg_write(EIS_IBAT_DET_CTRL, reg_val);
		reg_val = eis_reg_read(EIS_IBAT_DET_CTRL);
		reg_val = eis_reg_getbits(reg_val, 0, 2);
		eis_debug("k = %u, EIS_IBAT_DET_CTRL <2:0> after set is: 0x%x\n",
			 k, reg_val);
	}
	/*
	 * 3: starting from <3>, 2: length including starting bit
	 * 3, 4: <4:3> bit of reg_val
	 */
	reg_val = eis_reg_read(EIS_IBAT_DET_CTRL);
	eis_reg_setbits(reg_val, 3, 2, idx_j);
	eis_reg_write(EIS_IBAT_DET_CTRL, reg_val);

	reg_val = eis_reg_read(EIS_IBAT_DET_CTRL);
	eis_debug("j = %u, EIS_IBAT_DET_CTRL  after set is: 0x%x\n", j, reg_val);
	reg_val = eis_reg_getbits(reg_val, 3, 4);
	eis_debug("j = %u, EIS_IBAT_DET_CTRL <4:3> after set is: 0x%x\n", j, reg_val);

	return _SUCC;
}

_STATIC void eis_pmic_enable_chopper_1(enum eis_chopper_enable chp_enable)
{
	u8 reg_val = eis_reg_read(EIS_ANA_CTRL0);

	if (chp_enable != EIS_CHOPPER_DIS && chp_enable != EIS_CHOPPER_EN) {
		eis_err("chp_enable = %u\n", chp_enable);
		return;
	}
	/* 5: starting from <5>, 1: length including starting bit */
	eis_reg_setbits(reg_val, 5, 1, chp_enable);
	eis_reg_write(EIS_ANA_CTRL0, reg_val);
}

_STATIC void eis_pmic_enable_chopper_2(enum eis_chopper_enable chp_enable)
{
	u8 reg_val = (u8)eis_reg_read(EIS_ANA_CTRL0);

	if (chp_enable != EIS_CHOPPER_DIS && chp_enable != EIS_CHOPPER_EN) {
		eis_err("chp_enable = %u\n", chp_enable);
		return;
	}
	/* 4: starting from <4>, 1: length including starting bit */
	eis_reg_setbits(reg_val, 4, 1, chp_enable);
	eis_reg_write(EIS_ANA_CTRL0, reg_val);
}

_STATIC void eis_pmic_set_v_i_channel_mask(
	enum eis_v_channel_mask v_mask, enum eis_i_channel_mask i_mask)
{
	u8 reg_val = (u8)eis_reg_read(EIS_ANA_CTRL0);

	if ((v_mask != VOL_NON_MASK && v_mask != VOL_MASK) ||
		(i_mask != CUR_NON_MASK && i_mask != CUR_MASK)) {
		eis_err("v_mask = %u, i_mask = %u\n", v_mask, i_mask);
		return;
	}
	/*
	 * 2: starting from <2>, 2: length including starting bit
	 * 0: starting from <0>, 2: length including starting bit
	 */
	eis_reg_setbits(reg_val, 2, 2, v_mask);
	eis_reg_setbits(reg_val, 0, 2, i_mask);
	eis_reg_write(EIS_ANA_CTRL0, reg_val);
}

_STATIC void eis_pmic_set_ana_enable_2_adc_start_interval(unsigned int ts)
{
	u8 reg_val = eis_reg_read(EIS_CTRL2);
	unsigned int t_ana;

	if (locate_in_array(t_ana_adc, LEN_T_ANA2ADC, ts) == -1) {
		eis_err("ts = %u\n", ts);
		return;
	}
	/*
	 * 6: starting from <6>, 1: length including starting bit
	 * 0, 0: <0:0> bit of n_adc
	 */
	t_ana = locate_in_array(t_ana_adc, LEN_T_ANA2ADC, ts);
	eis_reg_setbits(reg_val, 6, 1, eis_reg_getbits(t_ana, 0, 0));
	eis_reg_write(EIS_CTRL2, reg_val);

	reg_val = eis_reg_read(EIS_CTRL2);
	eis_debug("after set<6>, reg_val = %u\n", reg_val);
}

_STATIC void eis_pmic_set_adc_avg_times(unsigned int adc_time)
{
	u8 reg_val = eis_reg_read(EIS_CTRL2);
	unsigned int n_adc;

	if (locate_in_array(adc_avg_times, LEN_ADC_AVG_TIMES, adc_time) == -1) {
		eis_err("adc_time = %u\n", adc_time);
		return;
	}
	/*
	 * 4: starting from <4>, 2: length including starting bit
	 * 0, 1: <1:0> bit of n_adc
	 */
	n_adc = locate_in_array(adc_avg_times, LEN_ADC_AVG_TIMES, adc_time);
	eis_reg_setbits(reg_val, 4, 2, eis_reg_getbits(n_adc, 0, 1));
	eis_reg_write(EIS_CTRL2, reg_val);

	reg_val = eis_reg_read(EIS_CTRL2);
	eis_debug("after set<5:4>, reg_val = %u\n", reg_val);
}

_STATIC void eis_pmic_set_chanl2start_interval(unsigned int t_chnal)
{
	u8 reg_val = eis_reg_read(EIS_CTRL2);
	unsigned int t_chl;

	if (locate_in_array(t_chanl2start, LEN_T_CHNL2START, t_chnal) == -1) {
		eis_err("t_chnal = %u\n", t_chnal);
		return;
	}
	/*
	 * 2: starting from <2>, 2: length including starting bit
	 * 0, 1: <1:0> bit of t_chl
	 */
	t_chl = locate_in_array(t_chanl2start, LEN_T_CHNL2START, t_chnal);
	eis_reg_setbits(reg_val, 2, 2, eis_reg_getbits(t_chl, 0, 1));
	eis_reg_write(EIS_CTRL2, reg_val);

	reg_val = eis_reg_read(EIS_CTRL2);
	eis_debug("after set<3:2>, reg_val = %u\n", reg_val);
}

_STATIC void eis_pmic_set_wait_interval(unsigned int t_w)
{
	u8 reg_val;
	unsigned int t_wait;

	if (locate_in_array(t_w_unit, LEN_T_WAIT, t_w) == -1) {
		eis_err("t_w = %u\n", t_w);
		return;
	}
	/*
	 * 0: starting from <0>, 2: length including starting bit
	 * 0, 1: <1:0> bit of t_wait
	 */
	reg_val = eis_reg_read(EIS_CTRL2);
	t_wait = locate_in_array(t_w_unit, LEN_T_WAIT, t_w);
	eis_reg_setbits(reg_val, 0, 2, eis_reg_getbits(t_wait, 0, 1));
	eis_reg_write(EIS_CTRL2, reg_val);

	eis_debug("after set<1:0>, reg_val = %u\n", eis_reg_read(EIS_CTRL2));
}

_STATIC unsigned int eis_pmic_get_i_bat_gain(void)
{
	u8 reg_val;
	unsigned int i_gain;
	/* 0, 2: <2:0> bit of reg_val */
	reg_val = eis_reg_read(EIS_ANA_CTRL1);
	reg_val = eis_reg_getbits(reg_val, 0, 2);
	/* -1: max gain value 4, according to nManager */
	if (reg_val > LEN_I_BAT_GAIN_5M - 1)
		reg_val = LEN_I_BAT_GAIN_5M - 1;

	i_gain = i_bat_gain_5_mohm[reg_val];
	eis_inf("EIS_ANA_CTRL1 <2:0> set to: 0x%x\n", reg_val);

	return i_gain;
}

_STATIC int eis_pmic_get_r_mohm(void)
{
	u32 r_coul_mohm;
	int ret;

	ret = of_property_read_u32(of_find_compatible_node(NULL, NULL,
		"hisilicon,coul_core"), "r_coul_mohm", &r_coul_mohm);
	if (ret != 0) {
		r_coul_mohm = R_OHM_DEFAULT;
		eis_err("get r_coul_mohm err,use default value 2 mohm!\n");
	}
	eis_inf("r_coul_mohm = %u\n", r_coul_mohm);

	return (int)r_coul_mohm;
}

_STATIC int eis_pmic_get_ana_cur_offset(void)
{
	int ret;
	u8 reg_l = eis_reg_read(EISADC_INIT_I_OFFSET_L);
	u16 reg_h = eis_reg_read(EISADC_INIT_I_OFFSET_H);

	/* 0, 3: <3:0> bit of reg_val */
	reg_h = eis_reg_getbits(reg_h, 0, 3);
	ret = (int)((reg_h << BITS_PER_BYTE) | reg_l);
	eis_inf("cur_offset = %d\n", ret);

	return ret;
}

_STATIC void eis_pmic_set_is_vol_averaged(enum eis_is_v_averaged is_v_avg)
{
	u8 reg_val;

	if (is_v_avg != VOL_NON_AVGED && is_v_avg != VOL_AVGED) {
		eis_err("is_v_avg = %u\n", is_v_avg);
		return;
	}
	/* 1: starting from <1>, 1: length including starting bit */
	reg_val = eis_reg_read(EIS_ADC_AVERAGE);
	eis_reg_setbits(reg_val, 1, 1, is_v_avg);
	eis_reg_write(EIS_ADC_AVERAGE, reg_val);

	eis_debug("after set <1>, EIS_ADC_AVERAGE = %u\n",
		eis_reg_read(EIS_ADC_AVERAGE));
}

_STATIC void eis_pmic_set_is_cur_averaged(enum eis_is_i_averaged is_i_avg)
{
	u8 reg_val;

	if (is_i_avg != CUR_NON_AVGED && is_i_avg != CUR_AVGED) {
		eis_err("is_v_avg = %u\n", is_i_avg);
		return;
	}
	/* 0: starting from <0>, 1: length including starting bit */
	reg_val = eis_reg_read(EIS_ADC_AVERAGE);
	eis_reg_setbits(reg_val, 0, 1, is_i_avg);
	eis_reg_write(EIS_ADC_AVERAGE, reg_val);

	eis_debug("after set <0>, EIS_ADC_AVERAGE = %u\n",
			eis_reg_read(EIS_ADC_AVERAGE));
}

_STATIC void eis_pmic_set_com_mode_op_amp(enum com_mode_op_amp_enable en)
{
	u8 reg_val;

	if (en != OP_AMP_DIS && en != OP_AMP_EN) {
		eis_err("is_v_avg = %u\n", en);
		return;
	}
	/* 5: starting from <5>, 1: length including starting bit */
	reg_val = eis_reg_read(EIS_RESERVE0);
	eis_reg_setbits(reg_val, 5, 1, en);
	eis_reg_write(EIS_ADC_AVERAGE, reg_val);
}

_STATIC void eis_pmic_set_v_bat_gain(unsigned int v_gain)
{
	u8 reg_val = eis_reg_read(EIS_ANA_CTRL1);
	u8 reg_tmp;
	unsigned int v_gain_tmp;

	if (locate_in_array(v_bat_gain, LEN_V_BAT_GAIN, v_gain) == -1) {
		eis_err("v_gain = %u, invalid input\n", v_gain);
		return;
	}

	/*
	 * config the vol gain
	 * 3: starting from <3>, 3: length including starting bit
	 * 0, 2: <2:0> bit of reg_val
	 * 3, 5: <5:3> bit of reg_val
	 */
	v_gain_tmp = locate_in_array(v_bat_gain, LEN_V_BAT_GAIN, v_gain);
	eis_reg_setbits(reg_val, 3, 3, eis_reg_getbits(v_gain_tmp, 0, 2));
	eis_reg_write(EIS_ANA_CTRL1, reg_val);

	reg_val = eis_reg_read(EIS_ANA_CTRL1);
	reg_val = eis_reg_getbits(reg_val, 3, 5);
	eis_debug("EIS_ANA_CTRL1 <5:3> set to: 0x%x\n", reg_val);

	/*
	 * config the compensatory capacitor gear corresponding to vol gain
	 * 4: starting from <4>, 2: length including starting bit
	 * 0, 1: <1:0> bit of reg_val
	 * 7: starting from <7>, 1: length including starting bit
	 * 2, 2: <2:2> bit of reg_val
	 */
	reg_val = v_bat_gain_cap[v_gain_tmp];
	reg_tmp = eis_reg_read(EIS_ANA_CTRL2);
	eis_reg_setbits(reg_tmp, 4, 2, eis_reg_getbits(reg_val, 0, 1));
	eis_reg_write(EIS_ANA_CTRL2, reg_tmp);

	reg_tmp = eis_reg_read(EIS_RESERVE0);
	eis_reg_setbits(reg_tmp, 7, 1, eis_reg_getbits(reg_val, 2, 2));
	eis_reg_write(EIS_RESERVE0, reg_tmp);

	/*
	 * config the compensatory resistance gear corresponding to vol gain
	 * 6: starting from <6>, 2: length including starting bit
	 * 0, 1: <1:0> bit of reg_val
	 * 6: starting from <6>, 1: length including starting bit
	 * 2, 2: <2:2> bit of reg_val
	 */
	reg_val = v_bat_gain_res[v_gain_tmp];
	reg_tmp = eis_reg_read(EIS_ANA_CTRL2);
	eis_reg_setbits(reg_tmp, 6, 2, eis_reg_getbits(reg_val, 0, 1));
	eis_reg_write(EIS_ANA_CTRL2, reg_tmp);

	reg_tmp = eis_reg_read(EIS_RESERVE0);
	eis_reg_setbits(reg_tmp, 6, 1, eis_reg_getbits(reg_val, 2, 2));
	eis_reg_write(EIS_RESERVE0, reg_tmp);
}

_STATIC int eis_pmic_get_v_bat_gain(void)
{
	u8 reg_val;
	int v_gain;

	reg_val = eis_reg_read(EIS_ANA_CTRL1);
	/* 3, 5: the <5:3> bit of reg_val */
	reg_val = eis_reg_getbits(reg_val, 3, 5);
	if (reg_val > LEN_V_BAT_GAIN - 1)
		reg_val = LEN_V_BAT_GAIN - 1;

	v_gain = v_bat_gain[reg_val];
	eis_inf("EIS_ANA_CTRL1 <5:3> set to: 0x%x\n", reg_val);

	return v_gain;
}

_STATIC void eis_pmic_set_i_bat_gain(unsigned int i_gain)
{
	u8 reg_val = eis_reg_read(EIS_ANA_CTRL1);
	u8 reg_tmp;
	unsigned int i_gain_tmp;
	unsigned int *i_gain_ptr = NULL;
	unsigned int i_gain_len;
	unsigned int r_ext;

	r_ext = (unsigned int)eis_pmic_get_r_mohm();
	if (locate_in_array(res_external, LEN_RES_EXTERNAL, r_ext) == -1) {
		eis_err("r_ext = %u, invalid input\n", r_ext);
		return;
	}

	if (r_ext == R_OHM_2M) {
		i_gain_ptr = i_bat_gain_2_mohm;
		i_gain_len = LEN_I_BAT_GAIN_2M;
	} else {
		i_gain_ptr = i_bat_gain_5_mohm;
		i_gain_len = LEN_I_BAT_GAIN_5M;
	}

	if (locate_in_array(i_gain_ptr, i_gain_len, i_gain) == -1) {
		eis_err("i_gain = %d, invalid input\n", i_gain);
		return;
	}

	/*
	 * config the cur gain
	 * 0: starting from <0>, 3: length including starting bit
	 * 0, 2: <2:0> bit of i_gain_tmp
	 */
	i_gain_tmp = locate_in_array(i_bat_gain_5_mohm, LEN_I_BAT_GAIN_5M, i_gain);
	eis_reg_setbits(reg_val, 0, 3, eis_reg_getbits(i_gain_tmp, 0, 2));
	eis_reg_write(EIS_ANA_CTRL1, reg_val);

	reg_val = eis_reg_read(EIS_ANA_CTRL1);
	reg_val = eis_reg_getbits(reg_val, 0, 2);
	eis_debug("EIS_ANA_CTRL1 <2:0> set to: 0x%x\n", reg_val);

	/*
	 * config the compensatory capacitor gear corresponding to cur gain
	 * 0: starting from <0>, 2: length including starting bit
	 * 0, 1: <1:0> bit of reg_val
	 */
	reg_val = i_bat_gain_cap[i_gain_tmp];
	reg_tmp = eis_reg_read(EIS_ANA_CTRL2);
	eis_reg_setbits(reg_tmp, 0, 2, eis_reg_getbits(reg_val, 0, 1));
	eis_reg_write(EIS_ANA_CTRL2, reg_tmp);

	/*
	 * config the compensatory resistance gear corresponding to cur gain
	 * 2: starting from <2>, 2: length including starting bit
	 * 0, 1: <1:0> bit of i_gain_tmp
	 */
	reg_val = i_bat_gain_res[i_gain_tmp];
	reg_tmp = eis_reg_read(EIS_ANA_CTRL2);
	eis_reg_setbits(reg_tmp, 2, 2, eis_reg_getbits(reg_val, 0, 1));
	eis_reg_write(EIS_ANA_CTRL2, reg_tmp);
}

static void eis_pmic_set_thres_val(int low, int high, int low_err, int high_err)
{
	u8 reg_val;

	if (high_err > HIGH_ERR_MAX)
		high_err = HIGH_ERR_MAX;
	if (low_err > LOW_ERR_MAX)
		low_err = LOW_ERR_MAX;
	if (high > HIGH_MAX)
		high = HIGH_MAX;
	if (low > LOW_MAX)
		low = LOW_MAX;
	/*
	 * 0: starting from <0>, 8: length including starting bit
	 * 0, 7: <7:0> bit of high_err
	 */
	reg_val = eis_reg_read(EIS_HTHRESHOLD_ERROR);
	eis_reg_setbits(reg_val, 0, 8, eis_reg_getbits(high_err, 0, 7));
	eis_debug("high_err = %d!\n", reg_val);
	eis_reg_write(EIS_HTHRESHOLD_ERROR, reg_val);
	/*
	 * 0: starting from <0>, 8: length including starting bit
	 * 0, 7: <7:0> bit of high
	 */
	reg_val = eis_reg_read(EIS_ICOMP_HTHRESHOLD_L);
	eis_reg_setbits(reg_val, 0, 8, eis_reg_getbits(high, 0, 7));
	eis_debug("high<7:0> = %d!\n", reg_val);
	eis_reg_write(EIS_ICOMP_HTHRESHOLD_L, reg_val);
	/*
	 * 0: starting from <0>, 4: length including starting bit
	 * 8, 11: <11:8> bit of high
	 */
	reg_val = eis_reg_read(EIS_ICOMP_HTHRESHOLD_H);
	eis_reg_setbits(reg_val, 0, 4, eis_reg_getbits(high, 8, 11));
	eis_debug("high<11:8> = %d!\n", reg_val);
	eis_reg_write(EIS_ICOMP_HTHRESHOLD_H, reg_val);
	/*
	 * 0: starting from <0>, 8: length including starting bit
	 * 0, 7: <7:0> bit of low_err
	 */
	reg_val = eis_reg_read(EIS_LTHRESHOLD_ERROR);
	eis_reg_setbits(reg_val, 0, 8, eis_reg_getbits(low_err, 0, 7));
	eis_debug("low_err = %d!\n", reg_val);
	eis_reg_write(EIS_LTHRESHOLD_ERROR, reg_val);
	/*
	 * 0: starting from <0>, 8: length including starting bit
	 * 0, 7: <7:0> bit of low
	 */
	reg_val = eis_reg_read(EIS_ICOMP_LTHRESHOLD_L);
	eis_reg_setbits(reg_val, 0, 8, eis_reg_getbits(low, 0, 7));
	eis_debug("low<7:0> = %d!\n", reg_val);
	eis_reg_write(EIS_ICOMP_LTHRESHOLD_L, reg_val);
	/*
	 * 0: starting from <0>, 4: length including starting bit
	 * 8, 11: <11:8> bit of low
	 */
	reg_val = eis_reg_read(EIS_ICOMP_LTHRESHOLD_H);
	eis_reg_setbits(reg_val, 0, 4, eis_reg_getbits(low, 8, 11));
	eis_debug("low<11:8> = %d!\n", reg_val);
	eis_reg_write(EIS_ICOMP_LTHRESHOLD_H, reg_val);
}

_STATIC void get_calibrated_ana_cur(int cur_befo_ma, int *cur_after_ma)
{
	int offset_a = CALBRAT_A_DFT;
	int offset_b = 0;
	int ret;

	ret = coul_drv_coulobmeter_cur_calibration(&offset_a, &offset_b);
	if (ret != 0) {
		eis_err("coulobmeter_cur_calibration err!\n");
		return;
	}
	eis_inf("offset_a = %d, offset_b = %d!\n", offset_a, offset_b);
	if (offset_a == 0) {
		eis_inf("offset_a = %d, set to 1000000!\n", offset_a);
		offset_a = CALBRAT_A_DFT;
	}
	*cur_after_ma = (cur_befo_ma - offset_b) * CALBRAT_A_DFT / offset_a;
	eis_debug("cur_after_ma = %d!\n", *cur_after_ma);
}

_STATIC void eis_pmic_freq_set_curt_sample_thres(int cur_offset)
{
	int low, high;
	int low_err, high_err;
	int ana2digit;
	int i_gain;
	int r_mohm;

	i_gain = eis_pmic_get_i_bat_gain();
	r_mohm = eis_pmic_get_r_mohm();
	ana2digit = i_gain * r_mohm * CUR_TOTAL_STEP / A_TO_MA;

	get_calibrated_ana_cur(CUR_FREQ_THRES_H, &high);
	get_calibrated_ana_cur(CUR_FREQ_THRES_L, &low);
	eis_inf("high_new = %d, low_new = %d!\n", high, low);
	high = high * ana2digit / VOL_REF_MV + cur_offset;
	low = low  * ana2digit / VOL_REF_MV + cur_offset;
	eis_inf("high_final_digit = %d, low_final_digit = %d!\n", high, low);
	/* for r_ohm equals to 1 case, the err is increased by 2 times */
	if (r_mohm == 1) {
		get_calibrated_ana_cur(2 * CUR_FREQ_THRES_H_ERR, &high_err);
		get_calibrated_ana_cur(2 * CUR_FREQ_THRES_L_ERR, &low_err);
		eis_inf("h_err_new = %d, l_err_new = %d!\n", high_err, low_err);
	} else {
		get_calibrated_ana_cur(CUR_FREQ_THRES_H_ERR, &high_err);
		get_calibrated_ana_cur(CUR_FREQ_THRES_L_ERR, &low_err);
		eis_inf("h_err_new = %d, l_err_new = %d!\n", high_err, low_err);
	}
	high_err = high_err * ana2digit / VOL_REF_MV;
	low_err = low_err * ana2digit / VOL_REF_MV;
	eis_inf("high = %d, low = %d, hish_err = %d, low_err = %d!\n",
		high, low, high_err, low_err);
	eis_pmic_set_thres_val(low, high, low_err, high_err);
}

_STATIC void eis_pmic_freq_set_bat_dischg_mode(
	enum eis_dischg_mode dischg_mode, enum eis_dischg_enable en)
{
	u8 reg_val = eis_reg_read(EIS_CTRL1);

	if (dischg_mode != EIS_HALF_EMPTY && dischg_mode != EIS_ANY_TIME) {
		eis_err("dischg_mode = %u,invalid!\n", dischg_mode);
		return;
	}
	if (en != EIS_DISCHG_DIS && en != EIS_DISCHG_EN) {
		eis_err("en = %u,invalid!\n", en);
		return;
	}
	/*
	 * 0: starting from <0>, 1: length including starting bit
	 * 1: starting from <1>, 1: length including starting bit
	 */
	eis_reg_setbits(reg_val, 0, 1, dischg_mode);
	eis_reg_setbits(reg_val, 1, 1, en);
	eis_reg_write(EIS_CTRL1, reg_val);

	eis_debug("eis freq mode, after set <1:0>, EIS_CTRL1 = %u!\n",
		eis_reg_read(EIS_CTRL1));
}

_STATIC void eis_pmic_freq_set_curt_pull_ampltd(unsigned int curt_pull)
{
	int idx;
	u8 reg_val = eis_reg_read(EIS_ANA_CTRL1);

	if (locate_in_array(curt_pull_ampltd,
		LEN_CURT_PULL_AMPLTD, curt_pull) == -1) {
		eis_err("curt_pull, invalid input\n");
		return;
	}
	idx = locate_in_array(curt_pull_ampltd,
		LEN_CURT_PULL_AMPLTD, curt_pull);
	/*
	 * 6: starting from <6>, 2: length including starting bit
	 * 0, 1: <1:0> bit of idx
	 */
	eis_reg_setbits(reg_val, 6, 2, eis_reg_getbits(idx, 0, 1));
	eis_debug("freq mode, after set <7:6> EIS_ANA_CTRL1= %x\n", reg_val);
	eis_reg_write(EIS_ANA_CTRL1, reg_val);
	reg_val = eis_reg_read(EIS_ANA_CTRL1);
	eis_debug("freq mode, after set <7:6> EIS_ANA_CTRL1= 0x%x\n", reg_val);
}

unsigned int eis_pmic_get_curt_pull_ampltd(void)
{
	u8 reg_val;
	unsigned int curt_amp;

	reg_val = eis_reg_read(EIS_ANA_CTRL1);
	/* 6, 7: <7:6> bit of reg_val */
	reg_val = eis_reg_getbits(reg_val, 6, 7);

	curt_amp = curt_pull_ampltd[reg_val];
	eis_debug("curt_pull = %u\n", curt_amp);

	return curt_amp;
}

_STATIC int eis_pmic_set_irq_mask(
	unsigned int mask_num, enum eis_irq_mask_ops irq_mask_ops)
{
	u8 reg_val;

	/* 5: only 0~5 are valid irq mask number */
	if (mask_num > 5) {
		eis_err("mask number = %u\n", mask_num);
		return _ERROR;
	}

	if (irq_mask_ops != IRQ_NON_MASK && irq_mask_ops != IRQ_MASK) {
		eis_err("irq_mask_ops = %u\n", irq_mask_ops);
		return _ERROR;
	}

	reg_val = eis_reg_read(EIS_IRQ_MASK);
	eis_reg_setbits(reg_val, mask_num, 1, irq_mask_ops);
	eis_reg_write(EIS_IRQ_MASK, reg_val);
	eis_debug("reg 0x%x: <%u> is set as: %u\n",
		EIS_IRQ_MASK, mask_num, irq_mask_ops);

	return _SUCC;
}

_STATIC int eis_pmic_read_and_clear_irq(
	unsigned int irq_num, enum eis_irq_ops irq_ops)
{
	u8 reg_val;
	int bit_val;

	/* 5: only 0~5 are valid irq mask number */
	if (irq_num > 5) {
		eis_err("mask number is invalid\n");
		return _ERROR;
	}

	reg_val = eis_reg_read(EIS_IRQ);
	switch (irq_ops) {
	case _READ:
		bit_val = (int)eis_reg_getbits(reg_val, irq_num, irq_num);
		break;
	case _CLEAR:
		eis_reg_setbits(reg_val, irq_num, 1, _CLEAR);
		eis_reg_write(EIS_IRQ, reg_val);
		reg_val = eis_reg_read(EIS_IRQ);
		bit_val = (int)eis_reg_getbits(reg_val, irq_num, irq_num);
		break;
	default:
		bit_val = -1;
		eis_inf("not read or clear, 0x%x <%u> = %d\n",
			EIS_IRQ, irq_num, bit_val);
		break;
	}
	return bit_val;
}

void eis_pmic_irq_seq_clear(unsigned int start, unsigned int end)
{
	int ret;
	unsigned int i;

	if (start < EIS_IRQ_START || end > EIS_FIFO_IRQ_16 || start > end) {
		eis_err("start = %u, end = %u\n", start, end);
		return;
	}

	for (i = start; i <= end; i++) {
		ret = eis_pmic_read_and_clear_irq(i, _CLEAR);
		if (ret == -1)
			eis_err("read and clear irq ret error\n");
	}
}

int eis_pmic_irq_seq_read(int start, int end)
{
	u8 reg_val;

	if (start < EIS_IRQ_START || end > EIS_FIFO_IRQ_16 || start > end) {
		eis_err("start = %d, end = %d\n", start, end);
		return _ERROR;
	}
	reg_val = eis_reg_read(EIS_IRQ);
	return (int)eis_reg_getbits(reg_val, start, end);
}

int eis_pmic_get_eis_det_flag(void)
{
	u8 reg_val = eis_reg_read(EIS_DET_FLAG);
	/*
	 * eis_reg_getbits(x, n, m), x is the value, n: the starting bit,
	 * m: the end bit. 0, 0 means get the <0:0> bit of reg_val
	 */
	reg_val = eis_reg_getbits(reg_val, 0, 0);
	return (int)reg_val;
}

void eis_pmic_eis_enable(bool enable)
{
	u8 reg_val;

	reg_val = eis_reg_read(EIS_CTRL0);
	eis_reg_setbits(reg_val, 0, 1, enable ? 1 : 0);
	eis_reg_write(EIS_CTRL0, reg_val);

	eis_inf("eis status is:%s\n", enable ? "enabled" : "disabled");
}

/* this func is to get the <11:0> of vol data, as <12> is cur overflow flag */
static int eis_pmic_get_pure_vol_digit_data(
	const int *raw_sample_vol_data, int len_raw,
	int *pure_sample_vol_data, int len_pure)
{
	int i;
	int pure_data;

	if (raw_sample_vol_data == NULL || len_raw < 1 ||
		pure_sample_vol_data == NULL || len_pure < len_raw) {
		eis_err("invalid parameter\n");
		return _ERROR;
	}

	for (i = 0; i < len_raw; i++) {
		/*
		 * eis_reg_getbits(x, n, m), x is the value, n: the starting bit,
		 * m: the end bit. 0, 11 means get the <11:0> bit of reg_val
		 */
		pure_data = eis_reg_getbits(raw_sample_vol_data[i], 0, 11);
		pure_sample_vol_data[i] = pure_data;
	}
	return _SUCC;
}

/* this func is to get the <11:0> of vol data, then convert to analog data */
static void eis_pmic_get_pure_vol_digit2ana_data(
	const int *raw_sample_vol_data, int len_raw,
	unsigned long *pure_sample_ana_vol_data, int len_pure)
{
	int i;
	int pure_data;
	long digit2ana;
	int v_gain;

	v_gain = eis_pmic_get_v_bat_gain();
	if (v_gain <= 0) {
		eis_err("v_gain = %d, invalid\n", v_gain);
		return;
	}
	digit2ana = DIGIT2ANA_MAG * VOL_REF_MV / (CUR_TOTAL_STEP * v_gain);
	eis_debug("v_gain = %d, digit2ana = %ld\n", v_gain, digit2ana);

	for (i = 0; i < len_raw; i++) {
		/*
		 * eis_reg_getbits(x, n, m), x is the value, n: the starting bit,
		 * m: the end bit. 0, 11 means get the <11:0> bit of reg_val
		 */
		pure_data = eis_reg_getbits(raw_sample_vol_data[i], 0, 11);
		pure_sample_ana_vol_data[i] = (unsigned long)(pure_data * digit2ana);
		eis_debug("%dth pure_data = %d, pure_sample_ana_vol_data = %lu\n",
			i, pure_data, pure_sample_ana_vol_data[i]);
	}
}

/* this func is to get the <12:0> of cur data, then convert to analog data */
static void eis_pmic_get_pure_cur_digit2ana_data(
	const int *raw_sample_cur_data, int len_raw,
	unsigned long *pure_sample_ana_cur_data, int len_pure)
{
	int i;
	int pure_data;
	long long digit2ana;
	int i_gain;
	int r_mohm;

	i_gain = eis_pmic_get_i_bat_gain();
	r_mohm = eis_pmic_get_r_mohm();
	if (i_gain <= 0 || r_mohm <= 0) {
		eis_err("i_gain = %d, r_mohm = %d, invalid\n", i_gain, r_mohm);
		return;
	}
	digit2ana = DIGIT2ANA_MAG * VOL_REF_MV / (CUR_TOTAL_STEP * i_gain * r_mohm);

	for (i = 0; i < len_raw; i++) {
		/*
		 * eis_reg_getbits(x, n, m), x is the value, n: the starting bit,
		 * m: the end bit. 0, 11 means get the <11:0> bit of reg_val
		 */
		pure_data = eis_reg_getbits(raw_sample_cur_data[i], 0, 11);
		pure_sample_ana_cur_data[i] = (unsigned long)(pure_data * digit2ana);
	}
}

_STATIC void eis_pmic_freq_start_sampling_init(void)
{
	unsigned int i;
	int ret;

	/* 6: 0x02A1 set 0~5 irq mask reg to be masked state */
	for (i = 0; i < 6; i++) {
		ret = eis_pmic_set_irq_mask(i, IRQ_MASK);
		if (ret != _SUCC)
			eis_err("pmic set irq mask error i %u", i);
	}

	/* config 0xA000 */
	eis_pmic_enable_chopper_1(EIS_CHOPPER_DIS);
	eis_pmic_enable_chopper_2(EIS_CHOPPER_DIS);
	eis_pmic_set_v_i_channel_mask(VOL_NON_MASK, CUR_NON_MASK);
	/* config 0xA006 */
	eis_pmic_set_com_mode_op_amp(OP_AMP_DIS);

	/* 0xA012, 50: 50uS, average 4 times for adc data, 30: 30uS, 0: 0mS */
	eis_pmic_set_ana_enable_2_adc_start_interval(50);
	eis_pmic_set_adc_avg_times(4);
	eis_pmic_set_chanl2start_interval(30);
	eis_pmic_set_wait_interval(0);
	/* config 0xA025, k = 1, j = 1t */
	ret = eis_pmic_set_curt_sample_interval(1, 1);
	if (ret != _SUCC)
		eis_err("pmic set curt sample interval error");

	/* config 0xA026 */
	eis_pmic_set_is_cur_averaged(CUR_AVGED);
	eis_pmic_set_is_vol_averaged(VOL_AVGED);

	/* config 0xA011, bat discharge enable */
	eis_pmic_freq_set_bat_dischg_mode(EIS_HALF_EMPTY, EIS_DISCHG_EN);
	/* 0xA001, set bat discharge current amplitude as 200mA */
	eis_pmic_freq_set_curt_pull_ampltd(EIS_F_CUR_PULL);
	eis_pmic_set_v_bat_gain(INITIAL_FREQ_V_GAIN);
	eis_pmic_set_i_bat_gain(INITIAL_FREQ_I_GAIN);
	/* 0xA02A~0xA02F, set current threshold and threshold error */
}

static int eis_pmic_count_over_thres_sample_data(
	const int *sample_vol_data, int len)
{
	int i;
	int cnt = 0;
	unsigned int cur_over_flag;

	if (sample_vol_data == NULL || len < 1) {
		eis_err("len = %d, or invalid null input ptr\n", len);
		return _ERROR;
	}

	for (i = 0; i < len; i++) {
		/*
		 * eis_reg_getbits(x, n, m), x is the value, n: the starting bit,
		 * m: the end bit. 12, 12 means get the <12:12> bit of reg_val
		 */
		cur_over_flag = eis_reg_getbits(sample_vol_data[i], 12, 12);
		if (cur_over_flag == CUR_OVERFLOW_FLAG)
			cnt += 1;
	}
	return cnt;
}

/* return -1 means locate fail, others value means sucessed */
static int eis_pmic_locate_in_2d_array(
	const unsigned int (*table)[RANGE_ELEMENT_COUNT],
	int tbl_len, long long val)
{
	int i;
	int index = -1;

	if (table == NULL || tbl_len < 1) {
		eis_err("table_len = %d, or null ptr!\n", tbl_len);
		return index;
	}
	for (i = tbl_len - 1; i >= 0; i--) {
		if (val > table[i][0] * DIGIT2ANA_MAG &&
			val <= table[i][1] * DIGIT2ANA_MAG) {
			index = i;
			break;
		}
	}
	eis_inf("index = %d!\n", index);
	return index;
}

_STATIC int eis_pmic_locate_in_vol_gain_table(
	long long pure_vol_ana_data)
{
	int index;
	int v_gain = -1;

	index = eis_pmic_locate_in_2d_array(v_bat_gain_range,
		LEN_V_BAT_GAIN, pure_vol_ana_data);
	if (index != -1)
		v_gain = v_bat_gain[index];
	eis_inf("v_gain = %d!\n", v_gain);
	return v_gain;
}

static int eis_pmic_freq_vol_gain_check_and_adjust(
	int *v_gain, int periods, int (*raw_sample_vol_data)[V_BAT_FIFO_DEPTH])
{
	int i;
	unsigned long pure_vol_ana[V_BAT_FIFO_DEPTH];
	long long diff;
	int adjusted = V_GAIN_UNADJUSTED;
	int v_gain_next, v_gain_min;
	int v_gain_anchor;

	if (raw_sample_vol_data == NULL || periods < 1 ||
		periods > FREQ_SAMP_PRDS_10 || v_gain == NULL) {
		eis_err("periods = %d, or null ptr, invalid input!\n",
			periods);
		return adjusted;
	}
	v_gain_anchor = eis_pmic_get_v_bat_gain();
	v_gain_min = v_gain_anchor;
	for (i = 0; i < periods; i++) {
		eis_pmic_get_pure_vol_digit2ana_data(
			&raw_sample_vol_data[i][0], V_BAT_FIFO_DEPTH,
			pure_vol_ana, V_BAT_FIFO_DEPTH);
		/* 15, 7: the 16th and 8th cur value */
		if (pure_vol_ana[7] == 0 || pure_vol_ana[15] == 0) {
			eis_err("abnormal sample, not adjust!\n");
			adjusted = V_GAIN_UNADJUSTED;
		} else { /* 15, 7: the final and middle one vol data */
			diff = abs(pure_vol_ana[7] - pure_vol_ana[15]);
			v_gain_next =
				eis_pmic_locate_in_vol_gain_table(diff);
			/* -1: locate fail */
			if (v_gain_next != -1 && v_gain_next != v_gain_anchor) {
				v_gain_min = (v_gain_next < v_gain_anchor) ?
					v_gain_next : v_gain_anchor;
				adjusted = V_GAIN_ADJUSTED;
			}
		}
	}
	*v_gain = v_gain_min;
	return adjusted;
}

_STATIC int eis_pmic_locate_in_cur_gain_table(
	unsigned int pure_cur_ana_data)
{
	int index;
	int i_gain = -1;
	int len_gain;
	int r_mohm = eis_pmic_get_r_mohm();
	unsigned int (*i_gain_tbl_ptr)[RANGE_ELEMENT_COUNT];
	unsigned int *i_gain_ptr = NULL;

	if (r_mohm == R_OHM_2M) {
		i_gain_tbl_ptr = i_bat_gain_range_2_mohm;
		i_gain_ptr = i_bat_gain_2_mohm;
		len_gain = LEN_I_BAT_GAIN_2M;
	} else {
		i_gain_tbl_ptr = i_bat_gain_range_5_mohm;
		i_gain_ptr = i_bat_gain_5_mohm;
		len_gain = LEN_I_BAT_GAIN_5M;
	}

	index = eis_pmic_locate_in_2d_array(i_gain_tbl_ptr,
		len_gain, pure_cur_ana_data);
	if (index != -1)
		i_gain = i_gain_ptr[index];
	eis_inf("i_gain = %d!\n", i_gain);
	return i_gain;
}

static int eis_pmic_freq_cur_gain_check_and_adjust(
	int *i_gain, int periods, int (*raw_sample_cur_data)[I_BAT_FIFO_DEPTH])
{
	int i;
	unsigned long pure_cur_ana[I_BAT_FIFO_DEPTH];
	int diff;
	int adjusted = I_GAIN_UNADJUSTED;
	int i_gain_next, i_gain_min;
	int i_gain_anchor;

	if (raw_sample_cur_data == NULL || periods < 1 ||
		periods > FREQ_SAMP_PRDS_10 || i_gain == NULL) {
		eis_err("periods = %d, or null ptr, invalid input!\n",
			periods);
		return adjusted;
	}
	i_gain_anchor = eis_pmic_get_i_bat_gain();
	i_gain_min = i_gain_anchor;
	for (i = 0; i < periods; i++) {
		eis_pmic_get_pure_cur_digit2ana_data(
			&raw_sample_cur_data[i][0], I_BAT_FIFO_DEPTH,
			pure_cur_ana, I_BAT_FIFO_DEPTH);
		/* 3, 1: the 4th and 2nd cur value */
		if (pure_cur_ana[3] == 0 || pure_cur_ana[1] == 0) {
			eis_err("abnormal sample, not adjust!\n");
			adjusted = I_GAIN_UNADJUSTED;
		} else { /* 3, 1: the final and middle one current */
			diff = abs(pure_cur_ana[3] - pure_cur_ana[1]);
			i_gain_next =
				eis_pmic_locate_in_cur_gain_table(diff);
			/* -1: locate fail */
			if (i_gain_next != -1 && i_gain_next != i_gain_anchor) {
				i_gain_min = (i_gain_next < i_gain_anchor) ?
					i_gain_next : i_gain_anchor;
				adjusted = I_GAIN_ADJUSTED;
			}
		}
	}
	*i_gain = i_gain_min;
	return adjusted;
}

static void parse_pmic_freq_dts(struct eis_pmic_device *di)
{
	int ret;
	struct device_node *np = NULL;

	np = di->dev->of_node;
	ret = of_property_read_u32(np, "pmic_eis_freq_support",
		(u32 *)&di->eis_freq_support);
	if (ret) {
		eis_err("eis freq support dts not exist!\n");
		di->eis_freq_support = 0;
	}
	eis_inf("support = %d\n", di->eis_freq_support);
}

static struct eis_freq_device_ops hisi_pmic_freq_ops = {
	.locate_in_ary = locate_in_array,
	.get_v_bat_gain = eis_pmic_get_v_bat_gain,
	.get_v_digit_data = eis_pmic_get_pure_vol_digit_data,
	.chk_adjust_freq_i_gain = eis_pmic_freq_cur_gain_check_and_adjust,
	.chk_adjust_freq_v_gain = eis_pmic_freq_vol_gain_check_and_adjust,
	.eis_sampling_init = eis_pmic_freq_start_sampling_init,
	.set_v_gain = eis_pmic_set_v_bat_gain,
	.set_i_gain = eis_pmic_set_i_bat_gain,
	.set_cur_thres = eis_pmic_freq_set_curt_sample_thres,
};

struct eis_device_ops hisi_eis_ops = {
	.get_fifo_period_now = eis_pmic_get_periods_order_now,
	.get_fifo_order_now = eis_pmic_get_v_fifo_order_now,
	.get_v_fifo_data = eis_pmic_get_v_bat_fifo_data,
	.get_i_fifo_data = eis_pmic_get_i_bat_fifo_data,
	.read_and_clear_irq = eis_pmic_read_and_clear_irq,
	.irq_seq_clear = eis_pmic_irq_seq_clear,
	.irq_seq_read = eis_pmic_irq_seq_read,
	.eis_detect_flag = eis_pmic_get_eis_det_flag,
	.set_t_n_m = eis_pmic_set_t_n_m,
	.get_t_n_m = eis_pmic_get_t_n_m,
	.eis_enable = eis_pmic_eis_enable,
	.cnt_overflow_num = eis_pmic_count_over_thres_sample_data,
	.cur_offset = eis_pmic_get_ana_cur_offset,
};

static int eis_pmic_freq_chip_init(struct eis_pmic_device *di)
{
	int ret;

	parse_pmic_freq_dts(di);

	ret = eis_core_drv_ops_register(&hisi_pmic_freq_ops, FREQ_DRV_OPS);
	if (ret != _SUCC) {
		eis_err("freq init fail!\n");
		return _ERROR;
	}

	eis_inf("freq init success!\n");
	return _SUCC;
}

static int hisi_eis_core_chip_init(struct eis_pmic_device *di)
{
	int ret;

	ret = eis_core_drv_ops_register(&hisi_eis_ops, EIS_OPS);
	if (ret != _SUCC) {
		eis_err("eis core init fail!\n");
		return _ERROR;
	}

	eis_inf("eis core init success!\n");
	return _SUCC;
}

static int eis_pmic_probe(struct spmi_device *pdev)
{
	int ret;
	struct eis_pmic_device *di = NULL;

	di = (struct eis_pmic_device *)devm_kzalloc(&pdev->dev, sizeof(*di),
		GFP_KERNEL);
	if (di == NULL) {
		eis_err("failed to alloc *di!\n");
		return -ENOMEM;
	}
	di->dev = &pdev->dev;
	spmi_set_devicedata(pdev, di);
	ret = eis_pmic_freq_chip_init(di);
	if (ret == _ERROR)
		goto eis_probe_err;
	ret = hisi_eis_core_chip_init(di);
	if (ret == _ERROR)
		goto eis_probe_err;
	eis_inf("success!\n");
	return 0;

eis_probe_err:
	spmi_set_devicedata(pdev, NULL);
	(void)eis_core_drv_ops_register(NULL, FREQ_DRV_OPS);
	(void)eis_core_drv_ops_register(NULL, EIS_OPS);
	eis_err("fail!\n");
	return -ENODEV;
}

static int eis_pmic_remove(struct spmi_device *pdev)
{
	struct eis_pmic_device *di = spmi_get_devicedata(pdev);

	if (di == NULL) {
		eis_err("di is null!\n");
		return -ENODEV;
	}

	spmi_set_devicedata(pdev, NULL);
	(void)eis_core_drv_ops_register(NULL, FREQ_DRV_OPS);
	(void)eis_core_drv_ops_register(NULL, EIS_OPS);
	devm_kfree(&pdev->dev, di);

	return 0;
}

static const struct of_device_id hisi_eis_pmic_match_table[] = {
	{
		.compatible = "hisilicon,pmic_eis",
	},
	{
	},
};

static const struct spmi_device_id hisi_eis_pmic_spmi_id[] = {
	{
		"hisi_pmic_eis",
		0
	},
	{
	}
};

static struct spmi_driver hisi_eis_pmic_driver = {
	.probe    = eis_pmic_probe,
	.remove   = eis_pmic_remove,
	.driver   = {
		.name           = "hisi_pmic_eis",
		.owner          = THIS_MODULE,
		.of_match_table = hisi_eis_pmic_match_table,
	},
	.id_table = hisi_eis_pmic_spmi_id,
};

int __init eis_pmic_init(void)
{
	return spmi_driver_register(&hisi_eis_pmic_driver);
}

void __exit eis_pmic_exit(void)
{
	spmi_driver_unregister(&hisi_eis_pmic_driver);
}

module_init(eis_pmic_init);
module_exit(eis_pmic_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("eis pmic driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
