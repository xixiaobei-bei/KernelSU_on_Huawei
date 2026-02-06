/*
 * wireless_transmitter_aux.c
 *
 * wireless aux tx reverse charging
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter_aux.h>
#include <huawei_platform/power/wireless/wireless_tx_pwm.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/wireless_charge/wireless_accessory.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/jiffies.h>
#include <../../charging_core.h>

#define HWLOG_TAG wireless_aux_tx
HWLOG_REGIST();

static struct wakeup_source g_wltx_aux_wakelock;
static struct wltx_aux_dev_info *g_wltx_aux_di;
static struct wltx_dev_ops *g_wltx_aux_ops;
static enum wireless_tx_stage g_tx_stage;
static enum wireless_tx_status_type g_tx_status;
static unsigned int g_tx_iin_samples[WL_TX_IIN_SAMPLE_LEN];
static bool g_tx_open_flag; /* record the UI operation state */
static int g_init_tbatt;
static bool g_hall_state;

static struct wltx_stage_info {
	enum wireless_tx_stage tx_stg;
	char tx_stage_name[WL_TX_STR_LEN_32];
} const g_tx_stg[WL_TX_STAGE_TOTAL] = {
	{ WL_TX_STAGE_DEFAULT, "STAGE_DEFAULT" },
	{ WL_TX_STAGE_POWER_SUPPLY, "STAGE_POWER_SUPPLY" },
	{ WL_TX_STAGE_CHIP_INIT, "STAGE_CHIP_INIT" },
	{ WL_TX_STAGE_PING_RX, "STAGE_PING_RX" },
	{ WL_TX_STAGE_REGULATION, "STAGE_REGULATION" },
};

static int wltx_aux_set_tx_volt(struct wltx_aux_dev_info *di, int vset, bool force)
{
	int ret;

	if (!force && (vset == di->tx_vset.cur_vset))
		return 0;

	if (di->pwm_support)
		ret = wltx_pwm_set_volt(WLTRX_IC_AUX, vset);
	else if (di->tx_ops && di->tx_ops->tx_adjust_vin)
		ret = di->tx_ops->tx_adjust_vin(di->tx_iin_avg);
	else
		ret = wltx_ic_set_vset(WLTRX_IC_AUX, vset);

	if (!ret)
		di->tx_vset.cur_vset = vset;
	return ret;
}

static int wltx_aux_ps_tx_volt_check(struct wltx_aux_dev_info *di)
{
	int i;
	int vset = di->tx_vset.cur_vset;

	for (i = 0; i < di->vset_cfg.vset_level; i++) {
		if ((di->tx_iin_avg >= di->vset_cfg.vset_para[i].iin_min) &&
			(di->tx_iin_avg <= di->vset_cfg.vset_para[i].iin_max)) {
			vset = di->vset_cfg.vset_para[i].vset;
			break;
		}
	}

	return wltx_aux_set_tx_volt(di, vset, false);
}

int wltx_aux_ops_register(struct wltx_dev_ops *tx_ops)
{
	int ret = 0;

	if (tx_ops) {
		g_wltx_aux_ops = tx_ops;
	} else {
		hwlog_err("wireless tx aux ops register fail\n");
		ret = -EPERM;
	}
	return ret;
}

int wltx_aux_get_tx_status(void)
{
	return g_tx_status;
}

bool wltx_aux_get_tx_open_flag(void)
{
	return g_tx_open_flag;
}

static void wltx_aux_wake_lock(void)
{
	if (!g_wltx_aux_wakelock.active) {
		__pm_stay_awake(&g_wltx_aux_wakelock);
		hwlog_info("%s:wake lock\n", __func__);
	}
}

static void wltx_aux_wake_unlock(void)
{
	if (g_wltx_aux_wakelock.active) {
		__pm_relax(&g_wltx_aux_wakelock);
		hwlog_info("%s:wake unlock\n", __func__);
	}
}

static void wltx_aux_set_tx_open_flag(bool enable)
{
	g_tx_open_flag = enable;
	hwlog_info("%s:set tx_open_flag = %d\n", __func__, g_tx_open_flag);
}

static void wltx_aux_set_stage(enum wireless_tx_stage stage)
{
	g_tx_stage = stage;
	hwlog_info("%s: %s\n", __func__, g_tx_stg[g_tx_stage].tx_stage_name);
}

static enum wireless_tx_stage wltx_aux_get_stage(void)
{
	return g_tx_stage;
}

static void wltx_aux_set_tx_status(enum wireless_tx_status_type event)
{
	g_tx_status = event;
	hwlog_info("%s: 0x%02x\n", __func__, g_tx_status);
}

static void wltx_aux_calc_tx_iin_avg(struct wltx_aux_dev_info *di,
	unsigned int tx_iin)
{
	static int index;
	int iin_sum = 0;
	int i;

	g_tx_iin_samples[index] = tx_iin;
	index = (index + 1) % WL_TX_IIN_SAMPLE_LEN;
	for (i = 0; i < WL_TX_IIN_SAMPLE_LEN; i++)
		iin_sum += g_tx_iin_samples[i];
	di->tx_iin_avg = iin_sum / WL_TX_IIN_SAMPLE_LEN;
}

static void wltx_aux_reset_avg_iout(struct wltx_aux_dev_info *di)
{
	int i;

	for (i = 0; i < WL_TX_IIN_SAMPLE_LEN; i++)
		g_tx_iin_samples[i] = 0;
	di->tx_iin_avg = 0;
}

static int wltx_check_handshake(struct wltx_aux_dev_info *di)
{
	if ((g_tx_status < WL_TX_STATUS_PING_SUCC) ||
		(g_tx_status >= WL_TX_STATUS_FAULT_BASE))
		return -1;

	return 0;
}

static void wltx_check_reverse_timeout(struct wltx_aux_dev_info *di)
{
	if (++di->tx_reverse_timeout_cnt >=
		(WL_TX_REVERSE_TIMEOUT_CNT / di->monitor_interval)) {
		wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		hwlog_info("%s:tx reverse timeout cnt %d over %ds, charge_stop\n",
			__func__, di->tx_reverse_timeout_cnt,
			WL_TX_REVERSE_TIMEOUT_CNT / WL_MSEC_PER_SEC);
	}
}

static int wltx_aux_chip_reset(struct wltx_aux_dev_info *di)
{
	if (!di->tx_ops) {
		wltx_ic_chip_reset(WLTRX_IC_AUX);
		return 0;
	}
	return di->tx_ops->chip_reset();
}

static void wltx_aux_set_max_fop(struct wltx_aux_dev_info *di,
	u16 fop)
{
	if (!di->tx_ops) {
		wltx_ic_set_max_fop(WLTRX_IC_AUX, fop);
		return;
	}
	di->tx_ops->set_tx_max_fop(fop);
}

static void wltx_aux_get_max_fop(struct wltx_aux_dev_info *di,
	u16 *fop)
{
	if (!di->tx_ops) {
		wltx_ic_get_max_fop(WLTRX_IC_AUX, fop);
		return;
	}
	di->tx_ops->get_tx_max_fop(fop);
}

static int wltx_aux_set_min_fop(struct wltx_aux_dev_info *di,
	u16 fop)
{
	if (!di->tx_ops)
		return wltx_ic_set_min_fop(WLTRX_IC_AUX, fop);
	return di->tx_ops->set_tx_min_fop(fop);
}

static void wltx_aux_get_min_fop(struct wltx_aux_dev_info *di,
	u16 *fop)
{
	if (!di->tx_ops) {
		wltx_ic_get_min_fop(WLTRX_IC_AUX, fop);
		return;
	}
	di->tx_ops->get_tx_min_fop(fop);
}

static void wltx_aux_get_fop(struct wltx_aux_dev_info *di, u16 *fop)
{
	if (!di->tx_ops) {
		wltx_ic_get_fop(WLTRX_IC_AUX, fop);
		return;
	}
	di->tx_ops->get_tx_fop(fop);
}

