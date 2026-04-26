/*
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

#define LOG_TAG "[tcpm]"

#include <linux/hisi/usb/hisi_tcpm.h>
#include <linux/hisi/usb/hisi_typec.h>

#include <securec.h>

#include "include/tcpci.h"
#include "include/tcpci_typec.h"

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#include "include/pd_policy_engine.h"
#include "include/pd_dpm_core.h"
#include "include/pd_dpm_pdo_select.h"
#endif

int hisi_tcpm_shutdown(struct tcpc_device *tcpc_dev)
{
	if (tcpc_dev->ops->deinit)
		tcpc_dev->ops->deinit(tcpc_dev);

	return 0;
}

int hisi_tcpm_inquire_remote_cc(struct tcpc_device *tcpc_dev,
		uint8_t *cc1, uint8_t *cc2, bool from_ic)
{
	int rv, t_cc1, t_cc2;

	if (from_ic) {
		if (tcpc_dev->ops && tcpc_dev->ops->get_cc) {
			rv = tcpc_dev->ops->get_cc(tcpc_dev, &t_cc1, &t_cc2);
			if (rv == 0) {
				*cc1 = (uint8_t)(uint32_t)t_cc1;
				*cc2 = (uint8_t)(uint32_t)t_cc2;
				return 0;
			}
		}
	}

	*cc1 = tcpc_dev->typec_remote_cc[0];
	*cc2 = tcpc_dev->typec_remote_cc[1];

	return 0;
}

void hisi_tcpm_force_cc_mode(struct tcpc_device *tcpc_dev, int mode)
{
	int pull = mode ? TYPEC_CC_DRP : TYPEC_CC_RD;

	I("-- mode:%d\n", mode);

	if ((pull == TYPEC_CC_DRP) &&
		(tcpci_ccdebouce_timer_enabled(tcpc_dev))) {
		I("CC Det, Bypass force DRP\n");
		return;
	}

	tcpci_set_cc(tcpc_dev, pull);

	if (pull == TYPEC_CC_RD)
		tcpc_dev->typec_role = TYPEC_ROLE_SNK;
	else
		tcpc_dev->typec_role = TYPEC_ROLE_TRY_SNK;
}

int hisi_tcpm_typec_set_wake_lock(struct tcpc_device *tcpc, bool wake_lock)
{
	if (wake_lock) {
		D("lock attach_wake_lock\n");
		__pm_stay_awake(&tcpc->attach_wake_lock);
	} else {
		D("unlock attach_wake_lock\n");
		__pm_relax(&tcpc->attach_wake_lock);
	}

	return 0;
}

int hisi_tcpm_typec_set_rp_level(struct tcpc_device *tcpc_dev, uint8_t level)
{
	uint8_t res;

	D("%u\n", level);
	if (level == 2) /* 2: TYPEC_CC_RP_3_0 */
		res = TYPEC_CC_RP_3_0;
	else if (level == 1)
		res = TYPEC_CC_RP_1_5;
	else
		res = TYPEC_CC_RP_DFT;

	return hisi_tcpc_typec_set_rp_level(tcpc_dev, res);
}

/*
 * Force role swap, include power and data role, I guess!
 */
int hisi_tcpm_typec_role_swap(struct tcpc_device *tcpc_dev)
{
	D("typec_attach_old %u\n", tcpc_dev->typec_attach_old);

	if (tcpc_dev->typec_attach_old == TYPEC_UNATTACHED)
		return TCPM_ERROR_UNATTACHED;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (hisi_tcpc_typec_swap_role(tcpc_dev) == 0)
		return TCPM_SUCCESS;
#endif

	return TCPM_ERROR_UNSUPPORT;
}

/*
 * Vendor defined case!
 */
int hisi_tcpm_typec_notify_direct_charge(struct tcpc_device *tcpc_dev,
		bool direct_charge)
{
	if (tcpc_dev->typec_attach_old == TYPEC_UNATTACHED)
		return TCPM_ERROR_UNATTACHED;

	return hisi_tcpc_typec_set_direct_charge(tcpc_dev, direct_charge);
}

int hisi_tcpm_typec_change_role(struct tcpc_device *tcpc_dev,
		uint8_t typec_role)
{
	return hisi_tcpc_typec_change_role(tcpc_dev, typec_role);
}

