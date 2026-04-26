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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _TCPCI_H
#define _TCPCI_H

#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/err.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/usb/class-dual-role.h>

#include "include/pd_dbg_info.h"
#include "include/tcpci_core.h"
#include "include/tcpci_timer.h"

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#endif

#include "../hisi-usb-typec.h"

void hisi_tcpc_dual_role_instance_changed(
		struct dual_role_phy_instance *dual_role);
int hisi_tcpc_dual_role_phy_init(struct tcpc_device *tcpc);
int hisi_usb_typec_otg_pwr_src(void);
bool hisi_usb_typec_force_vconn(void);
void hisi_tcpc_poll_cc_status(struct tcpc_device *tcpc);
void hisi_tcpci_vbus_level_init(struct tcpc_device *tcpc,
		uint16_t power_status);
int hisi_tcpci_alert(struct tcpc_device *tcpc);
int hisi_tcpci_report_usb_port_changed(struct tcpc_device *tcpc);
int hisi_tcpci_report_power_control(struct tcpc_device *tcpc, bool en);
const char *pd_pe_state_name(uint8_t pe_state);

static inline int tcpci_check_vbus_valid(struct tcpc_device *tcpc)
{
	return tcpc->vbus_level >= TCPC_VBUS_VALID;
}

static inline int tcpci_check_vsafe0v(struct tcpc_device *tcpc, bool detect_en)
{
	return (tcpc->vbus_level == TCPC_VBUS_INVALID);
}

static inline void tcpci_alert_status_clear(struct tcpc_device *tcpc,
		uint32_t mask)
{
	(void)tcpc->ops->alert_status_clear(tcpc, mask);
}

static inline void tcpci_fault_status_clear(struct tcpc_device *tcpc,
		uint8_t status)
{
	if (tcpc->ops->fault_status_clear)
		(void)tcpc->ops->fault_status_clear(tcpc, status);
}

static inline int tcpci_get_alert_status(struct tcpc_device *tcpc,
		uint32_t *alert)
{
	return tcpc->ops->get_alert_status(tcpc, alert);
}

static inline int tcpci_get_fault_status(struct tcpc_device *tcpc,
		uint8_t *fault)
{
	if (tcpc->ops->get_fault_status)
		return tcpc->ops->get_fault_status(tcpc, fault);
	*fault = 0;
	return 0;
}

static inline int tcpci_get_power_status(struct tcpc_device *tcpc,
		uint16_t *pw_status)
{
	return tcpc->ops->get_power_status(tcpc, pw_status);
}

static inline void tcpci_set_frswap(struct tcpc_device *tcpc, bool en)
{
	if (tcpc->ops->set_fr_swap)
		tcpc->ops->set_fr_swap(tcpc, en);
}

static inline void tcpci_sc_buck_ctrl(struct tcpc_device *tcpc, bool enable)
{
	if (tcpc->ops->sc_buck_ctrl)
		tcpc->ops->sc_buck_ctrl(tcpc, enable);
}

#define TCPCI_SRC_VBUS_TIMEOUT_CNT 100
static inline void tcpci_wait_source_vbus(struct tcpc_device *tcpc, bool expect)
{
	uint16_t power_status = 0;
	int rv;
	int count = TCPCI_SRC_VBUS_TIMEOUT_CNT;

	while (count-- > 0) {
		rv = tcpci_get_power_status(tcpc, &power_status);
		if (rv < 0) {
			E("get power_status failed\n");
			continue;
		}

		if ((!!(power_status & TCPC_REG_POWER_STATUS_SRC_VBUS)) == expect)
			break;

		msleep(2); /* 2: msleep time */
	}
}

/*
 * 0: cc keeps the same
 * 1: cc changed
 */
static inline int tcpci_get_cc(struct tcpc_device *tcpc)
{
	int ret, cc1, cc2;

	ret = tcpc->ops->get_cc(tcpc, &cc1, &cc2);
	if (ret < 0)
		return ret;

	if (((uint8_t)(uint32_t)cc1 == tcpc->typec_remote_cc[0]) &&
		((uint8_t)(uint32_t)cc2 == tcpc->typec_remote_cc[1]))
		return 0;

	tcpc->typec_remote_cc[0] = (uint8_t)(uint32_t)cc1;
	tcpc->typec_remote_cc[1] = (uint8_t)(uint32_t)cc2;
	D("remote_cc1 0x%x, remote_cc2 0x%x\n",
			tcpc->typec_remote_cc[0], tcpc->typec_remote_cc[1]);

	return 1;
}