static void wltx_aux_get_digital_ping_frequency(struct wltx_aux_dev_info *di,
	u16 *dping_freq)
{
	if (!di->tx_ops) {
		wltx_ic_get_ping_freq(WLTRX_IC_AUX, dping_freq);
		return;
	}
	di->tx_ops->get_tx_ping_frequency(dping_freq);
}

static void wltx_aux_set_digital_ping_frequency(struct wltx_aux_dev_info *di,
	u16 dping_freq)
{
	if (!di->tx_ops) {
		wltx_ic_set_ping_freq(WLTRX_IC_AUX, dping_freq);
		return;
	}
	di->tx_ops->set_tx_ping_frequency(dping_freq);
}

static void wltx_aux_get_digital_ping_interval(struct wltx_aux_dev_info *di,
	u16 *dping_interval)
{
	if (!di->tx_ops) {
		wltx_ic_get_ping_interval(WLTRX_IC_AUX, dping_interval);
		return;
	}
	di->tx_ops->get_tx_ping_interval(dping_interval);
}

static void wltx_aux_set_digital_ping_interval(struct wltx_aux_dev_info *di,
	u16 dping_interval)
{
	if (!di->tx_ops) {
		wltx_ic_set_ping_interval(WLTRX_IC_AUX, dping_interval);
		return;
	}
	di->tx_ops->set_tx_ping_interval(dping_interval);
}

static void wltx_aux_check_fwupdate(struct wltx_aux_dev_info *di)
{
	if (!di->tx_ops)
		return;

	di->tx_ops->check_fwupdate(WIRELESS_TX_MODE);
}

static void wltx_aux_get_tx_vin(struct wltx_aux_dev_info *di,
	u16 *tx_vin)
{
	if (!di->tx_ops) {
		wltx_ic_get_vin(WLTRX_IC_AUX, tx_vin);
		return;
	}
	di->tx_ops->get_tx_vin(tx_vin);
}

static void wltx_aux_get_tx_iin(struct wltx_aux_dev_info *di,
	u16 *tx_iin)
{
	if (!di->tx_ops) {
		wltx_ic_get_iin(WLTRX_IC_AUX, tx_iin);
		return;
	}
	di->tx_ops->get_tx_iin(tx_iin);
}

static void wltx_aux_dsm_dump(struct wltx_aux_dev_info *di,
	char *dsm_buff)
{
	int ret;
	int i;
	char buff[ERR_NO_STRING_SIZE] = { 0 };
	u16 tx_iin = 0;
	u16 tx_vin = 0;
	u16 tx_vrect = 0;
	s16 chip_temp = 0;

	if (!di->tx_ops) {
		ret = wltx_ic_get_iin(WLTRX_IC_AUX, &tx_iin);
		ret |= wltx_ic_get_vin(WLTRX_IC_AUX, &tx_vin);
		ret |= wltx_ic_get_vrect(WLTRX_IC_AUX, &tx_vrect);
		ret |= wltx_ic_get_temp(WLTRX_IC_AUX, &chip_temp);
	} else {
		ret = di->tx_ops->get_tx_iin(&tx_iin);
		ret |= di->tx_ops->get_tx_vin(&tx_vin);
		ret |= di->tx_ops->get_tx_vrect(&tx_vrect);
		ret |= di->tx_ops->get_chip_temp((u8 *)&chip_temp);
	}
	if (ret)
		hwlog_err("%s: get tx vin/iin/vrect/temp fail", __func__);
	snprintf(buff, sizeof(buff),
		"init_tbatt = %d, tx_vrect = %umV, tx_vin = %umV, tx_iin = %umA, tx_iin_avg = %dmA, chip_temp = %d\n",
		g_init_tbatt, tx_vrect, tx_vin, tx_iin, di->tx_iin_avg,
		chip_temp);
	strncat(dsm_buff, buff, strlen(buff));
	snprintf(buff, ERR_NO_STRING_SIZE, "tx_iin(mA): ");
	strncat(dsm_buff, buff, strlen(buff));
	for (i = 0; i < WL_TX_IIN_SAMPLE_LEN; i++) {
		snprintf(buff, ERR_NO_STRING_SIZE, "%d ", g_tx_iin_samples[i]);
		strncat(dsm_buff, buff, strlen(buff));
	}
}

static void wltx_aux_dsm_report(int err_no, char *dsm_buff)
{
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	if (di) {
		wltx_aux_dsm_dump(di, dsm_buff);
		power_dsm_report_dmd(POWER_DSM_BATTERY, err_no, dsm_buff);
	}
}

static void wltx_aux_check_rx_disconnect(struct wltx_aux_dev_info *di)
{
	bool rx_disconnect = false;

	if (!di->tx_ops)
		rx_disconnect = wltx_ic_is_rx_disconnect(WLTRX_IC_AUX);
	else
		rx_disconnect = di->tx_ops->check_rx_disconnect();

	if (rx_disconnect) {
		hwlog_info("%s: rx disconnect\n", __func__);
		wltx_aux_set_tx_status(WL_TX_STATUS_RX_DISCONNECT);
		di->stop_reverse_charge = true;
	}
}

static void wltx_aux_en_enable(struct wltx_aux_dev_info *di, bool enable)
{
	if (!di->tx_ops) {
		wltx_ic_chip_enable(WLTRX_IC_AUX, enable);
		return;
	}
	di->tx_ops->rx_enable(enable);
}

static void wltx_aux_enable_tx_mode(struct wltx_aux_dev_info *di, bool enable)
{
	/* chip en enable */
	wltx_aux_en_enable(di, enable);
	/* chip tx enable */
	if (enable) {
		if (!di->tx_ops)
			wltx_ic_mode_enable(WLTRX_IC_AUX, enable);
		else
			di->tx_ops->enable_tx_mode(enable);
	}
	hwlog_info("%s: enable = %d\n", __func__, enable);
}

static void wltx_aux_enable_power(bool enable)
{
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	hwlog_info("%s:gpio_tx_boost_en set to %d\n", __func__, enable);
	gpio_set_value(di->gpio_tx_boost_en, enable);
	if (di->pwm_support && !enable)
		wltx_pwm_close(WLTRX_IC_AUX);
	if (enable)
		wltx_ic_activate_chip(WLTRX_IC_AUX);
}

static int wltx_aux_power_supply(struct wltx_aux_dev_info *di)
{
	int ret;
	int count = 0;
	u16 tx_vin = 0;
	int charger_vbus;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	wltx_aux_enable_power(true);
	if (di->pwm_support)
		(void)wltx_aux_set_tx_volt(di, di->tx_vset.v_ps, true);

	do {
		msleep(WL_TX_VIN_SLEEP_TIME);
		wltx_aux_en_enable(di, true);
		if (!di->tx_ops)
			ret = wltx_ic_get_vin(WLTRX_IC_AUX, &tx_vin);
		else
			ret = di->tx_ops->get_tx_vin(&tx_vin);
		if (ret) {
			hwlog_info("%s: get tx_vin failed\n", __func__);
			tx_vin = 0;
		}
		charger_vbus = charge_get_vbus();
		if (tx_vin >= di->tx_vset.para[di->tx_vset.cur].lth &&
			tx_vin <= di->tx_vset.para[di->tx_vset.cur].hth) {
			hwlog_info("%s: tx_vin = %dmV, power supply succ\n",
				__func__, tx_vin);
			wltx_aux_en_enable(di, true);
			return WL_TX_SUCC;
		}
		if (!g_tx_open_flag || di->stop_reverse_charge) {
			hwlog_err("%s:tx flag = %d, stop reverse flag = %d\n",
				__func__, g_tx_open_flag,
				di->stop_reverse_charge);
			return WL_TX_FAIL;
		}
		count++;
		hwlog_info("%s:tx_vin = %dmV, vbus = %dmV, retry = %d\n",
			__func__, tx_vin, charger_vbus, count);
	} while (count < WL_TX_VIN_RETRY_CNT2);

	hwlog_err("%s: power supply for TX fail\n", __func__);
	wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
	wltx_aux_dsm_report(ERROR_WIRELESS_TX_POWER_SUPPLY_FAIL, dsm_buff);

	return WL_TX_FAIL;
}

static int wltx_aux_limit_iout(struct wltx_aux_dev_info *di)
{
	return 0;
}

