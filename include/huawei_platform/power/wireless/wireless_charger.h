/*
 * wireless_charger.h
 *
 * wireless charger driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_CHARGER_
#define _WIRELESS_CHARGER_

#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/common_module/power_common.h>

#define WLC_ERR_PARA_NULL           1
#define WLC_ERR_PARA_WRONG          2
#define WLC_ERR_MISMATCH            3
#define WLC_ERR_NO_SPACE            4
#define WLC_ERR_I2C_R               5
#define WLC_ERR_I2C_W               6
#define WLC_ERR_I2C_WR              7
#define WLC_ERR_STOP_CHRG           8
#define WLC_ERR_ACK_TIMEOUT         9
#define WLC_ERR_CHECK_FAIL          10
#define WLC_ERR_TX_EXIST            11

#define I2C_WR_MSG_LEN              1
#define I2C_RD_MSG_LEN              2
#define I2C_RETRY_CNT               3
#define BYTE_MASK                   0xff
#define WORD_MASK                   0xffff
#define BYTE_LEN                    1
#define WORD_LEN                    2

#define RX_EN_ENABLE                1
#define RX_EN_DISABLE               0
#define RX_SLEEP_EN_ENABLE          1
#define RX_SLEEP_EN_DISABLE         0
#define RX_PWR_ON_GOOD              1
#define RX_PWR_ON_NOT_GOOD          0

#define CERT_SEND_SRV_MSG_FAIL      1
#define CERT_RCV_SRV_MSG_FAIL       2

#define WIRELESS_CHRG_SUCC          0
#define WIRELESS_CHRG_FAIL          1

#define WL_MSEC_PER_SEC             1000
#define WL_MVOLT_PER_VOLT           1000
#define PERCENT                     100
#define RX_IOUT_MID                 500
#define TX_BOOST_VOUT               12000
#define TX_DEFAULT_VOUT             5000
#define RX_DEFAULT_VOUT             5500
#define RX_HIGH_VOUT                7000
#define RX_HIGH_VOUT2               12000
#define TX_REG_VOUT                 9000
#define RX_VREG_OFFSET              50
#define RX_DEFAULT_IOUT             1000
#define CHANNEL_SW_TIME             50
#define CHANNEL_SW_TIME_2           200

#define RX_HIGH_IOUT                750
#define RX_LOW_IOUT                 300
#define RX_EPT_IGNORE_IOUT          500
#define RX_AVG_IOUT_TIME            10000 /* 10s */
#define RX_VRECT_LOW_RESTORE_TIME   10000
#define RX_VRECT_LOW_IOUT_MIN       300
#define RX_VRECT_LOW_CNT            3
#define RX_VOUT_ERR_CHECK_TIME      1000
#define RX_BST_DELAY_TIME           3000 /* 3s */

#define WLRX_ILIM_DELAY             300 /* delay for ibus stability */

#define TX_ID_HW                    0x8866

#define CONTROL_INTERVAL_NORMAL     300
#define CONTROL_INTERVAL_FAST       100
#define MONITOR_INTERVAL            100
#define MONITOR_LOG_INTERVAL        5000
#define WL_DISCONN_DELAY_MS         1600
#define WL_RST_DISCONN_DELAY_MS     3000

#define RX_IOUT_SAMPLE_LEN          10
#define WIRELESS_STAGE_STR_LEN      32
#define WIRELESS_TMP_STR_LEN        16

#define TX_ID_ERR_CNT_MAX           3
#define TX_ABILITY_ERR_CNT_MAX      3
#define CERTI_ERR_CNT_MAX           3
#define BOOST_ERR_CNT_MAX           5
/* Qi: if reset more than twice, tx will end power transfer */
#define WLC_RST_CNT_MAX             2

#define RX_OCP_CNT_MAX              3
#define RX_OVP_CNT_MAX              3
#define RX_OTP_CNT_MAX              3

#define SET_CURRENT_LIMIT_STEP      100
#define RX_SAMPLE_WORK_DELAY        500
#define SERIALNO_LEN                16
#define WIRELESS_RX_KEY_LEN         8

/* rx charge state */
#define WIRELESS_STATE_CHRG_FULL            BIT(0)
#define WIRELESS_STATE_CHRG_DONE            BIT(1)

