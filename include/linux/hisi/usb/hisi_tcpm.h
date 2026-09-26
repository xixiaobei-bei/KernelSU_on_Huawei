/*
 * hisi_tcpm.h
 *
 * Hisilicon tcpm interface defination
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
 */

#ifndef TCPM_H_
#define TCPM_H_

#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/hisi/usb/hisi_typec.h>

struct tcpc_device;

#define PD_DATA_OBJ_SIZE		7
#define TCPM_PDO_MAX_SIZE		(PD_DATA_OBJ_SIZE)
#define PDO_MAX_NR			(PD_DATA_OBJ_SIZE)
#define VDO_MAX_NR			(PD_DATA_OBJ_SIZE - 1)
#define VDO_MAX_SVID_NR			(VDO_MAX_NR * 2)

enum hisi_pd_connect_result {
	HISI_PD_CONNECT_NONE = 0,
	HISI_PD_CONNECT_TYPEC_ONLY, /* Internal Only */
	HISI_PD_CONNECT_TYPEC_ONLY_SNK_DFT,
	HISI_PD_CONNECT_TYPEC_ONLY_SNK,
	HISI_PD_CONNECT_TYPEC_ONLY_SRC,
	HISI_PD_CONNECT_PE_READY, /* Internal Only */
	HISI_PD_CONNECT_PE_READY_SNK,
	HISI_PD_CONNECT_PE_READY_SRC,

#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT
	HISI_PD_CONNECT_PE_READY_DBGACC_UFP,
	HISI_PD_CONNECT_PE_READY_DBGACC_DFP,
#endif /* CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT */
};

/* Power role */
#define PD_ROLE_SINK   0
#define PD_ROLE_SOURCE 1
#define PD_ROLE_UNATTACHED 3

/* Data role */
#define PD_ROLE_UFP    0
#define PD_ROLE_DFP    1

/* Vconn role */
#define PD_ROLE_VCONN_OFF 0
#define PD_ROLE_VCONN_ON  1

enum {
	TCP_NOTIFY_DIS_VBUS_CTRL,
	TCP_NOTIFY_SOURCE_VCONN,
	TCP_NOTIFY_SOURCE_VBUS,
	TCP_NOTIFY_SINK_VBUS,
	TCP_NOTIFY_PR_SWAP,
	TCP_NOTIFY_DR_SWAP,
	TCP_NOTIFY_VCONN_SWAP,
	TCP_NOTIFY_ENTER_MODE,
	TCP_NOTIFY_EXIT_MODE,
	TCP_NOTIFY_AMA_DP_STATE,
	TCP_NOTIFY_AMA_DP_ATTENTION,
	TCP_NOTIFY_AMA_DP_HPD_STATE,

	TCP_NOTIFY_TYPEC_STATE,
	TCP_NOTIFY_PD_STATE,

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	TCP_NOTIFY_UVDM,
#endif

	TCP_NOTIFY_CABLE_VDO,

	/* PD V30 */
#ifdef CONFIG_USB_PD_REV30
	TCP_NOTIFY_ALERT,
	TCP_NOTIFY_STATUS,
	TCP_NOTIFY_PPS_STATUS,
	TCP_NOTIFY_PPS_READY,
#endif
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
	bool irq : 1;
	bool state : 1;
};

struct tcp_ny_uvdm {
	bool ack;
	uint8_t uvdm_cnt;
	uint16_t uvdm_svid;
	uint32_t *uvdm_data;
};

struct tcp_ny_cable_vdo {
	uint32_t vdo;
	uint32_t vdo_ext; /* the vdo when current is greater than 5A */
};

struct tcp_ny_pps_info {
	uint32_t max_volt;
	uint32_t min_volt;
	uint32_t max_curr;
};

struct tcp_ny_alert {
	uint32_t ado;
};

struct tcp_ny_status {
	const struct pd_status *sdb;
};

struct tcp_notify {
	union {
		struct tcp_ny_enable_state en_state; /* common */
		struct tcp_ny_vbus_state vbus_state;
		struct tcp_ny_typec_state typec_state;
		struct tcp_ny_swap_state swap_state;
		struct tcp_ny_pd_state pd_state;
		struct tcp_ny_mode_ctrl mode_ctrl;
		struct tcp_ny_ama_dp_state ama_dp_state;
		struct tcp_ny_ama_dp_attention ama_dp_attention;
		struct tcp_ny_ama_dp_hpd_state ama_dp_hpd_state;
		struct tcp_ny_uvdm uvdm_msg;
		struct tcp_ny_alert alert_msg;
		struct tcp_ny_status status_msg;
		struct tcp_ny_cable_vdo cable_vdo;
		struct tcp_ny_pps_info pps_apdo;
	};
};