static void wltx_aux_para_init(struct wltx_aux_dev_info *di)
{
	di->stop_reverse_charge = false;
	di->i2c_err_cnt = 0;
	di->tx_reverse_timeout_cnt = 0;
	di->tx_mode_err_cnt = 0;
	di->standard_rx = false;
	di->tx_vset.cur = 0;
	di->tx_vset.cur_vset = 0;
	di->monitor_interval = WL_TX_MONITOR_INTERVAL;
	wltx_aux_reset_avg_iout(di);
}

static int wltx_aux_chip_init(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!g_tx_open_flag || di->stop_reverse_charge) {
		hwlog_err("%s:tx open flag = %d, stop reverse charge = %d\n",
			__func__, g_tx_open_flag, di->stop_reverse_charge);
		return WL_TX_FAIL;
	}

	wltx_aux_check_fwupdate(di);
	if (!di->tx_ops) {
		ret = wltx_ic_chip_init(WLTRX_IC_AUX, 0);
		ret += wltx_ic_set_fod_coef(WLTRX_IC_AUX,
			di->tx_vset.para[di->tx_vset.cur].pl_th,
			di->tx_vset.para[di->tx_vset.cur].pl_cnt);
	} else {
		ret = di->tx_ops->tx_chip_init();
	}
	if (ret) {
		hwlog_err("%s: TX chip init fail\n", __func__);
		return WL_TX_FAIL;
	}
	ret = wltx_aux_limit_iout(di);
	if (ret) {
		hwlog_err("%s: limit TX iout fail\n", __func__);
		return WL_TX_FAIL;
	}
	hwlog_info("%s: TX chip init succ\n", __func__);
	return WL_TX_SUCC;
}

static void wltx_aux_check_in_tx_mode(struct wltx_aux_dev_info *di)
{
	int ret;
	bool in_tx_mode = false;

	if (!di->tx_ops)
		in_tx_mode = wltx_ic_is_in_tx_mode(WLTRX_IC_AUX);
	else
		in_tx_mode = di->tx_ops->in_tx_mode();

	if (!in_tx_mode) {
		if (++di->tx_mode_err_cnt >= WL_TX_MODE_ERR_CNT2) {
			hwlog_err("%s: not in tx mode, close TX\n", __func__);
			wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
			di->stop_reverse_charge = true;
		} else if (di->tx_mode_err_cnt >= WL_TX_MODE_ERR_CNT) {
			hwlog_err("%s: not in tx mode, reinit TX\n", __func__);
			ret = wltx_aux_chip_init(di);
			if (ret)
				hwlog_err("%s: chip_init fail\n", __func__);
			wltx_aux_enable_tx_mode(di, true);
		}
	} else {
		di->tx_mode_err_cnt = 0;
	}
}

static int wltx_aux_ping_rx(struct wltx_aux_dev_info *di)
{
	u16 tx_vin = 0;
	int ret;
	bool tx_vin_uvp_flag = false;
	int tx_vin_uvp_cnt = 0;
	struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = di->ping_timeout;
	ts64_interval.tv_nsec = 0;

	if (di->ping_timeout == di->ping_timeout_2) {
		ret = wltx_aux_chip_reset(di);
		if (ret)
			hwlog_err("%s: chip_reset fail\n", __func__);
		msleep(150); /* delay 150ms */
		ret = wltx_aux_chip_init(di);
		if (ret)
			hwlog_err("%s: chip_init fail\n", __func__);
	}
	/* enable_tx_ping */
	if (di && di->tx_ops && di->tx_ops->enable_tx_ping)
		di->tx_ops->enable_tx_ping();
	wltx_aux_enable_tx_mode(di, true);

	ts64_timeout = timespec64_add_safe(ts64_now, ts64_interval);
	if (ts64_timeout.tv_sec == TIME_T_MAX) {
		hwlog_err("%s: time overflow happend, TX ping RX fail\n",
			__func__);
		return WL_TX_FAIL;
	}

	wltx_aux_set_tx_status(WL_TX_STATUS_PING);
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	while (timespec64_compare(&ts64_now, &ts64_timeout) < 0) {
		/* wait for config packet interrupt */
		if (wltx_aux_get_tx_status() == WL_TX_STATUS_PING_SUCC)
			return WL_TX_SUCC;
		if (!di->tx_ops)
			ret =  wltx_ic_get_vin(WLTRX_IC_AUX, &tx_vin);
		else
			ret = di->tx_ops->get_tx_vin(&tx_vin);
		if (ret) {
			hwlog_err("%s: get tx_vin fail\n", __func__);
			wltx_aux_enable_tx_mode(di, true);
			tx_vin = 0;
		} else if ((tx_vin < di->tx_vset.para[di->tx_vset.cur].lth) ||
			(tx_vin >= di->tx_vset.para[di->tx_vset.cur].hth)) {
			hwlog_err("%s: tx_vin = %umV\n", __func__, tx_vin);
		}
		/* to solve the problem of tx reset when power_supply ocp/scp
		 * in case of putting tx on the metal or something like this
		 */
		if (tx_vin < di->tx_vset.para[di->tx_vset.cur].lth) {
			tx_vin_uvp_flag = true;
		} else if (tx_vin >= di->tx_vset.para[di->tx_vset.cur].lth) {
			if (tx_vin_uvp_flag &&
				(++tx_vin_uvp_cnt <= WL_TX_PING_VIN_UVP_CNT)) {
				hwlog_err("%s: tx vin uvp cnt = %d\n",
					__func__, tx_vin_uvp_cnt);
				ret = wltx_aux_chip_init(di);
				if (ret) {
					hwlog_err("%s: tx_chip_init fail\n",
						__func__);
					wltx_aux_set_tx_status(
						WL_TX_STATUS_TX_CLOSE);
					return WL_TX_FAIL;
				}
				wltx_aux_enable_tx_mode(di, true);
			}
			tx_vin_uvp_flag = false;
		}
		if (tx_vin_uvp_cnt >= WL_TX_PING_VIN_UVP_CNT) {
			hwlog_err("%s:uvp over %d times,tx ping fail\n",
				__func__, WL_TX_PING_VIN_UVP_CNT);
			wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
			return WL_TX_FAIL;
		}

		msleep(WL_TX_PING_CHECK_INTERVAL);
		ts64_now = current_kernel_time64();

		if (!g_tx_open_flag || di->stop_reverse_charge ||
			!g_hall_state) {
			hwlog_err("%s: tx flag=%d,stop_reverse_charge=%d,g_hall_state=%d\n",
				__func__, g_tx_open_flag, di->stop_reverse_charge, g_hall_state);
			return WL_TX_FAIL;
		}
	}
	wltx_aux_set_tx_open_flag(false);
	wltx_aux_set_tx_status(WL_TX_STATUS_PING_TIMEOUT);
	hwlog_info("%s: TX ping RX timeout\n", __func__);

	return WL_TX_FAIL;
}

static int wltx_aux_can_do_reverse_charging(void)
{
	int batt_temp = coul_drv_battery_temperature();
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	if (batt_temp <= WL_TX_BATT_TEMP_MIN) {
		hwlog_err("%s: battery temperature %d is too low,th: %d\n",
			__func__, batt_temp, WL_TX_BATT_TEMP_MIN);
		wltx_aux_set_tx_status(WL_TX_STATUS_TBATT_LOW);
		return WL_TX_FAIL;
	}
	if (batt_temp >= WL_TX_BATT_TEMP_MAX) {
		hwlog_err("%s: battery temperature %d is too high,th: %d\n",
			__func__, batt_temp, WL_TX_BATT_TEMP_MAX);
		wltx_aux_set_tx_status(WL_TX_STATUS_TBATT_HIGH);
		if (batt_temp - g_init_tbatt > WL_TX_TBATT_DELTA_TH)
			wltx_aux_dsm_report(
				ERROR_WIRELESS_TX_BATTERY_OVERHEAT, dsm_buff);
		return WL_TX_FAIL;
	}

	return WL_TX_SUCC;
}

