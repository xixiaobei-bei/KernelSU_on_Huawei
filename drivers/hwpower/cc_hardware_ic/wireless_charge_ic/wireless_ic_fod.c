// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_ic_fod.c
 *
 * ic fod for wireless charging
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

#include <linux/slab.h>
#include <chipset_common/hwpower/hardware_ic/wireless_ic_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#define HWLOG_TAG wireless_ic_fod
HWLOG_REGIST();

#define WLRX_FOD_TMP_STR_LEN       16
#define WLRX_FOD_CFG_ROW           20

static struct wlrx_fod_ploss *g_rx_fod_ploss[WLTRX_IC_MAX];

static struct wlrx_fod_ploss *wlrx_get_dts_fod_ploss(unsigned int ic_type)
{
	if (!wltrx_ic_is_type_valid(ic_type))
		return NULL;

	return g_rx_fod_ploss[ic_type];
}

u8 *wlrx_get_fod_ploss_th(unsigned int ic_type, int vfc_cfg, unsigned int tx_type, u16 fod_len)
{
	int i;
	u8 *rx_fod = NULL;
	enum wlrx_scene scn_id;
	struct wlrx_fod_ploss *rx_ploss = wlrx_get_dts_fod_ploss(ic_type);

	if (!rx_ploss)
		return NULL;

	scn_id = wlrx_get_scene();
	if ((scn_id < 0) || (scn_id >= WLRX_SCN_END))
		return NULL;

	for (i = 0; i < rx_ploss->rx_fod_cfg_row; i++) {
		if (rx_ploss->ploss_cond[i].vfc != vfc_cfg)
			continue;
		if ((rx_ploss->ploss_cond[i].scn >= 0) && (rx_ploss->ploss_cond[i].scn != scn_id))
			continue;
		if ((rx_ploss->ploss_cond[i].tx_type >= 0) &&
			(rx_ploss->ploss_cond[i].tx_type != tx_type))
			continue;
		break;
	}
	hwlog_info("[get_fod_ploss_th] id=%d scn_id=%d tx_type=%u vfc_cfg=%d\n", i, scn_id, tx_type, vfc_cfg);
	if (i >= rx_ploss->rx_fod_cfg_row) {
		hwlog_err("get_fod_ploss_th: ploss_th mismatch\n");
		return NULL;
	}

	rx_fod = rx_ploss->ploss_th + i * fod_len;
	power_print_u8_array("rx_ploss_th", rx_fod, 1, fod_len); /* row=1 */
	return rx_fod;
}

static int wlrx_parse_fod_ploss(unsigned int ic_type, const struct device_node *np,
	struct wlrx_fod_ploss *di, u16 fod_len)
{
	int i;
	char buffer[WLRX_FOD_TMP_STR_LEN] = { 0 };

	di->ploss_th = kcalloc(di->rx_fod_cfg_row, fod_len, GFP_KERNEL);
	if (!di->ploss_th)
		return -ENOMEM;

	for (i = 0; i < di->rx_fod_cfg_row; i++) {
		snprintf(buffer, WLRX_FOD_TMP_STR_LEN - 1, "rx_ploss_th%d", i);
		if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
			buffer, di->ploss_th + i * fod_len, fod_len))
			goto parse_plossth_fail;
		memset(buffer, 0, WLRX_FOD_TMP_STR_LEN);
	}

	power_print_u8_array("rx_ploss_th", di->ploss_th, di->rx_fod_cfg_row, fod_len);
	g_rx_fod_ploss[ic_type] = di;
	return 0;

parse_plossth_fail:
	kfree(di->ploss_th);
	return -EINVAL;
}

int wlrx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np, u16 fod_len)
{
	int i, len;
	struct wlrx_fod_ploss *di = NULL;

	if (!np || !wltrx_ic_is_type_valid(ic_type) || g_rx_fod_ploss[ic_type]) {
		hwlog_err("parse_fod_cfg: ic_type=%u err or repeate\n", ic_type);
		return -EINVAL;
	}

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_cond", WLRX_FOD_CFG_ROW, WLRX_FOD_COND_END);
	if (len <= 0)
		goto parse_fod_fail;

	di->rx_fod_cfg_row = len / WLRX_FOD_COND_END;
	di->ploss_cond = kcalloc(di->rx_fod_cfg_row, sizeof(*(di->ploss_cond)), GFP_KERNEL);
	if (!di->ploss_cond)
		goto parse_fod_fail;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_cond", (int *)di->ploss_cond, di->rx_fod_cfg_row, WLRX_FOD_COND_END);
	if (len <= 0)
		goto parse_ploss_cond_fail;

	for (i = 0; i < di->rx_fod_cfg_row; i++)
		hwlog_info("rx_ploss_cond[%d] id=%d scn=%d tx_type=%d vfc=%d\n", i,
			di->ploss_cond[i].id, di->ploss_cond[i].scn,
			di->ploss_cond[i].tx_type, di->ploss_cond[i].vfc);

	if (!wlrx_parse_fod_ploss(ic_type, np, di, fod_len))
		return 0;

parse_ploss_cond_fail:
	kfree(di->ploss_cond);
parse_fod_fail:
	kfree(di);
	hwlog_err("parse_fod_cfg: failed\n");
	return -EINVAL;
}
