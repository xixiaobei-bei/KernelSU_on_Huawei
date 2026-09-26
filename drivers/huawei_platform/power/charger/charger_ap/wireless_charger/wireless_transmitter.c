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
#include <huawei_platform/power/wireless/wireless_tx_cap.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/jiffies.h>
#include <chipset_common/hwpower/wireless_charge/wireless_accessory.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_acc.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>

#define HWLOG_TAG wireless_tx
HWLOG_REGIST();

static struct wakeup_source wireless_tx_wakelock;
static struct wltx_dev_info *g_wltx_di;
static struct wltx_logic_ops *g_wltx_logic_ops[WLTX_OPEN_TYPE_END];
static enum wltx_stage g_tx_stage = WL_TX_STAGE_DEFAULT;
static enum wireless_tx_status_type tx_status = WL_TX_STATUS_DEFAULT;
static struct wltx_stage_info {
	enum wltx_stage tx_stg;
	char tx_stage_name[WL_TX_STR_LEN_32];
} const g_tx_stg[WL_TX_STAGE_TOTAL] = {
	{ WL_TX_STAGE_DEFAULT, "STAGE_DEFAULT" },
	{ WL_TX_STAGE_POWER_SUPPLY, "STAGE_POWER_SUPPLY" },
	{ WL_TX_STAGE_CHIP_INIT, "STAGE_CHIP_INIT" },
	{ WL_TX_STAGE_PING_RX, "STAGE_PING_RX" },
	{ WL_TX_STAGE_REGULATION, "STAGE_REGULATION" },
};

static unsigned int tx_iin_samples[WL_TX_IIN_SAMPLE_LEN];
static unsigned int tx_fop_samples[WL_TX_FOP_SAMPLE_LEN];
static bool tx_open_flag; /* record the UI operation state */
static int tx_iin_limit[WL_TX_CHARGER_TYPE_MAX] = {0};
static int g_init_tbatt;

int wireless_tx_logic_ops_register(struct wltx_logic_ops *ops)
{
	if (!ops || (ops->type < WLTX_OPEN_TYPE_BEGIN) ||
		(ops->type >= WLTX_OPEN_TYPE_END))
		return -1;

	hwlog_info("[%s] type = %d\n", __func__, ops->type);
	if (g_wltx_logic_ops[ops->type])
		return -1;

	g_wltx_logic_ops[ops->type] = ops;

	return 0;
}

struct wltx_dev_info *wltx_get_dev_info(void)
{
	return g_wltx_di;
}

int wireless_tx_get_tx_status(void)
{
	return tx_status;
}

bool wireless_tx_get_tx_open_flag(void)
{
	return tx_open_flag;
}

bool wltx_is_tx_open_by_hall(void)
{
	if (!g_wltx_di)
		return false;

	return g_wltx_di->tx_open_type == WLTX_OPEN_BY_HALL;
}

bool wireless_is_in_tx_mode(void)
{
	int i;

	for (i = 0; i < WL_TX_MODE_ERR_CNT1; i++) {
		if ((g_tx_stage > WL_TX_STAGE_CHIP_INIT) &&
			wltx_ic_is_in_tx_mode(WLTRX_IC_MAIN))
			return true;
	}

	return false;
}

static void wireless_tx_wake_lock(void)
{
	if (!wireless_tx_wakelock.active) {
		__pm_stay_awake(&wireless_tx_wakelock);
		hwlog_info("wireless_tx wake lock\n");
	}
}

static void wireless_tx_wake_unlock(void)
{
	if (wireless_tx_wakelock.active) {
		__pm_relax(&wireless_tx_wakelock);
		hwlog_info("wireless_tx wake unlock\n");
	}
}

int wltx_msleep(int sleep_ms)
{
	int i;
	int interval = 25; /* sleep interval, ms */
	int cnt = sleep_ms / interval;
	struct wltx_dev_info *di = g_wltx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}

	for (i = 0; i < cnt; i++) {
		msleep(interval);
		if (!tx_open_flag || di->stop_reverse_charge) {
			hwlog_err("%s: TX mode has stopped\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int wltx_check_abnormal_vbatt(void)
{
	int i;
	int vbatt;

	/* vbatt_th=3450mV, 20ms*3=60ms timeout for vbatt debouncing */
	for (i = 0; i < 3; i++) {
		vbatt = coul_drv_battery_voltage();
		if (tx_open_flag && (vbatt < 3450)) {
			hwlog_err("abnormal vbatt=%d\n", vbatt);
			msleep(20);
		} else {
			break;
		}
	}
	if (i >= 3) {
		tx_open_flag = false;
		return -EINVAL;
	}

	return 0;
}

static int wltx_check_abnormal_ibatt(void)
{
	int i;
	int idischrg;
	int idischrg_max;
	bool high_power_charge = false;
	u8 tx_cap_type;
	struct wltx_dev_info *di = g_wltx_di;

	if (!di)
		return -EINVAL;

	tx_cap_type = wltx_get_tx_cap_type();
	if ((tx_cap_type == TX_TYPE_FCP) ||
		(di->tx_vset.para[di->tx_vset.cur].ext_hdl == WLTX_EXT_HDL_BP2CP)) {
		idischrg_max = di->tx_bp2cp_abnormal_ibat;
		high_power_charge = true;
	} else {
		idischrg_max = WLTX_ABNORMAL_IBAT;
	}

	/* 20ms*3=60ms timeout for ibatt debouncing */
	for (i = 0; i < 3; i++) {
		idischrg = coul_drv_battery_current();
		if (tx_open_flag && (idischrg > idischrg_max)) {
			hwlog_err("abnormal idischrg=%d\n", idischrg);
			if (di->need_reduce_power && high_power_charge) {
				wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, false);
				wltx_disable_pwr_supply();
				usleep_range(9500, 10500);
				di->stop_reverse_charge = true;
				di->tx_pd_flag = true;
				wltx_set_exp_cap_id(WLTX_DFLT_CAP);
				return 0;
			}
			msleep(20);
		} else {
			break;
		}
	}
	if (i >= 3) {
		tx_open_flag = false;
		return -EINVAL;
	}

	return 0;
}

static void wltx_chk_abnormal_tx_vin(void)
{
	int i;
	u16 tx_vin = 0;
	const char *src = NULL;
	struct wltx_dev_info *di = g_wltx_di;

	if (!di)
		return;

	src = wltx_get_pwr_src_name(di->cur_pwr_src);
	if (!strstr(src, "CP"))
		return;

	if (tx_status != WL_TX_STATUS_IN_CHARGING)
		return;

	/* tx_vin_th=7500mV, 20ms*3=60ms timeout for tx_vin debouncing */
	for (i = 0; i < 3; i++) {
		(void)wltx_ic_get_vin(WLTRX_IC_MAIN, &tx_vin);
		if (tx_open_flag && (tx_vin < 7500)) {
			hwlog_err("abnormal tx_vin=%u\n", tx_vin);
			msleep(20);
		} else {
			break;
		}
	}
	if (i >= 3) {
		wireless_tx_set_tx_status(WL_TX_STATUS_RX_DISCONNECT);
		di->stop_reverse_charge = true;
	}
}

static void wltx_check_abnormal_power(void)
{
	if (wltx_check_abnormal_vbatt())
		return;

	if (wltx_check_abnormal_ibatt())
		return;

	wltx_chk_abnormal_tx_vin();
}

void wireless_tx_set_tx_open_flag(bool enable)
{
	(void)wltx_ic_set_open_flag(WLTRX_IC_MAIN, enable);
	tx_open_flag = enable;
	hwlog_info("[%s] set tx_open_flag = %d\n", __func__, tx_open_flag);

	if (!enable)
		direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
			DC_DISABLE_WIRELESS_TX);
}

static void wireless_tx_set_stage(enum wltx_stage stage)
{
	g_tx_stage = stage;
	hwlog_info("[set_stage] %s\n", g_tx_stg[g_tx_stage].tx_stage_name);
}

enum wltx_stage wireless_tx_get_stage(void)
{
	return g_tx_stage;
}

void wireless_tx_set_tx_status(enum wireless_tx_status_type event)
{
	if (!tx_open_flag && (event > WL_TX_STATUS_DEFAULT) &&
		(event < WL_TX_STATUS_FAULT_BASE)) {
		hwlog_err("set_tx_status: tx_open_flag=%d,status=0x%x,return\n",
			tx_open_flag, event);
		return;
	}
	tx_status = event;
	hwlog_info("[set_tx_status] 0x%x\n", tx_status);
}

int wltx_get_tx_ilimit(int charger_type, int ibuck)
{
	struct wltx_dev_info *di = g_wltx_di;

	if (!di || !tx_open_flag ||
		(wireless_tx_get_stage() == WL_TX_STAGE_DEFAULT) ||
		(charger_type >= WL_TX_CHARGER_TYPE_MAX) || (charger_type < 0))
		return 0;

	if ((di->cur_pwr_sw_scn == PWR_SW_BY_DC_DONE) && (di->cur_pwr_src == PWR_SRC_VBUS_CP))
		return di->tx_dc_done_buck_ilim;

	return ibuck - tx_iin_limit[charger_type];
}

static void wltx_calc_tx_fop_avg(struct wltx_dev_info *di, unsigned int tx_fop)
{
	int i;
	int fop_sum = 0;
	static int index;

	tx_fop_samples[index] = tx_fop;
	index = (index + 1) % WL_TX_FOP_SAMPLE_LEN; /* 1:next sample */
	for (i = 0; i < WL_TX_FOP_SAMPLE_LEN; i++)
		fop_sum += tx_fop_samples[i];

	di->tx_fop_avg = fop_sum / WL_TX_FOP_SAMPLE_LEN;
}

static void wltx_reset_avg_tx_fop(struct wltx_dev_info *di)
{
	int i = 0;

	for (; i < WL_TX_FOP_SAMPLE_LEN; i++)
		tx_fop_samples[i] = 0;

	di->tx_fop_avg = 0;
}

static void wireless_tx_calc_tx_iin_avg(struct wltx_dev_info *di, unsigned int tx_iin)
{
	static int index = 0;
	int iin_sum = 0;
	int i;

	tx_iin_samples[index] = tx_iin;
	index = (index + 1) % WL_TX_IIN_SAMPLE_LEN;
	for (i = 0; i < WL_TX_IIN_SAMPLE_LEN; i++)
		iin_sum += tx_iin_samples[i];
	di->tx_iin_avg = iin_sum / WL_TX_IIN_SAMPLE_LEN;
}

static void wireless_tx_reset_avg_iout(struct wltx_dev_info *di)
{
	int i;

	for (i = 0; i < WL_TX_IIN_SAMPLE_LEN; i++)
		tx_iin_samples[i] = 0;
	di->tx_iin_avg = 0;
}

static void wltx_reset_rx_data(struct wltx_dev_info *di)
{
	di->rx_data.pmax = 0;
	di->rx_data.product_type = QI_RX_TYPE_UNKNOWN;
}

static int wltx_check_handshake(struct wltx_dev_info *di)
{
	if ((tx_status < WL_TX_STATUS_PING_SUCC) ||
		(tx_status >= WL_TX_STATUS_FAULT_BASE))
		return -1;

	return 0;
}

static void wireless_tx_dsm_dump(struct wltx_dev_info *di, char *dsm_buff)
{
	int ret, i;
	char buff[ERR_NO_STRING_SIZE] = {0};
	u16 tx_iin = 0;
	u16 tx_vin = 0;
	u16 tx_vrect = 0;
	s16 chip_temp = 0;
	int soc = coul_drv_battery_capacity();
	int tbatt = 0;
	int charger_vbus = charge_get_vbus();

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	ret = wltx_ic_get_vrect(WLTRX_IC_MAIN, &tx_vrect);
	ret += wltx_ic_get_iin(WLTRX_IC_MAIN, &tx_iin);
	ret += wltx_ic_get_vin(WLTRX_IC_MAIN, &tx_vin);
	ret += wltx_ic_get_temp(WLTRX_IC_MAIN, &chip_temp);
	if (ret)
		hwlog_err("%s: get tx vin/iin/vrect... fail", __func__);
	snprintf(buff, sizeof(buff),
		"soc=%d tbatt=%d init_tbatt=%d charger_vbus=%dmV "
		"tx_vrect=%umV tx_vin=%umV tx_iin=%umA tx_iin_avg=%umA chip_temp=%d\n",
		soc, tbatt, g_init_tbatt, charger_vbus, tx_vrect, tx_vin, tx_iin,
		di->tx_iin_avg, chip_temp);
	strncat(dsm_buff, buff, strlen(buff));
	snprintf(buff, ERR_NO_STRING_SIZE, "tx_iin(mA): ");
	strncat(dsm_buff, buff, strlen(buff));
	for (i = 0; i < WL_TX_IIN_SAMPLE_LEN; i++) {
		snprintf(buff, ERR_NO_STRING_SIZE, "%d ", tx_iin_samples[i]);
		strncat(dsm_buff, buff, strlen(buff));
	}
}

static void wireless_tx_dsm_report(int err_no, char *dsm_buff)
{
	struct wltx_dev_info *di = g_wltx_di;

	if (di) {
		wireless_tx_dsm_dump(di, dsm_buff);
		power_dsm_report_dmd(POWER_DSM_BATTERY, err_no, dsm_buff);
	}
}

static void wireless_tx_check_rx_disconnect(struct wltx_dev_info *di)
{
	if (wltx_ic_is_rx_disconnect(WLTRX_IC_MAIN)) {
		hwlog_info("[%s] rx disconnect\n", __func__);
		wireless_tx_set_tx_status(WL_TX_STATUS_RX_DISCONNECT);
		di->stop_reverse_charge = true;
	}
}

static void wireless_tx_enable_tx_mode(bool enable)
{
	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF) {
		(void)wltx_ic_mode_enable(WLTRX_IC_MAIN, enable);
		hwlog_info("[%s] enable=%d, wired_state:off\n", __func__, enable);
	} else {
		/* If enable equals false in wired charging, chip should be closed */
		wltx_ic_chip_enable(WLTRX_IC_MAIN, enable);
		if (enable)
			(void)wltx_ic_mode_enable(WLTRX_IC_MAIN, enable);
		hwlog_info("[%s] enable=%d, wired_state:on\n", __func__, enable);
	}
}

