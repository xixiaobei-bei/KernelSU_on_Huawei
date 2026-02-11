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
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <huawei_platform/power/power_mesg.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/wireless_direct_charger.h>
#include <../charging_core.h>
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger.h>
#endif
#include <huawei_platform/power/wired_channel_switch.h>
#include <linux/hisi/hisi_powerkey_event.h>
#include <linux/mtd/hisi_nve_interface.h>

#include <huawei_platform/power/vbus_channel/vbus_channel.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/wireless_power_supply.h>
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif

#define HWLOG_TAG wireless_charger
HWLOG_REGIST();

static struct wireless_charge_device_ops *g_wireless_ops;
static struct wireless_charge_device_info *g_wireless_di;
static int g_wireless_channel_state = WIRELESS_CHANNEL_OFF;
static int g_wired_channel_state = WIRED_CHANNEL_OFF;
static enum wireless_charge_stage g_wireless_charge_stage = WIRELESS_STAGE_DEFAULT;
static int wireless_normal_charge_flag = 0;
static int wireless_fast_charge_flag = 0;
static int wireless_super_charge_flag = 0;
static int wireless_start_sample_flag = 0;
static struct wakeup_source g_rx_con_wakelock;
static struct mutex g_rx_en_mutex;
static int rx_iout_samples[RX_IOUT_SAMPLE_LEN];
static int g_fop_fixed_flag = 0;
static int g_rx_vrect_restore_cnt = 0;
static int g_rx_vout_err_cnt = 0;
static int g_rx_ocp_cnt = 0;
static int g_rx_ovp_cnt = 0;
static int g_rx_otp_cnt = 0;
static bool g_bst_rst_complete = true;
static bool g_pwr_ct_srv_ready;
static bool g_need_recheck_cert;
static bool g_in_wldc_check;
static u8 *g_tx_fw_version;
static u8 random[WIRELESS_RANDOM_LEN] = {0};
static u8 tx_cipherkey[WIRELESS_TX_KEY_LEN] = {0};
static u8 rx_cipherkey[WIRELESS_RX_KEY_LEN]= {0};

static int wireless_charge_af_cb(unsigned char version, void * data, int len);
static int wireless_charge_af_srv_on_cb(void);

BLOCKING_NOTIFIER_HEAD(rx_event_nh);

static char chrg_stage[WIRELESS_STAGE_TOTAL][WIRELESS_STAGE_STR_LEN] = {
	{"DEFAULT"}, {"CHECK_TX_ID"}, {"CHECK_TX_ABILITY"}, {"CABLE_DETECT"}, {"CERTIFICATION"},
	{"CHECK_FWUPDATE"}, {"CHARGING"}, {"REGULATION"}, {"REGULATION_DC"}
};

static struct wlc_plim_src_info {
	enum wlc_plim_src src_id;
	char src_name[32]; /* 32 byte max */
	bool need_rst; /* rst when para init */
	int tx_vout; /* mV */
	int rx_vout; /* mV */
	int rx_iout; /* mA */
} const g_plim_para[WLC_PLIM_SRC_MAX] = {
	{ WLC_PLIM_SRC_OTG,      "otg",        false, 5000,  5500,  1000 },
	{ WLC_PLIM_SRC_RPP,      "rpp",        true,  12000, 12000, 1300 },
	{ WLC_PLIM_SRC_FAN,      "fan",        true,  9000,  9900,  1250 },
	{ WLC_PLIM_SRC_VOUT_ERR, "vout_err",   true,  9000,  9900,  1250 },
	{ WLC_PLIM_TX_ALARM,     "tx_alarm",   true,  12000, 12000, 1300 },
	{ WLC_PLIM_TX_BST_ERR,   "tx_bst_err", true,  5000,  5500,  1000 },
};

static const struct power_mesg_easy_cbs wc_af_ops[WC_AF_INFO_NL_OPS_NUM] = {
	{
		.cmd = POWER_CMD_WC_ANTIFAKE_HASH,
		.doit = wireless_charge_af_cb,
	}
};
static struct power_mesg_node wc_af_info_node = {
	.target = POWERCT_PORT,
	.name = "WC_AF",
	.ops = wc_af_ops,
	.n_ops = WC_AF_INFO_NL_OPS_NUM,
	.srv_on_cb = wireless_charge_af_srv_on_cb,
};
int wireless_charge_ops_register(struct wireless_charge_device_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_wireless_ops = ops;
	} else {
		hwlog_err("charge ops register fail!\n");
		ret = -EPERM;
	}

	return ret;
}
int register_wireless_charger_vbus_notifier(struct notifier_block *nb)
{
	if(g_wireless_di && nb)
		return blocking_notifier_chain_register(&g_wireless_di->wireless_charge_evt_nh, nb);

	return -EINVAL;
}
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

static void wireless_charge_msleep(int sleep_ms)
{
	int i;
	int interval = 25; /* ms */
	int cnt = sleep_ms / interval;

	for (i = 0; i < cnt; i++) {
		msleep(interval);
		if (g_wired_channel_state == WIRED_CHANNEL_ON)
			return;
		if (!wireless_charge_check_tx_exist())
			return;
	}
}

static void wireless_charge_en_enable(int enable)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	di->ops->rx_enable(enable);
}
static void wireless_charge_sleep_en_enable(int enable)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	di->ops->rx_sleep_enable(enable);
}

int wlc_get_rx_support_mode(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return WLC_RX_SP_BUCK_MODE;
	}

	return di->sysfs_data.rx_support_mode;
}

static void wlc_set_plimit_src(int src_id)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if ((src_id < 0) || (src_id >= WLC_PLIM_SRC_MAX))
		return;

	if (di) {
		if (test_bit(src_id, &di->plimit_src))
			return;
		set_bit(src_id, &di->plimit_src);
		if (src_id != g_plim_para[src_id].src_id)
			return;
		hwlog_info("%s: %s\n", __func__, g_plim_para[src_id].src_name);
	}
}

static void wlc_clear_plimit_src(int src_id)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if ((src_id < 0) || (src_id >= WLC_PLIM_SRC_MAX))
		return;

	if (di) {
		if (!test_bit(src_id, &di->plimit_src))
			return;
		clear_bit(src_id, &di->plimit_src);
		if (src_id != g_plim_para[src_id].src_id)
			return;
		hwlog_info("%s: %s\n", __func__, g_plim_para[src_id].src_name);
	}
}

static void wlc_reset_plimit(struct wireless_charge_device_info *di)
{
	int i;

	for (i = 0; i < WLC_PLIM_SRC_MAX; i++) {
		if (g_plim_para[i].need_rst)
			clear_bit(i, &di->plimit_src);
	}
	di->tx_evt_plim = 0;
}

int wireless_charge_get_wireless_channel_state(void)
{
	return g_wireless_channel_state;
}
void wireless_charge_set_wireless_channel_state(int state)
{
	hwlog_info("%s %d\n", __func__, state);
	g_wireless_channel_state = state;
}
static void wireless_charge_set_wired_channel_state(int state)
{
	hwlog_info("[%s] %d\n", __func__, state);
	g_wired_channel_state = state;
}
int wireless_charge_get_wired_channel_state(void)
{
	return g_wired_channel_state;
}
int wireless_charge_get_fast_charge_flag(void)
{
	return wireless_fast_charge_flag;
}

int wlc_get_super_charge_flag(void)
{
	return wireless_super_charge_flag;
}

static void wireless_charge_send_charge_uevent(struct wireless_charge_device_info *di, int icon_type)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging, return\n", __func__);
		return;
	}
	wireless_normal_charge_flag = 0;
	wireless_fast_charge_flag = 0;
	wireless_super_charge_flag = 0;
	switch (icon_type) {
		case WIRELESS_NORMAL_CHARGE_FLAG:
			wireless_normal_charge_flag = 1;
			break;
		case WIRELESS_FAST_CHARGE_FLAG:
			wireless_fast_charge_flag = 1;
			break;
		case WIRELESS_SUPER_CHARGE_FLAG:
			wireless_super_charge_flag = 1;
			break;
		default:
			hwlog_err("%s: unknown icon_type\n", __func__);
	}

	hwlog_info("[%s] cur type=%d, last type=%d\n",
		__func__, icon_type, di->curr_icon_type);
	if (di->curr_icon_type ^ icon_type)
		wireless_charge_connect_send_uevent();

	di->curr_icon_type = icon_type;
}
int wireless_charge_get_rx_iout_limit(void)
{
	int iin_set = RX_IOUT_MIN;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return iin_set;
	}
	iin_set = min(di->rx_iout_max, di->rx_iout_limit);
	if (di->sysfs_data.rx_iout_max > 0)
		iin_set = min(iin_set, di->sysfs_data.rx_iout_max);
	return iin_set;
}
bool wireless_charge_check_tx_exist(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return false;
	}

	return di->ops->check_tx_exist();
}

void wlc_rx_ext_pwr_ctrl_init(int flag)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (di && di->ops && di->ops->ext_pwr_ctrl_init) {
		ret = di->ops->ext_pwr_ctrl_init(flag);
		if (ret)
			hwlog_err("%s: fail\n", __func__);
	}
}

void wlc_send_charge_mode(u8 mode)
{
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (g_wireless_channel_state == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return;
	}
	if (di && di->ops && di->ops->send_chrg_mode) {
		ret = di->ops->send_chrg_mode(mode);
		if (ret)
			hwlog_err("%s: fail\n", __func__);
	}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */
}

static int wireless_charge_send_ept
	(struct wireless_charge_device_info *di, enum wireless_etp_type type)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	return di->ops->send_ept(type);
}

static void wlc_rx_chip_reset(struct wireless_charge_device_info *di)
{
	if (di->wlc_err_rst_cnt >= WLC_RST_CNT_MAX)
		return;

	(void)di->ops->chip_reset();
	di->wlc_err_rst_cnt++;
	di->discon_delay_time = WL_RST_DISCONN_DELAY_MS;
}

static void wireless_charge_set_input_current(struct wireless_charge_device_info *di)
{
	int iin_set = wireless_charge_get_rx_iout_limit();
	charge_set_input_current(iin_set);
}
static int wireless_charge_get_tx_id(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	return di->ops->get_tx_id();
}
static int wireless_charge_fix_tx_fop(int fop)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	if (di && di->ops && di->ops->fix_tx_fop) {
		return di->ops->fix_tx_fop(fop);
	}

	return 0;
}
static int wireless_charge_unfix_tx_fop(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	if (di && di->ops && di->ops->unfix_tx_fop) {
		return di->ops->unfix_tx_fop();
	}

	return 0;
}

static void wireless_charge_kick_watchdog(
	struct wireless_charge_device_info *di)
{
	int ret;

	if (g_wireless_channel_state == WIRELESS_CHANNEL_OFF)
		return;

	ret = di->ops->kick_watchdog();
	if (ret)
		hwlog_err("%s: fail\n", __func__);
}

static int wireless_charge_set_tx_vout(int tx_vout)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	if (tx_vout > TX_DEFAULT_VOUT &&
		g_wired_channel_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired vbus connect, tx_vout should be set to %dmV at most\n",
			__func__, TX_DEFAULT_VOUT);
		return -1;
	}
	if (di->pwroff_reset_flag && tx_vout > TX_DEFAULT_VOUT) {
		hwlog_err("%s: pwroff_reset_flag = %d, tx_vout should be set to %dmV at most\n",
			__func__, di->pwroff_reset_flag, TX_DEFAULT_VOUT);
		return -1;
	}
	hwlog_info("[%s] tx_vout is set to %dmV\n", __func__, tx_vout);
	return di->ops->set_tx_vout(tx_vout);
}

int wireless_charge_set_rx_vout(int rx_vout)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -1;
	}
	if (di->pwroff_reset_flag && rx_vout > RX_DEFAULT_VOUT) {
		hwlog_err("%s: pwroff_reset_flag = %d, rx_vout should be set to %dmV at most\n",
			__func__, di->pwroff_reset_flag, RX_DEFAULT_VOUT);
		return -1;
	}
	hwlog_info("%s: rx_vout is set to %dmV\n", __func__, rx_vout);
	return di->ops->set_rx_vout(rx_vout);
}
int wireless_charge_get_rx_vout(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->ops->get_rx_vout();
}
static int wireless_charge_get_rx_vout_reg(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->ops->get_rx_vout_reg();
}

int wireless_charge_get_tx_vout_reg(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->ops->get_tx_vout_reg();
}

int wireless_charge_get_rx_vrect(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->ops->get_rx_vrect();
}
int wireless_charge_get_rx_iout(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->ops->get_rx_iout();
}

static int wireless_charge_get_rx_fop(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->ops->get_rx_fop();
}

static u8 *wlc_get_die_id(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->get_die_id)
		return "no die_id";

	return di->ops->get_die_id();
}

struct tx_capability *wlc_get_tx_cap(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di)
		return NULL;

	return di->tx_cap;
}

int wlc_get_rx_max_iout(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->get_rx_max_iout)
		return RX_DFT_IOUT_MAX;

	return di->ops->get_rx_max_iout();
}

int wlc_get_tx_evt_plim(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di)
		return (int)(RX_DEFAULT_VOUT * RX_DEFAULT_IOUT);

	return di->tx_evt_plim;
}

static int wlc_get_rx_temp(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->get_rx_temp)
		return -1;

	return di->ops->get_rx_temp();
}

int wireless_charge_get_rx_avg_iout(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->iout_avg;
}

int wlc_get_pmode_id_by_mode_name(const char *mode_name)
{
	int i;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !mode_name) {
		hwlog_err("%s: di null\n", __func__);
		return 0;
	}
	for (i = 0; i < di->mode_data.total_mode; i++) {
		if (!strncmp(mode_name, di->mode_data.mode_para[i].mode_name,
			strlen(di->mode_data.mode_para[i].mode_name)))
			return i;
	}

	return 0;
}

static void wlc_cp_chip_init(void)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->cp_chip_init)
		return;

	ret = di->ops->cp_chip_init();
	if (ret)
		hwlog_err("%s: fail\n", __func__);
}

int wlc_set_bp_mode(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->set_bp_mode)
		return -WLC_ERR_PARA_NULL;

	return di->ops->set_bp_mode();
}

int wlc_set_cp_mode(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->set_cp_mode)
		return -WLC_ERR_PARA_NULL;

	return di->ops->set_cp_mode();
}

bool wlc_is_bp_open(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->is_bp_open)
		return true;

	return di->ops->is_bp_open();
}

bool wlc_is_cp_open(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->is_cp_open)
		return false;

	return di->ops->is_cp_open();
}

static int wlc_get_cp_ratio(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->get_cp_ratio ||
		!wlc_is_cp_open())
		return WLC_BP_RATIO;

	return di->ops->get_cp_ratio();
}

static void wireless_charge_count_avg_iout(
	struct wireless_charge_device_info *di)
{
	int cnt_max;

	if (di->monitor_interval <= 0)
		return;

	cnt_max = RX_AVG_IOUT_TIME / di->monitor_interval;

	if (g_bst_rst_complete && (di->iout_avg < RX_LOW_IOUT)) {
		di->iout_high_cnt = 0;
		di->iout_low_cnt++;
		if (di->iout_low_cnt >= cnt_max)
			di->iout_low_cnt = cnt_max;
		return;
	}

	if (di->iout_avg > RX_HIGH_IOUT) {
		di->iout_low_cnt = 0;
		di->iout_high_cnt++;
		if (di->iout_high_cnt >= cnt_max)
			di->iout_high_cnt = cnt_max;
		return;
	}
}

static void wireless_charge_calc_avg_iout(
	struct wireless_charge_device_info *di)
{
	int i;
	static int index = 0;
	int iout_sum = 0;

	rx_iout_samples[index] = wireless_charge_get_rx_iout();
	index = (index+1) % RX_IOUT_SAMPLE_LEN;
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++) {
		iout_sum += rx_iout_samples[i];
	}
	di->iout_avg = iout_sum/RX_IOUT_SAMPLE_LEN;
}

static void wireless_charge_reset_avg_iout(struct wireless_charge_device_info *di)
{
	int i;
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++) {
		rx_iout_samples[i] = di->rx_iout_min;
	}
	di->iout_avg = di->rx_iout_min;
}
static void wireless_charge_set_charge_stage(enum wireless_charge_stage charge_stage)
{
	if (charge_stage < WIRELESS_STAGE_TOTAL &&
		g_wireless_charge_stage != charge_stage) {
		g_wireless_charge_stage = charge_stage;
		hwlog_info("[%s] set charge stage to %s\n", __func__, chrg_stage[charge_stage]);
	}
}

static int  wireless_charge_check_fast_charge_succ(
	struct wireless_charge_device_info *di)
{
	if ((wireless_fast_charge_flag || wireless_super_charge_flag) &&
		g_wireless_charge_stage >= WIRELESS_STAGE_CHARGING)
		return WIRELESS_CHRG_SUCC;
	else
		return WIRELESS_CHRG_FAIL;
}

static int  wireless_charge_check_normal_charge_succ(struct wireless_charge_device_info *di)
{
	if (WIRELESS_TYPE_ERR != di->tx_cap->type && !wireless_fast_charge_flag &&
		g_wireless_charge_stage >= WIRELESS_STAGE_CHARGING)
		return WIRELESS_CHRG_SUCC;
	else
		return WIRELESS_CHRG_FAIL;
}

static int wlc_formal_check_direct_charge(const char *m_name)
{
	int ret;

	ret = wldc_formal_check(m_name);
	if (!ret)
		wireless_charge_set_charge_stage(WIRELESS_STAGE_REGULATION_DC);

	return ret;
}

