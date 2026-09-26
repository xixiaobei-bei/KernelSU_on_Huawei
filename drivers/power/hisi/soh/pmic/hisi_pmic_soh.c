/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: hisi_pmic_soh.c
 *
 * pmic soh module
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

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <./../hisi_soh_core.h>
#include "hisi_pmic_soh.h"

#ifndef STATIC
#define STATIC static
#endif

/*
 * acr mul and resistivity table for acr calculate,
 * Magnified 100,000 times times
 */
static long g_m_r_table[ACR_MUL_MAX][ACR_DATA_FIFO_DEPTH] = {
	{ -9194, -22196, -22196, -9194, 9194, 22196, 22196, 9194 },
	{ -4597, -11098, -11098, -4597, 4597, 11098, 11098, 4597 },
	{ -2298, -5549, -5549, -2298, 2298, 5549, 5549, 2298 },
	{ -1149, -2774, -2774, -1149, 1149, 2774, 2774, 1149 }
};
/*
 * acr mul and range table
 *  ACR_MUL_35 -> [10moh,200moh]
 *  ACR_MUL_70 -> [10moh,100moh]
 *  ACR_MUL_140-> [10moh,50moh]
 *  ACR_MUL_280-> [10moh,25moh]
 */
static int g_m_range_table[ACR_MUL_MAX][2] = {
	{ 10, 200 },
	{ 10, 100 },
	{ 10, 50 },
	{ 10, 25 }
};

static u32 acr_mul_sel = ACR_MUL_70;
static ATOMIC_NOTIFIER_HEAD(hisi_soh_drv_atomic_notifier_list);

int hisi_soh_drv_register_atomic_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(
		&hisi_soh_drv_atomic_notifier_list, nb);
}

int hisi_soh_drv_unregister_atomic_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(
		&hisi_soh_drv_atomic_notifier_list, nb);
}

int hisi_call_drv_atomic_notifiers(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(
		&hisi_soh_drv_atomic_notifier_list, val, v);
}

static irqreturn_t hisi_soh_pmic_ovp_irq_handle(int irq, void *_di)
{
	struct soh_pmic_device *di = _di;

	hisi_pmic_soh_inf("[%s] soh ovp,irq =[%d]\n", __func__, irq);

	if (!di) {
		hisi_pmic_soh_err("[%s] di null!\n", __func__);
		return IRQ_HANDLED;
	}

	if (irq == di->soh_ovh_irq)
		hisi_call_drv_atomic_notifiers(SOH_OVH, NULL);

	if (irq == di->soh_ovh_dis_irq)
		hisi_call_drv_atomic_notifiers(SOH_OVL, NULL);

	return IRQ_HANDLED;
}

STATIC void hisi_soh_pmic_pd_leak_enable(int enable)
{
	unsigned char reg_val;

	reg_val = hisi_soh_pmic_reg_read(PD_OCV_ONOFF_REG);

	if (enable)
		reg_val = reg_val | PD_OCV_ONOFF_BITMASK;
	else
		reg_val = reg_val & (~PD_OCV_ONOFF_BITMASK);

	hisi_soh_pmic_reg_write(SOH_WRITE_PRO, SOH_WRITE_UNLOCK);
	hisi_soh_pmic_reg_write(PD_OCV_ONOFF_REG, reg_val);
	hisi_soh_pmic_reg_write(SOH_WRITE_PRO, SOH_WRITE_LOCK);
}

#ifndef CONFIG_COUL_PMIC6X21V700
STATIC unsigned int hisi_soh_pmic_pd_get_ocv_rtc(unsigned int index)
{
	unsigned int ts_sec = 0;
	unsigned int ts; /* timestamp in seconds */
	unsigned int i;
	u8 reg_val, temp;

	reg_val = hisi_soh_pmic_reg_read(
		PD_OCV_RTC_OCV_REG);
	hisi_pmic_soh_debug("[%s] 0x%x, index: %u\n",
		__func__, reg_val, index);

	for (i = 0; i < index; i++) {
		temp = (reg_val >> pd_ocv_rtc_shift(i)) & PD_OCV_RTC_OCV_MASK;

		switch (temp) {
		case RTC_MIN_0:
			ts = RTC_VAL_MIN_0;
			break;
		case RTC_MIN_10:
			ts = RTC_VAL_MIN_10;
			break;
		case RTC_MIN_30:
			ts = RTC_VAL_MIN_30;
			break;
		case RTC_MIN_60:
			ts = RTC_VAL_MIN_60;
			break;
		default:
			ts = RTC_VAL_MIN_30;
			hisi_pmic_soh_err("[%s] error! %u\n", __func__, temp);
			break;
		}
		ts_sec += ts;
	}
	hisi_pmic_soh_debug("[%s] %d\n", __func__, ts_sec);

	return ts_sec;
}
#endif

STATIC void hisi_soh_pmic_pd_leak_get_info(
	struct pd_leak_chip_info *pd_ocv, unsigned int index)
{
	struct pd_leak_chip_info *ocv = pd_ocv;
	u32 reg_val = 0;
	unsigned int reg_offset_val = 0;
	int reg;

	if (!pd_ocv)
		return;

	if (index >= PD_OCV_FIFO_LEN)
		index = 0;

	hisi_pmic_soh_debug("[%s] index: %u!\n", __func__, index);

	hisi_soh_pmic_regs_read(PD_OCV_V_OCV_BASE + index * PD_OCV_REG_NUM,
		&reg_val, PD_OCV_REG_NUM);

#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_regs_read(PD_OCV_OFTV_OCV_BASE + index * PD_OCV_REG_NUM,
		&reg_offset_val, PD_OCV_REG_NUM);
