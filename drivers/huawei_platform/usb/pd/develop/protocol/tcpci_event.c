/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * TCPC Interface for event handler
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

#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/version.h>

#include <linux/sched/rt.h>
#include <uapi/linux/sched/types.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#include "inc/tcpci_event.h"
#include "inc/tcpci.h"
#include "inc/pd_dpm_core.h"
#include "inc/tcpc_core.h"

#ifdef CONFIG_USB_PD_POSTPONE_VDM
static void postpone_vdm_event(struct tcpc_device *tcpc_dev)
{
	/*
	 * Postpone VDM retry event due to the retry reason
	 * maybe interrupt by some PD event ....
	 */

	struct pd_event *vdm_event = &tcpc_dev->pd_vdm_event;

	if (tcpc_dev->pd_pending_vdm_event && vdm_event->pd_msg) {
		tcpc_dev->pd_postpone_vdm_timeout = false;
		timer_restart(tcpc_dev, PD_PE_VDM_POSTPONE);
	}
}
#endif	/* CONFIG_USB_PD_POSTPONE_VDM */

struct pd_msg *__pd_alloc_msg(struct tcpc_device *tcpc_dev)
{
	int i;
	uint8_t mask;

	for (i = 0, mask = 1; i < PD_MSG_BUF_SIZE; i++, mask <<= 1) {
		if ((mask & tcpc_dev->pd_msg_buffer_allocated) == 0) {
			tcpc_dev->pd_msg_buffer_allocated |= mask;
			return tcpc_dev->pd_msg_buffer + i;
		}
	}

	PD_ERR("pd_alloc_msg failed\r\n");
	PD_BUG_ON(true);

	return (struct pd_msg *)NULL;
}

struct pd_msg *pd_alloc_msg(struct tcpc_device *tcpc_dev)
{
	struct pd_msg *pd_msg = NULL;

	mutex_lock(&tcpc_dev->access_lock);
	pd_msg = __pd_alloc_msg(tcpc_dev);
	mutex_unlock(&tcpc_dev->access_lock);

	return pd_msg;
}

static void __pd_free_msg(struct tcpc_device *tcpc_dev, struct pd_msg *pd_msg)
{
	int index = pd_msg - tcpc_dev->pd_msg_buffer;
	uint8_t mask = 1 << index;

	PD_BUG_ON((mask & tcpc_dev->pd_msg_buffer_allocated) == 0);
	tcpc_dev->pd_msg_buffer_allocated &= (~mask);
}

static void __pd_free_event(
		struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	if (pd_event->pd_msg) {
		__pd_free_msg(tcpc_dev, pd_event->pd_msg);
		pd_event->pd_msg = NULL;
	}
}

bool __pd_is_msg_empty(struct tcpc_device *tcpc_dev)
{
	int i;
	uint8_t mask;

	for (i = 0, mask = 1; i < PD_MSG_BUF_SIZE; i++, mask <<= 1) {
		if ((mask & tcpc_dev->pd_msg_buffer_allocated) != 0)
			return false;
	}
	return true;
}

bool pd_is_msg_empty(struct tcpc_device *tcpc_dev)
{
	bool empty = false;

	mutex_lock(&tcpc_dev->access_lock);
	empty = __pd_is_msg_empty(tcpc_dev);
	mutex_unlock(&tcpc_dev->access_lock);
	return empty;
}