static bool wlc_pmode_final_judge_crit(struct wireless_charge_device_info *di,
	int pmode_index)
{
	int tbatt = hisi_battery_temperature();
	struct wireless_mode_para *mode_para =
		&di->mode_data.mode_para[pmode_index];

	if ((di->tx_vout_max < mode_para->ctrl_para.tx_vout) ||
		(di->rx_vout_max < mode_para->ctrl_para.rx_vout))
		goto judge_fail;
	if ((mode_para->tbatt >= 0) && (tbatt >= mode_para->tbatt))
		goto judge_fail;
	if ((pmode_index == di->curr_pmode_index) &&
		(g_wireless_charge_stage != WIRELESS_STAGE_CHARGING)) {
		if ((mode_para->max_time > 0) &&
			time_after(jiffies, di->curr_power_time_out))
			goto judge_fail;
	}

	return true;

judge_fail:
	hwlog_info("%s: fail. index=%d, tx_vout=%d, rx_vout=%d, tbatt=%d\n",
		__func__, pmode_index, di->tx_vout_max, di->rx_vout_max, tbatt);
	return false;
}

static bool wlc_pmode_normal_judge_crit(struct wireless_charge_device_info *di,
	int pmode_index)
{
	struct wireless_mode_para *mode_para =
		&di->mode_data.mode_para[pmode_index];

	if ((mode_para->expect_cable_detect >= 0) &&
		(di->cable_detect_succ_flag != mode_para->expect_cable_detect))
		goto judge_fail;
	if ((mode_para->expect_cert >= 0) &&
		(di->cert_succ_flag != mode_para->expect_cert))
		goto judge_fail;

	return true;

judge_fail:
	hwlog_info("%s: fail. index=%d\n", __func__, pmode_index);
	return false;
}

static bool wlc_pmode_quick_judge_crit(struct wireless_charge_device_info *di,
	int pmode_index, int crit_type)
{
	struct wireless_mode_para *mode_para =
		&di->mode_data.mode_para[pmode_index];

	if ((di->tx_cap->vout_max < mode_para->tx_vout_min) ||
		(di->product_para.tx_vout < mode_para->ctrl_para.tx_vout) ||
		(di->product_para.rx_vout < mode_para->ctrl_para.rx_vout) ||
		(di->product_para.rx_iout < mode_para->ctrl_para.rx_iout))
		goto judge_fail;

	if (di->tx_cap->vout_max * di->tx_cap->iout_max <
		mode_para->tx_vout_min * mode_para->tx_iout_min)
		goto judge_fail;

	return true;

judge_fail:
	hwlog_info("%s: fail. index=%d, vout=%d, iout=%d, tx_vout=%d, rx_vout=%d, rx_iout=%d\n",
		__func__, pmode_index, di->tx_cap->vout_max, di->tx_cap->iout_max,
		di->product_para.tx_vout, di->product_para.rx_vout, di->product_para.rx_iout);
	return false;
}

static bool wlc_pmode_dc_judge_crit(struct wireless_charge_device_info *di,
	int pmode_index)
{
	struct wireless_mode_para *mode_para =
		&di->mode_data.mode_para[pmode_index];

	hwlog_info("%s: start direct charge check, index=%d\n",
		__func__, pmode_index);
	if (wldc_prev_check(mode_para->mode_name)) {
		hwlog_info("wldc pre check fail. index=%d\n", pmode_index);
		return false;
	}
	if ((g_wireless_charge_stage == WIRELESS_STAGE_REGULATION_DC) ||
		g_in_wldc_check)
		return true;
	g_in_wldc_check = true;
	if (!wlc_formal_check_direct_charge(mode_para->mode_name)) {
		di->curr_pmode_index = pmode_index;
	} else {
		g_in_wldc_check = false;
		hwlog_info("formal check fail. index=%d\n", pmode_index);
		return false;
	}
	g_in_wldc_check = false;

	hwlog_info("%s: direct charge check succ\n", __func__);
	return true;
}

bool wireless_charge_mode_judge_criterion(int pmode_index, int crit_type)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return false;
	}
	if ((pmode_index < 0) || (pmode_index >= di->mode_data.total_mode))
		return false;

	switch (crit_type) {
	case WLDC_MODE_FINAL_JUDGE_CRIT:
	case WIRELESS_MODE_FINAL_JUDGE_CRIT:
		if (!wlc_pmode_final_judge_crit(di, pmode_index))
			return false;
	case WIRELESS_MODE_NORMAL_JUDGE_CRIT:
		if (!wlc_pmode_normal_judge_crit(di, pmode_index))
			return false;
	case WIRELESS_MODE_QUICK_JUDGE_CRIT:
		if (!wlc_pmode_quick_judge_crit(di, pmode_index, crit_type))
			return false;
		break;
	default:
		hwlog_err("%s: crit_type = %d error\n", __func__, crit_type);
		return false;
	}

	if (di->curr_pmode_index != pmode_index)
		hwlog_info("%s: succ. local index=%d, global index=%d\n",
			__func__, pmode_index, di->curr_pmode_index);

	if ((crit_type == WIRELESS_MODE_FINAL_JUDGE_CRIT) &&
		strstr(di->mode_data.mode_para[pmode_index].mode_name, "SC")) {
		if (!wlc_pmode_dc_judge_crit(di, pmode_index))
			return false;
	}

	return true;
}

static int  wireless_charge_check_fac_test_succ(
	struct wireless_charge_device_info *di)
{
	if (di->tx_cap->type == di->standard_tx_adaptor) {
		if (wireless_charge_mode_judge_criterion(1,
			WIRELESS_MODE_QUICK_JUDGE_CRIT))
			return wireless_charge_check_fast_charge_succ(di);
		else
			return  wireless_charge_check_normal_charge_succ(di);
	}
	return WIRELESS_CHRG_FAIL;
}

static void wireless_charge_dsm_dump(struct wireless_charge_device_info *di, char* dsm_buff)
{
	int i, soc, vrect, vout, iout, tbatt;
	char buff[ERR_NO_STRING_SIZE] = {0};
	soc = hisi_battery_capacity();
	tbatt = hisi_battery_temperature();
	vrect = wireless_charge_get_rx_vrect();
	vout = wireless_charge_get_rx_vout();
	iout = wireless_charge_get_rx_iout();
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

static u8 wlc_rename_tx_type(struct wireless_charge_device_info *di)
{
	u8 tx_type = di->tx_cap->type;

	if ((tx_type >= WLC_FAC_TX_TYPE_BASE) &&
		(tx_type <= WLC_FAC_TX_TYPE_MAX))
		tx_type %= WLC_FAC_TX_TYPE_BASE;
	else if ((tx_type >= WLC_CAR_TX_TYPE_BASE) &&
		(tx_type <= WLC_CAR_TX_TYPE_MAX))
		tx_type %= WLC_CAR_TX_TYPE_BASE;

	return tx_type;
}

static void wireless_charge_dsm_report(struct wireless_charge_device_info *di,
	int err_no, char *dsm_buff)
{
	if (wlc_rename_tx_type(di) == WIRELESS_QC) {
		hwlog_info("[%s] ignore err_no:%d, tx_type:%d\n", __func__,
			err_no, di->tx_cap->type);
		return;
	}
	msleep(di->monitor_interval);
	if (g_wireless_channel_state == WIRELESS_CHANNEL_ON) {
		wireless_charge_dsm_dump(di, dsm_buff);
		power_dsm_dmd_report(POWER_DSM_BATTERY, err_no, dsm_buff);
	}
}

static void wireless_charge_get_tx_capability(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return ;
	}
	di->tx_cap = di->ops->get_tx_capability();
}

static void wireless_charge_send_fod_status(
	struct wireless_charge_device_info *di)
{
	int ret;

	if (di->fod_status <= 0) {
		hwlog_debug("%s: fod_status invalid\n", __func__);
		return;
	}
	if (!di || !di->ops || !di->ops->send_fod_status) {
		hwlog_debug("%s: di null\n", __func__);
		return;
	}
	if (!di->tx_cap->support_fod_status) {
		hwlog_err("%s: tx not support fod_status detect\n", __func__);
		return;
	}

	ret = di->ops->send_fod_status(di->fod_status);
	if (!ret) {
		hwlog_info("[%s] succ\n", __func__);
		return;
	}

	hwlog_err("%s: fail\n", __func__);
}

static void wireless_charge_get_tx_prop(struct wireless_charge_device_info *di)
{
	int i;
	u8 tx_type;
	struct wireless_tx_prop_para *tx_prop = NULL;

	if (di->tx_prop_data.total_prop_type <= 0) {
		hwlog_err("%s: total_prop_type is %d\n",
			__func__, di->tx_prop_data.total_prop_type);
		return;
	}

	tx_type = wlc_rename_tx_type(di);
	for (i = 0; i < di->tx_prop_data.total_prop_type; i++) {
		if (tx_type == di->tx_prop_data.tx_prop[i].tx_type) {
			di->curr_tx_type_index = i;
			break;
		}
	}

	if (i == di->tx_prop_data.total_prop_type)
		di->curr_tx_type_index = 0;

	tx_prop = &di->tx_prop_data.tx_prop[di->curr_tx_type_index];
	if (!di->tx_cap->vout_max)
		di->tx_cap->vout_max = tx_prop->tx_default_vout;

	if (!di->tx_cap->iout_max)
		di->tx_cap->iout_max = tx_prop->tx_default_iout;
}

static char *wireless_charge_read_nvm_info(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->read_nvm_info) {
		hwlog_err("%s: di null\n", __func__);
		return "error";
	}

	return di->ops->read_nvm_info(di->sysfs_data.nvm_sec_no);
}

static void wlc_reset_icon_pmode(struct wireless_charge_device_info *di)
{
	int i;

	for (i = 0; i < di->mode_data.total_mode; i++)
		set_bit(i, &di->icon_pmode);
	hwlog_info("[%s] icon_pmode=0x%x", __func__, di->icon_pmode);
}

void wlc_clear_icon_pmode(int pmode)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di)
		return;
	if ((pmode < 0) || (pmode >= di->mode_data.total_mode))
		return;

	if (test_bit(pmode, &di->icon_pmode)) {
		clear_bit(pmode, &di->icon_pmode);
		hwlog_info("[%s] icon_pmode=0x%x", __func__, di->icon_pmode);
	}
}

void wireless_charge_icon_display(int crit_type)
{
	int pmode;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	for (pmode = di->mode_data.total_mode - 1; pmode >= 0; pmode--) {
		if (test_bit(pmode, &di->icon_pmode) &&
			wireless_charge_mode_judge_criterion(pmode, crit_type))
			break;
	}
	if (pmode < 0) {
		pmode = 0;
		hwlog_err("%s: no power mode matched, set icon mode to %s\n",
			__func__, di->mode_data.mode_para[pmode].mode_name);
	}

	wireless_charge_send_charge_uevent(di,
		di->mode_data.mode_para[pmode].icon_type);
}

void wlc_ignore_vbus_only_event(bool ignore_flag)
{
	int bst5v_status;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->bst5v_ignore_vbus_only)
		return;

#ifdef CONFIG_TCPC_CLASS
	bst5v_status = boost_5v_status();
	if (ignore_flag)
		pd_dpm_ignore_vbus_only_event(true);
	else if (!(bst5v_status & BIT(BOOST_CTRL_WLC))
		&& !(bst5v_status & BIT(BOOST_CTRL_WLDC)))
		pd_dpm_ignore_vbus_only_event(false);
#endif
}

static void wlc_extra_power_supply(bool enable)
{
	int ret;
	static bool boost_5v_flag;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->hvc_need_5vbst)
	    return;

	if (enable && di->supported_rx_vout > RX_HIGH_VOUT) {
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

static void wlc_get_supported_max_rx_vout(
		struct wireless_charge_device_info *di)
{
	int pmode_index;

	if (!di)
		return;

	pmode_index = di->mode_data.total_mode - 1;
	for (; pmode_index >= 0; pmode_index--) {
		if (wireless_charge_mode_judge_criterion(pmode_index,
			WIRELESS_MODE_QUICK_JUDGE_CRIT))
			break;
	}
	if (pmode_index < 0)
		pmode_index = 0;

	di->supported_rx_vout =
		di->mode_data.mode_para[pmode_index].ctrl_para.rx_vout;
}

static u8 *wlc_get_tx_fw_version(struct wireless_charge_device_info *di)
{
	if (g_tx_fw_version)
		return g_tx_fw_version;

	return di->ops->get_tx_fw_version();
}

static void wireless_charge_get_tx_info(struct wireless_charge_device_info *di)
{
#ifndef WIRELESS_CHARGER_FACTORY_VERSION
	if (!di->standard_tx) {
		hwlog_err("%s: not standard tx, don't get tx info\n", __func__);
		return;
	}
	g_tx_fw_version = wlc_get_tx_fw_version(di);
	hwlog_info("[%s] tx_fw_version = %s\n", __func__, g_tx_fw_version);
#endif
}
static void wireless_charge_set_default_tx_capability(struct wireless_charge_device_info *di)
{
	di->tx_cap->type = WIRELESS_TYPE_ERR;
	di->tx_cap->vout_max = ADAPTER_5V * WL_MVOLT_PER_VOLT;
	di->tx_cap->iout_max = CHARGE_CURRENT_1000_MA;
	di->tx_cap->can_boost = 0;
	di->tx_cap->cable_ok = 0;
	di->tx_cap->no_need_cert = 0;
	di->tx_cap->support_scp = 0;
	di->tx_cap->support_extra_cap = 0;
	/*extra cap*/
	di->tx_cap->support_fan = 0;
	di->tx_cap->support_tec = 0;
	di->tx_cap->support_fod_status = 0;
}
static int wireless_charge_af_calc_hash(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if(power_easy_send(&wc_af_info_node, POWER_CMD_WC_ANTIFAKE_HASH, 0,
                           random, sizeof(random))) {
		hwlog_err("%s: mesg send failed!\n", __func__);
		return CERT_SEND_SRV_MSG_FAIL;
	}
	if (!wait_for_completion_timeout(&di->wc_af_completion, WC_AF_WAIT_CT_TIMEOUT)) {
		/*if time out happend, we asume the powerct serivce is dead*/
		hwlog_err("%s: wait_for_completion_timeout timeout!!!\n", __func__);
		return CERT_RCV_SRV_MSG_FAIL;
	}
	return 0;
}

static int wlc_get_rx_hash(struct wireless_charge_device_info *di)
{
	int i;
	int ret;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	for (i = 0; i < WAIT_AF_SRV_RTY_CNT; i++) {
		ret = wireless_charge_af_calc_hash();
		if (!ret)
			return AF_SRV_SUCC;
	}

	if (i >= WAIT_AF_SRV_RTY_CNT) {
		if (ret != CERT_SEND_SRV_MSG_FAIL)
			wireless_charge_dsm_report(di,
				ERROR_WIRELESS_CERTI_SERVICE_FAIL, dsm_buff);
		hwlog_err("%s: power_ct service no response\n", __func__);
		return AF_SRV_NO_RESPONSE;
	}

	return AF_SRV_NOT_READY;
}

static void wlc_send_cert_confirm_msg(
	struct wireless_charge_device_info *di, bool cert_flag)
{
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	int ret;

	ret = di->ops->send_msg_cert_confirm(cert_flag);
	if (ret)
		hwlog_err("%s: fail\n", __func__);
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */
}

static int wlc_tx_certification(struct wireless_charge_device_info *di)
{
	int i;
	int ret;

	if (g_wireless_channel_state == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: not in wireless charging\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}

	random[0] = di->antifake_key_index;
	for (i = 1; i < WIRELESS_RANDOM_LEN; i++)
		get_random_bytes(&random[i], sizeof(u8));

	ret = di->ops->get_tx_cert(random, WIRELESS_RANDOM_LEN,
		tx_cipherkey, WIRELESS_TX_KEY_LEN);
	if (ret) {
		hwlog_err("%s: get hash from tx fail\n", __func__);
		return ret;
	}

	ret = wlc_get_rx_hash(di);
	if ((ret == AF_SRV_NOT_READY) || (ret == AF_SRV_NO_RESPONSE)) {
		di->cert_succ_flag = WIRELESS_CHECK_FAIL;
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_FWUPDATE);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	for (i = 0; (i < WIRELESS_TX_KEY_LEN) &&
		(i < WIRELESS_RX_KEY_LEN); i++) {
		if (rx_cipherkey[i] != tx_cipherkey[i]) {
			hwlog_info("[%s] trx cipherkey mismatch\n", __func__);
			return -WLC_ERR_MISMATCH;
		}
	}

	return 0;
}

static void wireless_charge_set_ctrl_interval(struct wireless_charge_device_info *di)
{
	if (g_wireless_charge_stage < WIRELESS_STAGE_REGULATION) {
		di->ctrl_interval = CONTROL_INTERVAL_NORMAL;
	} else {
		di->ctrl_interval = CONTROL_INTERVAL_FAST;
	}
}
void wireless_charge_chip_init(int tx_vset)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	ret = di->ops->chip_init(tx_vset);
	if(ret < 0) {
		hwlog_err("%s: rx chip init failed\n", __func__);
	}
}

static void wlc_set_iout_min(struct wireless_charge_device_info *di)
{
	di->rx_iout_max = di->rx_iout_min;
	wireless_charge_set_input_current(di);
}

int wireless_charge_select_vout_mode(int vout)
{
	int id = 0;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return id;
	}

	for (id = 0; id < di->volt_mode_data.total_volt_mode; id++) {
		if (vout == di->volt_mode_data.volt_mode[id].tx_vout)
			break;
	}
	if (id >= di->volt_mode_data.total_volt_mode) {
		id = 0;
		hwlog_err("%s: match vmode_index failed\n", __func__);
	}
	return id;
}

