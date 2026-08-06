/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * TCPC Interface for alert handler
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/cpu.h>
#include "inc/tcpci.h"
#include "inc/tcpm_dual_role.h"
#include "inc/tcpc_core.h"
#include "inc/typec_vbus.h"
#include "typec_lpm.h"
#include "typec_common.h"
#include "typec_state_machine.h"
#ifdef CONFIG_HW_USB_POWER_DELIVERY
#include "inc/tcpci_event.h"
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static inline int tcpci_alert_cc_changed(struct tcpc_device *tcpc)
{
	return typecs_handle_cc_change(tcpc);
}

static void tcpci_alert_vbus_changed(struct tcpc_device *tcpc)
{
	bool show_msg = true;

#ifdef CONFIG_USB_PD_DIRECT_CHARGE
	if (tcpc->pd_during_direct_charge && (tcpc->vbus_level != 0))
		show_msg = false;
#endif /* CONFIG_USB_PD_DIRECT_CHARGE */

	if (show_msg)
		TCPC_INFO("%s: ps_change=%d\n", __func__, tcpc->vbus_level);

	typecs_handle_ps_change(tcpc);
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	pd_put_vbus_changed_event(tcpc, true);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
}

static inline int tcpci_alert_power_changed(struct tcpc_device *tcpc)
{
	return typec_refresh_power_status(tcpc);
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
static int tcpci_alert_tx_success(struct tcpc_device *tcpc)
{
	uint8_t tx_state;
	struct pd_event evt = {
		.event_type = PD_EVT_CTRL_MSG,
		.msg = PD_CTRL_GOOD_CRC,
		.pd_msg = NULL,
	};

	mutex_lock(&tcpc->access_lock);
	tx_state = tcpc->pd_transmit_state;
	tcpc->pd_transmit_state = PD_TX_STATE_GOOD_CRC;
	mutex_unlock(&tcpc->access_lock);

	if (tx_state == PD_TX_STATE_WAIT_CRC_VDM)
		pd_put_vdm_event(tcpc, &evt, false);
	else
		pd_put_event(tcpc, &evt, false);

	return 0;
}

static int tcpci_alert_tx_failed(struct tcpc_device *tcpc)
{
	uint8_t tx_state;

	mutex_lock(&tcpc->access_lock);
	tx_state = tcpc->pd_transmit_state;
	tcpc->pd_transmit_state = PD_TX_STATE_NO_GOOD_CRC;
	mutex_unlock(&tcpc->access_lock);

	if (tx_state == PD_TX_STATE_WAIT_CRC_VDM)
		vdm_put_hw_event(tcpc, PD_HW_TX_FAILED);
	else
		pd_put_hw_event(tcpc, PD_HW_TX_FAILED);

	return 0;
}

static int tcpci_alert_tx_discard(struct tcpc_device *tcpc)
{
	uint8_t tx_state;
	bool retry_crc_discard = false;

	mutex_lock(&tcpc->access_lock);
	tx_state = tcpc->pd_transmit_state;
	tcpc->pd_transmit_state = PD_TX_STATE_DISCARD;
	mutex_unlock(&tcpc->access_lock);

	TCPC_INFO("alert discard\n");

	if (tx_state == PD_TX_STATE_WAIT_CRC_VDM) {
		pd_put_last_vdm_event(tcpc);
	} else {
		retry_crc_discard = tcpci_is_support(tcpc,
			TCPC_FLAGS_RETRY_CRC_DISCARD);
		if (retry_crc_discard) {
#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
			tcpc->pd_discard_pending = true;
			timer_enable(tcpc, PD_TIMER_DISCARD);
#else
			TCPC_ERR("retry crc discard\n");
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */
		} else {
			pd_put_hw_event(tcpc, PD_HW_TX_FAILED);
		}
	}
	return 0;
}

static int tcpci_alert_recv_msg(struct tcpc_device *tcpc)
{
	int ret;
	enum tcpc_pd_transmit_type type;
	struct pd_msg *pd_msg = NULL;
	const uint32_t alert_rx = TCPC_REG_ALERT_RX_STATUS |
		TCPC_REG_ALERT_RX_BUF_OVF;

	pd_msg = pd_alloc_msg(tcpc);
	if (!pd_msg) {
		tcpci_alert_status_clear(tcpc, alert_rx);
		return -ENOMEM;
	}

	ret = tcpci_get_message(tcpc, pd_msg->payload,
		&pd_msg->msg_hdr, &type);
	if (ret < 0) {
		TCPC_INFO("recv_msg failed: %d\n", ret);
		pd_free_msg(tcpc, pd_msg);
		return ret;
	}

	pd_msg->frame_type = (uint8_t) type;
	pd_put_pd_msg_event(tcpc, pd_msg);
	return 0;
}

static int tcpci_alert_rx_overflow(struct tcpc_device *tcpc)
{
	int rv;
	uint32_t alert_status;

	TCPC_INFO("alert tx overflow\n");

	rv = tcpci_get_alert_status(tcpc, &alert_status);
	if (rv)
		return rv;

	if (alert_status & TCPC_REG_ALERT_RX_STATUS)
		return tcpci_alert_recv_msg(tcpc);

	return 0;
}

static int tcpci_alert_recv_hard_reset(struct tcpc_device *tcpc)
{
	TCPC_INFO("alert recv hard reset\r\n");
	pd_put_recv_hard_reset_event(tcpc);
	tcpci_init_alert_mask(tcpc);
	return 0;
}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static int tcpci_alert_vendor_defined(struct tcpc_device *tcpc)
{
	tcpci_alert_vendor_defined_handler(tcpc);
	return 0;
}

static int tcpci_alert_fault(struct tcpc_device *tcpc)
{
	uint8_t status = 0;

	tcpci_get_fault_status(tcpc, &status);
	TCPC_INFO("alert fault=0x%x\n", status);
	tcpci_fault_status_clear(tcpc, status);
	return 0;
}

static int tcpci_alert_wakeup(struct tcpc_device *tcpc)
{
	TYPEC_INFO("%s++\n", __func__);

	if (tcpci_is_support(tcpc, TCPC_FLAGS_LPM_WAKEUP_WATCHDOG)) {
		TCPC_INFO("wakeup\n");

		if (tcpc->typec_remote_cc[0] == TYPEC_CC_DRP_TOGGLING)
			typec_enable_wakeup_timer(tcpc->lpm, true);
	}

	return 0;
}

#ifdef CONFIG_TYPEC_CAP_RA_DETACH
static int tcpci_alert_ra_detach(struct tcpc_device *tcpc)
{
	if (tcpci_is_support(tcpc, TCPC_FLAGS_CHECK_RA_DETACHE)) {
		TCPC_DBG("ra detach\n");

		if (tcpc->typec_remote_cc[0] == TYPEC_CC_DRP_TOGGLING)
			typec_enter_lpm_again(tcpc->lpm);
	}

	return 0;
}
#endif /* CONFIG_TYPEC_CAP_RA_DETACH */

struct tcpci_alert_handler {
	uint32_t bit_mask;
	int (*handler)(struct tcpc_device *tcpc);
};

#define decl_tcpci_alert_handler(xbit, xhandler) { \
	.bit_mask = 1 << (xbit), \
	.handler = (xhandler), \
}

