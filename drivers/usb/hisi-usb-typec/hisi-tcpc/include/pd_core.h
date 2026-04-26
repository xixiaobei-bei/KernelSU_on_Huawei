/*
 * Copyright (C) 2018 Hisilicon Technology Corp.
 * Author: Hisilicon <>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef PD_CORE_H_
#define PD_CORE_H_

#include "include/tcpci_timer.h"
#include "include/tcpci_event.h"
#include "include/pd_dbg_info.h"
#include "include/tcpci_config.h"
#include "include/pd_tcpm.h"
#include <linux/hisi/usb/hisi_tcpm.h>
#include <linux/hisi/usb/hisi_typec.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_uvdm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>

#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
#define CONFIG_PD_DISCOVER_CABLE_ID
#endif

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
#undef CONFIG_PD_DISCOVER_CABLE_ID
#define CONFIG_PD_DISCOVER_CABLE_ID
#endif

#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
#define CONFIG_USB_PD_MODE_OPERATION
#endif

#ifdef CONFIG_USB_PD_RESET_CABLE
#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
#define CONFIG_PD_DFP_RESET_CABLE
#endif
#endif

/* Protocol revision */
#define PD_REV10 0
#define PD_REV20 1
#define PD_REV30 2

#define PD_SOP_NR	3

/* Default retry count for transmitting */
#define PD_RETRY_COUNT 3

#if PD_RETRY_COUNT > 3
#error "PD_RETRY_COUNT Max = 3"
#endif

typedef struct __pd_port_power_cababilities {
	uint8_t nr;
	uint32_t pdos[PD_PORT_PDOS_MAX_SIZE];
} pd_port_power_caps;

typedef struct __svdm_mode {
	uint8_t mode_cnt;
	uint32_t mode_vdo[VDO_MAX_DATA_SIZE];
} svdm_mode_t;

struct __svdm_svid_ops;
typedef struct __svdm_svid_data {
	bool exist;
	uint16_t svid;
	uint8_t active_mode;
	svdm_mode_t local_mode;
	svdm_mode_t remote_mode;
	const struct __svdm_svid_ops *ops;
} svdm_svid_data_t;

typedef struct __svdm_svid_list {
	uint8_t cnt;
	uint16_t svids[VDO_MAX_SVID_SIZE];
} svdm_svid_list_t;

