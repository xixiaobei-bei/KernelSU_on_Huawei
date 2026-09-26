/*
 * optiga_swi.c
 *
 * optiga single wire interface operation function
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

#include <linux/io.h>
#include <linux/err.h>
#include <linux/timex.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/battery/battery_type_identify.h>

#include "../onewire/onewire_common.h"
#include "include/optiga_swi.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battct_swi
HWLOG_REGIST();

/* spinlock control */
bool g_lock_status;
unsigned long g_splk_flags;

/* gpio operator */
struct ow_gpio_des *g_gpio_phy;

/* time measurement setting */
#ifndef us2cycles
#define us2cycles(x)  (((((x) * 0x10c7UL) * loops_per_jiffy * HZ) + (1UL << 31)) >> 32)
#endif

static uint32_t g_t_logic0;
static uint32_t g_t_logic1;
static uint32_t g_t_stop;
static uint32_t g_t_resp_to;  /* response timeout */
static uint32_t g_t_resp_lto; /* response timeout long */
static uint32_t g_t_nvm_to;   /* nvm operation timeout */
static uint32_t g_t_pwr_up;
static uint32_t g_t_pwr_off;

int optiga_swi_init(struct sle95250_dev *di)
{
	if (!di) {
		hwlog_err("[%s] di NULL\n", __func__);
		return -EINVAL;
	}

	g_t_logic0 = us2cycles(di->tau);
	g_t_logic1 = 3 * g_t_logic0; /* time_len of logic1 = 3 tau */
	g_t_stop = 5 * g_t_logic0; /* time_len of stop signal = 5 tau */
	g_t_resp_to = 50 * g_t_logic0; /* time_len of resp timeout = 50 tau */
	g_t_resp_lto = us2cycles(OPTIGA_T_RESP_TIMEOUT_LONG);
	g_t_nvm_to = us2cycles(OPTIGA_T_NVM_TIMEOUT);
	g_t_pwr_off = us2cycles(OPTIGA_T_PWR_OFF);
	g_t_pwr_up = us2cycles(OPTIGA_T_PWR_UP);

	g_gpio_phy = &(di->pl061_gpio);
	g_splk_flags = 0;

	return 0;
}

void optiga_swi_unloading(void)
{
	g_gpio_phy = NULL;
}

void optiga_delay(unsigned long cnt_delay)
{
	cycles_t start = get_cycles();
	cycles_t cnt_gap = 0;

	while (cnt_gap < cnt_delay)
		cnt_gap = (get_cycles() - start);
}

unsigned long optiga_get_timeout_val(enum timeout_case to_case)
{
	switch (to_case) {
	case NVM_TIMEOUT:
		return (unsigned long)g_t_nvm_to;
	case RESP_TIMEOUT:
		return (unsigned long)g_t_resp_to;
	case RESP_TIMEOUT_LONG:
		return (unsigned long)g_t_resp_lto;
	default:
		return (unsigned long)g_t_resp_lto;
	}
}

void optiga_swi_slk_irqsave(void)
{
	if (!g_lock_status) {
		raw_spin_lock_irqsave(g_gpio_phy->lock, g_splk_flags);
		g_lock_status = true;
	}
}

void optiga_swi_slk_irqstore(void)
{
	if (g_lock_status) {
		raw_spin_unlock_irqrestore(g_gpio_phy->lock, g_splk_flags);
		g_lock_status = false;
	}
}

void optiga_get_gpio_bank_dir(void)
{
	unsigned char current_gpio_bank_dir;
	uint8_t gpio_in_mask, gpio_out_mask;

	if (!g_gpio_phy) {
		hwlog_err("%s: g_gpio_phy NULL\n", __func__);
		return;
	}
	current_gpio_bank_dir = readb(g_gpio_phy->gpio_dir_addr);
	gpio_in_mask = current_gpio_bank_dir & (~BIT(g_gpio_phy->offset));
	gpio_out_mask = current_gpio_bank_dir | BIT(g_gpio_phy->offset);
	g_gpio_phy->gpio_dir_in_data = gpio_in_mask;
	g_gpio_phy->gpio_dir_out_data = gpio_out_mask;
}