void wltx_reset_reverse_charging(void)
{
	if (tx_open_flag) {
		wireless_tx_set_tx_open_flag(false);
		wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		wltx_disable_pwr_supply();
		coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	}
}

void wltx_ps_get_tx_vin(u16 *vin)
{
	u16 tx_vin = 0;
	struct wltx_dev_info *di = g_wltx_di;

	if (!vin || !di)
		return;

	if ((di->cur_pwr_src == PWR_SRC_VBUS) ||
		(di->cur_pwr_src == PWR_SRC_OTG)) {
		tx_vin = charge_get_vbus();
	} else {
		wltx_ic_chip_enable(WLTRX_IC_MAIN, true);
		if (wltx_ic_get_vin(WLTRX_IC_MAIN, &tx_vin)) {
			hwlog_err("ps_get_tx_vin: failed\n");
			tx_vin = 0;
		}
	}

	*vin = tx_vin;
}

static int wireless_tx_power_supply(struct wltx_dev_info *di)
{
	int ret;
	int count = 0;
	u16 tx_vin = 0;
	int charger_vbus;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = {0};

	wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, false);
	ret = wltx_enable_pwr_supply();
	if (ret)
		return ret;

	wltx_set_tx_volt(WL_TX_STAGE_POWER_SUPPLY, di->tx_vset.v_ps, true);
	do {
		msleep(WL_TX_VIN_SLEEP_TIME);
		wltx_check_abnormal_power();
		wltx_ps_get_tx_vin(&tx_vin);
		charger_vbus = charge_get_vbus();
		if ((tx_vin >= di->tx_vset.para[di->tx_vset.cur].lth) &&
			(tx_vin <= di->tx_vset.para[di->tx_vset.cur].hth)) {
			hwlog_info("[%s] tx_vin = %dmV, charger_vbus = %dmV, "
				"power supply succ\n", __func__, tx_vin, charger_vbus);
			wltx_ic_chip_enable(WLTRX_IC_MAIN, true);
			return WL_TX_SUCC;
		}
		if (!tx_open_flag || di->stop_reverse_charge) {
			hwlog_err("power_supply: stop, tx_open=%d, stop=%d\n",
				tx_open_flag, di->stop_reverse_charge);
			return WL_TX_FAIL;
		}
		count++;
		if (count == WL_TX_VIN_RETRY_CNT1) {
			wltx_enable_extra_pwr_supply();
			wltx_set_tx_volt(WL_TX_STAGE_POWER_SUPPLY, di->tx_vset.v_ps, true);
		}
		hwlog_info("[%s] tx_vin = %dmV, charger_vbus = %dmV, "
			"retry times = %d\n", __func__, tx_vin, charger_vbus, count);
	} while (count < WL_TX_VIN_RETRY_CNT2);

	hwlog_err("%s: power supply for TX fail\n", __func__);
	wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
	wireless_tx_set_tx_open_flag(false);
	wireless_tx_dsm_report(ERROR_WIRELESS_TX_POWER_SUPPLY_FAIL, dsm_buff);
	return WL_TX_FAIL;
}

static int wireless_tx_limit_iout(struct wltx_dev_info *di)
{
	int ret;
	u8 tx_cap_type;
	u16 tx_ilim;

	if ((di->cur_pwr_sw_scn == PWR_SW_BY_OTG_ON) &&
		(di->cur_pwr_src == PWR_SRC_5VBST)) {
		ret = wltx_ic_set_min_fop(WLTRX_IC_MAIN, WL_TX_5VBST_MIN_FOP);
		if (ret) {
			hwlog_err("%s: set_min_fop fail\n", __func__);
			return ret;
		}
	}

	if (di && di->logic_ops && di->logic_ops->reinit_tx_chip)
		di->logic_ops->reinit_tx_chip();

	tx_cap_type = wltx_get_tx_cap_type();
	if (tx_cap_type == TX_TYPE_FCP)
		tx_ilim = WLTX_BP2CP_PWR_ILIM;
	else
		tx_ilim = (u16)di->tx_high_pwr_ilim;

	return wltx_ic_set_ilim(WLTRX_IC_MAIN, tx_ilim);
}

static void wireless_tx_para_init(struct wltx_dev_info *di)
{
	di->stop_reverse_charge = false;
	di->i2c_err_cnt = 0;
	di->tx_iin_low_cnt = 0;
	di->tx_mode_err_cnt = 0;
	di->standard_rx = false;
	di->tx_pd_flag = false;
	di->tx_cap.cur_id = WLTX_DFLT_CAP;
	di->tx_vout.v_ask = WIRELESS_ADAPTER_5V;
	di->hp_time_out = 0;
	di->irq_vset_time_out = 0;
	di->tx_rp_timeout_lim_volt = 0;
	di->monitor_interval = WL_TX_MONITOR_INTERVAL;
	wltx_reset_rx_data(di);
	wireless_tx_reset_avg_iout(di);
	wltx_reset_avg_tx_fop(di);
	wltx_reset_vset_para();
	wltx_reset_alarm_data();
}

static int wireless_tx_chip_init(struct wltx_dev_info *di)
{
	int ret;

	if (!tx_open_flag || di->stop_reverse_charge) {
		hwlog_err("%s: already stop, open_flag=%d, stop_charge_flag=%d\n",
			__func__, tx_open_flag, di->stop_reverse_charge);
		return WL_TX_FAIL;
	}

	if (di->cur_pwr_src == PWR_SRC_SPBST)
		wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, true);
	(void)wltx_ic_fw_update(WLTRX_IC_MAIN);
	ret = wltx_ic_chip_init(WLTRX_IC_MAIN, di->tx_open_type);
	if (ret) {
		hwlog_err("%s: TX chip init fail\n", __func__);
		return WL_TX_FAIL;
	}
	wltx_set_tx_volt(WL_TX_STAGE_CHIP_INIT, di->tx_vset.v_ping, true);
	ret = wireless_tx_limit_iout(di);
	if (ret)
		hwlog_err("%s: limit TX iout fail\n", __func__);
	(void)wltx_ic_set_rp_dm_to(WLTRX_IC_MAIN, WLTX_RP_DEMODULE_TIMEOUT_VAL);
	hwlog_info("%s: TX chip init succ\n", __func__);
	return WL_TX_SUCC;
}