typedef struct __pd_port {
	struct tcpc_device *tcpc_dev;
	struct mutex pd_lock;

	/* PD */
	bool explicit_contract;
	bool invalid_contract;
	bool vconn_source;

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	bool vconn_return;
#endif

	bool pe_ready;
	bool pd_connected;
	bool pd_prev_connected;
	bool msg_output_lock;

	uint8_t state_machine;
	uint8_t pd_connect_state;

	bool reset_vdm_state;
	uint8_t pe_pd_state;
	uint8_t pe_vdm_state;

	uint8_t pe_state_next;
	uint8_t pe_state_curr;

	uint8_t data_role;
	uint8_t power_role;

	uint8_t cap_counter;
	uint8_t discover_id_counter;
	uint8_t hard_reset_counter;
	uint8_t snk_cap_count;
	uint8_t src_cap_count;
	uint8_t get_snk_cap_count;
	uint8_t get_src_cap_count;

	bool vdm_discard_retry_flag;
	uint8_t vdm_discard_retry_count;

	uint8_t msg_id_rx[PD_SOP_NR];
	uint8_t msg_id_rx_init[PD_SOP_NR];
	uint8_t msg_id_tx[PD_SOP_NR];

	uint16_t pd_msg_data_size;
	uint8_t pd_msg_data_count;
	uint8_t *pd_msg_data_payload;
	uint16_t curr_msg_hdr;
	pd_event_t curr_pd_event;

	uint8_t tcp_event_id_2nd;
	bool tcp_event_drop_reset_once;
	uint8_t *tcpm_bk_cb_data;
	uint8_t tcpm_bk_cb_data_max;

	int tcpm_bk_ret;
	bool tcpm_bk_done;
	uint8_t tcpm_bk_event_id;
	struct mutex tcpm_bk_lock;
	wait_queue_head_t tcpm_bk_wait_que;

#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
	uint8_t msg_id_pr_swap_last;
#endif /* CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP */

	uint32_t last_rdo;
	uint32_t cable_vdos[VDO_MAX_SIZE];
	bool power_cable_present;

#ifdef CONFIG_USB_PD_RESET_CABLE
	bool reset_cable;
	bool detect_emark;
	int vswap_ret;
#endif

#ifdef CONFIG_USB_PD_REV30
	uint8_t pd_revision[2];
	uint8_t remote_rev;
#endif /* CONFIG_USB_PD_REV30 */

	uint8_t id_vdo_nr;
	uint32_t id_vdos[VDO_MAX_DATA_SIZE];

#ifdef CONFIG_USB_PD_KEEP_SVIDS
	svdm_svid_list_t remote_svid_list;
#endif /* CONFIG_USB_PD_KEEP_SVIDS */

	uint8_t svid_data_cnt;
	svdm_svid_data_t svid_data[VDO_MAX_SVID_SIZE];

	bool during_swap; /* pr or dr swap */

	/* DPM */
	int request_v;
	int request_i;
	int request_v_new;
	int request_i_new;
	int request_i_op;
	int request_i_max;

	int request_v_apdo;
	int request_i_apdo;
	bool request_apdo;
	bool request_apdo_new;
	uint8_t request_apdo_pos;
	uint8_t local_snk_cap_nr_pd30;
	uint8_t local_snk_cap_nr_pd20;

	uint8_t local_selected_cap;
	uint8_t remote_selected_cap;
	pd_port_power_caps local_src_cap;
	pd_port_power_caps local_snk_cap;
	pd_port_power_caps local_src_cap_default;
	pd_port_power_caps remote_src_cap;
	pd_port_power_caps remote_snk_cap;

	uint16_t mode_svid;
	uint8_t mode_obj_pos;
	bool modal_operation;
	bool dpm_ack_immediately;

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
	bool dpm_dfp_flow_delay_done;
#endif /* CONFIG_USB_PD_DFP_FLOW_DELAY */

#ifdef CONFIG_USB_PD_UFP_FLOW_DELAY
	bool dpm_ufp_flow_delay_done;
#endif /* CONFIG_USB_PD_UFP_FLOW_DELAY */

	uint32_t dpm_flags;
	uint32_t dpm_init_flags;
	uint32_t dpm_caps;
	uint32_t dpm_dfp_retry_cnt;

	uint8_t dpm_charging_policy;
	uint8_t dpm_charging_policy_default;

/* ALT Mode */
#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
	uint32_t local_dp_config;
	uint32_t remote_dp_config;
	uint8_t dp_ufp_u_attention;
	uint8_t dp_dfp_u_state;
#endif /* CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT */

	uint32_t dp_status;
	uint8_t dp_ufp_u_state;

	uint8_t dp_first_connected;
	uint8_t dp_second_connected;
#endif /* CONFIG_USB_PD_ALT_MODE_SUPPORT */

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	bool uvdm_wait_resp;
	uint8_t uvdm_cnt;
	uint16_t uvdm_svid;
	uint32_t uvdm_data[VDO_MAX_SIZE];
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT
	bool custom_dbgacc;
#endif /* CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT */

#ifdef CONFIG_USB_PD_REV30
#ifdef CONFIG_USB_PD_REV30_ALERT_REMOTE
	uint32_t remote_alert;
#endif

#ifdef CONFIG_USB_PD_REV30_ALERT_LOCAL
	uint32_t local_alert;
#endif

#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL
	uint8_t pd_status_present_in;

#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL_TEMP
	uint8_t pd_status_temp;
	uint8_t pd_status_temp_status;
#endif
#endif

#endif /* CONFIG_USB_PD_REV30 */

#ifdef CONFIG_USB_PD_HANDLE_PRDR_SWAP
	bool postpone_pr_swap;
	bool postpone_dr_swap;
#endif

	struct tcp_dpm_event tcp_event;
	uint8_t tcp_event_id_1st;

#ifdef CONFIG_USB_PD_REV30
	uint8_t local_status[PD_SDB_SIZE];
	uint8_t local_pps_status[PD_PPSDB_SIZE];

	uint8_t remote_status[PD_SDB_SIZE];
	uint8_t remote_pps_status[PD_PPSDB_SIZE];
#endif
} pd_port_t;