static int wltx_aux_set_acc_dev_state(struct wltx_aux_dev_info *di,
	u8 dev_state)
{
	if (!di->tx_ops)
		wireless_acc_set_tx_dev_state(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			dev_state);
	else
		di->tx_ops->set_tx_acc_dev_state(dev_state);
	return 0;
}

static int wltx_aux_set_acc_dev_info_cnt(struct wltx_aux_dev_info *di,
	u8 dev_info_cnt)
{
	if (!di->tx_ops)
		wireless_acc_set_tx_dev_info_cnt(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			dev_info_cnt);
	else
		di->tx_ops->set_tx_acc_dev_info_cnt(dev_info_cnt);
	return 0;
}

static int wltx_aux_get_acc_dev_info_cnt(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_info_cnt(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			&di->wireless_tx_acc->dev_info_cnt);
	else
		ret = di->tx_ops->get_tx_acc_dev_info_cnt(
			&di->wireless_tx_acc->dev_info_cnt);
	if (ret) {
		hwlog_err("%s: get_tx_acc_dev_info_cnt fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: get wireless_tx_acc dev_info_cnt = %d\n",
		__func__, di->wireless_tx_acc->dev_info_cnt);
	return 0;
}

static int wltx_aux_get_acc_dev_state(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_state(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			&di->wireless_tx_acc->dev_state);
	else
		ret = di->tx_ops->get_tx_acc_dev_state(&di->wireless_tx_acc->dev_state);
	if (ret) {
		hwlog_err("%s: get_tx_acc_dev_state fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: get wireless_tx_acc dev_state = %d\n",
		__func__, di->wireless_tx_acc->dev_state);
	return 0;
}

static int wltx_aux_get_acc_dev_no(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_no(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			&di->wireless_tx_acc->dev_no);
	else
		ret = di->tx_ops->get_tx_acc_dev_no(&di->wireless_tx_acc->dev_no);
	if (ret) {
		hwlog_err("%s: get_tx_acc_dev_no fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: get wireless_tx_acc dev_no = %d\n",
		__func__, di->wireless_tx_acc->dev_no);
	return 0;
}

static int wltx_aux_get_acc_dev_mac(struct wltx_aux_dev_info *di)
{
	int i;
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_mac(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			di->wireless_tx_acc->dev_mac, QI_ACC_TX_DEV_MAC_LEN);
	else
		ret = di->tx_ops->get_tx_acc_dev_mac(di->wireless_tx_acc->dev_mac);
	if (ret) {
		hwlog_err("%s: get_tx_acc_dev_mac fail\n", __func__);
		return ret;
	}

	for (i = 0; i < WL_TX_ACC_DEV_MAC_LEN; i++)
		hwlog_info("%s: get wireless_tx_acc dev_mac[%d] = 0x%02x\n",
			__func__, i, di->wireless_tx_acc->dev_mac[i]);

	return 0;
}

static int wltx_aux_get_acc_dev_model_id(struct wltx_aux_dev_info *di)
{
	int i;
	int ret;

	if (!di || !di->wireless_tx_acc) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_model_id(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			di->wireless_tx_acc->dev_model_id, QI_ACC_TX_DEV_MODELID_LEN);
	else
		ret = di->tx_ops->get_tx_acc_dev_model_id(
			di->wireless_tx_acc->dev_model_id);
	if (ret) {
		hwlog_err("%s: get_tx_acc_dev_model_id fail\n", __func__);
		return ret;
	}

	for (i = 0; i < WL_TX_ACC_DEV_MODELID_LEN; i++)
		hwlog_info("%s: wireless_tx_acc dev_model_id[%d] = 0x%02x\n",
			__func__, i, di->wireless_tx_acc->dev_model_id[i]);

	return 0;
}

static int wltx_aux_get_acc_dev_submodel_id(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_submodel_id(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			&di->wireless_tx_acc->dev_submodel_id);
	else
		ret = di->tx_ops->get_tx_acc_dev_submodel_id(
			&di->wireless_tx_acc->dev_submodel_id);
	if (ret) {
		hwlog_err("%s: TX get acc dev_submodeid fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: wireless_tx_acc dev_submodel_id = 0x%02x\n",
		__func__, di->wireless_tx_acc->dev_submodel_id);

	return 0;
}

static int wltx_aux_get_acc_dev_version(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_version(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			&di->wireless_tx_acc->dev_version);
	else
		ret = di->tx_ops->get_tx_acc_dev_version(
			&di->wireless_tx_acc->dev_version);
	if (ret) {
		hwlog_err("%s: TX get acc dev_submodeid fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: wireless_tx_acc dev_version = 0x%02x\n",
		__func__, di->wireless_tx_acc->dev_version);

	return 0;
}

static int wltx_aux_get_acc_dev_business(struct wltx_aux_dev_info *di)
{
	int ret;

	if (!di->wireless_tx_acc) {
		hwlog_err("%s: wireless_tx_acc is null\n", __func__);
		return -1;
	}

	if (!di->tx_ops)
		ret = wireless_acc_get_tx_dev_business(WLTRX_IC_AUX, WIRELESS_PROTOCOL_QI,
			&di->wireless_tx_acc->dev_business);
	else
		ret = di->tx_ops->get_tx_acc_dev_business(
			&di->wireless_tx_acc->dev_business);
	if (ret) {
		hwlog_err("%s: get acc dev_submodeid fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: wireless_tx_acc dev_business = 0x%02x\n",
		__func__, di->wireless_tx_acc->dev_business);
	return 0;
}

static int wltx_aux_get_acc_info(struct wltx_aux_dev_info *di)
{
	int ret;

	ret = wltx_aux_get_acc_dev_info_cnt(di);
	ret |= wltx_aux_get_acc_dev_state(di);
	ret |= wltx_aux_get_acc_dev_no(di);
	ret |= wltx_aux_get_acc_dev_mac(di);
	ret |= wltx_aux_get_acc_dev_model_id(di);
	ret |= wltx_aux_get_acc_dev_submodel_id(di);
	ret |= wltx_aux_get_acc_dev_version(di);
	ret |= wltx_aux_get_acc_dev_business(di);
	if (ret) {
		hwlog_err("%s: get acc info failed\n", __func__);
		return ret;
	}

	hwlog_info("%s: get acc info succ\n", __func__);
	return 0;
}

static void wltx_aux_fault_event_handler(struct wltx_aux_dev_info *di)
{
	wltx_aux_wake_lock();
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	hwlog_info("%s: tx_status = 0x%02x\n", __func__, g_tx_status);

	switch (g_tx_status) {
	case WL_TX_STATUS_RX_DISCONNECT:
		di->ping_timeout = di->ping_timeout_2;
		wltx_aux_set_stage(WL_TX_STAGE_PING_RX);
		schedule_work(&di->wltx_check_work);
		break;
	/* fall-through */
	case WL_TX_STATUS_TX_CLOSE:
	case WL_TX_STATUS_SOC_ERROR:
	case WL_TX_STATUS_TBATT_HIGH:
	case WL_TX_STATUS_TBATT_LOW:
	case WL_TX_STATUS_CHARGE_DONE:
	case WL_TX_STATUS_PING_SUCC:
		wltx_aux_set_tx_open_flag(false);
		wltx_aux_enable_tx_mode(di, false);
		wltx_aux_enable_power(false);
		wltx_aux_wake_unlock();
		break;
	default:
		wltx_aux_wake_unlock();
		hwlog_err("%s: has no this tx_status %d\n",
			__func__, g_tx_status);
		break;
	}
}

static void wltx_aux_iout_control(struct wltx_aux_dev_info *di)
{
	int ret;
	u16 tx_iin = 0;
	u16 tx_vin = 0;
	u16 tx_vrect = 0;
	u16 tx_fop = 0;
	s16 chip_temp = 0;
	static int log_cnt;

	if (!di->tx_ops) {
		ret = wltx_ic_get_iin(WLTRX_IC_AUX, &tx_iin);
		ret |= wltx_ic_get_vin(WLTRX_IC_AUX, &tx_vin);
		ret |= wltx_ic_get_vrect(WLTRX_IC_AUX, &tx_vrect);
		ret |= wltx_ic_get_temp(WLTRX_IC_AUX, &chip_temp);
		ret |= wltx_ic_get_fop(WLTRX_IC_AUX, &tx_fop);
	} else {
		ret = di->tx_ops->get_tx_iin(&tx_iin);
		ret |= di->tx_ops->get_tx_vin(&tx_vin);
		ret |= di->tx_ops->get_tx_vrect(&tx_vrect);
		ret |= di->tx_ops->get_chip_temp((u8 *)&chip_temp);
		ret |= di->tx_ops->get_tx_fop(&tx_fop);
	}
	if (ret) {
		di->i2c_err_cnt++;
		hwlog_err("%s: get tx vin/iin fail", __func__);
	}

	wltx_aux_calc_tx_iin_avg(di, tx_iin);
	if (log_cnt++ == (WL_TX_MONITOR_LOG_INTERVAL / di->monitor_interval)) {
		hwlog_info("%s:tx_fop=%ukHZ, tx_iin_avg=%dmA, tx_iin = %dmA",
			__func__, tx_fop, di->tx_iin_avg, tx_iin);
		hwlog_info("%s:tx_vin=%dmV, tx_vrect=%dmV,chip_temp = %d\n",
			__func__, tx_vin, tx_vrect, chip_temp);
		log_cnt = 0;
	}

	if (wltx_aux_ps_tx_volt_check(di))
		hwlog_err("aux_iout_control: ps_tx_volt_check failed\n");
}

static void wltx_aux_monitor_work(struct work_struct *work)
{
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (wltx_aux_can_do_reverse_charging() == WL_TX_FAIL)
		goto func_end;

	wltx_aux_check_rx_disconnect(di);
	wltx_aux_check_in_tx_mode(di);
	wltx_check_reverse_timeout(di);

	if (di->stop_reverse_charge) {
		hwlog_info("%s: stop monitor work\n", __func__);
		goto func_end;
	}

	if (!g_tx_open_flag || (di->i2c_err_cnt > WL_TX_I2C_ERR_CNT)) {
		hwlog_err("%s:TX closed, stop monitor work\n", __func__);
		wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		goto func_end;
	}

	wltx_aux_iout_control(di);
	schedule_delayed_work(&di->wltx_aux_monitor_work,
		msecs_to_jiffies(di->monitor_interval));
	return;

func_end:
	wltx_aux_set_stage(WL_TX_STAGE_DEFAULT);
	wltx_aux_fault_event_handler(di);
}

/* each dev should done */
struct wireless_acc_key_info g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_MAX] = {
	{ "DEVICENO", "0" },
	{ "DEVICESTATE", "UNKNOWN" },
	{ "DEVICEMAC", "FF:FF:FF:FF:FF:FF" },
	{ "DEVICEMODELID", "000000" },
	{ "DEVICESUBMODELID", "00" },
	{ "DEVICEVERSION", "00" },
	{ "DEVICEBUSINESS", "00" },
};

void wltx_aux_notify_android_uevent(struct wltx_acc_dev *di)
{
	if (!di || (di->dev_no < ACC_DEV_NO_BEGIN) ||
		(di->dev_no >= ACC_DEV_NO_END) ||
		(di->dev_info_cnt != (WL_TX_ACC_DEV_MAC_LEN +
			WL_TX_ACC_DEV_VERSION_LEN +
			WL_TX_ACC_DEV_BUSINESS_LEN +
			WL_TX_ACC_DEV_MODELID_LEN +
			WL_TX_ACC_DEV_SUBMODELID_LEN))) {
		hwlog_err("input invaild, wltx aux not notify uevent\n");
		return;
	}
	switch (di->dev_state) {
	case WL_ACC_DEV_STATE_ONLINE:
		snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_CONNECTED_STR);
		break;
	case WL_ACC_DEV_STATE_OFFLINE:
		snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_DISCONNECTED_STR);
		break;
	case WL_ACC_DEV_STATE_PING_SUCC:
		snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_PING_SUCC_STR);
		break;
	default:
		snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_UNKNOWN_STR);
		break;
	}

	snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_NO].value,
		ACC_VALUE_MAX_LEN, "%d", di->dev_no);
	/* dev_mac[0 1 2 3 4 5] is BT MAC ADDR */
	snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_MAC].value, ACC_VALUE_MAX_LEN,
		"%02x:%02x:%02x:%02x:%02x:%02x", di->dev_mac[0], di->dev_mac[1],
		di->dev_mac[2], di->dev_mac[3], di->dev_mac[4], di->dev_mac[5]);
	/* dev_model_id[0 1 2] is BT model id */
	snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_MODEL_ID].value,
		ACC_VALUE_MAX_LEN, "%02x%02x%02x", di->dev_model_id[0],
		di->dev_model_id[1], di->dev_model_id[2]);
	snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_SUBMODEL_ID].value,
		ACC_VALUE_MAX_LEN, "%02x", di->dev_submodel_id);
	snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_VERSION].value,
		ACC_VALUE_MAX_LEN, "%02x", di->dev_version);
	snprintf(g_wl_acc_aux_info_tab[WL_TX_ACC_INFO_BUSINESS].value,
		ACC_VALUE_MAX_LEN, "%02x", di->dev_business);
	wireless_acc_report_uevent(g_wl_acc_aux_info_tab,
		WL_TX_ACC_INFO_MAX, di->dev_no);
	hwlog_info("wltx aux notify uevent end\n");
}