static void wireless_tx_check_in_tx_mode(struct wltx_dev_info *di)
{
	int ret;

	if (!wltx_ic_is_in_tx_mode(WLTRX_IC_MAIN)) {
		if (++di->tx_mode_err_cnt >= WL_TX_MODE_ERR_CNT2) {
			hwlog_err("%s: not in tx mode, close TX\n", __func__);
			wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
			di->stop_reverse_charge = true;
		} else if (di->tx_mode_err_cnt >= WL_TX_MODE_ERR_CNT) {
			hwlog_err("%s: not in tx mode, reinit TX\n", __func__);
			ret = wireless_tx_chip_init(di);
			if (ret)
				hwlog_err("%s: chip_init fail\n", __func__);
			wireless_tx_enable_tx_mode(true);
		}
	} else {
		di->tx_mode_err_cnt = 0;
	}
}

static int wltx_ping_rx_para_reset(struct wltx_dev_info *di,
	struct wltx_ping_para *ping)
{
	ping->time_now = current_kernel_time64();
	ping->time_interval.tv_sec = di->ping_timeout;
	ping->time_out = timespec64_add_safe(ping->time_now, ping->time_interval);
	if (ping->time_out.tv_sec == TIME_T_MAX) {
		hwlog_err("ping_rx_para_reset: time_out val overflow\n");
		return -EINVAL;
	}

	return 0;
}

static int wltx_ping_rx_vin_uvp_check(struct wltx_dev_info *di,
	struct wltx_ping_para *ping, u16 vin)
{
	if (vin < di->tx_vset.para[di->tx_vset.cur].lth) {
		ping->vin_uvp_flag = true;
	} else if (vin >= di->tx_vset.para[di->tx_vset.cur].lth) {
		if (ping->vin_uvp_flag &&
			(++ping->vin_uvp_cnt <= WLTX_PING_RX_VIN_UVP_CNT)) {
			hwlog_err("ping_rx_vin_uvp_check: uvp_cnt=%d\n", ping->vin_uvp_cnt);
			if (wireless_tx_chip_init(di))
				return -EINVAL;

			wireless_tx_enable_tx_mode(true);
		}
		ping->vin_uvp_flag = false;
	}

	if ((ping->vin_uvp_cnt >= WLTX_PING_RX_VIN_UVP_CNT)) {
		hwlog_err("ping_rx_vin_uvp_check: cnt max, close tx\n");
		return -EINVAL;
	}

	return 0;
}

static int wltx_ping_rx_vin_ovp_check(struct wltx_dev_info *di,
	struct wltx_ping_para *ping, u16 vin)
{
	if ((wired_chsw_get_wired_channel(WIRED_CHANNEL_ALL) == WIRED_CHANNEL_CUTOFF) &&
		(vin >= di->tx_vset.para[di->tx_vset.cur].hth)) {
		if (++ping->vin_ovp_cnt >= WLTX_PING_RX_VIN_OVP_CNT) {
			hwlog_err("ping_rx_vin_ovp_check: tx_vin over %dmV for %dms\n",
				di->tx_vset.para[di->tx_vset.cur].hth,
				ping->vin_ovp_cnt * WLTX_PING_CHECK_INTERVAL);
			wltx_disable_pwr_supply();
			return -EINVAL;
		}
	} else {
		ping->vin_ovp_cnt = 0;
	}
	return 0;
}

static int wltx_ping_rx_vin_check(struct wltx_dev_info *di,
	struct wltx_ping_para *ping)
{
	u16 vin = 0;

	if (wltx_ic_get_vin(WLTRX_IC_MAIN, &vin)) {
		hwlog_err("ping_rx_vin_check: get tx_vin failed\n");
		wireless_tx_enable_tx_mode(true);
		ping->vin_err_cnt++;
	} else if (!vin) {
		vin = di->tx_vset.para[di->tx_vset.cur].vset;
	} else if ((vin < di->tx_vset.para[di->tx_vset.cur].lth) ||
		(vin >= di->tx_vset.para[di->tx_vset.cur].hth)) {
		hwlog_err("ping_rx_vin_check: tx_vin=%umV\n", vin);
		if (vin < (di->tx_vset.para[di->tx_vset.cur].vset / POWER_HALF))
			ping->vin_err_cnt++;
	}

	if (ping->vin_err_cnt >= WLTX_PING_RX_VIN_ERR_CNT) {
		hwlog_err("ping_rx_vin_check: err_cnt=%d\n", ping->vin_err_cnt);
		goto vin_abnormal;
	}
	if (wltx_ping_rx_vin_uvp_check(di, ping, vin))
		goto vin_abnormal;

	if (wltx_ping_rx_vin_ovp_check(di, ping, vin))
		goto vin_abnormal;

	return 0;

vin_abnormal:
	wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
	wireless_tx_set_tx_open_flag(false);
	return -EINVAL;
}

static int wireless_tx_ping_rx(struct wltx_dev_info *di)
{
	int ret;
	struct wltx_ping_para ping = {0};
	char dsm_buff[POWER_DSM_BUF_SIZE_0256] = {0};

	if (di->ping_timeout == WL_TX_PING_TIMEOUT_2) {
		wltx_ic_chip_reset(WLTRX_IC_MAIN);
		msleep(150); /* only used here */
		ret = wireless_tx_chip_init(di);
		if (ret)
			hwlog_err("ping_rx: chip_init failed\n");
	}

	wltx_set_tx_volt(WL_TX_STAGE_PING_RX, di->tx_vset.v_ping, true);
	wireless_tx_enable_tx_mode(true);
	if (wltx_ping_rx_para_reset(di, &ping))
		return -EINVAL;

	wireless_tx_set_tx_status(WL_TX_STATUS_PING);
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	while (timespec64_compare(&ping.time_now, &ping.time_out) < 0) {
		/* wait for config packet interrupt */
		if (wireless_tx_get_tx_status() == WL_TX_STATUS_PING_SUCC)
			return WL_TX_SUCC;

		wltx_check_abnormal_power();
		if (wltx_ping_rx_vin_check(di, &ping))
			return -EINVAL;

		msleep(WLTX_PING_CHECK_INTERVAL);
		if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF)
			ping.time_now = current_kernel_time64();
		if (!tx_open_flag || di->stop_reverse_charge) {
			hwlog_err("ping_rx: tx_open_flag=%d,stop_reverse_charge=%d\n",
				tx_open_flag, di->stop_reverse_charge);
			return -EINVAL;
		}
	}
	wireless_tx_set_tx_open_flag(false);
	wireless_tx_set_tx_status(WL_TX_STATUS_PING_TIMEOUT);
	hwlog_err("ping_rx: timeout\n");
	if (di->ping_timeout == WL_TX_PING_TIMEOUT_1) {
		snprintf(dsm_buff, sizeof(dsm_buff), "TX ping RX timeout\n");
		power_dsm_report_dmd(POWER_DSM_BATTERY, ERROR_WIRELESS_TX_STATISTICS, dsm_buff);
	}
	return WL_TX_FAIL;
}

static int wireless_tx_can_do_reverse_charging(void)
{
	int batt_temp = 0;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = {0};
	struct wltx_dev_info *di = g_wltx_di;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &batt_temp);
	if (batt_temp <= WL_TX_BATT_TEMP_MIN) {
		hwlog_info("[%s] battery temperature %d is too low th: %d\n",
			__func__, batt_temp, WL_TX_BATT_TEMP_MIN);
		wireless_tx_set_tx_status(WL_TX_STATUS_TBATT_LOW);
		return WL_TX_FAIL;
	}
	if (batt_temp >= WL_TX_BATT_TEMP_MAX) {
		hwlog_info("[%s] battery temperature %d is too high th: %d\n",
			__func__, batt_temp, WL_TX_BATT_TEMP_MAX);
		wireless_tx_set_tx_status(WL_TX_STATUS_TBATT_HIGH);
		if (batt_temp - g_init_tbatt > WL_TX_TBATT_DELTA_TH)
			wireless_tx_dsm_report(ERROR_WIRELESS_TX_BATTERY_OVERHEAT, dsm_buff);
		return WL_TX_FAIL;
	}
	if (wltx_ic_is_in_rx_mode(WLTRX_IC_MAIN)) {
		hwlog_info("[%s] in wireless charging, can not enable tx mode\n", __func__);
		wireless_tx_set_tx_status(WL_TX_STATUS_IN_WL_CHARGING);
		return WL_TX_FAIL;
	} else {
		wlc_reset_wireless_charge();
	}

	if (di && di->logic_ops && di->logic_ops->can_rev_charge_check &&
		!di->logic_ops->can_rev_charge_check())
		return WL_TX_FAIL;

	return WL_TX_SUCC;
}

static int wltx_get_rx_pmax(struct wltx_dev_info *di, u16 *rx_pmax)
{
	if (di->rx_data.pmax > 0) {
		*rx_pmax = di->rx_data.pmax;
	} else if (di->rx_data.product_type == QI_RX_TYPE_EARPHONE_BOX) {
		*rx_pmax = QI_EARPHONE_BOX_MIN_PWR;
	} else {
		*rx_pmax = WLTX_DEFAULT_RX_PMAX;
		return -EINVAL;
	}

	return 0;
}

static bool wltx_rx_disconnect_need_power_off(struct wltx_dev_info *di)
{
	u16 rx_pmax = 0;
	int tx_vset = di->tx_vset.para[di->tx_vset.cur].vset;

	if (di->tx_vset.para[di->tx_vset.cur].ext_hdl == WLTX_EXT_HDL_BP2CP)
		tx_vset *= CP_CP_RATIO;

	if (tx_vset <= WLTX_RX_DISC_PWR_OFF_VTH) {
		di->rx_disc_pwr_on = true;
		hwlog_info("[rx_disc_need_power_off] tx_vset=%dmV\n", tx_vset);
		return false;
	}
	if (wltx_get_rx_pmax(di, &rx_pmax))
		return true;

	if (rx_pmax <= WLTX_RX_DISC_PWR_OFF_PTH) {
		di->rx_disc_pwr_on = true;
		hwlog_info("[rx_disc_need_power_off] rx_pmax=0x%x\n", rx_pmax);
		return false;
	}

	return true;
}

static void wltx_rx_disconnect_handler(struct wltx_dev_info *di)
{
	wltx_reset_exp_tx_cap_id();
	di->ping_timeout = WL_TX_PING_TIMEOUT_2;
	if (wltx_rx_disconnect_need_power_off(di)) {
		/* power off to reset tx,in case tx runs err, ocp */
		wltx_disable_pwr_supply();
		usleep_range(9500, 10500); /* wait 10ms for tx power down */
		wireless_tx_set_stage(WL_TX_STAGE_POWER_SUPPLY);
	} else {
		wireless_tx_set_stage(WL_TX_STAGE_PING_RX);
	}
	schedule_work(&di->wireless_tx_check_work);
}

