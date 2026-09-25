/*
 * wireless_direct_charger.c
 *
 * wireless direct charge driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <huawei_platform/power/huawei_battery_temp.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/wireless_dc_error_handle.h>
#include <huawei_platform/power/wireless_direct_charger.h>
#include <huawei_platform/power/wireless_charger.h>

#define HWLOG_TAG wireless_dc
HWLOG_REGIST();

static struct wldc_dev_info *g_wldc_di;
static int g_wldc_need_wired_sw_on = 1;
static bool g_wldc_stop_complete_flag = true;

static char g_wldc_stage_char[WLDC_STAGE_TOTAL][WIRELESS_STAGE_STR_LEN] = {
	{ "STAGE_DEFAULT" }, { "STAGE_CHECK" }, { "STAGE_SUCCESS" },
	{ "STAGE_CHARGING" }, { "STAGE_CHARGE_DONE" }, { "STAGE_STOP_CHARGING" }
};

void wldc_set_di(struct wldc_dev_info *di)
{
	if (!di)
		hwlog_err("%s: di null!\n", __func__);
	else
		g_wldc_di = di;
}

void wldc_set_charge_stage(enum wldc_stage wldc_stage)
{
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (wldc_stage < WLDC_STAGE_TOTAL && di->wldc_stage != wldc_stage) {
		di->wldc_stage = wldc_stage;
		hwlog_info("[%s] set charge stage to %s\n",
			__func__, g_wldc_stage_char[wldc_stage]);
	}
}

static int wldc_msleep(int sleep_ms)
{
	int i;
	int interval = 25;  /* ms */
	int cnt = sleep_ms / interval;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: di null!\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	for (i = 0; i < cnt; i++) {
		msleep(interval);
		if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
			hwlog_err("%s: wireless tx disconnect\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
	}
	return 0;
}

static bool wldc_is_mode_para_valid(struct wldc_dev_info *di, int id)
{
	if (di->total_dc_mode <= 0)
		return false;

	if ((id < 0) || (id >= di->total_dc_mode)) {
		hwlog_err("%s: dc_mode 0x%x out of range[0, 0x%x]\n", __func__,
			id, di->total_dc_mode - 1); /* mode start from zero */
		return false;
	}

	return true;
}

static int wldc_get_bat_voltage(void)
{
	int btb_vol;
	int package_vol;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di || !di->bi_ops ||
		!di->bi_ops->get_bat_btb_voltage ||
		!di->bi_ops->get_bat_package_voltage) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	btb_vol = di->bi_ops->get_bat_btb_voltage();
	package_vol = di->bi_ops->get_bat_package_voltage();
	if ((btb_vol < 0) && (package_vol < 0)) {
		di->stop_flag_error = 1;
		hwlog_err("%s: error\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_GET_VBATT, NULL);
		return -WLC_ERR_PARA_WRONG;
	}

	return (btb_vol > package_vol) ? btb_vol : package_vol;
}

static int wldc_get_bat_current(void)
{
	int ret;
	int bat_curr = 0;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di || !di->bi_ops || !di->bi_ops->get_bat_current) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	ret = di->bi_ops->get_bat_current(&bat_curr);
	if (ret) {
		di->stop_flag_error = 1;
		hwlog_err("%s: error\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_GET_IBATT, NULL);
	}
	return bat_curr;
}

int wldc_get_ls_vbus(void)
{
	int ret;
	int vbus = 0;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di || !di->bi_ops || !di->bi_ops->get_vbus_voltage) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	ret = di->bi_ops->get_vbus_voltage(&vbus);
	if (ret < 0) {
		di->stop_flag_error = 1;
		hwlog_err("%s: error\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_GET_LS_VBUS, NULL);
	}
	return vbus;
}

static int wldc_get_ls_ibus(void)
{
	int ret;
	int ibus = 0;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di || !di->bi_ops || !di->bi_ops->get_ls_ibus) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	ret = di->bi_ops->get_ls_ibus(&ibus);
	if (ret < 0) {
		di->stop_flag_error = 1;
		hwlog_err("%s: error\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_GET_LS_IBUS, NULL);
	}
	return ibus;
}

static int wldc_get_cp_iout(void)
{
	int rx_ratio;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return 0;

	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;

	return rx_ratio * wireless_charge_get_rx_iout();
}

static int wldc_get_cp_avg_iout(void)
{
	int rx_ratio;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return 0;

	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;
	return rx_ratio * wireless_charge_get_rx_avg_iout();
}

int wldc_get_error_cnt(void)
{
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_debug("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return 0;

	return di->mode_para[di->cur_dc_mode].err_cnt;
}

int wldc_get_warning_cnt(void)
{
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_debug("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return 0;

	return di->mode_para[di->cur_dc_mode].warn_cnt;
}

bool wldc_is_stop_charging_complete(void)
{
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: g_wldc_di null\n", __func__);
		return true;
	}

	return (g_wldc_stop_complete_flag == true);
}

void wldc_extra_power_supply(int flag)
{
	int cur_dc_mode;
	enum wldc_ext_pwr_type pwr_type;
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: g_wldc_di null\n", __func__);
		return;
	}

	cur_dc_mode = di->cur_dc_mode;
	hwlog_info("[%s] cur dc_mode = 0x%x\n", __func__, cur_dc_mode);

	if ((cur_dc_mode < 0) || (cur_dc_mode >= di->total_dc_mode)) {
		hwlog_err("%s: cur_dc_mode(0x%x) out of range [0, 0x%x)\n",
			__func__, cur_dc_mode, di->total_dc_mode);
		return;
	}

	pwr_type = di->mode_para[cur_dc_mode].init_para.ext_pwr_type;
	switch (pwr_type) {
	case WLDC_EXT_PWR_TO_SC_IC:
		boost_5v_enable((flag == WLPS_CTRL_ON) ?
			BOOST_5V_ENABLE : BOOST_5V_DISABLE, BOOST_CTRL_WLDC);
		break;
	case WLDC_EXT_PWR_TO_RX_IC:
		wlps_control(WLPS_RX_EXT_PWR, flag);
		break;
	default:
		break;
	}
}

static bool can_vbatt_do_wldc_charge(struct wldc_dev_info *di, int dc_mode)
{
	int vbatt_min;
	int vbatt_max;
	int vbatt = hisi_battery_voltage();
	int ls_vbatt = wldc_get_bat_voltage();

	if (!wldc_is_mode_para_valid(di, dc_mode))
		return false;

	if ((ls_vbatt > 0) && (ls_vbatt < vbatt))
		vbatt = ls_vbatt;

	vbatt_min = di->mode_para[dc_mode].init_para.vbatt_min;
	vbatt_max = di->mode_para[dc_mode].init_para.vbatt_max;
	if ((vbatt < vbatt_min) || (vbatt > vbatt_max))
		return false;
	return true;
}

static bool can_tbatt_do_wldc_charge(struct wldc_dev_info *di)
{
	int tbatt = hisi_battery_temperature();

	if (tbatt <= di->tbatt_lth) {
		di->tbatt_lth = WLDC_TBATT_MIN_BACK;
		return false;
	}
	di->tbatt_lth = WLDC_TBATT_MIN;
	if (tbatt >= di->tbatt_hth) {
		di->tbatt_hth = WLDC_TBATT_MAX_BACK;
		return false;
	}
	di->tbatt_hth = WLDC_TBATT_MAX;

	return true;
}

static int wldc_retore_normal_charge_path(void)
{
	return charge_set_hiz_enable(HIZ_MODE_DISABLE);
}

int wldc_cut_off_normal_charge_path(void)
{
	return charge_set_hiz_enable(HIZ_MODE_ENABLE);
}

int wldc_turn_on_direct_charge_channel(void)
{
	if (g_wldc_need_wired_sw_on)
		return wired_chsw_set_wired_reverse_channel(
			WIRED_REVERSE_CHANNEL_RESTORE);
	else
		return 0;
}

