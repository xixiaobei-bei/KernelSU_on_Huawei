/*
 * typci.c
 *
 * tcpc interface
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

#include "inc/tcpci.h"
#include <linux/time.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include "inc/tcpc_core.h"

static struct tcpc_device *g_tcpc_dev;

void set_tcpc_dev(struct tcpc_device *tcpc)
{
	if (!g_tcpc_dev)
		g_tcpc_dev = tcpc;
}

void tcpci_set_cc_mode(int mode)
{
	int pull = mode ? TYPEC_CC_DRP : TYPEC_CC_RD;

	if (!g_tcpc_dev) {
		pr_info("g_tcpc_dev is NULL\n");
		return;
	}

	pr_info("set_cc_mode mode %d ,pull %d\n", mode, pull);
	tcpci_set_cc(g_tcpc_dev, pull);
	if (pull == TYPEC_CC_RD)
		g_tcpc_dev->typec_role = TYPEC_ROLE_SNK;
	else
		g_tcpc_dev->typec_role = TYPEC_ROLE_TRY_SNK;
}

int tcpci_get_cc_mode(void)
{
	if (!g_tcpc_dev) {
		pr_info("g_tcpc_dev is NULL\n");
		return TYPEC_ROLE_UNKNOWN;
	}

	return g_tcpc_dev->typec_role;
}

int tcpci_get_cc_status(void)
{
	if (!g_tcpc_dev || !g_tcpc_dev->ops ||
		!g_tcpc_dev->ops->get_cc_status) {
		pr_info("g_tcpc_dev tcpc or ops is null\n");
		return -1;
	}

	return g_tcpc_dev->ops->get_cc_status();
}

void tcpci_set_flags(struct tcpc_device *tcpc, uint32_t flags)
{
	if (!tcpc)
		return;

	tcpc->tcpc_flags = flags;
}

bool tcpci_is_support(struct tcpc_device *tcpc, uint32_t mask)
{
	if (!tcpc)
		return false;

	return tcpc->tcpc_flags & mask;
}

bool tcpci_is_pr_swaping(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return false;

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	return tcpc->pd_wait_pr_swap_complete;
#else
	return false;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
}

int tcpci_alert_status_clear(struct tcpc_device *tcpc, uint32_t mask)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->alert_status_clear)
		return 0;

	return tcpc->ops->alert_status_clear(tcpc->drv_data, mask);
}

int tcpci_fault_status_clear(struct tcpc_device *tcpc, uint8_t status)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->fault_status_clear)
		return 0;

	return tcpc->ops->fault_status_clear(tcpc->drv_data, status);
}

int tcpci_set_alert_mask(struct tcpc_device *tcpc, uint32_t mask)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_alert_mask)
		return 0;

	return tcpc->ops->set_alert_mask(tcpc->drv_data, mask);
}

int tcpci_get_alert_mask(struct tcpc_device *tcpc, uint32_t *mask)
{
	if (!mask)
		return -EINVAL;

	if (!tcpc || !tcpc->ops || !tcpc->ops->get_alert_mask)
		return -ENODEV;

	return tcpc->ops->get_alert_mask(tcpc->drv_data, mask);
}

int tcpci_get_alert_status(struct tcpc_device *tcpc, uint32_t *alert)
{
	if (!alert)
		return -EINVAL;

	if (!tcpc || !tcpc->ops || !tcpc->ops->get_alert_status)
		return -ENODEV;

	return tcpc->ops->get_alert_status(tcpc->drv_data, alert);
}

int tcpci_get_fault_status(struct tcpc_device *tcpc, uint8_t *fault)
{
	if (!fault)
		return -EINVAL;

	if (!tcpc || !tcpc->ops || !tcpc->ops->get_fault_status) {
		*fault = 0;
		return 0;
	}

	return tcpc->ops->get_fault_status(tcpc->drv_data, fault);
}

int tcpci_get_power_status(struct tcpc_device *tcpc, uint16_t *pw_status)
{
	if (!pw_status)
		return -EINVAL;

	if (!tcpc || !tcpc->ops || !tcpc->ops->get_power_status)
		return -ENODEV;

	return tcpc->ops->get_power_status(tcpc->drv_data, pw_status);
}

int tcpci_init(struct tcpc_device *tcpc, bool sw_reset)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->init)
		return -ENODEV;

	return tcpc->ops->init(tcpc->drv_data, sw_reset);
}

int tcpci_init_alert_mask(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->init_alert_mask)
		return -ENODEV;

	return tcpc->ops->init_alert_mask(tcpc->drv_data);
}

int tcpci_get_cc(struct tcpc_device *tcpc)
{
	int ret;
	int cc1, cc2;

	if (!tcpc || !tcpc->ops || !tcpc->ops->get_cc)
		return 0;

	ret = tcpc->ops->get_cc(tcpc->drv_data, &cc1, &cc2);
	if (ret < 0)
		return ret;

	if ((cc1 == tcpc->typec_remote_cc[0]) &&
		(cc2 == tcpc->typec_remote_cc[1])) {
		return 0;
	}

	tcpc->typec_remote_cc[0] = cc1;
	tcpc->typec_remote_cc[1] = cc2;
	return 1;
}

int tcpci_read_cc(struct tcpc_device *tcpc, uint8_t *cc1, uint8_t *cc2)
{
	int ret;

	if (!cc1 || !cc2)
		return -EINVAL;

	ret = tcpci_get_cc(tcpc);
	*cc1 = tcpc->typec_remote_cc[0];
	*cc2 = tcpc->typec_remote_cc[1];
	return ret;
}

int tcpci_set_cc(struct tcpc_device *tcpc, int pull)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_cc)
		return 0;

	if (pull & TYPEC_CC_DRP) {
		tcpc->typec_remote_cc[0] = TYPEC_CC_DRP_TOGGLING;
		tcpc->typec_remote_cc[1] = TYPEC_CC_DRP_TOGGLING;
	}

	if ((pull == TYPEC_CC_DRP) && tcpc->typec_legacy_cable) {
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
		if (tcpc->typec_legacy_cable == 2)
			pull = TYPEC_CC_RP;
		else if (tcpc->typec_legacy_retry_wk > 1)
			pull = TYPEC_CC_RP_3_0;
		else
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
			pull = TYPEC_CC_RP_1_5;
		TCPC_DBG2("LC->Toggling %d\n", pull);
	}

	if (tcpc->typec_local_cc != pull)
		TCPC_INFO("set local cc %d->%d\n", tcpc->typec_local_cc, pull);
	tcpc->typec_local_cc = pull;
	return tcpc->ops->set_cc(tcpc->drv_data, pull);
}

int tcpci_set_polarity(struct tcpc_device *tcpc, int polarity)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_polarity)
		return 0;

	if (polarity >= ARRAY_SIZE(tcpc->typec_remote_cc))
		return -EINVAL;

	return tcpc->ops->set_polarity(tcpc->drv_data, polarity,
		tcpc->typec_remote_cc[polarity]);
}

int tcpci_set_low_rp_duty(struct tcpc_device *tcpc, bool low_rp)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_low_rp_duty)
		return 0;

	if (low_rp)
		TCPC_INFO("low_rp_duty\n");

	return tcpc->ops->set_low_rp_duty(tcpc->drv_data, low_rp);
}

int tcpci_set_vconn(struct tcpc_device *tcpc, int enable)
{
#ifdef CONFIG_TCPC_SOURCE_VCONN
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_vconn)
		return 0;

	if (tcpc->tcpc_source_vconn == enable)
		return 0;

	tcpc->tcpc_source_vconn = enable;
	tcpm_notify_vconn(&tcpc->notifier, enable != 0);
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SOURCE_VCONN, &enable);
	return tcpc->ops->set_vconn(tcpc->drv_data, enable);
#else
	return 0;
#endif /* CONFIG_TCPC_SOURCE_VCONN */
}