#endif

	reg = ((int)reg_val << PD_OCV_SHIFT) - (reg_offset_val << PD_OCV_SHIFT);
	if (reg_val == PD_OCV_DEFAULT && reg_offset_val == PD_OCV_OFFSET_DEFAULT)
		ocv->ocv_vol_uv = 0;
	else
		ocv->ocv_vol_uv = coul_drv_convert_regval2uv(
			((u32)reg) >> (u32)PD_OCV_SHIFT);
	hisi_pmic_soh_debug("[%s] ocv: reg_val:0x%x, reg_offset_val: 0x%x, reg:0x%x, ocv_vol_uv:%d!\n",
		__func__, reg_val, reg_offset_val, reg, ocv->ocv_vol_uv);

	hisi_soh_pmic_regs_read(PD_OCV_I_OCV_BASE + index * PD_OCV_REG_NUM,
		&reg_val, PD_OCV_REG_NUM);
	ocv->ocv_cur_ua = coul_drv_convert_regval2ua(reg_val);
	hisi_pmic_soh_debug("[%s] current: reg_val:0x%x,  ocv_curr_ua:%d!\n",
		__func__, reg_val, ocv->ocv_cur_ua);

	hisi_soh_pmic_regs_read(PD_OCV_T_OCV_BASE + index * PD_OCV_TEMP_REG_NUM,
		&reg_val, PD_OCV_TEMP_REG_NUM);
	ocv->ocv_chip_temp = coul_drv_convert_regval2temp(
		reg_val << PD_OCV_TEMP_CODE_SHIFT);
	hisi_pmic_soh_debug("[%s] temp: reg_val:0x%x,  ocv_temp:%d!\n",
		__func__, reg_val, ocv->ocv_chip_temp);
#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_regs_read(PD_OCV_RTC_OCV_BASE + index * PD_OCV_RTC_REG_NUM,
		&reg_val, PD_OCV_RTC_REG_NUM);
	ocv->ocv_rtc = reg_val;
	hisi_pmic_soh_debug("[%s] rtc: reg_val:%d, ocv_rtc: %d!\n",
		__func__, reg_val, ocv->ocv_rtc);
#else
	ocv->ocv_rtc = hisi_soh_pmic_pd_get_ocv_rtc(index);
#endif
}

static unsigned int hisi_soh_pmic_pd_leak_get_fifo_depth(void)
{
	return PD_OCV_FIFO_LEN;
}

STATIC void hisi_soh_pmic_acr_enable(int enable)
{
	int reg_val = ACR_DIS;

	if (enable)
		reg_val = ACR_EN;

#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_reg_write(SOH_WRITE_PRO, SOH_WRITE_UNLOCK);
	hisi_soh_pmic_reg_write(ACR_EN_ADDR, reg_val);
	hisi_soh_pmic_reg_write(SOH_WRITE_PRO, SOH_WRITE_LOCK);

#else
	hisi_soh_pmic_reg_write(SOH_MODE_ADDR, reg_val);
	/* more than 100us clear acr flag after acr disalbeled */
	if (enable == 0)
		udelay(200); /* 200: delay 200 us */
#endif

	hisi_pmic_soh_debug("[%s]: %d!\n", __func__, enable);
}

STATIC u32 hisi_soh_pmic_acr_get_acr_flag(void)
{
	unsigned int reg_val;

	reg_val = hisi_soh_pmic_reg_read(ACR_FLAG_INT_ADDR);
	reg_val &= ACR_FLAG_INT_BIT;
	if (reg_val)
		return 1;

	return 0;
}

STATIC u32 hisi_soh_pmic_acr_get_ocp_status(void)
{
	u32 reg_val;

	reg_val = hisi_soh_pmic_reg_read(ACR_OCP_INT_ADDR);
	reg_val &= ACR_OCP_INT_BIT;
	if (reg_val)
		return 1;

	return 0;
}

STATIC void hisi_soh_pmic_acr_clear_acr_flag(void)
{
	hisi_soh_pmic_reg_write(ACR_FLAG_INT_ADDR, ACR_FLAG_INT_BIT);
}

STATIC void hisi_soh_pmic_acr_clear_ocp(void)
{
	hisi_soh_pmic_reg_write(ACR_OCP_INT_ADDR, ACR_OCP_INT_BIT);
}

STATIC void __hisi_soh_pmic_acr_set_mul(enum acr_mul mul)
{
	u32 reg_val;

	switch (mul) {
	case ACR_MUL_35:
		acr_mul_sel = ACR_MUL_35;
		break;
	case ACR_MUL_70:
		acr_mul_sel = ACR_MUL_70;
		break;
	case ACR_MUL_140:
		acr_mul_sel = ACR_MUL_140;
		break;
	case ACR_MUL_280:
		acr_mul_sel = ACR_MUL_280;
		break;
	default:
		acr_mul_sel = ACR_MUL_35;
		hisi_pmic_soh_err("[%s] mul is 0x%x,err!\n", __func__, mul);
		break;
	}
	reg_val = hisi_soh_pmic_reg_read(ACR_MUL_SEL_ADDR);
	hisi_soh_pmic_reg_write(ACR_MUL_SEL_ADDR,
		((reg_val & ACR_MUL_MASK) | (acr_mul_sel << ACR_MUL_SHIFT)));
	hisi_pmic_soh_inf("[%s]:mul = 0x%x\n", __func__,
		((reg_val & ACR_MUL_MASK) | (acr_mul_sel << ACR_MUL_SHIFT)));
}

STATIC u16 __hisi_soh_pmic_acr_get_fifo_data(int fifo_order)
{
	u16 reg_val_l;
	u16 reg_val_h;
	int tmp_addr_l;
	int tmp_addr_h;

	if (fifo_order > ACR_DATA_FIFO_DEPTH) {
		hisi_pmic_soh_err("[%s]:fifo_order = %d,overflow!\n",
			__func__, fifo_order);
		return 0;
	}

	tmp_addr_l = (int)ACR_L_DATA_BASE + ACR_DATA_REG_NUM * fifo_order;
	tmp_addr_h = (int)ACR_H_DATA_BASE + ACR_DATA_REG_NUM * fifo_order;

	reg_val_l = hisi_soh_pmic_reg_read(tmp_addr_l);
	reg_val_h = hisi_soh_pmic_reg_read(tmp_addr_h);

	return ((reg_val_h << ACR_DATA_H_SHIFT) | (reg_val_l >> ACR_DATA_L_SHIFT));
}