struct tcpm_power_cap_val {
	uint8_t type;
	uint8_t apdo_type;
	uint8_t pwr_limit;

	int max_mv;
	int min_mv;

	union {
		int uw;
		int ma;
	};
};

struct dpm_pdo_info_t {
	uint8_t type;
	uint8_t apdo_type;
	int vmin;
	int vmax;
	int uw;
	int ma;
};

struct dpm_rdo_info_t {
	uint8_t pos;
	uint8_t type;
	bool mismatch;

	int vmin;
	int vmax;

	union {
		uint32_t max_uw;
		uint32_t max_ma;
	};

	union {
		uint32_t oper_uw;
		uint32_t oper_ma;
	};
};

struct local_sink_cap {
	int mv;
	int ma;
	int uw;
};

/*
 * Type-C Port Control I/F
 */
enum tcpm_error_list {
	TCPM_SUCCESS = 0,
	TCPM_ERROR_UNKNOWN = -1,
	TCPM_ERROR_UNATTACHED = -2,
	TCPM_ERROR_PARAMETER = -3,
	TCPM_ERROR_PUT_EVENT = -4,
	TCPM_ERROR_UNSUPPORT = -5,
	TCPM_ERROR_NO_PD_CONNECTED = -6,
	TCPM_ERROR_NO_APDO = -7,
	TCPM_ERROR_NOT_FOUND = -8,
	TCPM_ERROR_REPEAT_POLICY = -9,
};

struct tcpm_power_cap {
	uint8_t cnt;
	uint32_t pdos[TCPM_PDO_MAX_SIZE];
};

struct tcpm_remote_power_cap {
	uint8_t selected_cap_idx;
	uint8_t nr;
	int max_mv[PDO_MAX_NR];
	int min_mv[PDO_MAX_NR];
	int ma[PDO_MAX_NR];
	uint8_t type[PDO_MAX_NR];
};


/* DPM Flags */

#define DPM_FLAGS_PARTNER_DR_POWER		(1 << 0)
#define DPM_FLAGS_PARTNER_DR_DATA		(1 << 1)
#define DPM_FLAGS_PARTNER_EXTPOWER		(1 << 2)
#define DPM_FLAGS_PARTNER_USB_COMM		(1 << 3)
#define DPM_FLAGS_PARTNER_USB_SUSPEND		(1 << 4)
#define DPM_FLAGS_PARTNER_HIGH_CAP		(1 << 5)

#define DPM_FLAGS_PARTNER_MISMATCH		(1 << 7)
#define DPM_FLAGS_PARTNER_GIVE_BACK		(1 << 8)
#define DPM_FLAGS_PARTNER_NO_SUSPEND		(1 << 9)

#define DPM_FLAGS_RESET_PARTNER_MASK		\
	(DPM_FLAGS_PARTNER_DR_POWER | DPM_FLAGS_PARTNER_DR_DATA|\
	DPM_FLAGS_PARTNER_EXTPOWER | DPM_FLAGS_PARTNER_USB_COMM)

#define DPM_FLAGS_CHECK_DC_MODE			(1 << 20)
#define DPM_FLAGS_CHECK_UFP_SVID		(1 << 21)
#define DPM_FLAGS_CHECK_EXT_POWER		(1 << 22)
#define DPM_FLAGS_CHECK_DP_MODE			(1 << 23)
#define DPM_FLAGS_CHECK_SINK_CAP		(1 << 24)
#define DPM_FLAGS_CHECK_SOURCE_CAP		(1 << 25)
#define DPM_FLAGS_CHECK_UFP_ID			(1 << 26)
#define DPM_FLAGS_CHECK_CABLE_ID		(1 << 27)
#define DPM_FLAGS_CHECK_CABLE_ID_DFP		(1 << 28)
#define DPM_FLAGS_CHECK_PR_ROLE			(1 << 29)
#define DPM_FLAGS_CHECK_DR_ROLE			(1 << 30)

/* DPM_CAPS */

#define DPM_CAP_LOCAL_DR_POWER			(1 << 0)
#define DPM_CAP_LOCAL_DR_DATA			(1 << 1)
#define DPM_CAP_LOCAL_EXT_POWER			(1 << 2)
#define DPM_CAP_LOCAL_USB_COMM			(1 << 3)
#define DPM_CAP_LOCAL_USB_SUSPEND		(1 << 4)
#define DPM_CAP_LOCAL_HIGH_CAP			(1 << 5)
#define DPM_CAP_LOCAL_GIVE_BACK			(1 << 6)
#define DPM_CAP_LOCAL_NO_SUSPEND		(1 << 7)
#define DPM_CAP_LOCAL_VCONN_SUPPLY		(1 << 8)