int hisi_pd_core_init(struct tcpc_device *tcpc_dev);
int pd_is_auto_discover_cable_id(pd_port_t *pd_port);

static inline int pd_is_support_modal_operation(pd_port_t *pd_port)
{
	if (!(pd_port->id_vdos[0] & PD_IDH_MODAL_SUPPORT))
		return false;

	return (pd_port->svid_data_cnt > 0);
}

/* new definitions */

#define PD_RX_CAP_PE_IDLE		0
#define PD_RX_CAP_PE_DISABLE		(TCPC_RX_CAP_HARD_RESET)
#define PD_RX_CAP_PE_STARTUP		(TCPC_RX_CAP_HARD_RESET)
#define PD_RX_CAP_PE_HARDRESET		0
#define PD_RX_CAP_PE_SEND_WAIT_CAP	(TCPC_RX_CAP_HARD_RESET | TCPC_RX_CAP_SOP)
#define PD_RX_CAP_PE_DISCOVER_CABLE	(TCPC_RX_CAP_HARD_RESET | TCPC_RX_CAP_SOP_PRIME)
#define PD_RX_CAP_PE_READY_UFP		(TCPC_RX_CAP_HARD_RESET | TCPC_RX_CAP_SOP)

#ifdef CONFIG_PD_DISCOVER_CABLE_ID
#define PD_RX_CAP_PE_READY_DFP	(TCPC_RX_CAP_HARD_RESET | \
		TCPC_RX_CAP_SOP | TCPC_RX_CAP_SOP_PRIME  | TCPC_RX_CAP_SOP_PRIME_PRIME)
#else
#define PD_RX_CAP_PE_READY_DFP	(TCPC_RX_CAP_HARD_RESET | \
		TCPC_RX_CAP_SOP | TCPC_RX_CAP_SOP_PRIME_PRIME)
#endif

enum {
	PD_BIST_MODE_DISABLE = 0,
	PD_BIST_MODE_EVENT_PENDING,
	PD_BIST_MODE_TEST_DATA,
};

void hisi_pd_reset_svid_data(pd_port_t *pd_port);
int hisi_pd_reset_protocol_layer(pd_port_t *pd_port);

int hisi_pd_set_rx_enable(pd_port_t *pd_port, uint8_t enable);
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
void hisi_pd_set_vbus_detect(pd_port_t *pd_port, bool enable);
#endif
int hisi_pd_enable_vbus_valid_detection(pd_port_t *pd_port, bool wait_valid);
int hisi_pd_enable_vbus_safe0v_detection(pd_port_t *pd_port);
int hisi_pd_enable_vbus_stable_detection(pd_port_t *pd_port);

uint32_t hisi_pd_reset_pdo_power(uint32_t pdo, uint32_t imax);

void hisi_pd_extract_rdo_power(
	uint32_t rdo, uint32_t pdo, uint32_t *op_curr, uint32_t *max_curr);

void hisi_pd_extract_pdo_power(uint32_t pdo,
	uint32_t *vmin, uint32_t *vmax, uint32_t *ioper);
bool pd_is_source_support_apdo(pd_port_t *pd_port);
uint32_t hisi_pd_extract_cable_curr(uint32_t vdo);


int hisi_pd_set_data_role(pd_port_t *pd_port, uint8_t dr);
int hisi_pd_set_power_role(pd_port_t *pd_port, uint8_t pr);
int hisi_pd_init_role(pd_port_t *pd_port, uint8_t pr, uint8_t dr, bool vr);

int pd_set_cc_res(pd_port_t *pd_port, int pull);
int hisi_pd_set_vconn(pd_port_t *pd_port, int enable);
int hisi_pd_reset_local_hw(pd_port_t *pd_port);