STATIC int hisi_soh_pmic_acr_calculate_acr(void)
{
	u16 fifo[ACR_DATA_FIFO_DEPTH];
	int i;
	long acr_value = 0;

	for (i = 0; i < ACR_DATA_FIFO_DEPTH; i++) {
		/* acr adc vol [0.1v,1.7v], acr data is greater than zero */
		fifo[i] = __hisi_soh_pmic_acr_get_fifo_data(i);
		if (fifo[i] == 0) {
			hisi_pmic_soh_err("%s: fifo[%d] =0,err!\n", __func__, i);
			return -1;
		}

		acr_value += fifo[i] * g_m_r_table[acr_mul_sel][i];
		hisi_pmic_soh_err("[%s]fifo[%d] = %u,m_r_table[%d][%d] = %ld, acr_value = %ld\n",
			__func__, i, fifo[i], acr_mul_sel, i,
			g_m_r_table[acr_mul_sel][i], acr_value);
	}
	/*
	 * if exceeding the gear position,
	 * the gear position is lowered and acr is recalculated
	 */
	if (acr_value / ACR_CAL_MAGNIFICATION >
		g_m_range_table[acr_mul_sel][1]) {
		hisi_pmic_soh_err("[%s]:acr_value = %ld,change [%u] range !\n",
			__func__, acr_value, acr_mul_sel);
		if (acr_mul_sel >= 1) {
			__hisi_soh_pmic_acr_set_mul(acr_mul_sel - 1);
			return -1;
		}
	}
	return (int)(abs(acr_value) * ACR_MOHM_TO_UOHM / ACR_CAL_MAGNIFICATION);
}

STATIC int hisi_soh_pmic_acr_get_chip_temp(void)
{
	int t_acr;
#ifdef CONFIG_COUL_PMIC6X21V700
	int retry = ACR_GET_TEMP_RETRY;
	unsigned int reg_value;
	u16 reg_val_l;
	u16 reg_val_h;
	int v_acr;

	/* start arc temp adc */
	hisi_soh_pmic_reg_write(ACR_CHIP_TEMP_CFG_ADDR, ACR_CHIP_TEMP_CFG);
	hisi_soh_pmic_reg_write(ACR_CHIP_TEMP_EN_ADDR, ACR_CHIP_TEMP_EN);
	udelay(ACR_ADC_WAIT_US);
	hisi_soh_pmic_reg_write(
		ACR_CHIP_TEMP_ADC_START_ADDR, ACR_CHIP_TEMP_ADC_START);

	do {
		if (retry == 0) {
			hisi_pmic_soh_err("[%s] retry fail!\n", __func__);
			t_acr = INVALID_TEMP;
			goto reset_temp_adc;
		}
		udelay(ACR_ADC_WAIT_US); /* chip requires 12.5us */
		reg_value = hisi_soh_pmic_reg_read(
			ACR_CHIP_TEMP_ADC_STATUS_ADDR);
		hisi_pmic_soh_inf("[%s] retry = [%d]\n",
			__func__, ACR_GET_TEMP_RETRY - retry);
		retry--;
	} while (!(reg_value & ACR_CHIP_TEMP_ADC_READY));

	reg_val_l = hisi_soh_pmic_reg_read(ACR_CHIP_TEMP_ADC_DATA0_ADDR);
	reg_val_h = hisi_soh_pmic_reg_read(ACR_CHIP_TEMP_ADC_DATA1_ADDR);
	reg_value = (int)((reg_val_h << ACR_CHIP_TEMP_H_SHIFT) |
		(reg_val_l >> ACR_CHIP_TEMP_L_SHIFT));
	/* v_acr= 1800*code /4095 (mV) */
	v_acr = (1800 * reg_value) / 4095;
	/* t_acr=358.68+1.3427*T */
	t_acr = (v_acr * 10000 - 3586800) / 13427;
	hisi_pmic_soh_debug("[%s] reg_val_l = [0x%x], reg_val_h = [0x%x], reg_value = [0x%x]\n",
		__func__, reg_val_l, reg_val_h, reg_value);
	/* close chip adc channel */
reset_temp_adc:
	hisi_soh_pmic_reg_write(
		ACR_CHIP_TEMP_CFG_ADDR, ACR_CHIP_TEMP_CFG_DEFAULT);
	hisi_soh_pmic_reg_write(ACR_CHIP_TEMP_EN_ADDR, ACR_CHIP_TEMP_EN_DEFAULT);

#else
	t_acr = coul_drv_coul_chip_temperature();
#endif
	return t_acr;
}

STATIC u32 hisi_soh_pmic_acr_get_fault_status(void)
{
#ifdef CONFIG_COUL_PMIC6X21V700
	return 0;
#else
	u8 fault;

	fault = hisi_soh_pmic_reg_read(SOH_FLAG_INT_ADDR);
	if (fault)
		hisi_pmic_soh_err("[%s] 0x%x\n", __func__, fault);

	return (unsigned int)(fault & SOH_FLAG_INT_MASK);
#endif
}

STATIC void hisi_soh_pmic_acr_clear_fault(void)
{
#ifdef CONFIG_COUL_PMIC6X21V700
#else
	hisi_soh_pmic_reg_write(SOH_FLAG_INT_ADDR, SOH_FLAG_INT_MASK);
#endif
}

STATIC void hisi_soh_pmic_acr_gpio_en(int enable)
{
}