void pd_free_msg(struct tcpc_device *tcpc_dev, struct pd_msg *pd_msg)
{
	mutex_lock(&tcpc_dev->access_lock);
	__pd_free_msg(tcpc_dev, pd_msg);
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_free_event(struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	mutex_lock(&tcpc_dev->access_lock);
	__pd_free_event(tcpc_dev, pd_event);
	mutex_unlock(&tcpc_dev->access_lock);
}

/*----------------------------------------------------------------------------*/

static bool __pd_get_event(
	struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	int index = 0;

	if (tcpc_dev->pd_event_count <= 0)
		return false;

	tcpc_dev->pd_event_count--;

	*pd_event =
		tcpc_dev->pd_event_ring_buffer[tcpc_dev->pd_event_head_index];

	if (tcpc_dev->pd_event_count) {
		index = tcpc_dev->pd_event_head_index + 1;
		index %= PD_EVENT_BUF_SIZE;
	}
	tcpc_dev->pd_event_head_index = index;
	return true;
}

bool pd_get_event(struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	bool ret = false;

	mutex_lock(&tcpc_dev->access_lock);
	ret = __pd_get_event(tcpc_dev, pd_event);
	mutex_unlock(&tcpc_dev->access_lock);
	return ret;
}

static bool __pd_put_event(struct tcpc_device *tcpc_dev,
	const struct pd_event *pd_event, bool from_port_partner)
{
	int index;

#ifdef CONFIG_USB_PD_POSTPONE_OTHER_VDM
	if (from_port_partner)
		postpone_vdm_event(tcpc_dev);
#endif	/* CONFIG_USB_PD_POSTPONE_OTHER_VDM */

	if (tcpc_dev->pd_event_count >= PD_EVENT_BUF_SIZE) {
		PD_ERR("pd_put_event failed\r\n");
		return false;
	}

	index = (tcpc_dev->pd_event_head_index + tcpc_dev->pd_event_count);
	index %= PD_EVENT_BUF_SIZE;

	tcpc_dev->pd_event_count++;
	tcpc_dev->pd_event_ring_buffer[index] = *pd_event;

	tcpci_wakeup_event_task(tcpc_dev);
	return true;
}

bool pd_put_event(struct tcpc_device *tcpc_dev, const struct pd_event *pd_event,
	bool from_port_partner)
{
	bool ret = false;

	mutex_lock(&tcpc_dev->access_lock);
	ret = __pd_put_event(tcpc_dev, pd_event, from_port_partner);
	mutex_unlock(&tcpc_dev->access_lock);

	return ret;
}

/*----------------------------------------------------------------------------*/

static inline void pd_get_attention_event(
	struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	struct pd_event attention_evt = {
		.event_type = PD_EVT_PD_MSG,
		.msg = PD_DATA_VENDOR_DEF,
		.pd_msg = NULL,
	};

	*pd_event = attention_evt;
	pd_event->pd_msg = __pd_alloc_msg(tcpc_dev);

	if (pd_event->pd_msg == NULL)
		return;

	tcpc_dev->pd_pending_vdm_attention = false;
	*pd_event->pd_msg = tcpc_dev->pd_attention_vdm_msg;
}

bool pd_get_vdm_event(struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	struct pd_event delay_evt = {
		.event_type = PD_EVT_CTRL_MSG,
		.msg = PD_CTRL_GOOD_CRC,
		.pd_msg = NULL,
	};

	struct pd_event reset_evt = {
		.event_type = PD_EVT_PE_MSG,
		.msg = PD_PE_VDM_RESET,
		.pd_msg = NULL,
	};

	struct pd_event discard_evt = {
		.event_type = PD_EVT_HW_MSG,
		.msg = PD_HW_TX_DISCARD,
		.pd_msg = NULL,
	};

	struct pd_event *vdm_event = &tcpc_dev->pd_vdm_event;

	if (tcpc_dev->pd_pending_vdm_discard) {
		mutex_lock(&tcpc_dev->access_lock);
		*pd_event = discard_evt;
		tcpc_dev->pd_pending_vdm_discard = false;
		mutex_unlock(&tcpc_dev->access_lock);
		return true;
	}

	if (tcpc_dev->pd_pending_vdm_event) {
		if (vdm_event->pd_msg && !tcpc_dev->pd_postpone_vdm_timeout)
			return false;

		mutex_lock(&tcpc_dev->access_lock);
		if (tcpc_dev->pd_pending_vdm_good_crc) {
			*pd_event = delay_evt;
			tcpc_dev->pd_pending_vdm_good_crc = false;
		} else if (tcpc_dev->pd_pending_vdm_reset) {
			*pd_event = reset_evt;
			tcpc_dev->pd_pending_vdm_reset = false;
		} else {
			*pd_event = *vdm_event;
			tcpc_dev->pd_pending_vdm_event = false;
		}

		mutex_unlock(&tcpc_dev->access_lock);
		return true;
	}

	if (tcpc_dev->pd_pending_vdm_attention
		&& pe_vdm_state_ready(&tcpc_dev->pd_port)) {
		mutex_lock(&tcpc_dev->access_lock);
		pd_get_attention_event(tcpc_dev, pd_event);
		mutex_unlock(&tcpc_dev->access_lock);
		return true;
	}

	return false;
}

static inline bool reset_pe_vdm_state(
		struct tcpc_device *tcpc_dev, uint32_t vdm_hdr)
{
	bool vdm_reset = false;
	struct pd_port *pd_port = &tcpc_dev->pd_port;

	if (PD_VDO_SVDM(vdm_hdr)) {
		if (PD_VDO_CMDT(vdm_hdr) == CMDT_INIT)
			vdm_reset = true;
	} else {
		if (pd_port->data_role == PD_ROLE_UFP)
			vdm_reset = true;
	}

	if (vdm_reset)
		tcpc_dev->pd_pending_vdm_reset = true;

	return vdm_reset;
}

static inline bool pd_is_init_attention_event(
	struct tcpc_device *tcpc_dev, struct pd_event *pd_event)
{
	uint32_t vdm_hdr = pd_event->pd_msg->payload[0];

	if ((PD_VDO_CMDT(vdm_hdr) == CMDT_INIT) &&
			PD_VDO_CMD(vdm_hdr) == CMD_ATTENTION) {
		return true;
	}

	return false;
}

bool pd_put_vdm_event(struct tcpc_device *tcpc_dev,
		struct pd_event *pd_event, bool from_port_partner)
{
	bool ignore_evt = false;
	struct pd_msg *pd_msg = pd_event->pd_msg;

	mutex_lock(&tcpc_dev->access_lock);

	if (from_port_partner &&
		pd_is_init_attention_event(tcpc_dev, pd_event)) {
		TCPC_DBG("AttEvt\r\n");
		ignore_evt = true;
		tcpc_dev->pd_pending_vdm_attention = true;
		tcpc_dev->pd_attention_vdm_msg = *pd_msg;

		/* do not really wake up process*/
		tcpci_wakeup_event_task(tcpc_dev);
	}

	if (tcpc_dev->pd_pending_vdm_event && (!ignore_evt)) {
		/* If message from port partner, we have to overwrite it */
		/* If message from TCPM, we will reset_vdm later */
		ignore_evt = !from_port_partner;

		if (from_port_partner) {
			if (pd_event_ctrl_msg_match(
					&tcpc_dev->pd_vdm_event,
					PD_CTRL_GOOD_CRC)) {
				TCPC_DBG2("PostponeVDM GoodCRC\r\n");
				tcpc_dev->pd_pending_vdm_good_crc = true;
			}

			__pd_free_event(tcpc_dev, &tcpc_dev->pd_vdm_event);
		}
	}

	if (ignore_evt) {
		__pd_free_event(tcpc_dev, pd_event);
		mutex_unlock(&tcpc_dev->access_lock);
		return false;
	}

	tcpc_dev->pd_vdm_event = *pd_event;
	tcpc_dev->pd_pending_vdm_event = true;
	tcpc_dev->pd_postpone_vdm_timeout = true;

	if (from_port_partner) {

		PD_BUG_ON(pd_msg == NULL);
		/* pd_msg->time_stamp = 0; */
		tcpc_dev->pd_last_vdm_msg = *pd_msg;
		reset_pe_vdm_state(tcpc_dev, pd_msg->payload[0]);

#ifdef CONFIG_USB_PD_POSTPONE_FIRST_VDM
		postpone_vdm_event(tcpc_dev);
		mutex_unlock(&tcpc_dev->access_lock);
		return true;
#endif	/* CONFIG_USB_PD_POSTPONE_FIRST_VDM */
	}

	tcpci_wakeup_event_task(tcpc_dev);
	mutex_unlock(&tcpc_dev->access_lock);

	return true;
}

bool pd_put_last_vdm_event(struct tcpc_device *tcpc_dev)
{
	struct pd_msg *pd_msg = &tcpc_dev->pd_last_vdm_msg;
	struct pd_event *vdm_event = &tcpc_dev->pd_vdm_event;

	mutex_lock(&tcpc_dev->access_lock);

	tcpc_dev->pd_pending_vdm_discard = true;
	tcpci_wakeup_event_task(tcpc_dev);

	/* If the last VDM event isn't INIT event, don't put it again */
	if (!reset_pe_vdm_state(tcpc_dev, pd_msg->payload[0])) {
		mutex_unlock(&tcpc_dev->access_lock);
		return true;
	}

	vdm_event->event_type = PD_EVT_HW_MSG;
	vdm_event->msg = PD_HW_RETRY_VDM;

	if (tcpc_dev->pd_pending_vdm_event)
		__pd_free_event(tcpc_dev, &tcpc_dev->pd_vdm_event);

	vdm_event->pd_msg = __pd_alloc_msg(tcpc_dev);

	if (vdm_event->pd_msg == NULL) {
		mutex_unlock(&tcpc_dev->access_lock);
		return false;
	}

	*vdm_event->pd_msg = *pd_msg;
	tcpc_dev->pd_pending_vdm_event = true;
	tcpc_dev->pd_postpone_vdm_timeout = true;

#ifdef CONFIG_USB_PD_POSTPONE_RETRY_VDM
	postpone_vdm_event(tcpc_dev);
#endif	/* CONFIG_USB_PD_POSTPONE_RETRY_VDM */

	mutex_unlock(&tcpc_dev->access_lock);
	return true;
}

/*----------------------------------------------------------------------------*/

static void __pd_event_buf_reset(struct tcpc_device *tcpc_dev, uint8_t reason)
{
	struct pd_event pd_event;

	tcpc_dev->pd_hard_reset_event_pending = false;
	while (__pd_get_event(tcpc_dev, &pd_event))
		__pd_free_event(tcpc_dev, &pd_event);

	if (tcpc_dev->pd_pending_vdm_event) {
		__pd_free_event(tcpc_dev, &tcpc_dev->pd_vdm_event);
		tcpc_dev->pd_pending_vdm_event = false;
	}

	tcpc_dev->pd_pending_vdm_reset = false;
	tcpc_dev->pd_pending_vdm_good_crc = false;
	tcpc_dev->pd_pending_vdm_attention = false;
	tcpc_dev->pd_pending_vdm_discard = false;

	dpm_clear_tcp_event(tcpc_dev->pd_port.devent, reason);
	/* PD_BUG_ON(tcpc_dev->pd_msg_buffer_allocated != 0); */
}

void pd_event_buf_reset(struct tcpc_device *tcpc_dev)
{
	mutex_lock(&tcpc_dev->access_lock);
	__pd_event_buf_reset(tcpc_dev, TCP_DPM_RET_DROP_CC_DETACH);
	mutex_unlock(&tcpc_dev->access_lock);
}

/*----------------------------------------------------------------------------*/

static inline bool __pd_put_hw_event(
	struct tcpc_device *tcpc_dev, uint8_t hw_event)
{
	struct pd_event evt = {
		.event_type = PD_EVT_HW_MSG,
		.msg = hw_event,
		.pd_msg = NULL,
	};

	return __pd_put_event(tcpc_dev, &evt, false);
}

static inline bool __pd_put_pe_event(
	struct tcpc_device *tcpc_dev, uint8_t pe_event)
{
	struct pd_event evt = {
		.event_type = PD_EVT_PE_MSG,
		.msg = pe_event,
		.pd_msg = NULL,
	};

	return __pd_put_event(tcpc_dev, &evt, false);
}

bool pd_put_cc_attach_event(
		struct tcpc_device *tcpc_dev, uint8_t type)
{
	struct pd_event evt = {
		.event_type = PD_EVT_HW_MSG,
		.msg = PD_HW_CC_ATTACHED,
		.msg_sec = type,
		.pd_msg = NULL,
	};

	switch (type) {
	case TYPEC_ATTACHED_SNK:
	case TYPEC_ATTACHED_SRC:
		tcpc_dev->pd_pe_running = true;
		break;
	default:
		break;
	}

	return pd_put_event(tcpc_dev, &evt, false);
}

static int tcpci_notify_hard_reset_state(struct tcpc_device *tcpc,
	uint8_t state)
{
	if (state >= TCP_HRESET_SIGNAL_SEND)
		tcpc->pd_wait_hard_reset_complete = true;
	else if (tcpc->pd_wait_hard_reset_complete)
		tcpc->pd_wait_hard_reset_complete = false;
	else
		return 0;

	tcpm_notify_hard_reset_state(&tcpc->notifier, state);
	return 0;
}

void pd_put_cc_detach_event(struct tcpc_device *tcpc_dev)
{
	mutex_lock(&tcpc_dev->access_lock);

	tcpci_notify_hard_reset_state(
		tcpc_dev, TCP_HRESET_RESULT_FAIL);

	__pd_event_buf_reset(tcpc_dev, TCP_DPM_RET_DROP_CC_DETACH);
	__pd_put_hw_event(tcpc_dev, PD_HW_CC_DETACHED);

	tcpc_dev->pd_pe_running = false;
	tcpc_dev->pd_wait_pr_swap_complete = false;
	tcpc_dev->pd_hard_reset_event_pending = false;
	tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
	tcpc_dev->pd_bist_mode = PD_BIST_MODE_DISABLE;
	tcpc_dev->pd_ping_event_pending = false;

#ifdef CONFIG_USB_PD_DIRECT_CHARGE
	tcpc_dev->pd_during_direct_charge = false;
	tcpc_dev->is_huawei_scp_chg = false;
#endif	/* CONFIG_USB_PD_DIRECT_CHARGE */

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	tcpc_dev->pd_discard_pending = false;
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */

	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_put_recv_hard_reset_event(struct tcpc_device *tcpc_dev)
{
	mutex_lock(&tcpc_dev->access_lock);

	tcpci_notify_hard_reset_state(
		tcpc_dev, TCP_HRESET_SIGNAL_RECV);

	tcpc_dev->pd_transmit_state = PD_TX_STATE_HARD_RESET;

	if ((!tcpc_dev->pd_hard_reset_event_pending) &&
		(!tcpc_is_wait_pe_idle(tcpc_dev)) &&
		tcpc_dev->pd_pe_running) {
		__pd_event_buf_reset(tcpc_dev, TCP_DPM_RET_DROP_RECV_HRESET);
		__pd_put_hw_event(tcpc_dev, PD_HW_RECV_HARD_RESET);
		tcpc_dev->pd_bist_mode = PD_BIST_MODE_DISABLE;
		tcpc_dev->pd_hard_reset_event_pending = true;
		tcpc_dev->pd_ping_event_pending = false;

#ifdef CONFIG_USB_PD_DIRECT_CHARGE
		tcpc_dev->pd_during_direct_charge = false;
#endif	/* CONFIG_USB_PD_DIRECT_CHARGE */
	}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	tcpc_dev->pd_discard_pending = false;
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */

	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_put_sent_hard_reset_event(struct tcpc_device *tcpc_dev)
{
	mutex_lock(&tcpc_dev->access_lock);

	if (tcpc_dev->pd_wait_hard_reset_complete)
		__pd_event_buf_reset(tcpc_dev, TCP_DPM_RET_DROP_SENT_HRESET);
	else
		TCPC_DBG2("[HReset] Unattached\r\n");

	tcpc_dev->pd_transmit_state = PD_TX_STATE_GOOD_CRC;
	__pd_put_pe_event(tcpc_dev, PD_PE_HARD_RESET_COMPLETED);

	mutex_unlock(&tcpc_dev->access_lock);
}

bool pd_put_pd_msg_event(struct tcpc_device *tcpc_dev, struct pd_msg *pd_msg)
{
	uint32_t cnt, cmd, extend;

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	bool discard_pending = false;
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */

	struct pd_event evt = {
		.event_type = PD_EVT_PD_MSG,
		.pd_msg = pd_msg,
	};

	cnt = PD_HEADER_CNT(pd_msg->msg_hdr);
	cmd = PD_HEADER_TYPE(pd_msg->msg_hdr);
	extend = PD_HEADER_EXT(pd_msg->msg_hdr);

	/* bist mode */
	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_bist_mode != PD_BIST_MODE_DISABLE) {
		TCPC_DBG2("BIST_MODE_RX\r\n");
		__pd_free_event(tcpc_dev, &evt);
		mutex_unlock(&tcpc_dev->access_lock);
		return 0;
	}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	if (tcpc_dev->pd_discard_pending &&
		(pd_msg->frame_type == TCPC_TX_SOP) &&
		tcpci_is_support(tcpc_dev, TCPC_FLAGS_RETRY_CRC_DISCARD)) {

		discard_pending = true;
		tcpc_dev->pd_discard_pending = false;

		if ((cmd == PD_CTRL_GOOD_CRC) && (cnt == 0)) {
			TCPC_DBG2("RETRANSMIT\r\n");
			__pd_free_event(tcpc_dev, &evt);
			mutex_unlock(&tcpc_dev->access_lock);

			/* TODO: check it later */
			timer_disable(tcpc_dev, PD_TIMER_DISCARD);
			tcpci_retransmit(tcpc_dev);
			return 0;
		}
	}
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */

#ifdef CONFIG_USB_PD_DROP_REPEAT_PING
	if (cnt == 0 && cmd == PD_CTRL_PING) {
		/* reset ping_test_mode only if cc_detached */
		if (!tcpc_dev->pd_ping_event_pending) {
			TCPC_INFO("ping_test_mode\r\n");
			tcpc_dev->pd_ping_event_pending = true;
			tcpci_set_bist_test_mode(tcpc_dev, true);
		} else {
			__pd_free_event(tcpc_dev, &evt);
			mutex_unlock(&tcpc_dev->access_lock);
			return 0;
		}
	}
#endif	/* CONFIG_USB_PD_DROP_REPEAT_PING */

	if (cnt != 0 && cmd == PD_DATA_BIST && extend == 0)
		tcpc_dev->pd_bist_mode = PD_BIST_MODE_EVENT_PENDING;

	mutex_unlock(&tcpc_dev->access_lock);

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	if (discard_pending) {
		timer_disable(tcpc_dev, PD_TIMER_DISCARD);
		pd_put_hw_event(tcpc_dev, PD_HW_TX_DISCARD);
	}
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */

	if (cnt != 0 && cmd == PD_DATA_VENDOR_DEF)
		return pd_put_vdm_event(tcpc_dev, &evt, true);

	if (!pd_put_event(tcpc_dev, &evt, true)) {
		pd_free_event(tcpc_dev, &evt);
		return false;
	}

	return true;
}

static void pd_report_vbus_event(struct tcpc_device *tcpc_dev, int event)
{
	tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
	__pd_put_hw_event(tcpc_dev, event);
}

void pd_put_vbus_changed_event(struct tcpc_device *tcpc_dev, bool from_ic)
{
	int vbus_valid;
	bool postpone_vbus_present = false;

	if (!tcpc_dev)
		return;

	mutex_lock(&tcpc_dev->access_lock);
	vbus_valid = typec_check_vbus_valid(tcpc_dev);

	if (tcpc_dev->pd_wait_vbus_once != PD_WAIT_VBUS_DISABLE)
		PE_DBG("%s: wait=%d, vbus_valid=%d, from_ic=%d\n", __func__,
			tcpc_dev->pd_wait_vbus_once, vbus_valid, from_ic);

	switch (tcpc_dev->pd_wait_vbus_once) {
	case PD_WAIT_VBUS_VALID_ONCE:
		if (vbus_valid) {
			postpone_vbus_present = from_ic;
			if (!postpone_vbus_present)
				pd_report_vbus_event(tcpc_dev, PD_HW_VBUS_PRESENT);
			else
				timer_enable(tcpc_dev, PD_TIMER_VBUS_PRESENT);
		}
		break;
	case PD_WAIT_VBUS_INVALID_ONCE:
		if (!vbus_valid)
			pd_report_vbus_event(tcpc_dev, PD_HW_VBUS_ABSENT);
		break;
	case PD_WAIT_VBUS_SAFE0V_ONCE:
		if (typec_check_vsafe0v(tcpc_dev)) {
			pd_report_vbus_event(tcpc_dev, PD_HW_VBUS_SAFE0V);
			mutex_unlock(&tcpc_dev->access_lock);
			tcpci_enable_ext_discharge(tcpc_dev, false);
			return;
		}
		break;
	}
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_put_vbus_safe0v_event(struct tcpc_device *tcpc_dev)
{
	if (!tcpc_dev)
		return;

	if (tcpc_dev->pd_wait_vbus_once == PD_WAIT_VBUS_SAFE0V_ONCE)
		tcpci_enable_ext_discharge(tcpc_dev, false);

	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_vbus_once == PD_WAIT_VBUS_SAFE0V_ONCE)
		pd_report_vbus_event(tcpc_dev, PD_HW_VBUS_SAFE0V);
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_put_vbus_stable_event(struct tcpc_device *tcpc_dev)
{
	if (!tcpc_dev)
		return;

	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_vbus_once == PD_WAIT_VBUS_STABLE_ONCE)
		pd_report_vbus_event(tcpc_dev, PD_HW_VBUS_STABLE);
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_put_vbus_present_event(struct tcpc_device *tcpc_dev)
{
	if (!tcpc_dev)
		return;

	mutex_lock(&tcpc_dev->access_lock);
	pd_report_vbus_event(tcpc_dev, PD_HW_VBUS_PRESENT);
	mutex_unlock(&tcpc_dev->access_lock);
}

/* ---- PD Notify TCPC ---- */

void pd_try_put_pe_idle_event(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_transmit_state < PD_TX_STATE_WAIT)
		__pd_put_pe_event(tcpc_dev, PD_PE_IDLE);
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_notify_pe_idle(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_pe_running = false;
	mutex_unlock(&tcpc_dev->access_lock);

	pd_update_connect_state(pd_port, PD_CONNECT_NONE);
	tcpc_notfiy_pe_idle(tcpc_dev);
}

void pd_notify_pe_wait_vbus_once(struct pd_port *pd_port, int wait_evt)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_vbus_once != wait_evt)
		PE_DBG("%s: wait from %d to %d\n", __func__,
			tcpc_dev->pd_wait_vbus_once, wait_evt);
	tcpc_dev->pd_wait_vbus_once = wait_evt;
	mutex_unlock(&tcpc_dev->access_lock);

	switch (wait_evt) {
	case PD_WAIT_VBUS_VALID_ONCE:
	case PD_WAIT_VBUS_INVALID_ONCE:
		pd_put_vbus_changed_event(tcpc_dev, false);
		break;

	case PD_WAIT_VBUS_SAFE0V_ONCE:
		if (tcpci_is_support(tcpc_dev, TCPC_FLAGS_VSAFE0V_DETECT)) {
			if (typec_check_vsafe0v(tcpc_dev)) {
				pd_put_vbus_safe0v_event(tcpc_dev);
				break;
			}
		} else {
			pd_enable_timer(pd_port, PD_TIMER_VSAFE0V_DELAY);
		}

		tcpci_enable_ext_discharge(tcpc_dev, true);
		break;
	}
}

void pd_notify_pe_error_recovery(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);

	tcpci_notify_hard_reset_state(
		tcpc_dev, TCP_HRESET_RESULT_FAIL);

	tcpc_dev->pd_wait_pr_swap_complete = false;
	dpm_clear_tcp_event(pd_port->devent, TCP_DPM_RET_DROP_ERROR_REOCVERY);
	mutex_unlock(&tcpc_dev->access_lock);

	tcpc_typec_error_recovery(tcpc_dev);
}

