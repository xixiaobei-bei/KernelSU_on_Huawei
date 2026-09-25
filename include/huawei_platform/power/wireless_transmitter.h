/*
 * wireless_transmitter.h
 *
 * wireless reverse charging driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_TRANSMITTER_H_
#define _WIRELESS_TRANSMITTER_H_

#include <huawei_platform/power/wlc_qi.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/hw_accessory.h>

#define WL_TX_FAIL                       (-1)
#define WL_TX_SUCC                       0
#define WL_TX_MONITOR_INTERVAL           300

#define WL_TX_SOC_MIN                    20
#define WL_TX_BATT_TEMP_MAX              50
#define WL_TX_TBATT_DELTA_TH             15
#define WL_TX_BATT_TEMP_MIN              (-10)
#define WL_TX_BATT_TEMP_RESTORE          35
#define WL_TX_STR_LEN_16                 16
#define WL_TX_STR_LEN_32                 32

#define WLTX_TX_VSET_TYPE_MAX            3
#define WLTX_TX_VSET_SOC_TH1             20
#define WLTX_TX_VSET_SOC_TH2             25
#define WLTX_TX_VSET_TBAT_TH1            40
#define WLTX_TX_VSET_TBAT_TH2            45
#define WLTX_TX_VSET_FOP_TH1             115
#define WLTX_TX_VSET_FOP_TH2             130
#define WLTX_TX_VSET_IIN_TH1             400
#define WLTX_TX_VSET_IIN_TH2             600
#define WLTX_TX_VSET_5V                  5000

/* high power cfg */
#define WL_TX_HI_PWR_SOC_MIN             90
#define WL_TX_HI_PWR_TBATT_MAX           37
#define WLTX_HI_PWR_TIME                 300000 /* 5min: 5*60*1000 */
#define WLTX_HI_PWR_HS_TIME              15000 /* 15s, handshake time */
#define WLTX_HI_PWR_ILIM                 1150
#define WLTX_RX_HIGH_VOUT                7000

#define WLTX_IRQ_VSET_TIME               5000 /* 5s */

#define WLTX_VBATT_LTH                   3450
#define WLTX_IBATT_HTH                   5500
#define WLTX_PWR_CHK_SLEEP               20 /* ms */
#define WLTX_PWR_CHK_CNT                 3

#define WL_TX_VIN_RETRY_CNT1             10
#define WL_TX_VIN_RETRY_CNT2             250
#define WL_TX_VIN_SLEEP_TIME             20
#define WL_TX_IIN_SAMPLE_LEN             10
#define WL_TX_FOP_SAMPLE_LEN             10
#define WL_TX_PING_TIMEOUT_1             120
#define WL_TX_PING_TIMEOUT_2             119
#define WL_TX_PING_CHECK_INTERVAL        20
#define WL_TX_PING_VIN_OVP_CNT           3
#define WL_TX_PING_VIN_UVP_CNT           10 /* 6s-10s */
#define WL_TX_I2C_ERR_CNT                5
#define WL_TX_IIN_LOW                    300 /* mA */
#define WL_TX_IIN_LOW_CNT                1800000 /* 30min: 30*60*1000 */
#define WL_TX_IIN_LOW_AUX                400 /* mA */
#define WL_TX_IIN_LOW_CNT_AUX            3600000 /* 60min: 60*60*1000 */
#define WL_TX_MODE_ERR_CNT               10
#define WL_TX_MODE_ERR_CNT1              3
#define WL_TX_MODE_ERR_CNT2              20
#define WL_TX_5VBST_MIN_FOP              138 /* kHz */

#define WL_TX_MONITOR_LOG_INTERVAL       3000
#define WL_TX_CHARGER_TYPE_MAX           12