static void wltx_aux_report_ping_succ(void)
{
	int ret;
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	wltx_aux_set_acc_dev_state(di, WL_ACC_DEV_STATE_PING_SUCC);
	wltx_aux_set_acc_dev_info_cnt(di, WL_TX_ACC_DEV_INFO_CNT);

	ret = wltx_aux_get_acc_info(di);
	if (!ret)
		wltx_aux_notify_android_uevent(di->wireless_tx_acc);
}

static void wltx_aux_start_check_work(struct work_struct *work)
{
	int ret;
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	wltx_aux_wake_lock();
	wltx_aux_para_init(di);
	g_init_tbatt = coul_drv_battery_temperature();
	hwlog_info("%s: 0x%02x\n", __func__, wltx_aux_get_stage());

	if (wltx_aux_get_stage() == WL_TX_STAGE_DEFAULT) {
		ret = wltx_aux_can_do_reverse_charging();
		if (ret)
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_POWER_SUPPLY);
	}
	if (wltx_aux_get_stage() == WL_TX_STAGE_POWER_SUPPLY) {
		ret = wltx_aux_power_supply(di);
		if (ret)
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_CHIP_INIT);
	}
	if (wltx_aux_get_stage() == WL_TX_STAGE_CHIP_INIT) {
		ret = wltx_aux_chip_init(di);
		if (ret)
			hwlog_err("%s: TX chip init fail, go on\n", __func__);
		wltx_aux_set_stage(WL_TX_STAGE_PING_RX);
	}
	if (wltx_aux_get_stage() == WL_TX_STAGE_PING_RX) {
		ret = wltx_aux_ping_rx(di);
		if (ret)
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_REGULATION);
	}

	hwlog_info("%s: start wireless reverse charging\n",
		__func__);
	wltx_aux_set_tx_status(WL_TX_STATUS_IN_CHARGING);
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	mod_delayed_work(system_wq, &di->wltx_aux_monitor_work,
		msecs_to_jiffies(0));
	return;

func_end:
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	if (wltx_aux_get_stage() == WL_TX_STAGE_DEFAULT)
		wltx_aux_set_tx_open_flag(false);
	if (wltx_aux_get_stage() >= WL_TX_STAGE_PING_RX)
		wltx_aux_enable_tx_mode(di, false);
	if (wltx_aux_get_stage() >= WL_TX_STAGE_POWER_SUPPLY)
		wltx_aux_enable_power(false);
	wltx_aux_set_stage(WL_TX_STAGE_DEFAULT);
	wltx_aux_wake_unlock();
}

static void wltx_aux_handle_ping_event(void)
{
	struct wltx_aux_dev_info *di = g_wltx_aux_di;
	static int abnormal_ping_cnt;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (g_tx_open_flag) {
		abnormal_ping_cnt = 0;
		return;
	}
	if (++abnormal_ping_cnt < 30) /* about 15s */
		return;

	wltx_aux_enable_tx_mode(di, false);
	wltx_aux_wake_unlock();
}