void wireless_charge_update_max_vout_and_iout(bool ignore_cnt_flag)
{
	int soc = hisi_battery_capacity();
	int i;
	int mode = VBUS_CH_NOT_IN_OTG_MODE;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	di->tx_vout_max = di->product_para.tx_vout;
	di->rx_vout_max = di->product_para.rx_vout;
	di->rx_iout_max = di->product_para.rx_iout;
	di->tx_vout_max = min(di->tx_vout_max, di->sysfs_data.tx_vout_max);
	di->rx_vout_max = min(di->rx_vout_max, di->sysfs_data.rx_vout_max);
	di->rx_iout_max = min(di->rx_iout_max, di->sysfs_data.rx_iout_max);
	vbus_ch_get_mode(VBUS_CH_USER_WR_TX,
		VBUS_CH_TYPE_BOOST_GPIO, &mode);
	if (mode == VBUS_CH_IN_OTG_MODE || di->pwroff_reset_flag ||
		!di->extra_pwr_good_flag) {
		di->tx_vout_max = min(di->tx_vout_max, TX_DEFAULT_VOUT);
		di->rx_vout_max = min(di->rx_vout_max, RX_DEFAULT_VOUT);
		di->rx_iout_max = min(di->rx_iout_max, RX_DEFAULT_IOUT);
	}
	for (i = 0; i < WLC_PLIM_SRC_MAX; i++) {
		if (!test_bit(i, &di->plimit_src))
			continue;
		di->tx_vout_max = min(di->tx_vout_max, g_plim_para[i].tx_vout);
		di->rx_vout_max = min(di->rx_vout_max, g_plim_para[i].rx_vout);
		di->rx_iout_max = min(di->rx_iout_max, g_plim_para[i].rx_iout);
	}
	/*check volt and curr limit caused by high soc*/
	for(i = 0; i < di->segment_data.segment_para_level; i++) {
		if(soc >= di->segment_data.segment_para[i].soc_min && soc <= di->segment_data.segment_para[i].soc_max) {
			di->tx_vout_max = min(di->tx_vout_max, di->segment_data.segment_para[i].tx_vout_limit);
			di->rx_vout_max = min(di->rx_vout_max, di->segment_data.segment_para[i].rx_vout_limit);
			di->rx_iout_max = min(di->rx_iout_max, di->segment_data.segment_para[i].rx_iout_limit);
			break;
		}
	}
	if (!ignore_cnt_flag && di->iout_low_cnt >= RX_AVG_IOUT_TIME/di->monitor_interval) {
		di->tx_vout_max = min(di->tx_vout_max, TX_DEFAULT_VOUT);
		di->rx_vout_max = min(di->rx_vout_max, RX_DEFAULT_VOUT);
		di->rx_iout_max = min(di->rx_iout_max, RX_DEFAULT_IOUT);
	}
}

static void wlc_notify_charger_vout(struct wireless_charge_device_info *di)
{
	int tx_vout;
	int cp_vout;
	int cp_ratio;

	tx_vout = di->volt_mode_data.volt_mode[di->curr_vmode_index].tx_vout;
	cp_ratio = wlc_get_cp_ratio();
	if (!cp_ratio) {
		hwlog_err("%s: cp_ratio err\n", __func__);
		return;
	}
	hwlog_info("[%s] cp_ratio=%d\n", __func__, cp_ratio);
	cp_vout = tx_vout / cp_ratio;
	blocking_notifier_call_chain(&di->wireless_charge_evt_nh,
		CHARGER_TYPE_WIRELESS, &cp_vout);
}

static void wlc_send_bst_succ_msg(struct wireless_charge_device_info *di)
{
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	if (g_wireless_charge_stage == WIRELESS_STAGE_CHARGING) {
		if (!di->ops->send_msg_rx_boost_succ())
			hwlog_info("[%s] send cmd boost_succ ok\n", __func__);
	}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */
}

static void wlc_report_bst_fail_dmd(struct wireless_charge_device_info *di)
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

static int wireless_charge_boost_vout(struct wireless_charge_device_info *di,
	int cur_vmode_id, int target_vmode_id)
{
	int vmode;
	int ret;
	int tx_vout;

	if (di->boost_err_cnt >= BOOST_ERR_CNT_MAX) {
		hwlog_debug("%s: boost fail exceed %d times\n",
			__func__, BOOST_ERR_CNT_MAX);
		return -WLC_ERR_CHECK_FAIL;
	}

	wlc_set_iout_min(di);
	wireless_charge_msleep(300); /* delay 300ms for ibus stablity */
	g_bst_rst_complete = false;

	for (vmode = cur_vmode_id + 1; vmode <= target_vmode_id; vmode++) {
		tx_vout = di->volt_mode_data.volt_mode[vmode].tx_vout;
		ret = wireless_charge_set_tx_vout(tx_vout);
		if (ret) {
			hwlog_err("%s: boost fail\n", __func__);
			wlc_report_bst_fail_dmd(di);
			g_bst_rst_complete = true;
			return ret;
		}
		di->curr_vmode_index = vmode;
		wlc_notify_charger_vout(di);
		wlc_set_iout_min(di);
		if (vmode != target_vmode_id)
			wireless_charge_msleep(3000); /* for vrect stablity */
	}

	g_bst_rst_complete = true;
	di->boost_err_cnt = 0;

	wlc_send_bst_succ_msg(di);
	return 0;
}

static int wireless_charge_reset_vout(struct wireless_charge_device_info *di,
	int cur_vmode_id, int target_vmode_id)
{
	int ret;
	int vmode;
	int tx_vout;

	wlc_set_iout_min(di);
	wireless_charge_msleep(300); /* delay 300ms for ibus stablity */
	g_bst_rst_complete = false;

	for (vmode = cur_vmode_id - 1; vmode >= target_vmode_id; vmode--) {
		tx_vout = di->volt_mode_data.volt_mode[vmode].tx_vout;
		ret = wireless_charge_set_tx_vout(tx_vout);
		if (ret) {
			hwlog_err("%s: reset fail\n", __func__);
			g_bst_rst_complete = true;
			return ret;
		}
		di->curr_vmode_index = vmode;
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
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	tx_vout = di->volt_mode_data.volt_mode[target_vmode_index].tx_vout;
	if (target_vmode_index > cur_vmode_index)
		ret = wireless_charge_boost_vout(di,
			cur_vmode_index, target_vmode_index);
	else if (target_vmode_index < cur_vmode_index)
		ret = wireless_charge_reset_vout(di,
			cur_vmode_index, target_vmode_index);
	else
		return wireless_charge_set_rx_vout(tx_vout);

	if (g_wired_channel_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired vbus connect\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}
	if (g_wireless_channel_state == WIRELESS_CHANNEL_OFF) {
		hwlog_err("%s: wireless vbus disconnect\n", __func__);
		return -WLC_ERR_STOP_CHRG;
	}

	if (di->curr_vmode_index == cur_vmode_index)
		return ret;

	tx_vout = di->volt_mode_data.volt_mode[di->curr_vmode_index].tx_vout;
	wireless_charge_chip_init(tx_vout);
	wlc_notify_charger_vout(di);

	return ret;
}

int wldc_set_trx_vout(int vout)
{
	int cur_vmode;
	int target_vmode;
	int tx_vout_reg;

	tx_vout_reg = wireless_charge_get_tx_vout_reg();
	cur_vmode = wireless_charge_select_vout_mode(tx_vout_reg);
	target_vmode = wireless_charge_select_vout_mode(vout);

	return wireless_charge_set_vout(cur_vmode, target_vmode);
}

static int wireless_charge_vout_control
		(struct wireless_charge_device_info *di, int pmode_index)
{
	int ret;
	int target_vout;
	int target_vmode_index;
	int tx_vout_reg;

	if (strstr(di->mode_data.mode_para[pmode_index].mode_name, "SC"))
		return 0;
	if (g_wireless_channel_state != WIRELESS_CHANNEL_ON)
		return -1;
	tx_vout_reg = wireless_charge_get_tx_vout_reg();
	if (tx_vout_reg != di->volt_mode_data.volt_mode[di->curr_vmode_index].tx_vout) {
		hwlog_err("%s: tx_vout_reg (%dmV) != cur_mode_vout (%dmV) !!\n", __func__, tx_vout_reg,
				di->volt_mode_data.volt_mode[di->curr_vmode_index].tx_vout);
		ret = wireless_charge_set_tx_vout(di->volt_mode_data.volt_mode[di->curr_vmode_index].tx_vout);
		if (ret) {
			hwlog_err("%s: set tx vout fail\n", __func__);
		}
	}
	target_vout = di->mode_data.mode_para[pmode_index].ctrl_para.tx_vout;
	target_vmode_index = wireless_charge_select_vout_mode(target_vout);
	di->tx_vout_max = min(di->tx_vout_max, di->mode_data.mode_para[pmode_index].ctrl_para.tx_vout);
	di->rx_vout_max = min(di->rx_vout_max, di->mode_data.mode_para[pmode_index].ctrl_para.rx_vout);
	return wireless_charge_set_vout(di->curr_vmode_index, target_vmode_index);
}

static void wlc_update_imax_by_tx_plimit(struct wireless_charge_device_info *di)
{
	int rx_epxt_vout;
	struct wireless_ctrl_para *ctrl_para =
		&di->mode_data.mode_para[di->curr_pmode_index].ctrl_para;

	if (di->tx_evt_plim && ctrl_para) {
		rx_epxt_vout = ctrl_para->rx_vout;
		if ((di->tx_evt_plim < rx_epxt_vout * di->rx_iout_max) &&
			(rx_epxt_vout > 0))
			di->rx_iout_max = di->tx_evt_plim / rx_epxt_vout;
	}
}

static void wireless_charge_iout_control(struct wireless_charge_device_info *di)
{
	static int rx_vrect_low_cnt = 0;
	int charger_iin_regval;
	int vrect, rx_vout_reg, tx_vout_reg;
	int ret;
	int cnt_max = RX_VRECT_LOW_RESTORE_TIME/di->ctrl_interval;
	int soc = hisi_battery_capacity();
	int i;

	if ((g_wireless_charge_stage == WIRELESS_STAGE_REGULATION_DC) ||
		(g_wireless_channel_state == WIRELESS_CHANNEL_OFF))
		return;

	di->rx_iout_max = min(di->rx_iout_max, di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.rx_iout);
	di->rx_iout_max = min(di->rx_iout_max, di->tx_cap->iout_max);

	if ((di->tx_cap->type >= WLC_FAC_TX_TYPE_BASE) &&
		(di->tx_cap->type <= WLC_FAC_TX_TYPE_MAX)) {
		wireless_start_sample_flag = 1;
		if (!delayed_work_pending(&di->rx_sample_work))
			mod_delayed_work(system_wq, &di->rx_sample_work,
				msecs_to_jiffies(0));
	}

	if (wireless_start_sample_flag) {
		di->rx_iout_limit = di->rx_iout_max;
		wireless_charge_set_input_current(di);
		return;
	}
	/*check charge segment para*/
	for (i = 0; i < di->segment_data.segment_para_level; i++) {
		if(soc >= di->segment_data.segment_para[i].soc_min && soc <= di->segment_data.segment_para[i].soc_max) {
			di->rx_iout_max = min(di->segment_data.segment_para[i].rx_iout_limit, di->rx_iout_max);
			break;
		}
	}
	if (di->pwroff_reset_flag)
		return;
	charger_iin_regval = charge_get_charger_iinlim_regval();
	vrect = wireless_charge_get_rx_vrect();
	tx_vout_reg = wireless_charge_get_tx_vout_reg();
	rx_vout_reg = wireless_charge_get_rx_vout_reg();
	if (tx_vout_reg != di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.tx_vout) {
		hwlog_err("%s: tx_vout_reg (%dmV) != tx_vout_set (%dmV) !!\n", __func__, tx_vout_reg,
				di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.tx_vout);
		ret = wireless_charge_set_tx_vout(di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.tx_vout);
		if (ret) {
			hwlog_err("%s: set tx vout fail\n", __func__);
		}
	} else if (rx_vout_reg != di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.rx_vout) {
		hwlog_err("%s: rx_vout_reg (%dmV) != rx_vout_set (%dmV) !!\n", __func__, rx_vout_reg,
				di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.rx_vout);
		ret = wireless_charge_set_rx_vout(di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.rx_vout);
		if (ret) {
			hwlog_err("%s: set rx vout fail\n", __func__);
		}
	} else if (vrect < di->mode_data.mode_para[di->curr_pmode_index].vrect_low_th) {
		if (++rx_vrect_low_cnt >= RX_VRECT_LOW_CNT) {
			rx_vrect_low_cnt = RX_VRECT_LOW_CNT;
			hwlog_err("%s: vrect(%dmV) < vrect_low_th(%dmV), decrease rx_iout %dmA\n",
					__func__, vrect, di->mode_data.mode_para[di->curr_pmode_index].vrect_low_th,
					di->rx_iout_step);
			di->rx_iout_limit = max(charger_iin_regval - di->rx_iout_step, RX_VRECT_LOW_IOUT_MIN);
			wireless_charge_set_input_current(di);
			g_rx_vrect_restore_cnt = cnt_max;
			return;
		}
	} else if (g_rx_vrect_restore_cnt > 0) {
		rx_vrect_low_cnt = 0;
		g_rx_vrect_restore_cnt--;
		return;
	} else {
		rx_vrect_low_cnt = 0;
	}
	for (i = 0; i < di->iout_ctrl_data.ictrl_para_level; i++) {
		if (di->iout_avg >= di->iout_ctrl_data.ictrl_para[i].iout_min &&
			di->iout_avg < di->iout_ctrl_data.ictrl_para[i].iout_max) {
			di->rx_iout_limit = di->iout_ctrl_data.ictrl_para[i].iout_set;
			break;
		}
	}

	wlc_update_imax_by_tx_plimit(di);
	wireless_charge_set_input_current(di);
}

static void wireless_charge_interference_work(struct work_struct *work)
{
	int i;
	int tx_fixed_fop;
	int tx_vout_max;
	int rx_vout_max;
	int rx_iout_max;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	tx_fixed_fop = -1;
	tx_vout_max = di->product_para.tx_vout;
	rx_vout_max = di->product_para.rx_vout;
	rx_iout_max = di->product_para.rx_iout;

	for (i = 0; i < di->interfer_data.total_src; i++) {
		if (!(di->interfer_data.interfer_src_state & BIT(i)))
			continue;
		if (di->interfer_data.interfer_para[i].tx_fixed_fop >= 0)
			tx_fixed_fop =
			    di->interfer_data.interfer_para[i].tx_fixed_fop;
		if (di->interfer_data.interfer_para[i].tx_vout_limit >= 0)
			tx_vout_max = min(tx_vout_max,
			    di->interfer_data.interfer_para[i].tx_vout_limit);
		if (di->interfer_data.interfer_para[i].rx_vout_limit >= 0)
			rx_vout_max = min(rx_vout_max,
			    di->interfer_data.interfer_para[i].rx_vout_limit);
		if (rx_iout_max >= 0)
			rx_iout_max = min(rx_iout_max,
			    di->interfer_data.interfer_para[i].rx_iout_limit);
	}
	di->sysfs_data.tx_fixed_fop = tx_fixed_fop;
	di->sysfs_data.tx_vout_max = tx_vout_max;
	di->sysfs_data.rx_vout_max = rx_vout_max;
	di->sysfs_data.rx_iout_max = rx_iout_max;
	hwlog_info("[%s] fop = %d, tx_rx_vout = %d %d,iout_max = %d\n",
		__func__, di->sysfs_data.tx_fixed_fop,
		di->sysfs_data.tx_vout_max, di->sysfs_data.rx_vout_max,
		di->sysfs_data.rx_iout_max);
}

static void wireless_charger_update_interference_settings
	(struct wireless_charge_device_info *di, u8 interfer_src_state)
{
	int i;

	for (i = 0; i < di->interfer_data.total_src; i++) {
		if (di->interfer_data.interfer_para[i].src_open == interfer_src_state) {
			di->interfer_data.interfer_src_state |= BIT(i);
			break;
		} else if (di->interfer_data.interfer_para[i].src_close == interfer_src_state) {
			di->interfer_data.interfer_src_state &= ~ BIT(i);
			break;
		} else {
			/*do nothing*/
		}
	}
	if (i == di->interfer_data.total_src) {
		hwlog_err("%s: interference settings error\n", __func__);
		return;
	}
	if (!delayed_work_pending(&di->interfer_work)) {
		hwlog_info("[%s] delay %dms to schedule work\n",
			__func__, WIRELESS_INTERFER_TIMEOUT);
		schedule_delayed_work(&di->interfer_work,
			msecs_to_jiffies(WIRELESS_INTERFER_TIMEOUT));
	}
}
static void wireless_charge_update_fop(struct wireless_charge_device_info *di)
{
	int ret;
	if (!di->standard_tx) {
		hwlog_debug("%s: not standard tx, don't update fop\n", __func__);
		return;
	}
	if (di->sysfs_data.tx_fixed_fop > 0 && !g_fop_fixed_flag) {
		ret = wireless_charge_fix_tx_fop(di->sysfs_data.tx_fixed_fop);
		if (ret) {
			hwlog_err("%s: fix tx_fop fail\n", __func__);
			return;
		}
		hwlog_info("[%s] fop fixed to %dkHZ\n", __func__, di->sysfs_data.tx_fixed_fop);
		g_fop_fixed_flag = 1;
	}
	if (di->sysfs_data.tx_fixed_fop <= 0 && g_fop_fixed_flag) {
		ret = wireless_charge_unfix_tx_fop();
		if (ret) {
			hwlog_err("%s: unfix tx_fop fail", __func__);
			return;
		}
		hwlog_info("[%s] fop unfixed succ \n", __func__);
		g_fop_fixed_flag = 0;
	}
}

static void wlc_update_charge_state(struct wireless_charge_device_info *di)
{
	int ret;
	int soc;
	static int retry_cnt;

	if (!di->standard_tx || !wireless_charge_check_tx_exist() ||
		(g_wired_channel_state == WIRED_CHANNEL_ON))
		return;

	if (g_wireless_charge_stage <= WIRELESS_STAGE_CHARGING) {
		retry_cnt = 0;
		return;
	}

	soc = hisi_battery_capacity();
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
		ret = di->ops->send_chrg_state(di->stat_rcd.chrg_state_cur);
		if (ret) {
			hwlog_err("%s: send charge_state fail\n", __func__);
			retry_cnt++;
			return;
		}
		retry_cnt = 0;
		di->stat_rcd.chrg_state_last = di->stat_rcd.chrg_state_cur;
	}
}

