/*
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * Power Delivery Policy Engine for SNK
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

#define LOG_TAG "[PE_SNK]"

#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#include "include/pd_dpm_core.h"
#include "include/tcpci.h"
#include "include/pd_policy_engine.h"

/*
 * [PD2.0] Figure 8-39 Sink Port state diagram
 */

void hisi_pe_snk_startup_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint8_t rx_cap = PD_RX_CAP_PE_STARTUP;

#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
	uint8_t msg_id_last = pd_port->msg_id_rx[TCPC_TX_SOP];
#endif

	pd_port->request_i = -1;
	pd_port->request_v = TCPC_VBUS_SINK_5V;
	pd_port->state_machine = PE_STATE_MACHINE_SINK;

	hisi_pd_reset_protocol_layer(pd_port);
	hisi_pd_cable_flag_clear(pd_port);

	switch (pd_event->event_type) {
	case PD_EVT_HW_MSG: /* CC attached */
		pd_put_pe_event(pd_port, PD_PE_RESET_PRL_COMPLETED);
		break;

	case PD_EVT_PE_MSG: /* From Hard-Reset */
		hisi_pd_enable_vbus_valid_detection(pd_port, false);
		break;

	case PD_EVT_CTRL_MSG: /* From PR-SWAP (Received PS_RDY) */
		/*
		 * If we reset rx_cap in here,
		 * maybe can't meet tSwapSink (Check it later)
		 */
		if (!pd_dpm_check_vbus_valid(pd_port)) {
			PE_INFO("rx_cap_on\r\n");
			rx_cap = PD_RX_CAP_PE_SEND_WAIT_CAP;
		}

#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
		pd_port->msg_id_pr_swap_last = msg_id_last;
#endif

		pd_put_pe_event(pd_port, PD_PE_RESET_PRL_COMPLETED);
		pd_free_pd_event(pd_port, pd_event);
		break;

	default:
		break;
	}

	hisi_pd_set_rx_enable(pd_port, rx_cap);
}

void hisi_pe_snk_discovery_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_USB_PD_SNK_HRESET_KEEP_DRAW
	/*
	 * iSafe0mA: Maximum current a Sink
	 * is allowed to draw when VBUS is driven to vSafe0V
	 */
	if (pd_port->tcpc_dev->pd_wait_hard_reset_complete)
		pd_dpm_sink_vbus(pd_port, false);
#endif

	hisi_pd_enable_vbus_valid_detection(pd_port, true);
}

void hisi_pe_snk_wait_for_capabilities_entry(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_USB_PD_SNK_HRESET_KEEP_DRAW
	/* Default current draw after HardReset */
	if (pd_port->tcpc_dev->pd_wait_hard_reset_complete)
		pd_dpm_sink_vbus(pd_port, true);
#endif

	hisi_pd_notify_pe_hard_reset_completed(pd_port);

	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_SEND_WAIT_CAP);
	pd_enable_timer(pd_port, PD_TIMER_SINK_WAIT_CAP);
	pd_free_pd_event(pd_port, pd_event);
}

void hisi_pe_snk_wait_for_capabilities_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_disable_timer(pd_port, PD_TIMER_SINK_WAIT_CAP);
}

/*
 * called when cap message received
 */
void hisi_pe_snk_evaluate_capability_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
	pd_port->msg_id_pr_swap_last = 0xff;
#endif
	pd_disable_timer(pd_port, PD_TIMER_NO_RESPONSE);

#ifdef CONFIG_USB_PD_REV30
	if (!pd_port->pd_prev_connected)
		pd_sync_sop_spec_revision(pd_port,
			PD_HEADER_REV(pd_event->pd_msg->msg_hdr));
#endif
	pd_port->hard_reset_counter = 0;
	pd_port->pd_connected = 1;
	pd_port->pd_prev_connected = 1;
	pd_port->explicit_contract = false;

	hisi_pd_dpm_snk_evaluate_caps(pd_port, pd_event);
	pd_free_pd_event(pd_port, pd_event);
}

void hisi_pe_snk_select_capability_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (pd_event->msg == PD_DPM_NOTIFIED)
		PE_DBG("SelectCap%d, rdo:0x%08x\n", pd_event->msg_sec, pd_port->last_rdo);
	else
		PE_DBG("NewReq, rdo:0x%08x\n", pd_port->last_rdo);

	hisi_pd_lock_msg_output(pd_port);

	/* HardReset may at any time, disable vbus detect as soon as possible */
	if (tcpci_tcpc_chip_version_v610(pd_port->tcpc_dev))
		hisi_pd_set_vbus_detect(pd_port, false);

	hisi_pd_send_data_msg(pd_port, TCPC_TX_SOP, PD_DATA_REQUEST, 1, &pd_port->last_rdo);
	pd_free_pd_event(pd_port, pd_event);
}

/*
 * called when accept message received.
 */