#define WL_TX_DMDLOG_SIZE                512
#define WL_TX_ACC_DEV_MAC_LEN            6
#define WL_TX_ACC_DEV_MODELID_LEN        3
#define WL_TX_ACC_DEV_SUBMODELID_LEN     1
#define WL_TX_ACC_DEV_VERSION_LEN        1
#define WL_TX_ACC_DEV_BUSINESS_LEN       1
#define WL_TX_ACC_DEV_INFO_CNT           12

/* rp demodulation timeout cfg */
#define WLTX_RP_DEMODULE_TIMEOUT_VAL     60

enum wireless_tx_acc_info_index {
	WL_TX_ACC_INFO_BEGIN = 0,
	WL_TX_ACC_INFO_NO = WL_TX_ACC_INFO_BEGIN,
	WL_TX_ACC_INFO_STATE,
	WL_TX_ACC_INFO_MAC,
	WL_TX_ACC_INFO_MODEL_ID,
	WL_TX_ACC_INFO_SUBMODEL_ID,
	WL_TX_ACC_INFO_VERSION,
	WL_TX_ACC_INFO_BUSINESS,
	WL_TX_ACC_INFO_MAX,
};

enum wireless_tx_pwr_sw_scene {
	PWR_SW_SCN_MIN = 0x00,
	PWR_SW_BY_VBUS_ON = PWR_SW_SCN_MIN,
	PWR_SW_BY_VBUS_OFF,
	PWR_SW_BY_OTG_ON,
	PWR_SW_BY_OTG_OFF,
	PWR_SW_SCN_MAX,
};

enum wireless_tx_power_src {
	PWR_SRC_NULL = 0x00,
	PWR_SRC_VBUS,
	PWR_SRC_OTG,
	PWR_SRC_5VBST,
	PWR_SRC_SPBST, /* Specialized boost */
	PWR_SRC_NA,
	PWR_SRC_MAX,
};

enum wireless_tx_power_type {
	WL_TX_PWR_VBUS_OTG = 0x00,
	WL_TX_PWR_5VBST_OTG,
	WL_TX_PWR_SPBST, /* Specialized boost */
	WL_TX_PWR_5VBST,
	WL_TX_PWR_MAX,
};

enum wireless_tx_status_type {
	/* tx normal status */
	WL_TX_STATUS_DEFAULT = 0x00,    /* default */
	WL_TX_STATUS_PING,              /* TX ping RX */
	WL_TX_STATUS_PING_SUCC,         /* ping success */
	WL_TX_STATUS_IN_CHARGING,       /* in reverse charging status */
	WL_TX_STATUS_CHARGE_DONE,       /* RX charge done */
	/* tx error status */
	WL_TX_STATUS_FAULT_BASE = 0x10, /* base */
	WL_TX_STATUS_TX_CLOSE,          /* chip fault or user close TX mode */
	WL_TX_STATUS_RX_DISCONNECT,     /* monitor RX disconnect */
	WL_TX_STATUS_PING_TIMEOUT,      /* can not ping RX */
	WL_TX_STATUS_TBATT_LOW,         /* battery temperature too low */
	WL_TX_STATUS_TBATT_HIGH,        /* battery temperature too high */
	WL_TX_STATUS_IN_WL_CHARGING,    /* in wireless charging */
	WL_TX_STATUS_SOC_ERROR,         /* capacity is too low */
	WL_TX_STATUS_TX_INIT,           /* tx init by wdt reset */
};

enum wireless_tx_event_type {
	WL_TX_EVENT_GET_CFG,
	WL_TX_EVENT_HANDSHAKE_SUCC,
	WL_TX_EVENT_CHARGEDONE,
	WL_TX_EVENT_CEP_TIMEOUT,
	WL_TX_EVENT_EPT_CMD,
	WL_TX_EVENT_OVP,
	WL_TX_EVENT_OCP,
	WL_TX_EVENT_PING_RX,
	WL_TX_EVENT_HALL_APPROACH,
	WL_TX_EVENT_HALL_AWAY_FROM,
	WL_TX_EVENT_ACC_DEV_CONNECTD,
	WLTX_EVT_RCV_DPING,
	WLTX_EVT_ASK_SET_VTX,
	WLTX_EVT_GET_TX_CAP,
	WLTX_EVT_TX_FOD,
	WLTX_EVT_RP_DM_TIMEOUT,
	WLTX_EVT_TX_INIT,
	WLTX_EVT_TX_AP_ON,
	WLTX_EVT_IRQ_SET_VTX,
};

