/*
 * dp_system_stubs.c
 *
 * dp_system_stubs driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifdef FSC_HAVE_DP

#include "../../platform.h"
#include "interface_dp.h"

#include <linux/hisi/usb/tca.h>
#include "../../core.h"
#include <huawei_platform/usb/hw_pd_dev.h>
#include <chipset_common/hwusb/hw_usb.h>

#ifdef CONFIG_CONTEXTHUB_PD
static TCPC_MUX_CTRL_TYPE g_mux_type = TCPC_DP;
extern int support_dp;
extern void dp_aux_switch_op(uint32_t value);
extern void dp_aux_uart_switch_enable(void);
extern int pd_dpm_handle_combphy_event(struct pd_dpm_combphy_event event);
extern void pd_dpm_set_last_hpd_status(bool hpd_status);
#endif /* CONFIG_CONTEXTHUB_PD */

#define MODE_DP_PIN_C 0x04
#define MODE_DP_PIN_D 0x08
#define MODE_DP_PIN_E 0x10
#define MODE_DP_PIN_F 0x20

/*
 * response for DP status update request
 * refer tcpci_report_hpd_state in tcpi.h
 */
void informStatus(DisplayPortStatus_t stat)
{
	int ret;
	struct pd_dpm_combphy_event event = { 0 };

	DpPpStatus.word = stat.word;

#ifdef CONFIG_CONTEXTHUB_PD
	if (!support_dp)
		return;

	if (DpPpConfig.word != 0) {
		event.dev_type = TCA_DP_IN;
		event.irq_type = TCA_IRQ_HPD_IN;
		event.mode_type = g_mux_type;
		event.typec_orien = core_get_cc_orientation();

		pr_info("%s + state = %d,irq = %d\n", __func__,
			stat.HpdState, stat.IrqHpd);

		if (!stat.HpdState) {
			event.dev_type = TCA_DP_OUT;
			event.irq_type = TCA_IRQ_HPD_OUT;
			ret = pd_dpm_handle_combphy_event(event);
			pd_dpm_set_last_hpd_status(false);
			hw_usb_send_event(DP_CABLE_OUT_EVENT);
		} else {
			event.dev_type = TCA_DP_IN;
			ret = pd_dpm_handle_combphy_event(event);
			pd_dpm_set_last_hpd_status(true);
			hw_usb_send_event(DP_CABLE_IN_EVENT);
		}

		if (stat.IrqHpd) {
			event.irq_type = TCA_IRQ_SHORT;
			ret = pd_dpm_handle_combphy_event(event);
		}

		pr_info("%s - ret = %d\n", __func__, ret);
	}
#endif /* CONFIG_CONTEXTHUB_PD */
}

/*
 * this function is called when a config message
 * either ACKd or NAKd by the other side
 */
void informConfigResult(FSC_BOOL success)
{
	FSC_U8 fsc_polarity;
	FSC_U32 pin_assignment = 0;
	int ret;
	struct pd_dpm_combphy_event event;

	if (success == FALSE)
		return;

	DpPpConfig.word = DpPpRequestedConfig.word;

#ifdef CONFIG_CONTEXTHUB_PD
	/* add aux switch */
	if (!support_dp)
		return;

	fsc_polarity = core_get_cc_orientation();
	dp_aux_switch_op(fsc_polarity);
	/* add aux uart switch */
	dp_aux_uart_switch_enable();

	switch (DpPpRequestedConfig.Conf) {
	case DP_CONF_UFP_D:
		/* pin_assignment mask */
		pin_assignment = DpPpRequestedConfig.DfpPa & 0xff;
		break;
	case DP_CONF_DFP_D:
		/* pin_assignment mask */
		pin_assignment = DpPpRequestedConfig.UfpPa & 0xff;
		break;
	default:
		break;
	}

	if (pin_assignment == MODE_DP_PIN_C || pin_assignment == MODE_DP_PIN_E)
		g_mux_type = TCPC_DP;
	else if (pin_assignment == MODE_DP_PIN_D ||
		pin_assignment == MODE_DP_PIN_F)
		g_mux_type = TCPC_USB31_AND_DP_2LINE;

	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	event.typec_orien = core_get_cc_orientation();

	ret = pd_dpm_handle_combphy_event(event);
	pd_dpm_set_combphy_status(g_mux_type);

	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = g_mux_type;
	event.typec_orien = core_get_cc_orientation();
	ret = pd_dpm_handle_combphy_event(event);

	if (DpPpStatus.HpdState)
		informStatus(DpPpStatus);

	pr_info("%s pd_event_notify ret = %d, mux_type = %d\n",
		__func__, ret, g_mux_type);
#endif /* CONFIG_CONTEXTHUB_PD */
}

/*
 * called with a DisplayPort configuration to do!
 * return TRUE if/when successful, FALSE otherwise
 */
FSC_BOOL DpReconfigure(DisplayPortConfig_t config)
{
	DpConfig.word = config.word;
	/* must actually change configurations here before returning TRUE */
	return TRUE;
}

#endif /* FSC_HAVE_DP */