int tcpci_is_low_power_mode(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->is_low_power_mode)
		return 1;

	return tcpc->ops->is_low_power_mode(tcpc->drv_data);
}

int tcpci_set_low_power_mode(struct tcpc_device *tcpc, bool en, int pull)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_low_power_mode)
		return 0;

	return tcpc->ops->set_low_power_mode(tcpc->drv_data, en, pull);
}

int tcpci_set_watchdog(struct tcpc_device *tcpc, bool en)
{
	int rv = 0;

	if (!tcpc || !tcpc->ops || !tcpc->ops->set_watchdog)
		return rv;

	if (tcpc->tcpc_flags & TCPC_FLAGS_WATCHDOG_EN)
		rv = tcpc->ops->set_watchdog(tcpc->drv_data, en);

	return rv;
}

int tcpci_alert_vendor_defined_handler(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->alert_vendor_defined_handler)
		return 0;

	return tcpc->ops->alert_vendor_defined_handler(tcpc->drv_data);
}

bool tcpci_is_vsafe0v(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->is_vsafe0v)
		return false;

	return tcpc->ops->is_vsafe0v(tcpc->drv_data);
}

int tcpci_set_vbus_cc_short_detection(struct tcpc_device *tcpc, bool cc1,
	bool cc2)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_vbus_cc_short_detection)
		return 0;

	return tcpc->ops->set_vbus_cc_short_detection(tcpc->drv_data, cc1, cc2);
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
int tcpci_set_msg_header(struct tcpc_device *tcpc, uint8_t power_role,
	uint8_t data_role)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_msg_header)
		return 0;

	return tcpc->ops->set_msg_header(tcpc->drv_data, power_role, data_role);
}