static const struct tcpci_alert_handler tcpci_alert_handlers[] = {
	decl_tcpci_alert_handler(15, tcpci_alert_vendor_defined),
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	decl_tcpci_alert_handler(4, tcpci_alert_tx_failed),
	decl_tcpci_alert_handler(5, tcpci_alert_tx_discard),
	decl_tcpci_alert_handler(6, tcpci_alert_tx_success),
	decl_tcpci_alert_handler(2, tcpci_alert_recv_msg),
	decl_tcpci_alert_handler(7, NULL),
	decl_tcpci_alert_handler(8, NULL),
	decl_tcpci_alert_handler(3, tcpci_alert_recv_hard_reset),
	decl_tcpci_alert_handler(10, tcpci_alert_rx_overflow),
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	decl_tcpci_alert_handler(16, tcpci_alert_wakeup),
#ifdef CONFIG_TYPEC_CAP_RA_DETACH
	decl_tcpci_alert_handler(21, tcpci_alert_ra_detach),
#endif /* CONFIG_TYPEC_CAP_RA_DETACH */

	decl_tcpci_alert_handler(9, tcpci_alert_fault),
	decl_tcpci_alert_handler(0, tcpci_alert_cc_changed),
	decl_tcpci_alert_handler(1, tcpci_alert_power_changed),
};

#ifdef CONFIG_HW_USB_POWER_DELIVERY
static inline bool tcpci_check_hard_reset_complete(
	struct tcpc_device *tcpc, uint32_t alert_status)
{
	if ((alert_status & TCPC_REG_ALERT_HRESET_SUCCESS) ==
		TCPC_REG_ALERT_HRESET_SUCCESS) {
		pd_put_sent_hard_reset_event(tcpc);
		return true;
	}

	if (alert_status & TCPC_REG_ALERT_TX_DISCARDED) {
		TCPC_INFO("hard reset failed\r\n");
		tcpci_transmit(tcpc, TCPC_TX_HARD_RESET, 0, NULL);
		return false;
	}

	return false;
}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static int tcpci_hanlde_alert(struct tcpc_device *tcpc)
{
	int i;
	uint32_t alert_status;
	uint32_t alert_mask;

	if (tcpci_get_alert_status(tcpc, &alert_status) ||
		tcpci_get_alert_mask(tcpc, &alert_mask))
		return -EPERM;

	if (alert_status != 0)
		TCPC_DBG("alert:0x%04x, mask:0x%04x\n", alert_status,
			alert_mask);

	tcpci_set_alert_mask(tcpc, 0);
	alert_status &= alert_mask;
	tcpci_alert_status_clear(tcpc,
		alert_status & (~TCPC_REG_ALERT_RX_MASK));

	if (tcpc->typec_role == TYPEC_ROLE_UNKNOWN)
		return 0;

	if (alert_status & TCPC_REG_ALERT_EXT_VBUS_80)
		alert_status |= TCPC_REG_ALERT_POWER_STATUS;

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	if (tcpc->pd_transmit_state == PD_TX_STATE_WAIT_HARD_RESET) {
		tcpci_check_hard_reset_complete(tcpc, alert_status);
		alert_status &= ~TCPC_REG_ALERT_TX_MASK;
	}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	for (i = 0; i < ARRAY_SIZE(tcpci_alert_handlers); i++) {
		if ((tcpci_alert_handlers[i].bit_mask & alert_status) == 0)
			continue;
		if (tcpci_alert_handlers[i].handler)
			tcpci_alert_handlers[i].handler(tcpc);
	}

	tcpci_set_alert_mask(tcpc, alert_mask);
	typec_refresh_power_status(tcpc);
	tcpci_alert_vbus_changed(tcpc);
	return 0;
}

int tcpci_alert(struct tcpc_device *tcpc)
{
	int ret;

	typec_lock(tcpc);
	ret = tcpci_hanlde_alert(tcpc);
	typec_unlock(tcpc);

	return ret;
}