#define DPM_CAP_ATTEMP_ENTER_DC_MODE		(1 << 11)
#define DPM_CAP_ATTEMP_DISCOVER_CABLE_DFP	(1 << 12)
#define DPM_CAP_ATTEMP_ENTER_DP_MODE		(1 << 13)
#define DPM_CAP_ATTEMP_DISCOVER_CABLE		(1 << 14)
#define DPM_CAP_ATTEMP_DISCOVER_ID		(1 << 15)

enum dpm_cap_pr_check_prefer {
	DPM_CAP_PR_CHECK_DISABLE = 0,
	DPM_CAP_PR_CHECK_PREFER_SNK = 1,
	DPM_CAP_PR_CHECK_PREFER_SRC = 2,
};

#define DPM_CAP_PR_CHECK_PROP(cap)		(((cap) & 0x03) << 16)
#define DPM_CAP_EXTRACT_PR_CHECK(raw)		(((raw) >> 16) & 0x03)
#define DPM_CAP_PR_SWAP_REJECT_AS_SRC		(1 << 18)
#define DPM_CAP_PR_SWAP_REJECT_AS_SNK		(1 << 19)
#define DPM_CAP_PR_SWAP_CHECK_GP_SRC		(1 << 20)
#define DPM_CAP_PR_SWAP_CHECK_GP_SNK		(1 << 21)
#define DPM_CAP_PR_SWAP_CHECK_GOOD_POWER	\
	(DPM_CAP_PR_SWAP_CHECK_GP_SRC | DPM_CAP_PR_SWAP_CHECK_GP_SNK)

enum dpm_cap_dr_check_prefer {
	DPM_CAP_DR_CHECK_DISABLE = 0,
	DPM_CAP_DR_CHECK_PREFER_UFP = 1,
	DPM_CAP_DR_CHECK_PREFER_DFP = 2,
};

#define DPM_CAP_DR_CHECK_PROP(cap)		(((cap) & 0x03) << 22)
#define DPM_CAP_EXTRACT_DR_CHECK(raw)		(((raw) >> 22) & 0x03)
#define DPM_CAP_DR_SWAP_REJECT_AS_DFP		(1 << 24)
#define DPM_CAP_DR_SWAP_REJECT_AS_UFP		(1 << 25)

#define DPM_CAP_DP_PREFER_MF			(1 << 29)
#define DPM_CAP_SNK_PREFER_LOW_VOLTAGE		(1 << 30)
#define DPM_CAP_SNK_IGNORE_MISMATCH_CURRENT	(0x80000000)

/* Charging Policy: Select PDO */
#define DPM_CHARGING_POLICY_MASK	(0x0f)

enum dpm_charging_policy {
	/* VSafe5V only */
	DPM_CHARGING_POLICY_VSAFE5V = 0,

	/* Max Power */
	DPM_CHARGING_POLICY_MAX_POWER = 1,

	/* Custom defined Policy */
	DPM_CHARGING_POLICY_CUSTOM = 2,

	/*  Runtime Policy, restore to default after plug-out or hard-reset */
	DPM_CHARGING_POLICY_RUNTIME = 3,

	/* Direct charge <Variable PDO only> */
	DPM_CHARGING_POLICY_DIRECT_CHARGE = 3,

	/* PPS <Augmented PDO only> */
	DPM_CHARGING_POLICY_PPS = 4,

	/* Default Charging Policy <from DTS> */
	DPM_CHARGING_POLICY_DEFAULT = 0xff,

	DPM_CHARGING_POLICY_IGNORE_MISMATCH_CURR = 1 << 4,
	DPM_CHARGING_POLICY_PREFER_LOW_VOLTAGE = 1 << 5,
	DPM_CHARGING_POLICY_PREFER_HIGH_VOLTAGE = 1 << 6,

	DPM_CHARGING_POLICY_MAX_POWER_LV =
		DPM_CHARGING_POLICY_MAX_POWER |
		DPM_CHARGING_POLICY_PREFER_LOW_VOLTAGE,
	DPM_CHARGING_POLICY_MAX_POWER_LVIC =
		DPM_CHARGING_POLICY_MAX_POWER_LV |
		DPM_CHARGING_POLICY_IGNORE_MISMATCH_CURR,

