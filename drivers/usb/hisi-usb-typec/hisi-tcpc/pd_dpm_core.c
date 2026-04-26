/*
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * PD Device Policy Manager Core Driver
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

#include <linux/delay.h>
#include "include/tcpci.h"
#include "include/pd_tcpm.h"
#include "include/pd_policy_engine.h"
#include "include/pd_dpm_core.h"
#include "include/pd_dpm_pdo_select.h"
#include <pd_dpm_prv.h>
#include <securec.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "[dpm_core]"
#endif
#define PD_DPM_HW_VID                 0x12d1
#define VID_MASK                      0xffff

static const svdm_svid_ops_t svdm_svid_ops[] = {
#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
	{
		.name = "DisplayPort",
		.svid = USB_SID_DISPLAYPORT,

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
		.dfp_inform_id = hisi_dp_dfp_u_notify_discover_id,
		.dfp_inform_svids = hisi_dp_dfp_u_notify_discover_svid,
		.dfp_inform_modes = hisi_dp_dfp_u_notify_discover_modes,
		.dfp_inform_enter_mode = hisi_dp_dfp_u_notify_enter_mode,
		.dfp_inform_exit_mode = hisi_dp_dfp_u_notify_exit_mode,
		.dfp_inform_attention = hisi_dp_dfp_u_notify_attention,
#endif
		.ufp_request_enter_mode = hisi_dp_ufp_u_request_enter_mode,
		.ufp_request_exit_mode = hisi_dp_ufp_u_request_exit_mode,

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
		.notify_pe_startup = hisi_dp_dfp_u_notify_pe_startup,
		.notify_pe_ready = hisi_dp_dfp_u_notify_pe_ready,
#endif
		.reset_state = hisi_dp_reset_state,
	},
#endif
};

void hisi_dpm_check_supported_modes(void)
{
	unsigned int i;

	D("+\n");
	for (i = 0; i < ARRAY_SIZE(svdm_svid_ops); i++)
		I("SVDM supported mode [%u]: name = %s, svid = 0x%x\n", i,
				svdm_svid_ops[i].name, svdm_svid_ops[i].svid);
	D("-\n");
}

static void pd_dpm_update_pdos_flags(pd_port_t *pd_port, uint32_t pdo)
{
	pd_port->dpm_flags &= ~DPM_FLAGS_RESET_PARTNER_MASK;

	/* Only update PDO flags if pdo's type is fixed */
	if ((pdo & PDO_TYPE_MASK) != PDO_TYPE_FIXED)
		return;

	if (pdo & PDO_FIXED_DUAL_ROLE)
		pd_port->dpm_flags |= DPM_FLAGS_PARTNER_DR_POWER;

	if (pdo & PDO_FIXED_DATA_SWAP)
		pd_port->dpm_flags |= DPM_FLAGS_PARTNER_DR_DATA;

	if (pdo & PDO_FIXED_EXTERNAL) /* Unconstrained Power */
		pd_port->dpm_flags |= DPM_FLAGS_PARTNER_EXTPOWER;

	if (pdo & PDO_FIXED_COMM_CAP)
		pd_port->dpm_flags |= DPM_FLAGS_PARTNER_USB_COMM;
}

int hisi_pd_dpm_enable_vconn(pd_port_t *pd_port, bool en)
{
	return hisi_pd_set_vconn(pd_port, en);
}

int hisi_pd_dpm_send_sink_caps(pd_port_t *pd_port)
{
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	if (pd_port->pd_revision[0] < PD_REV30)
		snk_cap->nr = pd_port->local_snk_cap_nr_pd20;
	else
		snk_cap->nr = pd_port->local_snk_cap_nr_pd30;
#endif
	return hisi_pd_send_data_msg(pd_port, TCPC_TX_SOP, PD_DATA_SINK_CAP,
			snk_cap->nr, snk_cap->pdos);
}

int hisi_pd_dpm_send_source_caps(pd_port_t *pd_port)
{
	uint8_t i;
	uint32_t cable_curr = 3000; /* max cable current */
	pd_port_power_caps *src_cap0 = &pd_port->local_src_cap_default;
	pd_port_power_caps *src_cap1 = &pd_port->local_src_cap;
	int ret;

	D("+\n");
	if (pd_port->power_cable_present) {
		cable_curr = hisi_pd_extract_cable_curr(
				pd_port->cable_vdos[VDO_INDEX_CABLE]);
		DPM_DBG("cable_limit: %umA\n", cable_curr);
	}

	src_cap1->nr = src_cap0->nr;
	for (i = 0; i < src_cap0->nr; i++)
		src_cap1->pdos[i] = hisi_pd_reset_pdo_power(
				src_cap0->pdos[i], cable_curr);

	ret = hisi_pd_send_data_msg(pd_port, TCPC_TX_SOP, PD_DATA_SOURCE_CAP,
			src_cap1->nr, src_cap1->pdos);
	D("-\n");
	return ret;
}

enum {
	GOOD_PW_NONE = 0,  /* both no GP */
	GOOD_PW_PARTNER,   /* partner has GP */
	GOOD_PW_LOCAL,     /* local has GP */
	GOOD_PW_BOTH,      /* both have GPs */
};

static int dpm_check_good_power(pd_port_t *pd_port)
{
	bool local_ex = false;
	bool partner_ex = false;

	D("+-\n");
	local_ex = (pd_port->dpm_caps & DPM_CAP_LOCAL_EXT_POWER) != 0;
	partner_ex = (pd_port->dpm_flags & DPM_FLAGS_PARTNER_EXTPOWER) != 0;

	if (local_ex != partner_ex) {
		if (partner_ex)
			return GOOD_PW_PARTNER;
		return GOOD_PW_LOCAL;
	}

	if (local_ex)
		return GOOD_PW_BOTH;

	return GOOD_PW_NONE;
}

static bool dpm_response_request(pd_port_t *pd_port, bool accept)
{
	D("\n");
	if (accept)
		return pd_put_dpm_ack_event(pd_port);
	return pd_put_dpm_nak_event(pd_port, PD_DPM_NAK_REJECT);
}

/* ---- SNK ---- */
#ifdef CONFIG_USB_PD_REV30
static bool dpm_build_request_info_apdo(
		pd_port_t *pd_port, struct dpm_rdo_info_t *req_info,
		pd_port_power_caps *src_cap, uint8_t charging_policy)
{
	uint32_t snk_pdo = PDO_FIXED(
			(unsigned int)pd_port->request_v_apdo, (unsigned int)pd_port->request_i_apdo,
			DPM_PDO_TYPE_FIXED);

	D("snk_pdo: 0x%x\n", snk_pdo);
	return dpm_find_match_req_info(req_info,
			snk_pdo, src_cap->nr, src_cap->pdos,
			-1, charging_policy);
}
#endif

static bool dpm_build_request_info(pd_port_t *pd_port,
		struct dpm_rdo_info_t *req_info)
{
	int i;
	int max_power = 0;
	int max_uw = -1;
	bool find_cap = false;
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;
	uint8_t charging_policy = pd_port->dpm_charging_policy;
	errno_t errno;

	errno = memset_s(req_info, sizeof(*req_info), 0, sizeof(struct dpm_rdo_info_t));
	if (errno != EOK)
		E("memset_s failed\n");

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	if (charging_policy == DPM_CHARGING_POLICY_PPS) {
		return dpm_build_request_info_apdo(
				pd_port, req_info, src_cap, charging_policy);
	}
#endif

	for (i = 0; i < snk_cap->nr; i++) {
		find_cap = dpm_find_match_req_info(req_info,
				snk_cap->pdos[i], src_cap->nr, src_cap->pdos,
				max_uw, charging_policy);
		if (find_cap) {
			if (req_info->type == DPM_PDO_TYPE_BAT)
				max_uw = req_info->oper_uw;
			else
				max_uw = req_info->vmax * req_info->oper_ma;

			if (max_uw > max_power)
				max_power = max_uw;

			pd_port->local_selected_cap = i + 1;
		}
	}

	hisi_usb_typec_max_power(max_power);

	return max_uw > 0;
}

static bool dpm_build_default_request_info(pd_port_t *pd_port,
		struct dpm_rdo_info_t *req_info)
{
	struct dpm_pdo_info_t sink, source;
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;

	D("\n");
	pd_port->local_selected_cap = 1;