static void wlc_check_voltage(struct wireless_charge_device_info *di)
{
	int cnt_max = RX_VOUT_ERR_CHECK_TIME / di->monitor_interval;
	int tx_vout_reg = wireless_charge_get_tx_vout_reg();
	int vout_reg = wireless_charge_get_rx_vout_reg();
	int vout = wireless_charge_get_rx_vout();
	int vbus = charge_get_vbus();

	if ((vout <= 0) || !g_bst_rst_complete ||
		(g_wireless_charge_stage < WIRELESS_STAGE_CHECK_TX_ID))
		return;

	vout = (vout >= vbus) ? vout : vbus;
	if (vout >= vout_reg * di->rx_vout_err_ratio / PERCENT) {
		g_rx_vout_err_cnt = 0;
		return;
	}

	if (di->iout_avg >= RX_EPT_IGNORE_IOUT)
		return;

	hwlog_err("%s: abnormal vout=%dmV", __func__, vout);
	if (++g_rx_vout_err_cnt < cnt_max)
		return;

	g_rx_vout_err_cnt = cnt_max;
	if (tx_vout_reg >= RX_HIGH_VOUT2) {
		wlc_set_plimit_src(WLC_PLIM_SRC_VOUT_ERR);
		hwlog_err("%s: high vout err\n", __func__);
		return;
	}
	hwlog_info("[%s] vout lower than %d*%d%%mV for %dms, send EPT\n",
		__func__, vout_reg, di->rx_vout_err_ratio,
		RX_VOUT_ERR_CHECK_TIME);
	wireless_charge_send_ept(di, WIRELESS_EPT_ERR_VOUT);
}

static bool wlc_is_night_time(struct wireless_charge_device_info *di)
{
	struct timeval tv;
	struct rtc_time tm;

	if (di->sysfs_data.ignore_fan_ctrl)
		return false;

	do_gettimeofday(&tv); /* seconds since 1970-01-01 00:00:00 */
	tv.tv_sec -= sys_tz.tz_minuteswest * 60; /* GMT, 1min = 60s */
	rtc_time_to_tm(tv.tv_sec, &tm);

	/* night time: 21:00-7:00 */
	if ((tm.tm_hour >= 21) || (tm.tm_hour < 7))
		return true;

	return false;
}

static void wlc_fan_control_handle(struct wireless_charge_device_info *di,
	int *retry_cnt, u8 limit_val)
{
	int ret;

	if (*retry_cnt >= WLC_FAN_LIMIT_RETRY_CNT) {
		*retry_cnt = 0;
		di->stat_rcd.fan_last = di->stat_rcd.fan_cur;
		return;
	}
	ret = di->ops->limit_fan_speed(limit_val);
	if (ret) {
		(*retry_cnt)++;
		return;
	}
	*retry_cnt = 0;
	di->stat_rcd.fan_last = di->stat_rcd.fan_cur;
}

static bool wlc_is_need_fan_control(struct wireless_charge_device_info *di)
{
	if (!di->standard_tx || !di->tx_cap->support_fan ||
		!di->ops->limit_fan_speed)
		return false;

	/* in charger mode, time zone is not available */
	if (strstr(saved_command_line, "androidboot.mode=charger") ||
		strstr(saved_command_line, "androidboot.mode=factory"))
		return false;

	return true;
}

static void wlc_update_fan_control(struct wireless_charge_device_info *di,
	bool force_flag)
{
	static int retry_cnt;
	int soc;

	if (!wlc_is_need_fan_control(di))
		return;
	if (!wireless_charge_check_tx_exist() ||
		(g_wired_channel_state == WIRED_CHANNEL_ON))
		return;
	if (!force_flag &&
		(g_wireless_charge_stage <= WIRELESS_STAGE_CHARGING)) {
		retry_cnt = 0;
		return;
	}

	soc = hisi_battery_capacity();
	if (wlc_is_night_time(di)) {
		di->stat_rcd.fan_cur = WLC_LIMIT_FAN_HALF_SPEED;
		wlc_set_plimit_src(WLC_PLIM_SRC_FAN);
	} else if ((soc >= WLC_FAN_CTRL_SOC_MAX) &&
		(di->rx_vout_max < RX_HIGH_VOUT)) {
		di->stat_rcd.fan_cur = WLC_LIMIT_FAN_HALF_SPEED;
	} else if (!wireless_super_charge_flag) {
		di->stat_rcd.fan_cur = WLC_LIMIT_FAN_HALF_SPEED;
	} else {
		di->stat_rcd.fan_cur = WLC_LIMIT_FAN_FULL_SPEED;
		wlc_clear_plimit_src(WLC_PLIM_SRC_FAN);
	}

	if (di->stat_rcd.fan_last != di->stat_rcd.fan_cur) {
		if (di->stat_rcd.fan_cur == WLC_LIMIT_FAN_HALF_SPEED)
			wlc_fan_control_handle(di, &retry_cnt,
				WLC_FAN_HALF_SPEED_MAX);
		else if (di->stat_rcd.fan_cur == WLC_LIMIT_FAN_FULL_SPEED)
			wlc_fan_control_handle(di, &retry_cnt,
				WLC_FAN_FULL_SPEED);
	}
}

static void wireless_charge_update_status(struct wireless_charge_device_info *di)
{
	wireless_charge_update_fop(di);
	wlc_update_charge_state(di);
	wlc_update_fan_control(di, false);
}
static int wireless_charge_set_power_mode(struct wireless_charge_device_info *di, int pmode_index)
{
	int ret;
	if (pmode_index < 0 || pmode_index >= di->mode_data.total_mode)
		return -1;
	ret = wireless_charge_vout_control(di, pmode_index);
	if (!ret) {
		if (pmode_index != di->curr_pmode_index) {
			if (di->mode_data.mode_para[pmode_index].max_time > 0) {
				di->curr_power_time_out = jiffies +
					msecs_to_jiffies(di->mode_data.mode_para[pmode_index].max_time * WL_MSEC_PER_SEC);
			}
			di->curr_pmode_index = pmode_index;
			if (wireless_charge_set_rx_vout(di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.rx_vout))
				hwlog_err("%s: set rx vout fail\n", __func__);
		}
	}
	return ret;
}

static void wireless_charge_switch_power_mode(
	struct wireless_charge_device_info *di, int start_id, int end_id)
{
	int ret;
	int p_id;

	if ((g_wireless_charge_stage != WIRELESS_STAGE_CHARGING) &&
		wireless_start_sample_flag) {
		hwlog_debug("%s: start sample, don't sw pmode\n", __func__);
		return;
	}
	if ((start_id < 0) || (end_id < 0))
		return;

	for (p_id = start_id; p_id >= end_id; p_id--) {
		if (!wireless_charge_mode_judge_criterion(p_id,
			WIRELESS_MODE_FINAL_JUDGE_CRIT))
			continue;
		if (strstr(di->mode_data.mode_para[p_id].mode_name, "SC"))
			return;
		ret = wireless_charge_set_power_mode(di, p_id);
		if (!ret)
			break;
	}
	if (p_id < 0) {
		di->curr_pmode_index = 0;
		wireless_charge_set_power_mode(di, di->curr_pmode_index);
	}
}

static void wireless_charge_power_mode_control(struct wireless_charge_device_info *di)
{
	if (wireless_charge_mode_judge_criterion(di->curr_pmode_index, WIRELESS_MODE_FINAL_JUDGE_CRIT)) {
		if (WIRELESS_STAGE_CHARGING == g_wireless_charge_stage)
			wireless_charge_switch_power_mode(di, di->mode_data.total_mode - 1, 0);
		else
			wireless_charge_switch_power_mode(di,
				di->mode_data.mode_para[di->curr_pmode_index].expect_mode, di->curr_pmode_index + 1);
	} else {
		wireless_charge_switch_power_mode(di, di->curr_pmode_index - 1, 0);
	}
	wireless_charge_iout_control(di);
}
int wireless_charge_get_power_mode(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	return di->curr_pmode_index;
}

int wlc_get_expected_pmode_id(int pmode_id)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return di->mode_data.mode_para[pmode_id].expect_mode;
}

void wlc_set_cur_pmode_id(int pmode_id)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di)
		return;

	if ((pmode_id < 0) || (pmode_id >= di->mode_data.total_mode))
		return;

	di->curr_pmode_index = pmode_id;
}

static void wireless_charge_regulation(struct wireless_charge_device_info *di)
{
	int ret;

	if ((g_wireless_charge_stage != WIRELESS_STAGE_REGULATION) ||
		(g_wireless_channel_state == WIRELESS_CHANNEL_OFF))
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (g_need_recheck_cert && g_pwr_ct_srv_ready) {
		/* vout may be 9v, so here reset 5V for cert stability */
		ret = wldc_set_trx_vout(TX_DEFAULT_VOUT);
		if (ret)
			hwlog_err("%s: set default vout fail\n", __func__);
		wlc_set_iout_min(di);
		wireless_charge_msleep(300); /* delay 300ms for ibus stablity */
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CERTIFICATION);
		return;
	}

	wireless_charge_update_max_vout_and_iout(false);
	wireless_charge_power_mode_control(di);
	hwlog_info("[%s] --\n", __func__);
}

static void read_back_color_from_nv(char *back_color, unsigned int back_len)
{
	int ret;
	unsigned int nv_len;
	struct hisi_nve_info_user nv_user_info;

	if (!back_color) {
		hwlog_err("%s: back_color null\n", __func__);
		return;
	}
	memset(&nv_user_info, 0, sizeof(nv_user_info));
	nv_user_info.nv_operation = NV_READ_TAG;
	nv_user_info.nv_number = BACK_DEVICE_COLOR_NV_NUM;
	nv_user_info.valid_size = BACK_DEVICE_COLOR_LEN;
	strncpy(nv_user_info.nv_name, "DEVCOLR",
		(sizeof(nv_user_info.nv_name) - 1));
	ret = hisi_nve_direct_access(&nv_user_info);
	if (ret) {
		hwlog_err("%s: fail ret = %d\n", __func__, ret);
		return;
	}
	nv_len = sizeof(nv_user_info.nv_data);
	if (nv_len > back_len)
		nv_len = back_len;
	memcpy(back_color, nv_user_info.nv_data, nv_len);
}

static void wlc_check_dev_back_color(void)
{
	int ret;
	static bool check_done;
	static char dev_color[BACK_DEVICE_COLOR_LEN];
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops->check_dev_back_color)
		return;

	if (check_done)
		return;

	memset(dev_color, 0, sizeof(BACK_DEVICE_COLOR_LEN));
	read_back_color_from_nv(dev_color, BACK_DEVICE_COLOR_LEN - 1);
	ret = di->ops->check_dev_back_color(dev_color);
	if (!ret)
		check_done = true;
}

static void wireless_charge_start_charging(
	struct wireless_charge_device_info *di)
{
	const char *cur_mode_name = NULL;
	int cur_mode_tx_vout;

	if ((g_wireless_charge_stage != WIRELESS_STAGE_CHARGING) ||
	    (g_wireless_channel_state == WIRELESS_CHANNEL_OFF))
		return;

	hwlog_info("[%s] ++\n", __func__);
	/*
	 * avoid that charger has ignored RX_READY notifier_call_chain
	 * when charger vbus is not powered, so here redo notifier call
	 */
	wlc_get_supported_max_rx_vout(di);
	wlc_extra_power_supply(true);
	wlc_update_fan_control(di, true);
	wireless_charge_update_max_vout_and_iout(true);
	wireless_charge_icon_display(WIRELESS_MODE_NORMAL_JUDGE_CRIT);
	cur_mode_tx_vout =
		di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.tx_vout;
	cur_mode_tx_vout = min(di->tx_vout_max, cur_mode_tx_vout);
	blocking_notifier_call_chain(&di->wireless_charge_evt_nh,
		CHARGER_TYPE_WIRELESS, &cur_mode_tx_vout);

	di->iout_low_cnt = 0;
	wireless_charge_power_mode_control(di);
	cur_mode_name =
		di->mode_data.mode_para[di->curr_pmode_index].mode_name;
	if (strstr(cur_mode_name, "SC"))
		return;

	wireless_charge_set_charge_stage(WIRELESS_STAGE_REGULATION);
	hwlog_info("[%s] --\n", __func__);
}

static bool wlc_is_support_set_rpp_format(
	struct wireless_charge_device_info *di)
{
	int ret;
	u8 tx_rpp = 0;

	if (!di || !di->ops || !di->ops->get_rpp_format ||
		!di->ops->set_rpp_format) {
		hwlog_err("%s: di null\n", __func__);
		return false;
	}
	if (di->pmax <= 0)
		return false;

	if (di->cert_succ_flag != WIRELESS_CHECK_SUCC)
		return false;

	ret = di->ops->get_rpp_format(&tx_rpp);
	if (!ret && (tx_rpp == WLC_SUPPORT_24BIT_RPP))
		return true;

	return false;
}

static int wlc_set_rpp_format(struct wireless_charge_device_info *di)
{
	int ret;
	int count = 0;

	do {
		ret = di->ops->set_rpp_format(di->pmax);
		if (!ret) {
			hwlog_info("%s: succ\n", __func__);
			return 0;
		}
		wireless_charge_msleep(100); /* 100ms delay try again */
		count++;
		hwlog_err("%s: failed, try next time\n", __func__);
	} while (count < WLC_SET_RPP_FORMAT_RETRY_CNT);

	if (count < WLC_SET_RPP_FORMAT_RETRY_CNT) {
		hwlog_info("[%s] succ\n", __func__);
		return 0;
	}

	return -WLC_ERR_I2C_WR;
}

static void wlc_rpp_format_init(struct wireless_charge_device_info *di)
{
	int ret;

	if (!di->standard_tx) {
		hwlog_err("%s: not standard tx, no need init\n", __func__);
		return;
	}
	if (!wlc_is_support_set_rpp_format(di)) {
		wlc_set_plimit_src(WLC_PLIM_SRC_RPP);
		return;
	}

	ret = wlc_set_rpp_format(di);
	if (!ret) {
		hwlog_info("[%s] succ\n", __func__);
		wlc_clear_plimit_src(WLC_PLIM_SRC_RPP);
		return;
	}
	wlc_set_plimit_src(WLC_PLIM_SRC_RPP);
}

static void wireless_charge_check_fwupdate(
	struct wireless_charge_device_info *di)
{
	int ret;

	if (( g_wireless_charge_stage != WIRELESS_STAGE_CHECK_FWUPDATE) ||
		(g_wireless_channel_state == WIRELESS_CHANNEL_OFF))
		return;

	hwlog_info("[%s] ++\n", __func__);
	ret = di->ops->check_fwupdate(WIRELESS_RX_MODE);
	if (!ret)
		wireless_charge_chip_init(WIRELESS_CHIP_INIT);

	wlc_cp_chip_init();
	wireless_charge_get_tx_info(di);
	wlc_rpp_format_init(di);
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHARGING);
	hwlog_info("[%s] --\n", __func__);
}

static void wireless_charge_check_certification(
	struct wireless_charge_device_info *di)
{
	int ret;
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };

	if ((g_wireless_charge_stage != WIRELESS_STAGE_CERTIFICATION) ||
		(g_wireless_channel_state == WIRELESS_CHANNEL_OFF))
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (di->certi_err_cnt >= CERTI_ERR_CNT_MAX) {
		di->cert_succ_flag = WIRELESS_CHECK_FAIL;
		wlc_send_cert_confirm_msg(di, false);
		if (di->wlc_err_rst_cnt >= WLC_RST_CNT_MAX) {
			wireless_charge_icon_display(
				WIRELESS_MODE_NORMAL_JUDGE_CRIT);
			wireless_charge_dsm_report(di,
				ERROR_WIRELESS_CERTI_COMM_FAIL, dsm_buff);
		}
		wlc_rx_chip_reset(di);
		hwlog_err("%s: error exceed %d times\n",
			__func__, CERTI_ERR_CNT_MAX);
	} else if (!di->tx_cap->no_need_cert &&
		di->tx_prop_data.tx_prop[di->curr_tx_type_index].need_cert) {
		if (!g_pwr_ct_srv_ready) {
			g_need_recheck_cert = true;
			wireless_charge_set_charge_stage(
				WIRELESS_STAGE_CHECK_FWUPDATE);
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
		wlc_send_cert_confirm_msg(di, true);
		di->cert_succ_flag = WIRELESS_CHECK_SUCC;
	} else {
		di->cert_succ_flag = WIRELESS_CHECK_UNKNOWN;
	}

	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_FWUPDATE);
	hwlog_info("[%s] --\n", __func__);
}
static void wireless_charge_cable_detect(struct wireless_charge_device_info *di)
{
	if ((WIRELESS_STAGE_CABLE_DETECT != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (di->tx_prop_data.tx_prop[di->curr_tx_type_index].need_cable_detect) {
		di->cable_detect_succ_flag = di->tx_cap->cable_ok;
	} else {
		di->cable_detect_succ_flag = WIRELESS_CHECK_UNKNOWN;
	}
	if (WIRELESS_CHECK_FAIL == di->cable_detect_succ_flag) {
		di->cert_succ_flag = WIRELESS_CHECK_FAIL;
		hwlog_err("%s: cable detect failed, set cert_succ_flag %d\n", __func__, di->cert_succ_flag);
		wireless_charge_icon_display(WIRELESS_MODE_NORMAL_JUDGE_CRIT);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_FWUPDATE);
		return;
	}
	hwlog_info("[%s] cable_detect_succ_flag: %d\n", __func__, di->cable_detect_succ_flag);
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CERTIFICATION);
	hwlog_info("[%s] --\n", __func__);
}

static void wlc_get_ept_type(void)
{
	int ret;
	u16 ept_type = 0;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !di->ops || !di->ops->get_ept_type)
		return;

	ret = di->ops->get_ept_type(&ept_type);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return;
	}
	hwlog_info("[%s] ept_type = 0x%x\n", __func__, ept_type);
}