/* Request TCPC to send PD Request */
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static int tcpm_check_pd_attached(struct tcpc_device *tcpc)
{
	D("\n");
	if (tcpc->typec_attach_old == TYPEC_UNATTACHED) {
		D("TYPEC_UNATTACHED\n");
		return TCPM_ERROR_UNATTACHED;
	}

	if (!tcpc->pd_port.pd_prev_connected)
		return TCPM_ERROR_NO_PD_CONNECTED;

	return TCPM_SUCCESS;
}

bool hisi_tcpm_support_pd(struct tcpc_device *tcpc)
{
	pd_port_t *pd_port = &tcpc->pd_port;

	return pd_port->remote_src_cap.nr > 0;
}

bool hisi_tcpm_support_apdo(struct tcpc_device *tcpc)
{
	return pd_is_source_support_apdo(&tcpc->pd_port);
}

int hisi_tcpm_power_role_swap(struct tcpc_device *tcpc_dev)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpc_dev->typec_attach_old == TYPEC_UNATTACHED) {
		D("TYPEC_UNATTACHED\n");
		return TCPM_ERROR_UNATTACHED;
	}

	if (!pd_port->pd_prev_connected) {
#ifdef CONFIG_USB_PD_AUTO_TYPEC_ROLE_SWAP
		return hisi_tcpm_typec_role_swap(tcpc_dev);
#else
		D("pd_prev_connected is 0!\n");
		return TCPM_ERROR_NO_PD_CONNECTED;
#endif
	}

	/* Put a request into pd state machine. */
	ret = pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_PR_SWAP);
	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_power_role_swap);

int hisi_tcpm_data_role_swap(struct tcpc_device *tcpc_dev)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpc_dev->typec_attach_old == TYPEC_UNATTACHED)
		return TCPM_ERROR_UNATTACHED;

	if (!pd_port->pd_prev_connected) {
#ifdef CONFIG_USB_PD_AUTO_TYPEC_ROLE_SWAP
		return hisi_tcpm_typec_role_swap(tcpc_dev);
#else
		return TCPM_ERROR_NO_PD_CONNECTED;
#endif
	}

	ret = pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_DR_SWAP);
	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_data_role_swap);

int hisi_tcpm_vconn_swap(struct tcpc_device *tcpc_dev)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	ret = pd_put_dpm_pd_request_event(pd_port,
			PD_DPM_PD_REQUEST_VCONN_SWAP);
	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_vconn_swap);

int hisi_tcpm_soft_reset(struct tcpc_device *tcpc_dev)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	ret = pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_SOFTRESET);
	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_soft_reset);

int hisi_tcpm_hard_reset(struct tcpc_device *tcpc_dev)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	ret = pd_put_dpm_pd_request_event(pd_port, PD_DPM_PD_REQUEST_HARDRESET);
	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_hard_reset);

int hisi_tcpm_get_source_cap(struct tcpc_device *tcpc_dev,
		struct tcpm_power_cap *cap)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	if (!cap) {
		ret = pd_put_dpm_pd_request_event(pd_port,
				PD_DPM_PD_REQUEST_GET_SOURCE_CAP);
		if (!ret)
			return TCPM_ERROR_PUT_EVENT;
	} else {
		mutex_lock(&pd_port->pd_lock);
		cap->cnt = pd_port->remote_src_cap.nr;
		if (memcpy_s(cap->pdos,
				sizeof(uint32_t) * TCPM_PDO_MAX_SIZE,
				pd_port->remote_src_cap.pdos,
				sizeof(uint32_t) * cap->cnt) != EOK)
			E("memcpy_s failed\n");
		mutex_unlock(&pd_port->pd_lock);
	}

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_get_source_cap);

int hisi_tcpm_get_sink_cap(struct tcpc_device *tcpc_dev,
		struct tcpm_power_cap *cap)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	if (!cap) {
		ret = pd_put_dpm_pd_request_event(pd_port,
				PD_DPM_PD_REQUEST_GET_SINK_CAP);
		if (!ret)
			return TCPM_ERROR_PUT_EVENT;
	} else {
		mutex_lock(&pd_port->pd_lock);
		cap->cnt = pd_port->remote_snk_cap.nr;
		if (memcpy_s(cap->pdos,
				sizeof(uint32_t) * TCPM_PDO_MAX_SIZE,
				pd_port->remote_snk_cap.pdos,
				sizeof(uint32_t) * cap->cnt) != EOK)
			E("memcpy_s failed\n");
		mutex_unlock(&pd_port->pd_lock);
	}

	return TCPM_SUCCESS;
}
EXPORT_SYMBOL(hisi_tcpm_get_sink_cap);

