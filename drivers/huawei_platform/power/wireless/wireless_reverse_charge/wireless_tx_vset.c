/*
 * wireless_tx_vset.c
 *
 * set tx_vin for wireless reverse charging
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

#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <huawei_platform/power/wireless/wireless_tx_pwr_src.h>

#define HWLOG_TAG wireless_tx_vset
HWLOG_REGIST();

static void wltx_set_tx_fod_coef(struct wltx_dev_info *di)
{
	int ret;

	if ((di->tx_vset.para[di->tx_vset.cur].pl_th <= 0) ||
		(di->tx_vset.para[di->tx_vset.cur].pl_cnt <= 0))
		return;

	ret = wltx_ic_set_fod_coef(WLTRX_IC_MAIN,
		(u16)di->tx_vset.para[di->tx_vset.cur].pl_th,
		di->tx_vset.para[di->tx_vset.cur].pl_cnt);
	if (ret) {
		hwlog_err("set_tx_fod_coef: failed\n");
		return;
	}

	hwlog_info("[set_tx_fod_coef] succ\n");
}

static void wltx_set_tx_vset_id(struct wltx_dev_info *di, int id)
{
	di->tx_vset.cur = id;
	hwlog_info("[set_tx_vset_id] id=%d\n", id);
}

static const char *wltx_bridge_name(enum wltx_bridge_type type)
{
	static const char *const bridge_name[] = {
		[WLTX_PING_FULL_BRIDGE] = "ping in full bridge mode",
		[WLTX_PING_HALF_BRIDGE] = "ping in half bridge mode",
		[WLTX_PT_FULL_BRIDGE] = "power transfer in full bridge mode",
		[WLTX_PT_HALF_BRIDGE] = "power transfer in half bridge mode"
	};

	if ((type < WLTX_BRIDGE_BEGIN) || (type >= WLTX_BRIDGE_END))
		return "invalid bridge mode";

	return bridge_name[type];
}

static int wltx_set_bridge(int ask_vset, enum wltx_bridge_type type)
{
	hwlog_info("[set_tx_bridge] %s\n", wltx_bridge_name(type));
	return wltx_ic_set_bridge(WLTRX_IC_MAIN, ask_vset, type);
}

static bool wltx_is_half_volt(int vin, int vout)
{
	return vout * 100 < vin * 75; /* true: vout/vin<75%, otherwise false */
}

static enum wltx_bridge_type wltx_select_ping_bridge(int cur_vset, int vout)
{
	if (wltx_is_half_volt(cur_vset, vout))
		return WLTX_PING_HALF_BRIDGE;

	return WLTX_PING_FULL_BRIDGE;
}

static enum wltx_bridge_type wltx_select_pt_bridge(int cur_vset, int vout)
{
	if (wltx_is_half_volt(cur_vset, vout))
		return WLTX_PT_HALF_BRIDGE;

	return WLTX_PT_FULL_BRIDGE;
}

static enum wltx_bridge_type wltx_select_bridge(int cur_vset,
	enum wltx_stage stage, int vout)
{
	if (stage == WL_TX_STAGE_PING_RX)
		return wltx_select_ping_bridge(cur_vset, vout);

	return wltx_select_pt_bridge(cur_vset, vout);
}

static void wltx_config_adap_vout(struct wltx_dev_info *di, int vout)
{
	int adap_vout;

	if (vout < WIRELESS_ADAPTER_12V)
		adap_vout = WLTX_SC_ADAP_VSET;
	else if (vout <= WIRELESS_ADAPTER_15V)
		adap_vout = vout / CP_CP_RATIO + WLTX_SC_ADAP_V_DELTA;
	else
		adap_vout = vout / CP_CP_RATIO;

	if (di->tx_vout.v_adap == adap_vout)
		return;

	if (adaptor_cfg_for_wltx_vbus(adap_vout, WLTX_SC_ADAP_ISET))
		return;

	di->tx_vout.v_adap = adap_vout;
	di->tx_vout.v_tx = adap_vout * CP_CP_RATIO;
}