STATIC void __hisi_soh_pmic_ovp_set_dis_current(unsigned int reg_value)
{
	unsigned int reg_val;

	reg_val = hisi_soh_pmic_reg_read(SOH_OVP_IDIS_SEL);
	hisi_soh_pmic_reg_write(SOH_OVP_IDIS_SEL,
		((reg_val & OVP_IDIS_MASK) | (reg_value & ~OVP_IDIS_MASK)));
	hisi_pmic_soh_inf("[%s]:current = 0x%x\n", __func__,
		((reg_val & OVP_IDIS_MASK) | (reg_value & ~OVP_IDIS_MASK)));
}

STATIC void hisi_soh_pmic_dcr_enable(int enable)
{
	unsigned char reg_val = DCR_DISABLE;

	if (enable) {
		reg_val = DCR_ENABLE;
		/* OVP and DCR multiplexing discharge current configuration */
		__hisi_soh_pmic_ovp_set_dis_current(OVP_IDIS_200MA);
	}

#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_reg_write(SOH_WRITE_PRO, SOH_WRITE_UNLOCK);
	hisi_soh_pmic_reg_write(DCR_EN_REG, reg_val);
	hisi_soh_pmic_reg_write(SOH_WRITE_PRO, SOH_WRITE_LOCK);

#else
	if (enable) {
		hisi_soh_pmic_reg_write(DCR_CFG_ADDR, DCR_CFG_VAL);
		hisi_soh_pmic_reg_write(DCR_COUNT_ADDR, DCR_COUNT_1);
	}
	hisi_soh_pmic_reg_write(SOH_MODE_ADDR, reg_val);
#endif
	hisi_pmic_soh_inf("[%s] 0x%x\n", __func__, enable);
}

STATIC unsigned int hisi_soh_pmic_dcr_get_fifo_depth(void)
{
	return DCR_FIFO_LEN;
}

/*
 *  DCR irq flag only as an indication of data collection,
 *  will always be masked
 *  Suggest soh core get dcr info every 32 seconds
 */
STATIC int hisi_soh_pmic_dcr_get_data2(int *dcr_current, int *dcr_vol, int num)
{
	unsigned int v_reg = 0;
	unsigned int i_reg = 0;

	if (!dcr_current || !dcr_vol) {
		hisi_pmic_soh_err("[%s] point NULL!\n", __func__);
		return -1;
	}

	hisi_soh_pmic_regs_read(
		DCR_V_BASE_REG + num * DCR_VI_REG_NUM, &v_reg, DCR_VI_REG_NUM);
	hisi_soh_pmic_regs_read(
		DCR_I_BASE_REG + num * DCR_VI_REG_NUM, &i_reg, DCR_VI_REG_NUM);
	*dcr_current = coul_drv_convert_regval2ua(i_reg) / UAH_PER_MAH;
	*dcr_vol = coul_drv_convert_regval2uv(v_reg) / UAH_PER_MAH;

	hisi_pmic_soh_debug("[%s] fifo[%d]:vol = %d, current = %d\n",
		__func__, num, *dcr_vol, *dcr_current);
	return 0;
}

STATIC int hisi_soh_pmic_dcr_get_data1(int *dcr_current, int *dcr_vol)
{
#ifdef CONFIG_COUL_PMIC6X21V700
	return -1;
#else
	unsigned int v_reg = 0;
	unsigned int i_reg = 0;

	if (!dcr_current || !dcr_vol) {
		hisi_pmic_soh_err("[%s] point NULL!\n", __func__);
		return -1;
	}

	hisi_soh_pmic_regs_read(DCR_V1_BASE_REG, &v_reg, DCR_VI_REG_NUM);
	hisi_soh_pmic_regs_read(DCR_I1_BASE_REG, &i_reg, DCR_VI_REG_NUM);
	*dcr_current = coul_drv_convert_regval2ua(i_reg) / UAH_PER_MAH;
	*dcr_vol = coul_drv_convert_regval2uv(v_reg) / UAH_PER_MAH;

	hisi_pmic_soh_debug("[%s] :vol = %d, current = %d\n",
		__func__, *dcr_vol, *dcr_current);
	return 0;
#endif
}

STATIC u32 hisi_soh_pmic_dcr_get_dcr_flag(void)
{
	unsigned int reg_val;

	reg_val = hisi_soh_pmic_reg_read(DCR_IRQ_FLAG_INT_REG);
	hisi_pmic_soh_debug("[%s] 0x%x !\n", __func__, reg_val);
	reg_val &= DCR_DONE_FLAG_MASK;
	if (reg_val)
		return 1;

	return 0;
}

STATIC void hisi_soh_pmic_dcr_clear_dcr_flag(void)
{
	hisi_soh_pmic_reg_write(DCR_IRQ_FLAG_INT_REG, DCR_FLAG_MASK);
}

STATIC void hisi_soh_pmic_dcr_set_timer(enum dcr_timer_choose dcr_timer)
{
	u8 reg_val;
#ifndef CONFIG_COUL_PMIC6X21V700
	u8 timer1_val, timer3_val;
#endif

	switch (dcr_timer) {
	case DCR_TIMER_32:
		reg_val = DCR_TIME_1;
		break;
	case DCR_TIMER_64:
		reg_val = DCR_TIME_2;
		break;
	case DCR_TIMER_128:
		reg_val = DCR_TIME_3;
		break;
	case DCR_TIMER_256:
		reg_val = DCR_TIME_4;
		break;
	default:
		reg_val = DCR_TIME_1;
		hisi_pmic_soh_err("[%s] dcr timer is 0x%x,err!\n",
			__func__, dcr_timer);
		break;
	}
#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_reg_write(DCR_TIMER_REG, (reg_val & DCR_TIMER_MASK));
	hisi_pmic_soh_inf("[%s]:timer = 0x%x\n",
		__func__, (reg_val & DCR_TIMER_MASK));
#else
	timer1_val = DCR_TIMER_250_MS;
	timer3_val = DCR_TIMER_8_MS;
	hisi_soh_pmic_reg_write(DCR_TIMER_CFG0_REG,
		((timer1_val << DCR_TIMER1_SHIFT) | reg_val));
	hisi_soh_pmic_reg_write(DCR_TIMER_CFG1_REG, timer3_val);

	hisi_pmic_soh_inf("[%s]:timer1 0x%x, timer2 0x%x, timer3 0x%x\n",
		__func__, timer1_val, reg_val, timer3_val);
#endif
}