enum wireless_tx_stage {
	WL_TX_STAGE_DEFAULT = 0,
	WL_TX_STAGE_POWER_SUPPLY,
	WL_TX_STAGE_CHIP_INIT,
	WL_TX_STAGE_PING_RX,
	WL_TX_STAGE_REGULATION,
	WL_TX_STAGE_TOTAL,
};

enum wireless_tx_sysfs_type {
	WL_TX_SYSFS_TX_OPEN = 0,
	WL_TX_SYSFS_TX_STATUS,
	WL_TX_SYSFS_TX_IIN_AVG,
	WL_TX_SYSFS_DPING_FREQ,
	WL_TX_SYSFS_DPING_INTERVAL,
	WL_TX_SYSFS_MAX_FOP,
	WL_TX_SYSFS_MIN_FOP,
	WL_TX_SYSFS_TX_FOP,
	WL_TX_SYSFS_HANDSHAKE,
	WL_TX_SYSFS_CHK_TRXCOIL,
	WL_TX_SYSFS_TX_VIN,
	WL_TX_SYSFS_TX_IIN,
};

enum wltx_acc_dev_state {
	WL_ACC_DEV_STATE_BEGIN = 0,
	WL_ACC_DEV_STATE_OFFLINE = WL_ACC_DEV_STATE_BEGIN,
	WL_ACC_DEV_STATE_ONLINE,
	WL_ACC_DEV_STATE_PING_SUCC,
	WL_ACC_DEV_STATE_END,
};

enum wltx_stage_vset_info {
	WLTX_TX_STAGE_VPS = 0,
	WLTX_TX_STAGE_VPING,
	WLTX_TX_STAGE_VHS,
	WLTX_TX_STAGE_VDFLT,
	WLTX_TX_STAGE_VTOTAL
};

enum wltx_vset_info {
	WLTX_RX_VSET_MIN = 0,
	WLTX_RX_VSET_MAX,
	WLTX_TX_VSET,
	WLTX_TX_VSET_LTH,
	WLTX_TX_VSET_HTH,
	WLTX_TX_PLOSS_TH,
	WLTX_TX_PLOSS_CNT,
	WLTX_TX_VSET_TOTAL,
};

enum wltx_cap_level {
	WLTX_DFLT_CAP = 0,
	WLTX_HIGH_PWR_CAP,
	WLTX_TOTAL_CAP,
};

struct wltx_vset_data {
	u32 rx_vmin;
	u32 rx_vmax;
	int vset;
	u32 lth;
	u32 hth;
	u32 pl_th;
	u8 pl_cnt;
};

struct wltx_vset_para {
	int v_ps; /* power supply */
	int v_ping;
	int v_hs; /* handshake */
	int v_dflt;
	int cur;
	int total;
	int max_vset;
	struct wltx_vset_data para[WLTX_TX_VSET_TYPE_MAX];
};

struct wltx_cap_data {
	int exp_id; /* expect cap index */
	int cur_id;
	int cap_level;
	struct wlc_tx_cap cap_para[WLTX_TOTAL_CAP];
};