static void wireless_charge_check_tx_ability(struct wireless_charge_device_info *di)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = {0};

	if ((WIRELESS_STAGE_CHECK_TX_ABILITY != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("%s ++\n", __func__);
	if (di->tx_ability_err_cnt >= TX_ABILITY_ERR_CNT_MAX) {
		wireless_charge_get_tx_prop(di);
		wireless_charge_send_fod_status(di);
		hwlog_err("%s: error exceed %d times\n",
			__func__, TX_ABILITY_ERR_CNT_MAX);
		if (di->standard_tx &&
			di->wlc_err_rst_cnt >= WLC_RST_CNT_MAX) {
			wireless_charge_dsm_report(di,
				ERROR_WIRELESS_CHECK_TX_ABILITY_FAIL,
				dsm_buff);
		}
		wlc_rx_chip_reset(di);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CABLE_DETECT);
		return;
	}
	wlc_set_iout_min(di);
	wireless_charge_get_tx_capability(di);
	if (WIRELESS_TYPE_ERR == di->tx_cap->type) {
		hwlog_err("%s: get tx ability failed\n", __func__);
		di->tx_ability_err_cnt ++;
		return;
	}
	wireless_charge_get_tx_prop(di);

	if (di->tx_cap->no_need_cert)
		wireless_charge_icon_display(WIRELESS_MODE_NORMAL_JUDGE_CRIT);
	else
		wireless_charge_icon_display(WIRELESS_MODE_QUICK_JUDGE_CRIT);

	wlc_get_ept_type();
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CABLE_DETECT);
	wireless_charge_send_fod_status(di);
	hwlog_info("%s --\n", __func__);
}
static void wireless_charge_check_tx_id(struct wireless_charge_device_info *di)
{
	int tx_id;
	if ((WIRELESS_STAGE_CHECK_TX_ID != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (di->tx_id_err_cnt >= TX_ID_ERR_CNT_MAX) {
		wireless_charge_get_tx_prop(di);
		hwlog_err("%s: error exceed %d times, fast charge is disabled\n", __func__, TX_ID_ERR_CNT_MAX);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CABLE_DETECT);
		return;
	}
	wlc_set_iout_min(di);
	tx_id = wireless_charge_get_tx_id();
	if (tx_id < 0) {
		hwlog_err("%s: get id failed\n", __func__);
		di->tx_id_err_cnt++;
		return;
	}
	if (TX_ID_HW != tx_id) {
		wireless_charge_get_tx_prop(di);
		hwlog_err("%s: id(0x%x) is not correct(0x%x)\n", __func__, tx_id, TX_ID_HW);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CABLE_DETECT);
		return;
	}
	di->standard_tx = 1;
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_TX_ABILITY);
	hwlog_info("[%s] --\n", __func__);
	return;
}
static void wireless_charge_rx_stop_charing_config(struct wireless_charge_device_info *di)
{
	int ret;
	ret = di->ops->stop_charging();
	if (ret < 0) {
		hwlog_err("%s: rx stop charing config failed\n", __func__);
	}
}

static void wlc_state_record_para_init(struct wireless_charge_device_info *di)
{
	di->stat_rcd.chrg_state_cur = 0;
	di->stat_rcd.chrg_state_last = 0;
	di->stat_rcd.fan_cur = WLC_LIMIT_FAN_UNKNOWN_SPEED;
	di->stat_rcd.fan_last = WLC_LIMIT_FAN_UNKNOWN_SPEED;
}

static void wireless_charge_para_init(struct wireless_charge_device_info *di)
{
	di->monitor_interval = MONITOR_INTERVAL;
	di->ctrl_interval = CONTROL_INTERVAL_NORMAL;
	di->tx_vout_max = TX_DEFAULT_VOUT;
	di->rx_iout_max = di->rx_iout_min;
	di->rx_iout_limit = di->rx_iout_min;
	di->standard_tx = 0;
	di->tx_id_err_cnt = 0;
	di->tx_ability_err_cnt = 0;
	di->certi_err_cnt = 0;
	di->boost_err_cnt = 0;
	di->sysfs_data.en_enable = 0;
	di->iout_high_cnt = 0;
	di->iout_low_cnt = 0;
	di->cable_detect_succ_flag = 0;
	di->cert_succ_flag = 0;
	di->curr_tx_type_index = 0;
	di->curr_pmode_index = 0;
	di->curr_vmode_index = 0;
	di->curr_power_time_out = 0;
	di->pwroff_reset_flag = 0;
	di->supported_rx_vout = RX_DEFAULT_VOUT;
	di->extra_pwr_good_flag = 1;
	g_rx_vrect_restore_cnt = 0;
	g_rx_vout_err_cnt = 0;
	g_rx_ocp_cnt = 0;
	g_rx_ovp_cnt = 0;
	g_rx_otp_cnt = 0;
	g_tx_fw_version = NULL;
	wlc_reset_plimit(di);
	wlc_state_record_para_init(di);
	wireless_charge_set_default_tx_capability(di);
	wireless_charge_reset_avg_iout(di);
	wlc_reset_icon_pmode(di);
	charge_set_input_current_prop(di->rx_iout_step, CHARGE_CURRENT_DELAY);
	wlc_set_iout_min(di);
}
static void wireless_charge_control_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	wireless_charge_check_tx_id(di);
	wireless_charge_check_tx_ability(di);
	wireless_charge_cable_detect(di);
	wireless_charge_check_certification(di);
	wireless_charge_check_fwupdate(di);
	wireless_charge_start_charging(di);
	wireless_charge_regulation(di);
	wireless_charge_set_ctrl_interval(di);

	if (WIRELESS_CHANNEL_ON == g_wireless_channel_state &&
		WIRELESS_STAGE_REGULATION_DC != g_wireless_charge_stage) {
		schedule_delayed_work(&di->wireless_ctrl_work, msecs_to_jiffies(di->ctrl_interval));
	}
}
void wireless_charge_restart_charging(enum wireless_charge_stage stage_from)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (WIRELESS_CHANNEL_ON == g_wireless_channel_state &&
		g_wireless_charge_stage >= WIRELESS_STAGE_CHARGING) {
		wireless_charge_set_charge_stage(stage_from);
		schedule_delayed_work(&di->wireless_ctrl_work,
			msecs_to_jiffies(100)); /* 100ms for pmode stablity */
	}
}
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
static void wireless_charge_rx_program_otp_work(struct work_struct *work)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: wireless charge di null\n", __func__);
		return;
	}
	hwlog_info("%s ++\n", __func__);

	ret = di->ops->rx_program_otp();
	if (ret) {
		hwlog_err("%s: wireless rx program otp fail\n", __func__);
	}
	hwlog_info("%s --\n", __func__);
}
#endif
static void wireless_charge_stop_charging(struct wireless_charge_device_info *di)
{
	hwlog_info("%s ++\n", __func__);
	wireless_charge_sleep_en_enable(RX_SLEEP_EN_ENABLE);
	wlc_extra_power_supply(false);
	pd_dpm_ignore_vbus_only_event(false);
	wireless_charge_set_charge_stage(WIRELESS_STAGE_DEFAULT);
	charge_set_input_current_prop(0, 0);
	wireless_charge_rx_stop_charing_config(di);
	wireless_fast_charge_flag = 0;
	wireless_super_charge_flag = 0;
	g_fop_fixed_flag = 0;
	cancel_delayed_work_sync(&di->rx_sample_work);
	cancel_delayed_work_sync(&di->wireless_ctrl_work);
	di->curr_pmode_index = 0;
	di->curr_icon_type = 0;
	di->wlc_err_rst_cnt = 0;
	wireless_charge_set_default_tx_capability(di);
	hwlog_info("%s --\n", __func__);
}

static void wlc_wireless_vbus_connect_handler(
	enum wireless_charge_stage stage_from)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (g_wired_channel_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired vbus connect, stop wireless handler\n", __func__);
		return;
	}
	wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_ON);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	wlps_control(WLPS_RX_SW, WLPS_CTRL_ON);
	wireless_charge_sleep_en_enable(RX_SLEEP_EN_DISABLE);
	wireless_charge_chip_init(WIRELESS_CHIP_INIT);
	di->tx_vout_max = di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.tx_vout;
	di->rx_vout_max = di->mode_data.mode_para[di->curr_pmode_index].ctrl_para.rx_vout;
	ret = wireless_charge_set_tx_vout(TX_DEFAULT_VOUT);
	ret += wireless_charge_set_rx_vout(RX_DEFAULT_VOUT);
	if (ret)
		hwlog_err("%s: set trx vout fail\n", __func__);

	if (WIRELESS_CHANNEL_ON == g_wireless_channel_state) {
		wireless_charge_set_charge_stage(stage_from);
		mod_delayed_work(system_wq, &di->wireless_ctrl_work, msecs_to_jiffies(di->ctrl_interval));
		blocking_notifier_call_chain(&di->wireless_charge_evt_nh, CHARGER_TYPE_WIRELESS, &di->tx_vout_max);
		hwlog_info("%s --\n", __func__);
	}
}

static void wireless_charge_wireless_vbus_disconnect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (wireless_charge_check_tx_exist()) {
		hwlog_info("[%s] tx exist, ignore\n", __func__);
		mod_delayed_work(system_wq,
			&di->wireless_monitor_work, msecs_to_jiffies(0));
		wlc_wireless_vbus_connect_handler(WIRELESS_STAGE_REGULATION);
		return;
	}
	wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF);
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
	int i, vout, ret;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	mutex_lock(&g_rx_en_mutex);
	vout = wireless_charge_get_rx_vout();
	if (vout >= RX_HIGH_VOUT) {
		wireless_charge_rx_stop_charing_config(di);
		ret = wireless_charge_set_tx_vout(TX_DEFAULT_VOUT);
		ret |= wireless_charge_set_rx_vout(TX_DEFAULT_VOUT);
		if (ret)
			hwlog_err("%s: set trx vout fail\n", __func__);
		if (WIRED_CHANNEL_OFF == g_wired_channel_state) {
			hwlog_err("%s: wired vubs already off, reset rx\n", __func__);
			wlc_rx_chip_reset(di);
		}
		if (!wireless_is_in_tx_mode())
			wireless_charge_en_enable(RX_EN_DISABLE);
		wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	} else {
		if (!wireless_is_in_tx_mode())
			wireless_charge_en_enable(RX_EN_DISABLE);
		wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	}
	mutex_unlock(&g_rx_en_mutex);
	for (i = 0; i < 10; i++) {  //10: only used here
		if (wldc_is_stop_charging_complete()) {
			wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
			break;
		}
		msleep(50);  //here wait for 10*50ms at most, generally 300ms at most
	}
	hwlog_info("wired vbus connect, turn off wireless channel\n");
	wireless_charge_stop_charging(di);
}
static void wireless_charge_wired_vbus_disconnect_work(struct work_struct *work)
{
	mutex_lock(&g_rx_en_mutex);
	wireless_charge_en_enable(RX_EN_ENABLE);
	mutex_unlock(&g_rx_en_mutex);
	hwlog_info("wired vbus disconnect, turn on wireless channel\n");
}
void wireless_charge_wired_vbus_connect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (WIRED_CHANNEL_ON == g_wired_channel_state) {
		hwlog_err("%s: already in sink_vbus state, ignore\n", __func__);
		return;
	}
	hwlog_info("[%s] wired vbus connect\n", __func__);
	wireless_super_charge_flag = 0;
	wireless_charge_set_wired_channel_state(WIRED_CHANNEL_ON);
	wldc_tx_disconnect_handler();
	wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF);
	if (!wireless_fast_charge_flag) {
		wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
	}
	schedule_work(&di->wired_vbus_connect_work);
}
void wireless_charge_wired_vbus_disconnect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	static bool first_in = true;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	if (!first_in && WIRED_CHANNEL_OFF == g_wired_channel_state) {
		hwlog_err("%s: not in sink_vbus state, ignore\n", __func__);
		return;
	}
	first_in = false;
	hwlog_info("[%s] wired vbus disconnect\n", __func__);
	wireless_charge_set_wired_channel_state(WIRED_CHANNEL_OFF);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	schedule_work(&di->wired_vbus_disconnect_work);
}
#ifdef CONFIG_DIRECT_CHARGER
void direct_charger_disconnect_event(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	hwlog_info("wired vbus disconnect in scp charging mode\n");
	wireless_charge_set_wired_channel_state(WIRED_CHANNEL_OFF);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	schedule_work(&di->wired_vbus_disconnect_work);
}
#endif
void wireless_charger_pmic_vbus_handler(bool vbus_state)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (di && di->ops && di->ops->pmic_vbus_handler) {
		di->ops->pmic_vbus_handler(vbus_state);
	}
}

static int wireless_charge_check_tx_disconnect
			(struct wireless_charge_device_info *di)
{
	if (wireless_charge_check_tx_exist())
		return 0;

	wldc_tx_disconnect_handler();
	wireless_charge_sleep_en_enable(RX_SLEEP_EN_ENABLE);
	wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	wireless_charge_rx_stop_charing_config(di);
	cancel_delayed_work_sync(&di->wireless_ctrl_work);
	cancel_delayed_work_sync(&di->wireless_vbus_disconnect_work);
	schedule_delayed_work(&di->wireless_vbus_disconnect_work,
		msecs_to_jiffies(di->discon_delay_time));
	hwlog_err("%s: tx not exist, delay %dms to report disconnect event\n",
		__func__, di->discon_delay_time);

	return -1;
}

static void wlc_show_monitor_info(struct wireless_charge_device_info *di)
{
	int soc;
	int tbatt;
	int iout;
	int max_iout;
	int vout;
	int vrect;
	int rx_temp;
	int fop;
	int iin_regval;
	int wldc_err_cnt;
	int wldc_warning_cnt;
	static int cnt = 0;
	static int iin_regval_last = 0;

	iin_regval = charge_get_charger_iinlim_regval();
	if ((g_wireless_charge_stage < WIRELESS_STAGE_CHARGING) ||
		(++cnt == MONITOR_LOG_INTERVAL / di->monitor_interval) ||
		(iin_regval_last != iin_regval)) {
		soc = hisi_battery_capacity();
		tbatt = hisi_battery_temperature();
		vrect = wireless_charge_get_rx_vrect();
		vout = wireless_charge_get_rx_vout();
		iout = wireless_charge_get_rx_iout();
		max_iout = wlc_get_rx_max_iout();
		fop = wireless_charge_get_rx_fop();
		rx_temp = wlc_get_rx_temp();
		wldc_warning_cnt = wldc_get_warning_cnt();
		wldc_err_cnt = wldc_get_error_cnt();

		hwlog_info("[%s] soc:%-3d tbatt:%d pmode:%d plim_src:0x%02x\t"
			"[dc] warn:%d err:%d\t"
			"[tx] plim:%d\t"
			"[rx] temp:%-3d fop:%-3d vrect:%-5d vout=%-5d\t"
			"[rx] imax:%-4d iout:%-4d iout_avg:%-4d iin_reg:%-4d\t"
			"[sysfs] fop:%-3d irx:%-4d vrx:%-5d vtx:%-5d\n",
			__func__, soc, tbatt,
			di->curr_pmode_index, di->plimit_src,
			wldc_warning_cnt, wldc_err_cnt,
			di->tx_evt_plim, rx_temp, fop,
			vrect, vout, max_iout, iout, di->iout_avg, iin_regval,
			di->sysfs_data.tx_fixed_fop, di->sysfs_data.rx_iout_max,
			di->sysfs_data.rx_vout_max, di->sysfs_data.tx_vout_max);
		cnt = 0;
		iin_regval_last = iin_regval;
	}
}

static void wireless_charge_monitor_work(struct work_struct *work)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;

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
	wireless_charge_kick_watchdog(di);
	wlc_check_voltage(di);
	wireless_charge_update_status(di);
	wlc_show_monitor_info(di);

	schedule_delayed_work(&di->wireless_monitor_work,
		msecs_to_jiffies(di->monitor_interval));
}

static void wireless_charge_rx_sample_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di = container_of(work, struct wireless_charge_device_info, rx_sample_work.work);
	int ret = 0;
	int rx_vout;
	int rx_iout;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	/*send confirm message to TX */
	rx_vout = di->ops->get_rx_vout();
	rx_iout = di->ops->get_rx_iout();
	ret = di->ops->send_msg_rx_vout(rx_vout);
	ret |= di->ops->send_msg_rx_iout(rx_iout);
	if(ret) {
		hwlog_err("%s: RX send message to TX failed!\n", __func__);
	}
	hwlog_info("[%s] rx_vout = %d, rx_iout = %d\n", __func__, rx_vout,rx_iout);

	schedule_delayed_work(&di->rx_sample_work, msecs_to_jiffies(RX_SAMPLE_WORK_DELAY));
}
static void wireless_charge_pwroff_reset_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di =
		container_of(work, struct wireless_charge_device_info, wireless_pwroff_reset_work);
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		wireless_charge_wake_unlock();
		return;
	}
	if (di->pwroff_reset_flag) {
		msleep(60);  //test result, about 60ms
		(void)di->ops->chip_reset();
		wireless_charge_set_tx_vout(TX_DEFAULT_VOUT);
		wireless_charge_set_rx_vout(RX_DEFAULT_VOUT);
	}
	wireless_charge_wake_unlock();
}

static void wlc_rx_power_on_ready_handler(
	struct wireless_charge_device_info *di)
{
	wldc_set_charge_stage(WLDC_STAGE_DEFAULT);
	wireless_charge_set_charge_stage(WIRELESS_STAGE_DEFAULT);
	wireless_charge_para_init(di);
	if (((di->rx_event_type == WIRELESS_CHARGE_RX_POWER_ON) &&
		di->rx_event_data) ||  /* power on good */
		(di->rx_event_type == WIRELESS_CHARGE_RX_READY)) {
		wltx_reset_reverse_charging();
		charger_source_sink_event(START_SINK_WIRELESS);
	}
	pd_dpm_ignore_vbus_only_event(true);
	mod_delayed_work(system_wq, &di->wireless_monitor_work,
		msecs_to_jiffies(0));
	if (delayed_work_pending(&di->wireless_vbus_disconnect_work))
		cancel_delayed_work_sync(&di->wireless_vbus_disconnect_work);
	if (di->rx_event_type == WIRELESS_CHARGE_RX_READY) {
		if (!di->wlc_err_rst_cnt)
			wireless_fast_charge_flag = 0;
		if (di->ops->get_ext_5v_fod_cfg &&
			di->ops->get_ext_5v_fod_cfg())
			wlc_check_dev_back_color();
		di->discon_delay_time = WL_DISCONN_DELAY_MS;
		wlc_wireless_vbus_connect_handler(WIRELESS_STAGE_CHECK_TX_ID);
	}
}