static void wltx_stop_charging(void)
{
	wltx_stop_monitor_protection();
	wltx_stop_monitor_chrg_curve();
	wltx_stop_monitor_fod();
	(void)wltx_ic_set_stop_config(WLTRX_IC_MAIN);
}

static void wireless_tx_fault_event_handler(struct wltx_dev_info *di)
{
	wireless_tx_wake_lock();
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	hwlog_info("[%s] tx_status = 0x%02x\n", __func__, tx_status);
	wltx_stop_charging();

	switch (tx_status) {
	case WL_TX_STATUS_RX_DISCONNECT:
		wltx_rx_disconnect_handler(di);
		break;
	case WL_TX_STATUS_TX_CLOSE:
	case WL_TX_STATUS_SOC_ERROR:
	case WL_TX_STATUS_TBATT_HIGH:
	case WL_TX_STATUS_TBATT_LOW:
	case WL_TX_STATUS_CHARGE_DONE:
	case WL_TX_STATUS_IN_WL_CHARGING:
		wireless_tx_set_tx_open_flag(false);
		wireless_tx_enable_tx_mode(false);
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_TX_STOP, NULL);
		wltx_disable_pwr_supply();
		wireless_tx_wake_unlock();
		break;
	default:
		wireless_tx_wake_unlock();
		hwlog_err("%s: has no this tx_status %d\n", __func__, tx_status);
		break;
	}

	if (di->tx_pd_flag) {
		di->tx_pd_flag = false;
		if (tx_open_flag &&
			(tx_status != WL_TX_STATUS_RX_DISCONNECT)) {
			wireless_tx_set_stage(WL_TX_STAGE_POWER_SUPPLY);
			schedule_work(&di->wireless_tx_check_work);
		}
	}
}

void wltx_cancle_work_handler(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return;

	hwlog_info("[cancle_work_handler] start\n");
	wireless_tx_limit_iout(di);
	di->cancle_work_flag = true;
	di->stop_reverse_charge = true;
	cancel_work_sync(&di->wireless_tx_check_work);
	cancel_delayed_work_sync(&di->wireless_tx_monitor_work);
	cancel_delayed_work(&di->wltx_abnormal_power_check_work);
	di->cancle_work_flag = false;
	wireless_tx_enable_tx_mode(false);
	wltx_disable_pwr_supply();
	hwlog_info("[cancle_work_handler] end\n");
}

void wltx_restart_work_handler(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return;

	hwlog_info("[restart_work_handler] start\n");
	wireless_tx_limit_iout(di);
	wltx_reset_exp_tx_cap_id();
	wireless_tx_set_stage(WL_TX_STAGE_DEFAULT);
	schedule_work(&di->wireless_tx_check_work);
	hwlog_info("[restart_work_handler] end\n");
}

static void wireless_tx_iout_control(struct wltx_dev_info *di)
{
	int ret;
	u16 tx_iin = 0;
	u16 tx_fop = 0;

	ret = wltx_ic_get_iin(WLTRX_IC_MAIN, &tx_iin);
	ret += wltx_ic_get_fop(WLTRX_IC_MAIN, &tx_fop);
	if (ret) {
		di->i2c_err_cnt++;
		hwlog_err("%s: get tx vin/iin fail", __func__);
	}
	if ((di->standard_rx == false) && (tx_iin <= WL_TX_IIN_LOW) &&
		(wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF)) {
		if (++di->tx_iin_low_cnt >= WL_TX_IIN_LOW_CNT / di->monitor_interval) {
			di->tx_iin_low_cnt = WL_TX_IIN_LOW_CNT;
			wireless_tx_set_tx_status(WL_TX_STATUS_CHARGE_DONE);
			di->stop_reverse_charge = true;
			hwlog_info("[%s] tx_iin below for %ds, set tx_status to charge_done\n",
				__func__, WL_TX_IIN_LOW_CNT / WL_MSEC_PER_SEC);
		}
	} else {
		di->tx_iin_low_cnt = 0;
	}
	wltx_calc_tx_fop_avg(di, tx_fop);
	wireless_tx_calc_tx_iin_avg(di, tx_iin);
}

static int wltx_update_vset_by_soc(struct wltx_dev_info *di)
{
	int soc = bci_show_capacity();

	if (soc <= WLTX_TX_VSET_SOC_TH1)
		return WIRELESS_ADAPTER_5V;
	if (soc >= WLTX_TX_VSET_SOC_TH2)
		return di->tx_vset.v_dflt;
	return di->tx_vset.para[di->tx_vset.cur].vset;
}

static int wltx_update_vset_by_tbat(struct wltx_dev_info *di)
{
	int tbatt = 0;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	if (tbatt >= di->tx_vset_tbat_high)
		return WIRELESS_ADAPTER_5V;
	if (tbatt <= di->tx_vset_tbat_low)
		return di->tx_vset.v_dflt;
	return di->tx_vset.para[di->tx_vset.cur].vset;
}

static int wltx_update_vset_by_fop(struct wltx_dev_info *di)
{
	if ((di->tx_fop_avg >= WLTX_TX_VSET_FOP_TH2) &&
		(di->tx_iin_avg <= WLTX_TX_VSET_IIN_TH1))
		return WIRELESS_ADAPTER_5V;

	if ((di->tx_fop_avg <= WLTX_TX_VSET_FOP_TH1) &&
		(di->tx_iin_avg >= WLTX_TX_VSET_IIN_TH2))
		return di->tx_vset.v_dflt;

	return di->tx_vset.para[di->tx_vset.cur].vset;
}

static void wltx_ps_tx_vset_check(struct wltx_dev_info *di)
{
	int tx_vset_by_fop;
	int tx_vset_by_soc;
	int tx_vset_by_tbat;
	int tx_vset = di->tx_vset.max_vset;
	u16 rx_pmax = 0;

	if (di->tx_cap.cur_id == WLTX_HIGH_PWR_CAP)
		return;

	if (!time_after(jiffies, di->irq_vset_time_out))
		return;

	tx_vset_by_fop = wltx_update_vset_by_fop(di);
	if (tx_vset > tx_vset_by_fop)
		tx_vset = tx_vset_by_fop;

	tx_vset_by_soc = wltx_update_vset_by_soc(di);
	if (tx_vset > tx_vset_by_soc)
		tx_vset = tx_vset_by_soc;

	tx_vset_by_tbat = wltx_update_vset_by_tbat(di);
	if (tx_vset > tx_vset_by_tbat)
		tx_vset = tx_vset_by_tbat;

	if ((di->tx_rp_timeout_lim_volt > 0) &&
		(tx_vset > di->tx_rp_timeout_lim_volt))
		tx_vset = di->tx_rp_timeout_lim_volt;

	(void)wltx_get_rx_pmax(di, &rx_pmax);
	if (rx_pmax < WLTX_FULL_BRIDGE_RX_PWR_TH)
		tx_vset = WIRELESS_ADAPTER_5V;

	wltx_set_tx_volt(WL_TX_STAGE_REGULATION, tx_vset, false);
}

static int wltx_get_full_bridge_ith(void)
{
	u16 ith = 0;

	if (wltx_ic_get_full_bridge_ith(WLTRX_IC_MAIN, &ith))
		return WLTX_FULL_BRIDGE_ITH;

	return ith;
}

static void wltx_ps_tx_low_vout_check(struct wltx_dev_info *di)
{
	int ret;
	u16 tx_iin = 0;
	int tx_vout;
	int ith;
	int tbat = 0;
	u16 rx_pmax = 0;

	ret = wltx_ic_get_iin(WLTRX_IC_MAIN, &tx_iin);
	if (ret)
		return;

	ith = wltx_get_full_bridge_ith();
	(void)wltx_get_rx_pmax(di, &rx_pmax);
	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	if ((tx_iin > ith) && (tbat <= di->tx_vset_tbat_low) &&
		(rx_pmax >= WLTX_FULL_BRIDGE_RX_PWR_TH)) {
		tx_vout = di->tx_vset.v_dflt;
		wltx_set_tx_volt(WL_TX_STAGE_REGULATION, tx_vout, false);
	}
	if ((tx_iin < WLTX_HALF_BRIDGE_ITH) &&
		(di->tx_iin_avg < WLTX_HALF_BRIDGE_ITH)) {
		tx_vout = WIRELESS_ADAPTER_5V;
		wltx_set_tx_volt(WL_TX_STAGE_REGULATION, tx_vout, false);
	}
	if (tbat > di->tx_vset_tbat_high) {
		tx_vout = WIRELESS_ADAPTER_5V;
		wltx_set_tx_volt(WL_TX_STAGE_REGULATION, tx_vout, false);
	}
}

static void wltx_ps_tx_vout_check(struct wltx_dev_info *di)
{
	if (di->tx_vout.v_ask <= WIRELESS_ADAPTER_5V)
		return wltx_ps_tx_low_vout_check(di);

	wltx_ps_tx_high_vout_check();
}

static void wltx_ps_tx_volt_check(struct wltx_dev_info *di)
{
	if (!time_after(jiffies, di->hs_time_out))
		return;

	if (di->pwr_type != WL_TX_PWR_5VBST_VBUS_OTG_CP)
		return wltx_ps_tx_vset_check(di);

	return wltx_ps_tx_vout_check(di);
}

static void wltx_check_expect_power(struct wltx_dev_info *di)
{
	int soc;
	int tbatt = 0;
	int tbatt_max = WL_TX_HI_PWR_TBATT_MAX;

	if (di->tx_cap.cur_id != WLTX_HIGH_PWR_CAP)
		return;
	if (di->tx_vout.v_ask < WLTX_RX_HIGH_VOUT)
		return;

	soc = bci_show_capacity();
	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	if (di->hp_time_out && time_after(jiffies, di->hp_time_out)) {
		hwlog_err("%s: high pwr timeout\n", __func__);
		goto sw_pwr;
	}
	if (soc <= (di->tx_high_pwr_soc - WL_TX_HI_PWR_SOC_BACK)) {
		hwlog_err("%s: soc=%d low\n", __func__, soc);
		goto sw_pwr;
	}
	if (di->tx_vset.para[di->tx_vset.cur].ext_hdl == WLTX_EXT_HDL_BP2CP)
		tbatt_max = WL_TX_HI_PWR_BP2CP_TBATT_MAX;
	if (tbatt >= tbatt_max) {
		hwlog_err("%s: tbatt=%d high\n", __func__, tbatt);
		goto sw_pwr;
	}
	if (di->tx_rp_timeout_lim_volt > 0) {
		hwlog_err("%s: rpp timeout\n", __func__);
		goto sw_pwr;
	}

	return;

sw_pwr:
	wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, false);
	wltx_disable_pwr_supply();
	usleep_range(9500, 10500); /* wait 10ms for tx power down */
	di->stop_reverse_charge = true;
	di->tx_pd_flag = true;
	wltx_set_exp_cap_id(WLTX_DFLT_CAP);
}

