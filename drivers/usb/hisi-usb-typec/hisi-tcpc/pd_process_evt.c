/*
 * pd_process_evt.c
 *
 * Power Delivery Process Event
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define LOG_TAG "[EVT]"

#include <securec.h>

#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#include "include/tcpci_event.h"
#include "include/pd_process_evt.h"
#include "include/pd_dpm_core.h"


#ifdef CONFIG_HISI_TCOC_DEBUG
/*
 * [BLOCK] print event
 */

#if PE_EVENT_DBG_ENABLE
static const char * const pd_ctrl_msg_name[] = {
	"ctrl0",
	"good_crc",
	"goto_min",
	"accept",
	"reject",
	"ping",
	"ps_rdy",
	"get_src_cap",
	"get_snk_cap",
	"dr_swap",
	"pr_swap",
	"vs_swap",
	"wait",
	"soft_reset",
	"ctrlE",
	"ctrlF",
#ifdef CONFIG_USB_PD_REV30
	"no_support",
	"get_src_cap_ex",
	"get_status",
	"fr_swap",
	"get_pps",
#endif
};

static inline void print_ctrl_msg_event(uint8_t msg)
{
	if (msg < PD_CTRL_MSG_NR) {
		PE_EVT_INFO("%s\n", pd_ctrl_msg_name[msg]);
		D("%s\n", pd_ctrl_msg_name[msg]);
	}
}

static const char * const pd_data_msg_name[] = {
	"data0",
	"src_cap",
	"request",
	"bist",
	"sink_cap",
#ifdef CONFIG_USB_PD_REV30
	"bat_status",
	"alert",
#else
	"data5",
	"data6",
#endif
	"data7",
	"data8",
	"data9",
	"dataA",
	"dataB",
	"dataC",
	"dataD",
	"dataE",
	"vdm",
};

#ifdef CONFIG_USB_PD_REV30
static const char * const pd_ext_msg_name[] = {
	"ext0",
	"src_cap_ex",
	"status",
	"get_bat_cap",
	"get_bat_status",
	"bat_cap",
	"get_mfr_info",
	"mfr_info",
	"sec_request",
	"sec_response",
	"fw_update_request",
	"fw_update_response",
	"pps_status",
};

static inline void print_ext_msg_event(uint8_t msg)
{
	if (msg < PD_EXT_MSG_NR)
		PE_EVT_INFO("%s\r\n", pd_ext_msg_name[msg]);
}
#endif

static inline void print_data_msg_event(uint8_t msg)
{
	if (msg < PD_DATA_MSG_NR) {
		PE_EVT_INFO("%s\n", pd_data_msg_name[msg]);
		D("%s\n", pd_data_msg_name[msg]);
	}
}

static const char * const pd_hw_msg_name[] = {
	"Detached",
	"Attached",
	"hard_reset",
	"vbus_high",
	"vbus_low",
	"vbus_0v",
	"vbus_stable",
	"tx_err",
	"discard",
	"retry_vdm",
};

static inline void print_hw_msg_event(uint8_t msg)
{
	if (msg < PD_HW_MSG_NR) {
		PE_EVT_INFO("%s\n", pd_hw_msg_name[msg]);
		D("HW Message: %s\n", pd_hw_msg_name[msg]);
	}
}

static const char * const pd_pe_msg_name[] = {
	"reset_prl_done",
	"pr_at_dft",
	"hard_reset_done",
	"pe_idle",
	"vdm_reset",
};

static inline void print_pe_msg_event(uint8_t msg)
{
	if (msg < PD_PE_MSG_NR)
		PE_EVT_INFO("%s\n", pd_pe_msg_name[msg]);
}

static const char * const pd_dpm_msg_name[] = {
	"ack",
	"nak",

	"pd_req",
	"vdm_req",
	"cable_req",

	"cap_change",
	"recover",
};

static inline void print_dpm_msg_event(uint8_t msg)
{
	if (msg < PD_DPM_MSG_NR) {
		PE_EVT_INFO("dpm_%s\n", pd_dpm_msg_name[msg]);
		D("dpm_%s\n", pd_dpm_msg_name[msg]);
	}
}

const char * const hisi_pd_dpm_pd_request_name[] = {
	"pr_swap",
	"dr_swap",
	"vs_swap",
	"gotomin",
	"softreset",
	"hardreset",
	"get_src_cap",
	"get_snk_cap",
	"request",
	"bist_cm2",
#ifdef CONFIG_USB_PD_REV30
	"get_src_cap_ex",
	"get_status",
	"fr_swap_snk",
	"fr_swap_src",
	"get_pps",
#endif
};