int hisi_pd_enable_bist_test_mode(pd_port_t *pd_port, bool en);

void hisi_pd_lock_msg_output(pd_port_t *pd_port);
void hisi_pd_unlock_msg_output(pd_port_t *pd_port);

int hisi_pd_update_connect_state(pd_port_t *pd_port, uint8_t state);

/* ---- PD notify TCPC Policy Engine State Changed ---- */

void hisi_pd_try_put_pe_idle_event(pd_port_t *pd_port);
void hisi_pd_notify_pe_transit_to_default(pd_port_t *pd_port);
void hisi_pd_notify_pe_hard_reset_completed(pd_port_t *pd_port);
void hisi_pd_notify_pe_send_hard_reset(pd_port_t *pd_port);
void hisi_pd_notify_pe_running(pd_port_t *pd_port);
void hisi_pd_notify_pe_idle(pd_port_t *pd_port);
void hisi_pd_notify_pe_wait_vbus_once(pd_port_t *pd_port, int wait_evt);
void hisi_pd_notify_pe_error_recovery(pd_port_t *pd_port);
void hisi_pd_notify_pe_execute_pr_swap(pd_port_t *pd_port, bool start_swap);
void hisi_pd_notify_pe_cancel_pr_swap(pd_port_t *pd_port);
void hisi_pd_notify_pe_reset_protocol(pd_port_t *pd_port);
void hisi_pd_noitfy_pe_bist_mode(pd_port_t *pd_port, uint8_t mode);
void hisi_pd_notify_pe_pr_changed(pd_port_t *pd_port);
void hisi_pd_notify_pe_src_explicit_contract(pd_port_t *pd_port);
void hisi_pd_notify_pe_transmit_msg(pd_port_t *pd_port, uint8_t type);
void hisi_pd_notify_pe_recv_ping_event(pd_port_t *pd_port);

void pd_notify_tcp_event_buf_reset(pd_port_t *pd_port, uint8_t reason);
void pd_notify_tcp_event_1st_result(pd_port_t *pd_port, int ret);
void pd_notify_tcp_event_2nd_result(pd_port_t *pd_port, int ret);
void pd_init_spec_revision(pd_port_t *pd_port);

/* ---- pd_timer ---- */

static inline void pd_restart_timer(pd_port_t *pd_port, uint32_t timer_id)
{
	hisi_tcpc_restart_timer(pd_port->tcpc_dev, timer_id);
}

static inline void pd_enable_timer(pd_port_t *pd_port, uint32_t timer_id)
{
	hisi_tcpc_enable_timer(pd_port->tcpc_dev, timer_id);
}

static inline void pd_disable_timer(pd_port_t *pd_port, uint32_t timer_id)
{
	hisi_tcpc_disable_timer(pd_port->tcpc_dev, timer_id);
}

static inline void pd_reset_pe_timer(pd_port_t *pd_port)
{
	hisi_tcpc_reset_pe_timer(pd_port->tcpc_dev);
}

/* ---- pd_event ---- */

static inline void pd_free_pd_event(pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_free_event(pd_port->tcpc_dev, pd_event);
}

bool pd_put_pe_event(pd_port_t *pd_port, uint8_t pe_event);
bool pd_put_dpm_event(pd_port_t *pd_port, uint8_t event);
bool pd_put_dpm_pd_request_event(pd_port_t *pd_port, uint8_t event);
bool vdm_put_dpm_vdm_request_event(pd_port_t *pd_port, uint8_t event);
bool pd_put_dpm_notify_event(pd_port_t *pd_port, uint8_t notify);
bool pd_put_dpm_ack_event(pd_port_t *pd_port);
bool pd_put_dpm_nak_event(pd_port_t *pd_port, uint8_t notify);
bool vdm_put_hw_event(struct tcpc_device *tcpc_dev, uint8_t hw_event);
bool vdm_put_dpm_event(pd_port_t *pd_port, uint8_t dpm_event, pd_msg_t *pd_msg);
bool vdm_put_dpm_discover_cable_event(pd_port_t *pd_port);
bool pd_put_hw_event(struct tcpc_device *tcpc_dev, uint8_t hw_event);
bool pd_put_cc_attached_event(struct tcpc_device *tcpc_dev, uint8_t type);