int wldc_turn_off_direct_charge_channel(void)
{
	if (g_wldc_need_wired_sw_on)
		return wired_chsw_set_wired_reverse_channel(
			WIRED_REVERSE_CHANNEL_CUTOFF);
	else
		return 0;
}

static int wldc_check_ops_valid(struct wldc_dev_info *di)
{
	if (!di || !di->ls_ops ||
		!di->ls_ops->ls_init ||
		!di->ls_ops->ls_enable ||
		!di->ls_ops->is_ls_close ||
		!di->ls_ops->ls_exit) {
		hwlog_err("%s: ls_ops null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	if (!di || !di->bi_ops ||
		!di->bi_ops->init ||
		!di->bi_ops->get_ls_ibus ||
		!di->bi_ops->get_bat_btb_voltage ||
		!di->bi_ops->get_bat_package_voltage ||
		!di->bi_ops->exit) {
		hwlog_err("%s: bi_ops null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	return 0;
}

int wldc_set_rx_init_vout(struct wldc_dev_info *di)
{
	int ret;
	int rx_init_vout;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return -WLC_ERR_PARA_WRONG;

	rx_init_vout = di->mode_para[di->cur_dc_mode].init_para.rx_vout;
	ret = wldc_set_trx_vout(rx_init_vout);
	if (ret) {
		hwlog_err("%s: set trx init vout fail\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_SET_TRX_INIT_VOUT, NULL);
		return ret;
	}
	di->rx_vout_set = rx_init_vout;
	return 0;
}

int wldc_chip_init(struct wldc_dev_info *di)
{
	int ret;

	if (!di || !di->ls_ops || !di->ls_ops->ls_init ||
		!di->bi_ops || !di->bi_ops->init) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = di->ls_ops->ls_init();
	if (ret) {
		hwlog_err("%s: ls init fail\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_LS_INIT, NULL);
		return ret;
	}
	ret = di->bi_ops->init();
	if (ret) {
		hwlog_err("%s: bi init fail\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_BI_INIT, NULL);
		return ret;
	}
	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

static int wldc_open_direct_charge_path(struct wldc_dev_info *di)
{
	int ret;
	int i;
	int vbatt;
	int ls_vbatt;
	int ls_ibus;
	int rx_vrect;
	int rx_vout;
	int rx_vout_set;
	int soc;
	int vatt_max;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return -WLC_ERR_PARA_WRONG;

	soc = hisi_battery_capacity();
	vbatt = hisi_battery_voltage();
	ls_vbatt = wldc_get_bat_voltage();
	if (ls_vbatt < 0) {
		hwlog_err("%s: get ls_vbatt fail\n", __func__);
		return -WLC_ERR_CHECK_FAIL;
	}
	hwlog_info("[%s] vbatt ls:%dmV,coul:%dmV\n", __func__, ls_vbatt, vbatt);
	rx_vout_set = di->mode_para[di->cur_dc_mode].init_para.rx_ratio *
		(ls_vbatt * di->volt_ratio +
		di->mode_para[di->cur_dc_mode].init_para.vdelt);
	ret = wireless_charge_set_rx_vout(rx_vout_set);
	if (ret) {
		hwlog_err("%s: set rx vout fail\n", __func__);
		snprintf(tmp_buf, sizeof(tmp_buf), "set rx_vout\n");
		goto open_fail;
	}
	di->rx_vout_set = rx_vout_set;
	ret = wldc_msleep(500); /* delay for vout stability, generally 300ms */
	if (ret)
		return ret;
	ret = di->ls_ops->ls_enable(1);
	if (ret) {
		hwlog_err("%s: ls enable fail!\n", __func__);
		snprintf(tmp_buf, sizeof(tmp_buf), "ls_en\n");
		goto open_fail;
	}

	vatt_max = di->mode_para[di->cur_dc_mode].init_para.vbatt_max;
	for (i = 0; i < WLDC_OPEN_PATH_CNT; i++) {
		ret = wldc_msleep(500); /* delay 500ms for iout check */
		if (ret)
			return ret;
		ls_ibus = wldc_get_ls_ibus();
		if (ls_ibus > WLDC_OPEN_PATH_IOUT_MIN) {
			hwlog_info("[%s] succ\n", __func__);
			return 0;
		}
		ls_vbatt = wldc_get_bat_voltage();
		if (ls_vbatt > vatt_max) {
			hwlog_err("%s: ls_vbatt(%dmV) too high, ls_ibus = %d\n",
				__func__, ls_vbatt, ls_ibus);
			snprintf(tmp_buf, sizeof(tmp_buf),
				"ls_vbatt(%dmV) high, ls_ibus=%dmA",
				ls_vbatt, ls_ibus);
			goto open_fail;
		}
		rx_vout_set += WLDC_DEFAULT_VSTEP;
		ret = wireless_charge_set_rx_vout(rx_vout_set);
		if (ret) {
			hwlog_err("%s: set rx vout fail\n", __func__);
			snprintf(tmp_buf, sizeof(tmp_buf), "set rx_vout");
			goto open_fail;
		}
		di->rx_vout_set = rx_vout_set;
		rx_vrect = wireless_charge_get_rx_vrect();
		rx_vout = wireless_charge_get_rx_vout();
		hwlog_info("[%s] ls_ibus = %dmA, vrect = %dmV, vout = %dmV\n",
			__func__, ls_ibus, rx_vrect, rx_vout);
		di->ls_ops->ls_enable(1);
		if (di->ls_ops->is_ls_close())
			hwlog_err("%s: ls is close\n", __func__);
		if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
			hwlog_err("%s: wireless tx disconnect\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
	}
	if (i >= WLDC_OPEN_PATH_CNT) {
		hwlog_info("%s: try too many times, ls_ibus = %d\n",
			__func__, ls_ibus);
		snprintf(tmp_buf, sizeof(tmp_buf),
			"try too many times, ls_ibus=%dmA", ls_ibus);
		goto open_fail;
	}

	hwlog_info("[%s] succ\n", __func__);
	return 0;

open_fail:
	wldc_fill_eh_buf(di->wldc_err_dsm_buff, sizeof(di->wldc_err_dsm_buff),
		WLDC_EH_OPEN_DC_PATH, tmp_buf);
	if (soc >= BATTERY_CAPACITY_HIGH_TH) {
		++di->mode_para[di->cur_dc_mode].dc_open_retry_cnt;
		hwlog_info("[%s] fail caused by high battery capacity(%d%%)\n",
			__func__, soc);
	}
	return -WLC_ERR_CHECK_FAIL;
}

static int wldc_leak_current_check(struct wldc_dev_info *di)
{
	int i;
	int iout;
	int ileak_th;
	char tmp_buf[POWER_DSM_BUF_SIZE_0128] = { 0 };

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return -WLC_ERR_PARA_WRONG;

	ileak_th = di->mode_para[di->cur_dc_mode].init_para.ileak_th;
	for (i = 0; i < WLDC_LEAK_CURRENT_CHECK_CNT; i++) {
		iout = wireless_charge_get_rx_iout();
		hwlog_info("%s: leak_current = %dmA\n", __func__, iout);
		if (iout > ileak_th) {
			snprintf(tmp_buf, sizeof(tmp_buf), "ileak=%d\n", iout);
			wldc_fill_eh_buf(di->wldc_err_dsm_buff,
				sizeof(di->wldc_err_dsm_buff), WLDC_EH_ILEAK,
				tmp_buf);
			return -WLC_ERR_CHECK_FAIL;
		}
		if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
			hwlog_err("%s: wireless tx disconnect\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
		wldc_msleep(50); /* wait 50ms for ileak check */
	}

	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

static int wldc_vrect_vout_diff_check(struct wldc_dev_info *di)
{
	int i;
	int vrect;
	int vout;
	int vdiff_th;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	char dsm_buf[POWER_DSM_BUF_SIZE_0256 * WLDC_VDIFF_CHECK_CNT] = { 0 };

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return -WLC_ERR_PARA_WRONG;

	vdiff_th = di->mode_para[di->cur_dc_mode].init_para.vdiff_th;
	for (i = 0; i < WLDC_VDIFF_CHECK_CNT; i++) {
		vrect = wireless_charge_get_rx_vrect();
		vout = wireless_charge_get_rx_vout();
		hwlog_info("%s: vrect = %dmV, vout = %dmV, vdiff_th = %dmV\n",
			__func__, vrect, vout, vdiff_th);
		if (vrect - vout > vdiff_th) {
			hwlog_info("[%s] succ\n", __func__);
			return 0;
		}
		snprintf(tmp_buf, sizeof(tmp_buf),
			"vrect = %dmV, vout = %dmV\n", vrect, vout);
		strncat(dsm_buf, tmp_buf, strlen(tmp_buf));
		if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
			hwlog_err("%s: wireless tx disconnect\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
		wldc_msleep(100); /* delay 100ms for vdiff check */
	}
	wldc_fill_eh_buf(di->wldc_err_dsm_buff, sizeof(di->wldc_err_dsm_buff),
		WLDC_EH_VDIFF, dsm_buf);
	return -WLC_ERR_CHECK_FAIL;
}

static int wldc_vout_accuracy_check(struct wldc_dev_info *di)
{
	int ret;
	int i;
	int rx_vout;
	int rx_iout;
	int rx_init_vout;
	int rx_vout_err_th;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	char dsm_buf[POWER_DSM_BUF_SIZE_1024] = { 0 };

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return -WLC_ERR_PARA_WRONG;

	ret = wldc_msleep(1000); /* only used here, delay for stable vout */
	if (ret)
		return ret;
	rx_init_vout = di->mode_para[di->cur_dc_mode].init_para.rx_vout;
	rx_vout_err_th = di->mode_para[di->cur_dc_mode].init_para.rx_vout_th;

	for (i = 0; i < WLDC_VOUT_ACCURACY_CHECK_CNT; i++) {
		if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
			hwlog_err("%s: wireless tx disconnect\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
		rx_vout = wireless_charge_get_rx_vout();
		rx_iout = wireless_charge_get_rx_iout();
		snprintf(tmp_buf, sizeof(tmp_buf),
			"vout_set = %dmV, vout = %dmV, iout = %dmA\n",
			rx_init_vout, rx_vout, rx_iout);
		strncat(dsm_buf, tmp_buf, strlen(tmp_buf));
		hwlog_info("[%s] rx_vout_set = %dmV, rx_vout = %dmV,\t"
			"err_th = %dmV, rx_iout = %dmA\n", __func__,
			rx_init_vout, rx_vout, rx_vout_err_th, rx_iout);
		if (abs(rx_vout - rx_init_vout) <= rx_vout_err_th) {
			wldc_msleep(100); /* delay 100ms for vout accuracy */
			continue;
		}
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_EH_VOUT_ACCURACY, dsm_buf);
		return -WLC_ERR_CHECK_FAIL;
	}

	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

int wldc_security_check(struct wldc_dev_info *di)
{
	int ret;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = wldc_vout_accuracy_check(di);
	if (ret) {
		hwlog_err("%s: voltage_accuracy_check fail\n", __func__);
		return ret;
	}
	ret = wldc_leak_current_check(di);
	if (ret) {
		hwlog_err("%s: leak_current_check fail\n", __func__);
		return ret;
	}
	ret = wldc_vrect_vout_diff_check(di);
	if (ret) {
		hwlog_err("%s: vrect_vout_delta_check fail\n", __func__);
		return ret;
	}
	ret = wldc_open_direct_charge_path(di);
	if (ret) {
		hwlog_err("%s: open_direct_charge_path fail\n", __func__);
		return ret;
	}
	return 0;
}

static void wldc_send_chrg_mode(struct wldc_dev_info *di, bool flag)
{
	u8 mode = 0;

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return;

	if (flag)
		mode = (u8)di->mode_para[di->cur_dc_mode].init_para.dc_type;

	wlc_send_charge_mode(mode);
}

/*
 * Notice that :
 * if cur pmode is forever forbidden, lower power in priority must be permitted
 */
static bool wldc_is_forever_forbidden(struct wldc_dev_info *di, int dc_mode)
{
	u8 rx_support_mode;
	int pmode_id;
	const char *cur_mode_name = NULL;

	if (!wldc_is_mode_para_valid(di, dc_mode))
		goto en_priority;

	rx_support_mode = wlc_get_rx_support_mode();
	if (!(rx_support_mode &
		di->mode_para[dc_mode].init_para.dc_type))
		goto en_priority;

	cur_mode_name = di->mode_para[dc_mode].init_para.dc_name;
	if (di->mode_para[dc_mode].dis_flag & WLDC_DIS_BY_PRIORITY) {
		pmode_id = wlc_get_pmode_id_by_mode_name(cur_mode_name);
		if (!wireless_charge_mode_judge_criterion(pmode_id + 1,
			WIRELESS_MODE_FINAL_JUDGE_CRIT))
			di->mode_para[dc_mode].dis_flag
				&= ~WLDC_DIS_BY_PRIORITY;
	}
	if (di->mode_para[dc_mode].dis_flag)
		goto en_priority;

	if (di->mode_para[dc_mode].err_cnt >= WLDC_ERR_CNT_MAX) {
		if (!di->mode_para[dc_mode].dmd_report_flag &&
			(di->mode_para[dc_mode].dc_open_retry_cnt <=
			WLDC_OPEN_RETRY_CNT_MAX)) {
			wldc_show_eh_buf(di->wldc_err_dsm_buff);
			wldc_report_eh_buf(di->wldc_err_dsm_buff,
				ERROR_WIRELESS_ERROR);
			wldc_reset_eh_buf(di->wldc_err_dsm_buff,
				sizeof(di->wldc_err_dsm_buff));
			di->mode_para[dc_mode].dmd_report_flag = true;
			pmode_id = wlc_get_pmode_id_by_mode_name(cur_mode_name);
			wlc_clear_icon_pmode(pmode_id);
			wireless_charge_icon_display(
				WIRELESS_MODE_NORMAL_JUDGE_CRIT);
		}
		wldc_send_chrg_mode(di, false);
		goto en_priority;
	}
	if (di->mode_para[dc_mode].warn_cnt >= WLDC_WARNING_CNT_MAX)
		goto en_priority;

	if (dc_mode > 0)
		di->mode_para[dc_mode - 1].dis_flag |=
			WLDC_DIS_BY_PRIORITY;
	return false;


en_priority:
	if (dc_mode > 0)
		di->mode_para[dc_mode - 1].dis_flag &=
			~WLDC_DIS_BY_PRIORITY;
	return true;
}

static bool wldc_is_tmp_forbidden(struct wldc_dev_info *di, int dc_mode)
{

	if (!can_vbatt_do_wldc_charge(di, dc_mode))
		return true;
	if (!can_tbatt_do_wldc_charge(di))
		return true;

	return false;
}

static bool wldc_is_volt_param_match(struct wldc_dev_info *di, int id)
{
	int tbatt = 0;
	char *batt_brand = NULL;

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return false;

	batt_brand = hisi_battery_brand();
	huawei_battery_temp(BAT_TEMP_MIXED, &tbatt);

	if (!di->orig_volt_para_p[id].bat_info.parse_ok)
		return false;
	if (!strstr(batt_brand,
		di->orig_volt_para_p[id].bat_info.batid))
		return false;
	if (tbatt < di->orig_volt_para_p[id].bat_info.tbatt_lth)
		return false;
	if (tbatt >= di->orig_volt_para_p[id].bat_info.tbatt_hth)
		return false;
	if (di->mode_para[di->cur_dc_mode].init_para.dc_type !=
		di->orig_volt_para_p[id].bat_info.dc_type)
		return false;
	return true;
}

static bool wldc_is_dflt_volt_param_match(struct wldc_dev_info *di, int id)
{
	if (strstr(di->orig_volt_para_p[id].bat_info.batid, "default") &&
		(di->mode_para[di->cur_dc_mode].init_para.dc_type ==
		di->orig_volt_para_p[id].bat_info.dc_type))
		return true;

	return false;
}

static void wldc_select_group_volt_param(struct wldc_dev_info *di)
{
	int i;
	int tbatt = 0;

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return;
	huawei_battery_temp(BAT_TEMP_MIXED, &tbatt);
	hwlog_info("[%s] tbatt:%d cur_dc_type:0x%x\n", __func__, tbatt,
		di->mode_para[di->cur_dc_mode].init_para.dc_type);

	for (i = 0; i < di->stage_group_size; i++) {
		if (wldc_is_volt_param_match(di, i))
			break;
	}
	if (i >= di->stage_group_size) {
		for (i = 0; i < di->stage_group_size; i++) {
			if (wldc_is_dflt_volt_param_match(di, i))
				break;
		}
	}
	if ((i >= di->stage_group_size) || (i < 0))
		i = 0;

	di->stage_size = di->orig_volt_para_p[i].stage_size;
	memcpy(di->volt_para, di->orig_volt_para_p[i].volt_info,
		sizeof(di->volt_para));
	memcpy(di->orig_volt_para, di->orig_volt_para_p[i].volt_info,
		sizeof(di->orig_volt_para));

	for (i = 0; i < di->stage_size; i++)
		hwlog_info("[%s] volt_para[%d], vbatt_th:%d icp_th:(%d,%d)\n",
			__func__, i, di->volt_para[i].vbatt_hth,
			di->volt_para[i].cp_iout_hth,
			di->volt_para[i].cp_iout_lth);
}

static void wldc_get_imax_by_tx_ability(struct wldc_dev_info *di)
{
	s64 tmp;
	int rx_ratio;
	struct wireless_protocol_tx_cap *tx_cap = wlc_get_tx_cap();

	if (!tx_cap) {
		hwlog_err("%s: tx_cap null\n", __func__);
		return;
	}
	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;
	tmp = (s64)(tx_cap->vout_max / PERCENT) *
		tx_cap->iout_max * WLDC_TX_PWR_RATIO;
	di->tx_imax = (int)tmp / (WLDC_VBAT_OVP_TH * rx_ratio * di->volt_ratio);
	hwlog_info("[%s] tx_imax = %dmA\n", __func__, di->tx_imax);
}

static int wldc_get_imax_by_tx_plim(struct wldc_dev_info *di)
{
	int plim;
	int vmax;
	int rx_ratio;

	plim = wlc_get_tx_evt_plim();
	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;
	vmax = WLDC_VBAT_OVP_TH * rx_ratio * di->volt_ratio;
	if (vmax > 0)
		return plim / vmax;

	return 0;
}

static int wldc_get_imax_by_pwr_limit(struct wldc_dev_info *di)
{
	int vmax;
	int rx_ratio;

	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;
	vmax = WLDC_VBAT_OVP_TH * rx_ratio * di->volt_ratio;
	if (vmax > 0)
		return di->sysfs_data.pwr_limit / vmax;

	return 0;
}

static void wldc_start_charging(struct wldc_dev_info *di)
{
	wldc_select_group_volt_param(di);
	wldc_set_charge_stage(WLDC_STAGE_CHARGING);
	wldc_send_chrg_mode(di, true);
	wldc_get_imax_by_tx_ability(di);
	mod_delayed_work(system_wq, &di->wldc_ctrl_work,
		msecs_to_jiffies(di->ctrl_interval));
	mod_delayed_work(system_wq, &di->wldc_calc_work,
		msecs_to_jiffies(di->calc_interval));
}

int wldc_prev_check(const char *mode_name)
{
	int ret;
	int i;
	int wired_ch_state;
	struct wldc_dev_info *di = NULL;

	wireless_sc_get_di(&di);
	if (!di) {
		hwlog_err("%s: sc_di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	wldc_set_di(di);

	for (i = 0; i < di->total_dc_mode; i++) {
		if (!strncmp(mode_name, di->mode_para[i].init_para.dc_name,
			strlen(di->mode_para[i].init_para.dc_name)))
			break;
	}
	if ((i >= di->total_dc_mode) || (i < 0))
		return -WLC_ERR_MISMATCH;

	ret = wldc_check_ops_valid(di);
	if (ret) {
		hwlog_err("%s: di ops null\n", __func__);
		return ret;
	}
	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (wired_ch_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired vbus connect\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}
	if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
		hwlog_err("%s: wireless tx disconnect\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}
	if (!di->sysfs_data.enable_charger) {
		hwlog_debug("%s: wireless_sc is disabled\n", __func__);
		return -WLC_ERR_CHECK_FAIL;
	}
	if (wldc_is_forever_forbidden(di, i))
		return -WLC_ERR_CHECK_FAIL;

	if (wldc_is_tmp_forbidden(di, i))
		return -WLC_ERR_CHECK_FAIL;

	return 0;
}

int wldc_formal_check(const char *mode_name)
{
	int i;
	int ret = -WLC_ERR_CHECK_FAIL;
	struct wldc_dev_info *di = NULL;

	if (!g_wldc_stop_complete_flag) {
		hwlog_err("%s: in wldc, ignore\n", __func__);
		return -WLC_ERR_MISMATCH;
	}

	wireless_sc_get_di(&di);
	if (!di) {
		hwlog_err("%s: sc_di null\n", __func__);
		return ret;
	}

	wldc_set_di(di);

	for (i = 0; i < di->total_dc_mode; i++) {
		di->cur_dc_mode = i;
		if (strncmp(mode_name, di->mode_para[i].init_para.dc_name,
			strlen(di->mode_para[i].init_para.dc_name)))
			continue;
		wldc_set_charge_stage(WLDC_STAGE_CHECK);
		g_wldc_stop_complete_flag = false;
		ret =  wireless_sc_charge_check();
		if (!ret) {
			wldc_set_charge_stage(WLDC_STAGE_SUCCESS);
			wldc_start_charging(di);
		}
		return ret;
	}

	return ret;
}

static void wldc_charge_regulation(struct wldc_dev_info *di)
{
	int ret;
	int rx_ratio;
	int rx_vrect;
	int rx_vout;
	int rx_iout;
	int rx_iout_avg;
	int cp_iout;
	int cp_iout_avg;
	int ls_vbus;
	int ls_ibus;
	int ls_vbatt;
	int ls_ibatt;
	int rx_max_iout;
	int rx_vmax;

	if (!wldc_is_mode_para_valid(di, di->cur_dc_mode))
		return;

	rx_vmax = di->mode_para[di->cur_dc_mode].init_para.rx_vmax;
	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;
	rx_max_iout = wlc_get_rx_max_iout();
	rx_vrect = wireless_charge_get_rx_vrect();
	rx_vout = wireless_charge_get_rx_vout();
	rx_iout = wireless_charge_get_rx_iout();
	rx_iout_avg = wireless_charge_get_rx_avg_iout();
	cp_iout = wldc_get_cp_iout();
	cp_iout_avg = wldc_get_cp_avg_iout();
	ls_vbatt = wldc_get_bat_voltage();
	ls_ibatt = wldc_get_bat_current();
	ls_vbus = wldc_get_ls_vbus();
	ls_ibus = wldc_get_ls_ibus();
	hwlog_info("cur_stage=%d, ls: vbus=%d ibus=%d vbatt=%d ibatt=%d,\t"
		"rx: ratio=%d vrect=%d vout=%d iout=%d iout_avg=%d,\t"
		"cp: iout=%d iout_avg=%d, vbat_th=%d,\t"
		"cp_iout_hth=%d, cp_iout_lth=%d, cp_iout_err_th=%d\n",
		di->cur_stage, ls_vbus, ls_ibus, ls_vbatt, ls_ibatt,
		rx_ratio, rx_vrect, rx_vout, rx_iout, rx_iout_avg,
		cp_iout, cp_iout_avg, di->cur_vbat_hth, di->cur_cp_iout_hth,
		di->cur_cp_iout_lth, di->cp_iout_err_hth);
	if (rx_iout > rx_max_iout + WLDC_DFT_IMAX_ERR_TH) {
		di->rx_vout_set -= di->vstep * rx_ratio;
		hwlog_info("[%s] rx_iout:%dmA>%dmA, decrease rx_vout\n",
			__func__, rx_iout, rx_max_iout);
		goto set_vout;
	}
	if (cp_iout_avg > di->cur_cp_iout_hth + di->cp_iout_err_hth) {
		di->rx_vout_set -= di->vstep * rx_ratio;
		hwlog_info("[%s] cp_iout_avg:%dmA>%dmA, decrease rx_vout\n",
			__func__, cp_iout_avg,
			di->cur_cp_iout_hth + di->cp_iout_err_hth);
		goto set_vout;
	}
	if (ls_vbatt > di->cur_vbat_hth) {
		di->rx_vout_set -= rx_ratio * di->volt_ratio *
			(ls_vbatt - di->cur_vbat_hth);
		hwlog_info("[%s] ls_vbatt:%dmV>%dmV, decrease rx_vout\n",
			__func__, ls_vbatt, di->cur_vbat_hth);
		goto set_vout;
	}
	if (ls_ibatt > di->ibat_max_hth) {
		di->rx_vout_set -= di->vstep * rx_ratio;
		hwlog_info("[%s] ls_ibatt:%dmA>%dmA, decrease rx_vout\n",
			__func__, ls_ibatt, di->ibat_max_hth);
		goto set_vout;
	}
	if (cp_iout_avg > di->cur_cp_iout_hth - di->cp_iout_err_hth)
		return;
	if (cp_iout > di->cur_cp_iout_hth - di->cp_iout_err_hth)
		return;
	if (ls_ibatt > di->ibat_min_lth)
		return;
	if (cp_iout_avg < di->cur_cp_iout_hth - di->cp_iout_err_hth) {
		di->rx_vout_set += di->vstep * rx_ratio;
		hwlog_info("[%s] cp_iout_avg:%dmA<%dmA, increase rx_vout\n",
			__func__, cp_iout_avg,
			di->cur_cp_iout_hth - di->cp_iout_err_hth);
		goto set_vout;
	}

set_vout:
	if ((rx_vmax > 0) && (di->rx_vout_set > rx_vmax))
		di->rx_vout_set = rx_vmax;
	ret = wireless_charge_set_rx_vout(di->rx_vout_set);
	if (ret)
		hwlog_err("%s: set_rx_vout fail\n", __func__);
}

static void wldc_select_charge_stage(struct wldc_dev_info *di)
{
	int vbatt;
	int cp_iout;
	int i;
	int cur_stage;
	int vbat_th;

	vbatt = wldc_get_bat_voltage();
	cp_iout = wldc_get_cp_iout();
	di->pre_stage = di->cur_stage;

	for (i = di->stage_size - 1; i >= 0; --i) {
		vbat_th = di->volt_para[i].vbatt_hth;
		if ((vbatt >= vbat_th) &&
			(cp_iout <= di->volt_para[i].cp_iout_lth)) {
			/* cc stage */
			cur_stage =
				WLDC_BATT_STG_TOTAL * (i + 1) + WLDC_CC_STAGE;
			break;
		} else if (vbatt >= vbat_th) {
			/* cv stage */
			cur_stage =
				WLDC_BATT_STG_TOTAL * i + WLDC_CV_STAGE;
			break;
		}
	}
	if (i < 0)
		cur_stage = 0;
	if (cur_stage >= di->pre_stage)
		di->cur_stage = cur_stage;
}

static bool wldc_abnormal_iout_check(struct wldc_dev_info *di)
{
	static int cnt;
	int cp_iout;
	int cp_iout_avg;
	int rx_ratio;

	rx_ratio = di->mode_para[di->cur_dc_mode].init_para.rx_ratio;
	cp_iout = wldc_get_cp_iout();
	cp_iout_avg = wldc_get_cp_avg_iout();
	if ((rx_ratio > WLDC_RATION_MODE) && (cp_iout < WLDC_IOUT_LTH) &&
		(cp_iout_avg < WLDC_IOUT_LTH)) {
		cnt++;
		hwlog_err("cp_iout_avg:%d cp_iout:%d\n", cp_iout_avg, cp_iout);
	} else {
		cnt = 0;
	}

	if (cnt >= WLDC_IOUT_AVG_LOW_CNT) {
		cnt = 0;
		return true;
	}

	return false;
}

static void wldc_check_exit_condition(struct wldc_dev_info *di)
{
	int cur_pmode_id;
	int exp_pmode_id;

	if (!can_tbatt_do_wldc_charge(di)) {
		hwlog_err("%s: tbatt out of range, lth:%d hth:%d\n",
			__func__, di->tbatt_lth, di->tbatt_hth);
		di->stop_flag_info = 1;
		return;
	}
	wireless_charge_update_max_vout_and_iout(true);
	cur_pmode_id = wireless_charge_get_power_mode();
	if (!wireless_charge_mode_judge_criterion(cur_pmode_id,
		WLDC_MODE_FINAL_JUDGE_CRIT)) {
		hwlog_err("%s: pmode judge fail\n", __func__);
		di->stop_flag_info = 1;
		return;
	}
	exp_pmode_id = wlc_get_expected_pmode_id(cur_pmode_id);
	if (wireless_charge_mode_judge_criterion(exp_pmode_id,
		WIRELESS_MODE_FINAL_JUDGE_CRIT)) {
		hwlog_err("%s: switch to expected pmode\n", __func__);
		wlc_set_cur_pmode_id(exp_pmode_id);
		di->stop_flag_info = 1;
		return;
	}
	if (di->ls_ops->is_ls_close()) {
		hwlog_err("%s: ls is close\n", __func__);
		di->stop_flag_warning = 1;
		return;
	}
	if (wldc_abnormal_iout_check(di)) {
		hwlog_err("wldc abnormal iout_avg is low\n");
		di->stop_flag_warning = 1;
		return;
	}
}

static void wldc_select_charge_param(struct wldc_dev_info *di)
{
	int stage_id;
	int tx_evt_ilim;
	int cp_iout_low;
	int rx_max_iout;
	int cp_iin_thermal;
	int i_limit_by_pwr;

	stage_id = di->cur_stage / WLDC_BATT_STG_TOTAL;
	if ((stage_id < 0) || (stage_id >= WLDC_VOLT_LEVEL)) {
		hwlog_err("%s: stage_id %d out of range\n", __func__, stage_id);
		return;
	}
	if ((di->stage_size < 1) || (di->stage_size > WLDC_VOLT_LEVEL)) {
		hwlog_err("%s: stage_size %d out of range\n",
			__func__, di->stage_size);
		return;
	}

	di->cur_vbat_hth = di->volt_para[stage_id].vbatt_hth;
	di->cur_cp_iout_hth = di->volt_para[stage_id].cp_iout_hth;
	di->cur_cp_iout_lth = di->volt_para[stage_id].cp_iout_lth;

	cp_iout_low = di->volt_para[di->stage_size - 1].cp_iout_lth;
	cp_iin_thermal = di->sysfs_data.cp_iin_thermal;
	if (cp_iin_thermal < cp_iout_low)
		cp_iin_thermal = cp_iout_low;

	rx_max_iout = wlc_get_rx_max_iout();
	if (di->cur_cp_iout_hth > rx_max_iout * di->volt_ratio)
		di->cur_cp_iout_hth = rx_max_iout * di->volt_ratio;
	if (di->cur_cp_iout_hth > cp_iin_thermal)
		di->cur_cp_iout_hth = cp_iin_thermal;
	if ((di->tx_imax > 0) &&
		(di->cur_cp_iout_hth > di->tx_imax * di->volt_ratio))
		di->cur_cp_iout_hth = di->tx_imax * di->volt_ratio;

	tx_evt_ilim = wldc_get_imax_by_tx_plim(di);
	if (tx_evt_ilim && (di->cur_cp_iout_hth > tx_evt_ilim))
		di->cur_cp_iout_hth = tx_evt_ilim;

	i_limit_by_pwr = wldc_get_imax_by_pwr_limit(di);
	di->cur_cp_iout_hth = di->cur_cp_iout_hth > i_limit_by_pwr ?
		i_limit_by_pwr : di->cur_cp_iout_hth;
}

void wldc_stop_charging(struct wldc_dev_info *di)
{
	int ret;
	int wired_ch_state;
	int tx_vout_reg;
	int cur_mode;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	hwlog_info("%s: ++\n", __func__);
	cur_mode = di->cur_dc_mode;
	if ((cur_mode < 0) || (cur_mode >= di->total_dc_mode)) {
		hwlog_err("%s: cur_dc_mode(0x%x) out of range [0, 0x%x)\n",
			__func__, cur_mode, di->total_dc_mode);
		wldc_set_charge_stage(WLDC_STAGE_DEFAULT);
		return;
	}
	if (di->stop_flag_error)
		++di->mode_para[cur_mode].err_cnt;
	if (di->stop_flag_warning)
		++di->mode_para[cur_mode].warn_cnt;
	wired_ch_state = wireless_charge_get_wired_channel_state();
	if ((wired_ch_state == WIRED_CHANNEL_ON) ||
		(di->wldc_stage == WLDC_STAGE_STOP_CHARGING)) {
		wldc_set_charge_stage(WLDC_STAGE_STOP_CHARGING);
		wldc_reset_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff));
	} else if (di->stop_flag_error || di->stop_flag_info ||
		di->stop_flag_warning || !di->sysfs_data.enable_charger) {
		wldc_set_charge_stage(WLDC_STAGE_DEFAULT);
	} else {
		wldc_set_charge_stage(WLDC_STAGE_CHARGE_DONE);
		di->mode_para[cur_mode].dis_flag |= WLDC_DIS_BY_CHRG_DONE;
	}
	tx_vout_reg = wireless_charge_get_tx_vout_reg();
	wireless_charge_set_rx_vout(tx_vout_reg); /* in charging plugin otg ovp */
	msleep(15); /* wait for rx_vout fall to 15V */
	ret = di->ls_ops->ls_enable(0);
	if (ret)
		hwlog_err("%s: ls enable fail\n", __func__);
	ret = wldc_retore_normal_charge_path();
	if (ret)
		hwlog_err("%s: retore_normal_charge fail\n", __func__);
	ret = wldc_turn_off_direct_charge_channel();
	if (ret)
		hwlog_err("%s: close wired_reverse_channel fail\n", __func__);
	cancel_delayed_work_sync(&di->wldc_calc_work);
	if (di->wldc_stage != WLDC_STAGE_STOP_CHARGING)
		wireless_charge_restart_charging(WIRELESS_STAGE_REGULATION);
	ret = di->ls_ops->ls_exit();
	if (ret)
		hwlog_err("[%s]: ls exit fail!\n", __func__);
	ret = di->bi_ops->exit();
	if (ret)
		hwlog_err("[%s]: bi exit fail!\n", __func__);
	wldc_extra_power_supply(WLPS_CTRL_OFF);
	wlc_ignore_vbus_only_event(false);
	di->stop_flag_error = 0;
	di->stop_flag_warning = 0;
	di->stop_flag_info = 0;
	di->cur_stage = 0;
	g_wldc_stop_complete_flag = true;
	hwlog_info("%s: --\n", __func__);
}

static void wldc_update_soh_para(struct wldc_dev_info *di)
{
	int i;
	int volt_max;
	int cur_max;
	bool print = false;

	if (!di)
		return;

	volt_max = di->orig_volt_para[di->stage_size - 1].vbatt_hth -
		di->vterm_dec;
	if (di->volt_para[di->stage_size - 1].vbatt_hth != volt_max) {
		di->volt_para[di->stage_size - 1].vbatt_hth = volt_max;
		print = true;
	}

	for (i = 0; i < di->stage_size - 1; i++)
		di->volt_para[i].vbatt_hth =
			(di->orig_volt_para[i].vbatt_hth < volt_max) ?
			di->orig_volt_para[i].vbatt_hth : volt_max;

	if (di->ichg_ratio == 0)
		di->ichg_ratio = BASP_PARA_SCALE;
	cur_max = di->orig_volt_para[0].cp_iout_hth *
		di->ichg_ratio / BASP_PARA_SCALE;
	if (di->volt_para[0].cp_iout_hth != cur_max) {
		di->volt_para[0].cp_iout_hth = cur_max;
		print = true;
	}
	di->volt_para[0].cp_iout_lth =
		di->orig_volt_para[0].cp_iout_lth;

	for (i = 1; i < di->stage_size; i++) {
		di->volt_para[i].cp_iout_hth =
			(di->orig_volt_para[i].cp_iout_hth <= cur_max) ?
			di->orig_volt_para[i].cp_iout_hth : cur_max;
		di->volt_para[i].cp_iout_lth =
			di->orig_volt_para[i].cp_iout_lth;
	}

	if (!print)
		return;

	hwlog_info("dc_volt_dec:%d, dc_cur_ratio:%d\n",
		di->vterm_dec, di->ichg_ratio);
	for (i = 0; i < di->stage_size; i++)
		hwlog_info("volt_para[%d],vbatt_th:%d icp_th:(%d,%d)\n",
			i, di->volt_para[i].vbatt_hth,
			di->volt_para[i].cp_iout_hth,
			di->volt_para[i].cp_iout_lth);
}

static void wldc_update_basp_para(struct wldc_dev_info *di)
{
	int ret;
	int i;
	int cur_level;
	static int last_level = BASP_PARA_LEVEL;
	AGING_SAFE_POLICY_TYPE basp = {0};

	if (!di) {
		hwlog_err("%s: di NULL\n", __func__);
		return;
	}

	ret = hisi_battery_aging_safe_policy(&basp);
	if (ret) {
		wldc_update_soh_para(di);
		return;
	}

	cur_level = basp.level;
	if (cur_level != last_level && di->stage_size >= 1) {
		/*vout policy*/
		di->volt_para[di->stage_size - 1].vbatt_hth =
			di->orig_volt_para[di->stage_size - 1].vbatt_hth
				- basp.dc_volt_dec;
		for (i = 0; i < di->stage_size - 1; i++)
			di->volt_para[i].vbatt_hth =
				(di->orig_volt_para[i].vbatt_hth <
				di->volt_para[di->stage_size - 1].vbatt_hth) ?
				di->orig_volt_para[i].vbatt_hth :
				di->volt_para[di->stage_size - 1].vbatt_hth;
		/*iout policy*/
		di->volt_para[di->stage_size - 1].cp_iout_hth =
			di->orig_volt_para[di->stage_size - 1].cp_iout_hth *
			basp.cur_ratio/BASP_PARA_SCALE;
		di->volt_para[di->stage_size - 1].cp_iout_lth =
			di->orig_volt_para[di->stage_size - 1].cp_iout_lth;
		for (i = 0; i < di->stage_size - 1; i++) {
			di->volt_para[i].cp_iout_hth =
				di->orig_volt_para[i].cp_iout_hth *
				basp.cur_ratio / BASP_PARA_SCALE;
			di->volt_para[i].cp_iout_lth =
				di->orig_volt_para[i].cp_iout_lth *
				basp.cur_ratio / BASP_PARA_SCALE;
		}

		last_level = cur_level;
		hwlog_info(BASP_TAG"cur_level = %d\n", cur_level);
		for (i = 0; i < di->stage_size; i++)
			hwlog_info("volt_para[%d],vbatt_th:%d icp_th:(%d,%d)\n",
			i, di->volt_para[i].vbatt_hth,
			di->volt_para[i].cp_iout_hth,
			di->volt_para[i].cp_iout_lth);
	}
}

static bool wldc_need_stop_work(struct wldc_dev_info *di)
{
	if (di->stop_flag_error || di->stop_flag_info ||
		di->stop_flag_warning || !di->sysfs_data.enable_charger) {
		hwlog_info("%s: stop charging!\n", __func__);
		return true;
	}
	return false;
}

void wldc_calc_work(struct work_struct *work)
{
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (wldc_need_stop_work(di)) {
		hwlog_err("%s: stop calc work!\n", __func__);
		return;
	}
	wldc_check_exit_condition(di);
	wldc_update_basp_para(di);
	wldc_select_charge_stage(di);
	wldc_select_charge_param(di);
	if (di->cur_stage == di->stage_size * WLDC_BATT_STG_TOTAL) {
		hwlog_info("%s: direct charge done\n", __func__);
		return;
	}
	if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
		hwlog_info("%s: tx disconnect\n", __func__);
		return;
	}
	mod_delayed_work(system_wq, &di->wldc_calc_work,
		msecs_to_jiffies(di->calc_interval));
}

void wldc_control_work(struct work_struct *work)
{
	struct wldc_dev_info *di = g_wldc_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (wldc_need_stop_work(di)) {
		wldc_stop_charging(di);
		return;
	}
	if (di->wldc_stage == WLDC_STAGE_STOP_CHARGING) {
		hwlog_info("%s: tx disconnect\n", __func__);
		di->stop_flag_info = 1;
		wldc_stop_charging(di);
		return;
	}
	if (di->cur_stage == di->stage_size * WLDC_BATT_STG_TOTAL) {
		hwlog_info("%s: wireless direct charge done\n", __func__);
		wldc_stop_charging(di);
		return;
	}

	wldc_charge_regulation(di);

	mod_delayed_work(system_wq, &di->wldc_ctrl_work,
		msecs_to_jiffies(di->ctrl_interval));
}

static void wldc_para_reset(struct wldc_dev_info *di)
{
	int i;

	for (i = 0; i < di->total_dc_mode; i++) {
		di->mode_para[i].dis_flag = WLDC_EN;
		di->mode_para[i].dmd_report_flag = false;
		di->mode_para[i].dc_open_retry_cnt = 0;
		di->mode_para[i].err_cnt = 0;
		di->mode_para[i].warn_cnt = 0;
	}
	di->cur_stage = 0;
	di->tbatt_lth = WLDC_TBATT_MIN;
	di->tbatt_hth = WLDC_TBATT_MAX;
	wldc_set_charge_stage(WLDC_STAGE_STOP_CHARGING);
}

void wldc_tx_disconnect_handler(void)
{
	struct wldc_dev_info *sc_di = NULL;

	wireless_sc_get_di(&sc_di);

	if (!sc_di) {
		hwlog_err("sc_di null\n");
		return;
	}
	wldc_para_reset(sc_di);
}

static int wldc_parse_volt_para(struct device_node *np,
		struct wldc_dev_info *di, int group)
{
	int i;
	int row;
	int col;
	int array_len;
	int idata;
	int j = group;
	const char *tmp_str = NULL;
	char *volt_para = NULL;

	volt_para = di->orig_volt_para_p[j].bat_info.volt_para_id;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		volt_para, WLDC_VOLT_LEVEL, WLDC_VOLT_INFO_TOTAL);
	if (array_len < 0)
		return -EINVAL;
	di->orig_volt_para_p[j].stage_size = array_len / WLDC_VOLT_INFO_TOTAL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, volt_para, i, &tmp_str))
			return -EINVAL;

		/* 10: decimal base */
		if (kstrtoint(tmp_str, 10, &idata)) {
			hwlog_err("%s: get kstrtoint fail\n", __func__);
			return -EINVAL;
		}

		row = i / WLDC_VOLT_INFO_TOTAL;
		col = i % WLDC_VOLT_INFO_TOTAL;

		switch (col) {
		case WLDC_PARA_VBATT_HTH:
			di->orig_volt_para_p[j].volt_info[row].vbatt_hth =
				idata;
			break;

		case WLDC_PARA_CP_IOUT_HTH:
			di->orig_volt_para_p[j].volt_info[row].cp_iout_hth =
				idata;
			break;

		case WLDC_PARA_CP_IOUT_LTH:
			di->orig_volt_para_p[j].volt_info[row].cp_iout_lth =
				idata;
			break;
		default:
			break;
		}
	}

	di->orig_volt_para_p[j].bat_info.parse_ok = 1;

	for (i = 0; i < di->orig_volt_para_p[j].stage_size; i++) {
		hwlog_info("%s[%d] vbatt_th:%dmV icp_th:(%d,%d)mA\n", volt_para,
			i, di->orig_volt_para_p[j].volt_info[i].vbatt_hth,
			di->orig_volt_para_p[j].volt_info[i].cp_iout_hth,
			di->orig_volt_para_p[j].volt_info[i].cp_iout_lth);
	}

	return 0;
}

static void wldc_parse_group_volt_para(struct device_node *np,
		struct wldc_dev_info *di)
{
	int i;

	di->stage_size = 0;

	for (i = 0; i < di->stage_group_size; i++) {
		if (wldc_parse_volt_para(np, di, i))
			return;
	}

	di->stage_size = di->orig_volt_para_p[0].stage_size;
}

static void wldc_parse_batt_para(struct device_node *np,
	struct wldc_dev_info *di)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_str = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"bat_para", WLDC_VOLT_GROUP_MAX, WLDC_BAT_INFO_TOTAL);
	if (array_len < 0) {
		di->stage_group_size = 1;
		di->orig_volt_para_p[0].bat_info.tbatt_hth = WLDC_TBATT_MAX;
		di->orig_volt_para_p[0].bat_info.tbatt_lth = WLDC_TBATT_MIN;
		strncpy(di->orig_volt_para_p[0].bat_info.batid,
			"default", WLDC_BAT_BRAND_LEN_MAX - 1);
		strncpy(di->orig_volt_para_p[0].bat_info.volt_para_id,
			"volt_para00", WLDC_VOLT_NODE_LEN_MAX - 1);
		return;
	}

	di->stage_group_size = array_len / WLDC_BAT_INFO_TOTAL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "bat_para", i, &tmp_str))
			return;

		row = i / WLDC_BAT_INFO_TOTAL;
		col = i % WLDC_BAT_INFO_TOTAL;

		switch (col) {
		case WLDC_BAT_ID:
			strncpy(di->orig_volt_para_p[row].bat_info.batid,
				tmp_str, WLDC_BAT_BRAND_LEN_MAX - 1);
			break;

		case WLDC_BAT_TEMP_LOW:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret) {
				hwlog_err("%s: kstrtoint tbatt_lth fail\n",
					__func__);
				return;
			}
			if ((idata < WLDC_TBATT_MIN) ||
				(idata > WLDC_TBATT_MAX)) {
				hwlog_err("%s:tbatt_low(%d) invalid\n",
					__func__, idata);
				return;
			}
			di->orig_volt_para_p[row].bat_info.tbatt_lth = idata;
			break;

		case WLDC_BAT_TEMP_HIGH:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret) {
				hwlog_err("%s: kstrtoint tbatt_hth fail\n",
					__func__);
				return;
			}
			if ((idata < WLDC_TBATT_MIN) ||
				(idata > WLDC_TBATT_MAX)) {
				hwlog_err("%s:tbatt_high(%d) invalid\n",
					__func__, idata);
				return;
			}
			di->orig_volt_para_p[row].bat_info.tbatt_hth = idata;
			break;

		case WLDC_BAT_DC_TYPE:
			ret = kstrtoint(tmp_str, WLC_HEXADECIMAL, &idata);
			if (ret) {
				hwlog_err("%s: kstrtoint dc_type fail\n",
					__func__);
				return;
			}
			if ((idata < WLC_RX_SP_BUCK_MODE) ||
				(idata > WLC_RX_SP_ALL_MODE)) {
				hwlog_err("%s:dc_type(%d) invalid\n",
					__func__, idata);
				return;
			}
			di->orig_volt_para_p[row].bat_info.dc_type = idata;
			break;
		case WLDC_BAT_PARA_INDEX:
			strncpy(di->orig_volt_para_p[row].bat_info.volt_para_id,
				tmp_str, WLDC_VOLT_NODE_LEN_MAX - 1);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < di->stage_group_size; i++)
		hwlog_info("bat_para[%d]=%d %d 0x%x %s\n", i,
			di->orig_volt_para_p[i].bat_info.tbatt_lth,
			di->orig_volt_para_p[i].bat_info.tbatt_hth,
			di->orig_volt_para_p[i].bat_info.dc_type,
			di->orig_volt_para_p[i].bat_info.volt_para_id);
}

static void wldc_parse_mode_para(struct device_node *np,
	struct wldc_dev_info *di)
{
	int i;
	int ret;
	int array_len;
	int idata;
	int row;
	int col;
	size_t size;
	const char *tmp_str = NULL;
	char *tmp_p = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"init_para", WLDC_MODE_MAX, WLDC_INIT_INFO_TOTAL);
	if (array_len < 0)
		goto para_err;

	di->total_dc_mode = array_len / WLDC_INIT_INFO_TOTAL;
	size = sizeof(*(di->mode_para)) * di->total_dc_mode;
	di->mode_para =  kzalloc(size, GFP_KERNEL);
	if (!di->mode_para) {
		di->total_dc_mode = 0;
		hwlog_err("%s: alloc wldc_mode_para fail\n", __func__);
		goto alloc_err;
	}

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "init_para", i, &tmp_str)) {
			di->total_dc_mode = 0;
			goto parse_err;
		}

		row = i / WLDC_INIT_INFO_TOTAL;
		col = i % WLDC_INIT_INFO_TOTAL;
		switch (col) {
		case WLDC_TYPE:
			ret = kstrtoint(tmp_str, WLC_HEXADECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.dc_type = idata;
			break;
		case WLDC_NAME:
			tmp_p = strncpy(di->mode_para[row].init_para.dc_name,
				tmp_str, WLDC_TYPE_LEN_MAX - 1);
			if (!tmp_p)
				goto parse_err;
			break;
		case WLDC_EXT_PWR_TYPE:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.ext_pwr_type =
				(enum wldc_ext_pwr_type)idata;
			break;
		case WLDC_RX_RATIO:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.rx_ratio = idata;
			break;
		case WLDC_VBATT_MIN:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.vbatt_min = idata;
			break;
		case WLDC_VBATT_MAX:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.vbatt_max = idata;
			break;
		case WLDC_RX_VOUT:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.rx_vout = idata;
			break;
		case WLDC_RX_VOUT_TH:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.rx_vout_th = idata;
			break;
		case WLDC_VDIFF_TH:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.vdiff_th = idata;
			break;
		case WLDC_ILEAK_TH:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.ileak_th = idata;
			break;
		case WLDC_VDELT:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.vdelt = idata;
			break;
		case WLDC_RX_VMAX:
			ret = kstrtoint(tmp_str, WLC_DECIMAL, &idata);
			if (ret)
				goto parse_err;
			di->mode_para[row].init_para.rx_vmax = idata;
			break;
		default:
			goto parse_err;
		}
	}

	for (i = 0; i < di->total_dc_mode; i++) {
		di->mode_para[i].dis_flag = WLDC_EN;
		hwlog_info("mode_para[%d] type:0x%x name:%s ext_pwr_type:0x%x\t"
			"ratio:%d vbatt_min:%-4d vbatt_max:%-4d rx_vout:%-5d\t"
			"rx_vout_th:%-3d vdiff_th:%-4d ileak_th:%-3d\t"
			"vdelt:%-3d rx_vmax:%-3d dis:%d\n", i,
			di->mode_para[i].init_para.dc_type,
			di->mode_para[i].init_para.dc_name,
			di->mode_para[i].init_para.ext_pwr_type,
			di->mode_para[i].init_para.rx_ratio,
			di->mode_para[i].init_para.vbatt_min,
			di->mode_para[i].init_para.vbatt_max,
			di->mode_para[i].init_para.rx_vout,
			di->mode_para[i].init_para.rx_vout_th,
			di->mode_para[i].init_para.vdiff_th,
			di->mode_para[i].init_para.ileak_th,
			di->mode_para[i].init_para.vdelt,
			di->mode_para[i].init_para.rx_vmax,
			di->mode_para[i].dis_flag);
	}

	return;

