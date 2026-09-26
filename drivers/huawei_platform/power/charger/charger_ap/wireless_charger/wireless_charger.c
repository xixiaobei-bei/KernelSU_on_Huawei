#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/random.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif
#ifdef CONFIG_COUL_DRV
#include <linux/power/hisi/coul/coul_drv.h>
#endif
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <huawei_platform/power/huawei_charger.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_alarm.h>
#include <../charging_core.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <linux/hisi/powerkey_event.h>

#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ui.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_interfere.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_auth.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pmode.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif
#include <huawei_platform/power/wireless/wireless_keyboard.h>

#define HWLOG_TAG wireless_charger
HWLOG_REGIST();

static struct wlrx_dev_info *g_wlrx_di;
static int wireless_normal_charge_flag;
static int wireless_fast_charge_flag;
static int wireless_super_charge_flag;
static struct wakeup_source g_rx_con_wakelock;
static struct mutex g_rx_en_mutex;
static int rx_iout_samples[RX_IOUT_SAMPLE_LEN];
static int g_fop_fixed_flag;
static int g_rx_vrect_restore_cnt;
static int g_rx_vout_err_cnt;
static int g_rx_ocp_cnt;
static int g_rx_ovp_cnt;
static int g_rx_otp_cnt;
static int g_rx_imax = WLRX_IC_DFT_IOUT_MAX;
static bool g_wlc_start_sample_flag;
static bool g_bst_rst_complete = true;
static bool g_in_wldc_check;
static bool g_high_pwr_test_flag;
static u8 *g_tx_fw_version;
static u8 random[WLRX_AUTH_RANDOM_LEN];
static u8 tx_cipherkey[WLRX_AUTH_TX_KEY_LEN];
static bool g_need_recheck_cert;
static int g_plimit_time_num;
static bool g_need_force_5v_vout;

const enum wireless_protocol_tx_type g_qval_err_tx[] = {
	WIRELESS_TX_TYPE_CP39S, WIRELESS_TX_TYPE_CP39S_HK
};

static void wireless_charge_wake_lock(void)
{
	if (!g_rx_con_wakelock.active) {
		__pm_stay_awake(&g_rx_con_wakelock);
		hwlog_info("wireless_charge wake lock\n");
	}
}

static void wireless_charge_wake_unlock(void)
{
	if (g_rx_con_wakelock.active) {
		__pm_relax(&g_rx_con_wakelock);
		hwlog_info("wireless_charge wake unlock\n");
	}
}

static bool wlrx_msleep_exit(void)
{
	if ((wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) ||
		!wlrx_ic_is_tx_exist(WLTRX_IC_MAIN))
		return true;

	return false;
}

int wlc_get_rx_support_mode(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return WLC_RX_SP_BUCK_MODE;
	}

	return di->sysfs_data.rx_support_mode & di->qval_support_mode;
}

static void wlc_set_cur_vmode_id(struct wlrx_dev_info *di, int id)
{
	di->curr_vmode_index = id;
	hwlog_info("[set_cur_vmode_id] id=%d\n", di->curr_vmode_index);
}

static void wlc_prepare_wlrx_ui_para(struct wlrx_ui_para *ui_para)
{
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!dts || !di)
		return;

	ui_para->ui_max_pwr = dts->ui_pmax_lth;
	ui_para->product_max_pwr = dts->product_pmax_hth;
	ui_para->tx_max_pwr = di->tx_cap->vout_max / MVOLT_PER_VOLT *
		di->tx_cap->iout_max * WLRX_ACC_TX_PWR_RATIO / POWER_PERCENT;
}

static void wireless_charge_send_charge_uevent(struct wlrx_dev_info *di, int icon_type)
{
	int icon = ICON_TYPE_WIRELESS_NORMAL;
	struct wlrx_ui_para wlrx_ui_para = { 0 };

	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging, return\n", __func__);
		return;
	}
	wireless_normal_charge_flag = 0;
	wireless_fast_charge_flag = 0;
	wireless_super_charge_flag = 0;
	switch (icon_type) {
	case WIRELESS_NORMAL_CHARGE_FLAG:
		wireless_normal_charge_flag = 1;
		icon = ICON_TYPE_WIRELESS_NORMAL;
		break;
	case WIRELESS_FAST_CHARGE_FLAG:
		wireless_fast_charge_flag = 1;
		icon = ICON_TYPE_WIRELESS_QUICK;
		break;
	case WIRELESS_SUPER_CHARGE_FLAG:
		wireless_super_charge_flag = 1;
		icon = ICON_TYPE_WIRELESS_SUPER;
		break;
	default:
		hwlog_err("%s: unknown icon_type\n", __func__);
	}

	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_ICON_TYPE, &icon_type);
	hwlog_info("[%s] cur type=%d, last type=%d\n",
		__func__, icon_type, di->curr_icon_type);
	if (di->curr_icon_type ^ icon_type) {
		wireless_connect_send_icon_uevent(icon);
		if (wireless_super_charge_flag) {
			wlc_prepare_wlrx_ui_para(&wlrx_ui_para);
			wlrx_ui_send_soc_decimal_evt(&wlrx_ui_para);
			wlrx_ui_send_max_pwr_evt(&wlrx_ui_para);
		}
	}

	di->curr_icon_type = icon_type;
}

int wireless_charge_get_rx_iout_limit(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return 0;
	}
	return min(di->rx_iout_max, di->rx_iout_limit);
}

static void wlc_rx_chip_reset(struct wlrx_dev_info *di)
{
	if (di->wlc_err_rst_cnt >= WLC_RST_CNT_MAX)
		return;

	wlrx_ic_chip_reset(WLTRX_IC_MAIN);
	di->wlc_err_rst_cnt++;
	di->discon_delay_time = WL_RST_DISCONN_DELAY_MS;
}

static void wireless_charge_set_input_current(struct wlrx_dev_info *di)
{
	int iin_set = wireless_charge_get_rx_iout_limit();

	charge_set_input_current(iin_set);
}

static int wireless_charge_get_tx_id(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	return wireless_get_tx_id(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI);
}

static int wireless_charge_fix_tx_fop(int fop)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}

	return wireless_fix_tx_fop(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, fop);
}

static int wireless_charge_unfix_tx_fop(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}

	return wireless_unfix_tx_fop(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI);
}

static int wireless_charge_set_tx_vout(int tx_vout)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	if ((tx_vout > TX_DEFAULT_VOUT) &&
		(wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON)) {
		hwlog_err("%s: wired vbus connect, tx_vout should be set to %dmV at most\n",
			__func__, TX_DEFAULT_VOUT);
		return -1;
	}
	if (di->pwroff_reset_flag && (tx_vout > TX_DEFAULT_VOUT)) {
		hwlog_err("%s: pwroff_reset_flag = %d, tx_vout should be set to %dmV at most\n",
			__func__, di->pwroff_reset_flag, TX_DEFAULT_VOUT);
		return -1;
	}
	hwlog_info("[%s] tx_vout is set to %dmV\n", __func__, tx_vout);
	return wlrx_ic_set_vfc(WLTRX_IC_MAIN, tx_vout);
}

int wireless_charge_set_rx_vout(int rx_vout)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	if (di->pwroff_reset_flag && (rx_vout > RX_DEFAULT_VOUT)) {
		hwlog_err("%s: pwroff_reset_flag = %d, rx_vout should be set to %dmV at most\n",
			__func__, di->pwroff_reset_flag, RX_DEFAULT_VOUT);
		return -1;
	}
	hwlog_info("%s: rx_vout is set to %dmV\n", __func__, rx_vout);
	return wlrx_ic_set_vout(WLTRX_IC_MAIN, rx_vout);
}

void wireless_charge_get_tx_adaptor_type(u8 *type)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di || !type)
		return;

	*type = di->tx_cap->type;
}

struct wireless_protocol_tx_cap *wlc_get_tx_cap(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di)
		return NULL;

	return di->tx_cap;
}

int wlc_get_rx_max_iout(void)
{
	if (g_rx_imax <= 0)
		return WLRX_IC_DFT_IOUT_MAX;

	return g_rx_imax;
}

int wireless_charge_get_rx_avg_iout(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->iout_avg;
}

int wlc_get_cp_avg_iout(void)
{
	int cp_ratio = charge_pump_get_cp_ratio(CP_TYPE_MAIN);

	if (cp_ratio <= 0) {
		hwlog_err("%s: cp_ratio err\n", __func__);
		return wireless_charge_get_rx_avg_iout();
	}

	return cp_ratio * wireless_charge_get_rx_avg_iout();
}

int wlc_get_pmode_id_by_mode_name(const char *mode_name)
{
	int i;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || !mode_name) {
		hwlog_err("get_pmode_id_by_mode_name: para null\n");
		return 0;
	}
	for (i = 0; i < dts->pmode_cfg_level; i++) {
		if (!strncmp(mode_name, dts->pmode_cfg[i].name,
			strlen(dts->pmode_cfg[i].name)))
			return i;
	}

	return 0;
}

static void wireless_charge_count_avg_iout(struct wlrx_dev_info *di)
{
	int cnt_max;
	const char *cur_mode_name = NULL;
	struct wlrx_dts *dts = wlrx_get_dts();

	if ((di->monitor_interval <= 0) || !dts)
		return;

	cnt_max = RX_AVG_IOUT_TIME / di->monitor_interval;

	if (g_bst_rst_complete && (di->iout_avg < RX_LOW_IOUT)) {
		di->iout_high_cnt = 0;
		di->iout_low_cnt++;
		if (di->iout_low_cnt >= cnt_max)
			di->iout_low_cnt = cnt_max;
		return;
	}

	cur_mode_name = dts->pmode_cfg[di->curr_pmode_index].name;
	if ((di->iout_avg > RX_HIGH_IOUT) || strstr(cur_mode_name, "SC")) {
		di->iout_low_cnt = 0;
		di->iout_high_cnt++;
		if (di->iout_high_cnt >= cnt_max)
			di->iout_high_cnt = cnt_max;
		return;
	}
}

static void wireless_charge_calc_avg_iout(struct wlrx_dev_info *di)
{
	int i;
	int iout = 0;
	static int index = 0;
	int iout_sum = 0;

	(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &iout);
	rx_iout_samples[index] = iout;
	index = (index + 1) % RX_IOUT_SAMPLE_LEN;
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++)
		iout_sum += rx_iout_samples[i];
	di->iout_avg = iout_sum / RX_IOUT_SAMPLE_LEN;
}

static void wireless_charge_reset_avg_iout(struct wlrx_dev_info *di)
{
	int i;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++)
		rx_iout_samples[i] = dts->rx_imin;
	di->iout_avg = dts->rx_imin;
}

static int  wireless_charge_check_fast_charge_succ(struct wlrx_dev_info *di)
{
	if ((wireless_fast_charge_flag || wireless_super_charge_flag) &&
		(wlrx_get_charge_stage() >= WLRX_STAGE_CHARGING))
		return WIRELESS_CHRG_SUCC;
	else
		return WIRELESS_CHRG_FAIL;
}

static int  wireless_charge_check_normal_charge_succ(struct wlrx_dev_info *di)
{
	if (!wlrx_is_err_tx(di->tx_cap->type) && !wireless_fast_charge_flag &&
		(wlrx_get_charge_stage() >= WLRX_STAGE_CHARGING))
		return WIRELESS_CHRG_SUCC;
	else
		return WIRELESS_CHRG_FAIL;
}

static int wlc_formal_check_direct_charge(const char *m_name)
{
	int ret;

	ret = wldc_formal_check(m_name);
	if (!ret)
		wlrx_set_charge_stage(WLRX_STAGE_REGULATION_DC);

	return ret;
}

static void wlc_update_thermal_control(u8 thermal_ctrl)
{
	u8 thermal_status;

	thermal_status = thermal_ctrl & WLC_THERMAL_EXIT_SC_MODE;
	if ((thermal_status == WLC_THERMAL_EXIT_SC_MODE) && !g_high_pwr_test_flag)
		wlrx_set_plim_src(WLRX_PLIM_SRC_THERMAL);
	else
		wlrx_clear_plim_src(WLRX_PLIM_SRC_THERMAL);
}

static int wireless_set_thermal_ctrl(unsigned char value)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di || (value > 0xFF)) /* 0xFF: maximum of u8 */
		return -EINVAL;
	di->sysfs_data.thermal_ctrl = value;
	wlc_update_thermal_control(di->sysfs_data.thermal_ctrl);
	hwlog_info("thermal_ctrl = 0x%x", di->sysfs_data.thermal_ctrl);
	return 0;
}

static int wireless_get_thermal_ctrl(unsigned char *value)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di || !value)
		return -EINVAL;
	*value = di->sysfs_data.thermal_ctrl;
	return 0;
}

static struct power_if_ops wl_if_ops = {
	.set_wl_thermal_ctrl = wireless_set_thermal_ctrl,
	.get_wl_thermal_ctrl = wireless_get_thermal_ctrl,
	.type_name = "wl",
};

static bool wlc_pmode_final_judge_crit(struct wlrx_dev_info *di,
	struct wlrx_pmode *pmode_cfg, int pid)
{
	int tbatt = 0;

	if (!di->tx_cap->support_12v && (pmode_cfg->vtx == WIRELESS_ADAPTER_12V))
		return false;

	if ((di->tx_vout_max < pmode_cfg->vtx) || (di->rx_vout_max < pmode_cfg->vrx))
		return false;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	if ((pmode_cfg->tbatt >= 0) && (tbatt >= pmode_cfg->tbatt))
		return false;

	if ((pid == di->curr_pmode_index) &&
		(wlrx_get_charge_stage() != WLRX_STAGE_CHARGING)) {
		if ((pmode_cfg->timeout > 0) && time_after(jiffies, di->curr_power_time_out))
			return false;
	}

	return true;
}

static bool wlc_pmode_normal_judge_crit(struct wlrx_dev_info *di, struct wlrx_pmode *pmode_cfg)
{
	if ((pmode_cfg->cable >= 0) && (di->cable_detect_succ_flag != pmode_cfg->cable))
		return false;

	if ((pmode_cfg->auth >= 0) && (di->cert_succ_flag != pmode_cfg->auth))
		return false;

	return true;
}

static bool wlc_pmode_quick_judge_crit(struct wlrx_dev_info *di,
	struct wlrx_dts *dts, struct wlrx_pmode *pmode_cfg)
{
	if (!dts->product_cfg)
		return false;

	if ((di->tx_cap->vout_max < pmode_cfg->vtx_min) ||
		(dts->product_cfg->vtx < pmode_cfg->vtx) ||
		(dts->product_cfg->vrx < pmode_cfg->vrx) ||
		(dts->product_cfg->irx < pmode_cfg->irx))
		return false;

	if (di->tx_cap->vout_max * di->tx_cap->iout_max <
		pmode_cfg->vtx_min * pmode_cfg->itx_min)
		return false;

	return true;
}

static bool wlc_pmode_dc_judge_crit(struct wlrx_dev_info *di,
	struct wlrx_pmode *pmode_cfg, int pid)
{
	if (wldc_prev_check(pmode_cfg->name))
		return false;

	if ((wlrx_get_charge_stage() == WLRX_STAGE_REGULATION_DC) || g_in_wldc_check)
		return true;

