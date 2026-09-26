/*
 * tcpm_notifier.h
 *
 * tcpm notifier
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

#ifndef _TCPM_NOTIFIER_H_
#define _TCPM_NOTIFIER_H_

#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include "tcpm_pd.h"

enum {
	TCP_NOTIFY_IDX_MODE = 0,
	TCP_NOTIFY_IDX_VBUS,
	TCP_NOTIFY_IDX_USB,
	TCP_NOTIFY_IDX_MISC,
	TCP_NOTIFY_IDX_NR,
};

#define TCP_NOTIFY_TYPE_MODE  (1 << TCP_NOTIFY_IDX_MODE)
#define TCP_NOTIFY_TYPE_VBUS  (1 << TCP_NOTIFY_IDX_VBUS)
#define TCP_NOTIFY_TYPE_USB   (1 << TCP_NOTIFY_IDX_USB)
#define TCP_NOTIFY_TYPE_MISC  (1 << TCP_NOTIFY_IDX_MISC)
#define TCP_NOTIFY_TYPE_ALL   ((1 << TCP_NOTIFY_IDX_NR) - 1)

enum {
	/* TCP_NOTIFY_TYPE_MODE */
	TCP_NOTIFY_ENTER_MODE,
	TCP_NOTIFY_MODE_START = TCP_NOTIFY_ENTER_MODE,
	TCP_NOTIFY_EXIT_MODE,
	TCP_NOTIFY_AMA_DP_STATE,
	TCP_NOTIFY_AMA_DP_ATTENTION,
	TCP_NOTIFY_AMA_DP_HPD_STATE,
	TCP_NOTIFY_DC_EN_UNLOCK,
	TCP_NOTIFY_UVDM,
	TCP_NOTIFY_MODE_END = TCP_NOTIFY_UVDM,

	/* TCP_NOTIFY_TYPE_VBUS */
	TCP_NOTIFY_DIS_VBUS_CTRL,
	TCP_NOTIFY_VBUS_START = TCP_NOTIFY_DIS_VBUS_CTRL,
	TCP_NOTIFY_SOURCE_VCONN,
	TCP_NOTIFY_SOURCE_VBUS,
	TCP_NOTIFY_SINK_VBUS,
	TCP_NOTIFY_EXT_DISCHARGE,
	TCP_NOTIFY_ATTACHWAIT_SNK,
	TCP_NOTIFY_ATTACHWAIT_SRC,
	TCP_NOTIFY_VBUS_END = TCP_NOTIFY_ATTACHWAIT_SRC,

	/* TCP_NOTIFY_TYPE_USB */
	TCP_NOTIFY_TYPEC_STATE,
	TCP_NOTIFY_USB_START = TCP_NOTIFY_TYPEC_STATE,
	TCP_NOTIFY_PD_STATE,
	TCP_NOTIFY_USB_END = TCP_NOTIFY_PD_STATE,

	/* TCP_NOTIFY_TYPE_MISC */
	TCP_NOTIFY_PR_SWAP,
	TCP_NOTIFY_MISC_START = TCP_NOTIFY_PR_SWAP,
	TCP_NOTIFY_DR_SWAP,
	TCP_NOTIFY_VCONN_SWAP,
	TCP_NOTIFY_HARD_RESET_STATE,
	TCP_NOTIFY_ALERT,
	TCP_NOTIFY_STATUS,
	TCP_NOTIFY_REQUEST_BAT_INFO,
	TCP_NOTIFY_WD_STATUS,
	TCP_NOTIFY_FOD_STATUS,
	TCP_NOTIFY_CABLE_TYPE,
	TCP_NOTIFY_VBUS_CC_SHORT,
	TCP_NOTIFY_TYPEC_OTP,
	TCP_NOTIFY_PLUG_OUT,
	TCP_NOTIFY_MISC_END = TCP_NOTIFY_PLUG_OUT,
};

struct tcp_ny_pd_state {
	uint8_t connected;
};

struct tcp_ny_swap_state {
	uint8_t new_role;
};

struct tcp_ny_enable_state {
	bool en;
};

struct tcp_ny_typec_state {
	uint8_t rp_level;
	uint8_t polarity;
	uint8_t old_state;
	uint8_t new_state;
};

enum {
	TCP_VBUS_CTRL_REMOVE = 0,
	TCP_VBUS_CTRL_TYPEC = 1,
	TCP_VBUS_CTRL_PD = 2,