static int hisi_tcpm_get_local_sink_cap(struct tcpc_device *tcpc_dev,
		struct local_sink_cap *cap)
{
	int i;
	pd_port_t *pd_port = &tcpc_dev->pd_port;
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;
	struct dpm_pdo_info_t sink;

	if (!pd_port->pd_prev_connected)
		return TCPM_ERROR_NO_PD_CONNECTED;

	for (i = 0; i < snk_cap->nr; i++) {
		hisi_dpm_extract_pdo_info(snk_cap->pdos[i], &sink);
		cap[i].mv = sink.vmin;
		cap[i].ma = sink.ma;
		cap[i].uw = sink.uw;
	}

	return TCPM_SUCCESS;
}

static int hisi_tcpm_request(struct tcpc_device *tcpc_dev, int mv, int ma)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	mutex_lock(&pd_port->pd_lock);
	ret = hisi_pd_dpm_send_request(pd_port, mv, ma);
	mutex_unlock(&pd_port->pd_lock);

	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}

void hisi_tcpm_request_voltage(struct tcpc_device *tcpc_dev, int set_voltage)
{
	bool overload = false;
	int vol_mv = 0;
	int cur_ma = 0;
	int max_uw = 0;
	int ret, i;
	struct local_sink_cap sink_cap_info[TCPM_PDO_MAX_SIZE] = { {0} };

	D("+\n");
	if (!hisi_tcpm_support_pd(tcpc_dev))
		return;

	ret = hisi_tcpm_get_local_sink_cap(tcpc_dev, sink_cap_info);
	if (ret != TCPM_SUCCESS) {
		D("hisi_tcpm_get_local_sink_cap ret %d\n", ret);
		return;
	}

	for (i = 0; i < TCPM_PDO_MAX_SIZE; i++) {
		if (sink_cap_info[i].mv > set_voltage)
			continue;
		overload = (sink_cap_info[i].uw > max_uw)
				|| ((sink_cap_info[i].uw == max_uw)
					&& (sink_cap_info[i].mv < vol_mv));
		if (overload) {
			max_uw = sink_cap_info[i].uw;
			vol_mv = sink_cap_info[i].mv;
			cur_ma = sink_cap_info[i].ma;
		}
	}

	if (!(vol_mv && cur_ma)) {
		D("vol or cur == 0\n");
		return;
	}

	ret = hisi_tcpm_request(tcpc_dev, vol_mv, cur_ma);
	if (ret != TCPM_SUCCESS)
		TYPEC_INFO("hisi_tcpm_request ret %d\n", ret);

	D("-\n");
}
EXPORT_SYMBOL(hisi_tcpm_request_voltage);

int hisi_tcpm_discover_cable(struct tcpc_device *tcpc_dev, uint32_t *vdos,
		unsigned int max_size)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	D("+\n");
	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	if (!vdos) {
		mutex_lock(&pd_port->pd_lock);
		pd_port->dpm_flags |= DPM_FLAGS_CHECK_CABLE_ID;
		ret = vdm_put_dpm_discover_cable_event(pd_port);
		mutex_unlock(&pd_port->pd_lock);

		if (!ret)
			return TCPM_ERROR_PUT_EVENT;
	} else {
		if (!pd_port->power_cable_present)
			return TCPM_ERROR_UNKNOWN;

		if (max_size > VDO_MAX_SIZE)
			max_size = VDO_MAX_SIZE;

		mutex_lock(&pd_port->pd_lock);
		if (memcpy_s(vdos,
				sizeof(uint32_t) * max_size,
				pd_port->cable_vdos,
				sizeof(uint32_t) * max_size) != EOK)
			E("memcpy_s failed\n");
		mutex_unlock(&pd_port->pd_lock);
	}

	D("-\n");
	return TCPM_SUCCESS;
}