	hisi_dpm_extract_pdo_info(snk_cap->pdos[0], &sink);
	hisi_dpm_extract_pdo_info(src_cap->pdos[0], &source);

	req_info->pos = 1;
	req_info->type = source.type;
	req_info->mismatch = true;
	req_info->vmax = 5000; /* 5000: max voltage */
	req_info->vmin = 5000; /* 5000: min voltage */

	if (req_info->type == DPM_PDO_TYPE_BAT) {
		req_info->max_uw = sink.uw;
		req_info->oper_uw = source.uw;
	} else {
		req_info->max_ma = sink.ma;
		req_info->oper_ma = source.ma;
	}

	return true;
}

static void dpm_update_request(pd_port_t *pd_port,
		struct dpm_rdo_info_t *req_info)
{
	uint32_t mw_op, mw_max;
	uint32_t flags = 0;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_GIVE_BACK)
		flags |= RDO_GIVE_BACK;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_NO_SUSPEND)
		flags |= RDO_NO_SUSPEND;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_USB_COMM)
		flags |= RDO_COMM_CAP;

	if (req_info->mismatch)
		flags |= RDO_CAP_MISMATCH;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	pd_port->request_apdo = false;
#endif
	pd_port->request_v_new = req_info->vmax;

	if (req_info->type == DPM_PDO_TYPE_BAT) {
		mw_op = req_info->oper_uw / 1000; /* 1000: oper uw */
		mw_max = req_info->max_uw / 1000; /* 1000 max uw */

		pd_port->request_i_op =
				req_info->oper_uw / (uint32_t)req_info->vmin;
		pd_port->request_i_max =
				req_info->max_uw / (uint32_t)req_info->vmin;

		if (req_info->mismatch)
			pd_port->request_i_new = pd_port->request_i_op;
		else
			pd_port->request_i_new = pd_port->request_i_max;

		pd_port->last_rdo =
				RDO_BATT(req_info->pos, mw_op, mw_max, flags);
	} else {
		pd_port->request_i_op = req_info->oper_ma;
		pd_port->request_i_max = req_info->max_ma;

		if (req_info->mismatch)
			pd_port->request_i_new = pd_port->request_i_op;
		else
			pd_port->request_i_new = pd_port->request_i_max;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
		if (req_info->type == DPM_PDO_TYPE_APDO) {
			pd_port->request_apdo = true;
			pd_port->last_rdo = RDO_APDO(
				req_info->pos, (unsigned int)req_info->vmin,
				req_info->oper_ma, flags);
			return;
		}
#endif

		pd_port->last_rdo = RDO_FIXED(req_info->pos,
			req_info->oper_ma, req_info->max_ma, flags);
	}
}

bool hisi_pd_dpm_send_request(pd_port_t *pd_port, int mv, int ma)
{
	bool find_cap = false;
	struct dpm_rdo_info_t req_info;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;
	uint32_t snk_pdo = PDO_FIXED((unsigned int)mv, (unsigned int)ma, 0);
	bool ret = false;
	errno_t errno;

	D("\n");
	errno = memset_s(&req_info, sizeof(struct dpm_rdo_info_t),
			0, sizeof(struct dpm_rdo_info_t));
	if (errno != EOK)
		E("memset_s failed\n");

	find_cap = dpm_find_match_req_info(&req_info, snk_pdo, src_cap->nr,
			src_cap->pdos, -1, pd_port->dpm_charging_policy);
	if (!find_cap) {
		DPM_DBG("Can't find match_cap\n");
		return false;
	}

	dpm_update_request(pd_port, &req_info);
	ret = pd_put_dpm_pd_request_event(pd_port,
			PD_DPM_PD_REQUEST_PW_REQUEST);

	return ret;
}

bool pd_dpm_update_tcp_request(pd_port_t *pd_port,
		struct tcp_dpm_pd_request *pd_req)
{
	bool find_cap = false;
	struct dpm_rdo_info_t req_info;
	uint8_t charging_policy = pd_port->dpm_charging_policy;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;
	uint32_t snk_pdo = PDO_FIXED((unsigned int)pd_req->mv, (unsigned int)pd_req->ma, 0);

	if (memset_s(&req_info, sizeof(struct dpm_rdo_info_t),
			0, sizeof(struct dpm_rdo_info_t)))
		E("clr req info\n");

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	if (charging_policy == DPM_CHARGING_POLICY_PPS) {
		req_info.pos = pd_port->request_apdo_pos;
		pd_port->request_v_apdo = pd_req->mv;
		pd_port->request_i_apdo = pd_req->ma;
	}
#endif

	find_cap = dpm_find_match_req_info(&req_info,
			snk_pdo, src_cap->nr, src_cap->pdos,
			-1, charging_policy);
	if (!find_cap) {
		I("Can't find match_cap\r\n");
		return false;
	}

	dpm_update_request(pd_port, &req_info);

	return true;
}

bool pd_dpm_update_tcp_request_ex(pd_port_t *pd_port,
	struct tcp_dpm_pd_request_ex *pd_req)
{
	struct dpm_pdo_info_t source;
	struct dpm_rdo_info_t req_info;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;

	D("\n");
	if (pd_req->pos > src_cap->nr)
		return false;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	if (pd_port->dpm_charging_policy == DPM_CHARGING_POLICY_PPS) {
		D("Reject charging_policy pps\n");
		return false;
	}
#endif

	hisi_dpm_extract_pdo_info(src_cap->pdos[pd_req->pos - 1], &source);

	req_info.pos = pd_req->pos;
	req_info.type = source.type;
	req_info.mismatch = false;
	req_info.vmax = source.vmax;
	req_info.vmin = source.vmin;

	if (req_info.type == DPM_PDO_TYPE_BAT) {
		req_info.max_uw = pd_req->max_uw;
		req_info.oper_uw = pd_req->oper_uw;
		if (pd_req->oper_uw < pd_req->max_uw)
			req_info.mismatch = true;
	} else {
		req_info.max_ma = pd_req->max_ma;
		req_info.oper_ma = pd_req->oper_ma;
		if (pd_req->oper_ma < pd_req->max_ma)
			req_info.mismatch = true;
	}

	dpm_update_request(pd_port, &req_info);

	return true;
}

bool pd_dpm_update_tcp_request_again(pd_port_t *pd_port)
{
	bool find_cap = false;
	int sink_nr, source_nr;
	struct dpm_rdo_info_t req_info;
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;

	sink_nr = snk_cap->nr;
	source_nr = src_cap->nr;

	D("\n");
	if ((source_nr <= 0) || (sink_nr <= 0)) {
		D("SrcNR or SnkNR <= 0\n");
		return false;
	}

	find_cap = dpm_build_request_info(pd_port, &req_info);
	/* Can't find , choose default */
	if (!find_cap)
		dpm_build_default_request_info(pd_port, &req_info);

	dpm_update_request(pd_port, &req_info);
	return false;
}
void hisi_pd_dpm_snk_evaluate_caps(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool find_cap = false;
	int sink_nr, source_nr;
	struct dpm_rdo_info_t req_info;
	pd_msg_t *pd_msg = pd_event->pd_msg;
	/* assigned in pd_parse_pdata */
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;
	errno_t errno;

	D("\n");
	if (!pd_msg) {
		pr_err("%s: the pd_msg is NULL\n", __func__);
		return;
	}

	sink_nr = snk_cap->nr;
	source_nr = PD_HEADER_CNT(pd_msg->msg_hdr);
	if ((source_nr <= 0) || (sink_nr <= 0)) {
		DPM_DBG("SrcNR or SnkNR = 0\n");
		return;
	}

	src_cap->nr = source_nr;
	errno = memcpy_s(src_cap->pdos,
			sizeof(uint32_t) * PD_PORT_PDOS_MAX_SIZE,
			pd_msg->payload,
			sizeof(uint32_t) * source_nr);
	if (errno != EOK)
		E("memcpy_s failed\n");

	pd_dpm_update_pdos_flags(pd_port, src_cap->pdos[0]);

	find_cap = dpm_build_request_info(pd_port, &req_info);
	/* If we can't find any cap to use, choose default setting */
	if (!find_cap) {
		DPM_DBG("Can't find any SrcCap\n");
		dpm_build_default_request_info(pd_port, &req_info);
	}

	dpm_update_request(pd_port, &req_info);

	pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_SOURCE_CAP;
	if (!(pd_port->dpm_flags & DPM_FLAGS_PARTNER_DR_POWER))
		pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_SINK_CAP;

	if (req_info.pos > 0)
		pd_put_dpm_notify_event(pd_port, req_info.pos);
	return;
}