	DPM_CHARGING_POLICY_MAX_POWER_HV =
		DPM_CHARGING_POLICY_MAX_POWER |
		DPM_CHARGING_POLICY_PREFER_HIGH_VOLTAGE,
	DPM_CHARGING_POLICY_MAX_POWER_HVIC =
		DPM_CHARGING_POLICY_MAX_POWER_HV |
		DPM_CHARGING_POLICY_IGNORE_MISMATCH_CURR,
};

int hisi_tcpm_shutdown(struct tcpc_device *tcpc_dev);
int hisi_tcpm_inquire_remote_cc(struct tcpc_device *tcpc_dev,
		uint8_t *cc1, uint8_t *cc2, bool from_ic);
void hisi_tcpm_force_cc_mode(struct tcpc_device *tcpc_dev, int mode);
int hisi_tcpm_typec_set_wake_lock(struct tcpc_device *tcpc, bool user_lock);
int hisi_tcpm_typec_set_rp_level(struct tcpc_device *tcpc_dev, uint8_t level);
int hisi_tcpm_typec_role_swap(struct tcpc_device *tcpc_dev);
int hisi_tcpm_typec_notify_direct_charge(struct tcpc_device *tcpc_dev, bool dc);
int hisi_tcpm_typec_change_role(struct tcpc_device *tcpc_dev, uint8_t typec_role);
uint8_t hisi_tcpc_get_cc_from_analog_ch(struct tcpc_device *tcpc_dev);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
int hisi_tcpm_power_role_swap(struct tcpc_device *tcpc_dev);
int hisi_tcpm_data_role_swap(struct tcpc_device *tcpc_dev);
int hisi_tcpm_vconn_swap(struct tcpc_device *tcpc_dev);
int hisi_tcpm_soft_reset(struct tcpc_device *tcpc_dev);
int hisi_tcpm_hard_reset(struct tcpc_device *tcpc_dev);
int hisi_tcpm_get_source_cap(struct tcpc_device *tcpc_dev, struct tcpm_power_cap *cap);
int hisi_tcpm_get_sink_cap(struct tcpc_device *tcpc_dev, struct tcpm_power_cap *cap);
void hisi_tcpm_request_voltage(struct tcpc_device *tcpc_dev, int set_voltage);
void hisi_tcpm_detect_emark_cable(struct tcpc_device *tcpc_dev);
bool hisi_tcpm_support_apdo(struct tcpc_device *tcpc);
bool hisi_tcpm_support_pd(struct tcpc_device *tcpc);

int hisi_tcpm_discover_cable(struct tcpc_device *tcpc_dev, uint32_t *vdos,
		unsigned max_size);

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
#define PD_UVDM_HDR(vid, custom)	(((vid) << 16) | ((custom) & 0x7FFF))
#define PD_UVDM_HDR_CMD(hdr)		((hdr) & 0x7FFF)
int hisi_tcpm_send_uvdm(struct tcpc_device *tcpc_dev, uint8_t cnt,
		uint32_t *data, bool wait_resp);
#endif

#define TCPM_APDO_TYPE_MASK	(0x0f)

enum tcpm_power_cap_apdo_type {
	TCPM_POWER_CAP_APDO_TYPE_PPS = 1 << 0,

	TCPM_POWER_CAP_APDO_TYPE_PPS_CF = (1 << 7),
};

enum TCP_DPM_EVT_ID {
	TCP_DPM_EVT_UNKONW = 0,

	TCP_DPM_EVT_GOTOMIN,
	TCP_DPM_EVT_REQUEST,
	TCP_DPM_EVT_REQUEST_EX,
	TCP_DPM_EVT_REQUEST_AGAIN,
	TCP_DPM_EVT_BIST_CM2,
	TCP_DPM_EVT_DUMMY,

	TCP_DPM_EVT_PD30_COMMAND,
	TCP_DPM_EVT_GET_SOURCE_CAP_EXT = TCP_DPM_EVT_PD30_COMMAND,
	TCP_DPM_EVT_GET_STATUS,
	TCP_DPM_EVT_GET_PPS_STATUS,

	TCP_DPM_EVT_ALERT,
	TCP_DPM_EVT_GET_MFRS_INFO,

	TCP_DPM_EVT_VDM_COMMAND,

	TCP_DPM_EVT_NR,
};

struct tcp_dpm_event;
typedef int (*tcp_dpm_event_cb)(
	struct tcpc_device *tcpc, int ret, struct tcp_dpm_event *event);