	TCP_VBUS_CTRL_HRESET = TCP_VBUS_CTRL_PD,
	TCP_VBUS_CTRL_PR_SWAP = 3,
	TCP_VBUS_CTRL_REQUEST = 4,
	TCP_VBUS_CTRL_STANDBY = 5,
	TCP_VBUS_CTRL_STANDBY_UP = 6,
	TCP_VBUS_CTRL_STANDBY_DOWN = 7,

	TCP_VBUS_CTRL_PD_DETECT = (1 << 7),

	TCP_VBUS_CTRL_PD_HRESET = TCP_VBUS_CTRL_HRESET |
		TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_PR_SWAP = TCP_VBUS_CTRL_PR_SWAP |
		TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_REQUEST = TCP_VBUS_CTRL_REQUEST |
		TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_STANDBY = TCP_VBUS_CTRL_STANDBY |
		TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_STANDBY_UP = TCP_VBUS_CTRL_STANDBY_UP |
		TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_STANDBY_DOWN = TCP_VBUS_CTRL_STANDBY_DOWN |
		TCP_VBUS_CTRL_PD_DETECT,
};

struct tcp_ny_vbus_state {
	int mv;
	int ma;
	uint8_t type;
};

struct tcp_ny_mode_ctrl {
	uint16_t svid;
	uint8_t ops;
	uint32_t mode;
};

enum {
	SW_USB = 0,
	SW_DFP_D,
	SW_UFP_D,
};

#define TCPM_DP_STATE_SIGNAL_BIT  2
#define TCPM_DP_STATE_SNK_BIT     8
#define TCPM_DP_STATE_SRC_BIT     16
#define TCPM_DP_STATE_PIN_MASK    0xff
#define TCPM_DP_STATE_MODE_MASK   0x03
#define TCPM_DP_STATE_SIGNAL_MASK 0x0f

struct tcp_ny_ama_dp_state {
	uint8_t sel_config;
	uint8_t signal;
	uint8_t pin_assignment;
	uint8_t polarity;
	uint8_t active;
};

enum {
	TCP_DP_UFP_U_MASK = 0x7C,
	TCP_DP_UFP_U_POWER_LOW = 1 << 2,
	TCP_DP_UFP_U_ENABLED = 1 << 3,
	TCP_DP_UFP_U_MF_PREFER = 1 << 4,
	TCP_DP_UFP_U_USB_CONFIG = 1 << 5,
	TCP_DP_UFP_U_EXIT_MODE = 1 << 6,
};

struct tcp_ny_ama_dp_attention {
	uint8_t state;
};

struct tcp_ny_ama_dp_hpd_state {
	uint8_t irq;
	uint8_t state;
};

struct tcp_ny_uvdm {
	bool ack;
	uint8_t uvdm_cnt;
	uint16_t uvdm_svid;
	uint32_t *uvdm_data;
};

/*
 * Please don't expect that every signal will have a corresponding result.
 * The signal can be generated multiple times before receiving a result.
 */
enum {
	/* HardReset finished because recv GoodCRC or TYPE-C only */
	TCP_HRESET_RESULT_DONE = 0,
	/* HardReset failed because detach or error recovery */
	TCP_HRESET_RESULT_FAIL,
	/* HardReset signal from Local Policy Engine */
	TCP_HRESET_SIGNAL_SEND,
	/* HardReset signal from Port Partner */
	TCP_HRESET_SIGNAL_RECV,
};

struct tcp_ny_hard_reset_state {
	uint8_t state;
};

struct tcp_ny_alert {
	uint32_t ado;
};

struct tcp_ny_status {
	const struct pd_status *sdb;
};

struct tcp_ny_request_bat {
	enum pd_battery_reference ref;
};

struct tcp_ny_wd_status {
	bool water_detected;
};

struct tcp_ny_vbus_cc_short {
	bool cc1;
	bool cc2;
};

enum tcpc_fod_status {
	TCPC_FOD_NONE = 0,
	TCPC_FOD_NORMAL,
	TCPC_FOD_OV,
	TCPC_FOD_DISCHG_FAIL,
	TCPC_FOD_LR,
	TCPC_FOD_HR,
	TCPC_FOD_STAT_MAX,
};

enum tcpc_cable_type {
	TCPC_CABLE_TYPE_NONE = 0,
	TCPC_CABLE_TYPE_A2C,
	TCPC_CABLE_TYPE_C2C,
	TCPC_CABLE_TYPE_MAX,
};

struct tcp_ny_fod_status {
	enum tcpc_fod_status fod;
};

struct tcp_ny_cable_type {
	enum tcpc_cable_type type;
};

