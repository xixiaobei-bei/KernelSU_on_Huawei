/*
 * dpm_event.c
 *
 * dpm event
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

#include "dpm_event.h"
#include <linux/slab.h>
#include "inc/timer.h"
#include "inc/tcpc_core.h"
#include "inc/tcpm.h"
#include "inc/pd_core.h"
#include "inc/pd_dpm_core.h"

static inline struct pd_port *dpm_event_to_pd_port(struct dpm_event *devent)
{
	return devent->port;
}

static inline struct tcpc_device *dpm_event_to_tcpc_dev(
	struct dpm_event *devent)
{
	return devent->port->tcpc_dev;
}

static bool dpm_get_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *tcp_event)
{
	int index = devent->tcp_event_head_index;

	if (devent->tcp_event_count == 0)
		return false;

	devent->tcp_event_count--;
	*tcp_event = devent->tcp_event_buf[index];

	if (devent->tcp_event_count > 0) {
		index++;
		index %= DPM_TCP_EVENT_SIZE;
	}
	devent->tcp_event_head_index = index;
	return true;
}

static bool dpm_push_tcp_event(struct dpm_event *devent,
	const struct dpm_tcp_event *tcp_event)
{
	int index;

	if (devent->tcp_event_count >= DPM_TCP_EVENT_SIZE) {
		PD_ERR("tcp event buffer is full\n");
		return false;
	}

	index = devent->tcp_event_head_index + devent->tcp_event_count;
	index %= DPM_TCP_EVENT_SIZE;

	devent->tcp_event_count++;
	devent->tcp_event_buf[index] = *tcp_event;

	tcpci_wakeup_event_task(dpm_event_to_tcpc_dev(devent));
	return true;
}

uint8_t dpm_pop_tcp_event(struct dpm_event *devent)
{
	bool ret = false;
	uint8_t id = TCP_DPM_EVT_UNKONW;

	if (!devent)
		return TCP_DPM_EVT_UNKONW;

	mutex_lock(&devent->tcp_event_lock);
	ret = dpm_get_tcp_event(devent, &devent->tcp_curr_event);
	if (ret) {
		devent->tcp_drop_once = true;
		devent->tcp_event_1st_id = devent->tcp_curr_event.event_id;
		id = devent->tcp_curr_event.event_id;
	}
	mutex_unlock(&devent->tcp_event_lock);

#ifdef CONFIG_HW_USB_PD_REV30
#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	if (devent->tcp_event_count)
		timer_restart(dpm_event_to_tcpc_dev(devent),
			PD_TIMER_DEFERRED_EVT);
	else
		timer_disable(dpm_event_to_tcpc_dev(devent),
			PD_TIMER_DEFERRED_EVT);
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */
#endif /* CONFIG_HW_USB_PD_REV30 */

	return id;
}

static bool dpm_tcp_event_ready(struct dpm_event *devent)
{
	struct tcpc_device *tcpc = dpm_event_to_tcpc_dev(devent);

	if (!tcpc->pd_pe_running || tcpc_is_wait_pe_idle(tcpc)) {
		PD_ERR("%s: pe is idle\n", __func__);
		return false;
	}

	if (tcpc->pd_wait_hard_reset_complete) {
		PD_ERR("%s: hard reset not complete\n", __func__);
		return false;
	}

	return true;
}