	g_in_wldc_check = true;
	if (!wlc_formal_check_direct_charge(pmode_cfg->name)) {
		di->curr_pmode_index = pid;
	} else {
		g_in_wldc_check = false;
		return false;
	}
	g_in_wldc_check = false;

	return true;
}

bool wireless_charge_mode_judge_criterion(int pid, int crit_type)
{
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_pmode *pmode_cfg = NULL;

	if (!di || !dts) {
		hwlog_err("mode_judge_criterion: para null\n");
		return false;
	}
	if ((pid < 0) || (pid >= dts->pmode_cfg_level))
		return false;

	pmode_cfg = &dts->pmode_cfg[pid];
	switch (crit_type) {
	case WLDC_PMODE_FINAL_JUDGE:
	case WLRX_PMODE_FINAL_JUDGE:
		if (!wlc_pmode_final_judge_crit(di, pmode_cfg, pid))
			return false;
		/* fall-through */
	case WLRX_PMODE_NORMAL_JUDGE:
		if (!wlc_pmode_normal_judge_crit(di, pmode_cfg))
			return false;
		/* fall-through */
	case WLRX_PMODE_QUICK_JUDGE:
		if (!wlc_pmode_quick_judge_crit(di, dts, pmode_cfg))
			return false;
		break;
	default:
		hwlog_err("%s: crit_type = %d error\n", __func__, crit_type);
		return false;
	}

	if ((crit_type == WLRX_PMODE_FINAL_JUDGE) && strstr(dts->pmode_cfg[pid].name, "SC")) {
		if (!wlc_pmode_dc_judge_crit(di, pmode_cfg, pid))
			return false;
	}

	return true;
}

static void wireless_charge_dsm_dump(struct wlrx_dev_info *di, char *dsm_buff)
{
	int i;
	int tbatt = 0;
	char buff[ERR_NO_STRING_SIZE] = { 0 };
	int soc = coul_drv_battery_capacity();
	int vrect = 0;
	int vout = 0;
	int iout = 0;

	(void)wlrx_ic_get_vrect(WLTRX_IC_MAIN, &vrect);
	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &vout);
	(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &iout);
	(void)bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	snprintf(buff, sizeof(buff),
		"soc = %d, vrect = %dmV, vout = %dmV, iout = %dmA, iout_avg = %dmA, tbatt = %d\n",
		soc, vrect, vout, iout, di->iout_avg, tbatt);
	strncat(dsm_buff, buff, strlen(buff));
	snprintf(buff, ERR_NO_STRING_SIZE, "iout(mA): ");
	strncat(dsm_buff, buff, strlen(buff));
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++) {
		snprintf(buff, ERR_NO_STRING_SIZE, "%d ", rx_iout_samples[i]);
		strncat(dsm_buff, buff, strlen(buff));
	}
}

static void wireless_charge_dsm_report(struct wlrx_dev_info *di,
	int err_no, char *dsm_buff)
{
	if (wlrx_redef_tx_type(di->tx_cap->type) == TX_TYPE_QC) {
		hwlog_info("[%s] ignore err_no:%d, tx_type:%d\n", __func__,
			err_no, di->tx_cap->type);
		return;
	}
	msleep(di->monitor_interval);
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		wireless_charge_dsm_dump(di, dsm_buff);
		power_dsm_report_dmd(POWER_DSM_BATTERY, err_no, dsm_buff);
	}
}

static void wireless_charge_send_fan_status_uevent(int fan_status)
{
	power_ui_event_notify(POWER_UI_NE_WL_FAN_STATUS, &fan_status);
}

static bool wlc_is_high_pwr_wltx(struct wlrx_dev_info *di)
{
	int max_pwr_mw;

	if (!wlrx_is_rvs_tx(di->tx_cap->type))
		return false;

	max_pwr_mw = di->tx_cap->vout_max / MVOLT_PER_VOLT *
		di->tx_cap->iout_max;

	return max_pwr_mw >= WLC_HIGH_PWR_TH_FOR_WLTX;
}

static void wlc_revise_tx_cap(struct wlrx_dev_info *di)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || dts->support_high_pwr_wltx)
		return;

	if (wlc_is_high_pwr_wltx(di))
		di->tx_cap->no_need_cert = true;
}

static void wireless_charge_get_tx_capability(struct wlrx_dev_info *di)
{
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return;
	}
	wireless_get_tx_capability(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, di->tx_cap);
	wlc_revise_tx_cap(di);
	if (di->tx_cap->support_fan)
		wireless_charge_send_fan_status_uevent(1); /* 1: fan exist */
}

static void wireless_charge_get_tx_fop_range(struct wlrx_dev_info *di)
{
	if (!di->tx_cap->support_fop_range)
		return;
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return;
	}
	if (wireless_get_tx_fop_range(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, &di->tx_fop_range))
		hwlog_err("%s: get tx fop range fail\n", __func__);
}

static void wlc_ignore_qval_work(struct work_struct *work)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di)
		return;

	hwlog_info("[%s] timeout, permit SC mode\n", __func__);
	di->qval_support_mode = WLC_RX_SP_ALL_MODE;
}

static void wlc_preproccess_fod_status(struct wlrx_dev_info *di)
{
	hwlog_err("%s: tx_fod_err, forbid SC mode\n", __func__);
	di->qval_support_mode = WLC_RX_SP_BUCK_MODE;
	schedule_delayed_work(&di->ignore_qval_work,
		msecs_to_jiffies(30000)); /* 30s timeout to restore state */
}

static bool wlc_need_ignore_fod_status(struct wlrx_dev_info *di)
{
	int i;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || dts->ignore_qval <= 0)
		return false;

	for (i = 0; i < ARRAY_SIZE(g_qval_err_tx); i++) {
		if (di->tx_type == g_qval_err_tx[i])
			return true;
	}

	return false;
}

static void wireless_charge_send_fod_status(struct wlrx_dev_info *di)
{
	if (!di->tx_cap->support_fod_status) {
		hwlog_err("%s: tx not support fod_status detect\n", __func__);
		return;
	}
	if (wlc_need_ignore_fod_status(di)) {
		wlc_preproccess_fod_status(di);
		return;
	}
	wlrx_send_fod_status(WLTRX_DRV_MAIN, WIRELESS_PROTOCOL_QI);
}

static void wlrx_get_tx_prop(struct wlrx_dev_info *di)
{
	int tx_type;
	struct tx_dft_prop *dft_prop = NULL;

	tx_type = wlrx_redef_tx_type(di->tx_cap->type);
	dft_prop = wlrx_get_dft_tx_prop(tx_type);
	if (!dft_prop)
		return;

	if (di->tx_cap->vout_max <= 0) {
		di->tx_cap->vout_max = dft_prop->vout;
		hwlog_info("[get_tx_prop] set vmax=%d\n", di->tx_cap->vout_max);
	}

	if (di->tx_cap->iout_max <= 0) {
		di->tx_cap->iout_max = dft_prop->iout;
		hwlog_info("[get_tx_prop] set imax=%d\n", di->tx_cap->iout_max);
	}
}

static void wlc_reset_icon_pmode(struct wlrx_dev_info *di, struct wlrx_dts *dts)
{
	int i;

	for (i = 0; i < dts->pmode_cfg_level; i++)
		set_bit(i, &di->icon_pmode);
	hwlog_info("[%s] icon_pmode=0x%x", __func__, di->icon_pmode);
}

void wlc_clear_icon_pmode(int pmode)
{
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts)
		return;
	if ((pmode < 0) || (pmode >= dts->pmode_cfg_level))
		return;

	if (test_bit(pmode, &di->icon_pmode)) {
		clear_bit(pmode, &di->icon_pmode);
		hwlog_info("[%s] icon_pmode=0x%x", __func__, di->icon_pmode);
	}
}

static void wlc_revise_icon_display(struct wlrx_dev_info *di, int *icon_type)
{
	if (*icon_type != WIRELESS_SUPER_CHARGE_FLAG)
		return;

	if (wlc_is_high_pwr_wltx(di))
		*icon_type = WIRELESS_FAST_CHARGE_FLAG;
}

void wireless_charge_icon_display(int crit_type)
{
	int pmode;
	int icon_type;
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts) {
		hwlog_err("icon_display: para null\n");
		return;
	}

	for (pmode = dts->pmode_cfg_level - 1; pmode >= 0; pmode--) {
		if (test_bit(pmode, &di->icon_pmode) &&
			wireless_charge_mode_judge_criterion(pmode, crit_type))
			break;
	}

	if (pmode < 0) {
		pmode = 0;
		hwlog_err("icon_display: mismatched,set icon_mode=%s\n",
			dts->pmode_cfg[pmode].name);
	}

	icon_type = dts->pmode_cfg[pmode].icon;
	wlc_revise_icon_display(di, &icon_type);
	wireless_charge_send_charge_uevent(di, icon_type);
}

void wlc_ignore_vbus_only_event(bool ignore_flag)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || !dts->bst5v_ignore_vbus_only)
		return;

#ifdef CONFIG_TCPC_CLASS
	if (ignore_flag)
		pd_dpm_ignore_vbus_only_event(true);
	else if (!boost_5v_status(BOOST_CTRL_WLC) && !boost_5v_status(BOOST_CTRL_WLDC))
		pd_dpm_ignore_vbus_only_event(false);
#endif
}

static void wlc_extra_power_supply(bool enable)
{
	int ret;
	static bool boost_5v_flag;
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts || !dts->hvc_need_5vbst)
		return;

	if (enable && (di->supported_rx_vout > RX_HIGH_VOUT)) {
		ret = boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_WLC);
		if (ret) {
			hwlog_err("%s: boost_5v enable fail\n", __func__);
			di->extra_pwr_good_flag = 0;
			return;
		}
		wlc_ignore_vbus_only_event(true);
		boost_5v_flag = true;
	} else if (!enable && boost_5v_flag) {
		ret = boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_WLC);
		if (ret) {
			hwlog_err("%s: boost_5v disable fail\n", __func__);
			return;
		}
		wlc_ignore_vbus_only_event(false);
		boost_5v_flag = false;
	}
}

static void wlc_get_supported_max_rx_vout(struct wlrx_dev_info *di, struct wlrx_dts *dts)
{
	int pmode_index = dts->pmode_cfg_level - 1;

	for (; pmode_index >= 0; pmode_index--) {
		if (wireless_charge_mode_judge_criterion(pmode_index, WLRX_PMODE_QUICK_JUDGE))
			break;
	}
	if (pmode_index < 0)
		pmode_index = 0;

	di->supported_rx_vout = dts->pmode_cfg[pmode_index].vrx;
	hwlog_info("[%s] rx_support_mode = 0x%x\n", __func__,
		wlc_get_rx_support_mode());
}

static u8 *wlc_get_tx_fw_version(struct wlrx_dev_info *di)
{
	if (g_tx_fw_version)
		return g_tx_fw_version;

	return wireless_get_tx_fw_version(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI);
}

static void wireless_charge_get_tx_info(struct wlrx_dev_info *di)
{
	if (!di->standard_tx || wlrx_is_fac_tx(di->tx_cap->type))
		return;

	g_tx_fw_version = wlc_get_tx_fw_version(di);
	di->tx_type = wireless_get_tx_type(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI);
	hwlog_info("[%s] tx_fw_version = %s, tx_type = %d\n", __func__,
		g_tx_fw_version, di->tx_type);
	wireless_get_tx_bigdata_info(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI);
}

static void wireless_charge_set_default_tx_capability(struct wlrx_dev_info *di)
{
	di->tx_cap->type = TX_TYPE_ERR;
	di->tx_cap->vout_max = ADAPTER_5V * MVOLT_PER_VOLT;
	di->tx_cap->iout_max = CHARGE_CURRENT_1000_MA;
	di->tx_cap->can_boost = 0;
	di->tx_cap->cable_ok = 0;
	di->tx_cap->no_need_cert = 0;
	di->tx_cap->support_scp = 0;
	di->tx_cap->support_extra_cap = 0;
	/* extra cap */
	di->tx_cap->support_fan = 0;
	di->tx_cap->support_tec = 0;
	di->tx_cap->support_get_ept = 0;
	di->tx_cap->support_fod_status = 0;
	di->tx_cap->support_fop_range = 0;
	di->tx_fop_range.base_min = 0;
	di->tx_fop_range.base_max = 0;
	di->tx_fop_range.ext1_min = 0;
	di->tx_fop_range.ext1_max = 0;
	di->tx_fop_range.ext2_min = 0;
	di->tx_fop_range.ext2_max = 0;
}

static void wlc_send_cert_confirm_msg(struct wlrx_dev_info *di, bool cert_flag)
{
	if (!wlrx_is_fac_tx(di->tx_cap->type))
		return;

	if (!wireless_send_cert_confirm(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, cert_flag))
		hwlog_info("%s: succ\n", __func__);
}

static int wlc_tx_certification(struct wlrx_dev_info *di)
{
	int i;
	int ret;
	struct wlrx_dts *dts = NULL;
	u8 *af_key = wlrx_auth_get_hash_data_header();
	unsigned int af_len = wlrx_auth_get_hash_data_size();

	if (!af_key) {
		hwlog_err("get hash data header fail\n");
		return -ENXIO;
	}

	if (af_len != (WLRX_AUTH_RANDOM_LEN + WLRX_AUTH_TX_KEY_LEN)) {
		hwlog_err("hash len error\n");
		return -ENXIO;
	}

	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}

	dts = wlrx_get_dts();
	if (!dts)
		return -ENXIO;

	hwlog_info("antifake_kid=%d\n", dts->antifake_kid);
	ret = wireless_auth_encrypt_start(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, dts->antifake_kid,
		random, WLRX_AUTH_RANDOM_LEN, tx_cipherkey, WLRX_AUTH_TX_KEY_LEN);
	if (ret) {
		hwlog_err("%s: get hash from tx fail\n", __func__);
		di->certi_comm_err_cnt++;
		return ret;
	}
	di->certi_comm_err_cnt = 0;

	wlrx_auth_clean_hash_data();
	for (i = 0; i < WLRX_AUTH_RANDOM_LEN; i++)
		af_key[i] = random[i];
	for (i = 0; i < WLRX_AUTH_TX_KEY_LEN; i++)
		af_key[WLRX_AUTH_RANDOM_LEN + i] = tx_cipherkey[i];

	ret = wlrx_auth_wait_completion();
	wlrx_auth_clean_hash_data();
	if (ret) {
		di->cert_succ_flag = WIRELESS_CHECK_FAIL;
		wlrx_set_charge_stage(WLRX_STAGE_FW_UPDATE);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static void wireless_charge_set_ctrl_interval(struct wlrx_dev_info *di)
{
	if (wlrx_get_charge_stage() < WLRX_STAGE_REGULATION)
		di->ctrl_interval = CONTROL_INTERVAL_NORMAL;
	else
		di->ctrl_interval = CONTROL_INTERVAL_FAST;
}

void wireless_charge_chip_init(int tx_vset)
{
	int ret;
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	ret = wlrx_ic_chip_init(WLTRX_IC_MAIN, tx_vset, di->tx_type);
	if (ret < 0)
		hwlog_err("%s: rx chip init failed\n", __func__);
}

static void wlc_set_iout_min(struct wlrx_dev_info *di)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	di->rx_iout_max = dts->rx_imin;
	wireless_charge_set_input_current(di);
}