void hisi_pd_dpm_snk_standby_power(pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_USB_PD_SNK_STANDBY_POWER
	/*
	 * pSnkStdby : Maximum power consumption while in Sink Standby. (2.5W)
	 * I1 = (pSnkStdby/VBUS)
	 * I2 = (pSnkStdby/VBUS) + cSnkBulkPd(DVBUS/Dt)
	 * STANDBY_UP = I1 < I2, STANDBY_DOWN = I1 > I2
	 */

	uint8_t type;
	int ma = -1;
	int standby_curr = 2500000 / pd_port->request_v; /* 2500000: standby current */

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	if (pd_port->request_apdo)
		return;
#endif

	if (pd_port->request_v_new > pd_port->request_v) {
		/*
		 * Case2 Increasing the Voltage
		 * Case3 Increasing the Voltage and Current
		 * Case4 Increasing the Voltage and Decreasing the Curren
		 */
		ma = standby_curr;
		type = TCP_VBUS_CTRL_STANDBY_UP;
	} else if (pd_port->request_v_new < pd_port->request_v) {
		/*
		 * Case5 Decreasing the Voltage and Increasing the Current
		 * Case7 Decreasing the Voltage
		 * Case8 Decreasing the Voltage and the Current
		 */
		ma = standby_curr;
		type = TCP_VBUS_CTRL_STANDBY_DOWN;
	} else if (pd_port->request_i_new < pd_port->request_i) {
		/* Case6 Decreasing the Current, t1 i = new */
		ma = pd_port->request_i_new;
		type = TCP_VBUS_CTRL_PD_STANDBY;
	}

	if (ma >= 0)
		tcpci_sink_vbus(pd_port->tcpc_dev,
				type, pd_port->request_v, ma);
#endif
}

void hisi_pd_dpm_snk_transition_power(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("\n");

	tcpci_sink_vbus(pd_port->tcpc_dev, TCP_VBUS_CTRL_REQUEST,
			pd_port->request_v_new, pd_port->request_i_new);

	pd_port->request_v = pd_port->request_v_new;
	pd_port->request_i = pd_port->request_i_new;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	if (pd_port->request_apdo != pd_port->request_apdo_new)
		pd_port->request_apdo = pd_port->request_apdo_new;
#endif
}

void hisi_pd_dpm_snk_hard_reset(pd_port_t *pd_port, pd_event_t *pd_event)
{
	/*
	 * tSnkHardResetPrepare :
	 *     Time allotted for the Sink power electronics
	 *     to prepare for a Hard Reset
	 */
	int mv = 0;
	int ma = 0;

	D("\n");
#ifdef CONFIG_USB_PD_SNK_HRESET_KEEP_DRAW
	if (!pd_port->pd_prev_connected) {
		ma = -1;
		mv = TCPC_VBUS_SINK_5V;
	}
#endif

	tcpci_sink_vbus(pd_port->tcpc_dev, TCP_VBUS_CTRL_HRESET, mv, ma);
	pd_put_pe_event(pd_port, PD_PE_POWER_ROLE_AT_DEFAULT);
}

/* ---- SRC ---- */
void hisi_pd_dpm_src_evaluate_request(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint8_t rdo_pos;
	uint32_t rdo, pdo;
	uint32_t op_curr = 0;
	uint32_t max_curr = 0;
	uint32_t source_vmin = 0;
	uint32_t source_vmax, source_i;
	bool accept_request = true;
	pd_msg_t *pd_msg = pd_event->pd_msg;
	pd_port_power_caps *src_cap = &pd_port->local_src_cap;

	D("\n");
	if (!pd_msg) {
		pr_err("%s: the pd_msg is NULL\n", __func__);
		return;
	}

	rdo = pd_msg->payload[0];
	rdo_pos = RDO_POS(rdo);
	pd_port->dpm_flags &= (~DPM_FLAGS_PARTNER_MISMATCH);
	if ((rdo_pos > 0) && (rdo_pos <= src_cap->nr)) {
		pdo = src_cap->pdos[rdo_pos - 1];

		hisi_pd_extract_rdo_power(rdo, pdo, &op_curr, &max_curr);
		hisi_pd_extract_pdo_power(pdo,
				&source_vmin, &source_vmax, &source_i);

		if (source_i < op_curr) {
			DPM_DBG("src_i [%u] < op_i [%u]\n",
					source_i, op_curr);
			accept_request = false;
		}

		if (rdo & RDO_CAP_MISMATCH) {
			DPM_DBG("CAP_MISMATCH\n");
			pd_port->dpm_flags |= DPM_FLAGS_PARTNER_MISMATCH;
		} else if (source_i < max_curr) {
			DPM_DBG("src_i [%u] < max_i [%u]\n",
					source_i, max_curr);
			accept_request = false;
		}
	} else {
		accept_request = false;
		DPM_DBG("RequestPos Wrong %u\n", rdo_pos);
	}

	if (accept_request) {
		pd_port->local_selected_cap = rdo_pos;
		pd_port->request_i_op = op_curr;
		pd_port->request_i_max = max_curr;

		if (rdo & RDO_CAP_MISMATCH)
			pd_port->request_i_new = op_curr;
		else
			pd_port->request_i_new = max_curr;

		pd_port->request_v_new = source_vmin;
		pd_put_dpm_notify_event(pd_port, rdo_pos);
	} else {
		pd_port->local_selected_cap = 0;
		pd_put_dpm_nak_event(pd_port, PD_DPM_NAK_REJECT);
	}
}

void hisi_pd_dpm_src_transition_power(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("\n");
	hisi_pd_enable_vbus_stable_detection(pd_port);

	tcpci_source_vbus(pd_port->tcpc_dev, TCP_VBUS_CTRL_REQUEST,
			pd_port->request_v_new, pd_port->request_i_new);

	if (pd_port->request_v == pd_port->request_v_new)
		hisi_pd_put_vbus_stable_event(pd_port->tcpc_dev);
#if CONFIG_USB_PD_VBUS_STABLE_TOUT
	else
		pd_enable_timer(pd_port, PD_TIMER_VBUS_STABLE);
#endif

	pd_port->request_v = pd_port->request_v_new;
	pd_port->request_i = pd_port->request_i_new;
}

void hisi_pd_dpm_src_inform_cable_vdo(pd_port_t *pd_port, pd_event_t *pd_event)
{
	const int size = sizeof(uint32_t) * VDO_MAX_SIZE;
	errno_t errno;

	D("\n");
	if (pd_event->pd_msg) {
		errno = memcpy_s(pd_port->cable_vdos, size,
				pd_event->pd_msg->payload, size);
		if (errno != EOK)
			E("memcpy_s failed\n");
	}

	pd_put_dpm_ack_event(pd_port);
}

void hisi_pd_dpm_src_hard_reset(pd_port_t *pd_port)
{
	D("\n");
	tcpci_source_vbus(pd_port->tcpc_dev,
			TCP_VBUS_CTRL_HRESET, TCPC_VBUS_SOURCE_0V, 0);
	hisi_pd_enable_vbus_safe0v_detection(pd_port);
}

/* ---- UFP : update_svid_data ---- */
static bool dpm_ufp_update_svid_data_enter_mode(pd_port_t *pd_port,
		uint16_t svid, uint8_t ops)
{
	svdm_svid_data_t *svid_data = NULL;

	D("EnterMode (svid0x%04x, ops:%u)\n", svid, ops);
	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	if (!svid_data)
		return false;

	/* Only accept 1 mode active at the same time */
	if (svid_data->active_mode)
		return false;

	if ((ops == 0) || (ops > svid_data->local_mode.mode_cnt))
		return false;

	svid_data->active_mode = ops;
	pd_port->modal_operation = true;

	svdm_ufp_request_enter_mode(pd_port, svid, ops);

	tcpci_enter_mode(pd_port->tcpc_dev, svid, ops,
			svid_data->local_mode.mode_vdo[ops]);

	return true;
}