/* ---- Handle PD Message ----*/

int hisi_pd_handle_soft_reset(pd_port_t *pd_port, uint8_t state_machine);
int hisi_pd_send_cable_soft_reset(pd_port_t *pd_port);

int hisi_pd_send_soft_reset(pd_port_t *pd_port, uint8_t state_machine);
int hisi_pd_send_hard_reset(pd_port_t *pd_port);
int hisi_pd_send_bist_mode2(pd_port_t *pd_port);
int hisi_pd_disable_bist_mode2(pd_port_t *pd_port);

/* ---- Send PD Message ----*/
int pd_send_message(pd_port_t *pd_port, uint8_t sop_type,
		uint8_t msg, uint16_t count, const uint32_t *data);

int hisi_pd_send_ctrl_msg(pd_port_t *pd_port,
		uint8_t sop_type, uint8_t msg);

int hisi_pd_send_data_msg(pd_port_t *pd_port, uint8_t sop_type,
		uint8_t msg, uint8_t cnt, uint32_t *payload);

static inline void *pd_get_msg_data_payload(pd_port_t *pd_port)
{
	return pd_port->pd_msg_data_payload;
}

static inline uint8_t pd_get_msg_data_count(pd_port_t *pd_port)
{
	return pd_port->pd_msg_data_count;
}

static inline uint16_t pd_get_msg_data_size(pd_port_t *pd_port)
{
	return pd_port->pd_msg_data_size;
}

static inline uint8_t pd_get_msg_hdr_rev(pd_port_t *pd_port)
{
	return PD_HEADER_REV(pd_port->curr_msg_hdr);
}

static inline uint8_t pd_get_msg_hdr_cnt(pd_port_t *pd_port)
{
	return PD_HEADER_CNT(pd_port->curr_msg_hdr);
}

#ifdef CONFIG_USB_PD_REV30
int pd_send_ext_msg(pd_port_t *pd_port,
		uint8_t sop_type, uint8_t msg, bool request,
		uint8_t chunk_nr, uint8_t size, uint8_t *data);

int pd_send_status(pd_port_t *pd_port);
uint8_t *pd_get_ext_msg_payload(pd_event_t *pd_event);

static inline uint8_t pd_get_msg_hdr_ext(pd_port_t *pd_port)
{
	return PD_HEADER_EXT(pd_port->curr_msg_hdr);
}
#endif

static inline void hisi_pd_cable_flag_clear(pd_port_t *pd_port)
{
	pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_CABLE_ID_DFP;
#ifdef CONFIG_USB_PD_RESET_CABLE
	pd_port->reset_cable = false;
	pd_port->detect_emark = false;
	pd_port->vswap_ret = 0;
#endif
}

static inline void hisi_pd_cable_flag_set(pd_port_t *pd_port)
{
#ifdef CONFIG_USB_PD_RESET_CABLE
	pd_port->reset_cable = true;
	pd_port->detect_emark = true;
	pd_port->vswap_ret = 0;
#endif
}

/* Auto enable timer if success */
int hisi_pd_send_svdm_request(pd_port_t *pd_port,
		uint8_t sop_type, uint16_t svid, uint8_t vdm_cmd,
		uint8_t obj_pos, uint8_t cnt, uint32_t *data_obj,
		uint32_t timer_id);

int hisi_pd_reply_svdm_request(pd_port_t *pd_port, pd_event_t *pd_event,
		uint8_t reply, uint8_t cnt, uint32_t *data_obj);

static inline int pd_send_vdm_discover_id(pd_port_t *pd_port, uint8_t sop_type)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, USB_SID_PD,
		CMD_DISCOVER_IDENT, 0, 0, NULL, PD_TIMER_VDM_RESPONSE);
}