static void wlc_tx_alarm_handler(struct wireless_charge_device_info *di)
{
	di->tx_evt_plim = di->rx_event_data * 1000 * 1000; /* mA*mV */
	/* if tx_plim is less than 12w, sc 4:1 is prohibited */
	if (di->rx_event_data && (di->rx_event_data < 12))
		wlc_set_plimit_src(WLC_PLIM_TX_ALARM);
	else
		wlc_clear_plimit_src(WLC_PLIM_TX_ALARM);
	hwlog_err("%s: limit rx power to %dw\n", __func__, di->tx_evt_plim);
}

static void wlc_rx_event_work(struct work_struct *work)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0512] = { 0 };
	struct wireless_charge_device_info *di = container_of(work,
		struct wireless_charge_device_info, rx_event_work.work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		wireless_charge_wake_unlock();
		return;
	}

	switch(di->rx_event_type) {
		case WIRELESS_CHARGE_RX_POWER_ON:
			hwlog_info("[%s] RX power on \n",__func__);
			wlc_rx_power_on_ready_handler(di);
			break;
		case WIRELESS_CHARGE_RX_READY:
			hwlog_info("[%s] RX ready \n",__func__);
			wlc_rx_power_on_ready_handler(di);
			break;
		case WIRELESS_CHARGE_SET_CURRENT_LIMIT:
			hwlog_info("[%s] set current limit = %dmA\n",__func__, di->rx_event_data * SET_CURRENT_LIMIT_STEP);
			di->rx_iout_limit = di->rx_event_data * SET_CURRENT_LIMIT_STEP;
			wireless_charge_set_input_current(di);
			break;
		case WIRELESS_CHARGE_START_SAMPLE:
			hwlog_info("[%s] RX start sample \n",__func__);
			wireless_start_sample_flag = 1;
			if (!delayed_work_pending(&di->rx_sample_work))
				mod_delayed_work(system_wq, &di->rx_sample_work,
					msecs_to_jiffies(0));
			break;
		case WIRELESS_CHARGE_STOP_SAMPLE:
			hwlog_info("[%s] RX stop sample \n",__func__);
			wireless_start_sample_flag = 0;
			cancel_delayed_work_sync(&di->rx_sample_work);
			break;
		case WIRELESS_CHARGE_RX_OCP:
			if (g_wireless_charge_stage >= WIRELESS_STAGE_REGULATION) {
				hwlog_err("RX ocp happend \n");
				g_rx_ocp_cnt++;
			}
			if (g_rx_ocp_cnt >= RX_OCP_CNT_MAX) {
				g_rx_ocp_cnt = RX_OCP_CNT_MAX;
				wireless_charge_dsm_report(di, ERROR_WIRELESS_RX_OCP, dsm_buff);
			}
			break;
		case WIRELESS_CHARGE_RX_OVP:
			if (g_wireless_charge_stage >= WIRELESS_STAGE_REGULATION) {
				hwlog_err("RX ovp happend \n");
				g_rx_ovp_cnt++;
			}
			if (g_rx_ovp_cnt >= RX_OVP_CNT_MAX) {
				g_rx_ovp_cnt = RX_OVP_CNT_MAX;
				wireless_charge_dsm_report(di, ERROR_WIRELESS_RX_OVP, dsm_buff);
			}
			break;
		case WIRELESS_CHARGE_RX_OTP:
			if (g_wireless_charge_stage >= WIRELESS_STAGE_REGULATION) {
				hwlog_err("RX otp happend \n");
				g_rx_otp_cnt++;
			}
			if (g_rx_otp_cnt >= RX_OTP_CNT_MAX) {
				g_rx_otp_cnt = RX_OTP_CNT_MAX;
				wireless_charge_dsm_report(di, ERROR_WIRELESS_RX_OTP, dsm_buff);
			}
			break;
		case WIRELESS_CHARGE_RX_LDO_OFF:
			hwlog_info("[%s] RX ldo off happend\n", __func__);
			charger_source_sink_event(STOP_SINK_WIRELESS);
			cancel_delayed_work_sync(&di->wireless_ctrl_work);
			cancel_delayed_work_sync(&di->wireless_monitor_work);
			break;
	case WLC_RX_PWR_LIM_TX_ALARM:
		wlc_tx_alarm_handler(di);
		break;
	case WLC_RX_PWR_LIM_TX_BST_ERR:
		wlc_set_plimit_src(WLC_PLIM_TX_BST_ERR);
		wireless_charge_update_max_vout_and_iout(true);
		break;
	default:
		hwlog_err("%s: invalid event type\n", __func__);
		break;
	}
	wireless_charge_wake_unlock();
}

static int wireless_charge_rx_event_notifier_call(
	struct notifier_block *rx_event_nb, unsigned long event, void *data)
{
	struct wireless_charge_device_info *di = container_of(rx_event_nb,
		struct wireless_charge_device_info, rx_event_nb);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return NOTIFY_OK;
	}

	wireless_charge_wake_lock();
	di->rx_event_data = 0;
	if (data)
		di->rx_event_data = *((int *)data);
	di->rx_event_type = (enum rx_event_type)event;

	cancel_delayed_work_sync(&di->rx_event_work);
	mod_delayed_work(system_wq, &di->rx_event_work,
		msecs_to_jiffies(0));

	return NOTIFY_OK;
}

static int wireless_charge_pwrkey_event_notifier_call(struct notifier_block *pwrkey_event_nb, unsigned long event, void *data)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if(!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return NOTIFY_OK;
	}

	switch(event) {
		case HISI_PRESS_KEY_6S:
			wireless_charge_wake_lock();
			hwlog_err("%s: response long press 6s interrupt, reset tx vout\n", __func__);
			di->pwroff_reset_flag = 1;
			schedule_work(&di->wireless_pwroff_reset_work);
			break;
		case HISI_PRESS_KEY_UP:
			di->pwroff_reset_flag = 0;
			break;
		default:
			break;
	}
	return NOTIFY_OK;
}
static int wireless_charge_chrg_event_notifier_call(struct notifier_block *chrg_event_nb, unsigned long event, void *data)
{
	struct wireless_charge_device_info *di =
	    container_of(chrg_event_nb, struct wireless_charge_device_info, chrg_event_nb);
	if(!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return NOTIFY_OK;
	}
	switch(event) {
		case CHARGER_CHARGING_DONE_EVENT:
			hwlog_debug("[%s] charge done\n", __func__);
			di->stat_rcd.chrg_state_cur |= WIRELESS_STATE_CHRG_DONE;
			break;
		default:
			break;
	}
	return NOTIFY_OK;
}

static int wireless_charge_af_srv_on_cb(void)
{
	g_pwr_ct_srv_ready = true;
	hwlog_info("[%s] power_ct service ready\n", __func__);

	return 0;
}

static int wireless_charge_af_cb(unsigned char version, void *data, int len)
{
	int i;
	struct wireless_charge_device_info *di = g_wireless_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (len != WIRELESS_RX_KEY_LEN) {
		hwlog_err("%s: err, len=%d\n", __func__, len);
		return -WLC_ERR_PARA_WRONG;
	}

	memcpy(rx_cipherkey, (u8 *)data, len);
	complete(&di->wc_af_completion);

	for (i = 0; i < WIRELESS_RX_KEY_LEN; i++)
		hwlog_info("rx_cipherkey[%d]=0x%x\n", i, rx_cipherkey[i]);

	return 0;
}