STATIC unsigned int hisi_soh_pmic_dcr_get_timer(void)
{
	unsigned int reg_val;
	unsigned int timer_s = 0;

#ifdef CONFIG_COUL_PMIC6X21V700
	reg_val = hisi_soh_pmic_reg_read(DCR_TIMER_REG);
	switch (reg_val) {
	case DCR_TIMER_32:
		timer_s = 32; /* dcr timer */
		break;
	case DCR_TIMER_64:
		timer_s = 64; /* dcr timer */
		break;
	case DCR_TIMER_128:
		timer_s = 128; /* dcr timer */
		break;
	case DCR_TIMER_256:
		timer_s = 256; /* dcr timer */
		break;
	default:
		break;
	}
#else
	reg_val = hisi_soh_pmic_reg_read(DCR_TIMER_CFG0_REG);
	switch (reg_val & DCR_TIMER2_MASK) {
	case DCR_TIME_35_SEC:
		timer_s = 35; /* dcr timer */
		break;
	case DCR_TIME_60_SEC:
		timer_s = 60; /* dcr timer */
		break;
	case DCR_TIME_120_SEC:
		timer_s = 120; /* dcr timer */
		break;
	case DCR_TIME_300_SEC:
		timer_s = 300; /* dcr timer */
		break;
	default:
		break;
	}
#endif

	hisi_pmic_soh_inf("[%s]:timer = %u!\n", __func__, timer_s);

	return timer_s;
}

static void parse_pmic_acr_dts(struct soh_pmic_device *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di)
		return;

	np = di->dev->of_node;
	ret = of_property_read_u32(np,
		"pmic_acr_support", (u32 *)&di->pmic_acr_support);
	if (ret) {
		hisi_pmic_soh_err("[%s]:acr support dts not exist!\n", __func__);
		di->pmic_acr_support = 0;
	}
	hisi_pmic_soh_inf("[%s]:support =%d\n", __func__, di->pmic_acr_support);
}

STATIC void parse_pmic_dcr_dts(struct soh_pmic_device *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di)
		return;

	np = di->dev->of_node;
	ret = of_property_read_u32(np,
		"pmic_dcr_support", (u32 *)&di->pmic_dcr_support);
	if (ret) {
		hisi_pmic_soh_err("[%s]:dcr support dts not exist!\n", __func__);
		di->pmic_dcr_support = 0;
	}
	hisi_pmic_soh_inf("[%s]:support =%d\n", __func__, di->pmic_dcr_support);
}

STATIC void parse_pmic_pd_leak_dts(struct soh_pmic_device *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di)
		return;

	np = di->dev->of_node;
	ret = of_property_read_u32(np,
		"pmic_pd_leak_support", (u32 *)&di->pmic_pd_leak_support);
	if (ret) {
		hisi_pmic_soh_err("[%s]:pd leak support dts not exist!\n",
			__func__);
		di->pmic_pd_leak_support = 0;
	}
	hisi_pmic_soh_inf("[%s]:support =%d\n",
		__func__, di->pmic_pd_leak_support);
}

STATIC void parse_pmic_soh_ovp_dts(struct soh_pmic_device *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di)
		return;

	np = di->dev->of_node;
	ret = of_property_read_u32(np, "pmic_soh_ovp_support",
			(u32 *)&di->pmic_soh_ovp_support);
	if (ret) {
		hisi_pmic_soh_err("[%s]:soh ovp support dts not exist!\n",
			__func__);
		di->pmic_soh_ovp_support = 0;
	}
	hisi_pmic_soh_inf("[%s]:support =%d\n",
		__func__, di->pmic_soh_ovp_support);
}

static void __hisi_soh_pmic_ovp_set_vol_temp_thd(
	enum soh_thd_type thd_type, int value, int reg_addr_h, int reg_addr_l)
{
	unsigned int reg_value = 0;

	switch (thd_type) {
	case VOL_MV:
		if (value > OVP_MAX_VOL_MV || value < OVP_MIN_VOL_MV) {
			hisi_pmic_soh_err("[%s]: vol err!\n", __func__);
			return;
		}
		reg_value = coul_drv_convert_mv2regval(value);
		/* write high addr */
		hisi_soh_pmic_reg_write(reg_addr_h, ((reg_value &
			OVP_VOL_H_ADDR_MASK) >> OVP_VOL_H_ADDR_SHIFT));
		/* write low addr, it only is valid in 7-4 bit */
		hisi_soh_pmic_reg_write(reg_addr_l, ((reg_value &
			OVP_VOL_L_ADDR_MASK) >> OVP_VOL_L_ADDR_SHIFT));
		break;
	case TEMP:
		reg_value = coul_drv_convert_temp_to_adc(value);
		/* write high addr */
		hisi_soh_pmic_reg_write(reg_addr_h,
			((reg_value & OVP_TEMP_H_ADDR_MASK) >> OVP_TEMP_SHIFT));
		/* write low addr, it only is valid in 7-4 bit */
		hisi_soh_pmic_reg_write(reg_addr_l,
			((reg_value & OVP_TEMP_L_ADDR_MASK) << OVP_TEMP_SHIFT));
		break;
	default:
		hisi_pmic_soh_err("[%s]: type err!\n", __func__);
		return;
	}

	hisi_pmic_soh_debug("[%s]:thd_type = %d,value =%d, reg_value = 0x%x, h=0x%x, l=0x%x\n",
		__func__, thd_type, value, reg_value, reg_addr_h, reg_addr_l);
}

static void __hisi_soh_pmic_ovp_enable_irq(void)
{
	hisi_soh_pmic_reg_write(SOH_OVP_IRQ_MASK_REG,
		~(SOH_OVH_IRQ_MASK | SOH_OVL_IRQ_MASK));

	hisi_pmic_soh_inf("[%s]:en irq\n", __func__);
}