struct wireless_tx_device_ops {
	void (*pre_ps)(bool);
	int (*chip_reset)(void);
	void (*rx_enable)(int);
	void (*rx_sleep_enable)(int);
	int (*check_fwupdate)(enum wireless_mode);
	int (*kick_watchdog)(void);
	int (*tx_chip_init)(void);
	int (*tx_stop_config)(void);
	int (*enable_tx_mode)(bool);
	int (*get_tx_iin)(u16 *);
	int (*get_tx_vin)(u16 *);
	int (*get_tx_vrect)(u16 *);
	int (*get_chip_temp)(u8 *);
	int (*get_tx_fop)(u16 *);
	int (*set_tx_ilimit)(int);
	int (*set_tx_max_fop)(u16);
	int (*get_tx_max_fop)(u16 *);
	int (*set_tx_min_fop)(u16);
	int (*get_tx_min_fop)(u16 *);
	int (*set_tx_ping_frequency)(u16);
	int (*get_tx_ping_frequency)(u16 *);
	int (*set_tx_ping_interval)(u16);
	int (*get_tx_ping_interval)(u16 *);
	bool (*check_rx_disconnect)(void);
	bool (*in_tx_mode)(void);
	void (*set_tx_open_flag)(bool);
	int (*set_tx_fod_coef)(u32, u8);
	void (*set_rp_dm_timeout_val)(u8);
	void (*activate_tx_chip)(void);
};

struct wltx_acc_dev {
	u8 dev_state;
	u8 dev_no;
	u8 dev_mac[WL_TX_ACC_DEV_MAC_LEN];
	u8 dev_model_id[WL_TX_ACC_DEV_MODELID_LEN];
	u8 dev_submodel_id;
	u8 dev_version;
	u8 dev_business;
	u8 dev_info_cnt;
};

struct wireless_tx_device_info {
	struct device *dev;
	struct notifier_block tx_event_nb;
	struct work_struct wireless_tx_check_work;
	struct work_struct wireless_tx_evt_work;
	struct delayed_work wireless_tx_monitor_work;
	struct wireless_tx_device_ops *tx_ops;
	struct wltx_vset_para tx_vset;
	struct wltx_cap_data tx_cap;
	enum wireless_tx_power_type pwr_type;
	enum wireless_tx_power_src cur_pwr_src;
	enum wireless_tx_pwr_sw_scene cur_pwr_sw_scn;
	unsigned int tx_vset_tbat_high;
	unsigned int tx_vset_tbat_low;
	unsigned int monitor_interval;
	unsigned int ping_timeout;
	unsigned int tx_event_type;
	unsigned int tx_event_data;
	unsigned int tx_iin_avg;
	unsigned int tx_fop_avg;
	unsigned int i2c_err_cnt;
	unsigned int tx_mode_err_cnt;
	unsigned int tx_iin_low_cnt;
	unsigned long hs_time_out; /* hs: handshake */
	unsigned long hp_time_out; /* hp: high power */
	unsigned long irq_vset_time_out;
	unsigned int tx_rp_timeout_lim_volt;
	bool tx_pd_flag; /* tx power down by itself */
	bool standard_rx;
	bool stop_reverse_charge; /* record driver state */
	int gpio_tx_boost_en;
	struct qi_protocol_acc_device_info *wireless_tx_acc;
};

extern struct blocking_notifier_head tx_event_nh;
extern int wireless_tx_ops_register(struct wireless_tx_device_ops *ops);
extern void wireless_tx_cancel_work(enum wireless_tx_pwr_sw_scene);
extern void wireless_tx_restart_check(enum wireless_tx_pwr_sw_scene);
extern int wireless_tx_get_tx_iin_limit(enum huawei_usb_charger_type);
extern bool wltx_need_disable_wired_dc(void);

#ifdef CONFIG_WIRELESS_CHARGER
bool wireless_is_in_tx_mode(void);
bool wireless_tx_get_tx_open_flag(void);
void wltx_reset_reverse_charging(void);
void wireless_tx_event_notify(unsigned long e, void *v);
#else
static inline bool wireless_is_in_tx_mode(void)
{
	return false;
}

static inline bool wireless_tx_get_tx_open_flag(void)
{
	return false;
}
extern struct blocking_notifier_head tx_event_nh;
static inline void wltx_reset_reverse_charging(void) {}
static inline void wireless_tx_event_notify(unsigned long e, void *v) {}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TRANSMITTER_H_ */