int wireless_charge_select_vout_mode(int vout)
{
	int id = 0;
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts) {
		hwlog_err("select_vout_mode: di or dts is null\n");
		return id;
	}

	for (id = 0; id < dts->vmode_cfg_level; id++) {
		if (vout == dts->vmode_cfg[id].vtx)
			break;
	}
	if (id >= dts->vmode_cfg_level) {
		id = 0;
		hwlog_err("select_vout_mode: match vmode_index failed\n");
	}
	return id;
}

static bool wireless_charger_tx_fop_support(struct wlrx_dev_info *di, int fop)
{
	struct wireless_protocol_tx_fop_range *fop_range = &di->tx_fop_range;

	if ((!di->tx_cap->support_fop_range && (fop > NORMAL_FOP_MAX)) ||
		(di->tx_cap->support_fop_range && (fop > 0) &&
		!(((fop >= fop_range->base_min) && (fop <= fop_range->base_max)) ||
		((fop >= fop_range->ext1_min) && (fop <= fop_range->ext1_max)) ||
		((fop >= fop_range->ext2_min) && (fop <= fop_range->ext2_max)))))
		return false;

	return true;
}

static void wlc_update_high_fop_para(struct wlrx_dev_info *di)
{
	int fop = 0;

	(void)wlrx_ic_get_fop(WLTRX_IC_MAIN, &fop);
	if (di->tx_cap->support_fop_range && (fop > NORMAL_FOP_MAX)) {
		di->tx_vout_max = min(di->tx_vout_max, VOUT_9V_STAGE_MAX);
		di->rx_vout_max = min(di->rx_vout_max, VOUT_9V_STAGE_MAX);
	}
}

static void wlc_update_iout_low_para(struct wlrx_dev_info *di, bool ignore_cnt_flag)
{
	if (ignore_cnt_flag || (di->monitor_interval == 0))
		return;
	if (di->iout_low_cnt < RX_AVG_IOUT_TIME / di->monitor_interval)
		return;
	if (di->tx_cap->support_fop_range &&
		(wlrx_get_intfr_fixed_fop() >= NORMAL_FOP_MAX))
		return;
	di->tx_vout_max = min(di->tx_vout_max, TX_DEFAULT_VOUT);
	di->rx_vout_max = min(di->rx_vout_max, RX_DEFAULT_VOUT);
	di->rx_iout_max = min(di->rx_iout_max, RX_DEFAULT_IOUT);
}

static void wlc_update_tx_alarm_vmax(struct wlrx_dev_info *di)
{
	int vmax;

	vmax = wlrx_get_alarm_vlim();
	if (vmax <= 0)
		return;

	di->tx_vout_max = min(di->tx_vout_max, vmax);
	di->rx_vout_max = min(di->rx_vout_max, vmax);
}

void wireless_charge_update_max_vout_and_iout(bool ignore_cnt_flag)
{
	int mode = VBUS_CH_NOT_IN_OTG_MODE;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di || !dts || !dts->product_cfg)
		return;

	di->tx_vout_max = dts->product_cfg->vtx;
	di->rx_vout_max = dts->product_cfg->vrx;
	di->rx_iout_max = dts->product_cfg->irx;
	di->pctrl->vtx = dts->product_cfg->vtx;
	di->pctrl->vrx = dts->product_cfg->vrx;
	di->pctrl->irx = dts->product_cfg->irx;

	vbus_ch_get_mode(VBUS_CH_USER_WR_TX, VBUS_CH_TYPE_BOOST_GPIO, &mode);
	if ((mode == VBUS_CH_IN_OTG_MODE) || di->pwroff_reset_flag ||
		!di->extra_pwr_good_flag) {
		di->tx_vout_max = min(di->tx_vout_max, TX_DEFAULT_VOUT);
		di->rx_vout_max = min(di->rx_vout_max, RX_DEFAULT_VOUT);
		di->rx_iout_max = min(di->rx_iout_max, RX_DEFAULT_IOUT);
	}
	wlrx_update_pctrl(di->pctrl);
	wlc_update_tx_alarm_vmax(di);
	wlc_update_high_fop_para(di);
	wlc_update_iout_low_para(di, ignore_cnt_flag);
	di->tx_vout_max = min(di->tx_vout_max, di->pctrl->vtx);
	di->rx_vout_max = min(di->rx_vout_max, di->pctrl->vrx);
	di->rx_iout_max = min(di->rx_iout_max, di->pctrl->irx);
}

static void wlc_notify_charger_vout(struct wlrx_dev_info *di)
{
	int tx_vout;
	int cp_vout;
	int cp_ratio;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	tx_vout = dts->vmode_cfg[di->curr_vmode_index].vtx;
	cp_ratio = charge_pump_get_cp_ratio(CP_TYPE_MAIN);
	if (cp_ratio <= 0) {
		hwlog_err("%s: cp_ratio err\n", __func__);
		return;
	}
	hwlog_info("[%s] cp_ratio=%d\n", __func__, cp_ratio);
	cp_vout = tx_vout / cp_ratio;
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_CHARGER_VBUS, &cp_vout);
}

static void wlc_send_bst_succ_msg(struct wlrx_dev_info *di)
{
	if (!wlrx_is_fac_tx(di->tx_cap->type) ||
		(wlrx_get_charge_stage() != WLRX_STAGE_CHARGING))
		return;

	if (!wireless_send_rx_boost_succ(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI))
		hwlog_info("[%s] send cmd boost_succ ok\n", __func__);
}

static void wlc_report_bst_fail_dmd(struct wlrx_dev_info *di)
{
	static bool dsm_report_flag;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	if (++di->boost_err_cnt < BOOST_ERR_CNT_MAX) {
		dsm_report_flag = false;
		return;
	}

	di->boost_err_cnt = BOOST_ERR_CNT_MAX;
	if (dsm_report_flag)
		return;

	wireless_charge_dsm_report(di, ERROR_WIRELESS_BOOSTING_FAIL, dsm_buff);
	dsm_report_flag = true;
}

static int wireless_charge_boost_vout(struct wlrx_dev_info *di,
	int cur_vmode_id, int target_vmode_id)
{
	int vmode;
	int ret;
	int tx_vout;
	int bst_delay = RX_BST_DELAY_TIME;
	int last_vfc_reg = 0;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return -ENODEV;

	if (di->boost_err_cnt >= BOOST_ERR_CNT_MAX) {
		hwlog_debug("%s: boost fail exceed %d times\n",
			__func__, BOOST_ERR_CNT_MAX);
		return -WLC_ERR_CHECK_FAIL;
	}

	wlc_set_iout_min(di);
	(void)power_msleep(WLRX_ILIM_DELAY, DT_MSLEEP_25MS, wlrx_msleep_exit);
	g_bst_rst_complete = false;
	wlrx_ic_get_bst_delay_time(WLTRX_IC_MAIN, &bst_delay);

	for (vmode = cur_vmode_id + 1; vmode <= target_vmode_id; vmode++) {
		(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &last_vfc_reg);
		tx_vout = dts->vmode_cfg[vmode].vtx;
		ret = wireless_charge_set_tx_vout(tx_vout);
		if (ret) {
			hwlog_err("%s: boost fail\n", __func__);
			wlc_report_bst_fail_dmd(di);
			(void)wireless_charge_set_tx_vout(last_vfc_reg);
			g_bst_rst_complete = true;
			return ret;
		}
		wlc_set_cur_vmode_id(di, vmode);
		wlc_notify_charger_vout(di);
		wlc_set_iout_min(di);
		if (vmode != target_vmode_id)
			(void)power_msleep(bst_delay, DT_MSLEEP_25MS,
				wlrx_msleep_exit);
	}

	g_bst_rst_complete = true;
	di->boost_err_cnt = 0;
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_TX_VSET, &tx_vout);

	wlc_send_bst_succ_msg(di);
	return 0;
}

static void wireless_charge_wait_fop_fix_to_default(void)
{
	int i;

	hwlog_info("%s\n", __func__);
	g_need_force_5v_vout = true;
	/* delay 60*50 = 3000ms for direct charger check finish */
	for (i = 0; i < 60; i++) {
		if (g_fop_fixed_flag <= NORMAL_FOP_MAX)
			break;
		if (!power_msleep(DT_MSLEEP_50MS, DT_MSLEEP_25MS,
			wlrx_msleep_exit))
			break;
	}
}

static int wireless_charge_reset_vout(struct wlrx_dev_info *di,
	int cur_vmode_id, int target_vmode_id)
{
	int ret;
	int vmode;
	int tx_vout;
	int last_vfc_reg = 0;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return -ENODEV;

	wlc_set_iout_min(di);
	(void)power_msleep(WLRX_ILIM_DELAY, DT_MSLEEP_25MS, wlrx_msleep_exit);
	g_bst_rst_complete = false;

	for (vmode = cur_vmode_id - 1; vmode >= target_vmode_id; vmode--) {
		(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &last_vfc_reg);
		tx_vout = dts->vmode_cfg[vmode].vtx;
		if ((tx_vout < RX_HIGH_VOUT) && (g_fop_fixed_flag > NORMAL_FOP_MAX))
			wireless_charge_wait_fop_fix_to_default();
		ret = wireless_charge_set_tx_vout(tx_vout);
		if (ret) {
			hwlog_err("%s: reset fail\n", __func__);
			(void)wireless_charge_set_tx_vout(last_vfc_reg);
			g_bst_rst_complete = true;
			return ret;
		}
		wlc_set_cur_vmode_id(di, vmode);
		wlc_notify_charger_vout(di);
		wlc_set_iout_min(di);
	}

	g_bst_rst_complete = true;
	return 0;
}

static int wireless_charge_set_vout(int cur_vmode_index, int target_vmode_index)
{
	int ret;
	int tx_vout;
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts) {
		hwlog_err("set_vout: di or dts is null\n");
		return -ENODEV;
	}

	tx_vout = dts->vmode_cfg[target_vmode_index].vtx;
	if (target_vmode_index > cur_vmode_index)
		ret = wireless_charge_boost_vout(di,
			cur_vmode_index, target_vmode_index);
	else if (target_vmode_index < cur_vmode_index)
		ret = wireless_charge_reset_vout(di,
			cur_vmode_index, target_vmode_index);
	else
		return wireless_charge_set_rx_vout(tx_vout);

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired vbus connect\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: wireless vbus disconnect\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}

	if (di->curr_vmode_index == cur_vmode_index)
		return ret;

	tx_vout = dts->vmode_cfg[di->curr_vmode_index].vtx;
	wireless_charge_chip_init(tx_vout);
	wlc_notify_charger_vout(di);

	return ret;
}

int wldc_set_trx_vout(int vout)
{
	int cur_vmode;
	int target_vmode;
	int vfc_reg = 0;

	if (wlrx_ic_is_sleep_enable(WLTRX_IC_MAIN)) {
		hwlog_info("set_trx_vout: sleep_en eanble, return\n");
		return -ENXIO;
	}

	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	cur_vmode = wireless_charge_select_vout_mode(vfc_reg);
	target_vmode = wireless_charge_select_vout_mode(vout);

	return wireless_charge_set_vout(cur_vmode, target_vmode);
}

static int wireless_charge_vout_control(struct wlrx_dev_info *di, int pmode_index)
{
	int ret;
	int tx_vout;
	int target_vout;
	int curr_vmode_index;
	int target_vmode_index;
	int vfc_reg = 0;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return -ENODEV;

	if (strstr(dts->pmode_cfg[pmode_index].name, "SC"))
		return 0;
	if (wlrx_get_wireless_channel_state() != WIRELESS_CHANNEL_ON)
		return -1;
	if (wlrx_ic_is_sleep_enable(WLTRX_IC_MAIN)) {
		hwlog_info("vout_control: sleep_en eanble, return\n");
		return -ENXIO;
	}
	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	tx_vout = dts->vmode_cfg[di->curr_vmode_index].vtx;
	if (vfc_reg != tx_vout) {
		hwlog_err("%s: vfc_reg %dmV != cur_mode_vout %dmV\n", __func__,
			vfc_reg, tx_vout);
		ret = wireless_charge_set_tx_vout(tx_vout);
		if (ret)
			hwlog_err("%s: set tx vout fail\n", __func__);
	}
	target_vout = dts->pmode_cfg[pmode_index].vtx;
	target_vmode_index = wireless_charge_select_vout_mode(target_vout);
	curr_vmode_index = di->curr_vmode_index;
	di->tx_vout_max = min(di->tx_vout_max, dts->pmode_cfg[pmode_index].vtx);
	di->rx_vout_max = min(di->rx_vout_max, dts->pmode_cfg[pmode_index].vrx);
	ret = wireless_charge_set_vout(curr_vmode_index, target_vmode_index);
	if (ret)
		return ret;
	if (di->curr_vmode_index != curr_vmode_index)
		return 0;
	tx_vout = dts->vmode_cfg[di->curr_vmode_index].vtx;
	wireless_charge_chip_init(tx_vout);
	wlc_notify_charger_vout(di);

	return 0;
}

static void wlc_update_imax_by_tx_plimit(struct wlrx_dev_info *di)
{
	int ilim;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	ilim = wlrx_get_alarm_ilim(dts->pmode_cfg[di->curr_pmode_index].vrx);
	if (ilim <= 0)
		return;

	di->rx_iout_max = min(di->rx_iout_max, ilim);
}

static int wlc_start_sample_iout(struct wlrx_dev_info *di)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || !wlrx_is_fac_tx(di->tx_cap->type))
		return -EINVAL;

	if (!delayed_work_pending(&di->rx_sample_work))
		mod_delayed_work(system_wq, &di->rx_sample_work,
			msecs_to_jiffies(dts->sample_delay_time));

	if (g_wlc_start_sample_flag) {
		di->rx_iout_limit = di->rx_iout_max;
		wireless_charge_set_input_current(di);
		return 0;
	}

	return -EINVAL;
}