static void print_dpm_pd_request(uint8_t msg)
{
	if (msg < PD_DPM_PD_REQUEST_NR) {
		PE_EVT_INFO("dpm_pd_req(%s)\n",
				hisi_pd_dpm_pd_request_name[msg]);
		D("dpm_pd_req(%s)\n", hisi_pd_dpm_pd_request_name[msg]);
	}
}
#endif /* PE_EVENT_DBG_ENABLE */

static void print_event(pd_port_t *pd_port, pd_event_t *pd_event)
{
#if PE_EVENT_DBG_ENABLE
	switch (pd_event->event_type) {
	case PD_EVT_CTRL_MSG:
		D("Control Message\n");
		print_ctrl_msg_event(pd_event->msg);
		break;

	case PD_EVT_DATA_MSG:
		D("Data Message\n");
		print_data_msg_event(pd_event->msg);
		break;

#ifdef CONFIG_USB_PD_REV30
	case PD_EVT_EXT_MSG:
		print_ext_msg_event(pd_event->msg);
		break;
#endif

	case PD_EVT_DPM_MSG:
		D("DPM Message\n");
		if (pd_event->msg == PD_DPM_PD_REQUEST)
			print_dpm_pd_request(pd_event->msg_sec);
		else
			print_dpm_msg_event(pd_event->msg);
		break;

	case PD_EVT_HW_MSG:
		D("HW Message\n");
		print_hw_msg_event(pd_event->msg);
		break;

	case PD_EVT_PE_MSG:
		D("PE Message\n");
		print_pe_msg_event(pd_event->msg);
		break;

	case PD_EVT_TIMER_MSG:
		D("Timer message\n");
		PE_EVT_INFO("timer\n");
		break;

	case PD_EVT_TCP_MSG:
		D("TCP message\n");
		break;

	default:
		D("unknown message\n");
		break;
	}
#endif
}
#else /* PE_EVENT_DBG_ENABLE */
static inline void print_event(const pd_port_t *pd_port, const pd_event_t *pd_event) {}
#endif

bool hisi_pd_make_pe_state_transit(pd_port_t *pd_port, uint8_t curr_state,
		const pe_state_reaction_t *state_reaction)
{
	int i;
	const pe_state_transition_t *state_transition =
			state_reaction->state_transition;

	for (i = 0; i < state_reaction->nr_transition; i++) {
		if (state_transition[i].curr_state == curr_state) {
			PE_TRANSIT_STATE(pd_port,
					state_transition[i].next_state);
			return true;
		}
	}

	return false;
}

bool hisi_pd_make_pe_state_transit_virt(pd_port_t *pd_port,
		uint8_t curr_state, const pe_state_reaction_t *state_reaction)
{
	bool ret = false;

	ret = hisi_pd_make_pe_state_transit(
			pd_port, curr_state, state_reaction);
	if (ret) {
		switch (pd_port->pe_state_next) {
		case PE_VIRT_READY:
			PE_TRANSIT_READY_STATE(pd_port);
			break;

		case PE_VIRT_HARD_RESET:
			PE_TRANSIT_HARD_RESET_STATE(pd_port);
			break;

		default:
			D("Ignore\n");
			break;
		}
	}

	return ret;
}

bool hisi_pd_make_pe_state_transit_force(
		pd_port_t *pd_port, uint8_t curr_state, uint8_t force_state,
		const pe_state_reaction_t *state_reaction)
{
	bool ret = NULL;

	ret = hisi_pd_make_pe_state_transit(
		pd_port, curr_state, state_reaction);
	if (ret)
		return ret;

	PE_TRANSIT_STATE(pd_port, force_state);

	return true;
}

bool hisi_pd_make_pe_state_transit_single(pd_port_t *pd_port,
	uint8_t curr_state, uint8_t reaction_state, uint8_t next_state)
{
	if (curr_state == reaction_state)
		PE_TRANSIT_STATE(pd_port, next_state);
	return true;
}

