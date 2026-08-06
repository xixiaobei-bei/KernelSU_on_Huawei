/*
 * wireless_rx_plim.c
 *
 * power limit for wireless charging
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

#include <linux/slab.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>

#define HWLOG_TAG wireless_rx_plim
HWLOG_REGIST();

static struct wlrx_plim g_plim;
static struct wlrx_plim_para const g_plim_tbl[WLRX_PLIM_SRC_END] = {
	{ WLRX_PLIM_SRC_OTG,        "otg",        false, 5000,  5500,  1000 },
	{ WLRX_PLIM_SRC_RPP,        "rpp",        true,  12000, 12000, 1300 },
	{ WLRX_PLIM_SRC_FAN,        "fan",        true,  9000,  9900,  1250 },
	{ WLRX_PLIM_SRC_VOUT_ERR,   "vout_err",   true,  9000,  9900,  1250 },
	{ WLRX_PLIM_SRC_TX_ALARM,   "tx_alarm",   true,  12000, 12000, 1300 },
	{ WLRX_PLIM_SRC_TX_BST_ERR, "tx_bst_err", true,  5000,  5500,  1000 },
	{ WLRX_PLIM_SRC_KB,         "keyboard",   true,  9000,  9900,  1100 },
	{ WLRX_PLIM_SRC_THERMAL,    "thermal",    false, 9000,  9900,  1250 },
};

int wlrx_get_plim_src(void)
{
	return g_plim.src;
}

void wlrx_set_plim_src(int src_id)
{
	if ((src_id < WLRX_PLIM_SRC_BEGIN) || (src_id >= WLRX_PLIM_SRC_END))
		return;

	if (test_bit(src_id, &g_plim.src))
		return;
	set_bit(src_id, &g_plim.src);
	if (src_id != g_plim_tbl[src_id].src_id)
		return;
	hwlog_info("set_plimit_src: %s\n", g_plim_tbl[src_id].src_name);
}

void wlrx_clear_plim_src(int src_id)
{
	if ((src_id < WLRX_PLIM_SRC_BEGIN) || (src_id >= WLRX_PLIM_SRC_END))
		return;

	if (!test_bit(src_id, &g_plim.src))
		return;
	clear_bit(src_id, &g_plim.src);
	if (src_id != g_plim_tbl[src_id].src_id)
		return;
	hwlog_info("clear_plimit_src: %s\n", g_plim_tbl[src_id].src_name);
}

void wlrx_reset_plim_para(void)
{
	int i;

	for (i = WLRX_PLIM_SRC_BEGIN; i < WLRX_PLIM_SRC_END; i++) {
		if (g_plim_tbl[i].need_rst)
			clear_bit(i, &g_plim.src);
	}
}

void wlrx_update_plim_pctrl(struct wlrx_pctrl *pctrl)
{
	int i;

	if (!pctrl)
		return;

	for (i = WLRX_PLIM_SRC_BEGIN; i < WLRX_PLIM_SRC_END; i++) {
		if (!test_bit(i, &g_plim.src))
			continue;
		pctrl->vtx = min(pctrl->vtx, g_plim_tbl[i].vtx);
		pctrl->vrx = min(pctrl->vrx, g_plim_tbl[i].vrx);
		pctrl->irx = min(pctrl->irx, g_plim_tbl[i].irx);
	}
}
