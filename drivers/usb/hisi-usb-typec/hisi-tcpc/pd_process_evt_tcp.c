/*
 * pd_process_evt_tcp.c
 *
 * Tcp event process program
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#include "include/tcpci_event.h"
#include "include/pd_process_evt.h"
#include "include/pd_dpm_core.h"

#ifdef CONFIG_USB_PD_REV30
static int pd_handle_tcp_event_gotomin(pd_port_t *pd_port)
{
	if (pd_port->pe_state_curr != PE_SRC_READY)
		return -1;

	if (!(pd_port->dpm_flags & DPM_FLAGS_PARTNER_GIVE_BACK))
		return -1;

	PE_TRANSIT_STATE(pd_port, PE_SRC_TRANSITION_SUPPLY);
	return 0;
}

static int pd_handle_tcp_event_request(pd_port_t *pd_port)
{
	int ret = 0;
	struct tcp_dpm_event *tcp_event = &pd_port->tcp_event;

	if (pd_port->pe_state_curr != PE_SNK_READY)
		return -1;

	D("msg type: %u\n", tcp_event->event_id);
	switch (tcp_event->event_id) {
	case TCP_DPM_EVT_REQUEST:
		ret = pd_dpm_update_tcp_request(
			pd_port, &tcp_event->tcp_dpm_data.pd_req);
		break;
	case TCP_DPM_EVT_REQUEST_EX:
		ret = pd_dpm_update_tcp_request_ex(
			pd_port, &tcp_event->tcp_dpm_data.pd_req_ex);
		break;
	case TCP_DPM_EVT_REQUEST_AGAIN:
		ret = pd_dpm_update_tcp_request_again(pd_port);
		break;

	default:
		D("not match\n");
	}

	if (ret != 0)
		return ret;

	PE_TRANSIT_STATE(pd_port, PE_SNK_SELECT_CAPABILITY);
	return ret;
}

#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE
static int pd_handle_tcp_event_get_source_cap_ext(pd_port_t *pd_port)
{
	switch (pd_port->pe_state_curr) {
	case PE_SNK_READY:
		PE_TRANSIT_STATE(pd_port, PE_SNK_GET_SOURCE_CAP_EXT);
		return TCP_DPM_RET_SENT;

	default:
		return TCP_DPM_RET_DENIED_LOCAL_CAP;
	}

	return TCP_DPM_RET_DENIED_NOT_READY;
}
#endif

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
int pd_handle_tcp_event_get_pps_status(pd_port_t *pd_port)
{
	if (pd_port->pe_state_curr != PE_SNK_READY)
		return -1;

	PE_TRANSIT_STATE(pd_port, PE_SNK_GET_PPS_STATUS);
	return 0;
}
#endif

static int pd_make_tcp_event_transit_ready(
	pd_port_t *pd_port, uint8_t snk_state, uint8_t src_state)
{
	switch (pd_port->pe_state_curr) {
	case PE_SNK_READY:
		PE_TRANSIT_STATE(pd_port, snk_state);
		return 0;

	case PE_SRC_READY:
		PE_TRANSIT_STATE(pd_port, src_state);
		return 0;

	default:
		break;
	}

	return -1;
}

#ifdef CONFIG_USB_PD_REV30_ALERT_LOCAL
static int pd_handle_tcp_event_alert(pd_port_t *pd_port, pd_event_t *pd_event)
{
	struct tcp_dpm_event *tcp_event = &pd_port->tcp_event;

	if (pd_event->msg_sec == PD_TCP_FROM_TCPM)
		pd_port->local_alert |= tcp_event->tcp_dpm_data.index;

	return pd_make_tcp_event_transit_ready(pd_port,
			PE_SNK_SEND_SINK_ALERT, PE_SRC_SEND_SOURCE_ALERT);
}
#endif
#endif /* CONFIG_USB_PD_REV30 */

static int pd_handle_tcp_dpm_event(
	pd_port_t *pd_port, pd_event_t *pd_event)
{
	int ret = -1;

#ifdef CONFIG_USB_PD_REV30
	D("pd_event msg: %u\n", pd_event->msg);
	switch (pd_event->msg) {
	case TCP_DPM_EVT_GOTOMIN:
		ret =  pd_handle_tcp_event_gotomin(pd_port);
		break;

	case TCP_DPM_EVT_REQUEST:
	case TCP_DPM_EVT_REQUEST_EX:
	case TCP_DPM_EVT_REQUEST_AGAIN:
		ret =  pd_handle_tcp_event_request(pd_port);
		break;

#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE
	case TCP_DPM_EVT_GET_SOURCE_CAP_EXT:
		ret = pd_handle_tcp_event_get_source_cap_ext(pd_port);
		break;
#endif

#ifdef CONFIG_USB_PD_REV30_STATUS_REMOTE
	case TCP_DPM_EVT_GET_STATUS:
		ret = pd_make_tcp_event_transit_ready(pd_port,
			PE_SNK_GET_SOURCE_STATUS, PE_SRC_GET_SINK_STATUS);
		break;
#endif

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	case TCP_DPM_EVT_GET_PPS_STATUS:
		ret = pd_handle_tcp_event_get_pps_status(pd_port);
		break;
#endif

#ifdef CONFIG_USB_PD_REV30_ALERT_LOCAL
	case TCP_DPM_EVT_ALERT:
		ret = pd_handle_tcp_event_alert(pd_port, pd_event);
		break;
#endif

	default:
		break;
	}
#endif /* CONFIG_USB_PD_REV30 */

	return ret;
}

bool hisi_pd_process_event_tcp(pd_port_t *pd_port, pd_event_t *pd_event)
{
	int ret;

	ret = pd_handle_tcp_dpm_event(pd_port, pd_event);
	pd_notify_tcp_event_1st_result(pd_port, ret);

	return ret == TCP_DPM_RET_SENT;
}