static void hall_approach_process_work(struct work_struct *work)
{
	int ret;
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	hwlog_info("%s: --enter--\n", __func__);
	wltx_aux_wake_lock();
	wltx_aux_para_init(di);

	wltx_aux_set_tx_open_flag(true);
	di->stop_reverse_charge = false;
	di->ping_timeout = di->ping_timeout_1;
	wltx_aux_set_stage(WL_TX_STAGE_DEFAULT);

	if (wltx_aux_get_stage() == WL_TX_STAGE_DEFAULT) {
		ret = wltx_aux_can_do_reverse_charging();
		if (ret || (g_hall_state == false))
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_POWER_SUPPLY);
	}
	if (wltx_aux_get_stage() == WL_TX_STAGE_POWER_SUPPLY) {
		ret = wltx_aux_power_supply(di);
		if (ret || (g_hall_state == false))
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_CHIP_INIT);
	}
	if (wltx_aux_get_stage() == WL_TX_STAGE_CHIP_INIT) {
		wltx_aux_chip_init(di);
		if (g_hall_state == false)
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_PING_RX);
	}
	if (wltx_aux_get_stage() == WL_TX_STAGE_PING_RX) {
		if (g_hall_state == false)
			goto func_end;
		ret = wltx_aux_ping_rx(di);
		if (ret)
			goto func_end;
		wltx_aux_set_stage(WL_TX_STAGE_REGULATION);
		wltx_aux_report_ping_succ();
	}
	hwlog_info("%s: start wireless reverse charging, -out-\n", __func__);
	wltx_aux_set_tx_status(WL_TX_STATUS_IN_CHARGING);
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	mod_delayed_work(system_wq, &di->wltx_aux_monitor_work,
		msecs_to_jiffies(di->monitor_delay));
	return;

func_end:
	hwlog_info("%s: g_hall_state = %d, wltx_stage = %d\n", __func__,
		g_hall_state, wltx_aux_get_stage());
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	if (wltx_aux_get_stage() == WL_TX_STAGE_DEFAULT)
		wltx_aux_set_tx_open_flag(false);
	if (wltx_aux_get_stage() >= WL_TX_STAGE_PING_RX)
		wltx_aux_enable_tx_mode(di, false);
	if (wltx_aux_get_stage() >= WL_TX_STAGE_POWER_SUPPLY)
		wltx_aux_enable_power(false);
	wltx_aux_set_stage(WL_TX_STAGE_DEFAULT);
	wltx_aux_wake_unlock();
	wltx_aux_set_tx_status(WL_TX_STATUS_DEFAULT);
	hwlog_info("%s: -out-\n", __func__);
}

static void hall_away_process_work(struct work_struct *work)
{
	struct wltx_aux_dev_info *di = g_wltx_aux_di;
	int ret;

	if (g_hall_state == true) {
		hwlog_info("%s: g_hall_state is true\n", __func__);
		return;
	}

	wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
	wltx_aux_set_tx_open_flag(false);
	di->stop_reverse_charge = true;
	/* set dev state offline */
	wltx_aux_set_acc_dev_state(di, WL_ACC_DEV_STATE_OFFLINE);
	wltx_aux_set_acc_dev_info_cnt(di, WL_TX_ACC_DEV_INFO_CNT);
	ret = wltx_aux_get_acc_info(di);
	if (!ret)
		wltx_aux_notify_android_uevent(di->wireless_tx_acc);
	wltx_aux_set_tx_status(WL_TX_STATUS_DEFAULT);
	wltx_aux_enable_tx_mode(di, false);
	wltx_aux_enable_power(false);
	hwlog_info("%s: g_hall_state is false\n", __func__);
}

static void wltx_aux_event_work(struct work_struct *work)
{
	int ret;
	struct wltx_aux_dev_info *di = g_wltx_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	switch (di->tx_event_type) {
	case POWER_NE_WLTX_GET_CFG:
		/* get configure packet, ping succ */
		wltx_aux_set_tx_status(WL_TX_STATUS_PING_SUCC);
		break;
	case POWER_NE_WLTX_HANDSHAKE_SUCC:
		/* 0x8866 handshake, security authentic succ */
		di->standard_rx = true;
		break;
	case POWER_NE_WLTX_CHARGEDONE:
		wltx_aux_set_tx_status(WL_TX_STATUS_CHARGE_DONE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_CEP_TIMEOUT:
		wltx_aux_set_tx_status(WL_TX_STATUS_RX_DISCONNECT);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_EPT_CMD:
		wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_OVP:
		wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_OCP:
		wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_PING_RX:
		wltx_aux_handle_ping_event();
		break;
	case POWER_NE_WLTX_HALL_APPROACH:
		hwlog_info("%s: POWER_NE_WLTX_HALL_APPROACH\n", __func__);
		g_hall_state = true;
		queue_delayed_work(g_wltx_aux_di->aux_tx_wq,
			&g_wltx_aux_di->hall_approach_work,
			msecs_to_jiffies(0));
		break;
	case POWER_NE_WLTX_HALL_AWAY_FROM:
		hwlog_info("%s: POWER_NE_WLTX_HALL_AWAY_FROM\n", __func__);
		g_hall_state = false;
		queue_delayed_work(g_wltx_aux_di->aux_tx_wq,
			&g_wltx_aux_di->hall_away_work, msecs_to_jiffies(0));
		break;
	case POWER_NE_WLTX_ACC_DEV_CONNECTED:
		hwlog_info("%s: POWER_NE_WLTX_ACC_DEV_CONNECTED\n",
			__func__);
		ret = wltx_aux_get_acc_info(di);
		if (ret)
			hwlog_err("%s: get_acc_info fail\n", __func__);
		else
			wltx_aux_notify_android_uevent(di->wireless_tx_acc);
		break;
	case POWER_NE_WLTX_TX_FOD:
		wltx_aux_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		hwlog_info("%s: POWER_NE_WLTX_TX_FOD\n",
			__func__);
		break;
	default:
		hwlog_err("%s: has no this event_type %d\n",
				__func__, di->tx_event_type);
		break;
	}
}

static int wltx_aux_event_notifier_call(struct notifier_block *tx_event_nb,
	unsigned long event, void *data)
{
	u8 *tx_notify_data = NULL;
	struct wltx_aux_dev_info *di =
		container_of(tx_event_nb, struct wltx_aux_dev_info,
			tx_event_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WLTX_GET_CFG:
	case POWER_NE_WLTX_HANDSHAKE_SUCC:
	case POWER_NE_WLTX_CHARGEDONE:
	case POWER_NE_WLTX_CEP_TIMEOUT:
	case POWER_NE_WLTX_EPT_CMD:
	case POWER_NE_WLTX_OVP:
	case POWER_NE_WLTX_OCP:
	case POWER_NE_WLTX_PING_RX:
	case POWER_NE_WLTX_HALL_APPROACH:
	case POWER_NE_WLTX_HALL_AWAY_FROM:
	case POWER_NE_WLTX_ACC_DEV_CONNECTED:
	case POWER_NE_WLTX_TX_FOD:
		break;
	default:
		return NOTIFY_OK;
	}

	if (data) {
		tx_notify_data = (u8 *)data;
		di->tx_event_data = *tx_notify_data;
	}
	di->tx_event_type = event;
	schedule_work(&di->wltx_evt_work);
	return NOTIFY_OK;
}

static int wltx_parse_tx_vset_para(struct device_node *np,
	struct wltx_aux_dev_info *di)
{
	int i;
	int ret;
	int array_len;
	u32 tmp_para[WLTX_TX_VSET_TOTAL * WLTX_TX_VSET_TYPE_MAX] = { 0 };

	array_len = of_property_count_u32_elems(np, "tx_vset_para");
	if ((array_len <= 0) || (array_len % WLTX_TX_VSET_TOTAL) ||
		(array_len > WLTX_TX_VSET_TOTAL * WLTX_TX_VSET_TYPE_MAX)) {
		hwlog_err("%s: tx_vset_para is invalid\n", __func__);
		return -EINVAL;
	}
	di->tx_vset.total = array_len / WLTX_TX_VSET_TOTAL;
	ret = of_property_read_u32_array(np, "tx_vset_para",
		tmp_para, array_len);
	if (ret) {
		hwlog_err("%s: get tx_vset_para fail\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < di->tx_vset.total; i++) {
		di->tx_vset.para[i].rx_vmin =
			tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_RX_VSET_MIN];
		di->tx_vset.para[i].rx_vmax =
			tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_RX_VSET_MAX];
		di->tx_vset.para[i].vset =
			(int)tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_TX_VSET];
		di->tx_vset.para[i].lth =
			tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_TX_VSET_LTH];
		di->tx_vset.para[i].hth =
			tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_TX_VSET_HTH];
		di->tx_vset.para[i].pl_th =
			tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_TX_PLOSS_TH];
		di->tx_vset.para[i].pl_cnt =
			(u8)tmp_para[WLTX_TX_VSET_TOTAL * i +
				WLTX_TX_PLOSS_CNT];

		if (di->tx_vset.max_vset < di->tx_vset.para[i].vset)
			di->tx_vset.max_vset = di->tx_vset.para[i].vset;
		hwlog_info("[%s][%d] rx_min:%dmV rx_max:%dmV vset:%dmV\t"
			"lth:%dmV hth:%dmV pl_th:%dmW pl_cnt:%d\n",
			__func__, i, di->tx_vset.para[i].rx_vmin,
			di->tx_vset.para[i].rx_vmax, di->tx_vset.para[i].vset,
			di->tx_vset.para[i].lth, di->tx_vset.para[i].hth,
			di->tx_vset.para[i].pl_th, di->tx_vset.para[i].pl_cnt);
	}

	return 0;
}

