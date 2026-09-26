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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/cpu.h>

#include "include/tcpci.h"
#include "include/tcpci_typec.h"
#include "include/tcpci_event.h"

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
#include "include/pd_dpm_core.h"
#endif

static inline void tcpci_alert_frswap(struct tcpc_device *tcpc_dev)
{
	D("FR_swap\n");

	if (tcpc_dev->ops->da_vbus_en_status)
		tcpc_dev->ops->da_vbus_en_status(tcpc_dev);
}

static inline int tcpci_alert_cc_changed(struct tcpc_device *tcpc_dev)
{
	return hisi_tcpc_typec_handle_cc_change(tcpc_dev);
}

void hisi_tcpci_vbus_level_init(struct tcpc_device *tcpc_dev,
		uint16_t power_status)
{
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->vbus_level =
		(power_status & TCPC_REG_POWER_STATUS_VBUS_PRES) ?
				TCPC_VBUS_VALID : TCPC_VBUS_INVALID;
	D("vbus_level %s\n", (tcpc_dev->vbus_level == TCPC_VBUS_VALID)
				? "TCPC_VBUS_VALID" : "TCPC_VBUS_INVALID");
	mutex_unlock(&tcpc_dev->access_lock);
}

static int tcpci_alert_power_status_changed(struct tcpc_device *tcpc_dev)
{
	int rv;
	uint16_t power_status = 0;

	rv = tcpci_get_power_status(tcpc_dev, &power_status);
	if (rv < 0)
		return rv;

	hisi_tcpci_vbus_level_init(tcpc_dev, power_status);

	TCPC_INFO("ps_change=%u\n", tcpc_dev->vbus_level);
	rv = hisi_tcpc_typec_handle_ps_change(tcpc_dev, tcpc_dev->vbus_level);
	if (rv < 0)
		return rv;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	hisi_pd_put_vbus_changed_event(tcpc_dev, true);
#endif

	return rv;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static int tcpci_alert_tx_success(struct tcpc_device *tcpc_dev)
{
	uint8_t tx_state;
	pd_event_t evt = {
		.event_type = PD_EVT_CTRL_MSG,
		.msg = PD_CTRL_GOOD_CRC,
		.pd_msg = NULL,
	};

	mutex_lock(&tcpc_dev->access_lock);
	tx_state = tcpc_dev->pd_transmit_state;
	tcpc_dev->pd_transmit_state = PD_TX_STATE_GOOD_CRC;
	mutex_unlock(&tcpc_dev->access_lock);

	if (tx_state == PD_TX_STATE_WAIT_CRC_VDM)
		hisi_pd_put_vdm_event(tcpc_dev, &evt, false);
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	else if (tx_state == PD_TX_STATE_WAIT_CRC_PD)
#else
	else
#endif
		hisi_pd_put_event(tcpc_dev, &evt, false);

	return 0;
}

static int tcpci_alert_tx_failed(struct tcpc_device *tcpc_dev)
{
	uint8_t tx_state;

	if (tcpc_dev->typec_attach_new == TYPEC_ATTACHED_SNK)
		tcpci_tcpc_print_pd_fsm_state(tcpc_dev);

	mutex_lock(&tcpc_dev->access_lock);
	tx_state = tcpc_dev->pd_transmit_state;
	tcpc_dev->pd_transmit_state = PD_TX_STATE_NO_GOOD_CRC;
	mutex_unlock(&tcpc_dev->access_lock);

	if (tx_state == PD_TX_STATE_WAIT_CRC_VDM)
		vdm_put_hw_event(tcpc_dev, PD_HW_TX_FAILED);
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	else if (tx_state == PD_TX_STATE_WAIT_CRC_PD)
#else
	else
#endif
		pd_put_hw_event(tcpc_dev, PD_HW_TX_FAILED);

	return 0;
}

static int tcpci_alert_tx_discard(struct tcpc_device *tcpc_dev)
{
	uint8_t tx_state;
	bool retry_crc_discard = false;

	mutex_lock(&tcpc_dev->access_lock);
	tx_state = tcpc_dev->pd_transmit_state;
	tcpc_dev->pd_transmit_state = PD_TX_STATE_DISCARD;
	mutex_unlock(&tcpc_dev->access_lock);

	tcpci_tcpc_print_pd_fsm_state(tcpc_dev);

	if (tx_state == PD_TX_STATE_WAIT_CRC_VDM) {
		hisi_pd_put_last_vdm_event(tcpc_dev);
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	} else if (tx_state == PD_TX_STATE_WAIT_CRC_PD) {
#else
	} else {
#endif
		retry_crc_discard = (tcpc_dev->tcpc_flags &
				TCPC_FLAGS_RETRY_CRC_DISCARD) != 0;

		if (retry_crc_discard) {
#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
			tcpc_dev->pd_discard_pending = true;
			hisi_tcpc_enable_timer(tcpc_dev, PD_TIMER_DISCARD);
#else
			TCPC_ERR("RETRY_CRC_DISCARD\n");
#endif
		} else {
			pd_put_hw_event(tcpc_dev, PD_HW_TX_FAILED);
		}
	}

	return 0;
}

static int tcpci_alert_recv_msg(struct tcpc_device *tcpc_dev)
{
	int retval;
	pd_msg_t *pd_msg = NULL;
	enum tcpm_transmit_type type = TCPC_TX_SOP;
	const uint32_t alert_rx =
			TCPC_REG_ALERT_RX_STATUS | TCPC_REG_ALERT_RX_BUF_OVF;

	pd_msg = hisi_pd_alloc_msg(tcpc_dev);
	if (!pd_msg) {
		tcpci_alert_status_clear(tcpc_dev, alert_rx);
		return -ENOMEM;
	}

	retval = tcpci_get_message(tcpc_dev, pd_msg->payload,
			&pd_msg->msg_hdr, &type);
	if (retval < 0) {
		TCPC_INFO("recv_msg failed: %d\n", retval);
		hisi_pd_free_msg(tcpc_dev, pd_msg);
		return retval;
	}

	pd_msg->frame_type = (uint8_t)type;

	hisi_pd_put_pd_msg_event(tcpc_dev, pd_msg);

	return 0;
}

static int tcpci_alert_rx_overflow(struct tcpc_device *tcpc_dev)
{
	int rv;
	uint32_t alert_status = 0;

	rv = tcpci_get_alert_status(tcpc_dev, &alert_status);
	if (rv)
		return rv;

	if (alert_status & TCPC_REG_ALERT_RX_STATUS)
		rv = tcpci_alert_recv_msg(tcpc_dev);
	else
		tcpci_alert_status_clear(tcpc_dev,
			TCPC_REG_ALERT_RX_BUF_OVF | TCPC_REG_ALERT_RX_STATUS);

	return rv;
}

static int tcpci_alert_recv_hard_reset(struct tcpc_device *tcpc_dev)
{
	hisi_pd_put_recv_hard_reset_event(tcpc_dev);
	tcpci_set_frswap(tcpc_dev, false);

	return 0;
}
#endif

static int tcpci_alert_fault(struct tcpc_device *tcpc_dev)
{
	uint8_t status = 0;

	tcpci_get_fault_status(tcpc_dev, &status);
	tcpci_fault_status_clear(tcpc_dev, status);

	return 0;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static bool tcpci_check_hard_reset_complete(
			struct tcpc_device *tcpc_dev, uint32_t alert_status)
{
	D("+\n");

	if ((alert_status & TCPC_REG_ALERT_HRESET_SUCCESS) ==
			TCPC_REG_ALERT_HRESET_SUCCESS) {
		hisi_pd_put_sent_hard_reset_event(tcpc_dev);
		return true;
	}
	if (alert_status & TCPC_REG_ALERT_TX_DISCARDED) {
		TCPC_INFO("HResetFailed\n");
		tcpci_transmit(tcpc_dev, TCPC_TX_HARD_RESET, 0, NULL);
		return false;
	}

	D("-\n");
	return false;
}
#endif


static int __tcpci_alert(struct tcpc_device *tcpc_dev)
{
	int rv;
	uint32_t alert_status = 0;

	rv = tcpci_get_alert_status(tcpc_dev, &alert_status);
	if (rv)
		return HISI_TCPC_ALERT_I2C_TIMEOUT;

	if (!alert_status)
		return HISI_TCPC_ALERT_STATUS_ZERO;

	/* Don't clear tx interrupt, clear it when read Message. */
	tcpci_alert_status_clear(tcpc_dev,
			alert_status & ~TCPC_REG_ALERT_RX_MASK);

	if (tcpc_dev->typec_role == TYPEC_ROLE_UNKNOWN) {
		D("typec_role unknown\n");
		return 0;
	}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_transmit_state == PD_TX_STATE_WAIT_HARD_RESET) {
		D("PD_TX_STATE_WAIT_HARD_RESET\n");
		tcpci_check_hard_reset_complete(tcpc_dev, alert_status);
		/*
		 * if waiting for hard reset complete,
		 * ignore tx sucess, failed, discard
		 */
		alert_status &= ~TCPC_REG_ALERT_TX_MASK;
	}
#endif

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (alert_status & TCPC_REG_ALERT_TX_FAILED) {
		D("TCPC_REG_ALERT_TX_FAILED\n");
		tcpci_alert_tx_failed(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_TX_DISCARDED) {
		D("TCPC_REG_ALERT_TX_DISCARDED\n");
		tcpci_alert_tx_discard(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_TX_SUCCESS) {
		D("TCPC_REG_ALERT_TX_SUCCESS\n");
		tcpci_alert_tx_success(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_RX_STATUS) {
		D("TCPC_REG_ALERT_RX_STATUS\n");
		tcpci_alert_recv_msg(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_V_ALARM_HI)
		D("TCPC_REG_ALERT_V_ALARM_HI\n");

	if (alert_status & TCPC_REG_ALERT_V_ALARM_LO)
		D("TCPC_REG_ALERT_V_ALARM_LO\n");

	if (alert_status & TCPC_REG_ALERT_RX_HARD_RST) {
		D("TCPC_REG_ALERT_RX_HARD_RST\n");
		tcpci_alert_recv_hard_reset(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_RX_BUF_OVF) {
		D("TCPC_REG_ALERT_RX_BUF_OVF\n");
		tcpci_alert_rx_overflow(tcpc_dev);
	}
#endif

	if (alert_status & TCPC_REG_ALERT_FAULT) {
		D("TCPC_REG_ALERT_FAULT\n");
		tcpci_alert_fault(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_CC_STATUS) {
		D("TCPC_REG_ALERT_CC_STATUS\n");
		tcpci_alert_cc_changed(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_POWER_STATUS) {
		D("TCPC_REG_ALERT_POWER_STATUS\n");
		tcpci_alert_power_status_changed(tcpc_dev);
	}

	if (alert_status & TCPC_REG_ALERT_FR_SWAP)
		tcpci_alert_frswap(tcpc_dev);

	return 0;
}

int hisi_tcpci_alert(struct tcpc_device *tcpc_dev)
{
	int ret;
	static uint32_t cnt = 0;

	hisi_tcpci_lock_typec(tcpc_dev);
	ret = __tcpci_alert(tcpc_dev);
	hisi_tcpci_unlock_typec(tcpc_dev);

	/*
	 * Charger Irq or I2C Timeout: cnt ascend by 1,
	 * PD Irq: cnt inited to 0.
	 */
	if (ret == HISI_TCPC_ALERT_STATUS_ZERO ||
			ret == HISI_TCPC_ALERT_I2C_TIMEOUT) {
		cnt++;

		if (cnt >= HISI_TCPC_OTHER_IRQ_MAX_LOOPS) {
			E("Irq gpio lows too long\n");
			cnt = 0;
			return ret;
		}

		if (cnt <= 10)
			mdelay(1);
		else
			msleep(2);

		ret = 0;
	} else {
		cnt = 0;
	}

	return ret;
}
EXPORT_SYMBOL(hisi_tcpci_alert);

static int __tcpci_set_wake_lock_pd(struct tcpc_device *tcpc,
		bool pd_lock, bool user_lock)
{
	bool ori_lock = false;
	bool new_lock = false;

	if (tcpc->wake_lock_pd && tcpc->wake_lock_user)
		ori_lock = true;
	else
		ori_lock = false;

	if (pd_lock && user_lock)
		new_lock = true;
	else
		new_lock = false;

	if (new_lock != ori_lock) {
		if (new_lock) {
			D("lock attach_wake_lock\n");
			__pm_stay_awake(&tcpc->attach_wake_lock);
		} else {
			D("unlock attach_wake_lock\n");
			__pm_relax(&tcpc->attach_wake_lock);
		}
	}

	return 0;
}

static int tcpci_set_wake_lock_pd(struct tcpc_device *tcpc, bool pd_lock)
{
	uint8_t wake_lock_pd;

	mutex_lock(&tcpc->access_lock);
	wake_lock_pd = tcpc->wake_lock_pd;

	if (pd_lock)
		wake_lock_pd++;
	else if (wake_lock_pd > 0)
		wake_lock_pd--;

	if (wake_lock_pd == 0)
		__pm_wakeup_event(&tcpc->dettach_temp_wake_lock,
				jiffies_to_msecs(5 * HZ));

	__tcpci_set_wake_lock_pd(tcpc, wake_lock_pd, tcpc->wake_lock_user);

	if (wake_lock_pd == 1)
		__pm_relax(&tcpc->dettach_temp_wake_lock);

	tcpc->wake_lock_pd = wake_lock_pd;
	mutex_unlock(&tcpc->access_lock);

	return 0;
}

static int tcpci_report_usb_port_attached(struct tcpc_device *tcpc)
{
	TCPC_INFO("usb_port_attached\n");

	if (tcpc->typec_attach_new != TYPEC_ATTACHED_AUDIO)
		tcpci_set_wake_lock_pd(tcpc, true);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	pd_put_cc_attached_event(tcpc, tcpc->typec_attach_new);
#endif

	return 0;
}

static int tcpci_report_usb_port_detached(struct tcpc_device *tcpc)
{
	TCPC_INFO("usb_port_detached\n");

	tcpci_set_wake_lock_pd(tcpc, false);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	hisi_pd_put_cc_detached_event(tcpc);
#endif

	return 0;
}

int hisi_tcpci_report_usb_port_changed(struct tcpc_device *tcpc)
{
#ifdef CONFIG_DUAL_ROLE_USB_INTF
	switch (tcpc->typec_attach_new) {
	case TYPEC_UNATTACHED:
	case TYPEC_ATTACHED_AUDIO:
	case TYPEC_ATTACHED_DEBUG:
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case TYPEC_ATTACHED_DBGACC_SNK:
#endif
		tcpc->dual_role_pr = DUAL_ROLE_PROP_PR_NONE;
		tcpc->dual_role_dr = DUAL_ROLE_PROP_DR_NONE;
		tcpc->dual_role_mode = DUAL_ROLE_PROP_MODE_NONE;
		tcpc->dual_role_vconn = DUAL_ROLE_PROP_VCONN_SUPPLY_NO;
		break;

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case TYPEC_ATTACHED_CUSTOM_SRC:
#endif
	case TYPEC_ATTACHED_SNK:
		tcpc->dual_role_pr = DUAL_ROLE_PROP_PR_SNK;
		tcpc->dual_role_dr = DUAL_ROLE_PROP_DR_DEVICE;
		tcpc->dual_role_mode = DUAL_ROLE_PROP_MODE_UFP;
		tcpc->dual_role_vconn = DUAL_ROLE_PROP_VCONN_SUPPLY_NO;
		break;
	case TYPEC_ATTACHED_SRC:
		tcpc->dual_role_pr = DUAL_ROLE_PROP_PR_SRC;
		tcpc->dual_role_dr = DUAL_ROLE_PROP_DR_HOST;
		tcpc->dual_role_mode = DUAL_ROLE_PROP_MODE_DFP;
		tcpc->dual_role_vconn = DUAL_ROLE_PROP_VCONN_SUPPLY_YES;
		break;
	default:
		D("unsupport attach:%d\n", tcpc->typec_attach_new);
		break;
	}
	hisi_tcpc_dual_role_instance_changed(tcpc->dr_usb);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	tcpci_notify_typec_state(tcpc);

	if (tcpc->typec_attach_old == TYPEC_UNATTACHED) {
		tcpci_report_usb_port_attached(tcpc);
		tcpci_snk_unattach_by_vbus_bypass(tcpc, true);
	} else if (tcpc->typec_attach_new == TYPEC_UNATTACHED) {
		tcpci_snk_unattach_by_vbus_bypass(tcpc, false);
		tcpci_report_usb_port_detached(tcpc);
	} else {
		TCPC_DBG("TCPC Attach Again\n");
	}

	return 0;
}
EXPORT_SYMBOL(hisi_tcpci_report_usb_port_changed);

static inline int tcpci_report_power_control_on(struct tcpc_device *tcpc)
{
	tcpci_set_wake_lock_pd(tcpc, true);
	return 0;
}

static inline int tcpci_report_power_control_off(struct tcpc_device *tcpc)
{
	tcpci_set_wake_lock_pd(tcpc, false);
	return 0;
}

int hisi_tcpci_report_power_control(struct tcpc_device *tcpc, bool en)
{
	D("%s\n", en ? "true" : "false");

	if (tcpc->typec_power_ctrl == en)
		return 0;

	tcpc->typec_power_ctrl = en;

	if (en)
		tcpci_report_power_control_on(tcpc);
	else
		tcpci_report_power_control_off(tcpc);

	return 0;
}