parse_err:
	kfree(di->mode_para);
	di->mode_para = NULL;
alloc_err:
para_err:
	di->total_dc_mode = 0;
}

static void wldc_parse_basic_para(struct device_node *np,
	struct wldc_dev_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "need_wired_sw_on",
		&g_wldc_need_wired_sw_on);
	if (ret)
		g_wldc_need_wired_sw_on = 1;
	hwlog_info("%s: g_wldc_need_wired_sw_on = %d\n",
		__func__, g_wldc_need_wired_sw_on);

	ret = of_property_read_u32(np, "cp_iout_err_hth",
		&di->cp_iout_err_hth);
	if (ret)
		di->cp_iout_err_hth = WLDC_DEFAULT_IOUT_ERR_TH;
	hwlog_info("%s: cp_iout_err_hth  = %dmA\n",
		__func__, di->cp_iout_err_hth);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibat_max_hth", &di->ibat_max_hth,
		WLDC_DEFAULT_IBAT_MAX_TH);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibat_min_lth", &di->ibat_min_lth,
		WLDC_DEFAULT_IBAT_MIN_TH);

	ret = of_property_read_u32(np, "volt_ratio", &di->volt_ratio);
	if (ret)
		di->volt_ratio = SC_DEFAULT_VOLT_RATIO;
	hwlog_info("%s: sc_volt_ratio  = %d\n", __func__, di->volt_ratio);

	ret = of_property_read_u32(np, "ctrl_interval", &di->ctrl_interval);
	if (ret)
		di->ctrl_interval = WLDC_DEFAULT_CTRL_INTERVAL;
	hwlog_info("%s: ctrl_interval  = %dms\n", __func__, di->ctrl_interval);

	ret = of_property_read_u32(np, "calc_interval", &di->calc_interval);
	if (ret)
		di->calc_interval = WLDC_DEFAULT_CALC_INTERVAL;
	hwlog_info("%s: calc_interval  = %dms\n", __func__, di->calc_interval);

	ret = of_property_read_u32(np, "vstep", &di->vstep);
	if (ret)
		di->vstep = WLDC_DEFAULT_VSTEP;
	hwlog_info("%s: vstep  = %dmV\n", __func__, di->vstep);
	di->tbatt_lth = WLDC_TBATT_MIN;
	di->tbatt_hth = WLDC_TBATT_MAX;
}

void wldc_parse_dts(struct device_node *np, struct wldc_dev_info *di)
{
	wldc_parse_basic_para(np, di);
	wldc_parse_mode_para(np, di);
	wldc_parse_batt_para(np, di);
	wldc_parse_group_volt_para(np, di);
}