struct tcp_dpm_event_cb_data {
	void *user_data;
	tcp_dpm_event_cb event_cb;
};

enum pd_present_temperature_flag {
	PD_PTF_NO_SUPPORT = 0,
	PD_PTF_NORMAL,
	PD_PTF_WARNING,
	PD_PTF_OVER_TEMP,
};

struct tcp_dpm_pd_request {
	int mv;
	int ma;
};

struct tcp_dpm_pd_request_ex {
	uint8_t pos;

	union {
		uint32_t max;
		uint32_t max_uw;
		uint32_t max_ma;
	};

	union {
		uint32_t oper;
		uint32_t oper_uw;
		uint32_t oper_ma;
	};
};

struct tcp_dpm_event {
	uint8_t event_id;
	void *user_data;
	tcp_dpm_event_cb event_cb;

	union {
		struct tcp_dpm_pd_request pd_req;
		struct tcp_dpm_pd_request_ex pd_req_ex;

		uint32_t index;
		uint32_t data_object[TCPM_PDO_MAX_SIZE];
	} tcp_dpm_data;
};

struct pd_source_cap_ext {
	uint16_t vid;
	uint16_t pid;
	uint32_t xid;
	uint8_t fw_ver;
	uint8_t hw_ver;
	uint8_t voltage_regulation;
	uint8_t hold_time_ms;
	uint8_t compliance;
	uint8_t touch_current;
	uint16_t peak_current[3];
	uint8_t touch_temp;
	uint8_t source_inputs;
	uint8_t batteries;
	uint8_t source_pdp;
};

/* PPSSDB, PPSStatus */
#define TCPM_PPSSDB_SIZE		4 /* PPS_Status */
#define TCPM_PPS_GET_OUTPUT_MV(raw)	((raw) * 20)
#define TCPM_PPS_GET_OUTPUT_MA(raw)	((raw) * 50)

#define TCPM_STASUS_EVENT_OCP		(1 << 1)
#define TCPM_STATUS_EVENT_OTP		(1 << 2)
#define TCPM_STATUS_EVENT_OVP		(1 << 3)

struct pd_pps_status_raw {
	uint16_t output_vol_raw; /* 0xffff means no support */
	uint8_t output_curr_raw; /* 0xff means no support */
	uint8_t real_time_flags;
};

struct pd_pps_status {
	int output_mv;
	int output_ma;
	uint8_t real_time_flags;
};

struct pd_status {
	uint8_t internal_temp; /* 0 means no support */
	uint8_t present_input;
	uint8_t present_battey_input;
	uint8_t event_flags;
	uint8_t temp_status;
	uint8_t power_status;
};


#define TCPM_ADO_ALERT_OCP		(1 << 2)
#define TCPM_ADO_ALERT_OTP		(1 << 3)
#define TCPM_ADO_ALERT_OVP		(1 << 6)
#define TCPM_ADO_ALERT_TYPE(raw)	(raw >> 24)

int tcpm_set_apdo_charging_policy(struct tcpc_device *tcpc, uint8_t policy,
		int mv, int ma, struct tcp_dpm_event_cb_data *cb_data);

int tcpm_set_pd_charging_policy(struct tcpc_device *tcpc, uint8_t policy,
		struct tcp_dpm_event_cb_data *cb_data);

int tcpm_inquire_pd_source_apdo(struct tcpc_device *tcpc,
		uint8_t apdo_type, uint8_t *cap_i,
		struct tcpm_power_cap_val *cap);

int tcpm_dpm_pd_get_source_cap_ext(struct tcpc_device *tcpc,
	const struct tcp_dpm_event_cb_data *cb_data,
	struct pd_source_cap_ext *src_cap_ext);

int tcpm_dpm_pd_get_status(struct tcpc_device *tcpc,
		struct tcp_dpm_event_cb_data *cb_data, struct pd_status *status);

int tcpm_dpm_pd_get_pps_status(struct tcpc_device *tcpc,
	const struct tcp_dpm_event_cb_data *cb_data,
	struct pd_pps_status *pps_status);

int tcpm_get_remote_power_cap(struct tcpc_device *tcpc_dev,
	struct tcpm_remote_power_cap *cap);

#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

int hisi_tcpm_register_tcpc_dev_notifier(struct tcpc_device *tcp_dev,
			      struct notifier_block *nb);
int hisi_tcpm_unregister_tcpc_dev_notifier(struct tcpc_device *tcp_dev,
			      struct notifier_block *nb);

#endif /* TCPM_H_ */