#define WLC_ICTRL_PARA_LEVEL                15
#define WIRELESS_CHIP_INIT                  0
#define WILREESS_SC_CHIP_INIT               1

#define WIRELESS_INT_CNT_TH                 10
#define WIRELESS_INT_TIMEOUT_TH             15000 /* 15 * 1000ms */

#define WIRELESS_NORMAL_CHARGE_FLAG         0
#define WIRELESS_FAST_CHARGE_FLAG           1
#define WIRELESS_SUPER_CHARGE_FLAG          2

#define WIRELESS_CHECK_UNKNOWN              (-1)
#define WIRELESS_CHECK_FAIL                 0
#define WIRELESS_CHECK_SUCC                 1

#define WAIT_AF_SRV_RTY_CNT                 3
#define WC_AF_INFO_NL_OPS_NUM               1
#define WC_AF_WAIT_CT_TIMEOUT               2000
#define WC_AF_TOTAL_KEY_NUM                 11

#define WLC_OTP_PROGRAMED                   1
#define WLC_OTP_NON_PROGRAMED               0
#define WLC_OTP_ERR_PROGRAMED               2

#define WLC_PROGRAM_OTP                     0
#define WLC_RECOVER_OTP                     1

/* attention: mode type should not be modified */
#define WLC_RX_SP_BUCK_MODE                 BIT(0)
#define WLC_RX_SP_SC_2_MODE                 BIT(1)
#define WLC_RX_SP_SC_4_MODE                 BIT(2)
#define WLC_RX_SP_ALL_MODE                  0xff
/* back device color nv info */
#define BACK_DEVICE_COLOR_LEN               16

/* cmd: send charge state */
#define WLC_SEND_CHARGE_STATE_RETRY_CNT     3

#define WLC_FAN_HALF_SPEED_MAX_QI           0x00
#define WLC_FAN_FULL_SPEED_MAX_QI           0x01
#define WLC_FAN_FULL_SPEED_QI               0x30
#define WLC_FAN_UNKNOWN_SPEED               0
#define WLC_FAN_HALF_SPEED_MAX              1
#define WLC_FAN_FULL_SPEED_MAX              2
#define WLC_FAN_HALF_SPEED                  3
#define WLC_FAN_FULL_SPEED                  4
#define WLC_FAN_LIMIT_RETRY_CNT             3
#define WLC_FAN_CTRL_PWR                    10000000 /* 10w */

#define WLC_THERMAL_FORCE_FAN_FULL_SPEED    BIT(0)
#define WLC_THERMAL_EXIT_SC_MODE            BIT(1)

/* cmd: set tx rpp format */
#define WLC_SET_RPP_FORMAT_RETRY_CNT        3
#define WLRX_SET_RPP_FORMAT_RETRY_DELAY     100

#define WLC_TIME_PARA_LEVEL                 6
#define WLC_TEMP_PARA_LEVEL                 3
#define WLC_START_MON_TIME_TH               10

#define WLC_NO_NEED_MON_IMAX                (-1)
#define WLC_NEED_MON_IMAX                   1
#define WLC_FIRST_MON_IMAX                  2
#define WLC_NON_FIRST_MON_IMAX              3

#define WLC_HIGH_PWR_TH_FOR_WLTX            27000

enum tx_power_state {
	TX_POWER_GOOD_UNKNOWN = 0,
	TX_POWER_GOOD,
	TX_NOT_POWER_GOOD,  /* weak source tx */
};

enum wireless_mode {
	WIRELESS_RX_MODE = 0,
	WIRELESS_TX_MODE,
};

enum wireless_etp_type {
	WIRELESS_EPT_UNKOWN         = 0x00,
	WIRELESS_EPT_CHRG_COMPLETE  = 0x01,
	WIRELESS_EPT_INTERNAL_FAULT = 0x02,
	WIRELESS_EPT_OTP            = 0x03,
	WIRELESS_EPT_OVP            = 0x04,
	WIRELESS_EPT_OCP            = 0x05,
	WIRELESS_EPT_BATT_FAILURE   = 0x06,
	WIRELESS_EPT_RESERVED       = 0x07,
	WIRELESS_EPT_NO_RESPONSE    = 0x08,
	WIRELESS_EPT_ERR_VRECT      = 0xA0,
	WIRELESS_EPT_ERR_VOUT       = 0xA1,
};