bool dpm_put_tcp_event(struct dpm_event *devent,
	const struct dpm_tcp_event *tcp_event)
{
	bool ret = false;
	struct pd_port *port = NULL;

	if (!devent || !tcp_event)
		return false;

	if (!dpm_tcp_event_ready(devent))
		return false;

#ifdef CONFIG_HW_USB_PD_REV30
#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	if (devent->tcp_event_count == 0)
		timer_enable(dpm_event_to_tcpc_dev(devent),
			PD_TIMER_DEFERRED_EVT);
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */
#endif /* CONFIG_HW_USB_PD_REV30 */

	port = dpm_event_to_pd_port(devent);
	switch (tcp_event->event_id) {
	case TCP_DPM_EVT_DISCOVER_CABLE:
	case TCP_DPM_EVT_CABLE_SOFTRESET:
		dpm_reaction_set(port,
			DPM_REACTION_DYNAMIC_VCONN |
			DPM_REACTION_VCONN_STABLE_DELAY);
		break;
	default:
		break;
	}

	mutex_lock(&devent->tcp_event_lock);
	ret = dpm_push_tcp_event(devent, tcp_event);
	mutex_unlock(&devent->tcp_event_lock);

#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	if (ret)
		port->pe_data.pd_traffic_idle = false;
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */

	dpm_reaction_set_ready_once(port);
	return ret;
}

struct dpm_tcp_event *dpm_get_curr_tcp_event(struct dpm_event *devent)
{
	if (!devent)
		return NULL;

	return &devent->tcp_curr_event;
}