static inline void tcpci_set_cc(struct tcpc_device *tcpc, int pull)
{
	int ret;

	if ((unsigned int)pull & TYPEC_CC_DRP) {
		tcpc->typec_remote_cc[1] = TYPEC_CC_DRP_TOGGLING;
		tcpc->typec_remote_cc[0] = TYPEC_CC_DRP_TOGGLING;
	}

	I("old remote_cc1 0x%x, remote_cc2 0x%x\n",
			tcpc->typec_remote_cc[0], tcpc->typec_remote_cc[1]);

	tcpc->typec_local_cc = pull;
	ret = tcpc->ops->set_cc(tcpc, pull);
	if (ret)
		E("set_cc error ret %d\n", ret);
}

static inline void tcpci_snk_src_set_cc(struct tcpc_device *tcpc, int pull)
{
	if (tcpc->ops->set_polarity_cc)
		tcpc->ops->set_polarity_cc(tcpc);

	mdelay(5); /* 5: polarity CC stable */

	/* Both CC */
	tcpci_set_cc(tcpc, pull);
}

static inline bool tcpci_ccdebouce_timer_enabled(struct tcpc_device *tcpc)
{
	return hisi_tcpc_timer_enabled(tcpc, TYPEC_TIMER_CCDEBOUNCE);
}

static inline int tcpci_set_polarity(struct tcpc_device *tcpc, int polarity)
{
	return tcpc->ops->set_polarity(tcpc, polarity);
}

static inline int tcpci_set_vconn(struct tcpc_device *tcpc, int enable)
{
	struct tcp_notify tcp_noti;

	tcp_noti.en_state.en = enable != 0;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_SOURCE_VCONN, &tcp_noti);

	return tcpc->ops->set_vconn(tcpc, enable);
}

static inline void tcpci_reset_pd_fsm(struct tcpc_device *tcpc)
{
	if (tcpc->ops->reset_pd_fsm)
		tcpc->ops->reset_pd_fsm(tcpc);
}

static inline bool tcpci_tcpc_chip_version_v610(struct tcpc_device *tcpc)
{
	if (tcpc->ops->chip_version_v610)
		return tcpc->ops->chip_version_v610(tcpc);

	return false;
}

static inline void tcpci_set_vbus_detect(struct tcpc_device *tcpc, bool enable)
{
	if (tcpc->ops->set_vbus_detect)
		tcpc->ops->set_vbus_detect(tcpc, enable);
	tcpc->vbus_detect = enable;
}

static inline void tcpci_snk_unattach_by_vbus_bypass(struct tcpc_device *tcpc,
		bool enable)
{
	if (tcpc->ops->snk_unattach_by_vbus_bypass)
		tcpc->ops->snk_unattach_by_vbus_bypass(tcpc, enable);
}

static inline void tcpci_tcpc_print_pd_fsm_state(struct tcpc_device *tcpc)
{
	if (tcpc->ops->tcpc_pd_fsm_state)
		tcpc->ops->tcpc_pd_fsm_state(tcpc);
}

static inline int tcpci_tcpc_cc_is_realdetach(struct tcpc_device *tcpc)
{
	if (tcpc->ops->tcpc_cc_is_realdetach)
		return tcpc->ops->tcpc_cc_is_realdetach(tcpc);

	return 0;
}