static int wireless_charge_gen_nl_init(struct platform_device *pdev)
{
	int ret;
	ret = power_easy_node_register(&wc_af_info_node);
	if(ret)
		hwlog_err("%s: power_genl_add_op failed!\n", __func__);
	return ret;
}
static void wireless_charge_parse_interfer_para
		(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	unsigned int i = 0;
	int string_len = 0;
	int idata = 0;
	const char *chrg_data_string = NULL;

	string_len = of_property_count_strings(np, "interference_para");
	if ((string_len <= 0) ||(string_len % WIRELESS_INTERFER_TOTAL != 0)) {
		di->interfer_data.total_src = 0;
		hwlog_err("%s: para is invaild, please check interference_para number!!\n", __func__);
	} else if (string_len > WIRELESS_INTERFER_PARA_LEVEL * WIRELESS_INTERFER_TOTAL) {
		di->interfer_data.total_src = 0;
		hwlog_err("%s: para is too long(%d)!!\n", __func__, string_len);
	} else {
		di->interfer_data.interfer_src_state = 0;
		di->interfer_data.total_src = string_len / WIRELESS_INTERFER_TOTAL;
		for (i = 0; i < string_len; i++) {
			ret = of_property_read_string_index(np, "interference_para", i, &chrg_data_string);
			if (ret) {
				di->interfer_data.total_src = 0;
				hwlog_err("%s: get interference_para failed\n", __func__);
				return;
			}
			idata = simple_strtol(chrg_data_string, NULL, 0);
			switch (i % WIRELESS_INTERFER_TOTAL) {
			case WIRELESS_INTERFER_SRC_OPEN:
				di->interfer_data.interfer_para[i / WIRELESS_INTERFER_TOTAL].src_open = (u8)idata;
				break;
			case WIRELESS_INTERFER_SRC_CLOSE:
				di->interfer_data.interfer_para[i / WIRELESS_INTERFER_TOTAL].src_close = (u8)idata;
				break;
			case WIRELESS_INTERFER_TX_FIXED_FOP:
				di->interfer_data.interfer_para[i / WIRELESS_INTERFER_TOTAL].tx_fixed_fop = (int)idata;
				break;
			case WIRELESS_INTERFER_TX_VOUT_LIMIT:
				di->interfer_data.interfer_para[i / WIRELESS_INTERFER_TOTAL].tx_vout_limit = (int)idata;
				break;
			case WIRELESS_INTERFER_RX_VOUT_LIMIT:
				di->interfer_data.interfer_para[i / WIRELESS_INTERFER_TOTAL].rx_vout_limit = (int)idata;
				break;
			case WIRELESS_INTERFER_RX_IOUT_LIMIT:
				di->interfer_data.interfer_para[i / WIRELESS_INTERFER_TOTAL].rx_iout_limit = (int)idata;
				break;
			default:
				hwlog_err("%s: get interference_para failed\n", __func__);
			}
		}
		for (i = 0; i < di->interfer_data.total_src; i++) {
			hwlog_info("wireless_interfer_para[%d], src_open: 0x%-2x src_close: 0x%-2x tx_fixed_fop: %-3d "
						"tx_vout_limit: %-5d rx_vout_limit: %-5d rx_iout_limit: %-4d\n",
						i, di->interfer_data.interfer_para[i].src_open, di->interfer_data.interfer_para[i].src_close,
						di->interfer_data.interfer_para[i].tx_fixed_fop, di->interfer_data.interfer_para[i].tx_vout_limit,
						di->interfer_data.interfer_para[i].rx_vout_limit, di->interfer_data.interfer_para[i].rx_iout_limit);
		}
	}
}
static void wireless_charge_parse_segment_para
			(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	int i = 0;
	int array_len = 0;
	u32 temp_para[WIRELESS_SEGMENT_PARA_TOTAL * WIRELESS_SEGMENT_PARA_LEVEL];

	array_len = of_property_count_u32_elems(np, "segment_para");
	if ((array_len <= 0) || (array_len % WIRELESS_SEGMENT_PARA_TOTAL != 0)) {
		di->segment_data.segment_para_level = 0;
		hwlog_err("%s: para is invaild, please check!\n", __func__);
	} else if (array_len > WIRELESS_SEGMENT_PARA_LEVEL * WIRELESS_SEGMENT_PARA_TOTAL) {
		di->segment_data.segment_para_level = 0;
		hwlog_err("%s: para is too long, please check!!\n", __func__);
	} else {
		ret = of_property_read_u32_array(np, "segment_para", temp_para, array_len);
		if (ret) {
			di->segment_data.segment_para_level = 0;
			hwlog_err("%s: get para fail!\n", __func__);
		} else {
			di->segment_data.segment_para_level = array_len / WIRELESS_SEGMENT_PARA_TOTAL;
			for (i = 0; i < di->segment_data.segment_para_level; i++) {
				di->segment_data.segment_para[i].soc_min = (int)temp_para[WIRELESS_SEGMENT_PARA_SOC_MIN + WIRELESS_SEGMENT_PARA_TOTAL * i];
				di->segment_data.segment_para[i].soc_max = (int)temp_para[WIRELESS_SEGMENT_PARA_SOC_MAX + WIRELESS_SEGMENT_PARA_TOTAL * i];
				di->segment_data.segment_para[i].tx_vout_limit = (int)temp_para[WIRELESS_SEGMENT_PARA_TX_VOUT_LIMIT + WIRELESS_SEGMENT_PARA_TOTAL * i];
				di->segment_data.segment_para[i].rx_vout_limit = (int)temp_para[WIRELESS_SEGMENT_PARA_RX_VOUT_LIMIT + WIRELESS_SEGMENT_PARA_TOTAL * i];
				di->segment_data.segment_para[i].rx_iout_limit = (int)temp_para[WIRELESS_SEGMENT_PARA_RX_IOUT_LIMIT + WIRELESS_SEGMENT_PARA_TOTAL * i];
				hwlog_info("wireless_segment_para[%d], soc_min: %-3d soc_max: %-3d tx_vout_limit: %-5d rx_vout_limit: %-5d rx_iout_limit: %-4d\n",
							i, di->segment_data.segment_para[i].soc_min, di->segment_data.segment_para[i].soc_max,
							di->segment_data.segment_para[i].tx_vout_limit, di->segment_data.segment_para[i].rx_vout_limit,
							di->segment_data.segment_para[i].rx_iout_limit);
			}
		}
	}
}
static void wireless_charge_parse_iout_ctrl_para
			(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	int i = 0;
	int array_len = 0;
	u32 temp_para[WIRELESS_ICTRL_TOTAL * WIRELESS_IOUT_CTRL_PARA_LEVEL];

	array_len = of_property_count_u32_elems(np, "rx_iout_ctrl_para");
	if ((array_len <= 0) || (array_len % WIRELESS_ICTRL_TOTAL != 0)) {
		di->iout_ctrl_data.ictrl_para_level = 0;
		hwlog_err("%s: para is invaild, please check!\n", __func__);
	} else if (array_len > WIRELESS_IOUT_CTRL_PARA_LEVEL * WIRELESS_ICTRL_TOTAL) {
		di->iout_ctrl_data.ictrl_para_level = 0;
		hwlog_err("%s: para is too long, please check!!\n", __func__);
	} else {
		di->iout_ctrl_data.ictrl_para = kzalloc(sizeof(u32)*array_len, GFP_KERNEL);
		if (!di->iout_ctrl_data.ictrl_para) {
			di->iout_ctrl_data.ictrl_para_level = 0;
			hwlog_err("%s: alloc ictrl_para failed\n", __func__);
			return;
		}
		ret = of_property_read_u32_array(np, "rx_iout_ctrl_para", temp_para, array_len);
		if (ret) {
			di->iout_ctrl_data.ictrl_para_level = 0;
			hwlog_err("%s: get rx_iout_ctrl_para fail!\n", __func__);
		} else {
			di->iout_ctrl_data.ictrl_para_level = array_len / WIRELESS_ICTRL_TOTAL;
			for (i = 0; i < di->iout_ctrl_data.ictrl_para_level; i++) {
				di->iout_ctrl_data.ictrl_para[i].iout_min = (int)temp_para[WIRELESS_ICTRL_IOUT_MIN + WIRELESS_ICTRL_TOTAL * i];
				di->iout_ctrl_data.ictrl_para[i].iout_max = (int)temp_para[WIRELESS_ICTRL_IOUT_MAX + WIRELESS_ICTRL_TOTAL * i];
				di->iout_ctrl_data.ictrl_para[i].iout_set = (int)temp_para[WIRELESS_ICTRL_IOUT_SET + WIRELESS_ICTRL_TOTAL * i];
				hwlog_info("wireless_iout_ctrl_para[%d], iout_min: %-4d iout_max: %-4d iout_set: %-4d\n",
							i, di->iout_ctrl_data.ictrl_para[i].iout_min, di->iout_ctrl_data.ictrl_para[i].iout_max,
							di->iout_ctrl_data.ictrl_para[i].iout_set);
			}
		}
	}
}
static int wireless_charge_parse_mode_para(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	int i = 0;
	int string_len = 0;
	int idata = 0;
	const char *chrg_data_string = NULL;
	string_len = of_property_count_strings(np, "rx_mode_para");
	if ((string_len <= 0) || (string_len % WIRELESS_MODE_INFO_TOTAL != 0)) {
		di->mode_data.total_mode = 0;
		hwlog_err("%s: rx_mode_para is invaild,please check rx_mode_para number!!\n", __func__);
		return -EINVAL;
	}
	else if (string_len > WIRELESS_MODE_TYPE_MAX * WIRELESS_MODE_INFO_TOTAL) {
		di->mode_data.total_mode = 0;
		string_len = WIRELESS_MODE_TYPE_MAX * WIRELESS_MODE_INFO_TOTAL;
		hwlog_err("%s: rx_mode_para is too long,use only front %d paras!!\n", __func__, string_len);
		return -EINVAL;
	}
	else {
		di->mode_data.total_mode = string_len / WIRELESS_MODE_INFO_TOTAL;
		di->mode_data.mode_para = kzalloc(sizeof(struct wireless_mode_para)*di->mode_data.total_mode, GFP_KERNEL);
		if (!di->mode_data.mode_para) {
			di->mode_data.total_mode = 0;
			hwlog_err("%s: alloc mode_para failed\n", __func__);
			return -EINVAL;
		}
		for (i = 0; i < string_len; i++) {
			ret = of_property_read_string_index(np, "rx_mode_para", i, (const char **)&chrg_data_string);
			if (ret) {
				di->mode_data.total_mode = 0;
				hwlog_err("%s: get rx_mode_para failed\n", __func__);
				return -EINVAL;
			}
			idata = simple_strtol(chrg_data_string, NULL, 10);
			switch (i % WIRELESS_MODE_INFO_TOTAL) {
			case WIRELESS_MODE_NAME:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].mode_name = chrg_data_string;
				break;
			case WIRELESS_MODE_TX_VOUT_MIN:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].tx_vout_min = (int)idata;
				break;
			case WIRELESS_MODE_TX_IOUT_MIN:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].tx_iout_min = (int)idata;
				break;
			case WIRELESS_MODE_TX_VOUT:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].ctrl_para.tx_vout = (int)idata;
				break;
			case WIRELESS_MODE_RX_VOUT:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].ctrl_para.rx_vout = (int)idata;
				break;
			case WIRELESS_MODE_RX_IOUT:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].ctrl_para.rx_iout = (int)idata;
				break;
			case WIRELESS_MODE_VRECT_LOW_TH:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].vrect_low_th = (int)idata;
				break;
			case WIRELESS_MODE_TBATT:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].tbatt = (int)idata;
				break;
			case WIRELESS_MODE_EXPECT_CABLE_DETECT:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].expect_cable_detect = (s8)idata;
				break;
			case WIRELESS_MODE_EXPECT_CERT:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].expect_cert = (s8)idata;
				break;
			case WIRELESS_MODE_ICON_TYPE:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].icon_type = (u8)idata;
				break;
			case WIRELESS_MODE_MAX_TIME:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].max_time = (int)idata;
				break;
			case WIRELESS_MODE_EXPECT_MODE:
				di->mode_data.mode_para[i / (WIRELESS_MODE_INFO_TOTAL)].expect_mode = (s8)idata;
				break;
			default:
				hwlog_err("%s: get rx_mode_para failed\n", __func__);
			}
		}
		for (i = 0; i < di->mode_data.total_mode; i++) {
			hwlog_info("wireless_mode[%d], mode_name: %-4s tx_vout_min: %-5d tx_iout_min: %-4d tx_vout: %-5d rx_vout: %-5d "
						"rx_iout: %-4d vrect_low_th: %-5d tbatt: %-3d expect_cable_detect: %-2d expect_cert: %-2d icon_type: %d "
						"max_time: %-4d expect_mode: %-2d\n",
						i, di->mode_data.mode_para[i].mode_name, di->mode_data.mode_para[i].tx_vout_min,
						di->mode_data.mode_para[i].tx_iout_min, di->mode_data.mode_para[i].ctrl_para.tx_vout,
						di->mode_data.mode_para[i].ctrl_para.rx_vout, di->mode_data.mode_para[i].ctrl_para.rx_iout,
						di->mode_data.mode_para[i].vrect_low_th, di->mode_data.mode_para[i].tbatt,
						di->mode_data.mode_para[i].expect_cable_detect, di->mode_data.mode_para[i].expect_cert,
						di->mode_data.mode_para[i].icon_type, di->mode_data.mode_para[i].max_time,
						di->mode_data.mode_para[i].expect_mode);
		}
	}
	return ret;
}
static int wireless_charge_parse_tx_prop_para
		(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	unsigned int i = 0;
	int string_len = 0;
	int idata = 0;
	const char *chrg_data_string = NULL;

	string_len = of_property_count_strings(np, "tx_prop");
	if ((string_len <= 0) ||(string_len % WIRELESS_TX_PROP_TOTAL != 0)) {
		di->tx_prop_data.total_prop_type = 0;
		hwlog_err("%s: para is invaild, please check tx_prop number!!\n", __func__);
		return -EINVAL;
	} else if (string_len > WIRELESS_TX_PROP_TOTAL*WIRELESS_TX_TYPE_MAX) {
		di->tx_prop_data.total_prop_type = 0;
		hwlog_err("%s: para is too long(%d)!!\n", __func__, string_len);
		return -EINVAL;
	} else {
		di->tx_prop_data.total_prop_type = string_len / WIRELESS_TX_PROP_TOTAL;
		di->tx_prop_data.tx_prop = kzalloc(sizeof(struct wireless_tx_prop_para)*di->tx_prop_data.total_prop_type, GFP_KERNEL);
		if (!di->tx_prop_data.tx_prop) {
			di->tx_prop_data.total_prop_type = 0;
			hwlog_err("%s: alloc tx_prop failed\n", __func__);
			return -EINVAL;
		}
		for (i = 0; i < string_len; i++) {
			ret = of_property_read_string_index(np, "tx_prop", i, (const char **)&chrg_data_string);
			if (ret) {
				di->tx_prop_data.total_prop_type = 0;
				hwlog_err("%s: get tx_prop failed\n", __func__);
				return -EINVAL;
			}
			idata = simple_strtol(chrg_data_string, NULL, 0);
			switch (i % WIRELESS_TX_PROP_TOTAL) {
			case WIRELESS_TX_ADAPTOR_TYPE:
				di->tx_prop_data.tx_prop[i / WIRELESS_TX_PROP_TOTAL].tx_type = (u8)idata;
				break;
			case WIRELESS_TX_TYPE_NAME:
				di->tx_prop_data.tx_prop[i / WIRELESS_TX_PROP_TOTAL].type_name = chrg_data_string;
				break;
			case WIRELESS_TX_NEED_CABLE_DETECT:
				di->tx_prop_data.tx_prop[i / WIRELESS_TX_PROP_TOTAL].need_cable_detect = (u8)idata;
				break;
			case WIRELESS_TX_NEED_CERT:
				di->tx_prop_data.tx_prop[i / WIRELESS_TX_PROP_TOTAL].need_cert = (u8)idata;
				break;
			case WIRELESS_TX_DEFAULT_VOUT:
				di->tx_prop_data.tx_prop[i / WIRELESS_TX_PROP_TOTAL].tx_default_vout = (int)idata;
				break;
			case WIRELESS_TX_DEFAULT_IOUT:
				di->tx_prop_data.tx_prop[i / WIRELESS_TX_PROP_TOTAL].tx_default_iout = (int)idata;
				break;
			default:
				hwlog_err("%s: get tx_prop failed\n", __func__);
			}
		}
		for (i = 0; i < di->tx_prop_data.total_prop_type; i++) {
			hwlog_info("tx_prop[%d], tx_type: 0x%-2x type_name: %-7s need_cable_detect: %d need_cert: %d "
						"tx_default_vout: %-5d tx_default_iout: %-4d\n",
						i, di->tx_prop_data.tx_prop[i].tx_type, di->tx_prop_data.tx_prop[i].type_name,
						di->tx_prop_data.tx_prop[i].need_cable_detect, di->tx_prop_data.tx_prop[i].need_cert,
						di->tx_prop_data.tx_prop[i].tx_default_vout, di->tx_prop_data.tx_prop[i].tx_default_iout);
		}
	}
	return ret;
}
static int wireless_charge_parse_product_para
		(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	int array_len = 0;
	u32 tmp_para[WIRELESS_CHARGE_PARA_TOTAL];

	/*product_para*/
	array_len = of_property_count_u32_elems(np, "product_para");
	if ((array_len <= 0) ||(array_len % WIRELESS_CHARGE_PARA_TOTAL != 0)) {
		hwlog_err("%s: product_para is invaild, please check product_para number!!\n", __func__);
		return -EINVAL;
	} else if (array_len > WIRELESS_CHARGE_PARA_TOTAL) {
		hwlog_err("%s: product_para is too long(%d)!!\n" , __func__, array_len);
		return -EINVAL;
	} else {
		ret = of_property_read_u32_array(np, "product_para", tmp_para, array_len);
		if (ret) {
			hwlog_err("%s: get product_para fail!\n", __func__);
			return -EINVAL;
		} else {
			di->product_para.tx_vout = (int)tmp_para[WIRELESS_CHARGE_TX_VOUT];
			di->product_para.rx_vout = (int)tmp_para[WIRELESS_CHARGE_RX_VOUT];
			di->product_para.rx_iout = (int)tmp_para[WIRELESS_CHARGE_RX_IOUT];
			hwlog_info("product_para, tx_vout: %-5dmV rx_vout: %-5dmV rx_iout: %-4dmA\n",
						di->product_para.tx_vout, di->product_para.rx_vout, di->product_para.rx_iout);
		}
	}
	return ret;
}
static int wireless_charge_parse_volt_mode_para
		(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	int array_len = 0;
	unsigned int i = 0;
	u32 tmp_para[WIRELESS_VOLT_MODE_TOTAL*WIRELESS_VOLT_MODE_TYPE_MAX];

	/*volt_mode_para*/
	array_len = of_property_count_u32_elems(np, "volt_mode");
	if ((array_len <= 0) ||(array_len % WIRELESS_VOLT_MODE_TOTAL != 0)) {
		di->volt_mode_data.total_volt_mode = 0;
		hwlog_err("%s: volt_mode_para is invaild, please check volt_mode_para number!!\n", __func__);
		return -EINVAL;
	} else if (array_len > WIRELESS_VOLT_MODE_TOTAL*WIRELESS_VOLT_MODE_TYPE_MAX) {
		di->volt_mode_data.total_volt_mode = 0;
		hwlog_err("%s: volt_mode_para is too long(%d)!!\n" , __func__, array_len);
		return -EINVAL;
	} else {
		ret = of_property_read_u32_array(np, "volt_mode", tmp_para, array_len);
		if (ret) {
			di->volt_mode_data.total_volt_mode = 0;
			hwlog_err("%s: get volt_mode fail!\n", __func__);
			return -EINVAL;
		} else {
			di->volt_mode_data.total_volt_mode = array_len / WIRELESS_VOLT_MODE_TOTAL;
			di->volt_mode_data.volt_mode = kzalloc(sizeof(struct wireless_volt_mode_para)*di->volt_mode_data.total_volt_mode, GFP_KERNEL);
			if (!di->volt_mode_data.volt_mode) {
				di->volt_mode_data.total_volt_mode = 0;
				hwlog_err("%s: alloc volt_mode failed\n", __func__);
				return -EINVAL;
			}
			for (i = 0; i < di->volt_mode_data.total_volt_mode; i++) {
				di->volt_mode_data.volt_mode[i].mode_type =
							(u8)tmp_para[WIRELESS_VOLT_MODE_TYPE + WIRELESS_VOLT_MODE_TOTAL * i];
				di->volt_mode_data.volt_mode[i].tx_vout =
							(int)tmp_para[WIRELESS_VOLT_MODE_TX_VOUT + WIRELESS_VOLT_MODE_TOTAL * i];
				hwlog_info("volt_mode[%d], mode_type: %d tx_vout: %-5d\n",
							i, di->volt_mode_data.volt_mode[i].mode_type, di->volt_mode_data.volt_mode[i].tx_vout);
			}
		}
	}
	return ret;
}
static int wireless_charge_parse_dts(struct device_node *np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	ret = of_property_read_u32(np, "hvc_need_5vbst", &di->hvc_need_5vbst);
	if (ret) {
		hwlog_err("%s: get hvc_need_5vbst failed\n", __func__);
		di->hvc_need_5vbst = 0;
	}
	hwlog_info("[%s] hvc_need_5vbst = %d\n", __func__, di->hvc_need_5vbst);
	ret = of_property_read_u32(np, "bst5v_ignore_vbus_only",
		&di->bst5v_ignore_vbus_only);
	if (ret) {
		hwlog_err("%s: get bst5v_ignore_vbus_only failed\n", __func__);
		di->bst5v_ignore_vbus_only = 0;
	}
	hwlog_info("[%s] bst5v_ignore_vbus_only = %d\n",
		__func__, di->bst5v_ignore_vbus_only);
	ret = of_property_read_u32(np, "standard_tx_adaptor", &di->standard_tx_adaptor);
	if (ret) {
		hwlog_err("%s: get standard_tx_adaptor failed\n", __func__);
		di->standard_tx_adaptor = WIRELESS_UNKOWN;
	}
	hwlog_info("[%s] standard_tx_adaptor  = %d.\n", __func__, di->standard_tx_adaptor);
	ret = of_property_read_u32(np, "rx_vout_err_ratio", &di->rx_vout_err_ratio);
	if (ret) {
		hwlog_err("%s: get rx_vout_err_ratio failed\n", __func__);
		di->rx_vout_err_ratio = RX_VOUT_ERR_RATIO;
	}
	hwlog_info("[%s] rx_vout_err_ratio  = %d%%.\n", __func__, di->rx_vout_err_ratio);
	ret = of_property_read_u32(np, "rx_iout_min", &di->rx_iout_min);
	if (ret) {
		hwlog_err("%s: get rx_iout_min failed\n", __func__);
		di->rx_iout_min = RX_IOUT_MIN;
	}
	hwlog_info("[%s] rx_iout_min = %dmA\n", __func__, di->rx_iout_min);
	ret = of_property_read_u32(np, "rx_iout_step", &di->rx_iout_step);
	if (ret) {
		hwlog_err("%s: get rx_iout_step failed\n", __func__);
		di->rx_iout_step = RX_IOUT_REG_STEP;
	}
	hwlog_info("[%s] rx_iout_step = %dmA\n", __func__, di->rx_iout_step);
	ret = of_property_read_u32(np, "antifake_key_index", &di->antifake_key_index);
	if (ret) {
		hwlog_err("%s: get antifake_key_index failed\n", __func__);
		di->antifake_key_index = 1;
	}
	if (di->antifake_key_index >= WC_AF_TOTAL_KEY_NUM || di->antifake_key_index < 0)
		di->antifake_key_index = 1;
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	di->antifake_key_index = 0;
#endif
	hwlog_info("[%s] antifake_key_index = %d\n", __func__, di->antifake_key_index);

	(void)power_dts_read_u32(np, "fod_status", &di->fod_status, 0);
	(void)power_dts_read_u32(np, "pmax", &di->pmax, WLC_PMAX_DEFAULT_VAL);

	wireless_charge_parse_interfer_para(np,di);
	wireless_charge_parse_segment_para(np,di);
	wireless_charge_parse_iout_ctrl_para(np,di);

	ret = wireless_charge_parse_mode_para(np,di);
	if (ret) {
		hwlog_err("%s: get rx_mode_para failed\n", __func__);
		return -EINVAL;
	}
	ret = wireless_charge_parse_tx_prop_para(np,di);
	if (ret) {
		hwlog_err("%s: get tx_act failed\n", __func__);
		return -EINVAL;
	}
	ret = wireless_charge_parse_product_para(np,di);
	if (ret) {
		hwlog_err("%s: get product_para failed\n", __func__);
		return -EINVAL;
	}
	ret = wireless_charge_parse_volt_mode_para(np,di);
	if (ret) {
		hwlog_err("%s: get volt_mode failed\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int wireless_charge_check_ops(struct wireless_charge_device_info *di)
{
	int ret = 0;

	if (!di->ops || !di->ops->chip_init ||
		!di->ops->check_fwupdate || !di->ops->set_tx_vout ||
		!di->ops->set_rx_vout || !di->ops->get_rx_vout ||
		!di->ops->get_rx_iout || !di->ops->rx_enable ||
		!di->ops->rx_sleep_enable || !di->ops->check_tx_exist ||
		!di->ops->send_chrg_state || !di->ops->kick_watchdog ||
		!di->ops->set_rx_fod_coef || !di->ops->get_rx_fod_coef ||
		!di->ops->get_rx_chip_id || !di->ops->get_chip_info ||
		!di->ops->get_tx_id || !di->ops->get_tx_adaptor_type ||
		!di->ops->get_tx_capability || !di->ops->get_tx_fw_version ||
		!di->ops->get_tx_cert || !di->ops->send_msg_rx_vout ||
		!di->ops->send_msg_rx_iout ||
		!di->ops->send_msg_cert_confirm || !di->ops->chip_reset ||
		!di->ops->send_ept || !di->ops->pmic_vbus_handler) {
		hwlog_err("wireless_charge ops null\n");
		ret = -EINVAL;
	}

#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	if (!di->ops->rx_program_otp || !di->ops->rx_check_otp ||
		!di->ops->check_is_otp_exist) {
		hwlog_err("wireless_charge fac_ops null\n");
		ret = -EINVAL;
	}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

	return ret;
}
/*
 * There are a numerous options that are configurable on the wireless receiver
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
 #ifdef CONFIG_SYSFS
#define WIRELESS_CHARGE_SYSFS_FIELD(_name, n, m, store)	\
{					\
	.attr = __ATTR(_name, m, wireless_charge_sysfs_show, store),	\
	.name = WIRELESS_CHARGE_SYSFS_##n,		\
}
#define WIRELESS_CHARGE_SYSFS_FIELD_RW(_name, n)               \
	WIRELESS_CHARGE_SYSFS_FIELD(_name, n, S_IWUSR | S_IRUGO, wireless_charge_sysfs_store)
#define WIRELESS_CHARGE_SYSFS_FIELD_RO(_name, n)               \
	WIRELESS_CHARGE_SYSFS_FIELD(_name, n, S_IRUGO, NULL)
static ssize_t wireless_charge_sysfs_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t wireless_charge_sysfs_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count);
struct wireless_charge_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};
static struct wireless_charge_sysfs_field_info wireless_charge_sysfs_field_tbl[] = {
	WIRELESS_CHARGE_SYSFS_FIELD_RO(chip_info, CHIP_INFO),
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	WIRELESS_CHARGE_SYSFS_FIELD_RW(program_otp, PROGRAM_OTP),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(check_otp, CHECK_OTP),
#endif
	WIRELESS_CHARGE_SYSFS_FIELD_RO(tx_adaptor_type, TX_ADAPTOR_TYPE),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(rx_temp, RX_TEMP),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(vout, VOUT),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(iout, IOUT),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(vrect, VRECT),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(en_enable, EN_ENABLE),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(wireless_succ, WIRELESS_SUCC),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(normal_chrg_succ, NORMAL_CHRG_SUCC),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(fast_chrg_succ, FAST_CHRG_SUCC),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(fod_coef, FOD_COEF),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(interference_setting, INTERFERENCE_SETTING),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(rx_support_mode, RX_SUPPORT_MODE),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(nvm_data, NVM_DATA),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(die_id, DIE_ID),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(ignore_fan_ctrl, IGNORE_FAN_CTRL),
};
static struct attribute *wireless_charge_sysfs_attrs[ARRAY_SIZE(wireless_charge_sysfs_field_tbl) + 1];
static const struct attribute_group wireless_charge_sysfs_attr_group = {
	.attrs = wireless_charge_sysfs_attrs,
};
/**********************************************************
*  Function:       wireless_charge_sysfs_init_attrs
*  Discription:    initialize wireless_charge_sysfs_attrs[] for wireless_charge attribute
*  Parameters:   NULL
*  return value:  NULL
**********************************************************/
static void wireless_charge_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(wireless_charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		wireless_charge_sysfs_attrs[i] = &wireless_charge_sysfs_field_tbl[i].attr.attr;

	wireless_charge_sysfs_attrs[limit] = NULL;
}
/**********************************************************
*  Function:       wireless_charge_sysfs_field_lookup
*  Discription:    get the current device_attribute from wireless_charge_sysfs_field_tbl by attr's name
*  Parameters:   name:evice attribute name
*  return value:  wireless_charge_sysfs_field_tbl[]
**********************************************************/
static struct wireless_charge_sysfs_field_info *wireless_charge_sysfs_field_lookup(const char *name)
{
	int i, limit = ARRAY_SIZE(wireless_charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, wireless_charge_sysfs_field_tbl[i].attr.attr.name, strlen(name)))
			break;
	}
	if (i >= limit)
		return NULL;

	return &wireless_charge_sysfs_field_tbl[i];
}
/**********************************************************
*  Function:       wireless_charge_sysfs_create_group
*  Discription:    create the wireless_charge device sysfs group
*  Parameters:   di:wireless_charge_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int wireless_charge_sysfs_create_group(struct wireless_charge_device_info *di)
{
	wireless_charge_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &wireless_charge_sysfs_attr_group);
}

/**********************************************************
*  Function:       wireless_charge_sysfs_remove_group
*  Discription:    remove the wireless_charge device sysfs group
*  Parameters:   di:wireless_charge_device_info
*  return value:  NULL
**********************************************************/
static void wireless_charge_sysfs_remove_group(struct wireless_charge_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &wireless_charge_sysfs_attr_group);
}
#else
static int wireless_charge_sysfs_create_group(struct wireless_charge_device_info *di)
{
	return 0;
}
static void wireless_charge_sysfs_remove_group(struct wireless_charge_device_info *di)
{
}
#endif