static void wlc_revise_vout_para(struct wlrx_dev_info *di, struct wlrx_dts *dts)
{
	int ret;
	int vfc_reg = 0;
	int rx_vout_reg = 0;

	if ((wlrx_get_charge_stage() == WLRX_STAGE_REGULATION_DC) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	(void)wlrx_ic_get_vout_reg(WLTRX_IC_MAIN, &rx_vout_reg);

	if ((vfc_reg <= dts->pmode_cfg[di->curr_pmode_index].vtx - RX_VREG_OFFSET) ||
		(vfc_reg >= dts->pmode_cfg[di->curr_pmode_index].vtx + RX_VREG_OFFSET)) {
		hwlog_err("%s: revise tx_vout\n", __func__);
		ret = wireless_charge_set_tx_vout(dts->pmode_cfg[di->curr_pmode_index].vtx);
		if (ret)
			hwlog_err("%s: set tx vout fail\n", __func__);
	}

	if ((rx_vout_reg <= dts->pmode_cfg[di->curr_pmode_index].vrx - RX_VREG_OFFSET) ||
		(rx_vout_reg >= dts->pmode_cfg[di->curr_pmode_index].vrx + RX_VREG_OFFSET)) {
		hwlog_err("%s: revise rx_vout\n", __func__);
		ret = wireless_charge_set_rx_vout(dts->pmode_cfg[di->curr_pmode_index].vrx);
		if (ret)
			hwlog_err("%s: set rx vout fail\n", __func__);
	}
}

static void wlc_update_ilim_by_low_vrect(struct wlrx_dev_info *di)
{
	static int rx_vrect_low_cnt;
	int cnt_max;
	int vrect = 0;
	int vrect_lth;
	int charger_iin_regval = charge_get_charger_iinlim_regval();
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || di->ctrl_interval <= 0)
		return;

	(void)wlrx_ic_get_vrect(WLTRX_IC_MAIN, &vrect);
	vrect_lth = dts->pmode_cfg[di->curr_pmode_index].vrect_lth;
	cnt_max = RX_VRECT_LOW_RESTORE_TIME / di->ctrl_interval;
	if (vrect < vrect_lth) {
		if (++rx_vrect_low_cnt >= RX_VRECT_LOW_CNT) {
			rx_vrect_low_cnt = RX_VRECT_LOW_CNT;
			hwlog_err("update_ilim_by_low_vrect: vrect:%d<lth:%d,decrease irx:%d\n",
				vrect, vrect_lth, dts->rx_istep);
			di->rx_iout_limit = max(RX_VRECT_LOW_IOUT_MIN,
				charger_iin_regval - dts->rx_istep);
			g_rx_vrect_restore_cnt = cnt_max;
		}
	} else if (g_rx_vrect_restore_cnt > 0) {
		rx_vrect_low_cnt = 0;
		g_rx_vrect_restore_cnt--;
		di->rx_iout_limit = charger_iin_regval;
	} else {
		rx_vrect_low_cnt = 0;
	}
}

static void wlc_update_iout_ctrl_para(struct wlrx_dev_info *di)
{
	int i;
	struct wireless_iout_ctrl_para *ictrl_para = NULL;

	for (i = 0; i < di->iout_ctrl_data.ictrl_para_level; i++) {
		ictrl_para = &di->iout_ctrl_data.ictrl_para[i];
		if ((di->iout_avg >= ictrl_para->iout_min) &&
			(di->iout_avg < ictrl_para->iout_max)) {
			di->rx_iout_limit = ictrl_para->iout_set;
			break;
		}
	}
}

static void wlc_update_iout_para(struct wlrx_dev_info *di)
{
	wlc_update_iout_ctrl_para(di);
	wlc_update_ilim_by_low_vrect(di);
	wlc_update_imax_by_tx_plimit(di);
}

static void wlc_iout_control(struct wlrx_dev_info *di, struct wlrx_dts *dts)
{
	int ret;

	if ((wlrx_get_charge_stage() == WLRX_STAGE_REGULATION_DC) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	if (di->pwroff_reset_flag)
		return;

	di->rx_iout_max = min(di->rx_iout_max, dts->pmode_cfg[di->curr_pmode_index].irx);
	di->rx_iout_max = min(di->rx_iout_max, di->tx_cap->iout_max);
	di->rx_iout_limit = di->rx_iout_max;

	ret = wlc_start_sample_iout(di);
	if (!ret)
		return;

	wlc_update_iout_para(di);
	wireless_charge_set_input_current(di);
}

static int wlc_high_fop_vout_check(void)
{
	int rx_vout = 0;
	int vfc_reg = 0;

	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &rx_vout);
	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	if ((rx_vout < VOUT_9V_STAGE_MIN) || (rx_vout > VOUT_9V_STAGE_MAX) ||
		(vfc_reg < VOUT_9V_STAGE_MIN) || (vfc_reg > VOUT_9V_STAGE_MAX))
		return -1;

	return 0;
}

static int wireless_charge_fop_fix_check(bool force_flag)
{
	if ((wlrx_get_intfr_fixed_fop() <= 0) ||
		(g_fop_fixed_flag == wlrx_get_intfr_fixed_fop()))
		return 0;

	/* reset tx to 9V for high fop; else delay 40*100ms for limit iout */
	if (wlrx_get_intfr_fixed_fop() >= NORMAL_FOP_MAX) {
		if (wlc_high_fop_vout_check())
			return -EINVAL;
	} else if (!force_flag && (g_plimit_time_num < 40)) {
		g_plimit_time_num++;
		return -EINVAL;
	}
	if (wireless_charge_fix_tx_fop(wlrx_get_intfr_fixed_fop())) {
		hwlog_err("fop_fix_check: fixed failed\n");
		return -EINVAL;
	}

	g_fop_fixed_flag = wlrx_get_intfr_fixed_fop();
	g_plimit_time_num = 0;
	return 0;
}

static int wireless_charge_fop_unfix_check(void)
{
	if (g_fop_fixed_flag <= 0)
		return 0;
	if ((wlrx_get_intfr_fixed_fop() > 0) && !g_need_force_5v_vout)
		return 0;
	if (wireless_charge_unfix_tx_fop()) {
		hwlog_err("fop_unfix_check: unfix failed\n");
		return -EINVAL;
	}

	g_fop_fixed_flag = 0;
	g_need_force_5v_vout = false;
	return 0;
}

static void wireless_charge_update_fop(struct wlrx_dev_info *di, bool force_flag)
{
	if (!di->standard_tx) {
		hwlog_debug("%s: not standard tx, don't update fop\n", __func__);
		return;
	}
	if (!force_flag && (wlrx_get_charge_stage() <= WLRX_STAGE_CHARGING))
		return;
	if (!wireless_charger_tx_fop_support(di, wlrx_get_intfr_fixed_fop()))
		return;
	if (wireless_charge_fop_fix_check(force_flag))
		return;
	if (wireless_charge_fop_unfix_check())
		return;
}

static void wlc_update_charge_state(struct wlrx_dev_info *di)
{
	int ret;
	int soc;
	static int retry_cnt;

	if (!di->standard_tx || !wlrx_ic_is_tx_exist(WLTRX_IC_MAIN) ||
		(wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON))
		return;

	if (wlrx_get_charge_stage() <= WLRX_STAGE_CHARGING) {
		retry_cnt = 0;
		return;
	}

	soc = coul_drv_battery_capacity();
	if (soc >= CAPACITY_FULL)
		di->stat_rcd.chrg_state_cur |= WIRELESS_STATE_CHRG_FULL;
	else
		di->stat_rcd.chrg_state_cur &= ~WIRELESS_STATE_CHRG_FULL;

	if (di->stat_rcd.chrg_state_cur != di->stat_rcd.chrg_state_last) {
		if (retry_cnt >= WLC_SEND_CHARGE_STATE_RETRY_CNT) {
			retry_cnt = 0;
			di->stat_rcd.chrg_state_last =
				di->stat_rcd.chrg_state_cur;
			return;
		}
		hwlog_info("[%s] charge_state=%d\n",
			__func__, di->stat_rcd.chrg_state_cur);
		ret = wireless_send_charge_state(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
			di->stat_rcd.chrg_state_cur);
		if (ret) {
			hwlog_err("%s: send charge_state fail\n", __func__);
			retry_cnt++;
			return;
		}
		retry_cnt = 0;
		di->stat_rcd.chrg_state_last = di->stat_rcd.chrg_state_cur;
	}
}

static void wlc_check_voltage(struct wlrx_dev_info *di)
{
	int vout = 0;
	int vout_reg = 0;
	int vfc_reg = 0;
	int vbus = charge_get_vbus();
	int cnt_max = RX_VOUT_ERR_CHECK_TIME / di->monitor_interval;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &vout);
	if ((vout <= 0) || !g_bst_rst_complete ||
		(wlrx_get_charge_stage() < WLRX_STAGE_HANDSHAKE))
		return;

	vout = (vout >= vbus) ? vout : vbus;
	(void)wlrx_ic_get_vout_reg(WLTRX_IC_MAIN, &vout_reg);
	if (vout >= vout_reg * dts->rx_vout_err_ratio / PERCENT) {
		g_rx_vout_err_cnt = 0;
		return;
	}

	if (di->iout_avg >= RX_EPT_IGNORE_IOUT)
		return;

	hwlog_err("%s: abnormal vout=%dmV", __func__, vout);
	if (++g_rx_vout_err_cnt < cnt_max)
		return;

	g_rx_vout_err_cnt = cnt_max;
	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	if (!wlrx_ic_is_sleep_enable(WLTRX_IC_MAIN) &&
		(vfc_reg >= RX_HIGH_VOUT2)) {
		wlrx_set_plim_src(WLRX_PLIM_SRC_VOUT_ERR);
		hwlog_err("%s: high vout err\n", __func__);
		return;
	}
	hwlog_info("[%s] vout lower than %d*%d%%mV for %dms, send EPT\n",
		__func__, vout_reg, dts->rx_vout_err_ratio,
		RX_VOUT_ERR_CHECK_TIME);
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON)
		wlrx_ic_send_ept(WLTRX_IC_MAIN, WIRELESS_EPT_ERR_VOUT);
}

void wlc_set_high_pwr_test_flag(bool flag)
{
	g_high_pwr_test_flag = flag;

	if (g_high_pwr_test_flag) {
		wlrx_clear_plim_src(WLRX_PLIM_SRC_THERMAL);
		wlrx_clear_intfr_settings();
	}
}

bool wlc_get_high_pwr_test_flag(void)
{
	return g_high_pwr_test_flag;
}

static bool wlc_is_night_time(struct wlrx_dev_info *di)
{
	struct timeval tv;
	struct rtc_time tm;

	if (di->sysfs_data.ignore_fan_ctrl)
		return false;
	if (g_high_pwr_test_flag)
		return false;
	if (wlrx_is_car_tx(di->tx_cap->type))
		return false;

	do_gettimeofday(&tv); /* seconds since 1970-01-01 00:00:00 */
	tv.tv_sec -= sys_tz.tz_minuteswest * 60; /* GMT, min to sec */
	rtc_time_to_tm(tv.tv_sec, &tm);

	/* night time: 21:00-7:00 */
	if ((tm.tm_hour >= 21) || (tm.tm_hour < 7))
		return true;

	return false;
}

static void wlc_fan_control_handle(struct wlrx_dev_info *di,
	int *retry_cnt, u8 limit_val)
{
	int ret;

	if (*retry_cnt >= WLC_FAN_LIMIT_RETRY_CNT) {
		*retry_cnt = 0;
		di->stat_rcd.fan_last = di->stat_rcd.fan_cur;
		return;
	}

	hwlog_info("[%s] limit_val=0x%x\n", __func__, limit_val);
	ret = wireless_set_fan_speed_limit(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, limit_val);
	if (ret) {
		(*retry_cnt)++;
		return;
	}
	*retry_cnt = 0;
	di->stat_rcd.fan_last = di->stat_rcd.fan_cur;
}

static bool wlc_is_need_fan_control(struct wlrx_dev_info *di)
{
	if (!di->standard_tx || !di->tx_cap->support_fan)
		return false;

	/* in charger mode, time zone is not available */
	if (power_cmdline_is_factory_mode() ||
		power_cmdline_is_powerdown_charging_mode())
		return false;

	return true;
}

static void wlc_update_fan_control(struct wlrx_dev_info *di, bool force_flag)
{
	static int retry_cnt;
	int tx_pwr;
	u8 thermal_status;
	u8 fan_limit;

	if (!wlc_is_need_fan_control(di))
		return;
	if (!wlrx_ic_is_tx_exist(WLTRX_IC_MAIN) ||
		(wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON))
		return;
	if (!force_flag &&
		(wlrx_get_charge_stage() <= WLRX_STAGE_CHARGING)) {
		retry_cnt = 0;
		return;
	}

	thermal_status = di->sysfs_data.thermal_ctrl &
		WLC_THERMAL_FORCE_FAN_FULL_SPEED;
	tx_pwr = di->tx_cap->vout_max * di->tx_cap->iout_max;
	if (wlc_is_night_time(di)) {
		di->stat_rcd.fan_cur = WLC_FAN_HALF_SPEED_MAX;
		wlrx_set_plim_src(WLRX_PLIM_SRC_FAN);
	} else if (tx_pwr <= WLC_FAN_CTRL_PWR) {
		di->stat_rcd.fan_cur = WLC_FAN_FULL_SPEED_MAX;
		wlrx_clear_plim_src(WLRX_PLIM_SRC_FAN);
	} else if (thermal_status == WLC_THERMAL_FORCE_FAN_FULL_SPEED) {
		di->stat_rcd.fan_cur = WLC_FAN_FULL_SPEED;
		wlrx_clear_plim_src(WLRX_PLIM_SRC_FAN);
	} else {
		di->stat_rcd.fan_cur = WLC_FAN_FULL_SPEED_MAX;
		wlrx_clear_plim_src(WLRX_PLIM_SRC_FAN);
	}

	if (di->stat_rcd.fan_last != di->stat_rcd.fan_cur) {
		switch (di->stat_rcd.fan_cur) {
		case WLC_FAN_HALF_SPEED_MAX:
			fan_limit = WLC_FAN_HALF_SPEED_MAX_QI;
			break;
		case WLC_FAN_FULL_SPEED_MAX:
			fan_limit = WLC_FAN_FULL_SPEED_MAX_QI;
			break;
		case WLC_FAN_FULL_SPEED:
			fan_limit = WLC_FAN_FULL_SPEED_QI;
			break;
		default:
			return;
		}
		wlc_fan_control_handle(di, &retry_cnt, fan_limit);
	}
}

static void wireless_charge_update_status(struct wlrx_dev_info *di)
{
	wireless_charge_update_fop(di, false);
	wlc_update_charge_state(di);
	wlc_update_fan_control(di, false);
}

static int wireless_charge_set_power_mode(struct wlrx_dev_info *di, struct wlrx_dts *dts, int pid)
{
	int ret;

	if ((pid < 0) || (pid >= dts->pmode_cfg_level))
		return -EINVAL;

	ret = wireless_charge_vout_control(di, pid);
	if (!ret) {
		if (pid != di->curr_pmode_index) {
			if (dts->pmode_cfg[pid].timeout > 0)
				di->curr_power_time_out = jiffies + msecs_to_jiffies(
					dts->pmode_cfg[pid].timeout * WL_MSEC_PER_SEC);
			di->curr_pmode_index = pid;
			if (wireless_charge_set_rx_vout(dts->pmode_cfg[di->curr_pmode_index].vrx))
				hwlog_err("%s: set rx vout fail\n", __func__);
		}
	}
	return ret;
}

static void wlrx_switch_power_mode(struct wlrx_dev_info *di, int start_id, int end_id)
{
	int ret;
	int p_id;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	if ((wlrx_get_charge_stage() != WLRX_STAGE_CHARGING) &&
		g_wlc_start_sample_flag) {
		hwlog_debug("%s: start sample, don't sw pmode\n", __func__);
		return;
	}
	if ((start_id < 0) || (end_id < 0))
		return;

	for (p_id = start_id; p_id >= end_id; p_id--) {
		if (!wireless_charge_mode_judge_criterion(p_id, WLRX_PMODE_FINAL_JUDGE))
			continue;
		if (strstr(dts->pmode_cfg[p_id].name, "SC"))
			return;
		ret = wireless_charge_set_power_mode(di, dts, p_id);
		if (!ret)
			break;
	}
	if (p_id < 0) {
		di->curr_pmode_index = 0;
		wireless_charge_set_power_mode(di, dts, di->curr_pmode_index);
	}
}