static void wltx_aux_parse_tx_stage_vset_para(struct device_node *np, struct wltx_aux_dev_info *di)
{
	if (!power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"tx_stage_vset", (u32 *)&di->tx_vset, WLTX_TX_VSET_STAGE))
		goto print_para;

	di->tx_vset.v_ps = 5000; /* mV */
	di->tx_vset.v_ping = 5000; /* mV */
	di->tx_vset.v_hs = 5000; /* mV */
	di->tx_vset.v_dflt = 5000; /* mV */

print_para:
	hwlog_info("[parse_tx_stage_vset_para] pwr_supply:%d ping:%d handshake:%d default:%d\n",
		di->tx_vset.v_ps, di->tx_vset.v_ping, di->tx_vset.v_hs, di->tx_vset.v_dflt);
}

static void wltx_aux_parse_iin_vset_para(struct device_node *np, struct wltx_aux_dev_info *di)
{
	int i;
	int arr_len;

	arr_len = of_property_count_u32_elems(np, "iin_vset_para");
	if ((arr_len <= 0) || power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"iin_vset_para", (int *)di->vset_cfg.vset_para, WLTX_VSET_ROW * WLTX_VSET_COL)) {
		di->vset_cfg.vset_level = WLTX_TX_VSET_DEFAULT_LEVEL;
		di->vset_cfg.vset_para[0].iin_min = 0;
		di->vset_cfg.vset_para[0].iin_max = 410; /* < 410mA is 1.5c, level one */
		di->vset_cfg.vset_para[0].vset = 4850; /* mV, default level one volt to set */
		di->vset_cfg.vset_para[1].iin_min = 430; /* > 430mA is 3c, level two */
		di->vset_cfg.vset_para[1].iin_max = 20000; /* infinity iin */
		di->vset_cfg.vset_para[1].vset = 5500; /* mV, default level two volt to set */
		goto print_vset_para;
	}
	di->vset_cfg.vset_level = arr_len / WLTX_VSET_COL;

print_vset_para:
	for (i = 0; i < di->vset_cfg.vset_level; i++)
		hwlog_info("iin_vset_para[%d] iin_min:%u iin_max:%u vset:%d\n", i,
			di->vset_cfg.vset_para[i].iin_min, di->vset_cfg.vset_para[i].iin_max,
			di->vset_cfg.vset_para[i].vset);
}

static void wltx_aux_set_default_tx_vset_para(struct wltx_aux_dev_info *di)
{
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	di->tx_vset.total = 1; /* only one level */
	di->tx_vset.max_vset = 5000; /* mV */
	di->tx_vset.para[0].rx_vmin = 4400; /* mV */
	di->tx_vset.para[0].rx_vmax = 5900; /* mV */
	di->tx_vset.para[0].vset = 5000; /* mV */
	di->tx_vset.para[0].lth = 4500; /* mV */
	di->tx_vset.para[0].hth = 5800; /* mV */
}

static void wltx_aux_parse_dts(struct device_node *np,
	struct wltx_aux_dev_info *di)
{
	int ret;

	ret = wltx_parse_tx_vset_para(np, di);
	if (ret) {
		hwlog_info("%s: use default tx_vset para\n", __func__);
		wltx_aux_set_default_tx_vset_para(di);
	}

	wltx_aux_parse_tx_stage_vset_para(np, di);
	wltx_aux_parse_iin_vset_para(np, di);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ping_timeout_1", &di->ping_timeout_1, WL_TX_PING_TIMEOUT_1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ping_timeout_2", &di->ping_timeout_2, WL_TX_PING_TIMEOUT_2);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pwm_support", &di->pwm_support, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"monitor_delay", &di->monitor_delay, 0);
}

static int wltx_aux_boost_en_init(struct device_node *np,
	struct wltx_aux_dev_info *di)
{
	if (power_gpio_config_output(np,
		"gpio_tx_boost_en", "gpio_tx_boost_en", &di->gpio_tx_boost_en, 0))
		return -1;

	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t wltx_aux_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t wltx_aux_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wltx_aux_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(wltx_aux, 0644, WL_TX_SYSFS_TX_OPEN, tx_open),
	power_sysfs_attr_ro(wltx_aux, 0444, WL_TX_SYSFS_TX_STATUS, tx_status),
	power_sysfs_attr_ro(wltx_aux, 0444, WL_TX_SYSFS_TX_IIN_AVG, tx_iin_avg),
	power_sysfs_attr_rw(wltx_aux, 0644, WL_TX_SYSFS_DPING_FREQ, dping_freq),
	power_sysfs_attr_rw(wltx_aux, 0644, WL_TX_SYSFS_DPING_INTERVAL, dping_interval),
	power_sysfs_attr_rw(wltx_aux, 0644, WL_TX_SYSFS_MAX_FOP, max_fop),
	power_sysfs_attr_rw(wltx_aux, 0644, WL_TX_SYSFS_MIN_FOP, min_fop),
	power_sysfs_attr_ro(wltx_aux, 0444, WL_TX_SYSFS_TX_FOP, tx_fop),
	power_sysfs_attr_ro(wltx_aux, 0444, WL_TX_SYSFS_HANDSHAKE, tx_handshake),
	power_sysfs_attr_rw(wltx_aux, 0644, WL_TX_SYSFS_CHK_TRXCOIL, check_trxcoil),
	power_sysfs_attr_ro(wltx_aux, 0444, WL_TX_SYSFS_TX_VIN, tx_vin),
	power_sysfs_attr_ro(wltx_aux, 0444, WL_TX_SYSFS_TX_IIN, tx_iin),
};

static struct attribute *wltx_aux_sysfs_attrs[ARRAY_SIZE(wltx_aux_sysfs_field_tbl) + 1];

static const struct attribute_group wltx_aux_sysfs_attr_group = {
	.attrs = wltx_aux_sysfs_attrs,
};

static void wltx_aux_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(wltx_aux_sysfs_attrs,
		wltx_aux_sysfs_field_tbl, ARRAY_SIZE(wltx_aux_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "wireless_aux_tx",
		dev, &wltx_aux_sysfs_attr_group);
}

static void wltx_aux_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "wireless_aux_tx",
		dev, &wltx_aux_sysfs_attr_group);
}
#else
static inline void wltx_aux_sysfs_create_group(struct device *dev)
{
}