static inline bool tcpci_tcpc_vusb_uv_det_masked(struct tcpc_device *tcpc)
{
	if (tcpc->ops->vusb_uv_det_mask)
		return tcpc->ops->vusb_uv_det_mask();

	return false;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static inline int tcpci_set_msg_header(
		struct tcpc_device *tcpc, int power_role, int data_role)
{
#ifdef CONFIG_USB_PD_REV30
	uint8_t pd_rev = tcpc->pd_port.pd_revision[0];
#else
	uint8_t pd_rev = PD_REV20;
#endif
	return tcpc->ops->set_msg_header(tcpc, power_role, data_role, pd_rev);
}

static inline int tcpci_set_rx_enable(struct tcpc_device *tcpc, uint8_t enable)
{
	return tcpc->ops->set_rx_enable(tcpc, enable);
}

static inline int tcpci_get_message(struct tcpc_device *tcpc, uint32_t *payload,
		uint16_t *head, enum tcpm_transmit_type *type)
{
	return tcpc->ops->get_message(tcpc, payload, head, type);
}

static inline int tcpci_transmit(struct tcpc_device *tcpc,
		enum tcpm_transmit_type type, uint16_t header,
		const uint32_t *data)
{
	return tcpc->ops->transmit(tcpc, type, header, data);
}

static inline int tcpci_set_bist_test_mode(struct tcpc_device *tcpc, bool en)
{
	return tcpc->ops->set_bist_test_mode(tcpc, en);
}

static inline int tcpci_set_bist_carrier_mode(struct tcpc_device *tcpc,
		uint8_t pattern)
{
	if (pattern)
		udelay(240); /* wait for GoodCRC */

	return 0;
}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
static inline int tcpci_retransmit(struct tcpc_device *tcpc)
{
	return tcpc->ops->retransmit(tcpc);
}
#endif
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

static inline int tcpci_notify_typec_state(struct tcpc_device *tcpc)
{
	struct tcp_notify tcp_noti;

	tcp_noti.typec_state.rp_level = tcpc->typec_remote_rp_level;
	tcp_noti.typec_state.polarity = tcpc->typec_polarity;
	tcp_noti.typec_state.old_state = tcpc->typec_attach_old;
	tcp_noti.typec_state.new_state = tcpc->typec_attach_new;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_TYPEC_STATE, &tcp_noti);

	return 0;
}

static inline int tcpci_notify_role_swap(struct tcpc_device *tcpc,
		uint8_t event, uint8_t role)
{
	struct tcp_notify tcp_noti;

	D("event %u, role %u\n", event, role);
	tcp_noti.swap_state.new_role = role;
	srcu_notifier_call_chain(&tcpc->evt_nh, event, &tcp_noti);

	return 0;
}

static inline int tcpci_source_vbus(struct tcpc_device *tcpc,
		uint8_t type, int mv, int ma)
{
	struct tcp_notify tcp_noti;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (type >= TCP_VBUS_CTRL_PD && tcpc->pd_port.pd_prev_connected)
		type |= TCP_VBUS_CTRL_PD_DETECT;
#endif

	if (ma < 0) {
		if (mv != 0) {
			switch (tcpc->typec_local_rp_level) {
			case TYPEC_CC_RP_1_5:
				ma = TYPEC_CC_PWR_CUR_1_5;
				break;

			case TYPEC_CC_RP_3_0:
				ma = TYPEC_CC_PWR_CUR_3_0;
				break;

			case TYPEC_CC_RP_DFT:
			default:
				ma = CONFIG_TYPEC_SRC_CURR_DFT;
				break;
			}
		} else {
			ma = 0;
		}
	}

	tcp_noti.vbus_state.type = type;
	tcp_noti.vbus_state.mv = mv;
	tcp_noti.vbus_state.ma = ma;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_SOURCE_VBUS, &tcp_noti);

	if (tcpc->ops->source_vbus)
		tcpc->ops->source_vbus(tcpc, type, mv, ma);

	return 0;
}