static bool dpm_ufp_update_svid_data_exit_mode(pd_port_t *pd_port,
		uint16_t svid, uint8_t ops)
{
	uint8_t i;
	bool modal_operation = false;
	svdm_svid_data_t *svid_data = NULL;

	D("ExitMode (svid0x%04x, mode:%u)\n", svid, ops);
	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	if (!svid_data)
		return false;

	if (svid_data->active_mode == 0)
		return false;

	if ((ops == 0) || (ops == svid_data->active_mode)) {
		svid_data->active_mode = 0;

		modal_operation = false;
		for (i = 0; i < pd_port->svid_data_cnt; i++) {
			svid_data = &pd_port->svid_data[i];
			if (svid_data->active_mode) {
				modal_operation = true;
				break;
			}
		}

		pd_port->modal_operation = modal_operation;

		svdm_ufp_request_exit_mode(pd_port, svid, ops);
		tcpci_exit_mode(pd_port->tcpc_dev, svid);
		return true;
	}

	return false;
}

/* ---- UFP : Evaluate VDM Request ---- */

static inline bool pd_dpm_ufp_reply_request(pd_port_t *pd_port,
		pd_event_t *pd_event, bool ack)
{
	D("%s\n", ack ? "ACK" : "NAK");
	return vdm_put_dpm_event(pd_port,
			ack ? PD_DPM_ACK : PD_DPM_NAK, pd_event->pd_msg);
}

static uint32_t dpm_vdm_get_svid(pd_event_t *pd_event)
{
	pd_msg_t *pd_msg = pd_event->pd_msg;

	D("\n");
	if (!pd_msg) {
		E("%s: the pd_msg is NULL\n", __func__);
		return 0;
	}
	return PD_VDO_VID(pd_msg->payload[0]);
}

void hisi_pd_dpm_ufp_request_id_info(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_dpm_ufp_reply_request(pd_port, pd_event,
			dpm_vdm_get_svid(pd_event) == USB_SID_PD);
}

void hisi_pd_dpm_ufp_request_svid_info(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ack = false;

	if (pd_is_support_modal_operation(pd_port))
		ack = (dpm_vdm_get_svid(pd_event) == USB_SID_PD);

	pd_dpm_ufp_reply_request(pd_port, pd_event, ack);
}

void hisi_pd_dpm_ufp_request_mode_info(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint16_t svid;
	bool ack = false;

	svid = dpm_vdm_get_svid(pd_event);
	ack = dpm_get_svdm_svid_data(pd_port, svid) != NULL;
	pd_dpm_ufp_reply_request(pd_port, pd_event, ack);
}

void hisi_pd_dpm_ufp_request_enter_mode(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	bool ack = false;
	uint16_t svid = 0;
	uint8_t ops = 0;

	if (!pd_event->pd_msg)
		E("%s: the pd_msg is NULL\n", __func__);

	dpm_vdm_get_svid_ops(pd_event, &svid, &ops);
	ack = dpm_ufp_update_svid_data_enter_mode(pd_port, svid, ops);

	pd_dpm_ufp_reply_request(pd_port, pd_event, ack);
}

void hisi_pd_dpm_ufp_request_exit_mode(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ack = false;
	uint16_t svid = 0;
	uint8_t ops = 0;

	D("\n");
	dpm_vdm_get_svid_ops(pd_event, &svid, &ops);
	ack = dpm_ufp_update_svid_data_exit_mode(pd_port, svid, ops);
	pd_dpm_ufp_reply_request(pd_port, pd_event, ack);
}

/* ---- UFP : Response VDM Request ---- */

int hisi_pd_dpm_ufp_response_id(pd_port_t *pd_port, pd_event_t *pd_event)
{
	return hisi_pd_reply_svdm_request(pd_port, pd_event, CMDT_RSP_ACK,
			pd_port->id_vdo_nr, pd_port->id_vdos);
}

int hisi_pd_dpm_ufp_response_svids(pd_port_t *pd_port, pd_event_t *pd_event)
{
	svdm_svid_data_t *svid_data = NULL;
	uint16_t svid_list[2];
	uint32_t svids[VDO_MAX_DATA_SIZE];
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t cnt = pd_port->svid_data_cnt;
	int ret;

	D("\n");
	if (pd_port->svid_data_cnt >= VDO_MAX_SVID_SIZE)
		E("the %u is over vdo max svid size\n", pd_port->svid_data_cnt);

	if (unlikely(cnt >= VDO_MAX_SVID_SIZE))
		cnt = VDO_MAX_SVID_SIZE;

	while (i < cnt) {
		svid_data = &pd_port->svid_data[i++];
		svid_list[0] = svid_data->svid;

		if (i < cnt) {
			svid_data = &pd_port->svid_data[i++];
			svid_list[1] = svid_data->svid;
		} else {
			svid_list[1] = 0;
		}
		svids[j++] = VDO_SVID(svid_list[0], svid_list[1]);
	}

	if ((cnt % 2) == 0) /* 2: svid_data_cnt */
		svids[j++] = VDO_SVID(0, 0);

	ret = hisi_pd_reply_svdm_request(pd_port, pd_event,
			CMDT_RSP_ACK, j, svids);

	return ret;
}

int hisi_pd_dpm_ufp_response_modes(pd_port_t *pd_port, pd_event_t *pd_event)
{
	svdm_svid_data_t *svid_data = NULL;
	uint16_t svid = dpm_vdm_get_svid(pd_event);

	D("\n");
	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	if (svid_data) {
		return hisi_pd_reply_svdm_request(pd_port, pd_event,
				CMDT_RSP_ACK,
				svid_data->local_mode.mode_cnt,
				svid_data->local_mode.mode_vdo);
	} else {
		PE_DBG("ERROR-4965\n");
		return hisi_pd_reply_svdm_request_simply(pd_port, pd_event,
				CMDT_RSP_NAK);
	}
}

/* ---- DFP : update_svid_data ---- */

static void dpm_dfp_update_svid_data_exist(pd_port_t *pd_port,
		uint16_t svid)
{
	uint8_t k;
	svdm_svid_data_t *svid_data = NULL;

	D("+\n");

#ifdef CONFIG_USB_PD_KEEP_SVIDS
	svdm_svid_list_t *list = &pd_port->remote_svid_list;

	if (list->cnt < VDO_MAX_SVID_SIZE)
		list->svids[list->cnt++] = svid;
	else
		DPM_DBG("ERR:SVIDCNT\n");
#endif
	/* inform svid to other subsystems */
	hisi_usb_pd_ufp_update_dock_svid(svid);

	for (k = 0; k < pd_port->svid_data_cnt; k++) {
		svid_data = &pd_port->svid_data[k];

		if (svid_data->svid == svid)
			svid_data->exist = 1;
	}
	D("-\n");
}

static void dpm_dfp_update_svid_data_modes(pd_port_t *pd_port,
		uint16_t svid, uint32_t *mode_list, uint8_t count)
{
	uint8_t i;
	svdm_svid_data_t *svid_data = NULL;
	errno_t errno;

	D("InformMode (0x%04x:%u):\n", svid, count);
	for (i = 0; i < count; i++)
		DPM_DBG("Mode[%u]: 0x%08x\n", i, mode_list[i]);

	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	if (!svid_data)
		return;

	svid_data->remote_mode.mode_cnt = count;

	if (count != 0) {
		errno = memcpy_s(svid_data->remote_mode.mode_vdo,
				sizeof(uint32_t) * VDO_MAX_DATA_SIZE,
				mode_list,
				sizeof(uint32_t) * count);
		if (errno != EOK)
			E("memcpy_s failed\n");
	}
}

static void dpm_dfp_update_svid_enter_mode(pd_port_t *pd_port,
		uint16_t svid, uint8_t ops)
{
	svdm_svid_data_t *svid_data = NULL;

	D("EnterMode (svid0x%04x, mode:%u)\n", svid, ops);

	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	if (!svid_data)
		return;

	svid_data->active_mode = ops;
	pd_port->modal_operation = true;

	tcpci_enter_mode(pd_port->tcpc_dev, svid_data->svid, ops,
			svid_data->remote_mode.mode_vdo[ops]);
}