static bool dpm_filter_repeat_result_2nd(struct dpm_event *devent, int ret)
{
	switch (ret) {
	case TCP_DPM_RET_DROP_SENT_SRESET:
		if ((devent->tcp_event_2nd_id == TCP_DPM_EVT_SOFTRESET) &&
			devent->tcp_drop_once) {
			devent->tcp_drop_once = false;
			return true;
		}
		break;
	case TCP_DPM_RET_DROP_SENT_HRESET:
		if ((devent->tcp_event_2nd_id == TCP_DPM_EVT_HARD_RESET) &&
			devent->tcp_drop_once) {
			devent->tcp_drop_once = false;
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

void dpm_reply_tcp_event_2nd(struct dpm_event *devent, int ret)
{
	struct dpm_tcp_event *tcp_event = NULL;

	if (!devent)
		return;

	if ((devent->tcp_event_2nd_id  == TCP_DPM_EVT_UNKONW) ||
		dpm_filter_repeat_result_2nd(devent, ret))
		return;

	DPM_DBG("%s: event id=%d,ret=%d\n", __func__,
		devent->tcp_event_2nd_id, ret);
	mutex_lock(&devent->tcp_event_lock);
	tcp_event = &devent->tcp_curr_event;
	tcp_event->result = ret;
	if (tcp_event->event_cb)
		tcp_event->event_cb(dpm_event_to_tcpc_dev(devent), tcp_event);

	devent->tcp_event_2nd_id = TCP_DPM_EVT_UNKONW;
	mutex_unlock(&devent->tcp_event_lock);
}

void dpm_reply_tcp_event_1st(struct dpm_event *devent, int ret)
{
	bool cb = true;
	struct dpm_tcp_event *tcp_event = NULL;

	if (!devent)
		return;

	if (devent->tcp_event_1st_id == TCP_DPM_EVT_UNKONW)
		return;

	DPM_DBG("%s: event id=%d,ret=%d\n", __func__,
		devent->tcp_event_1st_id, ret);
	mutex_lock(&devent->tcp_event_lock);
	tcp_event = &devent->tcp_curr_event;
	if (ret == TCP_DPM_RET_SENT) {
		cb = false;
		devent->tcp_event_2nd_id = tcp_event->event_id;
	}

	if (cb && tcp_event->event_cb) {
		tcp_event->result = ret;
		tcp_event->event_cb(dpm_event_to_tcpc_dev(devent), tcp_event);
	}

	devent->tcp_event_1st_id = TCP_DPM_EVT_UNKONW;
	mutex_unlock(&devent->tcp_event_lock);
}

static void dpm_clear_tcp_event_buf(struct dpm_event *devent,
	uint8_t reason)
{
	struct dpm_tcp_event tcp_event;

	mutex_lock(&devent->tcp_event_lock);
	while (dpm_get_tcp_event(devent, &tcp_event)) {
		if (tcp_event.event_cb) {
			tcp_event.result = reason;
			tcp_event.event_cb(dpm_event_to_tcpc_dev(devent),
				&tcp_event);
		}
	}
	mutex_unlock(&devent->tcp_event_lock);
}

void dpm_clear_tcp_event(struct dpm_event *devent, uint8_t reason)
{
	dpm_reply_tcp_event_1st(devent, reason);
	dpm_reply_tcp_event_2nd(devent, reason);

	dpm_clear_tcp_event_buf(devent, reason);
}

bool dpm_tcp_event_empty(struct dpm_event *devent)
{
	if (!devent)
		return NULL;

	return devent->tcp_event_count == 0;
}

static void dpm_replace_curr_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *event)
{
	int reason = TCP_DPM_RET_DENIED_UNKNOWN;

	switch (event->event_id) {
	case TCP_DPM_EVT_HARD_RESET:
		reason = TCP_DPM_RET_DROP_SENT_HRESET;
		break;
	case TCP_DPM_EVT_ERROR_RECOVERY:
		reason = TCP_DPM_RET_DROP_ERROR_REOCVERY;
		break;
	default:
		break;
	}

	dpm_reply_tcp_event_2nd(devent, reason);

	mutex_lock(&devent->tcp_event_lock);
	devent->tcp_drop_once = true;
	memcpy(&devent->tcp_curr_event, event, sizeof(struct dpm_tcp_event));
	mutex_unlock(&devent->tcp_event_lock);
}

int dpm_put_event(struct dpm_event *devent, struct dpm_tcp_event *event)
{
	int ret;
	bool imme = event->event_id >= TCP_DPM_EVT_IMMEDIATELY;
	struct pd_port *port = NULL;

	if (!devent)
		return TCPM_ERROR_UNKNOWN;

	port = devent->port;
	ret = pd_check_attached(port);
	DPM_DBG("%s: event id=%d,imme=%d,pd state=%d\n", __func__,
		event->event_id, imme, ret);
	if ((!imme && (ret != TCPM_SUCCESS)) ||
		(imme && (ret == TCPM_ERROR_UNATTACHED)))
		return ret;

	if (imme) {
		ret = pd_put_tcp_pd_event(port, event->event_id,
			PD_TCP_FROM_TCPM);
		if (ret)
			dpm_replace_curr_tcp_event(devent, event);
	} else {
		ret = dpm_put_tcp_event(devent, event);
	}

	if (!ret)
		return TCPM_ERROR_PUT_EVENT;

	return TCPM_SUCCESS;
}

static const char * const bk_event_ret_name[] = {
	"OK",
	"Unknown",
	"NotReady",
	"LocalCap",
	"PartnerCap",
	"SameRole",
	"InvalidReq",
	"RepeatReq",
	"WrongDR",
	"PDRev",

	"Detach",
	"SReset0",
	"SReset1",
	"HReset0",
	"HReset1",
	"Recovery",
	"BIST",
	"PEBusy",
	"Discard",
	"Unexpected",

	"Wait",
	"Reject",
	"TOUT",
	"NAK",
	"NotSupport",

	"BKTOUT",
	"NoResponse",
};

static void dpm_copy_bk_data(struct dpm_event *devent,
	struct dpm_tcp_event *event)
{
	struct pd_port *port = devent->port;
	uint8_t *payload = NULL;
	int size;

	if ((devent->tcp_bk_size == 0) || !devent->tcp_bk_data)
		return;

	payload = pd_get_msg_data_payload(port);
	if (!payload) {
		PD_ERR("%s: payload is null\n", __func__);
		return;
	}

	size = devent->tcp_bk_size;
	if (size >= pd_get_msg_data_size(port))
		size = pd_get_msg_data_size(port);

	memcpy(devent->tcp_bk_data, payload, size);
}

static void dpm_sync_tcp_event_cb(struct tcpc_device *tcpc,
	struct dpm_tcp_event *event)
{
	struct dpm_event *devent = NULL;

	if (!tcpc || !event)
		return;

	devent = tcpc->pd_port.devent;
	if (devent->tcp_wait_id != event->event_id) {
		DPM_DBG("%s: expect=%d,real=%d\n", __func__,
			devent->tcp_wait_id, event->event_id);
		return;
	}

	devent->tcp_wait_done = true;
	devent->tcp_bk_ret = event->result;

	if (event->result == TCP_DPM_RET_SUCCESS)
		dpm_copy_bk_data(devent, event);

	wake_up_interruptible(&devent->tcp_wait_queue);
}

static int dpm_wait_sync_tcp_event(struct dpm_event *devent, uint32_t tout_ms)
{
	int ret = TCP_DPM_RET_BK_TIMEOUT;

	wait_event_interruptible_timeout(devent->tcp_wait_queue,
		devent->tcp_wait_done, msecs_to_jiffies(tout_ms));
	if (devent->tcp_wait_done)
		ret = devent->tcp_bk_ret;

	devent->tcp_wait_id = TCP_DPM_EVT_UNKONW;
	devent->tcp_bk_data = NULL;

	if (ret < TCP_DPM_RET_NR)
		DPM_DBG("bk_event_cb->%s\n", bk_event_ret_name[ret]);
	return ret;
}

static int dpm_put_sync_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *event,
	uint32_t tout_ms,
	uint8_t *data,
	uint8_t size)
{
	int ret;

	event->event_cb = dpm_sync_tcp_event_cb;
	devent->tcp_wait_done = false;
	devent->tcp_wait_id = event->event_id;
	devent->tcp_bk_data = data;
	devent->tcp_bk_size = size;

	ret = dpm_put_event(devent, event);
	if (ret != TCPM_SUCCESS)
		return ret;

	return dpm_wait_sync_tcp_event(devent, tout_ms);
}

int dpm_send_sync_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *event,
	uint32_t tout_ms,
	uint8_t *data,
	uint8_t size)
{
	int ret;
	int retry = DPM_TCP_EVENT_SEND_RETRY;

	if (!devent || !event)
		return TCPM_ERROR_UNKNOWN;

	mutex_lock(&devent->tcp_sync_lock);
	while (retry-- >= 0) {
		ret = dpm_put_sync_tcp_event(devent, event,
			tout_ms, data, size);
		if (ret != TCP_DPM_RET_TIMEOUT &&
		    ret != TCP_DPM_RET_DROP_DISCARD &&
		    ret != TCP_DPM_RET_DROP_UNEXPECTED)
			break;
	}
	mutex_unlock(&devent->tcp_sync_lock);

	DPM_DBG("%s finish: event id=%d,ret=%d\n", __func__,
		event->event_id, ret);
	if (data && (ret == TCPM_SUCCESS))
		return TCPM_ERROR_EXPECT_CB2;

	if (ret == TCP_DPM_RET_DENIED_REPEAT_REQUEST)
		ret = TCPM_SUCCESS;

	return ret;
}

int dpm_send_async_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *event,
	const struct dpm_tcp_event_cb_data *cb_data)
{
	if (!devent || !event || !cb_data)
		return TCPM_ERROR_UNKNOWN;

	event->event_cb = cb_data->event_cb;
	return dpm_put_event(devent, event);
}

int dpm_event_init(struct pd_port *port)
{
	struct dpm_event *devent = NULL;

	if (!port)
		return -EINVAL;

	devent = devm_kzalloc(&port->tcpc_dev->dev, sizeof(*devent),
		GFP_KERNEL);
	if (!devent)
		return -ENOMEM;

	devent->port = port;
	mutex_init(&devent->tcp_event_lock);
	mutex_init(&devent->tcp_sync_lock);
	init_waitqueue_head(&devent->tcp_wait_queue);
	port->devent = devent;
	return 0;
}