void wltx_open_tx(enum wltx_open_type type, bool enable)
{
	if (!g_wltx_di || (type < WLTX_OPEN_TYPE_BEGIN) ||
		(type >= WLTX_OPEN_TYPE_END))
		return;

	if (tx_open_flag && enable) {
		hwlog_info("[%s] tx mode has already open, ignore\n", __func__);
		return;
	}

	hwlog_info("[%s] type:%d\n", __func__, type);
	g_wltx_di->logic_ops = g_wltx_logic_ops[type];
	wireless_tx_set_tx_open_flag(enable);

	if (tx_open_flag) {
		wltx_reset_exp_tx_cap_id();
		g_wltx_di->tx_open_type = type;
		g_wltx_di->ping_timeout = WL_TX_PING_TIMEOUT_1;
		wireless_tx_set_stage(WL_TX_STAGE_DEFAULT);
		schedule_work(&g_wltx_di->wireless_tx_check_work);
	} else {
		wltx_disable_pwr_supply();
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_TX_STOP, NULL);
	}
	wireless_tx_set_tx_status(WL_TX_STATUS_DEFAULT);
}

static void wltx_handle_tx_init_evt(struct wltx_dev_info *di)
{
}

static void wltx_handle_tx_ap_on_evt(struct wltx_dev_info *di)
{
	wltx_set_tx_volt(WL_TX_STAGE_PING_RX, di->tx_vset.v_ps, true);
}

static bool wltx_need_show_mon_info(struct wltx_dev_info *di)
{
	static int log_cnt;

	if (wireless_tx_get_tx_status() < WL_TX_STATUS_IN_CHARGING)
		log_cnt = 0;

	if (di->monitor_interval <= 0)
		return false;

	if (++log_cnt >= (di->log_interval / di->monitor_interval)) {
		log_cnt = 0;
		return true;
	}

	return false;
}

static void wltx_show_monitor_info(struct wltx_dev_info *di)
{
	struct wltx_log_data data;

	if (!wltx_need_show_mon_info(di))
		return;

	memset(&data, 0, sizeof(data));
	(void)wltx_ic_get_vrect(WLTRX_IC_MAIN, &data.vrect);
	(void)wltx_ic_get_vin(WLTRX_IC_MAIN, &data.vin);
	(void)wltx_ic_get_iin(WLTRX_IC_MAIN, &data.iin);
	(void)wltx_ic_get_temp(WLTRX_IC_MAIN, &data.temp);
	(void)wltx_ic_get_fop(WLTRX_IC_MAIN, &data.fop);
	(void)wltx_ic_get_cep(WLTRX_IC_MAIN, &data.cep);
	(void)wltx_ic_get_duty(WLTRX_IC_MAIN, &data.duty);
	(void)wltx_ic_get_ptx(WLTRX_IC_MAIN, &data.ptx);
	(void)wltx_ic_get_prx(WLTRX_IC_MAIN, &data.prx);
	(void)wltx_ic_get_ploss(WLTRX_IC_MAIN, &data.ploss);
	(void)wltx_ic_get_ploss_id(WLTRX_IC_MAIN, &data.ploss_id);
	(void)bat_temp_get_temperature(BAT_TEMP_MIXED, &data.tbatt);
	data.iin_avg = di->tx_iin_avg;
	data.fop_avg = di->tx_fop_avg;
	data.soc = bci_show_capacity();
	hwlog_info("[monitor_info] soc=%d tbatt=%d fop=%uk fop_avg=%uk iin=%u "
		"iin_avg=%u vin=%u vrect=%u cep=%d duty=%u ptx=%u prx=%u "
		"ploss=%d ploss_id=%u temp=%d\n",
		data.soc, data.tbatt, data.fop, data.fop_avg, data.iin,
		data.iin_avg, data.vin, data.vrect, data.cep, data.duty,
		data.ptx, data.prx, data.ploss, data.ploss_id, data.temp);
}

static bool wltx_need_stop_reverse_charge(struct wltx_dev_info *di)
{
	if (!tx_open_flag || (di->i2c_err_cnt > WL_TX_I2C_ERR_CNT)) {
		hwlog_err("need_stop: ic fault or client closed\n");
		wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		return true;
	}

	if (di->stop_reverse_charge) {
		hwlog_info("[need_stop] stop reverse charge\n");
		return true;
	}

	return false;
}

static void wltx_abnormal_power_check_work(struct work_struct *work)
{
	struct wltx_dev_info *di = g_wltx_di;

	if (!di) {
		hwlog_err("di null\n");
		return;
	}

	if (wltx_need_stop_reverse_charge(di))
		return;
	wltx_check_abnormal_power();

	schedule_delayed_work(&di->wltx_abnormal_power_check_work,
		msecs_to_jiffies(di->power_check_work_interval));
}

static void wireless_tx_monitor_work(struct work_struct *work)
{
	struct wltx_dev_info *di = g_wltx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (wireless_tx_can_do_reverse_charging() == WL_TX_FAIL)
		goto exit;

	if (wltx_need_stop_reverse_charge(di))
		goto exit;

	wireless_tx_check_rx_disconnect(di);
	wireless_tx_check_in_tx_mode(di);
	wltx_ps_tx_volt_check(di);
	wltx_check_expect_power(di);

	if (wltx_need_stop_reverse_charge(di))
		goto exit;

	wireless_tx_iout_control(di);
	wltx_show_monitor_info(di);
	schedule_delayed_work(&di->wireless_tx_monitor_work,
		msecs_to_jiffies(di->monitor_interval));
	return;

exit:
	hwlog_info("[%s] stop monitor work\n", __func__);
	wireless_tx_set_stage(WL_TX_STAGE_DEFAULT);
	wireless_tx_fault_event_handler(di);
}

static void wireless_tx_start_check_work(struct work_struct *work)
{
	int ret;
	struct wltx_dev_info *di = g_wltx_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	wireless_tx_wake_lock();
	wireless_tx_para_init(di);

	if (wireless_tx_get_stage() == WL_TX_STAGE_DEFAULT) {
		ret = wireless_tx_can_do_reverse_charging();
		if (ret)
			goto FuncEnd;
		wireless_tx_set_stage(WL_TX_STAGE_POWER_SUPPLY);
	}
	if (wireless_tx_get_stage() == WL_TX_STAGE_POWER_SUPPLY) {
		ret = wireless_tx_power_supply(di);
		if (ret)
			goto FuncEnd;
		wireless_tx_set_stage(WL_TX_STAGE_CHIP_INIT);
	}
	if (wireless_tx_get_stage() == WL_TX_STAGE_CHIP_INIT) {
		ret = wireless_tx_chip_init(di);
		if (ret)
			hwlog_err("%s: TX chip init fail, go on\n", __func__);
		wireless_tx_set_stage(WL_TX_STAGE_PING_RX);
	}
	if (wireless_tx_get_stage() == WL_TX_STAGE_PING_RX) {
		ret = wireless_tx_ping_rx(di);
		if (ret)
			goto FuncEnd;
		wireless_tx_set_stage(WL_TX_STAGE_REGULATION);
	}

	g_init_tbatt = 0;
	bat_temp_get_temperature(BAT_TEMP_MIXED, &g_init_tbatt);
	hwlog_info("[%s] start wireless reverse charging\n", __func__);
	wireless_tx_set_tx_status(WL_TX_STATUS_IN_CHARGING);
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_TX_START, NULL);
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	mod_delayed_work(system_wq, &di->wireless_tx_monitor_work, msecs_to_jiffies(0));
	mod_delayed_work(system_wq, &di->wltx_abnormal_power_check_work,
		msecs_to_jiffies(di->power_check_work_delay));
	return;

FuncEnd:
	coul_drv_charger_event_rcv(WIRELESS_TX_STATUS_CHANGED);
	if (wireless_tx_get_stage() == WL_TX_STAGE_DEFAULT)
		wireless_tx_set_tx_open_flag(false);
	if (wireless_tx_get_stage() >= WL_TX_STAGE_PING_RX)
		wireless_tx_enable_tx_mode(false);
	if (!di->cancle_work_flag &&
		(wireless_tx_get_stage() == WL_TX_STAGE_PING_RX))
		wireless_tx_set_tx_open_flag(false);
	if (wireless_tx_get_stage() >= WL_TX_STAGE_POWER_SUPPLY)
		wltx_disable_pwr_supply();

	wireless_tx_set_stage(WL_TX_STAGE_DEFAULT);
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_TX_STOP, NULL);
	wireless_tx_wake_unlock();
}

static void wireless_tx_handle_ping_event(void)
{
	struct wltx_dev_info *di = g_wltx_di;
	static int abnormal_ping_cnt;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (tx_open_flag) {
		abnormal_ping_cnt = 0;
		return;
	}
	if (++abnormal_ping_cnt < 30) /* about 15s */
		return;

	wireless_tx_enable_tx_mode(false);
	wltx_disable_all_pwr();
	wireless_tx_wake_unlock();
}

static void wltx_handle_irq_set_vtx_evt(struct wltx_dev_info *di)
{
	u16 rx_pmax = 0;

	/* during WLTX_IRQ_VSET_TIME, tx vset is invariable */
	di->irq_vset_time_out = jiffies + msecs_to_jiffies(WLTX_IRQ_VSET_TIME);

	if (!time_after(jiffies, di->hs_time_out))
		return;

	(void)wltx_get_rx_pmax(di, &rx_pmax);
	if (rx_pmax < WLTX_FULL_BRIDGE_RX_PWR_TH) {
		hwlog_info("%s: rx pmax=%u,set volt 5v\n", __func__, rx_pmax);
		wltx_set_tx_volt(WL_TX_STAGE_REGULATION, WIRELESS_ADAPTER_5V, true);
		return;
	}

	wltx_set_tx_volt(WL_TX_STAGE_REGULATION, di->tx_event_data, true);
}