static void dpm_dfp_update_svid_data_exit_mode(pd_port_t *pd_port,
		uint16_t svid, uint8_t ops)
{
	uint8_t i;
	bool modal_operation = false;
	svdm_svid_data_t *svid_data = NULL;

	D("ExitMode (svid0x%04x, mode:%u)\n", svid, ops);

	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	if (!svid_data)
		return;

	if ((ops == 0) || (ops == svid_data->active_mode)) {
		svid_data->active_mode = 0;

		modal_operation = false;
		for (i = 0; i < pd_port->svid_data_cnt; i++) {
			svid_data = &pd_port->svid_data[i];

			if (svid_data->active_mode) {
				modal_operation = true;
				break;
			}
		}

		pd_port->modal_operation = modal_operation;
		tcpci_exit_mode(pd_port->tcpc_dev, svid);
	}
}

/* ---- DFP : Inform VDM Result ---- */

static inline void vdm_put_dpm_notified_event(pd_port_t *pd_port)
{
	pd_port->dpm_ack_immediately = true;
}

void hisi_pd_dpm_dfp_inform_id(pd_port_t *pd_port,
		pd_event_t *pd_event, bool ack)
{
#if DPM_DBG_ENABLE
	pd_msg_t *pd_msg = pd_event->pd_msg;
#endif
	D("\n");
	if (ack) {
		DPM_DBG("InformID, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
				pd_msg->payload[0], pd_msg->payload[1],
				pd_msg->payload[2], pd_msg->payload[3]); /* 2 and 3: paylod */

		hisi_usb_typec_dfp_inform_id(pd_msg->payload,
				PD_MSG_PAYLOAD_SIZE);
	}

	if (!pd_port->vdm_discard_retry_flag) {
		if (!ack)
			pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_UFP_SVID;
		pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_UFP_ID;
		svdm_dfp_inform_id(pd_port, pd_event, ack);
	}
	vdm_put_dpm_notified_event(pd_port);
}

static int dpm_dfp_consume_svids(pd_port_t *pd_port,
		uint32_t *svid_list, uint8_t count)
{
	bool discover_again = true;
	uint8_t i, j;
	uint16_t svid[2];

	D("InformSVID %u:\n", count);
	if (count < 6) /* 6: discover count */
		discover_again = false;

	for (i = 0; i < count; i++) {
		svid[0] = PD_VDO_SVID_SVID0(svid_list[i]);
		svid[1] = PD_VDO_SVID_SVID1(svid_list[i]);

		DPM_DBG("svid[%u]: 0x%04x 0x%04x\n", i, svid[0], svid[1]);

		for (j = 0; j < 2; j++) { /* 2: count */
			if (svid[j] == 0) {
				discover_again = false;
				break;
			}

			dpm_dfp_update_svid_data_exist(pd_port, svid[j]);
		}
	}

	if (discover_again) {
		DPM_DBG("DiscoverSVID Again\n");
		vdm_put_dpm_vdm_request_event(pd_port,
				PD_DPM_VDM_REQUEST_DISCOVER_SVIDS);
		return 1;
	}

	return 0;
}

void hisi_pd_dpm_dfp_inform_svids(pd_port_t *pd_port,
		pd_event_t *pd_event, bool ack)
{
	uint8_t count;
	uint32_t *svid_list = NULL;
	pd_msg_t *pd_msg = pd_event->pd_msg;

	D("\n");
	if (ack) {
		svid_list = &pd_msg->payload[1];
		count = (PD_HEADER_CNT(pd_msg->msg_hdr) - 1);
		if (dpm_dfp_consume_svids(pd_port, svid_list, count))
			return;
	}

	if (!pd_port->vdm_discard_retry_flag) {
		pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_UFP_SVID;
		svdm_dfp_inform_svids(pd_port, ack);
	}
	vdm_put_dpm_notified_event(pd_port);
	return;
}

void hisi_pd_dpm_dfp_inform_modes(pd_port_t *pd_port,
		pd_event_t *pd_event, bool ack)
{
	uint8_t count;
	uint16_t svid;
	uint16_t expected_svid = pd_port->mode_svid;
	pd_msg_t *pd_msg = pd_event->pd_msg;

	D("\n");
	if (ack) {
		count = (PD_HEADER_CNT(pd_msg->msg_hdr));
		svid = PD_VDO_VID(pd_msg->payload[VDO_INDEX_HDR]);
		if (svid != expected_svid) {
			ack = false;
			DPM_DBG("Not expected SVID (0x%04x, 0x%04x)\n",
					svid, expected_svid);
		} else {
			dpm_dfp_update_svid_data_modes(pd_port, svid,
					&pd_msg->payload[1], count - 1);
		}
	}

	svdm_dfp_inform_modes(pd_port, expected_svid, ack);
	vdm_put_dpm_notified_event(pd_port);
	return;
}

void hisi_pd_dpm_dfp_inform_enter_mode(pd_port_t *pd_port,
		pd_event_t *pd_event, bool ack)
{
	uint16_t svid = 0;
	uint16_t expected_svid = pd_port->mode_svid;
	uint8_t ops = 0;

	D("\n");
	if (ack) {
		dpm_vdm_get_svid_ops(pd_event, &svid, &ops);

		if (svid != expected_svid) {
			ack = false;
			DPM_DBG("Not expected SVID (0x%04x, 0x%04x)\n",
					svid, expected_svid);
		} else {
			dpm_dfp_update_svid_enter_mode(pd_port, svid, ops);
		}
	}

	svdm_dfp_inform_enter_mode(pd_port, expected_svid, ops, ack);
	vdm_put_dpm_notified_event(pd_port);
}

void hisi_pd_dpm_dfp_inform_exit_mode(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint16_t svid = 0;
	uint16_t expected_svid = pd_port->mode_svid;
	uint8_t ops = 0;

	D("\n");
	if (pd_event->event_type != PD_EVT_TIMER_MSG) {
		dpm_vdm_get_svid_ops(pd_event, &svid, &ops);
	} else {
		svid = pd_port->mode_svid;
		ops = pd_port->mode_obj_pos;
	}

	dpm_dfp_update_svid_data_exit_mode(pd_port, expected_svid, ops);

	svdm_dfp_inform_exit_mode(pd_port, expected_svid, ops);
	vdm_put_dpm_notified_event(pd_port);
}

void hisi_pd_dpm_dfp_inform_attention(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint16_t svid = 0;
	uint8_t ops = 0;

	dpm_vdm_get_svid_ops(pd_event, &svid, &ops);
	D("Attention (svid0x%04x, mode:%u)\n", svid, ops);

	svdm_dfp_inform_attention(pd_port, svid, pd_event);
	vdm_put_dpm_notified_event(pd_port);
}

static void pd_dpm_dfp_extract_cable_vdo(pd_port_t *pd_port,
	int vdo_num, int vdo_ext_num)
{
#ifdef CONFIG_USB_PD_RESET_CABLE
	struct tcp_ny_cable_vdo  cable_vdo = {0};
	uint32_t vid;

	vid = pd_port->cable_vdos[VDO_INDEX_IDH] & VID_MASK;
	if (vid != PD_DPM_HW_VID)
		cable_vdo.vdo_ext = 0;
	else
		cable_vdo.vdo_ext = pd_port->cable_vdos[vdo_ext_num];

	cable_vdo.vdo = pd_port->cable_vdos[vdo_num];
	tcpci_notify_cable_vdo(pd_port->tcpc_dev, cable_vdo);
#endif
}

void hisi_pd_dpm_dfp_inform_cable_vdo(pd_port_t *pd_port, pd_event_t *pd_event)
{
	const int size = sizeof(uint32_t) * VDO_MAX_SIZE;
	int ret;

	if (pd_event->pd_msg) {
		ret = memcpy_s(pd_port->cable_vdos, size,
				pd_event->pd_msg->payload, size);
		if (ret != EOK)
			E("memcpy_s failed\n");

#ifdef CONFIG_USB_PD_RESET_CABLE
		if (pd_port->detect_emark) {
			pd_port->detect_emark = false;
			pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_CABLE_ID_DFP;
			pd_dpm_dfp_extract_cable_vdo(pd_port,
				VDO_INDEX_CABLE, VDO_INDEX_CABLE_EXT);
		}
#endif
	}

	vdm_put_dpm_notified_event(pd_port);
}