static inline void wltx_aux_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static ssize_t wltx_aux_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wltx_aux_dev_info *di = g_wltx_aux_di;
	u16 temp_data = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wltx_aux_sysfs_field_tbl, ARRAY_SIZE(wltx_aux_sysfs_field_tbl));
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WL_TX_SYSFS_TX_OPEN:
		return snprintf(buf, PAGE_SIZE, "%d\n", g_tx_open_flag);
	case WL_TX_SYSFS_TX_STATUS:
		return snprintf(buf, PAGE_SIZE, "%d\n", g_tx_status);
	case WL_TX_SYSFS_TX_IIN_AVG:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->tx_iin_avg);
	case WL_TX_SYSFS_DPING_FREQ:
		wltx_aux_get_digital_ping_frequency(di, &temp_data);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	case WL_TX_SYSFS_DPING_INTERVAL:
		wltx_aux_get_digital_ping_interval(di, &temp_data);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	case WL_TX_SYSFS_MAX_FOP:
		wltx_aux_get_max_fop(di, &temp_data);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	case WL_TX_SYSFS_MIN_FOP:
		wltx_aux_get_min_fop(di, &temp_data);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	case WL_TX_SYSFS_TX_FOP:
		wltx_aux_get_fop(di, &temp_data);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	case WL_TX_SYSFS_HANDSHAKE:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			wltx_check_handshake(di));
	case WL_TX_SYSFS_CHK_TRXCOIL:
		hwlog_info("%s: WL_TX_SYSFS_CHK_TRXCOIL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	case WL_TX_SYSFS_TX_VIN:
		wltx_aux_get_tx_vin(di, &temp_data);
		hwlog_info("%s: WL_TX_SYSFS_TX_VIN\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	case WL_TX_SYSFS_TX_IIN:
		wltx_aux_get_tx_iin(di, &temp_data);
		hwlog_info("%s: WL_TX_SYSFS_TX_IIN\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp_data);
	default:
		hwlog_err("%s: NO THIS NODE:%d\n", __func__, info->name);
		break;
	}
	return 0;
}

static ssize_t wltx_aux_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wltx_aux_dev_info *di = g_wltx_aux_di;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wltx_aux_sysfs_field_tbl, ARRAY_SIZE(wltx_aux_sysfs_field_tbl));
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WL_TX_SYSFS_TX_OPEN:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 1)) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		if (g_tx_open_flag && val) {
			hwlog_err("%s: tx mode has already open, ignore",
				__func__);
			break;
		}
		wltx_aux_set_tx_open_flag(val);
		if (g_tx_open_flag) {
			di->ping_timeout = di->ping_timeout_1;
			wltx_aux_set_stage(WL_TX_STAGE_DEFAULT);
			schedule_work(&di->wltx_check_work);
		}
		wltx_aux_set_tx_status(WL_TX_STATUS_DEFAULT);
		break;
	case WL_TX_SYSFS_DPING_FREQ:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		wltx_aux_set_digital_ping_frequency(di, val);
		break;
	case WL_TX_SYSFS_DPING_INTERVAL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		wltx_aux_set_digital_ping_interval(di, val);
		break;
	case WL_TX_SYSFS_MAX_FOP:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		wltx_aux_set_max_fop(di, val);
		break;
	case WL_TX_SYSFS_MIN_FOP:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		wltx_aux_set_min_fop(di, val);
		break;
	case WL_TX_SYSFS_CHK_TRXCOIL:
		hwlog_info("%s: WL_TX_SYSFS_CHK_TRXCOIL\n", __func__);
		break;
	default:
		hwlog_err("(%s)NODE ERR!!HAVE NO THIS NODE:%d\n",
			__func__, info->name);
		break;
	}
	return count;
}

static struct wltx_acc_dev *wltx_acc_dev_alloc(void)
{
	struct wltx_acc_dev *di = kzalloc(sizeof(*di), GFP_KERNEL);
	return di;
}

static struct wltx_aux_dev_info *wltx_aux_dev_info_alloc(void)
{
	struct wltx_aux_dev_info *di = kzalloc(sizeof(*di), GFP_KERNEL);
	return di;
}

static void wltx_aux_shutdown(struct platform_device *pdev)
{
	struct wltx_aux_dev_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	hwlog_info("%s:\n", __func__);
	cancel_delayed_work(&di->wltx_aux_monitor_work);
	wltx_aux_set_tx_open_flag(false);
	wltx_aux_enable_tx_mode(di, false);
}

static int wltx_aux_remove(struct platform_device *pdev)
{
	struct wltx_aux_dev_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return 0;
	}

	(void)power_event_bnc_unregister(POWER_BNT_WLTX_AUX, &di->tx_event_nb);
	wltx_aux_sysfs_remove_group(di->dev);
	wakeup_source_trash(&g_wltx_aux_wakelock);

	hwlog_info("%s:\n", __func__);
	return 0;
}

static int wltx_aux_probe(struct platform_device *pdev)
{
	int ret;
	struct wltx_aux_dev_info *di = NULL;
	struct wltx_acc_dev *tx_acc_dev_di = NULL;
	struct device_node *np = NULL;

	di = wltx_aux_dev_info_alloc();
	if (!di) {
		hwlog_err("%s:di alloc failed\n", __func__);
		return -ENOMEM;
	}

	g_wltx_aux_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	di->tx_ops = g_wltx_aux_ops;

	wakeup_source_init(&g_wltx_aux_wakelock, "wireless_aux_tx_wakelock");

	wltx_aux_parse_dts(np, di);

	ret = wltx_aux_boost_en_init(np, di);
	if (ret) {
		hwlog_err("%s: aux boost en init failed\n", __func__);
		goto boost_init_fail;
	}
	INIT_WORK(&di->wltx_check_work, wltx_aux_start_check_work);
	INIT_WORK(&di->wltx_evt_work, wltx_aux_event_work);
	INIT_DELAYED_WORK(&di->wltx_aux_monitor_work,
		wltx_aux_monitor_work);
	di->aux_tx_wq = create_singlethread_workqueue("wltx_aux_wq");

	INIT_DELAYED_WORK(&di->hall_approach_work, hall_approach_process_work);
	INIT_DELAYED_WORK(&di->hall_away_work, hall_away_process_work);

	di->tx_event_nb.notifier_call = wltx_aux_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLTX_AUX, &di->tx_event_nb);
	if (ret < 0)
		goto notifier_regist_fail;

	tx_acc_dev_di = wltx_acc_dev_alloc();
	if (!tx_acc_dev_di) {
		hwlog_err("%s:acc di alloc failed\n", __func__);
		goto alloc_acc_dev_fail;
	}
	di->wireless_tx_acc = tx_acc_dev_di;

	wltx_aux_sysfs_create_group(di->dev);

	hwlog_info("wireless_aux_tx probe ok\n");
	return 0;

alloc_acc_dev_fail:
	(void)power_event_bnc_unregister(POWER_BNT_WLTX_AUX, &di->tx_event_nb);
notifier_regist_fail:
	gpio_free(di->gpio_tx_boost_en);
boost_init_fail:
	wakeup_source_trash(&g_wltx_aux_wakelock);
	di->tx_ops = NULL;
	kfree(di);
	di = NULL;
	g_wltx_aux_di = NULL;
	np = NULL;
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static const struct of_device_id wireless_aux_tx_match_table[] = {
	{
		.compatible = "huawei, wireless_aux_tx",
		.data = NULL,
	},
	{},
};

static struct platform_driver wltx_aux_driver = {
	.probe = wltx_aux_probe,
	.remove = wltx_aux_remove,
	.shutdown = wltx_aux_shutdown,
	.driver = {
		.name = "huawei, wireless_aux_tx",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wireless_aux_tx_match_table),
	},
};

static int __init wltx_aux_init(void)
{
	hwlog_info("wireless_aux_tx init ok\n");

	return platform_driver_register(&wltx_aux_driver);
}

static void __exit wltx_aux_exit(void)
{
	platform_driver_unregister(&wltx_aux_driver);
}

device_initcall_sync(wltx_aux_init);
module_exit(wltx_aux_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wireless aux tx module driver");
MODULE_AUTHOR("HUAWEI Inc");