static void wireless_charge_power_mode_control(struct wlrx_dev_info *di)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	if (wireless_charge_mode_judge_criterion(di->curr_pmode_index, WLRX_PMODE_FINAL_JUDGE)) {
		if (wlrx_get_charge_stage() == WLRX_STAGE_CHARGING)
			wlrx_switch_power_mode(di, dts->pmode_cfg_level - 1, 0);
		else
			wlrx_switch_power_mode(di, dts->pmode_cfg[di->curr_pmode_index].expect_mode,
				di->curr_pmode_index + 1);
	} else {
		wlrx_switch_power_mode(di, di->curr_pmode_index - 1, 0);
	}
	if (wlrx_ic_is_sleep_enable(WLTRX_IC_MAIN)) {
		hwlog_info("power_mode_control: sleep_en eanble, return\n");
		return;
	}
	wlc_revise_vout_para(di, dts);
	wlc_iout_control(di, dts);
}

int wireless_charge_get_power_mode(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->curr_pmode_index;
}

int wlc_get_expected_pmode_id(int pid)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts) {
		hwlog_err("get_expected_pmode_id: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	return dts->pmode_cfg[pid].expect_mode;
}

void wlc_set_cur_pmode_id(int pmode_id)
{
	struct wlrx_dev_info *di = g_wlrx_di;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts)
		return;

	if ((pmode_id < 0) || (pmode_id >= dts->pmode_cfg_level))
		return;

	di->curr_pmode_index = pmode_id;
}

static void wlc_recheck_cert_preprocess(struct wlrx_dev_info *di)
{
	/* vout may be 9v, so here reset 5V for cert stability */
	if (wldc_set_trx_vout(TX_DEFAULT_VOUT))
		hwlog_err("%s: set default vout failed\n", __func__);

	wlc_set_iout_min(di);
	(void)power_msleep(WLRX_ILIM_DELAY, DT_MSLEEP_25MS, wlrx_msleep_exit);
	wlrx_set_charge_stage(WLRX_STAGE_AUTH);
}

static void wireless_charge_regulation(struct wlrx_dev_info *di)
{
	if ((wlrx_get_charge_stage() != WLRX_STAGE_REGULATION) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	if (g_need_recheck_cert && wlrx_auth_get_srv_state()) {
		g_need_recheck_cert = false;
		wlc_recheck_cert_preprocess(di);
		return;
	}

	wireless_charge_update_max_vout_and_iout(false);
	wireless_charge_power_mode_control(di);
}

static void read_back_color_from_nv(char *back_color, unsigned int back_len)
{
	power_nv_read(POWER_NV_DEVCOLR, back_color, back_len);
}

static void wlc_check_pu_shell(struct wlrx_dev_info *di)
{
	static bool check_done;
	static char dev_color[BACK_DEVICE_COLOR_LEN];

	if (check_done)
		return;

	memset(dev_color, 0, sizeof(BACK_DEVICE_COLOR_LEN));
	read_back_color_from_nv(dev_color, BACK_DEVICE_COLOR_LEN - 1);
	hwlog_info("[%s] pu_color:%s\n", __func__, dev_color);
	check_done = true;

	if (!strncmp(dev_color, "puorange", strlen("puorange")) ||
		!strncmp(dev_color, "pugreen", strlen("pugreen")))
		wlrx_ic_set_pu_shell_flag(WLTRX_IC_MAIN, true);
	else
		wlrx_ic_set_pu_shell_flag(WLTRX_IC_MAIN, false);
}

static void wlc_set_ext_fod_flag(struct wlrx_dev_info *di)
{
	if (wlrx_ic_need_chk_pu_shell(WLTRX_IC_MAIN))
		wlc_check_pu_shell(di);
}

static void wireless_charge_start_charging(struct wlrx_dev_info *di, struct wlrx_dts *dts)
{
	const char *cur_mode_name = NULL;
	int cur_mode_tx_vout;

	if ((wlrx_get_charge_stage() != WLRX_STAGE_CHARGING) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	/*
	 * avoid that charger has ignored RX_READY notifier_call_chain
	 * when charger vbus is not powered, so here redo notifier call
	 */
	wlc_get_supported_max_rx_vout(di, dts);
	wlc_extra_power_supply(true);
	wlc_update_fan_control(di, true);
	wireless_charge_update_fop(di, true);
	wlc_update_kb_control(WLC_START_CHARING);
	wireless_charge_update_max_vout_and_iout(true);
	wireless_charge_icon_display(WLRX_PMODE_NORMAL_JUDGE);
	cur_mode_tx_vout = dts->pmode_cfg[di->curr_pmode_index].vtx;
	cur_mode_tx_vout = min(di->tx_vout_max, cur_mode_tx_vout);
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_CHARGER_VBUS, &cur_mode_tx_vout);

	di->iout_low_cnt = 0;
	wireless_charge_power_mode_control(di);
	(void)wireless_send_charge_event(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		RX_STATR_CHARGING);
	cur_mode_name = dts->pmode_cfg[di->curr_pmode_index].name;
	if (strstr(cur_mode_name, "SC"))
		return;

	wlrx_set_charge_stage(WLRX_STAGE_REGULATION);
}

static bool wlc_is_support_set_rpp_format(struct wlrx_dev_info *di)
{
	int ret;
	u8 tx_rpp = 0;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || dts->pmax <= 0)
		return false;

	if (di->cert_succ_flag != WIRELESS_CHECK_SUCC)
		return false;

	ret = wireless_get_rpp_format(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, &tx_rpp);
	if (!ret && (tx_rpp == QI_ACK_RPP_FORMAT_24BIT))
		return true;

	return false;
}

static int wlc_set_rpp_format(struct wlrx_dev_info *di)
{
	int ret;
	int count = 0;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return -EINVAL;

	do {
		ret = wireless_set_rpp_format(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, dts->pmax);
		if (!ret) {
			hwlog_info("%s: succ\n", __func__);
			return 0;
		}
		(void)power_msleep(WLRX_SET_RPP_FORMAT_RETRY_DELAY,
			DT_MSLEEP_25MS, wlrx_msleep_exit);
		count++;
		hwlog_err("%s: failed, try next time\n", __func__);
	} while (count < WLC_SET_RPP_FORMAT_RETRY_CNT);

	if (count < WLC_SET_RPP_FORMAT_RETRY_CNT) {
		hwlog_info("[%s] succ\n", __func__);
		return 0;
	}

	return -EIO;
}

static void wlc_rpp_format_init(struct wlrx_dev_info *di)
{
	int ret;

	if (!di->standard_tx || wlrx_is_fac_tx(di->tx_cap->type))
		return;

	if (!wlc_is_support_set_rpp_format(di)) {
		wlrx_set_plim_src(WLRX_PLIM_SRC_RPP);
		return;
	}

	ret = wlc_set_rpp_format(di);
	if (!ret) {
		hwlog_info("[%s] succ\n", __func__);
		wlrx_clear_plim_src(WLRX_PLIM_SRC_RPP);
		return;
	}
	wlrx_set_plim_src(WLRX_PLIM_SRC_RPP);
}

static void wireless_charge_check_fwupdate(struct wlrx_dev_info *di)
{
	int ret;

	if ((wlrx_get_charge_stage() != WLRX_STAGE_FW_UPDATE) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	ret = wlrx_ic_fw_update(WLTRX_IC_MAIN);
	if (!ret)
		wireless_charge_chip_init(WIRELESS_CHIP_INIT);

	charge_pump_chip_init(CP_TYPE_MAIN);
	charge_pump_chip_init(CP_TYPE_AUX);
	wlc_rpp_format_init(di);
	wlrx_set_charge_stage(WLRX_STAGE_CHARGING);
}

static bool wlc_need_check_certification(struct wlrx_dev_info *di)
{
	int pmode;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (di->tx_cap->no_need_cert || !dts)
		return false;

	for (pmode = dts->pmode_cfg_level - 1; pmode >= 0; pmode--) {
		if (wireless_charge_mode_judge_criterion(pmode, WLRX_PMODE_QUICK_JUDGE))
			break;
	}

	if (pmode < 0)
		pmode = 0;

	if (dts->pmode_cfg[pmode].auth > 0)
		return true;

	hwlog_info("%s: max pmode=%d\n", __func__, pmode);
	return false;
}

static void wireless_charge_check_certification(struct wlrx_dev_info *di)
{
	int ret;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	if ((wlrx_get_charge_stage() != WLRX_STAGE_AUTH) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	if (di->certi_err_cnt >= CERTI_ERR_CNT_MAX) {
		di->cert_succ_flag = WIRELESS_CHECK_FAIL;
		wlc_send_cert_confirm_msg(di, false);
		if (di->certi_comm_err_cnt > 0) {
			wlc_rx_chip_reset(di);
		} else {
			wireless_charge_icon_display(WLRX_PMODE_NORMAL_JUDGE);
			wireless_charge_dsm_report(di,
				ERROR_WIRELESS_CERTI_COMM_FAIL, dsm_buff);
		}
		hwlog_err("%s: error exceed %d times\n",
			__func__, CERTI_ERR_CNT_MAX);
	} else if (wlc_need_check_certification(di)) {
		if (!wlrx_auth_get_srv_state()) {
			g_need_recheck_cert = true;
			wlrx_set_charge_stage(WLRX_STAGE_FW_UPDATE);
			return;
		}
		g_need_recheck_cert = false;
		wlc_set_iout_min(di);
		ret = wlc_tx_certification(di);
		if (ret) {
			hwlog_err("%s: fail\n", __func__);
			di->certi_err_cnt++;
			return;
		}
		hwlog_info("[%s] succ\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_CERT_SUCC, NULL);
		wlc_send_cert_confirm_msg(di, true);
		di->cert_succ_flag = WIRELESS_CHECK_SUCC;
	} else {
		di->cert_succ_flag = WIRELESS_CHECK_UNKNOWN;
	}

	wlrx_set_charge_stage(WLRX_STAGE_FW_UPDATE);
}

static void wireless_charge_cable_detect(struct wlrx_dev_info *di)
{
	if ((wlrx_get_charge_stage() != WLRX_STAGE_CABLE_DET) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	di->cable_detect_succ_flag = WIRELESS_CHECK_SUCC;
	wlrx_set_charge_stage(WLRX_STAGE_AUTH);
}

static void wlc_get_ept_type(struct wlrx_dev_info *di)
{
	int ret;
	u16 ept_type = 0;

	if (!di->tx_cap->support_get_ept)
		return;

	ret = wireless_get_ept_type(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, &ept_type);
	if (ret) {
		hwlog_err("%s: failed\n", __func__);
		return;
	}
	hwlog_info("[%s] type=0x%x\n", __func__, ept_type);
}

static void wireless_charge_send_acc(struct wlrx_dev_info *di, bool acc_on)
{
	struct wlrx_acc_det_para acc;
	int acc_det_status = WLRX_ACC_DET_DEFAULT;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	if (dts->product_pmax_hth < dts->accdet_pmax_lth)
		return;

	if (wlrx_is_rvs_tx(di->tx_cap->type) || wlrx_is_fac_tx(di->tx_cap->type))
		return;

	if (acc_on) {
		acc.acc_det_pwr = dts->accdet_pmax_lth;
		acc.tx_pwr = di->tx_cap->vout_max * di->tx_cap->iout_max / MVOLT_PER_VOLT;
		acc.standard_tx = di->standard_tx;
		wlrx_acc_det(&acc);
	} else {
		power_ui_event_notify(POWER_UI_NE_WL_ACC_STATUS, &acc_det_status);
	}
}

static void wireless_charge_check_tx_ability(struct wlrx_dev_info *di)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = {0};

	if ((wlrx_get_charge_stage() != WLRX_STAGE_GET_TX_CAP) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	hwlog_info("%s ++\n", __func__);
	if (di->tx_ability_err_cnt >= TX_ABILITY_ERR_CNT_MAX) {
		wlrx_get_tx_prop(di);
		wireless_charge_get_tx_info(di);
		wireless_charge_send_fod_status(di);
		hwlog_err("%s: error exceed %d times\n",
			__func__, TX_ABILITY_ERR_CNT_MAX);
		if (di->standard_tx &&
			(di->wlc_err_rst_cnt >= WLC_RST_CNT_MAX))
			wireless_charge_dsm_report(di,
				ERROR_WIRELESS_CHECK_TX_ABILITY_FAIL,
				dsm_buff);
		wlc_rx_chip_reset(di);
		wlrx_set_charge_stage(WLRX_STAGE_CABLE_DET);
		return;
	}
	wlc_set_iout_min(di);
	wireless_charge_get_tx_capability(di);
	if (wlrx_is_err_tx(di->tx_cap->type)) {
		hwlog_err("%s: get tx ability failed\n", __func__);
		di->tx_ability_err_cnt++;
		return;
	}
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_TX_CAP_SUCC, NULL);
	wireless_charge_get_tx_fop_range(di);
	wlrx_get_tx_prop(di);

	if (di->tx_cap->no_need_cert)
		wireless_charge_icon_display(WLRX_PMODE_NORMAL_JUDGE);
	else
		wireless_charge_icon_display(WLRX_PMODE_QUICK_JUDGE);

	wlc_get_ept_type(di);
	wlrx_set_charge_stage(WLRX_STAGE_CABLE_DET);
	wireless_charge_get_tx_info(di);
	wireless_charge_send_acc(di, true);
	wireless_charge_send_fod_status(di);
	hwlog_info("%s --\n", __func__);
}

static void wireless_charge_check_tx_id(struct wlrx_dev_info *di)
{
	int tx_id;

	if ((wlrx_get_charge_stage() != WLRX_STAGE_HANDSHAKE) ||
		(wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF))
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (di->tx_id_err_cnt >= TX_ID_ERR_CNT_MAX) {
		wlrx_get_tx_prop(di);
		hwlog_err("%s: error exceed %d times, fast charge is disabled\n", __func__, TX_ID_ERR_CNT_MAX);
		wlrx_set_charge_stage(WLRX_STAGE_CABLE_DET);
		return;
	}
	wlc_set_iout_min(di);
	tx_id = wireless_charge_get_tx_id();
	if (tx_id < 0) {
		hwlog_err("%s: get id failed\n", __func__);
		di->tx_id_err_cnt++;
		return;
	}
	if (tx_id != TX_ID_HW) {
		wlrx_get_tx_prop(di);
		hwlog_err("%s: id(0x%x) is not correct(0x%x)\n", __func__, tx_id, TX_ID_HW);
		wlrx_set_charge_stage(WLRX_STAGE_CABLE_DET);
		return;
	}
	di->standard_tx = 1;
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_HS_SUCC, NULL);
	wlrx_set_charge_stage(WLRX_STAGE_GET_TX_CAP);
	hwlog_info("[%s] --\n", __func__);
	return;
}

static void wireless_charge_rx_stop_charing_config(struct wlrx_dev_info *di)
{
	wlrx_ic_stop_charging(WLTRX_IC_MAIN);
	wireless_reset_dev_info(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI);
}

static void wlc_state_record_para_init(struct wlrx_dev_info *di)
{
	di->stat_rcd.chrg_state_cur = 0;
	di->stat_rcd.chrg_state_last = 0;
	di->stat_rcd.fan_cur = WLC_FAN_UNKNOWN_SPEED;
	di->stat_rcd.fan_last = WLC_FAN_UNKNOWN_SPEED;
}

static void wireless_charge_para_init(struct wlrx_dev_info *di)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts)
		return;

	di->monitor_interval = MONITOR_INTERVAL;
	di->ctrl_interval = CONTROL_INTERVAL_NORMAL;
	di->tx_vout_max = TX_DEFAULT_VOUT;
	di->rx_vout_max = RX_DEFAULT_VOUT;
	di->rx_iout_max = dts->rx_imin;
	di->rx_iout_limit = dts->rx_imin;
	di->standard_tx = 0;
	di->tx_id_err_cnt = 0;
	di->tx_ability_err_cnt = 0;
	di->certi_err_cnt = 0;
	di->certi_comm_err_cnt = 0;
	di->boost_err_cnt = 0;
	di->sysfs_data.en_enable = 0;
	di->iout_high_cnt = 0;
	di->iout_low_cnt = 0;
	di->cable_detect_succ_flag = 0;
	di->cert_succ_flag = 0;
	di->curr_tx_type_index = 0;
	di->curr_pmode_index = 0;
	wlc_set_cur_vmode_id(di, 0);
	di->curr_power_time_out = 0;
	di->pwroff_reset_flag = 0;
	di->supported_rx_vout = RX_DEFAULT_VOUT;
	di->extra_pwr_good_flag = 1;
	di->tx_type = WIRELESS_TX_TYPE_UNKNOWN;
	di->qval_support_mode = WLC_RX_SP_ALL_MODE;
	g_rx_vrect_restore_cnt = 0;
	g_rx_vout_err_cnt = 0;
	g_need_recheck_cert = false;
	g_wlc_start_sample_flag = false;
	g_rx_ocp_cnt = 0;
	g_rx_ovp_cnt = 0;
	g_rx_otp_cnt = 0;
	g_tx_fw_version = NULL;
	wlrx_reset_fsk_alarm();
	wlrx_reset_plim_para();
	wlc_state_record_para_init(di);
	wireless_charge_set_default_tx_capability(di);
	wireless_charge_reset_avg_iout(di);
	wlc_reset_icon_pmode(di, dts);
	charge_set_input_current_prop(dts->rx_istep, CHARGE_CURRENT_DELAY);
}

