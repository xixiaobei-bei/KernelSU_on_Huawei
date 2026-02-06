/*
 * wireless_transmitter_aux.h
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

#ifndef _WIRELESS_TRANSMITTER_AUX_H_
#define _WIRELESS_TRANSMITTER_AUX_H_

#include <chipset_common/hwpower/wireless_charge/wireless_accessory.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/huawei_charger.h>

#define WL_TX_FAIL                       (-1)
#define WL_TX_SUCC                       0
#define WL_TX_MONITOR_INTERVAL           300
#define WL_TX_MONITOR_LOG_INTERVAL       3000
#define WL_TX_BATT_TEMP_MAX              50
#define WL_TX_BATT_TEMP_MIN              (-10)
#define WL_TX_TBATT_DELTA_TH             15
#define WL_TX_STR_LEN_32                 32
#define WL_TX_VIN_RETRY_CNT2             250
#define WL_TX_VIN_SLEEP_TIME             20
#define WL_TX_IIN_SAMPLE_LEN             10
#define WL_TX_PING_TIMEOUT_1             30
#define WL_TX_PING_TIMEOUT_2             29
#define WL_TX_PING_CHECK_INTERVAL        20
#define WL_TX_PING_VIN_UVP_CNT           10 /* 6s-10s */
#define WL_TX_I2C_ERR_CNT                5
#define WL_TX_REVERSE_TIMEOUT_CNT        7200000 /* 120min: 120*60*1000 */
#define WL_TX_MODE_ERR_CNT               10
#define WL_TX_MODE_ERR_CNT2              20
#define WL_TX_ACC_DEV_MAC_LEN            6
#define WL_TX_ACC_DEV_MODELID_LEN        3
#define WL_TX_ACC_DEV_SUBMODELID_LEN     1
#define WL_TX_ACC_DEV_VERSION_LEN        1
#define WL_TX_ACC_DEV_BUSINESS_LEN       1
#define WL_TX_ACC_DEV_INFO_CNT           12
#define WLTX_TX_VSET_TYPE_MAX            3
#define WLTX_TX_VSET_DEFAULT_LEVEL       2
#define WLTX_TX_VSET_STAGE               4
#define WLTX_VSET_ROW                    2
#define WLTX_VSET_COL                    3

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
	WL_TX_STATUS_SOC_ERROR,         /* capacity is too low */
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
	int cur_vset;
	struct wltx_vset_data para[WLTX_TX_VSET_TYPE_MAX];
};

struct iin_vset_para {
	unsigned int iin_min;
	unsigned int iin_max;
	int vset;
};

struct iin_vset_cfg {
	int vset_level;
	struct iin_vset_para vset_para[WLTX_VSET_ROW];
};

struct wltx_dev_ops {
	int (*chip_reset)(void);
	void (*rx_enable)(int);
	int (*check_fwupdate)(enum wireless_mode);
	int (*tx_chip_init)(void);
	int (*tx_stop_config)(void);
	int (*enable_tx_mode)(bool);
	int (*enable_tx_ping)(void);
	int (*get_tx_iin)(u16 *);
	int (*get_tx_vin)(u16 *);
	int (*get_tx_vrect)(u16 *);
	int (*get_chip_temp)(u8 *);
	int (*tx_adjust_vin)(u16);
	int (*get_tx_fop)(u16 *);
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
	int (*get_tx_acc_dev_no)(u8 *);
	int (*get_tx_acc_dev_state)(u8 *);
	int (*get_tx_acc_dev_mac)(u8 *);
	int (*get_tx_acc_dev_model_id)(u8 *);
	int (*get_tx_acc_dev_submodel_id)(u8 *);
	int (*get_tx_acc_dev_version)(u8 *);
	int (*get_tx_acc_dev_business)(u8 *);
	int (*set_tx_acc_dev_state)(u8);
	int (*get_tx_acc_dev_info_cnt)(u8 *);
	int (*set_tx_acc_dev_info_cnt)(u8);
};

struct wltx_aux_dev_info {
	struct device *dev;
	struct notifier_block tx_event_nb;
	struct work_struct wltx_check_work;
	struct work_struct wltx_evt_work;
	struct delayed_work wltx_aux_monitor_work;
	struct workqueue_struct *aux_tx_wq;
	struct delayed_work hall_approach_work;
	struct delayed_work hall_away_work;
	struct wltx_dev_ops *tx_ops;
	struct wltx_vset_para tx_vset;
	unsigned int monitor_interval;
	unsigned int monitor_delay;
	unsigned int ping_timeout;
	unsigned int tx_event_type;
	unsigned int tx_event_data;
	unsigned int tx_iin_avg;
	unsigned int i2c_err_cnt;
	unsigned int tx_mode_err_cnt;
	unsigned int tx_reverse_timeout_cnt;
	bool standard_rx;
	bool stop_reverse_charge; /* record driver state */
	int gpio_tx_boost_en;
	unsigned int ping_timeout_1;
	unsigned int ping_timeout_2;
	struct wltx_acc_dev *wireless_tx_acc;
	struct iin_vset_cfg vset_cfg;
	unsigned int pwm_support;
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

extern int wltx_aux_ops_register(struct wltx_dev_ops *ops);

#endif /* _WIRELESS_TRANSMITTER_AUX_H_ */