/**********************************************************
*  Function:       wireless_charge_create_sysfs
*  Discription:    create the wireless_charge device sysfs group
*  Parameters:   di:wireless_charge_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int wireless_charge_create_sysfs(struct wireless_charge_device_info *di)
{
	int ret = 0;
	struct class *power_class = NULL;

	ret = wireless_charge_sysfs_create_group(di);
	if (ret) {
		hwlog_err("create sysfs entries failed!\n");
		return ret;
	}
	power_class = hw_power_get_class();
	if (power_class) {
		if (charge_dev == NULL)
			charge_dev = device_create(power_class, NULL, 0, NULL, "charger");
		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj, "wireless_charger");
		if (ret) {
			hwlog_err("create link to wireless_charge fail.\n");
			wireless_charge_sysfs_remove_group(di);
			return ret;
		}
	}

	return 0;
}

static ssize_t wireless_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct wireless_charge_sysfs_field_info *info = NULL;
	struct wireless_charge_device_info *di = g_wireless_di;
	int chrg_succ = WIRELESS_CHRG_FAIL;
	int cur_pmode_id;

	info = wireless_charge_sysfs_field_lookup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	cur_pmode_id = di->curr_pmode_index;
	switch (info->name) {
	case WIRELESS_CHARGE_SYSFS_CHIP_INFO:
		return snprintf(buf, PAGE_SIZE,
			"%s\n", di->ops->get_chip_info());
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	case WIRELESS_CHARGE_SYSFS_PROGRAM_OTP:
		hwlog_info("[%s] check otp exist\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->ops->check_is_otp_exist());
	case WIRELESS_CHARGE_SYSFS_CHECK_OTP:
		return snprintf(buf, PAGE_SIZE, "%s\n",
			di->ops->rx_check_otp() ?
			"0: otp is bad" : "1: otp is good");
#endif
	case WIRELESS_CHARGE_SYSFS_TX_ADAPTOR_TYPE:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->tx_cap->type);
	case WIRELESS_CHARGE_SYSFS_RX_TEMP:
		return snprintf(buf, PAGE_SIZE, "%d\n", wlc_get_rx_temp());
	case WIRELESS_CHARGE_SYSFS_VOUT:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->ops->get_rx_vout());
	case WIRELESS_CHARGE_SYSFS_IOUT:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->ops->get_rx_iout());
	case WIRELESS_CHARGE_SYSFS_VRECT:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->ops->get_rx_vrect());
	case WIRELESS_CHARGE_SYSFS_EN_ENABLE:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.en_enable);
	case WIRELESS_CHARGE_SYSFS_WIRELESS_SUCC:
		chrg_succ = wireless_charge_check_fac_test_succ(di);
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_NORMAL_CHRG_SUCC:
		chrg_succ = wireless_charge_check_normal_charge_succ(di);
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_FAST_CHRG_SUCC:
		chrg_succ = wireless_charge_check_fast_charge_succ(di);
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_FOD_COEF:
		return snprintf(buf, PAGE_SIZE, "%s\n",
			di->ops->get_rx_fod_coef());
	case WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING:
		return snprintf(buf, PAGE_SIZE, "%u\n",
			di->interfer_data.interfer_src_state);
	case WIRELESS_CHARGE_SYSFS_RX_SUPPORT_MODE:
		return snprintf(buf, PAGE_SIZE,
			"mode[support|current]:[0x%x|%s]\n",
			di->sysfs_data.rx_support_mode,
			di->mode_data.mode_para[cur_pmode_id].mode_name);
	case WIRELESS_CHARGE_SYSFS_NVM_DATA:
		return snprintf(buf, PAGE_SIZE, "%s\n",
				wireless_charge_read_nvm_info());
	case WIRELESS_CHARGE_SYSFS_DIE_ID:
		return snprintf(buf, PAGE_SIZE, "%s\n", wlc_get_die_id());
	case WIRELESS_CHARGE_SYSFS_IGNORE_FAN_CTRL:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.ignore_fan_ctrl);
	default:
		hwlog_err("invalid sysfs_name\n");
		break;
	}
	return 0;
}

/**********************************************************
*  Function:       wireless_charge_sysfs_store
*  Discription:    set the value for all wireless charge nodes
*  Parameters:   dev:device
*                      attr:device_attribute
*                      buf:string of node value
*                      count:unused
*  return value:  0-sucess or others-fail
**********************************************************/
static ssize_t wireless_charge_sysfs_store(struct device *dev,
				   struct device_attribute *attr, const char *buf, size_t count)
{
	struct wireless_charge_sysfs_field_info *info = NULL;
	struct wireless_charge_device_info *di = g_wireless_di;
	long val = 0;
	int ret;

	info = wireless_charge_sysfs_field_lookup(attr->attr.name);
	if (!info ||!di)
		return -EINVAL;

	switch (info->name) {
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	case WIRELESS_CHARGE_SYSFS_PROGRAM_OTP:
		if (strict_strtol(buf, 10, &val) < 0 || val != 1){
			hwlog_info("[%s] val is not valid!\n", __func__);
			return -EINVAL;
		}
		schedule_work(&di->rx_program_otp_work);
		hwlog_info("[%s] wireless rx program otp\n", __func__);
		break;
#endif
	case WIRELESS_CHARGE_SYSFS_EN_ENABLE:
		if ((strict_strtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;
		di->sysfs_data.en_enable = val;
		hwlog_info("set rx en_enable = %d\n", di->sysfs_data.en_enable);
		wireless_charge_en_enable(di->sysfs_data.en_enable);
		wlps_control(WLPS_SYSFS_EN_PWR, di->sysfs_data.en_enable ?
			WLPS_CTRL_ON : WLPS_CTRL_OFF);
		break;
	case WIRELESS_CHARGE_SYSFS_FOD_COEF:
		hwlog_info("[%s] set fod_coef:  %s\n", __func__, buf);
		ret = di->ops->set_rx_fod_coef((char*)buf);
		if (ret)
			hwlog_err("%s: set fod_coef fail\n", __func__);
		break;
	case WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING:
		hwlog_info("[%s] interference_settings:  %s", __func__, buf);
		if (strict_strtol(buf, 10, &val) < 0)
			return -EINVAL;
		wireless_charger_update_interference_settings(di, (u8)val);
		break;
	case WIRELESS_CHARGE_SYSFS_RX_SUPPORT_MODE:
		if ((strict_strtol(buf, WLC_HEXADECIMAL, &val) < 0) ||
			(val < 0) || (val > WLC_RX_SP_ALL_MODE))
			return -EINVAL;
		if (!val)
			di->sysfs_data.rx_support_mode = WLC_RX_SP_ALL_MODE;
		else
			di->sysfs_data.rx_support_mode = val;
		hwlog_info("[%s] rx_support_mode = 0x%x", __func__, val);
		break;
	case WIRELESS_CHARGE_SYSFS_NVM_DATA:
		if ((strict_strtol(buf, 10, &val) < 0) || (val < 0))
			return -EINVAL;
		di->sysfs_data.nvm_sec_no = val;
		break;
	case WIRELESS_CHARGE_SYSFS_IGNORE_FAN_CTRL:
		if ((kstrtol(buf, WLC_DECIMAL, &val) < 0) ||
			(val < 0) || (val > 1)) /* 1: ignore 0:otherwise */
			return -EINVAL;
		hwlog_info("[%s] ignore_fan_ctrl=0x%x", __func__, val);
		di->sysfs_data.ignore_fan_ctrl = val;
		break;
	default:
		hwlog_err("invalid sysfs_name\n");
		break;
	}
	return count;
}
static struct wireless_charge_device_info *wireless_charge_device_info_alloc(void)
{
	static struct wireless_charge_device_info *di;
	static struct tx_capability *tx_cap;
	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		hwlog_err("alloc di failed\n");
		return NULL;
	}
	tx_cap = kzalloc(sizeof(*tx_cap), GFP_KERNEL);
	if (!tx_cap) {
		hwlog_err("alloc tx_cap failed\n");
		goto alloc_fail_1;
	}

	di->tx_cap = tx_cap;
	return di;
alloc_fail_1:
	kfree(di);
	return NULL;
}
static void wireless_charge_device_info_free(struct wireless_charge_device_info *di)
{
	if(di) {
		if(di->tx_cap) {
			kfree(di->tx_cap);
			di->tx_cap = NULL;
		}
		if(di->iout_ctrl_data.ictrl_para) {
			kfree(di->iout_ctrl_data.ictrl_para);
			di->iout_ctrl_data.ictrl_para = NULL;
		}
		if(di->mode_data.mode_para) {
			kfree(di->mode_data.mode_para);
			di->mode_data.mode_para = NULL;
		}
		if(di->tx_prop_data.tx_prop) {
			kfree(di->tx_prop_data.tx_prop);
			di->tx_prop_data.tx_prop = NULL;
		}
		if(di->volt_mode_data.volt_mode) {
			kfree(di->volt_mode_data.volt_mode);
			di->volt_mode_data.volt_mode = NULL;
		}
		kfree(di);
		di = NULL;
	}
	g_wireless_di = NULL;
}
static void wireless_charge_shutdown(struct platform_device *pdev)
{
	int ret;
	struct wireless_charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (NULL == di) {
		hwlog_err("%s: di is null\n", __func__);
		return;
	}
	if (g_wireless_channel_state == WIRELESS_CHANNEL_ON) {
		di->pwroff_reset_flag = true;
		wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF);
		wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
		msleep(50); /* dalay 50ms for power off */
		ret = wireless_charge_set_tx_vout(ADAPTER_5V *
			WL_MVOLT_PER_VOLT);
		if (ret)
			hwlog_err("%s: wlc sw control fail\n", __func__);
	}
	cancel_delayed_work(&di->rx_sample_work);
	cancel_delayed_work(&di->wireless_ctrl_work);
	cancel_delayed_work(&di->wireless_monitor_work);
	hwlog_info("%s --\n", __func__);
}

static int wireless_charge_remove(struct platform_device *pdev)
{
	struct wireless_charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (NULL == di) {
		hwlog_err("%s: di is null\n", __func__);
		return 0;
	}

	wakeup_source_trash(&g_rx_con_wakelock);

	hwlog_info("%s --\n", __func__);

	return 0;
}
static int wireless_charge_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct wireless_charge_device_info *di = NULL;
	struct device_node *np = NULL;

	di = wireless_charge_device_info_alloc();
	if(!di) {
		hwlog_err("alloc di failed\n");
		return -ENOMEM;
	}

	g_wireless_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	di->ops = g_wireless_ops;
	platform_set_drvdata(pdev, di);
	wakeup_source_init(&g_rx_con_wakelock, "rx_con_wakelock");

	ret = wireless_charge_check_ops(di);
	if (ret)
		goto wireless_charge_fail_0;

	ret = wireless_charge_parse_dts(np, di);
	if (ret)
		goto wireless_charge_fail_0;

	di->sysfs_data.rx_support_mode = WLC_RX_SP_ALL_MODE;
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	di->sysfs_data.rx_support_mode &= ~WLC_RX_SP_SC_2_MODE;
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */
	di->sysfs_data.tx_vout_max = di->product_para.tx_vout;
	di->sysfs_data.rx_vout_max = di->product_para.rx_vout;
	di->sysfs_data.rx_iout_max = di->product_para.rx_iout;
	di->discon_delay_time = WL_DISCONN_DELAY_MS;
	wireless_charge_set_default_tx_capability(di);

	mutex_init(&g_rx_en_mutex);
	INIT_WORK(&di->wired_vbus_connect_work, wireless_charge_wired_vbus_connect_work);
	INIT_WORK(&di->wired_vbus_disconnect_work, wireless_charge_wired_vbus_disconnect_work);
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	INIT_WORK(&di->rx_program_otp_work, wireless_charge_rx_program_otp_work);
#endif
	INIT_DELAYED_WORK(&di->rx_event_work, wlc_rx_event_work);
	INIT_WORK(&di->wireless_pwroff_reset_work, wireless_charge_pwroff_reset_work);
	INIT_DELAYED_WORK(&di->wireless_ctrl_work, wireless_charge_control_work);
	INIT_DELAYED_WORK(&di->rx_sample_work, wireless_charge_rx_sample_work);
	INIT_DELAYED_WORK(&di->wireless_monitor_work, wireless_charge_monitor_work);
	INIT_DELAYED_WORK(&di->wireless_vbus_disconnect_work, wireless_charge_wireless_vbus_disconnect_work);
	INIT_DELAYED_WORK(&di->interfer_work,
		wireless_charge_interference_work);

	BLOCKING_INIT_NOTIFIER_HEAD(&di->wireless_charge_evt_nh);
	di->rx_event_nb.notifier_call = wireless_charge_rx_event_notifier_call;
	ret = blocking_notifier_chain_register(&rx_event_nh, &di->rx_event_nb);
	if (ret < 0) {
		hwlog_err("register rx_connect notifier failed\n");
		goto  wireless_charge_fail_1;
	}
	di->chrg_event_nb.notifier_call = wireless_charge_chrg_event_notifier_call;
	ret = blocking_notifier_chain_register(&charger_event_notify_head, &di->chrg_event_nb);
	if (ret < 0) {
		hwlog_err("register charger_event notifier failed\n");
		goto  wireless_charge_fail_2;
	}
	di->pwrkey_nb.notifier_call = wireless_charge_pwrkey_event_notifier_call;
	ret = hisi_powerkey_register_notifier(&di->pwrkey_nb);
	if (ret < 0) {
		hwlog_err("register power_key notifier failed\n");
		goto  wireless_charge_fail_3;
	}
	if (wireless_charge_check_tx_exist()) {
		wireless_charge_para_init(di);
		charger_source_sink_event(START_SINK_WIRELESS);
		pd_dpm_ignore_vbus_only_event(true);
		wlc_wireless_vbus_connect_handler(WIRELESS_STAGE_CHECK_TX_ID);
		schedule_delayed_work(&di->wireless_monitor_work, msecs_to_jiffies(0));
	} else {
		wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF);
	}
	init_completion(&di->wc_af_completion);
	ret = wireless_charge_create_sysfs(di);
	if (ret)
		hwlog_err("wirelss_charge create sysfs fail.\n");

	wireless_charge_gen_nl_init(pdev);
	hwlog_info("wireless_charger probe ok.\n");
	return 0;

wireless_charge_fail_3:
	blocking_notifier_chain_unregister(&charger_event_notify_head, &di->chrg_event_nb);
wireless_charge_fail_2:
	blocking_notifier_chain_unregister(&rx_event_nh, &di->rx_event_nb);
wireless_charge_fail_1:
wireless_charge_fail_0:
	wakeup_source_trash(&g_rx_con_wakelock);
	di->ops = NULL;
	wireless_charge_device_info_free(di);
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
/**********************************************************
*  Function:       wireless_charge_init
*  Description:    wireless charge module initialization
*  Parameters:   NULL
*  return value:  0-sucess or others-fail
**********************************************************/
static int __init wireless_charge_init(void)
{
	hwlog_info("wireless_charger init ok.\n");

	return platform_driver_register(&wireless_charge_driver);
}
/**********************************************************
*  Function:       wireless_charge_exit
*  Description:    wireless charge module exit
*  Parameters:   NULL
*  return value:  NULL
**********************************************************/
static void __exit wireless_charge_exit(void)
{
	platform_driver_unregister(&wireless_charge_driver);
}

device_initcall_sync(wireless_charge_init);
module_exit(wireless_charge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wireless charge module driver");
MODULE_AUTHOR("HUAWEI Inc");