#ifdef CONFIG_USB_PD_RECV_HRESET_COUNTER
void pd_notify_pe_over_recv_hreset(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_hard_reset_complete = false;
	tcpc_dev->pd_wait_pr_swap_complete = false;
	mutex_unlock(&tcpc_dev->access_lock);

	disable_irq(chip->irq);
	tcpci_init(tcpc_dev, true);
	typec_refresh_power_status(tcpc_dev);
	tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
	tcpci_set_rx_enable(tcpc_dev, PD_RX_CAP_PE_IDLE);
	timer_enable(tcpc_dev, TYPEC_TIMER_ERROR_RECOVERY);
	enable_irq_wake(chip->irq);
}
#endif	/* CONFIG_USB_PD_RECV_HRESET_COUNTER */

void pd_notify_pe_transit_to_default(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_hard_reset_event_pending = false;
	tcpc_dev->pd_wait_pr_swap_complete = false;
	tcpc_dev->pd_bist_mode = PD_BIST_MODE_DISABLE;

#ifdef CONFIG_USB_PD_DIRECT_CHARGE
	/* pd_during_direct_charge be cleared when non SCP chg */
	if (!tcpc_dev->is_huawei_scp_chg)
		tcpc_dev->pd_during_direct_charge = false;
#endif	/* CONFIG_USB_PD_DIRECT_CHARGE */
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_notify_pe_hard_reset_completed(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpci_notify_hard_reset_state(
		tcpc_dev, TCP_HRESET_RESULT_DONE);
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_notify_pe_send_hard_reset(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	tcpc_awake_attach_lock(tcpc_dev);
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_transmit_state = PD_TX_STATE_WAIT_HARD_RESET;
	tcpci_notify_hard_reset_state(tcpc_dev, TCP_HRESET_SIGNAL_SEND);
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_notify_pe_execute_pr_swap(struct pd_port *pd_port, bool start_swap)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	pd_port->pe_data.during_swap = start_swap;
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_pr_swap_complete = true;
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_notify_pe_cancel_pr_swap(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	if (!tcpc_dev->pd_wait_pr_swap_complete)
		return;

	pd_port->pe_data.during_swap = false;
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_pr_swap_complete = false;
	mutex_unlock(&tcpc_dev->access_lock);

	/*
	 *	CC_Alert was ignored if pd_wait_pr_swap_complete = true
	 *	So enable PDDebounce to detect CC_Again after cancel_pr_swap.
	 */

	timer_enable(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);

	if (!typec_check_vbus_valid(tcpc_dev)
		&& (pd_port->request_v >= 4000)) {
		TCPC_DBG("cancel_pr_swap_vbus=0\r\n");
		pd_put_tcp_pd_event(pd_port, TCP_DPM_EVT_ERROR_RECOVERY,
				    PD_TCP_FROM_PE);
	}
}

void pd_notify_pe_reset_protocol(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_pr_swap_complete = false;
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_noitfy_pe_bist_mode(struct pd_port *pd_port, uint8_t mode)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_bist_mode = mode;
	mutex_unlock(&tcpc_dev->access_lock);
}

bool pd_is_pe_wait_pd_transmit_done(struct pd_port *pd_port)
{
	bool tx_wait = false;
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tx_wait = tcpc_dev->pd_transmit_state == PD_TX_STATE_WAIT_CRC_PD;
	mutex_unlock(&tcpc_dev->access_lock);

	return tx_wait;
}

void pd_notify_pe_transmit_msg(
	struct pd_port *pd_port, uint8_t type)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_transmit_state = type;
	mutex_unlock(&tcpc_dev->access_lock);
}

void pd_notify_pe_pr_changed(struct pd_port *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	/* Check mutex later, actually,
	 * typec layer will ignore all cc-change during PR-SWAP
	 */
	tcpc_handle_pe_pr_swap(tcpc_dev);
}

void pd_notify_pe_snk_explicit_contract(struct pd_port *pd_port)
{
#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	struct pe_data *pe_data = &pd_port->pe_data;
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	if (pe_data->explicit_contract)
		return;

	if (tcpc_dev->typec_remote_rp_level == TYPEC_CC_VOLT_SNK_3_0)
		pe_data->pd_traffic_control = PD_SINK_TX_OK;
	else
		pe_data->pd_traffic_control = PD_SINK_TX_NG;

#ifdef CONFIG_USB_PD_REV30_SNK_FLOW_DELAY_STARTUP
	if (pe_data->pd_traffic_control == PD_SINK_TX_OK) {
		pe_data->pd_traffic_control = PD_SINK_TX_START;
		pd_restart_timer(pd_port, PD_TIMER_SNK_FLOW_DELAY);
	}
#endif	/* CONFIG_USB_PD_REV30_SNK_FLOW_DELAY_STARTUP */
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */
}

void pd_notify_pe_src_explicit_contract(struct pd_port *pd_port)
{
	uint8_t pull = 0;

	struct pe_data *pe_data = &pd_port->pe_data;
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	if (pe_data->explicit_contract) {
#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
#ifdef CONFIG_USB_PD_REV30_SRC_FLOW_DELAY_STARTUP
		if (pd_check_rev30(pd_port) &&
			(pe_data->pd_traffic_control == PD_SOURCE_TX_START))
			pd_restart_timer(pd_port, PD_TIMER_SINK_TX);
#endif	/* CONFIG_USB_PD_REV30_SRC_FLOW_DELAY_STARTUP */
#endif	/* CONFIG_USB_PD_REV30_COLLISION_AVOID */
		return;
	}

	if (tcpc_dev->typec_local_rp_level == TYPEC_CC_RP_DFT)
		pull = TYPEC_CC_RP_1_5;

#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	if (pd_check_rev30(pd_port)) {
		pull = TYPEC_CC_RP_3_0;

#ifdef CONFIG_USB_PD_REV30_SRC_FLOW_DELAY_STARTUP
		pe_data->pd_traffic_control = PD_SOURCE_TX_START;
		pd_enable_timer(pd_port, PD_TIMER_SINK_TX);
#else
		pe_data->pd_traffic_control = PD_SINK_TX_OK;
#endif	/* CONFIG_USB_PD_REV30_SRC_FLOW_DELAY_STARTUP */
	}
#endif	/* CONFIG_USB_PD_REV30_COLLISION_AVOID */

	if (pull)
		tcpci_set_cc(tcpc_dev, pull);
}

#ifdef CONFIG_USB_PD_DIRECT_CHARGE
void pd_notify_pe_direct_charge(struct pd_port *pd_port, bool en)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

#ifdef CONFIG_HW_USB_PD_REV30_PPS_SINK
	/* TODO: check it later */
	if (pd_port->request_apdo)
		en = true;
#endif	/* CONFIG_HW_USB_PD_REV30_PPS_SINK */

	mutex_lock(&tcpc_dev->access_lock);
	/* pd_during_direct_charge be cleared when non SCP chg */
	if (!tcpc_dev->is_huawei_scp_chg)
		tcpc_dev->pd_during_direct_charge = en;
	mutex_unlock(&tcpc_dev->access_lock);
}
#endif	/* CONFIG_USB_PD_DIRECT_CHARGE */