void hisi_tcpm_detect_emark_cable(struct tcpc_device *tcpc_dev)
{
	hisi_pd_cable_flag_set(&tcpc_dev->pd_port);
	(void)hisi_tcpm_data_role_swap(tcpc_dev);
	tcpc_dev->pd_port.dpm_flags |= DPM_FLAGS_CHECK_CABLE_ID_DFP;
}

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
int hisi_tcpm_send_uvdm(struct tcpc_device *tcpc_dev,
		uint8_t cnt, uint32_t *data, bool wait_resp)
{
	bool ret = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	D("+\n");
	if (tcpm_check_pd_attached(tcpc_dev))
		return ret;

	if (cnt > VDO_MAX_SIZE)
		return TCPM_ERROR_PARAMETER;

	mutex_lock(&pd_port->pd_lock);

	pd_port->uvdm_cnt = cnt;
	pd_port->uvdm_wait_resp = wait_resp;
	if (memcpy_s(pd_port->uvdm_data,
			sizeof(uint32_t) * VDO_MAX_SIZE,
			data,
			sizeof(uint32_t) * cnt) != EOK)
		E("memcpy_s failed\n");

	ret = vdm_put_dpm_vdm_request_event(
		pd_port, PD_DPM_VDM_REQUEST_UVDM);

	mutex_unlock(&pd_port->pd_lock);

	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	D("-\n");
	return 0;
}
EXPORT_SYMBOL(hisi_tcpm_send_uvdm);
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

static bool tcpm_extract_power_cap_val(uint32_t pdo, struct tcpm_power_cap_val *cap)
{
	struct dpm_pdo_info_t info;

	hisi_dpm_extract_pdo_info(pdo, &info);

	cap->type = info.type;
	cap->min_mv = info.vmin;
	cap->max_mv = info.vmax;

	if (info.type == DPM_PDO_TYPE_BAT)
		cap->uw = info.uw;
	else
		cap->ma = info.ma;

#ifdef CONFIG_USB_PD_REV30
	if (info.type == DPM_PDO_TYPE_APDO)
		cap->apdo_type = info.apdo_type;
#endif

	return cap->type != TCPM_POWER_CAP_VAL_TYPE_UNKNOWN;
}

int tcpm_get_remote_power_cap(struct tcpc_device *tcpc_dev,
		struct tcpm_remote_power_cap *remote_cap)
{
	struct tcpm_power_cap_val cap;
	int i;

	remote_cap->selected_cap_idx =
		tcpc_dev->pd_port.remote_selected_cap;
	remote_cap->nr = tcpc_dev->pd_port.remote_src_cap.nr;
	for (i = 0; i < remote_cap->nr; i++) {
		tcpm_extract_power_cap_val(
			tcpc_dev->pd_port.remote_src_cap.pdos[i], &cap);
		remote_cap->max_mv[i] = cap.max_mv;
		remote_cap->min_mv[i] = cap.min_mv;
		remote_cap->ma[i] = cap.ma;
		remote_cap->type[i] = cap.type;
	}
	return TCPM_SUCCESS;
}

#ifdef CONFIG_USB_PD_REV30
static int tcpm_put_tcp_dpm_event(struct tcpc_device *tcpc,
		struct tcp_dpm_event *event)
{
	bool ret;

	ret = tcpm_check_pd_attached(tcpc);
	if (ret != TCPM_SUCCESS)
		return ret;

	ret = pd_put_deferred_tcp_event(tcpc, event);
	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}

static int tcpm_put_tcp_dpm_event_cb(struct tcpc_device *tcpc,
		struct tcp_dpm_event *event,
		const struct tcp_dpm_event_cb_data *cb_data)
{
	event->user_data = cb_data->user_data;
	event->event_cb = cb_data->event_cb;

	return tcpm_put_tcp_dpm_event(tcpc, event);
}

static void tcpm_dpm_bk_copy_data(pd_port_t *pd_port)
{
	uint8_t size = pd_port->tcpm_bk_cb_data_max;
	errno_t errno;

	if (size >= pd_get_msg_data_size(pd_port))
		size = pd_get_msg_data_size(pd_port);

	if (pd_port->tcpm_bk_cb_data != NULL) {
		errno = memcpy_s(pd_port->tcpm_bk_cb_data, size,
			pd_get_msg_data_payload(pd_port), size);
		if (errno)
			E("cpy cb data\n");
	}
}