enum tx_cap_info {
	TX_CAP_TYPE = 1,
	TX_CAP_VOUT_MAX,
	TX_CAP_IOUT_MAX,
	TX_CAP_ATTR,
	TX_CAP_TOTAL,
};

enum wireless_iout_ctrl_info {
	WLC_ICTRL_BEGIN = 0,
	WLC_ICTRL_IOUT_MIN = WLC_ICTRL_BEGIN,
	WLC_ICTRL_IOUT_MAX,
	WLC_ICTRL_IOUT_SET,
	WLC_ICTRL_TOTAL,
};

struct wireless_iout_ctrl_para {
	int iout_min;
	int iout_max;
	int iout_set;
};

struct wireless_iout_ctrl_data {
	int ictrl_para_level;
	struct wireless_iout_ctrl_para *ictrl_para;
};

enum wlc_time_info {
	WLC_TIME_INFO_TIME_TH,
	WLC_TIME_INFO_IOUT_MAX,
	WLC_TIME_INFO_MAX
};

struct wlc_time_para {
	int time_th;
	int iout_max;
};

enum wlc_temp_info {
	WLC_TEMP_INFO_TEMP_LTH,
	WLC_TEMP_INFO_TEMP_HTH,
	WLC_TEMP_INFO_IOUT_MAX,
	WLC_TEMP_INFO_TEMP_BACK,
	WLC_TEMP_INFO_MAX
};

struct wlc_temp_para {
	int temp_lth;
	int temp_hth;
	int iout_max;
	int temp_back;
};

enum wireless_tx_prop_info {
	WLC_TX_ADAPTOR_TYPE = 0,
	WLC_TX_TYPE_NAME,
	WLC_TX_NEED_CABLE_DETECT,
	WLC_TX_NEED_CERT,
	WLC_TX_DEFAULT_VOUT,
	WLC_TX_DEFAULT_IOUT,
	WLC_TX_PROP_TOTAL,
};

enum af_srv_state {
	AF_SRV_NOT_READY = 0,
	AF_SRV_NO_RESPONSE,
	AF_SRV_SUCC,
};

struct wireless_charge_sysfs_data {
	int en_enable;
	int nvm_sec_no;
	int ignore_fan_ctrl;
	u8 rx_support_mode;
	u8 thermal_ctrl;
};

struct wlc_state_record {
	u8 fan_cur;
	u8 fan_last;
	u8 chrg_state_cur;
	u8 chrg_state_last;
};

struct wlc_flag {
	int mon_imax;
};

struct wlc_rx_evt {
	int type;
	int pwr_good;
	struct wireless_protocol_tx_alarm tx_alarm;
};

struct wlrx_pctrl;

struct wlrx_dev_info {
	struct device *dev;
	struct notifier_block rx_event_nb;
	struct notifier_block chrg_event_nb;
	struct notifier_block pwrkey_nb;
	struct work_struct wired_vbus_connect_work;
	struct work_struct wired_vbus_disconnect_work;
	struct delayed_work rx_event_work;
	struct work_struct wireless_pwroff_reset_work;
	struct delayed_work wireless_vbus_disconnect_work;
	struct delayed_work wireless_ctrl_work;
	struct delayed_work wireless_monitor_work;
	struct delayed_work wireless_watchdog_work;
	struct delayed_work rx_sample_work;
	struct delayed_work ignore_qval_work;
	struct wireless_protocol_tx_cap *tx_cap;
	struct wireless_protocol_tx_fop_range tx_fop_range;
	struct wireless_charge_sysfs_data sysfs_data;
	struct wireless_iout_ctrl_data iout_ctrl_data;
	struct wlc_state_record stat_rcd;
	struct wlc_temp_para temp_para[WLC_TEMP_PARA_LEVEL];
	struct wlc_time_para time_para[WLC_TIME_PARA_LEVEL];
	struct wlc_flag flag;
	struct wlc_rx_evt rx_evt;
	struct wlrx_pctrl *pctrl;
	int tx_type;
	int standard_tx;
	int tx_vout_max;
	int rx_iout_max;
	int rx_vout_max;
	int supported_rx_vout;
	int discon_delay_time;
	int ctrl_interval;
	int monitor_interval;
	int tx_id_err_cnt;
	int tx_ability_err_cnt;
	int certi_err_cnt;
	int certi_comm_err_cnt;
	int wlc_err_rst_cnt;
	int boost_err_cnt;
	int rx_iout_limit;
	int iout_avg;
	int iout_high_cnt;
	int iout_low_cnt;
	int cable_detect_succ_flag;
	int cert_succ_flag;
	int curr_tx_type_index;
	int curr_pmode_index;
	int curr_vmode_index;
	int curr_icon_type;
	unsigned long curr_power_time_out;
	int pwroff_reset_flag;
	int extra_pwr_good_flag;
	unsigned long icon_pmode; /* check icon type by icon_pmode */
	u8 qval_support_mode;
};