struct tcp_notify {
	union {
		struct tcp_ny_enable_state en_state;
		struct tcp_ny_vbus_state vbus_state;
		struct tcp_ny_typec_state typec_state;
		struct tcp_ny_swap_state swap_state;
		struct tcp_ny_pd_state pd_state;
		struct tcp_ny_mode_ctrl mode_ctrl;
		struct tcp_ny_ama_dp_state ama_dp_state;
		struct tcp_ny_ama_dp_attention ama_dp_attention;
		struct tcp_ny_ama_dp_hpd_state ama_dp_hpd_state;
		struct tcp_ny_uvdm uvdm_msg;
		struct tcp_ny_hard_reset_state hreset_state;
		struct tcp_ny_alert alert_msg;
		struct tcp_ny_status status_msg;
		struct tcp_ny_request_bat request_bat;
		struct tcp_ny_wd_status wd_status;
		struct tcp_ny_fod_status fod_status;
		struct tcp_ny_cable_type cable_type;
		struct tcp_ny_vbus_cc_short vbus_cc_short;
	};
};

struct tcpm_notifier_block_wrapper {
	struct notifier_block stub_nb;
	struct notifier_block *action_nb;
};

struct tcpm_notifier_node {
	struct tcpm_notifier_block_wrapper *wpr;
	struct notifier_block *nb_key;
	int idx;
	struct tcpm_notifier_node *next;
};

struct tcpm_notifier {
	struct srcu_notifier_head evt_nh[TCP_NOTIFY_IDX_NR];
	struct tcpm_notifier_node *nl_head;
	struct mutex nl_lock;
	struct device *dev;
};

struct tcpc_device;

void tcpm_notifier_init(struct tcpc_device *tcpc);
int tcpm_register_notifier(struct tcpm_notifier *notifier,
	struct notifier_block *nb, uint8_t flags);
void tcpm_unregister_dev_notifier(struct tcpm_notifier *notifier,
	struct notifier_block *nb, uint8_t flags);

void tcpm_notify_vconn(struct tcpm_notifier *notifier, bool en);
void tcpm_notify_wd_status(struct tcpm_notifier *notifier, bool detected);
void tcpm_notify_cable_type(struct tcpm_notifier *notifier,
	enum tcpc_cable_type type);
void tcpm_notify_vbus_cc_short(struct tcpm_notifier *notifier, bool cc1_short,
	bool cc2_short);
void tcpm_notify_typec_state(struct tcpm_notifier *notifier, uint8_t polarity,
	uint8_t old_state, uint8_t new_state, uint8_t rp);
void tcpm_notify_role_swap(struct tcpm_notifier *notifier, uint8_t event,
	uint8_t role);
void tcpm_notify_pd_state(struct tcpm_notifier *notifier, uint8_t connect);
void tcpm_notify_source_vbus_state(struct tcpm_notifier *notifier,
	uint8_t type, int mv, int ma);
void tcpm_notify_sink_vbus_state(struct tcpm_notifier *notifier, uint8_t type,
	int mv, int ma);
void tcpm_notify_diable_vbus_ctrl(struct tcpm_notifier *notifier);
void tcpm_notify_source_vbus_wait(struct tcpm_notifier *notifier);
void tcpm_notify_sink_vbus_wait(struct tcpm_notifier *notifier);
void tcpm_notify_vbus_ext_discharge(struct tcpm_notifier *notifier, bool en);
void tcpm_notify_hard_reset_state(struct tcpm_notifier *notifier,
	uint8_t state);
void tcpm_notify_enter_mode(struct tcpm_notifier *notifier, uint16_t svid,
	uint8_t ops, uint32_t mode);
void tcpm_notify_exit_mode(struct tcpm_notifier *notifier, uint16_t svid);
void tcpm_notify_hpd_state(struct tcpm_notifier *notifier, uint8_t irq,
	uint8_t state);
void tcpm_notify_dp_state(struct tcpm_notifier *notifier, uint8_t polarity,
	uint32_t dp_state);
void tcpm_notify_dp_attention(struct tcpm_notifier *notifier, uint8_t status);
void tcpm_notify_dp_cfg_start(struct tcpm_notifier *notifier);
void tcpm_notify_uvdm(struct tcpm_notifier *notifier, bool ack, uint8_t cnt,
	uint16_t svid, uint32_t *data);
void tcpm_notify_dc_en_unlock(struct tcpm_notifier *notifier);
void tcpm_notify_alert(struct tcpm_notifier *notifier, uint32_t ado);
void tcpm_notify_status(struct tcpm_notifier *notifier, struct pd_status *sdb);
void tcpm_notify_request_bat_info(struct tcpm_notifier *notifier,
	enum pd_battery_reference ref);

#endif /* _TCPM_NOTIFIER_H_ */