/* ---- init  ---- */
static int tcpc_event_thread(void *param)
{
	struct tcpc_device *tcpc_dev = param;
	struct sched_param sch_param = {.sched_priority = MAX_RT_PRIO - 2};

	/* set_user_nice(current, -20); */
	/* current->flags |= PF_NOFREEZE;*/

	sched_setscheduler(current, SCHED_FIFO, &sch_param);

	while (true) {
		wait_event_interruptible(tcpc_dev->event_loop_wait_que,
				atomic_read(&tcpc_dev->pending_event) ||
				tcpc_dev->event_loop_thead_stop);
		if (kthread_should_stop() || tcpc_dev->event_loop_thead_stop)
			break;
		do {
			atomic_dec_if_positive(&tcpc_dev->pending_event);
		} while (pe_run(tcpc_dev));
	}

	return 0;
}

int tcpci_event_init(struct tcpc_device *tcpc_dev)
{
	tcpc_dev->event_task = kthread_create(tcpc_event_thread, tcpc_dev,
			"tcpc_event_%s.%p", dev_name(&tcpc_dev->dev), tcpc_dev);
	tcpc_dev->event_loop_thead_stop = false;

	init_waitqueue_head(&tcpc_dev->event_loop_wait_que);
	atomic_set(&tcpc_dev->pending_event, 0);
	wake_up_process(tcpc_dev->event_task);

	return 0;
}