bool hisi_pd_process_protocol_error(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool power_change = false;
	pd_msg_t *pd_msg = pd_event->pd_msg;

	uint8_t event_type = pd_event->event_type;
	uint8_t msg_id = PD_HEADER_ID(pd_msg->msg_hdr);
	uint8_t msg_type = PD_HEADER_TYPE(pd_msg->msg_hdr);

	D("+\n");

	/*
	 * pe_state_curr assigned as pe_pd_state in pd_handle_event
	 * pe_pd_state assigned as new_state after process_event.
	 */
	switch (pd_port->pe_state_curr) {
	case PE_SNK_TRANSITION_SINK:
	case PE_SRC_TRANSITION_SUPPLY:
	case PE_SRC_TRANSITION_SUPPLY2:
		power_change = true;
		/* fall through */
	case PE_PRS_SRC_SNK_WAIT_SOURCE_ON:
		if (pd_event_msg_match(pd_event, PD_EVT_CTRL_MSG,
				PD_CTRL_PING)) {
			PE_DBG("Igrone Ping\n");
			return false;
		}
		break;

	case PE_SRC_SOFT_RESET:
	case PE_SRC_SEND_SOFT_RESET:
	case PE_SNK_SOFT_RESET:
	case PE_SNK_SEND_SOFT_RESET:
	case PE_SNK_READY:
	case PE_SRC_READY:
	case PE_BIST_TEST_DATA:
		PE_DBG("Igrone Unknown Event\n");
		return false;

	default:
		break;
	};

	if (pd_port->tcpc_dev->pd_wait_hard_reset_complete) {
		PE_DBG("Ignore Event during HReset\n");
		return false;
	}

	PE_INFO("PRL_ERR: %u-%u-%u\n", event_type, msg_type, msg_id);

	if (pd_port->during_swap) {
#ifdef CONFIG_USB_PD_PR_SWAP_ERROR_RECOVERY
		PE_TRANSIT_STATE(pd_port, PE_ERROR_RECOVERY);
#else
		PE_TRANSIT_HARD_RESET_STATE(pd_port);
#endif
	} else if (power_change) {
		PE_TRANSIT_HARD_RESET_STATE(pd_port);
	} else {
		PE_TRANSIT_SEND_SOFT_RESET_STATE(pd_port);
	}

	return true;
}

static bool hisi_pd_process_cable_ctrl_msg_accept(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_USB_PD_RESET_CABLE
	switch (pd_port->pe_state_curr) {
#ifdef CONFIG_PD_DFP_RESET_CABLE
	case PE_DFP_CBL_SEND_SOFT_RESET:
		PE_TRANSIT_READY_STATE(pd_port);
		return true;
#endif
	default:
		break;
	}
#endif

	return false;
}

static bool hisi_pd_process_event_cable(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;

#ifdef CONFIG_USB_PD_RESET_CABLE
	if (pd_event->msg == PD_CTRL_ACCEPT)
		ret = hisi_pd_process_cable_ctrl_msg_accept(pd_port, pd_event);
#endif
	return ret;
}

bool hisi_pd_process_data_msg_bist(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (!pd_check_pe_state_ready(pd_port))
		return false;

	if (pd_port->request_v > 5000) { /* 5000: Vbus: Vsafe_5V */
		PE_INFO("bist_not_vsafe5v\n");
		return false;
	}

	switch (BDO_MODE(pd_event->pd_msg->payload[0])) {
	case BDO_MODE_TEST_DATA:
		D("bist_test\n");
		PE_TRANSIT_STATE(pd_port, PE_BIST_TEST_DATA);
		hisi_pd_noitfy_pe_bist_mode(pd_port, PD_BIST_MODE_TEST_DATA);
		return true;

	case BDO_MODE_CARRIER2:
		D("bist_cm2\n");
		PE_TRANSIT_STATE(pd_port, PE_BIST_CARRIER_MODE_2);
		hisi_pd_noitfy_pe_bist_mode(pd_port, PD_BIST_MODE_DISABLE);
		return true;

	default:
	case BDO_MODE_RECV:
	case BDO_MODE_TRANSMIT:
	case BDO_MODE_COUNTERS:
	case BDO_MODE_CARRIER0:
	case BDO_MODE_CARRIER1:
	case BDO_MODE_CARRIER3:
	case BDO_MODE_EYE:
		D("Unsupport BIST, disable RX\n");
		hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_DISABLE);
		hisi_pd_noitfy_pe_bist_mode(pd_port, PD_BIST_MODE_DISABLE);
		return false;
	}
}

/*
 * DRP (Data Role Swap)
 */

bool hisi_pd_process_ctrl_msg_dr_swap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool reject = false;

	D("\n");
	if (!pd_check_pe_state_ready(pd_port))
		return false;

	reject = !(pd_port->dpm_caps & DPM_CAP_LOCAL_DR_DATA);

	if (!reject) {
		if (pd_port->data_role == PD_ROLE_DFP)
			reject = pd_port->dpm_caps &
					DPM_CAP_DR_SWAP_REJECT_AS_UFP;
		else
			reject = pd_port->dpm_caps &
					DPM_CAP_DR_SWAP_REJECT_AS_DFP;
	}

	if (reject) {
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_REJECT);
		return false;
	}
	if (pd_port->modal_operation) {
		PE_TRANSIT_HARD_RESET_STATE(pd_port);
	} else {
		pd_port->during_swap = false;
		pd_port->state_machine = PE_STATE_MACHINE_DR_SWAP;
		D("state_machine %s\n",
			pd_pe_state_machine_name(pd_port->state_machine));

		PE_TRANSIT_DATA_STATE(pd_port, PE_DRS_UFP_DFP_EVALUATE_DR_SWAP,
				PE_DRS_DFP_UFP_EVALUATE_DR_SWAP);
	}

	return true;
}