static void __hisi_soh_pmic_ovp_set_ov_check_time(int time)
{
#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_reg_write(SOH_TIME_EN_REG, time);
#endif
}

STATIC void __hisi_soh_pmic_ovp_mask_ovh_check(int mask)
{
#ifdef CONFIG_COUL_PMIC6X21V700

	unsigned int reg_val = 0;

	if (mask)
		reg_val |= SOH_OVH_CHECK_MASK;
	else
		reg_val &= ~SOH_OVH_CHECK_MASK;
	hisi_soh_pmic_reg_write(SOH_OVH_CHECK_MASK, reg_val);
#endif
}

static int hisi_soh_pmic_ovp_get_ovh_thred_cnt(void)
{
	return OVP_THRED_CNT;
}

static int hisi_soh_pmic_ovp_set_ovp_threshold(
	enum soh_ovp_type type, struct soh_ovp_temp_vol_threshold *ovp, int cnt)
{
	int i;
	struct soh_ovp_temp_vol_threshold *p = NULL;

	if (!ovp) {
		hisi_pmic_soh_err("[%s]:point is NULL!\n", __func__);
		return -1;
	}

	switch (type) {
	case SOH_OVH:
		for (i = 0; i < cnt; i++) {
			p = ovp + i;
			/* set vol thredshold */
			__hisi_soh_pmic_ovp_set_vol_temp_thd(VOL_MV,
				p->bat_vol_mv, (int)soh_ovh_vol_h_reg(i),
				(int)soh_ovh_vol_l_reg(i));
			/* set temp thredshold */
			__hisi_soh_pmic_ovp_set_vol_temp_thd(TEMP,
				p->temp, (int)soh_ovh_temp_h_reg(i),
				(int)soh_ovh_temp_l_reg(i));
		}
		break;
#ifdef CONFIG_COUL_PMIC6X21V700
	case SOH_OVL:
		p = ovp;
		__hisi_soh_pmic_ovp_set_vol_temp_thd(VOL_MV,
			p->bat_vol_mv, SOH_OVL_VOL_H_REG, SOH_OVL_VOL_L_REG);
		__hisi_soh_pmic_ovp_set_vol_temp_thd(TEMP,
			p->temp, SOH_OVL_TEMP_H_REG, SOH_OVL_TEMP_L_REG);
		break;
#endif
	default:
		hisi_pmic_soh_err("[%s]:default!\n", __func__);
		return -1;
	}
	hisi_pmic_soh_inf("[%s]:suc!\n", __func__);
	return 0;
}

STATIC int hisi_soh_pmic_ovp_get_dischg_stop_state(void)
{
#ifdef CONFIG_COUL_PMIC6X21V700

	int state;

	state = hisi_soh_pmic_reg_read(SOH_DISCHARGE_EN_REG);
	if (state == SOH_DISCHARGER_DIS)
		return 1;
#endif
	return 0;
}

STATIC void hisi_soh_pmic_ovp_enable_ov(int en)
{
	u8 reg_val = SOH_OVP_DIS;

	if (en)
		reg_val = SOH_OVP_EN;

#ifdef CONFIG_COUL_PMIC6X21V700
	hisi_soh_pmic_reg_write(SOH_OVP_EN_REG, reg_val);
#else
	hisi_soh_pmic_reg_write(SOH_MODE_ADDR, reg_val);
#endif
	hisi_pmic_soh_inf("[%s] 0x%x\n", __func__, reg_val);
}

STATIC void hisi_soh_pmic_ovp_enable_discharger(int en)
{
#ifdef CONFIG_COUL_PMIC6X21V700

	if (en) {
		/* OVP and DCR multiplexing discharge current configuration */
		__hisi_soh_pmic_ovp_set_dis_current(OVP_IDIS_50MA);
		hisi_soh_pmic_reg_write(
			SOH_DISCHARGE_EN_REG, SOH_DISCHARGER_EN);
	} else {
		/* first close dischg, second close ovp en,it is chip bug */
		hisi_soh_pmic_reg_write(
			SOH_DISCHARGE_EN_REG, SOH_DISCHARGER_DIS);
		hisi_soh_pmic_ovp_enable_ov(0);
	}
#endif
}

static void __hisi_soh_pmic_ovp_chip_init(void)
{
	/* disable ovp check */
	hisi_soh_pmic_ovp_enable_ov(0);

	/* set discharge time */
	__hisi_soh_pmic_ovp_set_dis_current(OVP_IDIS_50MA);

	/* set ov check time */
	__hisi_soh_pmic_ovp_set_ov_check_time(SOH_TIME_20S);

	/* unmask  ovh */
	__hisi_soh_pmic_ovp_mask_ovh_check(0);

	/* enable ovh and ovl interrupt */
	__hisi_soh_pmic_ovp_enable_irq();
}


static struct soh_dcr_device_ops hisi_pmic_dcr_ops = {
	.enable_dcr         = hisi_soh_pmic_dcr_enable,
	.get_dcr_flag       = hisi_soh_pmic_dcr_get_dcr_flag,
	.clear_dcr_flag     = hisi_soh_pmic_dcr_clear_dcr_flag,
	.get_dcr_info       = hisi_soh_pmic_dcr_get_data2,
	.get_dcr_data1       = hisi_soh_pmic_dcr_get_data1,
	.set_dcr_timer      = hisi_soh_pmic_dcr_set_timer,
	.get_dcr_timer      = hisi_soh_pmic_dcr_get_timer,
	.get_dcr_fifo_depth = hisi_soh_pmic_dcr_get_fifo_depth,
};