void tcpci_wakeup_event_task(struct tcpc_device *tcpc_dev)
{
	if (!tcpc_dev)
		return;

	atomic_inc(&tcpc_dev->pending_event);
	wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
}

int tcpci_event_deinit(struct tcpc_device *tcpc_dev)
{
	if (tcpc_dev->event_task != NULL) {
		tcpc_dev->event_loop_thead_stop = true;
		wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
		kthread_stop(tcpc_dev->event_task);
	}
	return 0;
}

void pd_notify_usb_port_attach(struct tcpc_device *tcpc, enum typec_attach_type type)
{
	if (!tcpc || !tcpc->pd_inited_flag)
		return;

#ifdef CONFIG_PD_WAIT_BC12
	tcpc->wait_bc12_cnt = 0;
	if (type == TYPEC_ATTACHED_SNK) {
		timer_enable(tcpc, TYPEC_RT_TIMER_SINK_WAIT_BC12);
		return;
	}
#endif /* CONFIG_PD_WAIT_BC12 */

	pd_put_cc_attach_event(tcpc, type);
}

void pd_notify_usb_port_detach(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return;

	if (tcpc->pd_inited_flag)
		pd_put_cc_detach_event(tcpc);
	else
		pd_event_buf_reset(tcpc);
}

void pd_notify_pe_rplvl_change(struct tcpc_device *tcpc, uint8_t cc_res)
{
	if (!tcpc || !tcpc->pd_port.pe_data.pd_prev_connected)
		return;

	pd_put_sink_tx_event(tcpc, cc_res);
}