static inline int pd_send_vdm_discover_svids(
	pd_port_t *pd_port, uint8_t sop_type)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, USB_SID_PD,
		CMD_DISCOVER_SVID, 0, 0, NULL, PD_TIMER_VDM_RESPONSE);
}

static inline int pd_send_vdm_discover_modes(
	pd_port_t *pd_port, uint8_t sop_type, uint16_t svid)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, svid,
		CMD_DISCOVER_MODES, 0, 0, NULL, PD_TIMER_VDM_RESPONSE);
}

static inline int pd_send_vdm_enter_mode(pd_port_t *pd_port, uint8_t sop_type,
		uint16_t svid, uint8_t obj_pos)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, svid,
		CMD_ENTER_MODE, obj_pos, 0, NULL, PD_TIMER_VDM_MODE_ENTRY);
}

static inline int pd_send_vdm_exit_mode(pd_port_t *pd_port, uint8_t sop_type,
		uint16_t svid, uint8_t obj_pos)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, svid,
		CMD_EXIT_MODE, obj_pos, 0, NULL, PD_TIMER_VDM_MODE_EXIT);
}

static inline int pd_send_vdm_attention(pd_port_t *pd_port, uint8_t sop_type,
		uint16_t svid, uint8_t obj_pos)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, svid,
		CMD_ATTENTION, obj_pos, 0, NULL, 0);
}

static inline int pd_send_vdm_dp_attention(pd_port_t *pd_port, uint8_t sop_type,
		uint8_t obj_pos, uint32_t dp_status)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, USB_SID_DISPLAYPORT,
		CMD_ATTENTION, obj_pos, 1, &dp_status, 0);
}

static inline int pd_send_vdm_dp_status(pd_port_t *pd_port, uint8_t sop_type,
		uint8_t obj_pos, uint8_t cnt, uint32_t *data_obj)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, USB_SID_DISPLAYPORT,
		CMD_DP_STATUS, obj_pos, cnt, data_obj, PD_TIMER_VDM_RESPONSE);
}

static inline int pd_send_vdm_dp_config(pd_port_t *pd_port, uint8_t sop_type,
		uint8_t obj_pos, uint8_t cnt, uint32_t *data_obj)
{
	return hisi_pd_send_svdm_request(pd_port, sop_type, USB_SID_DISPLAYPORT,
		CMD_DP_CONFIG, obj_pos, cnt, data_obj, PD_TIMER_VDM_RESPONSE);
}

static inline int hisi_pd_reply_svdm_request_simply(pd_port_t *pd_port,
		pd_event_t *pd_event, uint8_t reply)
{
	return hisi_pd_reply_svdm_request(pd_port, pd_event, reply, 0, NULL);
}

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
static inline int pd_send_uvdm(pd_port_t *pd_port, uint8_t sop_type)
{
	return hisi_pd_send_data_msg(pd_port, sop_type, PD_DATA_VENDOR_DEF,
			pd_port->uvdm_cnt, pd_port->uvdm_data);
}

static inline int pd_reply_uvdm(pd_port_t *pd_port, uint8_t sop_type,
		uint8_t cnt, uint32_t *payload)
{
	return hisi_pd_send_data_msg(pd_port, sop_type, PD_DATA_VENDOR_DEF,
			cnt, payload);
}
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

#ifdef CONFIG_ADAPTER_PROTOCOL_UVDM
static inline void hisi_dfp_uvdm_receive_data(pd_event_t *pd_event)
{
	if (pd_event && pd_event->pd_msg)
		power_event_bnc_notify(POWER_BNT_UVDM, POWER_NE_UVDM_RECEIVE, pd_event->pd_msg->payload);
}
#else
static inline void hisi_dfp_uvdm_receive_data(pd_event_t *pd_event)
{
}
#endif /* CONFIG_ADAPTER_PROTOCOL_UVDM */

#ifdef CONFIG_USB_PD_REV30
void pd_sync_sop_spec_revision(pd_port_t *pd_port, uint8_t rev);
void pd_sync_sop_prime_spec_revision(pd_port_t *pd_port, uint8_t rev);
#endif
#endif /* PD_CORE_H_ */
