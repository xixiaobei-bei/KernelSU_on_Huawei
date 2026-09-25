/*
 * wireless_transmitter_aux.h
 *
 * wireless aux tx reverse charging
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/hw_accessory.h>

struct wltx_dev_ops {
	int (*chip_reset)(void);
	void (*rx_enable)(int);
	void (*rx_sleep_enable)(int);
	int (*check_fwupdate)(enum wireless_mode);
	int (*kick_watchdog)(void);
	int (*tx_chip_init)(void);
	int (*tx_stop_config)(void);
	int (*enable_tx_mode)(bool);
	int (*enable_tx_ping)(void);
	int (*get_tx_iin)(u16 *);
	int (*get_tx_vin)(u16 *);
	int (*get_tx_vrect)(u16 *);
	int (*get_chip_temp)(u8 *);
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
	enum wireless_tx_power_type pwr_type;
	enum wireless_tx_power_src cur_pwr_src;
	enum wireless_tx_pwr_sw_scene cur_pwr_sw_scn;
	unsigned int monitor_interval;
	unsigned int ping_timeout;
	unsigned int tx_event_type;
	unsigned int tx_event_data;
	unsigned int tx_iin_avg;
	unsigned int i2c_err_cnt;
	unsigned int tx_mode_err_cnt;
	unsigned int tx_iin_low_cnt;
	bool standard_rx;
	bool stop_reverse_charge; /* record driver state */
	int gpio_tx_boost_en;
	struct wltx_acc_dev *wireless_tx_acc;
};

extern struct blocking_notifier_head tx_aux_event_nh;
extern int wltx_aux_ops_register(struct wltx_dev_ops *ops);

#endif /* _WIRELESS_TRANSMITTER_AUX_H_ */