/* ---- Unstructured VDM ---- */
#ifdef CONFIG_USB_PD_UVDM_SUPPORT
void hisi_pd_dpm_ufp_recv_uvdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_msg_t *pd_msg;
	svdm_svid_data_t *svid_data;
	uint16_t svid = dpm_vdm_get_svid(pd_event);

	D("\n");
	svid_data = dpm_get_svdm_svid_data(pd_port, svid);
	pd_msg = pd_event->pd_msg;
	pd_port->uvdm_svid = svid;
	pd_port->uvdm_cnt = PD_HEADER_CNT(pd_msg->msg_hdr);
	if (memcpy_s(pd_port->uvdm_data,
			sizeof(uint32_t) * VDO_MAX_SIZE,
			pd_msg->payload,
			pd_port->uvdm_cnt * sizeof(uint32_t)))
		E("memcpy_s failed\n");

	if (svid_data && svid_data->ops->ufp_notify_uvdm)
		svid_data->ops->ufp_notify_uvdm(pd_port, svid_data);

	tcpci_notify_uvdm(pd_port->tcpc_dev, true);
}

void hisi_pd_dpm_dfp_send_uvdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("\n");
	pd_send_uvdm(pd_port, TCPC_TX_SOP);
	pd_port->uvdm_svid = PD_VDO_VID(pd_port->uvdm_data[0]);

	if (pd_port->uvdm_wait_resp)
		pd_enable_timer(pd_port, PD_TIMER_UVDM_RESPONSE);
}

void hisi_pd_dpm_dfp_inform_uvdm(pd_port_t *pd_port,
		pd_event_t *pd_event, bool ack)
{
	uint16_t svid;
	pd_msg_t *pd_msg = pd_event->pd_msg;
	uint16_t expected_svid = pd_port->uvdm_svid;
	svdm_svid_data_t *svid_data;

	D("\n");
	svid_data = dpm_get_svdm_svid_data(pd_port, expected_svid);
	if (ack && pd_port->uvdm_wait_resp) {
		svid = dpm_vdm_get_svid(pd_event);
		if (svid != expected_svid) {
			ack = false;
			DPM_DBG("Not expected SVID (0x%04x, 0x%04x)\n",
					svid, expected_svid);
		} else {
			pd_port->uvdm_cnt = PD_HEADER_CNT(pd_msg->msg_hdr);
			if (memcpy_s(pd_port->uvdm_data,
					sizeof(uint32_t) * VDO_MAX_SIZE,
					pd_msg->payload,
					pd_port->uvdm_cnt * sizeof(uint32_t)))
				E("memcpy_s failed\n");
		}
	}

	if (svid_data && svid_data->ops->dfp_notify_uvdm)
		svid_data->ops->dfp_notify_uvdm(pd_port, svid_data, ack);

	tcpci_notify_uvdm(pd_port->tcpc_dev, ack);
	return;
}
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

/* DRP : Inform Source/Sink Cap */
void hisi_pd_dpm_dr_inform_sink_cap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_msg_t *pd_msg = pd_event->pd_msg;
	pd_port_power_caps *snk_cap = &pd_port->remote_snk_cap;

	D("\n");
	if (pd_event_msg_match(pd_event, PD_EVT_DATA_MSG, PD_DATA_SINK_CAP)) {
		if (!pd_msg) {
			E("the pd_msg is NULL\n");
			return;
		}
		snk_cap->nr = PD_HEADER_CNT(pd_msg->msg_hdr);
		if (memcpy_s(snk_cap->pdos,
				sizeof(uint32_t) * PD_PORT_PDOS_MAX_SIZE,
				pd_msg->payload,
				sizeof(uint32_t) * snk_cap->nr))
			E("memcpy_s failed\n");

		pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_SINK_CAP;
	} else {
		if (pd_event_msg_match(pd_event,
				PD_EVT_CTRL_MSG, PD_CTRL_REJECT))
			pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_SINK_CAP;

		snk_cap->nr = 0;
		snk_cap->pdos[0] = 0;
	}

	pd_dpm_update_pdos_flags(pd_port, snk_cap->pdos[0]);
}

void hisi_pd_dpm_dr_inform_source_cap(pd_port_t *pd_port, pd_event_t *pd_event)
{
	pd_msg_t *pd_msg = pd_event->pd_msg;
	pd_port_power_caps *src_cap = &pd_port->remote_src_cap;

	D("\n");
	if (pd_event_msg_match(pd_event, PD_EVT_DATA_MSG, PD_DATA_SOURCE_CAP)) {
		if (!pd_msg) {
			E("the pd_msg is NULL\n");
			return;
		}
		src_cap->nr = PD_HEADER_CNT(pd_msg->msg_hdr);
		if (memcpy_s(src_cap->pdos,
				sizeof(uint32_t) * PD_PORT_PDOS_MAX_SIZE,
				pd_msg->payload,
				sizeof(uint32_t) * src_cap->nr))
			E("memcpy_s failed\n");

		pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_SOURCE_CAP;
	} else {
		if (pd_event_msg_match(pd_event,
				PD_EVT_CTRL_MSG, PD_CTRL_REJECT))
			pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_SOURCE_CAP;

		src_cap->nr = 0;
		src_cap->pdos[0] = 0;
	}

	pd_dpm_update_pdos_flags(pd_port, src_cap->pdos[0]);
}

/* DRP : Data Role Swap */

void hisi_pd_dpm_drs_evaluate_swap(pd_port_t *pd_port, uint8_t role)
{
	pd_put_dpm_ack_event(pd_port);
}

void hisi_pd_dpm_drs_change_role(pd_port_t *pd_port, uint8_t role)
{
	D("\n");
	hisi_pd_set_data_role(pd_port, role);

	pd_port->dpm_ack_immediately = true;

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY_DRSWAP
	pd_port->dpm_dfp_flow_delay_done = false;
#else
	pd_port->dpm_dfp_flow_delay_done = true;
#endif
#endif
}

/*
 *  DRP : Power Role Swap
 *  Rules:
 *      External Sources -> EXS
 *      Provider/Consumers -> PC
 *      Consumers/Provider -> CP
 *
 *      1.  PC (with EXS) shall always deny PR_SWAP from CP (without EXS)
 *
 *      2.  PC (without EXS) shall always acppet PR_SWAP from CP (with EXS)
 *          unless the requester isn't able to provide PDOs.
 */
void hisi_pd_dpm_prs_evaluate_swap(pd_port_t *pd_port, uint8_t role)
{
	int good_power;
	bool accept = true;
	bool sink = false;
	bool check_src = false;
	bool check_snk = false;
	bool check_ext = false;

	D("\n");
	check_src = !!(pd_port->dpm_caps & DPM_CAP_PR_SWAP_CHECK_GP_SRC);
	check_snk = !!(pd_port->dpm_caps & DPM_CAP_PR_SWAP_CHECK_GP_SNK);
	check_ext = !!(pd_port->dpm_flags & DPM_FLAGS_CHECK_EXT_POWER);

	if (check_src || check_snk || check_ext) {
		sink = pd_port->power_role == PD_ROLE_SINK;
		good_power = dpm_check_good_power(pd_port);

		switch (good_power) {
		case GOOD_PW_PARTNER:
			if (sink && check_snk)
				accept = false;
			break;

		case GOOD_PW_LOCAL:
			if ((!sink) && (check_src || check_ext))
				accept = false;
			break;

		case GOOD_PW_NONE:
			accept = true;
			break;

		default:
			accept = true;
			break;
		}
	}

	dpm_response_request(pd_port, accept);
}

void hisi_pd_dpm_prs_turn_off_power_sink(pd_port_t *pd_port)
{
	tcpci_sink_vbus(pd_port->tcpc_dev, TCP_VBUS_CTRL_PR_SWAP,
			TCPC_VBUS_SINK_0V, 0);
}

/*
 * 1.Wireless Charging, Vbus valid always.
 * 2.Vusb_uv_det was disabled, Vusb valid always
 * 5V_boost disabled, and Vusb be invalid less than 15ms later.
 */
static void hisi_pd_assume_vbus_invalid(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	msleep(20); /* 20: msleep time */
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->vbus_level = TCPC_VBUS_INVALID;
	tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_SAFE0V_ONCE;
	mutex_unlock(&tcpc_dev->access_lock);
	hisi_pd_put_vbus_changed_event(tcpc_dev, false);
}