static void wltx_handle_fault_event(struct wltx_dev_info *di)
{
	switch (di->tx_event_type) {
	case POWER_NE_WLTX_RP_DM_TIMEOUT:
		di->tx_rp_timeout_lim_volt = WIRELESS_ADAPTER_5V;
		break;
	case POWER_NE_WLTX_IRQ_SET_VTX:
		wltx_handle_irq_set_vtx_evt(di);
		break;
	default:
		break;
	}
}

static void wltx_handle_rx_data(struct wltx_dev_info *di)
{
	switch (di->tx_event_type) {
	case POWER_NE_WLTX_GET_RX_PRODUCT_TYPE:
		di->rx_data.product_type = (u8)di->tx_event_data;
		hwlog_info("get_rx_type=%u\n", di->rx_data.product_type);
		break;
	case POWER_NE_WLTX_GET_RX_MAX_POWER:
		di->rx_data.pmax = di->tx_event_data;
		hwlog_info("get_rx_pmax=%u\n", di->rx_data.pmax);
		break;
	default:
		break;
	}
}

static void wltx_handle_rx_start_chrg_evt(struct wltx_dev_info *di)
{
	if (di->tx_cap.cur_id != WLTX_HIGH_PWR2_CAP)
		return;

	hwlog_info("handle rx_start_chrg_evt\n");
	wltx_start_monitor_chrg_curve(&di->cc_cfg, 0);
	wltx_start_monitor_protection(&di->protect_cfg, 0);
	wltx_start_monitor_fod(&di->fod_cfg, 0);
}

static void wltx_handle_ask_rx_event(struct wltx_dev_info *di)
{
	switch (di->tx_event_data) {
	case RX_STATR_CHARGING:
		wltx_handle_rx_start_chrg_evt(di);
		break;
	default:
		break;
	}
}

static void wltx_handle_ask_tx_cap(struct wltx_dev_info *di)
{
	wltx_send_tx_cap();

	if (di->tx_cap.cur_id != WLTX_HIGH_PWR2_CAP)
		return;
	wltx_start_monitor_chrg_curve(&di->cc_cfg, WLTX_CC_MON_DELAY);
	wltx_start_monitor_protection(&di->protect_cfg, WLTX_PROTECT_MON_DELAY);
	wltx_start_monitor_fod(&di->fod_cfg, WLTX_FOD_MON_DELAY);
}

static void wltx_handle_ask_set_vtx(struct wltx_dev_info *di)
{
	di->tx_vout.v_ask = di->tx_event_data;
	hwlog_info("[ask_set_vtx] vset:%dmV\n", di->tx_vout.v_ask);
	if (di->tx_cap.cur_id == WLTX_HIGH_PWR_CAP) {
		if (di->tx_vout.v_ask < WLTX_RX_HIGH_VOUT)
			return;
		di->hp_time_out = jiffies + msecs_to_jiffies(WLTX_HI_PWR_TIME);
	}

	wltx_set_tx_volt(WL_TX_STAGE_REGULATION, di->tx_vout.v_ask, true);
}

static void wireless_tx_event_work(struct work_struct *work)
{
	struct wltx_dev_info *di = g_wltx_di;

	if (!di)
		return;

	switch (di->tx_event_type) {
	case POWER_NE_WLTX_GET_CFG:
		/* get configure packet, ping succ */
		wireless_tx_set_tx_status(WL_TX_STATUS_PING_SUCC);
		di->hs_time_out = jiffies + msecs_to_jiffies(di->hs_timeout_offset);
		wltx_set_tx_volt(WL_TX_STAGE_REGULATION, di->tx_vset.v_hs, true);
		break;
	case POWER_NE_WLTX_HANDSHAKE_SUCC:
		/* 0x8866 handshake, security authentic succ */
		di->standard_rx = true;
		break;
	case POWER_NE_WLTX_CHARGEDONE:
		if ((wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF) ||
			di->is_keyboard_online) {
			wireless_tx_set_tx_status(WL_TX_STATUS_CHARGE_DONE);
			di->stop_reverse_charge = true;
		}
		break;
	case POWER_NE_WLTX_CEP_TIMEOUT:
		wireless_tx_set_tx_status(WL_TX_STATUS_RX_DISCONNECT);
		di->stop_reverse_charge = true;
		(void)wltx_ic_mode_enable(WLTRX_IC_MAIN, false);
		break;
	case POWER_NE_WLTX_EPT_CMD:
		wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_OVP:
		wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_OCP:
		wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		di->stop_reverse_charge = true;
		break;
	case POWER_NE_WLTX_PING_RX:
		wireless_tx_handle_ping_event();
		break;
	case POWER_NE_WLTX_ASK_SET_VTX:
		wltx_handle_ask_set_vtx(di);
		break;
	case POWER_NE_WLTX_GET_TX_CAP:
		wltx_handle_ask_tx_cap(di);
		break;
	case POWER_NE_WLTX_RCV_DPING:
		wltx_reset_reverse_charging();
		break;
	case POWER_NE_WLTX_HALL_APPROACH:
		hwlog_info("[%s] POWER_NE_WLTX_HALL_APPROACH\n", __func__);
		di->is_keyboard_online = true;
		wltx_open_tx(WLTX_OPEN_BY_HALL, true);
		break;
	case POWER_NE_WLTX_HALL_AWAY_FROM:
		hwlog_info("[%s] POWER_NE_WLTX_HALL_AWAY_FROM\n", __func__);
		di->tx_open_type = 0;
		di->is_keyboard_online = false;
		wltx_acc_set_dev_state(WLTX_ACC_DEV_STATE_OFFLINE);
		wltx_acc_get_dev_info_and_notify();
		break;
	case POWER_NE_WLTX_ACC_DEV_CONNECTED:
		hwlog_info("[%s] POWER_NE_WLTX_ACC_DEV_CONNECTED\n", __func__);
		wltx_acc_get_dev_info_and_notify();
		break;
	case POWER_NE_WLTX_TX_FOD:
		wltx_reset_reverse_charging();
		break;
	case POWER_NE_WLTX_TX_INIT:
		wltx_handle_tx_init_evt(di);
		break;
	case POWER_NE_WLTX_TX_AP_ON:
		wltx_handle_tx_ap_on_evt(di);
		break;
	case POWER_NE_WLTX_RP_DM_TIMEOUT:
	case POWER_NE_WLTX_IRQ_SET_VTX:
		wltx_handle_fault_event(di);
		break;
	case POWER_NE_WLTX_GET_RX_PRODUCT_TYPE:
	case POWER_NE_WLTX_GET_RX_MAX_POWER:
		wltx_handle_rx_data(di);
		break;
	case POWER_NE_WLTX_ASK_RX_EVT:
		wltx_handle_ask_rx_event(di);
		break;
	default:
		break;
	}
}

static int wireless_tx_event_notifier_call(struct notifier_block *tx_event_nb,
	unsigned long event, void *data)
{
	struct wltx_dev_info *di =
		container_of(tx_event_nb, struct wltx_dev_info, tx_event_nb);
	u16 *tx_notify_data = NULL;

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
	case POWER_NE_WLTX_RCV_DPING:
	case POWER_NE_WLTX_ASK_SET_VTX:
	case POWER_NE_WLTX_GET_TX_CAP:
	case POWER_NE_WLTX_TX_FOD:
	case POWER_NE_WLTX_RP_DM_TIMEOUT:
	case POWER_NE_WLTX_TX_INIT:
	case POWER_NE_WLTX_TX_AP_ON:
	case POWER_NE_WLTX_IRQ_SET_VTX:
	case POWER_NE_WLTX_GET_RX_PRODUCT_TYPE:
	case POWER_NE_WLTX_GET_RX_MAX_POWER:
		break;
	default:
		return NOTIFY_OK;
	}

	di->tx_event_data = 0;
	if (data) {
		tx_notify_data = (u16 *)data;
		di->tx_event_data = *tx_notify_data;
	}
	di->tx_event_type = event;
	schedule_work(&di->wireless_tx_evt_work);
	return NOTIFY_OK;
}

static void wltx_open_val_check(long val)
{
	bool status = false;
	enum wltx_open_type type;

	if ((val < 0) || (val >= (WLTX_OPEN_TYPE_END * WL_TX_OPEN_STATUS))) {
		hwlog_info("%s: input val is out of range\n", __func__);
		return;
	}

	type = val / WL_TX_OPEN_STATUS;
	status = (bool)(val % WL_TX_OPEN_STATUS);

	wltx_open_tx(type, status);
}

static int wltx_parse_tx_vset_para(struct device_node *np, struct wltx_dev_info *di)
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
			(u8)tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_TX_PLOSS_CNT];
		di->tx_vset.para[i].ext_hdl =
			(u8)tmp_para[WLTX_TX_VSET_TOTAL * i + WLTX_TX_EXT_HDL];

		if (di->tx_vset.max_vset < di->tx_vset.para[i].vset)
			di->tx_vset.max_vset = di->tx_vset.para[i].vset;
		hwlog_info("[%s][%d] rx_min:%dmV rx_max:%dmV vset:%dmV\t"
			"lth:%dmV hth:%dmV pl_th:%dmW pl_cnt:%d ext_hdl:%d\n",
			__func__, i, di->tx_vset.para[i].rx_vmin,
			di->tx_vset.para[i].rx_vmax, di->tx_vset.para[i].vset,
			di->tx_vset.para[i].lth, di->tx_vset.para[i].hth,
			di->tx_vset.para[i].pl_th, di->tx_vset.para[i].pl_cnt,
			di->tx_vset.para[i].ext_hdl);
	}

	return 0;
}

static void wltx_set_default_tx_vset_para(struct wltx_dev_info *di)
{
	di->tx_vset.total = 1; /* only one level */
	di->tx_vset.max_vset = 5000; /* mV */
	di->tx_vset.para[0].rx_vmin = 4400; /* mV */
	di->tx_vset.para[0].rx_vmax = 5900; /* mV */
	di->tx_vset.para[0].vset = 5000; /* mV */
	di->tx_vset.para[0].lth = 4500; /* mV */
	di->tx_vset.para[0].hth = 5800; /* mV */
}