static struct soh_acr_device_ops hisi_pmic_acr_ops = {
	.enable_acr             = hisi_soh_pmic_acr_enable,
	.get_acr_flag           = hisi_soh_pmic_acr_get_acr_flag,
	.get_acr_ocp            = hisi_soh_pmic_acr_get_ocp_status,
	.clear_acr_flag         = hisi_soh_pmic_acr_clear_acr_flag,
	.clear_acr_ocp          = hisi_soh_pmic_acr_clear_ocp,
	.calculate_acr          = hisi_soh_pmic_acr_calculate_acr,
	.get_acr_chip_temp      = hisi_soh_pmic_acr_get_chip_temp,
	.get_acr_fault_status   = hisi_soh_pmic_acr_get_fault_status,
	.clear_acr_fault_status = hisi_soh_pmic_acr_clear_fault,
	.io_ctrl_acr_chip_en    = hisi_soh_pmic_acr_gpio_en,
};

static struct soh_pd_leak_device_ops hisi_pmic_pd_leak_ops = {
	.enable_pd_leak         = hisi_soh_pmic_pd_leak_enable,
	.get_pd_leak_fifo_depth = hisi_soh_pmic_pd_leak_get_fifo_depth,
	.get_pd_leak_info       = hisi_soh_pmic_pd_leak_get_info,
};

static struct soh_ovp_device_ops hisi_pmic_soh_ovp_ops = {
	.set_ovp_threshold      = hisi_soh_pmic_ovp_set_ovp_threshold,
	.get_ovh_thred_cnt      = hisi_soh_pmic_ovp_get_ovh_thred_cnt,
	.enable_ovp             = hisi_soh_pmic_ovp_enable_ov,
	.enable_dischg          = hisi_soh_pmic_ovp_enable_discharger,
	.get_stop_dischg_state  = hisi_soh_pmic_ovp_get_dischg_stop_state,
};

STATIC int hisi_soh_pmic_dcr_init(struct soh_pmic_device *di)
{
	int ret;

	if (!di)
		return -1;
	/* get dts config */
	parse_pmic_dcr_dts(di);

	if (!di->pmic_dcr_support)
		return 0;

	/* mask dcr flag interrupt */
	hisi_soh_pmic_reg_write(DCR_FLAG_INT_MASK_REG, DCR_FLAG_INT_MASK_BIT);

	/* register dcr ops to soh core */
	ret = soh_core_drv_ops_register(&hisi_pmic_dcr_ops, DCR_DRV_OPS);
	if (ret)
		goto dcr_init_err;

	hisi_pmic_soh_inf("[%s] soh dcr init success!\n", __func__);
	return 0;

dcr_init_err:
	hisi_pmic_soh_err("[%s]fail!\n", __func__);
	return ret;
}

STATIC int hisi_soh_pmic_pd_leak_init(struct soh_pmic_device *di)
{
	int ret;

	if (!di)
		return -1;
	/* get dts config */
	parse_pmic_pd_leak_dts(di);

	if (!di->pmic_dcr_support)
		return 0;

	/* enable pd ocv */
	hisi_soh_pmic_pd_leak_enable(1);
	/* register pd leak ops to soh core */
	ret = soh_core_drv_ops_register(&hisi_pmic_pd_leak_ops, PD_LEAK_DRV_OPS);
	if (ret)
		goto pd_init_err;
	hisi_pmic_soh_inf("[%s] pd leak init success!\n", __func__);
	return 0;

pd_init_err:
	hisi_pmic_soh_err("[%s]fail!\n", __func__);
	return ret;
}

STATIC int hisi_soh_pmic_acr_init(struct soh_pmic_device *di)
{
	int ret;

	if (!di)
		return -1;

	/* get dts config */
	parse_pmic_acr_dts(di);

	/* acr starts according dts config */
	if (!di->pmic_acr_support)
		return 0;
#ifdef CONFIG_COUL_PMIC6X21V700
	/* mask acr flag and acr ocp interrupt */
	hisi_soh_pmic_reg_write(ACR_FLAG_INT_MASK_REG, ACR_FLAG_INT_MASK_BIT);
	hisi_soh_pmic_reg_write(ACR_OCP_INT_MASK_REG, ACR_OCP_INT_MASK_BIT);
#endif
	/* choose acr mul */
	__hisi_soh_pmic_acr_set_mul(ACR_MUL_70);
	/* register acr ops to soh core */
	ret = soh_core_drv_ops_register(&hisi_pmic_acr_ops, ACR_DRV_OPS);
	if (ret)
		goto acr_init_err;

	hisi_pmic_soh_inf("[%s] soh acr init success!\n", __func__);
	return 0;

acr_init_err:
	hisi_pmic_soh_err("[%s]fail!\n", __func__);
	return ret;
}