void hisi_pd_dpm_prs_enable_power_source(pd_port_t *pd_port, bool en)
{
	int vbus_level = en ? TCPC_VBUS_SOURCE_5V : TCPC_VBUS_SOURCE_0V;

	D("\n");
	tcpci_source_vbus(pd_port->tcpc_dev,
			TCP_VBUS_CTRL_PR_SWAP, vbus_level, -1);

	if (en) {
		hisi_pd_enable_vbus_valid_detection(pd_port, en);
	} else {
		if (tcpci_tcpc_vusb_uv_det_masked(pd_port->tcpc_dev))
			hisi_pd_assume_vbus_invalid(pd_port);
		else
			hisi_pd_enable_vbus_safe0v_detection(pd_port);
	}
}

void hisi_pd_dpm_prs_change_role(pd_port_t *pd_port, uint8_t role)
{
	D("\n");
	hisi_pd_set_power_role(pd_port, role);
	pd_put_dpm_ack_event(pd_port);
}

/* DRP : Vconn Swap */
void hisi_pd_dpm_vcs_evaluate_swap(pd_port_t *pd_port)
{
	dpm_response_request(pd_port, true);
}

void hisi_pd_dpm_vcs_enable_vconn(pd_port_t *pd_port, bool en)
{
	D("\n");
	hisi_pd_dpm_enable_vconn(pd_port, en);

	/*
	 * If we can't enable vconn immediately, then after vconn_on,
	 * Vconn Controller should pd_put_dpm_ack_event()
	 */
	pd_port->dpm_ack_immediately = true;
}

#ifdef CONFIG_USB_PD_REV30
void pd_dpm_inform_alert(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint32_t *data = pd_get_msg_data_payload(pd_port);

	pd_port->remote_alert = data[0];
	tcpci_notify_alert(pd_port->tcpc_dev, data[0]);
}

int pd_dpm_send_alert(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint32_t ado = pd_port->local_alert;

	pd_port->local_alert = 0;
	D("send_alert:0x%08x\n", ado);

	return pd_send_message(pd_port, TCPC_TX_SOP, PD_DATA_ALERT,
			PD_ADO_SIZE, &ado);
}

#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL
int pd_dpm_send_status(pd_port_t *pd_port)
{
	struct pd_status sdb;

	if (memset_s(&sdb, sizeof(struct pd_status), 0, sizeof(struct pd_status)))
		E("clr sdb\n");
	sdb.present_input = pd_port->pd_status_present_in;

#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL_TEMP
	sdb.internal_temp = pd_port->pd_status_temp;
	sdb.temp_status = pd_port->pd_status_temp_status;
#endif
	if (sdb.event_flags & PD_STATUS_EVENT_OTP)
		sdb.temp_status = PD_STATUS_TEMP_SET_PTF(PD_PTF_OVER_TEMP);

	if (pd_port->power_role !=  PD_ROLE_SINK)
		sdb.event_flags &= ~PD_STATUS_EVENT_OVP;

	return pd_send_ext_msg(pd_port, TCPC_TX_SOP, PD_EXT_STATUS,
			false, 0, PD_SDB_SIZE, (void *)&sdb);
}
#endif
#endif

/* PE : Notify DPM */
static bool pd_dpm_ready_power_role_swap(pd_port_t *pd_port)
{
	bool do_swap = false;
	uint32_t prefer_role;

	D("+\n");
	if (!(pd_port->dpm_flags & DPM_FLAGS_CHECK_PR_ROLE))
		return false;

	pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_PR_ROLE;
	prefer_role = DPM_CAP_EXTRACT_PR_CHECK(pd_port->dpm_caps);
	if (pd_port->power_role == PD_ROLE_SOURCE &&
			prefer_role == DPM_CAP_PR_CHECK_PREFER_SNK)
		do_swap = true;

	if (pd_port->power_role == PD_ROLE_SINK &&
			prefer_role == DPM_CAP_PR_CHECK_PREFER_SRC)
		do_swap = true;

	if (do_swap)
		pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_DR_SWAP);

	D("-\n");
	return do_swap;
}

static int pd_dpm_ready_data_role_swap(pd_port_t *pd_port)
{
	bool do_swap = false;
	uint32_t prefer_role;

	D("+\n");
	if (!(pd_port->dpm_flags & DPM_FLAGS_CHECK_DR_ROLE))
		return 0;

	pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_DR_ROLE;
	if (!(pd_port->dpm_flags & DPM_FLAGS_PARTNER_DR_DATA))
		return false;

	prefer_role = DPM_CAP_EXTRACT_DR_CHECK(pd_port->dpm_caps);
	if (pd_port->data_role == PD_ROLE_DFP &&
			prefer_role == DPM_CAP_DR_CHECK_PREFER_UFP)
		do_swap = true;

	if (pd_port->data_role == PD_ROLE_UFP &&
			prefer_role == DPM_CAP_DR_CHECK_PREFER_DFP)
		do_swap = true;

	if (do_swap)
		pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_DR_SWAP);

	D("-\n");
	return do_swap;
}

static int pd_dpm_ready_get_sink_cap(pd_port_t *pd_port)
{
	if (!(pd_port->dpm_flags & DPM_FLAGS_CHECK_SINK_CAP))
		return 0;

	if (pd_port->get_snk_cap_count >= PD_GET_SNK_CAP_RETRIES)
		return 0;

	pd_port->get_snk_cap_count++;
	pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_GET_SINK_CAP);

	return 1;
}

static int pd_dpm_ready_get_source_cap(pd_port_t *pd_port)
{
	if (!(pd_port->dpm_flags & DPM_FLAGS_CHECK_SOURCE_CAP))
		return 0;

	if (pd_port->get_src_cap_count >= PD_GET_SRC_CAP_RETRIES)
		return 0;

	pd_port->get_src_cap_count++;
	pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_GET_SOURCE_CAP);

	return 1;
}

static int pd_dpm_ready_attempt_get_flags(pd_port_t *pd_port)
{
	uint32_t prefer_role;
	bool attemp_get_flags = false;

	D("+\n");
	if (pd_port->remote_src_cap.nr >= 1)
		return 0;

	if (pd_port->remote_snk_cap.nr >= 1)
		return 0;

	if (pd_port->dpm_flags & DPM_FLAGS_CHECK_DR_ROLE) {
		prefer_role = DPM_CAP_EXTRACT_DR_CHECK(pd_port->dpm_caps);
		if (prefer_role == DPM_CAP_DR_CHECK_PREFER_UFP)
			attemp_get_flags = true;
	}

	if (pd_port->dpm_flags & DPM_FLAGS_CHECK_EXT_POWER)
		attemp_get_flags = true;

	if (!attemp_get_flags)
		return 0;

	if (pd_port->get_snk_cap_count >= PD_GET_SNK_CAP_RETRIES)
		return 0;

	pd_port->get_snk_cap_count++;
	pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_GET_SINK_CAP);
	D("-\n");
	return 1;
}

static inline int pd_dpm_notify_pe_src_ready(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	D("\n");
	return pd_dpm_ready_attempt_get_flags(pd_port);
}

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID

#define PD_DPM_RTN_VCONN 2
static int pd_dpm_support_vswap(pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_PD_DFP_RESET_CABLE
	if (!hisi_usb_typec_force_vconn() &&
			(pd_port->vswap_ret == PD_CTRL_REJECT)) {
		D("vswap reject return\n");
		return 0;
	}
#endif
	return 1;
}

static int pd_dpm_attempt_vswap(pd_port_t *pd_port, pd_event_t *pd_event)
{
#ifdef CONFIG_PD_DFP_RESET_CABLE
	if (!pd_port->detect_emark) {
		D("detect_emark false\n");
		return 0;
	}

	if (!pd_dpm_support_vswap(pd_port, pd_event))
		return 0;
#endif

	pd_port->vconn_return = true;
	pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_VCONN_SWAP);

	return 1;
}

static int __pd_dpm_ready_attempt_discover_cable(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	if (!(pd_port->dpm_flags & DPM_FLAGS_CHECK_CABLE_ID_DFP)) {
		D("check_cable_id_dfp not support\n");
		return PD_DPM_RTN_VCONN;
	}

	D("check_cable_id_dfp\n");
	if (!pd_is_auto_discover_cable_id(pd_port))
		return PD_DPM_RTN_VCONN;

#ifdef CONFIG_USB_PD_DISCOVER_CABLE_REQUEST_VCONN
	if (!pd_port->vconn_source)
		return pd_dpm_attempt_vswap(pd_port, pd_event);
#endif

#ifdef CONFIG_PD_DFP_RESET_CABLE
	if (pd_port->reset_cable) {
		pd_port->reset_cable = false;
		pd_put_dpm_pd_request_event(pd_port,
			PD_DPM_PD_REQUEST_CBL_SOFTRESET);
		return 1;
	}
#endif

	pd_restart_timer(pd_port, PD_TIMER_DISCOVER_ID);
	return 1;
}

