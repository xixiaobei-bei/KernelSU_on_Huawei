/*
 * wireless_rx_ui.c
 *
 * ui handler for wireless charge
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

#include <linux/kernel.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ui.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common.h>

#define HWLOG_TAG wireless_rx_ui
HWLOG_REGIST();

/*
 * tx_max_pwr only supports powers of integer multiples of 5
 * if 0 <= pwr <= 2, takes 0
 * if 3 <= pwr <= 6, takes 5
 * if 7 <= pwr <= 9, carry takes 1
 */
static struct wlrx_max_pwr_revise g_revise_tbl[] = {
	{ 0, 2, 0, 0 },
	{ 3, 6, 5, 0 },
	{ 7, 9, 0, 1 },
};
static int g_exceptional_pwr_tbl[] = { 27000 };

static bool wlrx_ui_is_exceptional_pwr(int tx_max_pwr_mv)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_exceptional_pwr_tbl); i++) {
		if (tx_max_pwr_mv == g_exceptional_pwr_tbl[i])
			return true;
	}

	return false;
}

static int wlrx_revise_ui_max_pwr(int tx_max_pwr_w)
{
	unsigned int i;
	int carry = 0;
	int remainder;
	int quotient;

	quotient = tx_max_pwr_w / POWER_BASE_DEC;
	remainder = tx_max_pwr_w % POWER_BASE_DEC;

	for (i = 0; i < ARRAY_SIZE(g_revise_tbl); i++) {
		if ((remainder >= g_revise_tbl[i].unit_low) &&
			(remainder <= g_revise_tbl[i].unit_high)) {
			remainder = g_revise_tbl[i].remainder;
			carry = g_revise_tbl[i].carry;
			break;
		}
	}

	return ((quotient + carry) * POWER_BASE_DEC + remainder) * MWATT_PER_WATT;
}

void wlrx_ui_send_max_pwr_evt(struct wlrx_ui_para *para)
{
	int cur_pwr;
	int tx_max_pwr_mw;

	if (!para || (para->ui_max_pwr <= 0) || (para->product_max_pwr <= 0) ||
		(para->tx_max_pwr < para->ui_max_pwr))
		return;

	tx_max_pwr_mw = para->tx_max_pwr;
	if (!wlrx_ui_is_exceptional_pwr(tx_max_pwr_mw))
		tx_max_pwr_mw = wlrx_revise_ui_max_pwr(tx_max_pwr_mw / MWATT_PER_WATT);

	hwlog_info("[send_max_pwr] revised_tx_pwr=%d ui_pwr=%d product_pwr=%d\n",
		tx_max_pwr_mw, para->ui_max_pwr, para->product_max_pwr);

	cur_pwr = (tx_max_pwr_mw < para->product_max_pwr) ?
		tx_max_pwr_mw : para->product_max_pwr;
	power_ui_event_notify(POWER_UI_NE_MAX_POWER, &cur_pwr);
}

void wlrx_ui_send_soc_decimal_evt(struct wlrx_ui_para *para)
{
	int cur_pwr;

	if (!para)
		return;

	cur_pwr = (para->tx_max_pwr < para->product_max_pwr) ?
		para->tx_max_pwr : para->product_max_pwr;
	hwlog_info("[send_soc_decimal] tx_pwr=%d product_pwr=%d\n",
		para->tx_max_pwr, para->product_max_pwr);
	power_event_bnc_notify(POWER_BNT_SOC_DECIMAL, POWER_NE_SOC_DECIMAL_WL_DC, &cur_pwr);
}