enum wireless_charge_sysfs_type {
	WIRELESS_CHARGE_SYSFS_CHIP_INFO = 0,
	WIRELESS_CHARGE_SYSFS_KEY_ID,
	WIRELESS_CHARGE_SYSFS_TX_ADAPTOR_TYPE,
	WIRELESS_CHARGE_SYSFS_RX_TEMP,
	WIRELESS_CHARGE_SYSFS_VOUT,
	WIRELESS_CHARGE_SYSFS_IOUT,
	WIRELESS_CHARGE_SYSFS_VRECT,
	WIRELESS_CHARGE_SYSFS_EN_ENABLE,
	WIRELESS_CHARGE_SYSFS_NORMAL_CHRG_SUCC,
	WIRELESS_CHARGE_SYSFS_FAST_CHRG_SUCC,
	WIRELESS_CHARGE_SYSFS_FOD_COEF,
	WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING,
	WIRELESS_CHARGE_SYSFS_RX_SUPPORT_MODE,
	WIRELESS_CHARGE_SYSFS_THERMAL_CTRL,
	WIRELESS_CHARGE_SYSFS_NVM_DATA,
	WIRELESS_CHARGE_SYSFS_IGNORE_FAN_CTRL,
};

enum wltx_stage wireless_tx_get_stage(void);
void wireless_charge_wired_vbus_connect_handler(void);
void wireless_charge_wired_vbus_disconnect_handler(void);
void direct_charger_disconnect_event(void);
int wireless_charge_get_rx_iout_limit(void);
void wireless_charger_pmic_vbus_handler(bool vbus_state);

void wireless_charge_chip_init(int tx_vset);
int wireless_charge_select_vout_mode(int vout);
int wldc_set_trx_vout(int vout);
int wireless_charge_set_rx_vout(int rx_vout);
void wireless_charge_get_tx_adaptor_type(u8 *type);

struct wireless_protocol_tx_cap *wlc_get_tx_cap(void);
int wlc_get_rx_support_mode(void);
int wlc_get_rx_max_iout(void);
int wlc_get_pmode_id_by_mode_name(const char *mode_name);
int wlc_get_expected_pmode_id(int pmode_id);
void wlc_set_cur_pmode_id(int pmode_id);
void wlc_clear_icon_pmode(int pmode);
void wireless_charge_icon_display(int crit_type);

int wireless_charge_get_rx_avg_iout(void);
int wlc_get_cp_avg_iout(void);
void wireless_charge_restart_charging(unsigned int stage_from);
bool wireless_charge_mode_judge_criterion(int pmode_index, int crit_type);
int wireless_charge_get_power_mode(void);
void wireless_charge_update_max_vout_and_iout(bool ignore_cnt_flag);

void wlc_ignore_vbus_only_event(bool ignore_flag);
int wlc_parse_dts(struct device_node *np, struct wlrx_dev_info *di);

#ifdef CONFIG_WIRELESS_CHARGER
void wlc_set_high_pwr_test_flag(bool flag);
bool wlc_get_high_pwr_test_flag(void);
void wlc_reset_wireless_charge(void);
#else
static inline void wlc_set_high_pwr_test_flag(bool flag)
{
}

static inline bool wlc_get_high_pwr_test_flag(void)
{
	return false;
}

static inline void wlc_reset_wireless_charge(void)
{
}

#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_CHARGER_ */