static int pd_dpm_ready_attempt_discover_cable(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	int ret;

	D("+\n");
	ret = __pd_dpm_ready_attempt_discover_cable(pd_port, pd_event);
	if (ret != PD_DPM_RTN_VCONN)
		return ret;

#ifdef CONFIG_USB_PD_DISCOVER_CABLE_RETURN_VCONN
	if (pd_port->vconn_return) {
		DPM_DBG("VconnReturn\n");
		pd_port->vconn_return = false;
		if (pd_port->vconn_source) {
			pd_put_dpm_pd_request_event(pd_port,
					PD_DPM_PD_REQUEST_VCONN_SWAP);
			return 1;
		}
	}
#endif
	D("-\n");
	return 0;
}
#endif

static int pd_dpm_notify_pe_dfp_ready(pd_port_t *pd_port,
		pd_event_t *pd_event)
{
	D("+\n");

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
	if (!pd_port->dpm_dfp_flow_delay_done) {
		DPM_DBG("Delay DFP Flow\n");
		pd_restart_timer(pd_port, PD_TIMER_DFP_FLOW_DELAY);
		return 1;
	}
#endif

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	if (pd_dpm_ready_attempt_discover_cable(pd_port, pd_event))
		return 1;
#endif

#ifdef CONFIG_USB_PD_ATTEMP_DISCOVER_ID
	if (pd_port->dpm_flags & DPM_FLAGS_CHECK_UFP_ID) {
		D("Discover ID\n");
		if (vdm_put_dpm_vdm_request_event(pd_port,
				PD_DPM_VDM_REQUEST_DISCOVER_ID))
			return 1;
	}
#endif

#ifdef CONFIG_USB_PD_ATTEMP_DISCOVER_SVID
	if (pd_port->dpm_flags & DPM_FLAGS_CHECK_UFP_SVID) {
		D("Discover SVID\n");
		if (vdm_put_dpm_vdm_request_event(pd_port,
				PD_DPM_VDM_REQUEST_DISCOVER_SVIDS))
			return 1;
	}
#endif

#ifdef CONFIG_USB_PD_MODE_OPERATION
	if (svdm_notify_pe_ready(pd_port, pd_event))
		return 1;
#endif
	D("-\n");
	return 0;
}

int hisi_pd_dpm_notify_pe_startup(pd_port_t *pd_port)
{
	uint32_t caps;
	uint32_t flags = 0;

	D("+\n");
	caps = DPM_CAP_EXTRACT_PR_CHECK(pd_port->dpm_caps);
	if (caps != DPM_CAP_PR_CHECK_DISABLE)
		flags |= DPM_FLAGS_CHECK_PR_ROLE;

	caps = DPM_CAP_EXTRACT_DR_CHECK(pd_port->dpm_caps);
	if (caps != DPM_CAP_DR_CHECK_DISABLE)
		flags |= DPM_FLAGS_CHECK_DR_ROLE;

	if (pd_port->dpm_caps & DPM_CAP_PR_SWAP_CHECK_GP_SRC)
		flags |= DPM_FLAGS_CHECK_EXT_POWER;

	if (pd_port->dpm_caps & DPM_CAP_PR_SWAP_CHECK_GP_SNK)
		flags |= DPM_FLAGS_CHECK_EXT_POWER;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_EXT_POWER)
		flags |= DPM_FLAGS_CHECK_EXT_POWER;

	if (pd_port->dpm_caps & DPM_CAP_ATTEMP_DISCOVER_CABLE)
		flags |= DPM_FLAGS_CHECK_CABLE_ID;

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
	if (pd_port->dpm_caps & DPM_CAP_ATTEMP_ENTER_DP_MODE) {
		flags |= DPM_FLAGS_CHECK_DP_MODE;
		flags |= DPM_FLAGS_CHECK_UFP_ID;
		flags |= DPM_FLAGS_CHECK_UFP_SVID;
	}
#endif

	if (pd_port->dpm_caps & DPM_CAP_ATTEMP_DISCOVER_ID)
		flags |= DPM_FLAGS_CHECK_UFP_ID;

	pd_port->dpm_flags = flags;
	pd_port->dpm_dfp_retry_cnt = CONFIG_USB_PD_DFP_FLOW_RETRY_MAX;

	svdm_notify_pe_startup(pd_port);
	D("-\n");
	return 0;
}

int hisi_pd_dpm_notify_pe_hardreset(pd_port_t *pd_port)
{
	uint32_t flags = 0;

	D("+\n");
	if (pd_port->dpm_dfp_retry_cnt) {
		pd_port->dpm_dfp_retry_cnt--;
#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
		if (pd_port->dpm_caps & DPM_CAP_ATTEMP_ENTER_DP_MODE) {
			flags |= DPM_FLAGS_CHECK_DP_MODE;
			flags |= DPM_FLAGS_CHECK_UFP_ID;
			flags |= DPM_FLAGS_CHECK_UFP_SVID;
		}
#endif
		pd_port->dpm_flags |= flags;
		svdm_notify_pe_startup(pd_port);
	}
	D("-\n");
	return 0;
}

int hisi_pd_dpm_notify_pe_ready(pd_port_t *pd_port, pd_event_t *pd_event)
{
	int ret = 0;

	D("dpm_flags 0x%08x\n", pd_port->dpm_flags);
	pd_notify_tcp_event_2nd_result(pd_port, TCP_DPM_RET_SUCCESS);

#ifdef CONFIG_USB_PD_HANDLE_PRDR_SWAP
	if (pd_port->postpone_pr_swap) {
		pd_port->postpone_pr_swap = false;
		D("Handle postpone pr_swap\n");
		pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_PR_SWAP);
		return 1;
	}

	if (pd_port->postpone_dr_swap) {
		pd_port->postpone_dr_swap = false;
		D("Handle postpone dr_swap\n");
		pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_DR_SWAP);
		return 1;
	}
#endif
	if (pd_dpm_ready_get_source_cap(pd_port))
		return 1;

	if (pd_dpm_ready_get_sink_cap(pd_port))
		return 1;

	if (pd_dpm_ready_power_role_swap(pd_port))
		return 1;

	if (pd_port->power_role == PD_ROLE_SOURCE)
		ret = pd_dpm_notify_pe_src_ready(pd_port, pd_event);

	if (ret != 0)
		return ret;

	if (pd_dpm_ready_data_role_swap(pd_port))
		return 1;

	if (pd_port->data_role == PD_ROLE_DFP)
		ret = pd_dpm_notify_pe_dfp_ready(pd_port, pd_event);

	if (ret != 0)
		return ret;

	if (!pd_port->pe_ready) {
		pd_port->pe_ready = true;
		hisi_pd_update_connect_state(pd_port, HISI_PD_CONNECT_PE_READY);
	}

	return 0;
}

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
int hisi_pd_dpm_notify_dfp_delay_done(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("\n");
	if (pd_port->data_role == PD_ROLE_DFP) {
		pd_port->dpm_dfp_flow_delay_done = true;
		pd_dpm_notify_pe_dfp_ready(pd_port, pd_event);
	}

	return 0;
}
#endif

#ifdef CONFIG_USB_PD_UFP_FLOW_DELAY
int hisi_pd_dpm_notify_ufp_delay_done(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("\n");
	if (pd_port->data_role == PD_ROLE_UFP) {
		pd_port->dpm_ufp_flow_delay_done = true;
		hisi_pd_dpm_notify_pe_ready(pd_port, pd_event);
	}

	return 0;
}
#endif

/* dpm_core_init */
int hisi_pd_dpm_core_init(pd_port_t *pd_port)
{
	unsigned int i;

	D("\n");
	for (i = 0; i < ARRAY_SIZE(svdm_svid_ops); i++)
		dpm_register_svdm_ops(pd_port, &svdm_svid_ops[i]);

	return 0;
}