static void wireless_charge_control_work(struct work_struct *work)
{
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_dev_info *di = container_of(work,
		struct wlrx_dev_info, wireless_ctrl_work.work);

	if (!di || !dts) {
		hwlog_err("control_work: para null\n");
		return;
	}

	wireless_charge_check_tx_id(di);
	wireless_charge_check_tx_ability(di);
	wireless_charge_cable_detect(di);
	wireless_charge_check_certification(di);
	wireless_charge_check_fwupdate(di);
	wireless_charge_start_charging(di, dts);
	wireless_charge_regulation(di);
	wireless_charge_set_ctrl_interval(di);

	if ((wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) &&
		(wlrx_get_charge_stage() != WLRX_STAGE_REGULATION_DC))
		schedule_delayed_work(&di->wireless_ctrl_work,
			msecs_to_jiffies(di->ctrl_interval));
}

void wireless_charge_restart_charging(unsigned int stage_from)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if ((wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) &&
		(wlrx_get_charge_stage() > WLRX_STAGE_CHARGING)) {
		wlrx_set_charge_stage(stage_from);
		schedule_delayed_work(&di->wireless_ctrl_work,
			msecs_to_jiffies(100)); /* 100ms for pmode stability */
	}
}

static void wireless_charge_switch_off(void)
{
	wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW2, false);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW_AUX, false);
	charge_pump_chip_enable(CP_TYPE_AUX, false);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
	charge_pump_chip_enable(CP_TYPE_MAIN, false);
}

static void wireless_charge_stop_charging(struct wlrx_dev_info *di)
{
	hwlog_info("%s ++\n", __func__);
	wlrx_ic_sleep_enable(WLTRX_IC_MAIN, true);
	wlc_extra_power_supply(false);
	pd_dpm_ignore_vbus_only_event(false);
	wlrx_set_charge_stage(WLRX_STAGE_DEFAULT);
	charge_set_input_current_prop(0, 0);
	wireless_charge_rx_stop_charing_config(di);
	wlc_update_kb_control(WLC_STOP_CHARING);
	wireless_fast_charge_flag = 0;
	wireless_super_charge_flag = 0;
	g_fop_fixed_flag = 0;
	g_need_force_5v_vout = false;
	g_plimit_time_num = 0;
	cancel_delayed_work_sync(&di->rx_sample_work);
	cancel_delayed_work_sync(&di->wireless_ctrl_work);
	cancel_delayed_work_sync(&di->ignore_qval_work);
	di->curr_pmode_index = 0;
	di->curr_icon_type = 0;
	di->wlc_err_rst_cnt = 0;
	di->supported_rx_vout = RX_DEFAULT_VOUT;
	wireless_charge_set_default_tx_capability(di);
	hwlog_info("%s --\n", __func__);
}

static void wlc_wireless_vbus_connect_handler(unsigned int stage_from)
{
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di || !dts) {
		hwlog_err("wireless_vbus_connect_handler: para null\n");
		return;
	}

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired vbus connect\n", __func__);
		return;
	}

	wlrx_set_wireless_channel_state(WIRELESS_CHANNEL_ON);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
	charge_pump_chip_enable(CP_TYPE_MAIN, true);
	wlrx_ic_sleep_enable(WLTRX_IC_MAIN, false);
	wireless_charge_chip_init(WIRELESS_CHIP_INIT);
	di->curr_pmode_index = 0;
	wlc_set_cur_vmode_id(di, 0);
	di->tx_vout_max = dts->pmode_cfg[0].vtx;
	di->rx_vout_max = dts->pmode_cfg[0].vrx;
	if (wireless_charge_set_rx_vout(di->rx_vout_max))
		hwlog_err("%s: set rx vout failed\n", __func__);

	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		wlrx_set_charge_stage(stage_from);
		wlc_set_iout_min(di);
		mod_delayed_work(system_wq, &di->wireless_ctrl_work,
			msecs_to_jiffies(di->ctrl_interval));
		power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_CHARGER_VBUS, &di->tx_vout_max);
		hwlog_info("%s --\n", __func__);
	}
}

static void wireless_charge_wireless_vbus_disconnect_handler(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (wlrx_ic_is_tx_exist(WLTRX_IC_MAIN)) {
		hwlog_info("[%s] tx exist, ignore\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_VBUS_CONNECT, NULL);
		mod_delayed_work(system_wq,
			&di->wireless_monitor_work, msecs_to_jiffies(0));
		mod_delayed_work(system_wq,
			&di->wireless_watchdog_work, msecs_to_jiffies(0));
		wldc_set_charge_stage(WLDC_STAGE_DEFAULT);
		wlc_wireless_vbus_connect_handler(WLRX_STAGE_REGULATION);
		return;
	}
	wlrx_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	wireless_charge_switch_off();
	charger_source_sink_event(STOP_SINK_WIRELESS);
	wireless_charge_stop_charging(di);
}

static void wireless_charge_wireless_vbus_disconnect_work(
	struct work_struct *work)
{
	wireless_charge_wireless_vbus_disconnect_handler();
}

static void wireless_charge_wired_vbus_connect_work(struct work_struct *work)
{
	int i, ret;
	int vout = 0;
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	mutex_lock(&g_rx_en_mutex);
	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &vout);
	if (vout >= RX_HIGH_VOUT) {
		wireless_charge_rx_stop_charing_config(di);
		ret = wireless_charge_set_tx_vout(TX_DEFAULT_VOUT);
		ret |= wireless_charge_set_rx_vout(TX_DEFAULT_VOUT);
		if (ret)
			hwlog_err("%s: set trx vout fail\n", __func__);
		if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF) {
			hwlog_err("%s: wired vubs already off, reset rx\n", __func__);
			wlc_rx_chip_reset(di);
		}
		if (!wireless_is_in_tx_mode())
			wlrx_ic_chip_enable(WLTRX_IC_MAIN, false);
		wlrx_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	} else {
		if (!wireless_is_in_tx_mode())
			wlrx_ic_chip_enable(WLTRX_IC_MAIN, false);
		wlrx_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	}
	mutex_unlock(&g_rx_en_mutex);
	/* wait for 50ms*10=500ms at most, generally 300ms at most */
	for (i = 0; i < 10; i++) {
		if (wldc_is_stop_charging_complete()) {
			wired_chsw_set_wired_channel(WIRED_CHANNEL_MAIN, WIRED_CHANNEL_RESTORE);
			break;
		}
		msleep(50);
	}
	hwlog_info("wired vbus connect, turn off wireless channel\n");
	wireless_charge_stop_charging(di);
}

static void wireless_charge_wired_vbus_disconnect_work(struct work_struct *work)
{
	mutex_lock(&g_rx_en_mutex);
	wlrx_ic_chip_enable(WLTRX_IC_MAIN, true);
	mutex_unlock(&g_rx_en_mutex);
	hwlog_info("wired vbus disconnect, turn on wireless channel\n");
}

void wireless_charge_wired_vbus_connect_handler(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		wired_chsw_set_wired_channel(WIRED_CHANNEL_MAIN, WIRED_CHANNEL_RESTORE);
		wireless_charge_switch_off();
		return;
	}
	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("%s: already in sink_vbus state, ignore\n", __func__);
		return;
	}
	hwlog_info("[%s] wired vbus connect\n", __func__);
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_WIRED_VBUS_CONNECT, NULL);
	wireless_super_charge_flag = 0;
	wlrx_set_wired_channel_state(WIRED_CHANNEL_ON);
	wldc_tx_disconnect_handler();
	wireless_charge_switch_off();
	if (!wireless_fast_charge_flag)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_MAIN, WIRED_CHANNEL_RESTORE);
	schedule_work(&di->wired_vbus_connect_work);
}

void wireless_charge_wired_vbus_disconnect_handler(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;
	static bool first_in = true;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (!first_in && (wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF)) {
		hwlog_err("%s: not in sink_vbus state, ignore\n", __func__);
		return;
	}
	first_in = false;
	hwlog_info("[%s] wired vbus disconnect\n", __func__);
	wlrx_set_wired_channel_state(WIRED_CHANNEL_OFF);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_WIRED_VBUS_DISCONNECT, NULL);
	schedule_work(&di->wired_vbus_disconnect_work);
}

#ifdef CONFIG_DIRECT_CHARGER
void direct_charger_disconnect_event(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	hwlog_info("wired vbus disconnect in scp charging mode\n");
	wlrx_set_wired_channel_state(WIRED_CHANNEL_OFF);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	schedule_work(&di->wired_vbus_disconnect_work);
}
#endif

void wireless_charger_pmic_vbus_handler(bool vbus_state)
{
	int vfc_reg = 0;
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di)
		return;

	if (wireless_tx_get_tx_open_flag())
		return;
	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	if (!vbus_state && (vfc_reg > TX_REG_VOUT) &&
		(di->cert_succ_flag == WIRELESS_CHECK_SUCC))
		wlrx_ic_sleep_enable(WLTRX_IC_MAIN, true);

	wlrx_ic_pmic_vbus_handler(WLTRX_IC_MAIN, vbus_state);
}

static int wireless_charge_check_tx_disconnect(struct wlrx_dev_info *di)
{
	if (wlrx_ic_is_tx_exist(WLTRX_IC_MAIN))
		return 0;

	g_fop_fixed_flag = 0;
	g_need_force_5v_vout = false;
	g_plimit_time_num = 0;
	wireless_charge_send_acc(di, false);
	wldc_tx_disconnect_handler();
	wlrx_ic_sleep_enable(WLTRX_IC_MAIN, true);
	wlrx_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_VBUS_DISCONNECT, NULL);
	wireless_charge_rx_stop_charing_config(di);
	cancel_delayed_work_sync(&di->wireless_ctrl_work);
	cancel_delayed_work_sync(&di->wireless_vbus_disconnect_work);
	schedule_delayed_work(&di->wireless_vbus_disconnect_work,
		msecs_to_jiffies(di->discon_delay_time));
	hwlog_err("%s: tx not exist, delay %dms to report disconnect event\n",
		__func__, di->discon_delay_time);

	return -1;
}

void wlc_reset_wireless_charge(void)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di)
		return;

	if (delayed_work_pending(&di->wireless_vbus_disconnect_work))
		mod_delayed_work(system_wq, &di->wireless_vbus_disconnect_work,
			msecs_to_jiffies(0));
}

static int wlc_monitor_imax_by_time(struct wlrx_dev_info *di)
{
	int i;
	int vfc_reg = 0;
	static unsigned int start_time;
	unsigned int delta_time;

	if ((di->flag.mon_imax == WLC_FIRST_MON_IMAX) ||
		(start_time < WLC_START_MON_TIME_TH))
		start_time = current_kernel_time().tv_sec;

	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	if (vfc_reg < RX_HIGH_VOUT2)
		return WLRX_IC_DFT_IOUT_MAX;

	delta_time = current_kernel_time().tv_sec - start_time;
	for (i = 0; i < WLC_TIME_PARA_LEVEL; i++) {
		if (delta_time >= di->time_para[i].time_th)
			break;
	}

	if ((i >= WLC_TIME_PARA_LEVEL) ||
		(di->time_para[i].iout_max <= 0))
		return WLRX_IC_DFT_IOUT_MAX;

	return di->time_para[i].iout_max;
}

static int wlc_monitor_imax_by_temp(struct wlrx_dev_info *di)
{
	int i;
	int rx_temp = 0;
	static int last_i;
	int cur_i = last_i;

	(void)wlrx_ic_get_temp(WLTRX_IC_MAIN, &rx_temp);
	for (i = 0; i < WLC_TEMP_PARA_LEVEL; i++) {
		if ((rx_temp > di->temp_para[i].temp_lth) &&
			(rx_temp <= di->temp_para[i].temp_hth)) {
			if ((di->flag.mon_imax == WLC_FIRST_MON_IMAX) ||
				(last_i < i) ||
				(di->temp_para[i].temp_hth - rx_temp >
				di->temp_para[i].temp_back))
				cur_i = i;
			else
				cur_i = last_i;
			break;
		}
	}

	if ((i >= WLC_TEMP_PARA_LEVEL) || (di->temp_para[cur_i].iout_max <= 0))
		return WLRX_IC_DFT_IOUT_MAX;

	last_i = cur_i;

	return di->temp_para[cur_i].iout_max;
}

static void wlc_monitor_rx_imax(struct wlrx_dev_info *di)
{
	int imax;
	int imax_by_time;
	int imax_by_chip_fault = 0;

	imax = wlc_monitor_imax_by_temp(di);
	imax_by_time = wlc_monitor_imax_by_time(di);
	(void)wlrx_ic_get_imax(WLTRX_IC_MAIN, &imax_by_chip_fault);
	if (imax > imax_by_time)
		imax = imax_by_time;
	if (imax > imax_by_chip_fault)
		imax = imax_by_chip_fault;

	g_rx_imax = imax;
	if (di->flag.mon_imax == WLC_FIRST_MON_IMAX)
		di->flag.mon_imax = WLC_NON_FIRST_MON_IMAX;
}