static inline int tcpci_sink_vbus(struct tcpc_device *tcpc,
		uint8_t type, int mv, int ma)
{
	struct tcp_notify tcp_noti;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (type >= TCP_VBUS_CTRL_PD && tcpc->pd_port.pd_prev_connected) {
		type |= TCP_VBUS_CTRL_PD_DETECT;
		tcp_noti.vbus_state.ext_power =
			tcpc->pd_port.dpm_flags & DPM_FLAGS_PARTNER_EXTPOWER;
	}
#endif

	if (ma < 0) {
		if (mv != 0) {
			switch (tcpc->typec_remote_rp_level) {
			case TYPEC_CC_VOLT_SNK_1_5:
				ma = TYPEC_CC_PWR_CUR_1_5;
				break;
			case TYPEC_CC_VOLT_SNK_3_0:
				ma = TYPEC_CC_PWR_CUR_3_0;
				break;
			case TYPEC_CC_VOLT_SNK_DFT:
			default:
				ma = tcpc->typec_usb_sink_curr;
				break;
			}
#if CONFIG_TYPEC_SNK_CURR_LIMIT > 0
			if (ma > CONFIG_TYPEC_SNK_CURR_LIMIT)
				ma = CONFIG_TYPEC_SNK_CURR_LIMIT;
#endif
		} else
			ma = 0;
	}

	tcp_noti.vbus_state.type = type;
	tcp_noti.vbus_state.mv = mv;
	tcp_noti.vbus_state.ma = ma;
	tcp_noti.vbus_state.remote_rp_level = tcpc->typec_remote_rp_level;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_SINK_VBUS, &tcp_noti);

	if (tcpc->ops->sink_vbus)
		tcpc->ops->sink_vbus(tcpc, type, mv, ma);

	return 0;
}

static inline int tcpci_disable_vbus_control(struct tcpc_device *tcpc)
{
	struct tcp_notify tcp_noti;

	tcp_noti.vbus_state.type = TCP_VBUS_CTRL_REMOVE;
	tcp_noti.vbus_state.mv = 0;
	tcp_noti.vbus_state.ma = 0;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_DIS_VBUS_CTRL, &tcp_noti);

	return 0;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static inline int tcpci_notify_pd_state(struct tcpc_device *tcpc, uint8_t connect)
{
	struct tcp_notify tcp_noti;

	tcp_noti.pd_state.connected = connect;
	srcu_notifier_call_chain(&tcpc->evt_nh, TCP_NOTIFY_PD_STATE, &tcp_noti);

	return 0;
}

static inline void tcpci_notify_cable_vdo(struct tcpc_device *tcpc,
		struct tcp_ny_cable_vdo cable_vdo)
{
	struct tcp_notify tcp_noti;

	tcp_noti.cable_vdo.vdo = cable_vdo.vdo;
	tcp_noti.cable_vdo.vdo_ext = cable_vdo.vdo_ext;
	srcu_notifier_call_chain(&tcpc->evt_nh, TCP_NOTIFY_CABLE_VDO, &tcp_noti);
}

static inline int tcpci_enter_mode(struct tcpc_device *tcpc,
		uint16_t svid, uint8_t ops, uint32_t mode)
{
	struct tcp_notify tcp_noti;

	/* DFP_U : DisplayPort Mode, USB Configuration */
	D("svid 0x%x, ops 0x%x, mode 0x%x\n", svid, ops, mode);

	tcp_noti.mode_ctrl.svid = svid;
	tcp_noti.mode_ctrl.ops = ops;
	tcp_noti.mode_ctrl.mode = mode;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_ENTER_MODE, &tcp_noti);
	return 0;
}

static inline int tcpci_exit_mode(struct tcpc_device *tcpc, uint16_t svid)
{
	struct tcp_notify tcp_noti;

	D("ExitMode\n");

	tcp_noti.mode_ctrl.svid = svid;
	tcp_noti.mode_ctrl.ops = 0;
	tcp_noti.mode_ctrl.mode = 0;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_EXIT_MODE, &tcp_noti);
	return 0;
}

#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
static inline int tcpci_report_hpd_state(struct tcpc_device *tcpc,
		uint32_t dp_status)
{
	struct tcp_notify tcp_noti;
	int ret = 0;

	/* UFP_D to DFP_D only */
	D("dp_status 0x%x\n", dp_status);
	if (PD_DP_CFG_DFP_D(tcpc->pd_port.local_dp_config)) {
		tcp_noti.ama_dp_hpd_state.irq =
				PD_VDO_DPSTS_HPD_IRQ(dp_status);
		tcp_noti.ama_dp_hpd_state.state =
				PD_VDO_DPSTS_HPD_LVL(dp_status);
		DP_INFO("+++ hpd_state: %d +++\n\n",
				tcp_noti.ama_dp_hpd_state.state);
		ret = srcu_notifier_call_chain(&tcpc->evt_nh,
				TCP_NOTIFY_AMA_DP_HPD_STATE, &tcp_noti);

		hisi_usb_pd_dp_hpd_state_change(&tcp_noti.ama_dp_hpd_state);
	}

	return ret;
}