static int tcpm_dpm_bk_event_cb(struct tcpc_device *tcpc,
		int ret, struct tcp_dpm_event *event)
{
	pd_port_t *pd_port = &tcpc->pd_port;

	if (pd_port->tcpm_bk_event_id != event->event_id) {
		D("expect:%u real:%u\n",
			pd_port->tcpm_bk_event_id, event->event_id);
		return 0;
	}

	pd_port->tcpm_bk_ret = ret;
	pd_port->tcpm_bk_done = true;

	if (ret == 0)
		tcpm_dpm_bk_copy_data(pd_port);

	wake_up_interruptible(&pd_port->tcpm_bk_wait_que);
	return 0;
}

static int tcpm_dpm_wait_bk_event(pd_port_t *pd_port, uint32_t tout_ms)
{
	wait_event_interruptible_timeout(pd_port->tcpm_bk_wait_que,
			pd_port->tcpm_bk_done, msecs_to_jiffies(tout_ms));

	if (pd_port->tcpm_bk_done)
		return pd_port->tcpm_bk_ret;

	mutex_lock(&pd_port->pd_lock);
	pd_port->tcpm_bk_event_id = TCP_DPM_EVT_UNKONW;
	pd_port->tcpm_bk_cb_data = NULL;
	mutex_unlock(&pd_port->pd_lock);

	return TCP_DPM_RET_BK_TIMEOUT;
}

static int __tcpm_put_tcp_dpm_event_bk(
		struct tcpc_device *tcpc, struct tcp_dpm_event *event,
		uint32_t tout_ms, uint8_t *data, uint8_t size)
{
	int ret;
	pd_port_t *pd_port = &tcpc->pd_port;
	mutex_lock(&pd_port->pd_lock);
	pd_port->tcpm_bk_done = false;
	pd_port->tcpm_bk_event_id = event->event_id;
	pd_port->tcpm_bk_cb_data = data;
	pd_port->tcpm_bk_cb_data_max = size;
	mutex_unlock(&pd_port->pd_lock);
	ret = tcpm_put_tcp_dpm_event(tcpc, event);
	if (ret != TCPM_SUCCESS)
		return ret;

	return tcpm_dpm_wait_bk_event(pd_port, tout_ms);
}

static int tcpm_put_tcp_dpm_event_bk(struct tcpc_device *tcpc,
		struct tcp_dpm_event *event, uint32_t tout_ms,
		uint8_t *data, uint8_t size)
{
	int ret;
	uint8_t retry = 3;
	pd_port_t *pd_port = &tcpc->pd_port;

	event->event_cb = tcpm_dpm_bk_event_cb;

	mutex_lock(&pd_port->tcpm_bk_lock);

	while (1) {
		ret = __tcpm_put_tcp_dpm_event_bk(
			tcpc, event, tout_ms, data, size);
		if (retry > 0 &&
		    (ret == TCP_DPM_RET_TIMEOUT ||
		    ret == TCP_DPM_RET_DROP_DISCARD ||
		    ret == TCP_DPM_RET_DROP_UNEXPECTED)) {
			retry--;
			continue;
		}
		break;
	}

	mutex_unlock(&pd_port->tcpm_bk_lock);

	if (ret == TCP_DPM_RET_DENIED_REPEAT_REQUEST)
		ret = TCPM_SUCCESS;

	return ret;
}

static int tcpm_put_tcp_dpm_event_cbk1(struct tcpc_device *tcpc,
		struct tcp_dpm_event *event,
		struct tcp_dpm_event_cb_data *cb_data, uint32_t tout_ms)
{
	if (!cb_data) {
		return tcpm_put_tcp_dpm_event_bk(
			tcpc, event, tout_ms, NULL, 0);
	}

	return tcpm_put_tcp_dpm_event_cb(tcpc, event, cb_data);
}

static int tcpm_put_tcp_dpm_event_cbk2(struct tcpc_device *tcpc,
		struct tcp_dpm_event *event,
		const struct tcp_dpm_event_cb_data *cb_data,
		uint32_t tout_ms, uint8_t *data, uint8_t size)
{
	if (!cb_data) {
		return tcpm_put_tcp_dpm_event_bk(
			tcpc, event, tout_ms, data, size);
	}

	return tcpm_put_tcp_dpm_event_cb(tcpc, event, cb_data);
}