static bool wlc_need_show_mon_info(struct wlrx_dev_info *di)
{
	int iin_regval;
	static int iin_regval_last;
	static int cnt;

	if ((wlrx_get_charge_stage() < WLRX_STAGE_CHARGING) || !g_bst_rst_complete)
		return true;
	if (++cnt == MONITOR_LOG_INTERVAL / di->monitor_interval) {
		cnt = 0;
		return true;
	}
	if (wlrx_is_fac_tx(di->tx_cap->type))
		return true;

	iin_regval = charge_get_charger_iinlim_regval();
	if (iin_regval_last != iin_regval) {
		iin_regval_last = iin_regval;
		return true;
	}

	return false;
}

static void wlc_show_monitor_info(struct wlrx_dev_info *di)
{
	int tbatt = 0;
	int vrect = 0;
	int vout = 0;
	int iout = 0;
	int fop = 0;
	int temp = 0;
	int cep = 0;

	if (!wlc_need_show_mon_info(di))
		return;

	(void)wlrx_ic_get_vrect(WLTRX_IC_MAIN, &vrect);
	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &vout);
	(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &iout);
	(void)wlrx_ic_get_fop(WLTRX_IC_MAIN, &fop);
	(void)wlrx_ic_get_temp(WLTRX_IC_MAIN, &temp);
	(void)wlrx_ic_get_cep(WLTRX_IC_MAIN, &cep);
	(void)bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	hwlog_info("monitor_info [sys]soc:%-3d tbatt:%d pmode:%d plim_src:0x%02x\t"
		"[dc] warn:%d err:%d\t"
		"[tx] plim:%d\t"
		"[rx] temp:%-3d fop:%-3d cep:%-3d vrect:%-5d vout=%-5d\t"
		"[rx] imax:%-4d iout:%-4d iout_avg:%-4d iin_reg:%-4d\t"
		"[sysfs] fop:%-3d irx:%-4d vrx:%-5d vtx:%-5d\n",
		coul_drv_battery_capacity(), tbatt,
		di->curr_pmode_index, wlrx_get_plim_src(),
		wldc_get_warning_cnt(), wldc_get_error_cnt(),
		wlrx_get_alarm_plim(), temp, fop, cep, vrect, vout,
		wlc_get_rx_max_iout(), iout, di->iout_avg,
		charge_get_charger_iinlim_regval(),
		wlrx_get_intfr_fixed_fop(), wlrx_get_intfr_irx(),
		wlrx_get_intfr_vrx(), wlrx_get_intfr_vtx());
}

static void wireless_charge_monitor_work(struct work_struct *work)
{
	int ret;
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	ret = wireless_charge_check_tx_disconnect(di);
	if (ret) {
		hwlog_info("[%s] tx disconnect, stop monitor work\n", __func__);
		return;
	}
	wireless_charge_calc_avg_iout(di);
	wireless_charge_count_avg_iout(di);
	wlc_check_voltage(di);
	wireless_charge_update_status(di);
	wlc_monitor_rx_imax(di);
	wlc_show_monitor_info(di);

	schedule_delayed_work(&di->wireless_monitor_work,
		msecs_to_jiffies(di->monitor_interval));
}

static void wireless_charge_watchdog_work(struct work_struct *work)
{
	if (!g_wlrx_di)
		return;

	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF)
		return;

	if (wlrx_ic_kick_watchdog(WLTRX_IC_MAIN))
		hwlog_err("%s: fail\n", __func__);

	/* kick watchdog at an interval of 100ms */
	schedule_delayed_work(&g_wlrx_di->wireless_watchdog_work,
		msecs_to_jiffies(100));
}

static void wireless_charge_rx_sample_work(struct work_struct *work)
{
	int rx_vout = 0;
	int rx_iout = 0;
	struct wlrx_dev_info *di = container_of(work,
		struct wlrx_dev_info, rx_sample_work.work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (!g_wlc_start_sample_flag)
		g_wlc_start_sample_flag = true;

	/* send confirm message to TX */
	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &rx_vout);
	(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &rx_iout);
	wireless_send_rx_vout(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, rx_vout);
	wireless_send_rx_iout(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, rx_iout);

	hwlog_info("[%s] rx_vout = %d, rx_iout = %d\n", __func__, rx_vout, rx_iout);

	schedule_delayed_work(&di->rx_sample_work, msecs_to_jiffies(RX_SAMPLE_WORK_DELAY));
}

static void wireless_charge_pwroff_reset_work(struct work_struct *work)
{
	struct wlrx_dev_info *di = container_of(work,
		struct wlrx_dev_info, wireless_pwroff_reset_work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		wireless_charge_wake_unlock();
		return;
	}
	if (di->pwroff_reset_flag) {
		msleep(60); /* test result, about 60ms */
		wlrx_ic_chip_reset(WLTRX_IC_MAIN);
		wireless_charge_set_tx_vout(TX_DEFAULT_VOUT);
		wireless_charge_set_rx_vout(RX_DEFAULT_VOUT);
	}
	wireless_charge_wake_unlock();
}

static void wlc_rx_power_on_ready_handler(struct wlrx_dev_info *di)
{
	static bool rx_pwr_good;

	wldc_set_charge_stage(WLDC_STAGE_DEFAULT);
	wlrx_set_charge_stage(WLRX_STAGE_DEFAULT);
	wireless_charge_para_init(di);

	if (di->rx_evt.pwr_good == RX_PWR_ON_GOOD) {
		rx_pwr_good = true;
		di->rx_evt.pwr_good = RX_PWR_ON_NOT_GOOD;
	}

	if (((di->rx_evt.type == POWER_NE_WLRX_PWR_ON) && rx_pwr_good) ||
		(di->rx_evt.type == POWER_NE_WLRX_READY)) {
		wltx_reset_reverse_charging();
		wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, false);
		power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_VBUS_CONNECT, NULL);
		charger_source_sink_event(START_SINK_WIRELESS);
	}
	pd_dpm_ignore_vbus_only_event(true);
	mod_delayed_work(system_wq, &di->wireless_monitor_work,
		msecs_to_jiffies(0));
	mod_delayed_work(system_wq, &di->wireless_watchdog_work,
		msecs_to_jiffies(0));
	if (delayed_work_pending(&di->wireless_vbus_disconnect_work))
		cancel_delayed_work_sync(&di->wireless_vbus_disconnect_work);
	if (di->rx_evt.type == POWER_NE_WLRX_READY) {
		if (di->flag.mon_imax != WLC_NO_NEED_MON_IMAX)
			di->flag.mon_imax = WLC_FIRST_MON_IMAX;
		if (!di->wlc_err_rst_cnt)
			wireless_fast_charge_flag = 0;
		wlc_set_ext_fod_flag(di);
		di->discon_delay_time = WL_DISCONN_DELAY_MS;
		power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_READY, NULL);
		wlc_wireless_vbus_connect_handler(WLRX_STAGE_HANDSHAKE);
	}
}

static void wlc_handle_tx_bst_err_evt(void)
{
	hwlog_info("handle_tx_bst_err_evt\n");
	wlrx_set_plim_src(WLRX_PLIM_SRC_TX_BST_ERR);
	wireless_charge_update_max_vout_and_iout(true);
}

static void wlc_handle_rx_ocp_evt(struct wlrx_dev_info *di)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	hwlog_info("handle_rx_ocp_evt\n");
	if (wlrx_get_charge_stage() < WLRX_STAGE_REGULATION)
		return;

	if (++g_rx_ocp_cnt < RX_OCP_CNT_MAX)
		return;

	g_rx_ocp_cnt = RX_OCP_CNT_MAX;
	wireless_charge_dsm_report(di, ERROR_WIRELESS_RX_OCP, dsm_buff);
}

static void wlc_handle_rx_ovp_evt(struct wlrx_dev_info *di)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	hwlog_info("handle_rx_ovp_evt\n");
	if (wlrx_get_charge_stage() < WLRX_STAGE_REGULATION)
		return;

	if (++g_rx_ovp_cnt < RX_OVP_CNT_MAX)
		return;

	g_rx_ovp_cnt = RX_OVP_CNT_MAX;
	wireless_charge_dsm_report(di, ERROR_WIRELESS_RX_OVP, dsm_buff);
}

static void wlc_handle_rx_otp_evt(struct wlrx_dev_info *di)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	hwlog_info("handle_rx_otp_evt\n");
	if (wlrx_get_charge_stage() < WLRX_STAGE_REGULATION)
		return;

	if (++g_rx_otp_cnt < RX_OTP_CNT_MAX)
		return;

	g_rx_otp_cnt = RX_OTP_CNT_MAX;
	wireless_charge_dsm_report(di, ERROR_WIRELESS_RX_OTP, dsm_buff);
}

static void wlc_handle_rx_ldo_off_evt(struct wlrx_dev_info *di)
{
	hwlog_info("handle_rx_ldo_off_evt\n");
	charger_source_sink_event(STOP_SINK_WIRELESS);
	cancel_delayed_work_sync(&di->wireless_ctrl_work);
	cancel_delayed_work_sync(&di->wireless_monitor_work);
	cancel_delayed_work_sync(&di->wireless_watchdog_work);
}

static void wlc_rx_event_work(struct work_struct *work)
{
	struct wlrx_dev_info *di = container_of(work,
		struct wlrx_dev_info, rx_event_work.work);

	if (!di) {
		wireless_charge_wake_unlock();
		return;
	}

	switch (di->rx_evt.type) {
	case POWER_NE_WLRX_PWR_ON:
		hwlog_info("[%s] RX power on\n", __func__);
		wlc_rx_power_on_ready_handler(di);
		break;
	case POWER_NE_WLRX_READY:
		hwlog_info("[%s] RX ready\n", __func__);
		wlc_rx_power_on_ready_handler(di);
		break;
	case POWER_NE_WLRX_OCP:
		wlc_handle_rx_ocp_evt(di);
		break;
	case POWER_NE_WLRX_OVP:
		wlc_handle_rx_ovp_evt(di);
		break;
	case POWER_NE_WLRX_OTP:
		wlc_handle_rx_otp_evt(di);
		break;
	case POWER_NE_WLRX_LDO_OFF:
		wlc_handle_rx_ldo_off_evt(di);
		break;
	case POWER_NE_WLRX_TX_ALARM:
		wlrx_handle_fsk_alarm(&di->rx_evt.tx_alarm);
		break;
	case POWER_NE_WLRX_TX_BST_ERR:
		wlc_handle_tx_bst_err_evt();
		break;
	default:
		break;
	}
	wireless_charge_wake_unlock();
}

static void wlc_save_rx_evt_data(struct wlrx_dev_info *di,
	unsigned long event, void *data)
{
	if (!data)
		return;

	switch (di->rx_evt.type) {
	case POWER_NE_WLRX_PWR_ON:
		di->rx_evt.pwr_good = *(int *)data;
		break;
	case POWER_NE_WLRX_TX_ALARM:
		memcpy(&di->rx_evt.tx_alarm, data,
			sizeof(struct wireless_protocol_tx_alarm));
		break;
	default:
		break;
	}
}

static int wireless_charge_rx_event_notifier_call(struct notifier_block *rx_event_nb,
	unsigned long event, void *data)
{
	struct wlrx_dev_info *di = container_of(rx_event_nb,
		struct wlrx_dev_info, rx_event_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WLRX_PWR_ON:
	case POWER_NE_WLRX_READY:
	case POWER_NE_WLRX_OCP:
	case POWER_NE_WLRX_OVP:
	case POWER_NE_WLRX_OTP:
	case POWER_NE_WLRX_LDO_OFF:
	case POWER_NE_WLRX_TX_ALARM:
	case POWER_NE_WLRX_TX_BST_ERR:
		break;
	default:
		return NOTIFY_OK;
	}

	wireless_charge_wake_lock();
	di->rx_evt.type = (int)event;
	wlc_save_rx_evt_data(di, event, data);

	cancel_delayed_work_sync(&di->rx_event_work);
	mod_delayed_work(system_wq, &di->rx_event_work,
		msecs_to_jiffies(0));

	return NOTIFY_OK;
}