static inline int tcpci_dp_status_update(struct tcpc_device *tcpc,
			uint32_t dp_status)
{
	tcpci_report_hpd_state(tcpc, dp_status);
	return 0;
}

static inline int tcpci_dp_configure(struct tcpc_device *tcpc,
		uint32_t dp_config)
{
	struct tcp_notify tcp_noti;
	int ret;

	DP_INFO("LocalCFG: 0x%x\n", dp_config);

	switch (dp_config & 0x03) {
	case 0:
		tcp_noti.ama_dp_state.sel_config = SW_USB;
		break;
	case MODE_DP_SRC:
		tcp_noti.ama_dp_state.sel_config = SW_UFP_D;
		tcp_noti.ama_dp_state.pin_assignment = (dp_config >> 16) & 0xff;
		break;
	case MODE_DP_SNK:
		tcp_noti.ama_dp_state.sel_config = SW_DFP_D;
		tcp_noti.ama_dp_state.pin_assignment = (dp_config >> 8) & 0xff;
		break;
	default:
		break;
	}

	tcp_noti.ama_dp_state.signal = (dp_config >> 2) & 0x0f;
	tcp_noti.ama_dp_state.polarity = tcpc->typec_polarity;
	tcp_noti.ama_dp_state.active = 1;
	ret = srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_AMA_DP_STATE, &tcp_noti);

	hisi_usb_pd_dp_state_change(&tcp_noti.ama_dp_state);

	return ret;
}

static inline int tcpci_dp_attention(struct tcpc_device *tcpc,
		uint32_t dp_status)
{
	struct tcp_notify tcp_noti;

	tcp_noti.ama_dp_attention.state = (uint8_t) dp_status;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_AMA_DP_ATTENTION, &tcp_noti);
	return tcpci_report_hpd_state(tcpc, dp_status);
}

static inline int tcpci_dp_notify_status_update_done(struct tcpc_device *tcpc,
		uint32_t dp_status, bool ack)
{
	return 0;
}

static inline int tcpci_dp_notify_config_start(struct tcpc_device *tcpc)
{
	/* DFP_U : Put signal & mux into the Safe State */
	struct tcp_notify tcp_noti;

	DP_INFO("ConfigStart\n");
	tcp_noti.ama_dp_state.sel_config = SW_USB;
	tcp_noti.ama_dp_state.active = 0;
	srcu_notifier_call_chain(&tcpc->evt_nh,
			TCP_NOTIFY_AMA_DP_STATE, &tcp_noti);
	return 0;
}

static inline int tcpci_dp_notify_config_done(struct tcpc_device *tcpc,
		uint32_t local_cfg, uint32_t remote_cfg, bool ack)
{
	if (ack)
		tcpci_dp_configure(tcpc, local_cfg);

	return 0;
}
#endif /* CONFIG_USB_PD_ALT_MODE_SUPPORT */

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
static inline int tcpci_notify_uvdm(struct tcpc_device *tcpc, bool ack)
{
	struct tcp_notify tcp_noti;
	pd_port_t *pd_port = &tcpc->pd_port;

	tcp_noti.uvdm_msg.ack = ack;

	if (ack) {
		tcp_noti.uvdm_msg.uvdm_cnt = pd_port->uvdm_cnt;
		tcp_noti.uvdm_msg.uvdm_svid = pd_port->uvdm_svid;
		tcp_noti.uvdm_msg.uvdm_data = pd_port->uvdm_data;
	}

	srcu_notifier_call_chain(&tcpc->evt_nh, TCP_NOTIFY_UVDM, &tcp_noti);

	return 0;
}
#endif

#ifdef CONFIG_USB_PD_REV30
#ifdef CONFIG_USB_PD_REV30_ALERT_REMOTE
static inline int tcpci_notify_alert(struct tcpc_device *tcpc, uint32_t ado)
{
	struct tcp_notify tcp_noti;

	tcp_noti.alert_msg.ado = ado;
	srcu_notifier_call_chain(&tcpc->evt_nh, TCP_NOTIFY_ALERT, &tcp_noti);
	return 0;
}
#endif
#endif /* CONFIG_USB_PD_REV30 */
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

#endif