static void wltx_parse_tx_stage_vset_para(struct device_node *np, struct wltx_dev_info *di)
{
	int ret;
	int array_len;
	u32 temp_arr[WLTX_TX_STAGE_VTOTAL] = { 0 };

	array_len = of_property_count_u32_elems(np, "tx_stage_vset");
	if (array_len != WLTX_TX_STAGE_VTOTAL)
		goto parse_err;

	ret = of_property_read_u32_array(np, "tx_stage_vset",
		temp_arr, array_len);
	if (ret)
		goto parse_err;

	di->tx_vset.v_ps = (int)temp_arr[WLTX_TX_STAGE_VPS];
	di->tx_vset.v_ping = (int)temp_arr[WLTX_TX_STAGE_VPING];
	di->tx_vset.v_hs = (int)temp_arr[WLTX_TX_STAGE_VHS];
	di->tx_vset.v_dflt = (int)temp_arr[WLTX_TX_STAGE_VDFLT];

	goto print_para;

parse_err:
	di->tx_vset.v_ps = 5000; /* mV */
	di->tx_vset.v_ping = 5000; /* mV */
	di->tx_vset.v_hs = 5000; /* mV */
	di->tx_vset.v_dflt = 5000; /* mV */

print_para:
	hwlog_info("[%s] pwr_supply:%d ping:%d handshake:%d default:%d\n",
		__func__, di->tx_vset.v_ps, di->tx_vset.v_ping,
		di->tx_vset.v_hs, di->tx_vset.v_dflt);
}

static void wltx_parse_tx_cap_para(struct device_node *np, struct wltx_dev_info *di)
{
	int ret;
	int i;
	int arr_len;
	u32 temp[WLTX_TOTAL_CAP * WLC_TX_CAP_TOTAL] = { 0 };

	arr_len = of_property_count_u32_elems(np, "tx_cap");
	if ((arr_len <= 0) || (arr_len % WLC_TX_CAP_TOTAL)) {
		hwlog_err("%s: para invaild\n", __func__);
		goto parse_err;
	}
	if (arr_len > WLTX_TOTAL_CAP * WLC_TX_CAP_TOTAL) {
		hwlog_err("%s: para too long\n", __func__);
		goto parse_err;
	}
	ret = of_property_read_u32_array(np, "tx_cap", temp, arr_len);
	if (ret) {
		hwlog_err("%s: get para fail\n", __func__);
		goto parse_err;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_cap_mode", (u32 *)&di->tx_cap.cap_mode, WLTX_CAP_LOW_PWR);
	di->tx_cap.cap_level = arr_len / WLC_TX_CAP_TOTAL;
	for (i = 0; i < di->tx_cap.cap_level; i++) {
		di->tx_cap.cap_para[i].type =
			(u8)temp[WLC_TX_CAP_TYPE + WLC_TX_CAP_TOTAL * i];
		di->tx_cap.cap_para[i].vout =
			(int)temp[WLC_TX_CAP_VMAX + WLC_TX_CAP_TOTAL * i];
		di->tx_cap.cap_para[i].iout =
			(int)temp[WLC_TX_CAP_IMAX + WLC_TX_CAP_TOTAL * i];
		di->tx_cap.cap_para[i].attr =
			(u8)temp[WLC_TX_CAP_ATTR + WLC_TX_CAP_TOTAL * i];
	}

	goto print_para;

parse_err:
	di->tx_cap.cap_level = 1;
	di->tx_cap.cap_mode = WLTX_CAP_LOW_PWR;
	di->tx_cap.cap_para[0].vout = 5000; /* 5V */
	di->tx_cap.cap_para[0].iout = 500; /* 500mA */
	di->tx_cap.cap_para[0].attr = 0; /* no attr */
print_para:
	hwlog_info("cap_mode=0x%x\n", di->tx_cap.cap_mode);
	for (i = 0; i < di->tx_cap.cap_level; i++)
		hwlog_info("[%s][%d] type:%d vmax:%d imax:%d attr:0x%x\n",
			__func__, i, di->tx_cap.cap_para[i].type,
			di->tx_cap.cap_para[i].vout,
			di->tx_cap.cap_para[i].iout,
			di->tx_cap.cap_para[i].attr);
}

static void wltx_parse_time_alarm_para(struct device_node *np, struct wltx_dev_info *di)
{
	int i;
	int len;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tx_time_alarm", (int *)di->cc_cfg.time_alarm,
		WLTX_TIME_ALARM_ROW, WLTX_TIME_ALARM_COL);
	if (len <= 0) {
		di->cc_cfg.time_alarm_level = 0;
		return;
	}

	di->cc_cfg.time_alarm_level = len / WLTX_TIME_ALARM_COL;
	for (i = 0; i < di->cc_cfg.time_alarm_level; i++)
		hwlog_info("time_alarm[%d]: %ums 0x%x %u %u %u\n",
			i, di->cc_cfg.time_alarm[i].time_th,
			di->cc_cfg.time_alarm[i].alarm.src_state,
			di->cc_cfg.time_alarm[i].alarm.plim,
			di->cc_cfg.time_alarm[i].alarm.vlim,
			di->cc_cfg.time_alarm[i].alarm.reserved);
}

static void wltx_parse_tbatt_alarm_para(struct device_node *np, struct wltx_dev_info *di)
{
	int i;
	int len;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tx_tbatt_alarm", (int *)di->protect_cfg.tbatt_alarm,
		WLTX_TBATT_ALARM_ROW, WLTX_TBATT_ALARM_COL);
	if (len <= 0) {
		di->protect_cfg.tbatt_alarm_level = 0;
		return;
	}

	di->protect_cfg.tbatt_alarm_level = len / WLTX_TBATT_ALARM_COL;
	for (i = 0; i < di->protect_cfg.tbatt_alarm_level; i++)
		hwlog_info("tbatt_alarm[%d]: tbatt[%d %d %d] 0x%x %u %u %u\n",
			i, di->protect_cfg.tbatt_alarm[i].tbatt_lth,
			di->protect_cfg.tbatt_alarm[i].tbatt_hth,
			di->protect_cfg.tbatt_alarm[i].tbatt_back,
			di->protect_cfg.tbatt_alarm[i].alarm.src_state,
			di->protect_cfg.tbatt_alarm[i].alarm.plim,
			di->protect_cfg.tbatt_alarm[i].alarm.vlim,
			di->protect_cfg.tbatt_alarm[i].alarm.reserved);
}

static void wltx_parse_fod_alarm_para(struct device_node *np, struct wltx_dev_info *di)
{
	int i;
	int len;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_alarm", (int *)di->fod_cfg.fod_alarm,
		WLTX_FOD_ALARM_ROW, WLTX_FOD_ALARM_COL);
	if (len <= 0) {
		di->fod_cfg.fod_alarm_level = 0;
		return;
	}

	di->fod_cfg.fod_alarm_level = len / WLTX_FOD_ALARM_COL;
	for (i = 0; i < di->fod_cfg.fod_alarm_level; i++)
		hwlog_info("fod_alarm[%d]: fod[%d %d %d %d] 0x%x %u %u %u\n",
			i, di->fod_cfg.fod_alarm[i].ploss_id,
			di->fod_cfg.fod_alarm[i].ploss_lth,
			di->fod_cfg.fod_alarm[i].ploss_hth,
			di->fod_cfg.fod_alarm[i].delay,
			di->fod_cfg.fod_alarm[i].alarm.src_state,
			di->fod_cfg.fod_alarm[i].alarm.plim,
			di->fod_cfg.fod_alarm[i].alarm.vlim,
			di->fod_cfg.fod_alarm[i].alarm.reserved);
}

static void wltx_parse_tx_alarm_para(struct wltx_dev_info *di)
{
	struct device_node *np = wltx_ic_get_dev_node(WLTRX_IC_MAIN);

	if (!di || !np)
		return;

	wltx_parse_time_alarm_para(np, di);
	wltx_parse_tbatt_alarm_para(np, di);
	wltx_parse_fod_alarm_para(np, di);
}

static void wltx_parse_high_vctrl_para(struct wltx_dev_info *di)
{
	int i;
	int len;
	struct device_node *np = wltx_ic_get_dev_node(WLTRX_IC_MAIN);

	if (!di || !np)
		return;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tx_high_vctrl", (int *)di->high_vctrl.vctrl,
		WLTX_HIGH_V_PARA_ROW, WLTX_HIGH_V_PARA_COL);
	if (len <= 0) {
		di->high_vctrl.vctrl_level = 0;
		return;
	}

	di->high_vctrl.vctrl_level = len / WLTX_HIGH_V_PARA_COL;
	for (i = 0; i < di->high_vctrl.vctrl_level; i++)
		hwlog_info("high_vctrl[%d]: %d %d %d %d %d %d\n",
			i, di->high_vctrl.vctrl[i].iin_th,
			di->high_vctrl.vctrl[i].fop_th,
			di->high_vctrl.vctrl[i].duty_th,
			di->high_vctrl.vctrl[i].cur_v,
			di->high_vctrl.vctrl[i].target_v,
			di->high_vctrl.vctrl[i].delay);
}

static void wireless_tx_parse_dts(struct device_node *np, struct wltx_dev_info *di)
{
	int ret, i;

	ret = of_property_read_u32_array(np, "tx_iin_limit", tx_iin_limit, WL_TX_CHARGER_TYPE_MAX);
	if (ret)
		hwlog_err("%s: get tx_iin_limit para fail\n", __func__);
	for (i = 0; i < WL_TX_CHARGER_TYPE_MAX; i++)
		hwlog_info("[%s] tx_iin_limit[%d] = %d\n", __func__, i, tx_iin_limit[i]);
	ret = of_property_read_u32(np, "pwr_type", &di->pwr_type);
	if (ret) {
		hwlog_err("%s: get pwr_type failed\n", __func__);
		di->pwr_type = WL_TX_PWR_VBUS_OTG;
	} else if (di->pwr_type >= WL_TX_PWR_END) {
		di->pwr_type = WL_TX_PWR_VBUS_OTG;
		hwlog_err("%s: pwr_type %d >= %d, set to %s type\n",
			__func__, di->pwr_type, WL_TX_PWR_END,
			wltx_get_pwr_type_name(di->pwr_type));
	}
	hwlog_info("[%s] pwr_type = %s\n", __func__,
		wltx_get_pwr_type_name(di->pwr_type));

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_vset_tbat_high", &di->tx_vset_tbat_high,
		WLTX_TX_VSET_TBAT_TH1);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_vset_tbat_low", &di->tx_vset_tbat_low,
		WLTX_TX_VSET_TBAT_TH2);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_high_pwr_soc", &di->tx_high_pwr_soc,
		WL_TX_HI_PWR_SOC_MIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_dc_done_buck_ilim", &di->tx_dc_done_buck_ilim,
		WLTX_DC_DONE_BUCK_ILIM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_high_pwr_ilim", &di->tx_high_pwr_ilim,
		WLTX_HI_PWR_ILIM);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ps_need_ext_pwr", &di->ps_need_ext_pwr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hs_timeout_offset", &di->hs_timeout_offset, WLTX_HI_PWR_HS_TIME);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dev_type", (u32 *)&di->dev_type, WIRELESS_DEVICE_UNKNOWN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_bp2cp_abnormal_ibat", &di->tx_bp2cp_abnormal_ibat,
		WLTX_BP2CP_ABNORMAL_IBAT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"power_check_work_interval", &di->power_check_work_interval,
		WL_TX_MONITOR_INTERVAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"power_check_work_delay", &di->power_check_work_delay, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"need_reduce_power", &di->need_reduce_power, 0);

	ret = wltx_parse_tx_vset_para(np, di);
	if (ret) {
		hwlog_err("%s: use default tx_vset para\n", __func__);
		wltx_set_default_tx_vset_para(di);
	}

	wltx_parse_tx_stage_vset_para(np, di);
	wltx_parse_tx_cap_para(np, di);
	wltx_parse_tx_alarm_para(di);
	wltx_parse_high_vctrl_para(di);
}