#define TCPM_BK_PD_CMD_TOUT	500
#define TCPM_BK_REQUEST_TOUT	1500
#define TCPM_SECDB_SIZE		24
int tcpm_dpm_pd_get_source_cap_ext(struct tcpc_device *tcpc,
		const struct tcp_dpm_event_cb_data *cb_data,
		struct pd_source_cap_ext *src_cap_ext)
{
	struct tcp_dpm_event tcp_event = {
		.event_id = TCP_DPM_EVT_GET_SOURCE_CAP_EXT,
	};

	return tcpm_put_tcp_dpm_event_cbk2(tcpc, &tcp_event, cb_data,
			TCPM_BK_PD_CMD_TOUT, (uint8_t *)src_cap_ext, TCPM_SECDB_SIZE);
}

int tcpm_dpm_pd_get_status(struct tcpc_device *tcpc,
		struct tcp_dpm_event_cb_data *cb_data, struct pd_status *status)
{
	struct tcp_dpm_event tcp_event = {
		.event_id = TCP_DPM_EVT_GET_STATUS,
	};

	return tcpm_put_tcp_dpm_event_cbk2(tcpc, &tcp_event, cb_data,
			TCPM_BK_PD_CMD_TOUT, (uint8_t *)status, PD_SDB_SIZE);
}

static int tcpm_dpm_pd_get_pps_status_raw(struct tcpc_device *tcpc,
		const struct tcp_dpm_event_cb_data *cb_data,
		struct pd_pps_status_raw *pps_status)
{
	struct tcp_dpm_event tcp_event = {
		.event_id = TCP_DPM_EVT_GET_PPS_STATUS,
	};

	return tcpm_put_tcp_dpm_event_cbk2(tcpc, &tcp_event, cb_data,
		TCPM_BK_PD_CMD_TOUT, (uint8_t *)pps_status, TCPM_PPSSDB_SIZE);
}

#define TCPM_INVALID_RAW_VOLT_MSK 0xFFFF
#define TCPM_INVALID_RAW_CUR_MSK 0xFF
int tcpm_dpm_pd_get_pps_status(struct tcpc_device *tcpc,
		const struct tcp_dpm_event_cb_data *cb_data,
		struct pd_pps_status *pps_status)
{
	int ret;
	struct pd_pps_status_raw pps_status_raw = {0};
	pd_port_t *pd_port = &tcpc->pd_port;

	if (pd_port->dpm_charging_policy != DPM_CHARGING_POLICY_PPS) {
		D("Not during pps charging\n");
		return 0;
	}

	ret = tcpm_dpm_pd_get_pps_status_raw(
		tcpc, cb_data, &pps_status_raw);
	if (ret != 0)
		return ret;

	if (pps_status_raw.output_vol_raw == TCPM_INVALID_RAW_VOLT_MSK)
		pps_status->output_mv = 0;
	else
		pps_status->output_mv =
			TCPM_PPS_GET_OUTPUT_MV(pps_status_raw.output_vol_raw);

	if (pps_status_raw.output_curr_raw == TCPM_INVALID_RAW_CUR_MSK)
		pps_status->output_ma = 0;
	else
		pps_status->output_ma =
			TCPM_PPS_GET_OUTPUT_MA(pps_status_raw.output_curr_raw);

	pps_status->real_time_flags = pps_status_raw.real_time_flags;
	return ret;
}

int tcpm_set_apdo_charging_policy(struct tcpc_device *tcpc, uint8_t policy,
		int mv, int ma, struct tcp_dpm_event_cb_data *cb_data)
{
	pd_port_t *pd_port = &tcpc->pd_port;
	struct tcp_dpm_event tcp_event = {
		.event_id = TCP_DPM_EVT_REQUEST_AGAIN,
	};

	if (!hisi_tcpm_support_apdo(tcpc)) {
		D("Not Support APDO");
		return TCPM_ERROR_NO_APDO;
	}

	if (pd_port->dpm_charging_policy == policy)
		D("Same policy\n");

	if (mv < 0 || ma < 0)
		return TCPM_ERROR_PARAMETER;

	if ((policy & DPM_CHARGING_POLICY_MASK) < DPM_CHARGING_POLICY_PPS)
		return TCPM_ERROR_PARAMETER;

	mutex_lock(&pd_port->pd_lock);
	pd_port->dpm_charging_policy = policy;
	pd_port->request_v_apdo = mv;
	pd_port->request_i_apdo = ma;
	mutex_unlock(&pd_port->pd_lock);

	return tcpm_put_tcp_dpm_event_cbk1(tcpc, &tcp_event,
			cb_data, TCPM_BK_REQUEST_TOUT);
}