static int wireless_charge_pwrkey_event_notifier_call(struct notifier_block *pwrkey_event_nb,
	unsigned long event, void *data)
{
	struct wlrx_dev_info *di = g_wlrx_di;

	if (!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return NOTIFY_OK;
	}

	switch (event) {
	case PRESS_KEY_6S:
		wireless_charge_wake_lock();
		hwlog_err("%s: response long press 6s interrupt, reset tx vout\n", __func__);
		di->pwroff_reset_flag = 1;
		schedule_work(&di->wireless_pwroff_reset_work);
		break;
	case PRESS_KEY_UP:
		di->pwroff_reset_flag = 0;
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static int wireless_charge_chrg_event_notifier_call(struct notifier_block *chrg_event_nb,
	unsigned long event, void *data)
{
	struct wlrx_dev_info *di = container_of(chrg_event_nb,
		struct wlrx_dev_info, chrg_event_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_CHG_CHARGING_DONE:
		hwlog_debug("[%s] charge done\n", __func__);
		di->stat_rcd.chrg_state_cur |= WIRELESS_STATE_CHRG_DONE;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

/*
 * There are a numerous options that are configurable on the wireless receiver
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#ifdef CONFIG_SYSFS
static ssize_t wireless_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t wireless_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wireless_charge_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(wireless_charge, 0444, WIRELESS_CHARGE_SYSFS_CHIP_INFO, chip_info),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_KEY_ID, kid),
	power_sysfs_attr_ro(wireless_charge, 0444,
		WIRELESS_CHARGE_SYSFS_TX_ADAPTOR_TYPE, tx_adaptor_type),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_RX_TEMP, rx_temp),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_VOUT, vout),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_IOUT, iout),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_VRECT, vrect),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_EN_ENABLE, en_enable),
	power_sysfs_attr_ro(wireless_charge, 0444,
		WIRELESS_CHARGE_SYSFS_NORMAL_CHRG_SUCC, normal_chrg_succ),
	power_sysfs_attr_ro(wireless_charge, 0444,
		WIRELESS_CHARGE_SYSFS_FAST_CHRG_SUCC, fast_chrg_succ),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_FOD_COEF, fod_coef),
	power_sysfs_attr_rw(wireless_charge, 0644,
		WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING, interference_setting),
	power_sysfs_attr_rw(wireless_charge, 0644,
		WIRELESS_CHARGE_SYSFS_RX_SUPPORT_MODE, rx_support_mode),
	power_sysfs_attr_rw(wireless_charge, 0644,
		WIRELESS_CHARGE_SYSFS_THERMAL_CTRL, thermal_ctrl),
	power_sysfs_attr_rw(wireless_charge, 0644, WIRELESS_CHARGE_SYSFS_NVM_DATA, nvm_data),
	power_sysfs_attr_rw(wireless_charge, 0644,
		WIRELESS_CHARGE_SYSFS_IGNORE_FAN_CTRL, ignore_fan_ctrl),
};
static struct attribute *wireless_charge_sysfs_attrs[ARRAY_SIZE(wireless_charge_sysfs_field_tbl) + 1];
static const struct attribute_group wireless_charge_sysfs_attr_group = {
	.attrs = wireless_charge_sysfs_attrs,
};

static void wireless_charge_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(wireless_charge_sysfs_attrs,
		wireless_charge_sysfs_field_tbl, ARRAY_SIZE(wireless_charge_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "wireless_charger",
		dev, &wireless_charge_sysfs_attr_group);
}

static void wireless_charge_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "wireless_charger",
		dev, &wireless_charge_sysfs_attr_group);
}
#else
static inline void wireless_charge_sysfs_create_group(struct device *dev)
{
}

static inline void wireless_charge_sysfs_remove_group(struct device *dev)
{
}
#endif

static ssize_t wireless_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int chrg_succ;
	int vrect = 0;
	int vout = 0;
	int iout = 0;
	int temp = 0;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_dev_info *di = dev_get_drvdata(dev);
	struct power_sysfs_attr_info *info = power_sysfs_lookup_attr(
		attr->attr.name, wireless_charge_sysfs_field_tbl,
		ARRAY_SIZE(wireless_charge_sysfs_field_tbl));

	if (!di || !dts || !info)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_CHARGE_SYSFS_CHIP_INFO:
		return wlrx_ic_get_chip_info(WLTRX_IC_MAIN, buf, PAGE_SIZE);
	case WIRELESS_CHARGE_SYSFS_KEY_ID:
		return snprintf(buf, PAGE_SIZE, "%d\n", dts->antifake_kid);
	case WIRELESS_CHARGE_SYSFS_TX_ADAPTOR_TYPE:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->tx_cap->type);
	case WIRELESS_CHARGE_SYSFS_RX_TEMP:
		(void)wlrx_ic_get_temp(WLTRX_IC_MAIN, &temp);
		return snprintf(buf, PAGE_SIZE, "%d\n", temp);
	case WIRELESS_CHARGE_SYSFS_VOUT:
		(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &vout);
		return snprintf(buf, PAGE_SIZE, "%d\n", vout);
	case WIRELESS_CHARGE_SYSFS_IOUT:
		(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &iout);
		return snprintf(buf, PAGE_SIZE, "%d\n", iout);
	case WIRELESS_CHARGE_SYSFS_VRECT:
		(void)wlrx_ic_get_vrect(WLTRX_IC_MAIN, &vrect);
		return snprintf(buf, PAGE_SIZE, "%d\n", vrect);
	case WIRELESS_CHARGE_SYSFS_EN_ENABLE:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.en_enable);
	case WIRELESS_CHARGE_SYSFS_NORMAL_CHRG_SUCC:
		chrg_succ = wireless_charge_check_normal_charge_succ(di);
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_FAST_CHRG_SUCC:
		chrg_succ = wireless_charge_check_fast_charge_succ(di);
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_FOD_COEF:
		return wlrx_ic_get_fod_coef(WLTRX_IC_MAIN, buf, PAGE_SIZE);
	case WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING:
		return snprintf(buf, PAGE_SIZE, "%u\n", wlrx_get_intfr_src());
	case WIRELESS_CHARGE_SYSFS_RX_SUPPORT_MODE:
		return snprintf(buf, PAGE_SIZE, "mode[support|current]:[0x%x|%s]\n",
			di->sysfs_data.rx_support_mode, dts->pmode_cfg[di->curr_pmode_index].name);
	case WIRELESS_CHARGE_SYSFS_THERMAL_CTRL:
		return snprintf(buf, PAGE_SIZE, "%u\n", di->sysfs_data.thermal_ctrl);
	case WIRELESS_CHARGE_SYSFS_NVM_DATA:
		return snprintf(buf, PAGE_SIZE, "%s\n", wlrx_ic_read_nvm_info(
			WLTRX_IC_MAIN, di->sysfs_data.nvm_sec_no));
	case WIRELESS_CHARGE_SYSFS_IGNORE_FAN_CTRL:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.ignore_fan_ctrl);
	default:
		break;
	}
	return 0;
}

static ssize_t wireless_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_dev_info *di = dev_get_drvdata(dev);
	struct power_sysfs_attr_info *info = power_sysfs_lookup_attr(
		attr->attr.name, wireless_charge_sysfs_field_tbl,
		ARRAY_SIZE(wireless_charge_sysfs_field_tbl));

	if (!di || !dts || !info)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_CHARGE_SYSFS_KEY_ID:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 1)) {
			hwlog_err("%s: please input 0 or 1\n", __func__);
			return -EINVAL;
		}
		dts->antifake_kid = (int)val;
		hwlog_info("[sysfs_store] kid=%d\n", dts->antifake_kid);
		break;
	case WIRELESS_CHARGE_SYSFS_EN_ENABLE:
		if ((strict_strtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 1))
			return -EINVAL;
		di->sysfs_data.en_enable = val;
		hwlog_info("set rx en_enable = %d\n", di->sysfs_data.en_enable);
		wlrx_ic_sleep_enable(WLTRX_IC_MAIN, val);
		wlps_control(WLTRX_IC_MAIN, WLPS_SYSFS_EN_PWR,
			di->sysfs_data.en_enable ? true : false);
		break;
	case WIRELESS_CHARGE_SYSFS_FOD_COEF:
		hwlog_info("[%s] set fod_coef: %s\n", __func__, buf);
		(void)wlrx_ic_set_fod_coef(WLTRX_IC_MAIN, buf);
		break;
	case WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING:
		if (strict_strtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		hwlog_info("[sysfs_store] interference_settings: 0x%x\n", val);
		wlrx_handle_intfr_settings((u8)val);
		break;
	case WIRELESS_CHARGE_SYSFS_RX_SUPPORT_MODE:
		if ((strict_strtol(buf, POWER_BASE_HEX, &val) < 0) ||
			(val < 0) || (val > WLC_RX_SP_ALL_MODE))
			return -EINVAL;
		if (!val)
			di->sysfs_data.rx_support_mode = WLC_RX_SP_ALL_MODE;
		else
			di->sysfs_data.rx_support_mode = val;
		hwlog_info("[%s] rx_support_mode = 0x%x", __func__, val);
		break;
	case WIRELESS_CHARGE_SYSFS_THERMAL_CTRL:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 0xFF)) /* 0xFF: maximum of u8 */
			return -EINVAL;
		wireless_set_thermal_ctrl((unsigned char)val);
		break;
	case WIRELESS_CHARGE_SYSFS_NVM_DATA:
		if ((strict_strtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0))
			return -EINVAL;
		di->sysfs_data.nvm_sec_no = val;
		break;
	case WIRELESS_CHARGE_SYSFS_IGNORE_FAN_CTRL:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 1)) /* 1: ignore 0:otherwise */
			return -EINVAL;
		hwlog_info("[%s] ignore_fan_ctrl=0x%x", __func__, val);
		di->sysfs_data.ignore_fan_ctrl = val;
		break;
	default:
		break;
	}
	return count;
}

static struct wlrx_dev_info *wlrx_dev_info_alloc(void)
{
	struct wlrx_dev_info *di = NULL;
	struct wireless_protocol_tx_cap *tx_cap = NULL;
	struct wlrx_pctrl *pctrl = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return NULL;
	tx_cap = kzalloc(sizeof(*tx_cap), GFP_KERNEL);
	if (!tx_cap)
		goto tx_cap_fail;
	pctrl = kzalloc(sizeof(*pctrl), GFP_KERNEL);
	if (!pctrl)
		goto pctrl_fail;

	di->tx_cap = tx_cap;
	di->pctrl = pctrl;
	return di;

pctrl_fail:
	kfree(tx_cap);
tx_cap_fail:
	kfree(di);
	return NULL;
}

static void wlrx_dev_info_free(struct wlrx_dev_info *di)
{
	if (di) {
		if (di->tx_cap) {
			kfree(di->tx_cap);
			di->tx_cap = NULL;
		}
		if (di->pctrl) {
			kfree(di->pctrl);
			di->pctrl = NULL;
		}
		if (di->iout_ctrl_data.ictrl_para) {
			kfree(di->iout_ctrl_data.ictrl_para);
			di->iout_ctrl_data.ictrl_para = NULL;
		}
		kfree(di);
		di = NULL;
	}
	g_wlrx_di = NULL;
}

static void wlrx_init_probe_para(struct wlrx_dev_info *di)
{
	di->sysfs_data.rx_support_mode = WLC_RX_SP_ALL_MODE;
	di->qval_support_mode = WLC_RX_SP_ALL_MODE;
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	di->sysfs_data.rx_support_mode &= ~WLC_RX_SP_SC_2_MODE;
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */
	di->discon_delay_time = WL_DISCONN_DELAY_MS;
	wireless_charge_set_default_tx_capability(di);
}

static void wireless_charge_shutdown(struct platform_device *pdev)
{
	int ret;
	struct wlrx_dev_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return;
	}
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		di->pwroff_reset_flag = true;
		wireless_charge_switch_off();
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
		msleep(50); /* dalay 50ms for power off */
		ret = wireless_charge_set_tx_vout(ADAPTER_5V *
			WL_MVOLT_PER_VOLT);
		if (ret)
			hwlog_err("%s: wlc sw control fail\n", __func__);
	}
	wlrx_ic_sleep_enable(WLTRX_IC_MAIN, true);
	cancel_delayed_work(&di->rx_sample_work);
	cancel_delayed_work(&di->wireless_ctrl_work);
	cancel_delayed_work(&di->wireless_monitor_work);
	cancel_delayed_work(&di->wireless_watchdog_work);
	cancel_delayed_work(&di->ignore_qval_work);
	wlrx_kfree_dts();
	hwlog_info("%s --\n", __func__);
}

static int wireless_charge_remove(struct platform_device *pdev)
{
	struct wlrx_dev_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return 0;
	}

	power_event_bnc_unregister(POWER_BNT_CHG, &di->chrg_event_nb);
	power_event_bnc_unregister(POWER_BNT_WLRX, &di->rx_event_nb);
	wireless_charge_sysfs_remove_group(di->dev);
	wlrx_kfree_dts();
	wakeup_source_trash(&g_rx_con_wakelock);

	hwlog_info("%s --\n", __func__);

	return 0;
}

static void wlrx_module_deinit(unsigned int drv_type)
{
	wlrx_fod_deinit(drv_type);
}

static int wlrx_module_init(unsigned int drv_type, const struct device_node *np)
{
	int ret;

	ret = wlrx_fod_init(drv_type, np);
	if (ret) {
		wlrx_module_deinit(drv_type);
		return -EINVAL;
	}

	return 0;
}

static int wireless_charge_probe(struct platform_device *pdev)
{
	int ret;
	struct wlrx_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!wlrx_ic_is_ops_registered(WLTRX_IC_MAIN))
		return -ENOTSUPP;

	di = wlrx_dev_info_alloc();
	if (!di) {
		hwlog_err("alloc di failed\n");
		return -ENOMEM;
	}

	g_wlrx_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	platform_set_drvdata(pdev, di);
	wakeup_source_init(&g_rx_con_wakelock, "rx_con_wakelock");

	ret = wlc_parse_dts(np, di);
	if (ret)
		goto wireless_charge_fail_0;
	ret = wlrx_parse_dts(np);
	if (ret)
		goto wireless_charge_fail_0;

	ret = wlrx_module_init(WLTRX_DRV_MAIN, np);
	if (ret)
		goto module_init_fail;

	wlrx_init_probe_para(di);

	mutex_init(&g_rx_en_mutex);
	INIT_WORK(&di->wired_vbus_connect_work, wireless_charge_wired_vbus_connect_work);
	INIT_WORK(&di->wired_vbus_disconnect_work, wireless_charge_wired_vbus_disconnect_work);
	INIT_DELAYED_WORK(&di->rx_event_work, wlc_rx_event_work);
	INIT_WORK(&di->wireless_pwroff_reset_work, wireless_charge_pwroff_reset_work);
	INIT_DELAYED_WORK(&di->wireless_ctrl_work, wireless_charge_control_work);
	INIT_DELAYED_WORK(&di->rx_sample_work, wireless_charge_rx_sample_work);
	INIT_DELAYED_WORK(&di->wireless_monitor_work, wireless_charge_monitor_work);
	INIT_DELAYED_WORK(&di->wireless_watchdog_work, wireless_charge_watchdog_work);
	INIT_DELAYED_WORK(&di->wireless_vbus_disconnect_work,
		wireless_charge_wireless_vbus_disconnect_work);
	INIT_DELAYED_WORK(&di->ignore_qval_work, wlc_ignore_qval_work);

	di->rx_event_nb.notifier_call = wireless_charge_rx_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLRX, &di->rx_event_nb);
	if (ret < 0) {
		hwlog_err("register rx_connect notifier failed\n");
		goto  wireless_charge_fail_1;
	}
	di->chrg_event_nb.notifier_call = wireless_charge_chrg_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHG, &di->chrg_event_nb);
	if (ret < 0) {
		hwlog_err("register charger_event notifier failed\n");
		goto  wireless_charge_fail_2;
	}
	di->pwrkey_nb.notifier_call = wireless_charge_pwrkey_event_notifier_call;
	ret = powerkey_register_notifier(&di->pwrkey_nb);
	if (ret < 0) {
		hwlog_err("register power_key notifier failed\n");
		goto  wireless_charge_fail_3;
	}
	if (wlrx_ic_is_tx_exist(WLTRX_IC_MAIN)) {
		wireless_charge_para_init(di);
		charger_source_sink_event(START_SINK_WIRELESS);
		pd_dpm_ignore_vbus_only_event(true);
		wlc_wireless_vbus_connect_handler(WLRX_STAGE_HANDSHAKE);
		schedule_delayed_work(&di->wireless_monitor_work, msecs_to_jiffies(0));
		schedule_delayed_work(&di->wireless_watchdog_work, msecs_to_jiffies(0));
	} else {
		wireless_charge_switch_off();
	}
	wireless_charge_sysfs_create_group(di->dev);
	power_if_ops_register(&wl_if_ops);
	hwlog_info("wireless_charger probe ok\n");
	return 0;

wireless_charge_fail_3:
	power_event_bnc_unregister(POWER_BNT_CHG, &di->chrg_event_nb);
wireless_charge_fail_2:
	power_event_bnc_unregister(POWER_BNT_WLRX, &di->rx_event_nb);
wireless_charge_fail_1:
	wlrx_module_deinit(WLTRX_DRV_MAIN);
module_init_fail:
	wlrx_kfree_dts();
wireless_charge_fail_0:
	wakeup_source_trash(&g_rx_con_wakelock);
	wlrx_dev_info_free(di);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static struct of_device_id wireless_charge_match_table[] = {
	{
		.compatible = "huawei,wireless_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver wireless_charge_driver = {
	.probe = wireless_charge_probe,
	.remove = wireless_charge_remove,
	.shutdown = wireless_charge_shutdown,
	.driver = {
		.name = "huawei,wireless_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wireless_charge_match_table),
	},
};

static int __init wireless_charge_init(void)
{
	hwlog_info("wireless_charger init ok\n");

	return platform_driver_register(&wireless_charge_driver);
}

static void __exit wireless_charge_exit(void)
{
	platform_driver_unregister(&wireless_charge_driver);
}

device_initcall_sync(wireless_charge_init);
module_exit(wireless_charge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wireless charge module driver");
MODULE_AUTHOR("HUAWEI Inc");
