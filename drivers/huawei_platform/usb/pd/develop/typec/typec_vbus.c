/*
 * typec_vbus.c
 *
 * typec vbus
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

#include "inc/typec_vbus.h"
#include "inc/tcpci.h"
#include "inc/tcpc_core.h"
#include "typec_lpm.h"

int typec_refresh_power_status(struct tcpc_device *tcpc)
{
	int ret;
	uint16_t vbus_present;
	uint16_t status = 0;

	if (!tcpc)
		return -EINVAL;

	ret = tcpci_get_power_status(tcpc, &status);
	if (ret < 0)
		return ret;

	mutex_lock(&tcpc->access_lock);
	vbus_present = (status & TCPC_REG_POWER_STATUS_VBUS_PRES) ?
		true : false;
	tcpc->vbus_level = vbus_present ? TCPC_VBUS_VALID :
		TCPC_VBUS_INVALID;

	if (tcpci_is_support(tcpc, TCPC_FLAGS_VSAFE0V_DETECT)) {
		if (tcpci_is_vsafe0v(tcpc) &&
			(tcpc->vbus_level == TCPC_VBUS_INVALID))
			tcpc->vbus_level = TCPC_VBUS_SAFE0V;
	}

	mutex_unlock(&tcpc->access_lock);
	return 0;
}

int typec_inquire_vbus_level(struct tcpc_device *tcpc, bool from_ic)
{
	int rv;

	if (!tcpc)
		return -EINVAL;

	if (from_ic) {
		rv = typec_refresh_power_status(tcpc);
		if (rv < 0)
			return rv;
	}

	return tcpc->vbus_level;
}

static void typec_report_power_control_on(struct tcpc_device *tcpc)
{
	typec_set_wake_lock_pd(tcpc->lpm, true);
	tcpci_enable_ext_discharge(tcpc, false);
	timer_disable(tcpc, TYPEC_RT_TIMER_AUTO_DISCHARGE);
}

static void typec_report_power_control_off(struct tcpc_device *tcpc)
{
	tcpci_enable_ext_discharge(tcpc, true);
	timer_enable(tcpc, TYPEC_RT_TIMER_AUTO_DISCHARGE);
	typec_set_wake_lock_pd(tcpc->lpm, false);
}

static int typec_report_power_control(struct tcpc_device *tcpc, bool en)
{
	if (!tcpc)
		return -EINVAL;

	if (tcpc->typec_power_ctrl == en)
		return 0;

	tcpc->typec_power_ctrl = en;
	if (en)
		typec_report_power_control_on(tcpc);
	else
		typec_report_power_control_off(tcpc);

	return 0;
}

static int typec_get_official_ma(struct tcpc_device *tcpc, int mv, int ma)
{
	if (ma >= 0)
		return ma;

	if (mv == 0)
		return 0;

	switch (tcpc->typec_local_rp_level) {
	case TYPEC_CC_RP_1_5:
		return 1500; /* 1.5A */
	case TYPEC_CC_RP_3_0:
		return 3000; /* 3.0A */
	case TYPEC_CC_RP_DFT:
	default:
		return CONFIG_TYPEC_SRC_CURR_DFT;
	}
}

int typec_source_vbus(struct tcpc_device *tcpc, uint8_t type, int mv, int ma)
{
	if (!tcpc)
		return -EINVAL;

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	if ((type >= TCP_VBUS_CTRL_PD) &&
		tcpc->pd_port.pe_data.pd_prev_connected)
		type |= TCP_VBUS_CTRL_PD_DETECT;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	ma = typec_get_official_ma(tcpc, mv, ma);

	typec_set_watchdog(tcpc->lpm, mv != 0);
	TYPEC_DBG("source_vbus: %dmV, %dmA\n", mv, ma);

	tcpm_notify_source_vbus_state(&tcpc->notifier, type, mv, ma);
	typec_report_power_control(tcpc, mv > 0);
	return 0;
}

int typec_sink_vbus(struct tcpc_device *tcpc, uint8_t type, int mv, int ma)
{
	if (!tcpc)
		return -EINVAL;

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	if ((type >= TCP_VBUS_CTRL_PD) &&
		tcpc->pd_port.pe_data.pd_prev_connected)
		type |= TCP_VBUS_CTRL_PD_DETECT;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	ma = typec_get_official_ma(tcpc, mv, ma);

	TYPEC_DBG("sink_vbus: %dmV, %dmA\n", mv, ma);
	tcpm_notify_sink_vbus_state(&tcpc->notifier, type, mv, ma);
	typec_report_power_control(tcpc, mv > 0);
	return 0;
}

int typec_disable_vbus_control(struct tcpc_device *tcpc)
{
	int ret;

	if (!tcpc)
		return -EINVAL;

	if (!tcpc->typec_power_ctrl)
		return 0;

	ret = typec_sink_vbus(tcpc, TCP_VBUS_CTRL_REMOVE,
		TCPC_VBUS_SINK_0V, 0);
	ret += typec_source_vbus(tcpc, TCP_VBUS_CTRL_REMOVE,
		TCPC_VBUS_SOURCE_0V, 0);
	return ret;
}

int typec_handle_auto_discharge_timeout(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return -EINVAL;

	if (tcpc->typec_power_ctrl)
		return 0;

	return tcpci_enable_ext_discharge(tcpc, false);
}

bool typec_check_vbus_valid(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return false;

	return tcpc->vbus_level >= TCPC_VBUS_VALID;
}

bool typec_check_vbus_valid_from_ic(struct tcpc_device *tcpc)
{
	int vbus_level;

	if (!tcpc)
		return false;

	vbus_level = tcpc->vbus_level;
	typec_refresh_power_status(tcpc);
	if (vbus_level != tcpc->vbus_level)
		TYPEC_INFO("[Warning] ps_chagned %d ->%d\n", vbus_level,
			tcpc->vbus_level);

	return typec_check_vbus_valid(tcpc);
}

bool typec_check_vsafe0v(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return false;

	if (!tcpci_is_support(tcpc, TCPC_FLAGS_VSAFE0V_DETECT))
		return tcpc->vbus_level == TCPC_VBUS_INVALID;

	return tcpc->vbus_level == TCPC_VBUS_SAFE0V;
}