int tcpci_set_rx_enable(struct tcpc_device *tcpc, uint8_t enable)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_rx_enable)
		return 0;

	return tcpc->ops->set_rx_enable(tcpc->drv_data, enable);
}

int tcpci_protocol_reset(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->protocol_reset)
		return 0;

	return tcpc->ops->protocol_reset(tcpc->drv_data);
}

int tcpci_get_message(struct tcpc_device *tcpc, uint32_t *payload,
	uint16_t *head, enum tcpc_pd_transmit_type *type)
{
	if (!payload || !head || !type)
		return -EINVAL;

	if (!tcpc || !tcpc->ops || !tcpc->ops->get_message)
		return -ENODEV;

	return tcpc->ops->get_message(tcpc->drv_data, payload, head, type);
}

int tcpci_transmit(struct tcpc_device *tcpc, enum tcpc_pd_transmit_type type,
	uint16_t header, const uint32_t *data)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->transmit)
		return 0;

	return tcpc->ops->transmit(tcpc->drv_data, type, header, data,
		tcpc->pd_retry_count);
}

int tcpci_set_bist_test_mode(struct tcpc_device *tcpc, bool en)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_bist_test_mode)
		return 0;

	return tcpc->ops->set_bist_test_mode(tcpc->drv_data, en);
}

int tcpci_set_bist_carrier_mode(struct tcpc_device *tcpc, uint8_t pattern)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_bist_carrier_mode)
		return 0;

	if (pattern)
		/* 240ms */
		udelay(240);

	return tcpc->ops->set_bist_carrier_mode(tcpc->drv_data, pattern);
}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
int tcpci_retransmit(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->retransmit)
		return 0;

	return tcpc->ops->retransmit(tcpc->drv_data, tcpc->pd_retry_count);
}
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

int tcpci_set_intrst(struct tcpc_device *tcpc, bool en)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_intrst)
		return 0;

	return tcpc->ops->set_intrst(tcpc->drv_data, en);
}

int tcpci_enable_watchdog(struct tcpc_device *tcpc, bool en)
{
	if (!tcpc || !tcpc->ops || !tcpc->ops->set_watchdog)
		return 0;

	if (!(tcpc->tcpc_flags & TCPC_FLAGS_WATCHDOG_EN))
		return 0;

	TCPC_DBG2("enable_WG: %d\r\n", en);

	return tcpc->ops->set_watchdog(tcpc->drv_data, en);
}

int tcpci_enable_ext_discharge(struct tcpc_device *tcpc, bool en)
{
	mutex_lock(&tcpc->access_lock);

	if (tcpc->typec_ext_discharge != en) {
		tcpc->typec_ext_discharge = en;
		TCPC_DBG("EXT-Discharge: %d\r\n", en);
		tcpm_notify_vbus_ext_discharge(&tcpc->notifier, en);
	}

	mutex_unlock(&tcpc->access_lock);
	return 0;
}

void tcpci_handle_vbus_cc_short(struct tcpc_device *tcpc, bool vbus_cc1_short,
	bool vbus_cc2_short)
{
	TCPC_INFO("%s short cc1=%d,cc2=%d\n", __func__, vbus_cc1_short,
		  vbus_cc2_short);
	if ((tcpc->vbus_cc1_short == vbus_cc1_short) &&
	    (tcpc->vbus_cc2_short == vbus_cc2_short))
		return;

	tcpc->vbus_cc1_short = vbus_cc1_short;
	tcpc->vbus_cc2_short = vbus_cc2_short;
	tcpm_notify_vbus_cc_short(&tcpc->notifier, vbus_cc1_short,
		vbus_cc2_short);
}