int tcpm_inquire_pd_source_apdo(struct tcpc_device *tcpc, uint8_t apdo_type,
		uint8_t *cap_i, struct tcpm_power_cap_val *cap_val)
{
	int ret;
	uint8_t i;
	struct tcpm_power_cap cap = {0};

	ret = hisi_tcpm_get_source_cap(tcpc, &cap);
	if (ret != TCPM_SUCCESS)
		return ret;

	for (i = *cap_i; i < cap.cnt; i++) {
		if (!tcpm_extract_power_cap_val(cap.pdos[i], cap_val))
			continue;

		if (cap_val->type != DPM_PDO_TYPE_APDO)
			continue;

		if ((cap_val->apdo_type & TCPM_APDO_TYPE_MASK) != apdo_type)
			continue;

		*cap_i = i + 1;
		return TCPM_SUCCESS;
	}

	return TCPM_ERROR_NOT_FOUND;
}

int tcpm_set_pd_charging_policy(struct tcpc_device *tcpc, uint8_t policy,
		struct tcp_dpm_event_cb_data *cb_data)
{
	struct tcp_dpm_event tcp_event = {
		.event_id = TCP_DPM_EVT_REQUEST_AGAIN,
	};
	pd_port_t *pd_port = &tcpc->pd_port;

	if (pd_port->dpm_charging_policy == policy)
		return TCPM_SUCCESS;

	/* PPS should call another function ... */
	if ((policy & DPM_CHARGING_POLICY_MASK) >= DPM_CHARGING_POLICY_PPS)
		return TCPM_ERROR_PARAMETER;

	mutex_lock(&pd_port->pd_lock);
	pd_port->dpm_charging_policy = policy;
	mutex_unlock(&pd_port->pd_lock);

	return tcpm_put_tcp_dpm_event_cbk1(
		tcpc, &tcp_event, cb_data, TCPM_BK_REQUEST_TOUT);
}

#else
int tcpm_dpm_pd_get_source_cap_ext(struct tcpc_device *tcpc,
		const struct tcp_dpm_event_cb_data *cb_data,
		struct pd_source_cap_ext *src_cap_ext)
{ return 0; }

int tcpm_dpm_pd_get_status(struct tcpc_device *tcpc,
		struct tcp_dpm_event_cb_data *cb_data, struct pd_status *status)
{ return 0; }

int tcpm_dpm_pd_get_pps_status(struct tcpc_device *tcpc,
		const struct tcp_dpm_event_cb_data *cb_data,
		struct pd_pps_status *pps_status)
{ return 0; }

int tcpm_set_apdo_charging_policy(struct tcpc_device *tcpc, uint8_t policy,
		int mv, int ma, struct tcp_dpm_event_cb_data *cb_data)
{ return 0; }

int tcpm_inquire_pd_source_apdo(struct tcpc_device *tcpc, uint8_t apdo_type,
		uint8_t *cap_i, struct tcpm_power_cap_val *cap_val)
{ return 0; }

int tcpm_set_pd_charging_policy(struct tcpc_device *tcpc, uint8_t policy,
		struct tcp_dpm_event_cb_data *cb_data)
{ return 0; }
#endif

#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

int hisi_tcpm_register_tcpc_dev_notifier(struct tcpc_device *tcp_dev,
		struct notifier_block *nb)
{
	return hisi_register_tcp_dev_notifier(tcp_dev, nb);
}
EXPORT_SYMBOL(hisi_tcpm_register_tcpc_dev_notifier);

int hisi_tcpm_unregister_tcpc_dev_notifier(struct tcpc_device *tcp_dev,
		struct notifier_block *nb)
{
	return hisi_unregister_tcp_dev_notifier(tcp_dev, nb);
}
EXPORT_SYMBOL(hisi_tcpm_unregister_tcpc_dev_notifier);