void hisi_pe_snk_select_capability_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_disable_timer(pd_port, PD_TIMER_SENDER_RESPONSE);

	if (pd_event_msg_match(pd_event, PD_EVT_CTRL_MSG, PD_CTRL_ACCEPT))
		pd_port->remote_selected_cap = RDO_POS(pd_port->last_rdo);

	/* Waiting for Hard-Reset Done */
	if (!pd_event_msg_match(pd_event, PD_EVT_TIMER_MSG, PD_TIMER_SENDER_RESPONSE))
		hisi_pd_unlock_msg_output(pd_port);
}

void hisi_pe_snk_transition_sink_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_enable_timer(pd_port, PD_TIMER_PS_TRANSITION);

	if (pd_event->msg == PD_CTRL_GOTO_MIN) {
		if (pd_port->dpm_caps & DPM_CAP_LOCAL_GIVE_BACK)
			pd_port->request_i_new = pd_port->request_i_op;
	}

	hisi_pd_dpm_snk_standby_power(pd_port, pd_event);
	pd_free_pd_event(pd_port, pd_event);
}

void hisi_pe_snk_transition_sink_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (pd_event_msg_match(pd_event, PD_EVT_CTRL_MSG, PD_CTRL_PS_RDY))
		hisi_pd_dpm_snk_transition_power(pd_port, pd_event);

	pd_disable_timer(pd_port, PD_TIMER_PS_TRANSITION);
}

void hisi_pe_snk_ready_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (pd_event_msg_match(pd_event, PD_EVT_CTRL_MSG, PD_CTRL_WAIT))
		pd_enable_timer(pd_port, PD_TIMER_SINK_REQUEST);

	pd_port->state_machine = PE_STATE_MACHINE_SINK;
	tcpci_set_frswap(pd_port->tcpc_dev, false);
	hisi_pe_power_ready_entry(pd_port, pd_event);
}

/*
 * This is used for sending a hardreset.
 */
void hisi_pe_snk_hard_reset_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	if (tcpci_tcpc_chip_version_v610(pd_port->tcpc_dev))
		hisi_pd_set_vbus_detect(pd_port, false);

	hisi_pd_send_hard_reset(pd_port);
	pd_free_pd_event(pd_port, pd_event);
}

/*
 * Call this function after hardreset completed.
 */
void hisi_pe_snk_transition_to_default_entry(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_reset_local_hw(pd_port);
	hisi_pd_dpm_snk_hard_reset(pd_port, pd_event);
}

void hisi_pe_snk_transition_to_default_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_enable_timer(pd_port, PD_TIMER_NO_RESPONSE);
}

void hisi_pe_snk_give_sink_cap_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_dpm_send_sink_caps(pd_port);
	pd_free_pd_event(pd_port, pd_event);
}

void hisi_pe_snk_get_source_cap_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_GET_SOURCE_CAP);
}

void hisi_pe_snk_send_soft_reset_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_send_soft_reset(pd_port, PE_STATE_MACHINE_SINK);
	pd_free_pd_event(pd_port, pd_event);
}

void hisi_pe_snk_soft_reset_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_handle_soft_reset(pd_port, PE_STATE_MACHINE_SINK);
	pd_free_pd_event(pd_port, pd_event);
}

#ifdef CONFIG_USB_PD_REV30
void hisi_pe_snk_source_alert_received_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_dpm_inform_alert(pd_port, pd_event);
	pd_free_pd_event(pd_port, pd_event);
}

#ifdef CONFIG_USB_PD_REV30_ALERT_LOCAL
void hisi_pe_snk_send_sink_alert_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_dpm_send_alert(pd_port, pd_event);
}
#endif

#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE
void hisi_pe_snk_get_source_cap_ext_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_port->pd_revision[0] = PD_REV30; /* stub */
	pd_send_message(pd_port, TCPC_TX_SOP, PD_CTRL_GET_SOURCE_CAP_EXT, 0, NULL);
}

void hisi_pe_snk_get_source_cap_ext_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_port->pd_revision[0] = PD_REV20; /* stub */
}
#endif

#ifdef CONFIG_USB_PD_REV30_STATUS_REMOTE
void hisi_pe_snk_get_source_status_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_send_message(pd_port, TCPC_TX_SOP, PD_CTRL_GET_STATUS, 0, NULL);
}

void hisi_pe_snk_get_source_status_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
}

#endif

#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL
void hisi_pe_snk_give_sink_status_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_dpm_send_status(pd_port);
}
#endif

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
void hisi_pe_snk_get_pps_status_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_GET_PPS_STATUS);
}

void hisi_pe_snk_get_pps_status_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
}

void hisi_pe_snk_pps_request(pd_port_t *pd_port)
{
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_GET_PPS_STATUS);
}
#endif /* CONFIG_USB_PD_REV30_PPS_SINK */
#endif /* CONFIG_USB_PD_REV30 */