/*
 * for issue a dr swap
 */
static bool hisi_pd_process_dpm_msg_dr_swap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (!(pd_port->dpm_caps & DPM_CAP_LOCAL_DR_DATA))
		return false;

	if (!pd_check_pe_state_ready(pd_port))
		return false;

	pd_port->during_swap = false;
	pd_port->state_machine = PE_STATE_MACHINE_DR_SWAP;
	PE_TRANSIT_DATA_STATE(pd_port,
			PE_DRS_UFP_DFP_SEND_DR_SWAP,
			PE_DRS_DFP_UFP_SEND_DR_SWAP);

	return true;
}

/*
 * handle a pr swap request
 */
bool hisi_pd_process_ctrl_msg_pr_swap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool reject = false;

	if (!pd_check_pe_state_ready(pd_port))
		return false;

	reject = !(pd_port->dpm_caps & DPM_CAP_LOCAL_DR_POWER);

	if (!reject) {
		if (pd_port->power_role == PD_ROLE_SOURCE)
			reject = pd_port->dpm_caps &
					DPM_CAP_PR_SWAP_REJECT_AS_SNK;
		else
			reject = pd_port->dpm_caps &
					DPM_CAP_PR_SWAP_REJECT_AS_SRC;
	}

	if (reject) {
		D("reject pr swap!\n");
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_REJECT);
		return false;
	}
	pd_port->during_swap = false;
	pd_port->state_machine = PE_STATE_MACHINE_PR_SWAP;

	/* state machine transmit */
	PE_TRANSIT_POWER_STATE(pd_port,
			PE_PRS_SNK_SRC_EVALUATE_PR_SWAP,
			PE_PRS_SRC_SNK_EVALUATE_PR_SWAP);

	return true;
}

/*
 * for issue a prs request.
 */
static bool hisi_pd_process_dpm_msg_pr_swap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (!(pd_port->dpm_caps & DPM_CAP_LOCAL_DR_POWER))
		return false;

	if (!pd_check_pe_state_ready(pd_port))
		return false;

	pd_port->during_swap = false;
	pd_port->state_machine = PE_STATE_MACHINE_PR_SWAP;

	PE_TRANSIT_POWER_STATE(pd_port, PE_PRS_SNK_SRC_SEND_SWAP,
			PE_PRS_SRC_SNK_SEND_SWAP);

	return true;
}

/*
 * DRP (Vconn Swap)
 */

/* handle a vconn swap request */
bool hisi_pd_process_ctrl_msg_vconn_swap(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	if (!pd_check_pe_state_ready(pd_port))
		return false;

	if (!(pd_port->dpm_caps & DPM_CAP_LOCAL_VCONN_SUPPLY)) {
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_REJECT);
		return false;
	}
	pd_port->state_machine = PE_STATE_MACHINE_VCONN_SWAP;
	PE_TRANSIT_STATE(pd_port, PE_VCS_EVALUATE_SWAP);
	return true;
}

/*
 * for issue a vconn swap
 */
static bool hisi_pd_process_dpm_msg_vconn_swap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (!(pd_port->dpm_caps & DPM_CAP_LOCAL_VCONN_SUPPLY))
		return false;

	if (!pd_check_pe_state_ready(pd_port))
		return false;

	pd_port->state_machine = PE_STATE_MACHINE_VCONN_SWAP;
	PE_TRANSIT_STATE(pd_port, PE_VCS_SEND_SWAP);
	return true;
}

static void hisi_pd_copy_msg_data(pd_port_t *pd_port, uint8_t *payload,
		uint16_t count, uint8_t unit_sz)
{
	pd_port->pd_msg_data_size = count * unit_sz;
	pd_port->pd_msg_data_count = count;
	pd_port->pd_msg_data_payload = payload;
}

#ifdef CONFIG_USB_PD_REV30
static void hisi_pd_copy_msg_data_from_ext_evt(
		pd_port_t *pd_port, pd_msg_t *pd_msg)
{
	uint32_t *payload = pd_msg->payload;
	uint16_t *ext_hdr = (uint16_t *) payload;
	uint8_t *ext_data = (uint8_t *) (ext_hdr + 1);
	uint16_t size = PD_EXT_HEADER_DATA_SIZE(*ext_hdr);

	hisi_pd_copy_msg_data(pd_port, ext_data, size, 1);
}
#endif