void optiga_gpio_dir_input(void)
{
	if (!g_gpio_phy) {
		hwlog_err("%s: g_gpio_phy NULL\n", __func__);
		return;
	}
	writeb(g_gpio_phy->gpio_dir_in_data, g_gpio_phy->gpio_dir_addr);
}

void optiga_gpio_dir_output(uint8_t value)
{
	if (!g_gpio_phy) {
		hwlog_err("%s: g_gpio_phy NULL\n", __func__);
		return;
	}
	writeb(g_gpio_phy->gpio_dir_out_data, g_gpio_phy->gpio_dir_addr);
	writeb(!!value << (g_gpio_phy->offset), g_gpio_phy->gpio_data_addr);
}

void optiga_gpio_set_value(uint8_t value)
{
	if (!g_gpio_phy) {
		hwlog_err("%s: g_gpio_phy NULL\n", __func__);
		return;
	}
	writeb(!!value << (g_gpio_phy->offset), g_gpio_phy->gpio_data_addr);
}

int optiga_gpio_get_value(void)
{
	if (!g_gpio_phy) {
		hwlog_err("%s: g_gpio_phy NULL\n", __func__);
		return SWI_GPIO_READ_FAIL;
	}
	return !!readb(g_gpio_phy->gpio_data_addr);
}

void opiga_select_chip_addr(uint16_t ic_addr)
{
	optiga_swi_send_byte(SWI_EDA, ((uint8_t)((ic_addr >> BIT_P_BYT) & 0xff)));
	optiga_swi_send_byte(SWI_SDA, ((uint8_t)(ic_addr & 0xff)));
}

void optiga_swi_pwr_on(void)
{
	optiga_get_gpio_bank_dir();
	optiga_gpio_dir_output(HIGH_VOLTAGE);
	optiga_delay(g_t_pwr_up);
	optiga_gpio_dir_input();
}

void optiga_swi_pwr_off(void)
{
	optiga_get_gpio_bank_dir();
	optiga_gpio_dir_output(LOW_VOLTAGE);
	optiga_delay(g_t_pwr_off);
	optiga_gpio_dir_input();
}

void optiga_swi_pwr_off_by_cmd(void)
{
	optiga_get_gpio_bank_dir();
	optiga_swi_send_byte(SWI_BC, SWI_PWDN);
	optiga_gpio_dir_input();
}

static int optiga_swi_inversion_check(uint8_t code, uint8_t data)
{
	int i;
	int cnt = 0;
	uint16_t data_cmb;

	data_cmb = code;
	data_cmb = (data_cmb << BIT_P_BYT) | data;
	for (i = 0; i < BIT_P_BYT_SWI; i++) {
		cnt += (data_cmb & 0x01);
		data_cmb >>= 1;
	}

	if (cnt > BIT_P_BYT_SWI / 2)
		return 1;
	else
		return 0;
}

void optiga_swi_send_byte(uint8_t code, uint8_t data)
{
	int inversion;

	inversion = optiga_swi_inversion_check(code, data);
	/* in case of inversion need, invert all bits except bit 0 & 1 */
	if (inversion) {
		code ^= 0x03; /* bit2 & bit3 do not need inversion */
		data = ~data;
	}

	optiga_get_gpio_bank_dir();
	optiga_gpio_dir_output(HIGH_VOLTAGE);
	optiga_delay(g_t_stop);

	/* send bytes in MSB order */
	/* send bit12 to bit9 */
	optiga_gpio_set_value(0);
	(code & 0x08) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(1);
	(code & 0x04) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(0);
	(code & 0x02) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(1);
	(code & 0x01) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);

	/* send bit8 to bit1 */
	optiga_gpio_set_value(0);
	(data & 0x80) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(1);
	(data & 0x40) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(0);
	(data & 0x20) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(1);
	(data & 0x10) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(0);
	(data & 0x08) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(1);
	(data & 0x04) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(0);
	(data & 0x02) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);
	optiga_gpio_set_value(1);
	(data & 0x01) ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);

	/* send bit0: inversion bit */
	optiga_gpio_set_value(0);
	inversion ? optiga_delay(g_t_logic1) : optiga_delay(g_t_logic0);

	optiga_gpio_set_value(1);
	optiga_gpio_dir_input();
}