static void wltx_set_tx_vout(struct wltx_dev_info *di,
	enum wltx_stage stage, int vout, bool force)
{
	int cur_vset;
	enum wltx_bridge_type type;

	cur_vset = di->tx_vset.para[di->tx_vset.cur].vset;

	if (wltx_get_cur_pwr_src() == PWR_SRC_VBUS_CP)
		wltx_config_adap_vout(di, vout);

	type = wltx_select_bridge(cur_vset, stage, vout);
	if (!force && (type == di->tx_vset.bridge))
		return;

	if (wltx_set_bridge(di->tx_vout.v_ask, type))
		return;

	di->tx_vset.bridge = type;
}

static int wltx_recalc_tx_vset(struct wltx_dev_info *di, int tx_vset)
{
	const char *src = NULL;

	if (wltx_is_tx_open_by_hall())
		return HALL_TX_VOUT;

	if (di->pwr_type != WL_TX_PWR_5VBST_VBUS_OTG_CP)
		return tx_vset;

	src = wltx_get_pwr_src_name(di->cur_pwr_src);
	if (strstr(src, "CP"))
		return tx_vset;

	tx_vset /= CP_CP_RATIO;
	hwlog_info("[recalc_tx_vset] tx_vset=%dmV\n", tx_vset);

	return tx_vset;
}

static void wltx_set_tx_vset_by_id(struct wltx_dev_info *di, int id, bool force_flag)
{
	int tx_vset;

	if ((id < 0) || (id >= di->tx_vset.total))
		return;

	if (di->pwr_type == WL_TX_PWR_5VBST_VBUS_OTG_CP) {
		wltx_set_tx_vset_id(di, id);
		return;
	}

	tx_vset = di->tx_vset.para[id].vset;
	if (!force_flag && (tx_vset == di->tx_vset.para[di->tx_vset.cur].vset))
		return;
	if (wltx_ic_set_vset(WLTRX_IC_MAIN, tx_vset))
		return;
	if (di->tx_vset.para[id].ext_hdl == WLTX_EXT_HDL_BP2CP)
		wltx_set_pwr_src_output(true, PWR_SRC_BP2CP);

	wltx_set_tx_vset_id(di, id);
}

static void wltx_set_tx_vset(struct wltx_dev_info *di, int tx_vset, bool force)
{
	int i;

	if (di->tx_vset.cur < 0)
		return;
	if (di->tx_vset.cur >= WLTX_TX_VSET_TYPE_MAX)
		return;

	tx_vset = wltx_recalc_tx_vset(di, tx_vset);
	for (i = 0; i < di->tx_vset.total; i++) {
		if ((tx_vset >= di->tx_vset.para[i].rx_vmin) &&
			(tx_vset < di->tx_vset.para[i].rx_vmax))
			break;
	}
	if (i >= di->tx_vset.total) {
		hwlog_err("set_tx_vset: tx_vset=%dmV mismatch\n", tx_vset);
		return;
	}

	wltx_set_tx_vset_by_id(di, i, force);
	wltx_set_tx_fod_coef(di);
}

void wltx_set_tx_volt(enum wltx_stage stage, int vtx, bool force)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return;

	if (di->pwr_type != WL_TX_PWR_5VBST_VBUS_OTG_CP)
		return wltx_set_tx_vset(di, vtx, force);

	if (stage == WL_TX_STAGE_POWER_SUPPLY)
		return wltx_set_tx_vset(di, vtx, force);

	if (stage >= WL_TX_STAGE_PING_RX)
		return wltx_set_tx_vout(di, stage, vtx, force);
}

void wltx_reset_vset_para(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return;

	if (di->rx_disc_pwr_on) {
		di->rx_disc_pwr_on = false;
		return;
	}

	wltx_set_tx_vset_id(di, 0);
	di->tx_vset.bridge = -1;
	di->tx_vout.v_adap = -1;
	di->tx_vout.v_tx = -1;
}