static void hisi_pd_copy_msg_data_from_evt(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_msg_t *pd_msg = pd_event->pd_msg;

	switch (pd_event->event_type) {
	case PD_EVT_DATA_MSG:
		hisi_pd_copy_msg_data(pd_port, (uint8_t *)pd_msg->payload,
			pd_get_msg_hdr_cnt(pd_port), sizeof(uint32_t));
		break;

#ifdef CONFIG_USB_PD_REV30
	case PD_EVT_EXT_MSG:
		hisi_pd_copy_msg_data_from_ext_evt(pd_port, pd_msg);
		return;
#endif

	default:
		hisi_pd_copy_msg_data(pd_port, NULL, 0, 0);
	}
}

bool hisi_pd_process_recv_hard_reset(pd_port_t *pd_port,
		pd_event_t *pd_event, uint8_t hreset_state)
{
	PE_TRANSIT_STATE(pd_port, hreset_state);
	return true;
}

static bool hisi_pd_process_dpm_msg_pw_request(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	if (pd_port->pe_state_curr != PE_SNK_READY)
		return false;

	PE_TRANSIT_STATE(pd_port, PE_SNK_SELECT_CAPABILITY);
	return true;
}

static bool hisi_pd_process_dpm_msg_bist_cm2(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint32_t bist = BDO_MODE_CARRIER2;

	if (!pd_check_pe_state_ready(pd_port))
		return false;

	hisi_pd_send_data_msg(pd_port, TCPC_TX_SOP, PD_DATA_BIST, 1, &bist);

	return false;
}

static bool hisi_pd_process_dpm_msg_gotomin(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (pd_port->pe_state_curr != PE_SRC_READY)
		return false;

	if (!(pd_port->dpm_flags & DPM_FLAGS_PARTNER_GIVE_BACK))
		return false;

	PE_TRANSIT_STATE(pd_port, PE_SRC_TRANSITION_SUPPLY);

	return true;
}

static bool hisi_pd_process_dpm_msg_softreset(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (!pd_check_pe_state_ready(pd_port))
		return false;

	PE_TRANSIT_SEND_SOFT_RESET_STATE(pd_port);

	return true;
}

static int hisi_pd_handle_tcp_event_cable_softreset(pd_port_t *pd_port)
{
#ifdef CONFIG_PD_DFP_RESET_CABLE
	if (!pd_check_pe_state_ready(pd_port))
		return false;

	if (pd_port->data_role != PD_ROLE_DFP)
		return false;

	PE_TRANSIT_STATE(pd_port, PE_DFP_CBL_SEND_SOFT_RESET);
#endif
	return true;
}

static bool hisi_pd_process_dpm_msg_hardreset(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (!pd_check_pe_state_ready(pd_port))
		return false;

	PE_TRANSIT_HARD_RESET_STATE(pd_port);

	return true;
}