static int hisi_soh_pmic_ovp_init(struct soh_pmic_device *di)
{
	int ret;
	struct spmi_device *pdev = NULL;

	if (!di)
		return -1;

	/* get dts config */
	parse_pmic_soh_ovp_dts(di);

	if (!di->pmic_soh_ovp_support)
		return 0;

	pdev = to_spmi_device(di->dev);
	if (!pdev) {
		hisi_pmic_soh_err("%s:pdev get failed!\n", __func__);
		ret = -1;
		goto ovp_init_err;
	}

	/* requeset ovh dis interrupt */
	di->soh_ovh_dis_irq = spmi_get_irq_byname(pdev, NULL, "soh_ovh_dis");
	if (di->soh_ovh_dis_irq < 0) {
		hisi_pmic_soh_err("[%s]failed to get soh_ovh_dis irq!\n",
			__func__);
		ret = -1;
		goto ovp_init_err;
	}
	hisi_pmic_soh_inf("[%s] soh_ovh_dis_irq = %d!\n",
		__func__, di->soh_ovh_dis_irq);
	ret = devm_request_irq(di->dev, di->soh_ovh_dis_irq,
		hisi_soh_pmic_ovp_irq_handle, IRQF_NO_SUSPEND, "soh_ovh_dis", di);
	if (ret < 0) {
		hisi_pmic_soh_err("[%s] failed to request soh ovh dis irq!\n",
			__func__);
		ret = -1;
		goto ovp_init_err;
	}

	/* requeset ovh interrupt */
	di->soh_ovh_irq = spmi_get_irq_byname(pdev, NULL, "soh_ovh");
	if (di->soh_ovh_irq < 0) {
		hisi_pmic_soh_err("[%s]failed to get soh_ovh irq!\n", __func__);
		ret = -1;
		goto ovp_init_err;
	}
	hisi_pmic_soh_inf("[%s] soh_ovh_irq = %d!\n", __func__, di->soh_ovh_irq);
	ret = devm_request_irq(di->dev, di->soh_ovh_irq,
		hisi_soh_pmic_ovp_irq_handle, IRQF_NO_SUSPEND, "soh_ovh", di);
	if (ret < 0) {
		hisi_pmic_soh_err("[%s] failed to request soh ovh irq!\n",
			__func__);
		ret = -1;
		goto ovp_init_err;
	}

	/* ovp chip init */
	__hisi_soh_pmic_ovp_chip_init();

	/* register pd leak ops to soh core */
	ret = soh_core_drv_ops_register(&hisi_pmic_soh_ovp_ops, SOH_OVP_DRV_OPS);
	if (ret)
		goto ovp_init_err;

	hisi_pmic_soh_inf("[%s]suc!\n", __func__);
	return 0;

ovp_init_err:
	hisi_pmic_soh_err("[%s]fail!\n", __func__);
	return ret;
}

#ifndef CONFIG_COUL_PMIC6X21V700
static void hisi_soh_pmic_soh_vbat_uvp_thd(int mv)
{
	u16 reg_val;

	reg_val = (((unsigned int)coul_drv_convert_mv2regval(mv)) >>
		SOH_VBAT_UVP_SHIFT);
	hisi_soh_pmic_regs_write(SOH_VBAT_UVP_BASE, &reg_val, SOH_VBAT_UVP_NUM);
}

static void hisi_soh_pmic_irq_init(struct soh_pmic_device *di)
{
	hisi_soh_pmic_reg_write(ACR_SOH_IRQ_REG, ACR_SOH_INT_MASK_BIT);
	hisi_soh_pmic_reg_write(DCR_IRQ_REG, ACR_SOH_INT_MASK_BIT);
}
#endif

static void hisi_soh_pmic_soh_init(struct soh_pmic_device *di)
{
#ifndef CONFIG_COUL_PMIC6X21V700
	/* set soh vbat uvp */
	hisi_soh_pmic_soh_vbat_uvp_thd(SOH_VBAT_UVP_THD);

	hisi_soh_pmic_irq_init(di);
#endif
}

static int hisi_soh_pmic_probe(struct spmi_device *pdev)
{
	struct soh_pmic_device *di = NULL;
	int ret;

	di = (struct soh_pmic_device *)devm_kzalloc(
		&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		hisi_pmic_soh_err("%s failed to alloc di struct\n", __func__);
		return -1;
	}
	di->dev = &pdev->dev;

	spmi_set_devicedata(pdev, di);

	hisi_soh_pmic_soh_init(di);

	ret = hisi_soh_pmic_acr_init(di);
	if (ret)
		goto soh_probe_err;
	ret = hisi_soh_pmic_dcr_init(di);
	if (ret)
		goto soh_probe_err;
	ret = hisi_soh_pmic_ovp_init(di);
	if (ret)
		goto soh_probe_err;
	ret = hisi_soh_pmic_pd_leak_init(di);
	if (ret)
		goto soh_probe_err;

	hisi_pmic_soh_inf("[%s]suc!\n", __func__);
	return 0;

soh_probe_err:
	spmi_set_devicedata(pdev, NULL);
	soh_core_drv_ops_register(NULL, ACR_DRV_OPS);
	soh_core_drv_ops_register(NULL, DCR_DRV_OPS);
	soh_core_drv_ops_register(NULL, PD_LEAK_DRV_OPS);
	soh_core_drv_ops_register(NULL, SOH_OVP_DRV_OPS);
	hisi_pmic_soh_err("[%s]fail!\n", __func__);
	return ret;
}

static int hisi_soh_pmic_remove(struct spmi_device *pdev)
{
	struct soh_pmic_device *di = spmi_get_devicedata(pdev);

	if (!di) {
		hisi_pmic_soh_err("[%s]di is null\n", __func__);
		return -1;
	}
	spmi_set_devicedata(pdev, NULL);
	soh_core_drv_ops_register(NULL, ACR_DRV_OPS);
	soh_core_drv_ops_register(NULL, DCR_DRV_OPS);
	soh_core_drv_ops_register(NULL, PD_LEAK_DRV_OPS);
	soh_core_drv_ops_register(NULL, SOH_OVP_DRV_OPS);
	devm_kfree(&pdev->dev, di);
	return 0;
}


static const struct of_device_id hisi_soh_pmic_match_table[] = {
	{
		.compatible = "hisi_soh_pmic",
	},
	{
	},
};


static const struct spmi_device_id hisi_soh_pmic_spmi_id[] = {
	{
		"hisi_soh_pmic",
		0
	},
	{
	}
};

static struct spmi_driver hisi_pmic_soh_driver = {
	.probe         = hisi_soh_pmic_probe,
	.remove        = hisi_soh_pmic_remove,
	.driver        = {
		.name           = "hisi_soh_pmic",
		.owner          = THIS_MODULE,
		.of_match_table = hisi_soh_pmic_match_table,
	},
	.id_table     = hisi_soh_pmic_spmi_id,
};

int __init hisi_soh_pmic_init(void)
{
	return spmi_driver_register(&hisi_pmic_soh_driver);
}

void __exit hisi_soh_pmic_exit(void)
{
	spmi_driver_unregister(&hisi_pmic_soh_driver);
}

module_init(hisi_soh_pmic_init);
module_exit(hisi_soh_pmic_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("batt soh module");
MODULE_LICENSE("GPL v2");