int optiga_swi_read_byte(uint8_t *data)
{
	uint16_t data_loc;
	int prv_state;
	int bit_ind;
	unsigned long cnt_start;
	unsigned long cnt_bit;
	unsigned long cnt_mid;
	unsigned long cnt_bit_min;
	unsigned long cnt_bit_max;
	unsigned long cnt_bits[BIT_P_BYT_SWI];
	unsigned long cnt_timeout = 0;

	cnt_start = get_cycles();
	optiga_get_gpio_bank_dir();
	optiga_gpio_dir_input();
	while (optiga_gpio_get_value() && cnt_timeout < g_t_resp_to)
		cnt_timeout = get_cycles() - cnt_start;
	cnt_start = cnt_start + cnt_timeout;
	if (cnt_timeout >= g_t_resp_to)
		goto swi_resp_timeout;

	prv_state = 0;
	for (bit_ind = BIT_P_BYT_SWI - 1; bit_ind >= 0; bit_ind--) {
		cnt_bit = 0;
		while ((optiga_gpio_get_value() == prv_state) && (cnt_bit < g_t_stop))
			cnt_bit = get_cycles() - cnt_start;
		cnt_start = cnt_start + cnt_bit;
		if (cnt_bit >= g_t_stop)
			goto swi_bit_timeout;
		cnt_bits[bit_ind] = cnt_bit;
		prv_state = !prv_state;
	}

	/* decoding */
	cnt_bit_min = cnt_bits[0];
	cnt_bit_max = cnt_bit_min;
	for (bit_ind = 1; bit_ind < BIT_P_BYT_SWI; bit_ind++) {
		if (cnt_bits[bit_ind] < cnt_bit_min)
			cnt_bit_min = cnt_bits[bit_ind];
		else if (cnt_bits[bit_ind] > cnt_bit_max)
			cnt_bit_max = cnt_bits[bit_ind];
	}
	cnt_mid = (cnt_bit_max + cnt_bit_min) / 2;

	data_loc = 0;
	for (bit_ind = 0; bit_ind < BIT_P_BYT_SWI; bit_ind++) {
		if (cnt_bits[bit_ind] > cnt_mid)
			data_loc |= BIT(bit_ind);
	}

	if (((data_loc >> (BIT_P_BYT_SWI - 1)) & 0x01) ==
		((data_loc >> (BIT_P_BYT_SWI - 2)) & 0x01))
		goto swi_training_err;

	*data = (uint8_t)((data_loc >> 1) & 0xff);
	if (data_loc & 0x01) /* check inversion bit */
		*data = ~(*data);

	return 0;
swi_training_err:
	hwlog_err("%s: training err\n", __func__);
	return -SWI_ERR_TRAINING;
swi_bit_timeout:
	hwlog_err("%s: bit %d timeout\n", __func__, bit_ind);
	return -SWI_ERR_BIT_TO;
swi_resp_timeout:
	hwlog_err("%s: byte response timeout\n", __func__);
	return -SWI_ERR_RESP_TO;
}

/* security ic operations */
static int onewire_phy_pl061_open_ic(void)
{
	optiga_swi_slk_irqsave();
	optiga_get_gpio_bank_dir();
	optiga_gpio_dir_output(HIGH_VOLTAGE);
	optiga_swi_slk_irqstore();

	return 0;
}

static int onewire_phy_pl061_close_ic(void)
{
	optiga_swi_slk_irqsave();
	optiga_get_gpio_bank_dir();
	optiga_gpio_dir_input();
	optiga_swi_slk_irqstore();

	return 0;
}

static const struct bat_security_ic_ops sle95250_ow_phy_pl061_ops = {
	.open_ic = onewire_phy_pl061_open_ic,
	.close_ic = onewire_phy_pl061_close_ic,
};

void sle95250_reg_sec_ic_ops(void)
{
	bat_security_ic_ops_register(&sle95250_ow_phy_pl061_ops);
}