void pd_put_sink_tx_event(struct tcpc_device *tcpc, uint8_t cc_res)
{
#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	struct pd_event evt = {
		.event_type = PD_EVT_HW_MSG,
		.msg = PD_HW_SINK_TX_CHANGE,
		.pd_msg = NULL,
	};

	if (!tcpc)
		return;

	evt.msg_sec = (cc_res == TYPEC_CC_VOLT_SNK_3_0);
	pd_put_event(tcpc, &evt, false);
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */
}

bool pd_is_pe_running(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return false;

	return tcpc->pd_pe_running;
}

void pd_handle_timer_out(struct tcpc_device *tcpc, uint32_t timer_id)
{
	struct pd_event pd_event = {0};

	if (!tcpc)
		return;

	pd_event.event_type = PD_EVT_TIMER_MSG;
	pd_event.msg = timer_id;
	pd_event.pd_msg = NULL;

	switch (timer_id) {
	case PD_TIMER_VDM_MODE_ENTRY:
	case PD_TIMER_VDM_MODE_EXIT:
	case PD_TIMER_VDM_RESPONSE:
	case PD_TIMER_UVDM_RESPONSE:
		pd_put_vdm_event(tcpc, &pd_event, false);
		break;
	case PD_TIMER_VSAFE0V_DELAY:
		pd_put_vbus_safe0v_event(tcpc);
		break;

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	case PD_TIMER_DISCARD:
		tcpc->pd_discard_pending = false;
		pd_put_hw_event(tcpc, PD_HW_TX_DISCARD);
		break;
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */

#if CONFIG_HW_USB_PD_VBUS_STABLE_TOUT
	case PD_TIMER_VBUS_STABLE:
		pd_put_vbus_stable_event(tcpc);
		break;
#endif /* CONFIG_HW_USB_PD_VBUS_STABLE_TOUT */

	case PD_TIMER_VBUS_PRESENT:
		pd_put_vbus_present_event(tcpc);
		break;
	case PD_PE_VDM_POSTPONE:
		tcpc->pd_postpone_vdm_timeout = true;
		tcpci_wakeup_event_task(tcpc);
		break;
	case PD_TIMER_PE_IDLE_TOUT:
		TCPC_INFO("pe_idle tout\n");
		pd_put_pe_event(&tcpc->pd_port, PD_PE_IDLE);
		break;
	default:
		pd_put_event(tcpc, &pd_event, false);
		break;
	}
}