static bool hisi_pd_process_dpm_msg_get_source_cap(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	switch (pd_port->pe_state_curr) {
	case PE_SNK_READY:
		PE_TRANSIT_STATE(pd_port, PE_SNK_GET_SOURCE_CAP);
		return true;

	case PE_SRC_READY:
		if (pd_port->dpm_caps & DPM_CAP_LOCAL_DR_POWER) {
			PE_TRANSIT_STATE(pd_port, PE_DR_SRC_GET_SOURCE_CAP);
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

static bool hisi_pd_process_dpm_msg_get_sink_cap(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	switch (pd_port->pe_state_curr) {
	case PE_SRC_READY:
		PE_TRANSIT_STATE(pd_port, PE_SRC_GET_SINK_CAP);
		return true;

	case PE_SNK_READY:
		if (pd_port->dpm_caps & DPM_CAP_LOCAL_DR_POWER) {
			PE_TRANSIT_STATE(pd_port, PE_DR_SNK_GET_SINK_CAP);
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

/**
 * DPM event haldler
 */
static bool hisi_pd_process_event_dpm_pd_request(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	bool ret = false;

	switch (pd_event->msg_sec) {
	case PD_DPM_PD_REQUEST_PR_SWAP:
#ifdef CONFIG_USB_PD_HANDLE_PRDR_SWAP
		D("state_machine %s\n",
			pd_pe_state_machine_name(pd_port->state_machine));
		if (pd_port->state_machine == PE_STATE_MACHINE_DR_SWAP) {
			PE_DBG("Postpone pr_swap\n");
			pd_port->postpone_pr_swap = true;
			break;
		}
#endif
		ret = hisi_pd_process_dpm_msg_pr_swap(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_DR_SWAP:
#ifdef CONFIG_USB_PD_HANDLE_PRDR_SWAP
		D("state_machine %s\n",
			pd_pe_state_machine_name(pd_port->state_machine));
		if (pd_port->state_machine == PE_STATE_MACHINE_PR_SWAP) {
			PE_DBG("Postpone dr_swap\n");
			pd_port->postpone_dr_swap = true;
			break;
		}
#endif
		ret = hisi_pd_process_dpm_msg_dr_swap(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_VCONN_SWAP:
		ret = hisi_pd_process_dpm_msg_vconn_swap(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_GOTOMIN:
		ret = hisi_pd_process_dpm_msg_gotomin(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_SOFTRESET:
		ret = hisi_pd_process_dpm_msg_softreset(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_CBL_SOFTRESET:
		ret = hisi_pd_handle_tcp_event_cable_softreset(pd_port);
		break;

	case PD_DPM_PD_REQUEST_HARDRESET:
		ret = hisi_pd_process_dpm_msg_hardreset(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_GET_SOURCE_CAP:
		ret = hisi_pd_process_dpm_msg_get_source_cap(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_GET_SINK_CAP:
		ret = hisi_pd_process_dpm_msg_get_sink_cap(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_PW_REQUEST:
		ret = hisi_pd_process_dpm_msg_pw_request(pd_port, pd_event);
		break;

	case PD_DPM_PD_REQUEST_BIST_CM2:
		ret = hisi_pd_process_dpm_msg_bist_cm2(pd_port, pd_event);
		break;

	default:
		D("Unknown PD_Request\n");
		return false;
	}

	if (!ret) {
		D("Reject DPM PD Request\n");
		return false;
	}

	return ret;
}

static bool pe_is_valid_pd_msg(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_msg_t *pd_msg = pd_event->pd_msg;

	uint8_t event_type = pd_event->event_type;
	uint8_t sop_type = pd_msg->frame_type;
	uint8_t msg_id = PD_HEADER_ID(pd_msg->msg_hdr);
	uint8_t msg_type = PD_HEADER_TYPE(pd_msg->msg_hdr);

	if (pd_port->pe_state_curr == PE_BIST_TEST_DATA)
		return false;

	if (event_type == PD_EVT_CTRL_MSG) {
		switch (msg_type) {
		/* SofReset always has a MessageID value of zero */
		case PD_CTRL_SOFT_RESET:
			if (msg_id != 0) {
				PE_INFO("Repeat soft_reset\n");
				return false;
			}

			return true;
		case PD_CTRL_GOOD_CRC:
			PE_DBG("Discard_CRC\n");
			return true;

#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
		case PD_CTRL_PS_RDY:
			/* Drop duplicated message */
			if (pd_port->msg_id_pr_swap_last == msg_id) {
				PE_INFO("Repeat ps_rdy\n");
				return false;
			}
			break;
#endif
		default:
			break;
		}
	}

	/* Drop duplicated message */
	if ((pd_port->msg_id_rx_init[sop_type]) &&
			(pd_port->msg_id_rx[sop_type] == msg_id)) {
		PE_INFO("Repeat msg: %c:%u:%u\n",
			(pd_event->event_type == PD_EVT_CTRL_MSG) ? 'C' : 'D',
					pd_event->msg, msg_id);
		return false;
	}

	pd_port->msg_id_rx[sop_type] = msg_id;
	pd_port->msg_id_rx_init[sop_type] = true;

	return true;
}


static bool pe_is_valid_pd_msg_role(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	bool ret = true;
	uint8_t msg_pr, msg_dr;
	pd_msg_t *pd_msg = pd_event->pd_msg;

	if (!pd_msg) /* Good-CRC */
		return true;

	if (pd_msg->frame_type != TCPC_TX_SOP)
		return true;

	msg_pr = PD_HEADER_PR(pd_msg->msg_hdr);
	msg_dr = PD_HEADER_DR(pd_msg->msg_hdr);

	/*
	 * The Port Power Role field of a received Message shall not be verified
	 * by the receiver and no error recovery action shall be
	 * taken if it is incorrect.
	 */

	if (msg_pr == pd_port->power_role)
		PE_INFO("Wrong PR:%u\n", msg_pr);
	/*
	 * Should a Type-C Port receive a Message with the Port Data Role field
	 * set to the same Data Role as its current Data Role,
	 * except for the GoodCRC Message,
	 * Type-C error recovery actions as defined
	 * in [USBType-C 1.0] shall be performed.
	 */

	if (msg_dr == pd_port->data_role) {
#ifdef CONFIG_USB_PD_CHECK_DATA_ROLE
		ret = false;
#endif
		PE_INFO("Wrong DR:%u\n", msg_dr);
	}

	return ret;
}

static bool pe_translate_pd_msg_event(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	pd_msg_t *pd_msg = NULL;
	bool ret = false;

	if (pd_event->event_type != PD_EVT_PD_MSG)
		return true;

	pd_msg = pd_event->pd_msg;
	pd_port->curr_msg_hdr = pd_msg->msg_hdr;

	if (PD_HEADER_CNT(pd_msg->msg_hdr))
		pd_event->event_type = PD_EVT_DATA_MSG;
	else
		pd_event->event_type = PD_EVT_CTRL_MSG;

	pd_event->msg = PD_HEADER_TYPE(pd_msg->msg_hdr);

#ifdef CONFIG_USB_PD_REV30
	if (PD_HEADER_EXT(pd_msg->msg_hdr)) {
		D("ext msg\n");
		pd_event->event_type = PD_EVT_EXT_MSG;
	}

	if (pd_msg->frame_type == TCPC_TX_SOP_PRIME)
		pd_sync_sop_prime_spec_revision(
				pd_port, PD_HEADER_REV(pd_msg->msg_hdr));
#endif

	ret = pe_is_valid_pd_msg(pd_port, pd_event);
	return ret;
}

static bool pe_exit_idle_state(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool act_as_sink = false;
	errno_t err;

	D("+\n");
#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT
	pd_port->custom_dbgacc = false;
#endif

	switch (pd_event->msg_sec) {
	case TYPEC_ATTACHED_SNK:
		act_as_sink = true;
		break;

	case TYPEC_ATTACHED_SRC:
		act_as_sink = false;
		break;

#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT
	case TYPEC_ATTACHED_DBGACC_SNK:
		act_as_sink = true;
		pd_port->custom_dbgacc = true;
		break;
#endif

	default:
		return false;
	}

	pd_init_spec_revision(pd_port);

	if (act_as_sink)
		hisi_pd_init_role(pd_port, PD_ROLE_SINK,
			PD_ROLE_UFP, PD_ROLE_VCONN_OFF);
	else
		hisi_pd_init_role(pd_port, PD_ROLE_SOURCE,
			PD_ROLE_DFP, PD_ROLE_VCONN_ON);

	pd_port->cap_counter = 0;
	pd_port->discover_id_counter = 0;
	pd_port->hard_reset_counter = 0;
	pd_port->get_snk_cap_count = 0;
	pd_port->get_src_cap_count = 0;
	pd_port->vdm_discard_retry_count = 0;

	pd_port->pe_ready = 0;
	pd_port->pd_connected = 0;
	pd_port->pd_prev_connected = 0;
	D("set pd_prev_connected %d\n", pd_port->pd_prev_connected);
	pd_port->reset_vdm_state = 0;
	pd_port->power_cable_present = 0;

	pd_port->explicit_contract = false;
	pd_port->invalid_contract = false;

	pd_port->modal_operation = false;
	pd_port->during_swap = false;
	pd_port->dpm_ack_immediately = false;

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY_STARTUP
	pd_port->dpm_dfp_flow_delay_done = false;
#else
	pd_port->dpm_dfp_flow_delay_done = true;
#endif /* CONFIG_USB_PD_DFP_FLOW_DELAY_STARTUP */
#endif /* CONFIG_USB_PD_DFP_FLOW_DELAY */

#ifdef CONFIG_USB_PD_UFP_FLOW_DELAY
	pd_port->dpm_ufp_flow_delay_done = false;
#endif /* CONFIG_USB_PD_UFP_FLOW_DELAY */

	pd_port->remote_src_cap.nr = 0;
	pd_port->remote_snk_cap.nr = 0;

#ifdef CONFIG_USB_PD_HANDLE_PRDR_SWAP
	pd_port->postpone_pr_swap = false;
	pd_port->postpone_dr_swap = false;
#endif /* CONFIG_USB_PD_HANDLE_PRDR_SWAP */

	err = memset_s(pd_port->cable_vdos,
			sizeof(uint32_t) * VDO_MAX_SIZE,
			0,
			sizeof(uint32_t) * VDO_MAX_SIZE);
	if (err)
		E("clr cable vdo\n");

#ifdef CONFIG_USB_PD_REV30
	err = memset_s(pd_port->local_status, PD_SDB_SIZE, 0, PD_SDB_SIZE);
	err += memset_s(pd_port->remote_status, PD_SDB_SIZE, 0, PD_SDB_SIZE);
#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	pd_port->request_apdo_pos = 0;
	err = memset_s(pd_port->remote_pps_status, PD_PPSDB_SIZE, 0, PD_PPSDB_SIZE);
#endif
	if (err)
		E("init status\n");
#endif

	hisi_pd_notify_pe_running(pd_port);
	hisi_pd_dpm_notify_pe_startup(pd_port);

	D("-\n");
	return true;
}

static bool pe_is_trap_in_idle_state(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	bool trap = true;

	switch (pd_port->pe_state_curr) {
	case PE_IDLE1:
		if (pd_event_msg_match(pd_event, PD_EVT_PE_MSG, PD_PE_IDLE))
			return false;
		hisi_pd_try_put_pe_idle_event(pd_port);
		/* fallthrough */
	case PE_IDLE2:
		/*
		 * for mqp test:
		 * receive vdm after just system startup,
		 * have not exit idle yet!
		 */
		if ((pd_event->event_type == PD_EVT_PE_MSG) &&
				(hisi_pd_convert_through_u8(pd_event->msg) ==
				PD_PE_VDM_RESET)) {
			D("vdm reset, exit idle\n");
			return false;
		}
		break;

	default:
		return false;
	}

	if (pd_event->event_type == PD_EVT_HW_MSG) {
		switch (hisi_pd_convert_through_u8(pd_event->msg)) {
		case PD_HW_CC_ATTACHED:
			trap = false;
			break;

		case PD_HW_CC_DETACHED:
			hisi_pd_notify_pe_idle(pd_port);
			break;

		default:
			break;
		}
	}

	if (!trap)
		trap = !pe_exit_idle_state(pd_port, pd_event);

	return trap;
}

static bool __hisi_pd_process_event(pd_port_t *pd_port, pd_event_t *pd_event,
		bool vdm_evt)
{
	bool ret = false;

	hisi_pd_copy_msg_data_from_evt(pd_port, pd_event);

	/* vdm route */
	if (vdm_evt) {
		D("vdm event\n");
		return hisi_pd_process_event_vdm(pd_port, pd_event);
	}

	if (pd_event->event_type == PD_EVT_TCP_MSG)
		return hisi_pd_process_event_tcp(pd_port, pd_event);

#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT
	if (pd_port->custom_dbgacc) {
		D("custom dbgacc\n");
		return hisi_pd_process_event_dbg(pd_port, pd_event);
	}
#endif

	if ((pd_event->event_type == PD_EVT_CTRL_MSG) &&
		(pd_event->msg != PD_CTRL_GOOD_CRC) &&
			(pd_event->pd_msg->frame_type != TCPC_TX_SOP)) {
		return hisi_pd_process_event_cable(pd_port, pd_event);
	}

	/* Special DPM event route */
	if (pd_event_msg_match(pd_event, PD_EVT_DPM_MSG, PD_DPM_PD_REQUEST)) {
		D("PD_DPM_PD_REQUEST\n");
		return hisi_pd_process_event_dpm_pd_request(pd_port, pd_event);
	}

	switch (pd_port->state_machine) {
	case PE_STATE_MACHINE_DR_SWAP:
		ret = hisi_pd_process_event_drs(pd_port, pd_event);
		break;

	case PE_STATE_MACHINE_PR_SWAP:
		ret = hisi_pd_process_event_prs(pd_port, pd_event);
		break;

	case PE_STATE_MACHINE_VCONN_SWAP:
		ret = hisi_pd_process_event_vcs(pd_port, pd_event);
		break;

	default:
		break;
	}

	if (ret)
		return true; /* means processed sucessfully */

	/* power_role initialized in hisi_pd_init_role, when exit idle */
	if (pd_port->power_role == PD_ROLE_SINK)
		ret = hisi_pd_process_event_snk(pd_port, pd_event);
	else
		ret = hisi_pd_process_event_src(pd_port, pd_event);

	return ret;
}

bool hisi_pd_process_event(pd_port_t *pd_port, pd_event_t *pd_event,
		bool vdm_evt)
{
	bool ret = false;

	if (pe_is_trap_in_idle_state(pd_port, pd_event)) {
		D("Trap in idle state, Igrone All MSG\n");
		return false;
	}

	/* control message or data message ? */
	if (!pe_translate_pd_msg_event(pd_port, pd_event))
		return false;

	print_event(pd_port, pd_event);

	switch (pd_event->event_type) {
	case PD_EVT_CTRL_MSG:
	case PD_EVT_DATA_MSG:
		/* check message legality */
		if (!pe_is_valid_pd_msg_role(pd_port, pd_event)) {
			PE_TRANSIT_STATE(pd_port, PE_ERROR_RECOVERY);
			return true;
		}
		break;

	default:
		break;
	}

	ret = __hisi_pd_process_event(pd_port, pd_event, vdm_evt);

	return ret;
}