#ifdef CONFIG_SYSFS
static ssize_t wireless_tx_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t wireless_tx_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wireless_tx_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(wireless_tx, 0644, WL_TX_SYSFS_TX_OPEN, tx_open),
	power_sysfs_attr_ro(wireless_tx, 0444, WL_TX_SYSFS_TX_STATUS, tx_status),
	power_sysfs_attr_ro(wireless_tx, 0444, WL_TX_SYSFS_TX_IIN_AVG, tx_iin_avg),
	power_sysfs_attr_rw(wireless_tx, 0644, WL_TX_SYSFS_DPING_FREQ, dping_freq),
	power_sysfs_attr_rw(wireless_tx, 0644, WL_TX_SYSFS_DPING_INTERVAL, dping_interval),
	power_sysfs_attr_rw(wireless_tx, 0644, WL_TX_SYSFS_MAX_FOP, max_fop),
	power_sysfs_attr_rw(wireless_tx, 0644, WL_TX_SYSFS_MIN_FOP, min_fop),
	power_sysfs_attr_ro(wireless_tx, 0444, WL_TX_SYSFS_TX_FOP, tx_fop),
	power_sysfs_attr_ro(wireless_tx, 0444, WL_TX_SYSFS_HANDSHAKE, tx_handshake),
	power_sysfs_attr_rw(wireless_tx, 0644, WL_TX_SYSFS_LOG_INTERVAL, log_interval),
};

static struct attribute *wireless_tx_sysfs_attrs[ARRAY_SIZE(wireless_tx_sysfs_field_tbl) + 1];
static const struct attribute_group wireless_tx_sysfs_attr_group = {
	.attrs = wireless_tx_sysfs_attrs,
};

static void wireless_tx_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(wireless_tx_sysfs_attrs,
		wireless_tx_sysfs_field_tbl, ARRAY_SIZE(wireless_tx_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "wireless_tx",
		dev, &wireless_tx_sysfs_attr_group);
}

static void wireless_tx_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "wireless_tx",
		dev, &wireless_tx_sysfs_attr_group);
}
#else
static inline void wireless_tx_sysfs_create_group(struct device *dev)
{
}

static inline void wireless_tx_sysfs_remove_group(struct device *dev)
{
}
#endif

static ssize_t wireless_tx_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wltx_dev_info *di = g_wltx_di;
	u16 dping_freq = 0;
	u16 dping_interval = 0;
	u16 max_fop = 0;
	u16 min_fop = 0;
	u16 tx_fop = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wireless_tx_sysfs_field_tbl, ARRAY_SIZE(wireless_tx_sysfs_field_tbl));
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WL_TX_SYSFS_TX_OPEN:
		return snprintf(buf, PAGE_SIZE, "%d\n", tx_open_flag);
	case WL_TX_SYSFS_TX_STATUS:
		return snprintf(buf, PAGE_SIZE, "%d\n", tx_status);
	case WL_TX_SYSFS_TX_IIN_AVG:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->tx_iin_avg);
	case WL_TX_SYSFS_DPING_FREQ:
		(void)wltx_ic_get_ping_freq(WLTRX_IC_MAIN, &dping_freq);
		return snprintf(buf, PAGE_SIZE, "%d\n", dping_freq);
	case WL_TX_SYSFS_DPING_INTERVAL:
		(void)wltx_ic_get_ping_interval(WLTRX_IC_MAIN, &dping_interval);
		return snprintf(buf, PAGE_SIZE, "%d\n", dping_interval);
	case WL_TX_SYSFS_MAX_FOP:
		(void)wltx_ic_get_max_fop(WLTRX_IC_MAIN, &max_fop);
		return snprintf(buf, PAGE_SIZE, "%d\n", max_fop);
	case WL_TX_SYSFS_MIN_FOP:
		(void)wltx_ic_get_min_fop(WLTRX_IC_MAIN, &min_fop);
		return snprintf(buf, PAGE_SIZE, "%d\n", min_fop);
	case WL_TX_SYSFS_TX_FOP:
		(void)wltx_ic_get_fop(WLTRX_IC_MAIN, &tx_fop);
		return snprintf(buf, PAGE_SIZE, "%d\n", tx_fop);
	case WL_TX_SYSFS_HANDSHAKE:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			wltx_check_handshake(di));
	case WL_TX_SYSFS_LOG_INTERVAL:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->log_interval);
	default:
		break;
	}
	return 0;
}

static ssize_t wireless_tx_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wltx_dev_info *di = g_wltx_di;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wireless_tx_sysfs_field_tbl, ARRAY_SIZE(wireless_tx_sysfs_field_tbl));
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WL_TX_SYSFS_TX_OPEN:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		wltx_open_val_check(val);
		break;
	case WL_TX_SYSFS_DPING_FREQ:
		if (strict_strtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_info("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, (u16)val);
		break;
	case WL_TX_SYSFS_DPING_INTERVAL:
		if (strict_strtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_info("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		(void)wltx_ic_set_ping_interval(WLTRX_IC_MAIN, (u16)val);
		break;
	case WL_TX_SYSFS_MAX_FOP:
		if (strict_strtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_info("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		(void)wltx_ic_set_max_fop(WLTRX_IC_MAIN, (u16)val);
		break;
	case WL_TX_SYSFS_MIN_FOP:
		if (strict_strtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_info("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		(void)wltx_ic_set_min_fop(WLTRX_IC_MAIN, (u16)val);
		break;
	case WL_TX_SYSFS_LOG_INTERVAL:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < WL_TX_MONITOR_INTERVAL) ||
			(val > WL_TX_MONITOR_INTERVAL_MAX)) {
			hwlog_err("%s: input err\n", __func__);
			return -EINVAL;
		}
		di->log_interval = val;
		hwlog_info("log_interval is set to %dms\n", di->log_interval);
	default:
		break;
	}
	return count;
}

static struct wltx_dev_info *wltx_dev_info_alloc(void)
{
	static struct wltx_dev_info *di;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		hwlog_err("%s: di alloc failed\n", __func__);
	return di;
}

static void wireless_tx_shutdown(struct platform_device *pdev)
{
	struct wltx_dev_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	hwlog_info("[%s]\n", __func__);
	cancel_delayed_work(&di->wireless_tx_monitor_work);
	cancel_delayed_work(&di->wltx_abnormal_power_check_work);
}

static int wireless_tx_remove(struct platform_device *pdev)
{
	struct wltx_dev_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return 0;
	}

	(void)power_event_bnc_unregister(POWER_BNT_WLTX, &di->tx_event_nb);
	wireless_tx_sysfs_remove_group(di->dev);
	wakeup_source_trash(&wireless_tx_wakelock);

	hwlog_info("[%s]\n", __func__);
	return 0;
}

static int wireless_tx_probe(struct platform_device *pdev)
{
	int ret;
	struct wltx_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!wltx_ic_is_ops_registered(WLTRX_IC_MAIN))
		return -ENOTSUPP;

	di = wltx_dev_info_alloc();
	if (!di) {
		hwlog_err("%s:di alloc failed\n", __func__);
		return -ENOMEM;
	}

	g_wltx_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;

	wakeup_source_init(&wireless_tx_wakelock, "wireless_tx_wakelock");
	wireless_tx_parse_dts(np, di);

	INIT_WORK(&di->wireless_tx_check_work, wireless_tx_start_check_work);
	INIT_WORK(&di->wireless_tx_evt_work, wireless_tx_event_work);
	INIT_DELAYED_WORK(&di->wireless_tx_monitor_work, wireless_tx_monitor_work);
	INIT_DELAYED_WORK(&di->wltx_abnormal_power_check_work, wltx_abnormal_power_check_work);

	di->tx_event_nb.notifier_call = wireless_tx_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLTX, &di->tx_event_nb);
	if (ret < 0) {
		hwlog_err("register rx_connect notifier failed\n");
		goto notifier_regist_fail;
	}

	di->log_interval = WL_TX_MONITOR_LOG_INTERVAL;

	wireless_tx_sysfs_create_group(di->dev);

	hwlog_info("wireless_tx probe ok\n");
	return 0;

notifier_regist_fail:
	wakeup_source_trash(&wireless_tx_wakelock);
	kfree(di);
	di = NULL;
	g_wltx_di = NULL;
	np = NULL;
	platform_set_drvdata(pdev, NULL);
	hwlog_info("wireless_tx probe failed\n");
	return ret;
}

static struct of_device_id wireless_tx_match_table[] = {
	{
		.compatible = "huawei,wireless_tx",
		.data = NULL,
	},
	{},
};

static struct platform_driver wireless_tx_driver = {
	.probe = wireless_tx_probe,
	.remove = wireless_tx_remove,
	.shutdown = wireless_tx_shutdown,
	.driver = {
		.name = "huawei,wireless_tx",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wireless_tx_match_table),
	},
};

static int __init wireless_tx_init(void)
{
	hwlog_info("wireless_tx init ok\n");

	return platform_driver_register(&wireless_tx_driver);
}

static void __exit wireless_tx_exit(void)
{
	platform_driver_unregister(&wireless_tx_driver);
}

device_initcall_sync(wireless_tx_init);
module_exit(wireless_tx_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wireless tx module driver");
MODULE_AUTHOR("HUAWEI Inc");
