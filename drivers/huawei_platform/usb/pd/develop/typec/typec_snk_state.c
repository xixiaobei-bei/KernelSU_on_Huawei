
/*
 * typec_snk_state.c
 *
 * typec snk state
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "typec_state.h"
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/common_module/power_platform.h>
#include "inc/tcpc_core.h"
#include "inc/timer.h"
#include "inc/typec_vbus.h"
#include "typec_common.h"
#include "typec_legacy_cable.h"

static int typecs_get_rp_present_flag(struct tcpc_device *tcpc)
{
	uint8_t rp_flag = 0;

	if ((typec_get_cc1() >= TYPEC_CC_VOLT_SNK_DFT) &&
		(typec_get_cc1() <= TYPEC_CC_VOLT_SNK_3_0))
		rp_flag |= 1; /* cc1 flag mask */

	if ((typec_get_cc2() >= TYPEC_CC_VOLT_SNK_DFT) &&
		(typec_get_cc2() <= TYPEC_CC_VOLT_SNK_3_0))
		rp_flag |= 2; /* cc1 flag mask */

	return rp_flag;
}

static void typecs_snk_to_attach(struct tcpc_device *tcpc)
{
	/* Both Rp */
	if (!typec_check_cc_any(TYPEC_CC_VOLT_OPEN))
		typecs_next_state(tcpc, typec_attached_custom_src);
	else if ((typec_get_role() == TYPEC_ROLE_TRY_SRC) &&
		(tcpc->fsm->cur_state == typec_attachwait_snk))
		typecs_next_state(tcpc, typec_try_src);
	else
		typecs_next_state(tcpc, typec_attached_snk);
}

static void typecs_snk_try_attach(struct tcpc_device *tcpc)
{
	/* If Port Partner act as Source without VBUS, wait vSafe5V */
	if (!typec_check_vbus_valid(tcpc)) {
		typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_SNK_VSAFE5V);
		return;
	}

	typecs_snk_to_attach(tcpc);
}

static bool typecs_snk_is_cc_attach(struct tcpc_device *tcpc)
{
	if (!typecs_is_act_as_sink_role(tcpc))
		return false;

	if (typec_check_cc_any(TYPEC_CC_VOLT_SNK_DFT) ||
		typec_check_cc_any(TYPEC_CC_VOLT_SNK_1_5) ||
		typec_check_cc_any(TYPEC_CC_VOLT_SNK_3_0))
		return true;

	return false;
}

static void typecs_snk_detach(struct tcpc_device *tcpc)
{
	if (pd_is_pe_running(tcpc)) {
		typecs_next_state(tcpc, typec_unattachwait_pe);
		return;
	}

	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
}

static int typecs_unattached_snk_entry(struct tcpc_device *tcpc)
{
	int unattached_cc = typecs_get_unattached_cc(tcpc);

	tcpc->fsm->sink_curr = CONFIG_TYPEC_SNK_CURR_DFT;
	tcpci_set_vconn(tcpc, false);
	typec_disable_vbus_control(tcpc);
	typecs_set_cc_short_detection(tcpc, false, false);

	if (tcpc->fsm->init_with_charge) {
		tcpc->fsm->init_with_charge = false;
		typecs_enable_low_power(tcpc, TYPEC_CC_DRP);
		/* sleep 1s */
		usleep_range(1000, 2000);
		if (typec_is_cc_open())
			timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
		else
			tcpci_set_cc(tcpc, TYPEC_CC_RD);
	} else {
		typecs_enable_low_power(tcpc, unattached_cc);
		TYPEC_DBG("%s: unattached_cc=%d\n", __func__, unattached_cc);
	}

	typecs_attach_state_change(tcpc, TYPEC_UNATTACHED);
	return 0;
}

static int typecs_unattached_snk_cc_change(struct tcpc_device *tcpc)
{
	bool as_sink = true;

	if (!typecs_is_cc_attach(tcpc))
		return 0;

	as_sink = typecs_is_act_as_sink_role(tcpc);
	if (!as_sink && typec_legacy_check_cable(tcpc))
		return 0;

	if (as_sink)
		typecs_next_state(tcpc, typec_attachwait_snk);
	else
		typecs_next_state(tcpc, typec_attachwait_src);

	return 0;
}

static int typecs_unattached_snk_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc) && typec_is_cc_no_res()) {
		TYPEC_INFO("%s: norp src detect\n", __func__);
		timer_enable(tcpc, TYPEC_TIMER_NORP_SRC);
	}

	return 0;
}

static bool typecs_try_enter_norp_src(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc) && typec_is_cc_no_res()) {
		typecs_next_state(tcpc, typec_attached_norp_src);
		return true;
	}

	return false;
}

static int typecs_unattached_snk_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TIMER_NORP_SRC:
		typecs_try_enter_norp_src(tcpc);
		break;
	case TYPEC_TIMER_PDDEBOUNCE:
		if (!typecs_try_enter_norp_src(tcpc))
			typecs_unattached_snk_cc_change(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_attachwait_snk_entry(struct tcpc_device *tcpc)
{
	tcpc->fsm->rp_present = typecs_get_rp_present_flag(tcpc);
	tcpm_notify_sink_vbus_wait(&tcpc->notifier);
	timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);

	return 0;
}

static int typecs_attachwait_snk_cc_change(struct tcpc_device *tcpc)
{
	uint8_t rp_present = typecs_get_rp_present_flag(tcpc);

	if (rp_present == tcpc->fsm->rp_present) {
		TYPEC_DBG("%s: ignore rplvl alert\n", __func__);
		return 0;
	}

	if (typecs_snk_is_cc_attach(tcpc))
		timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);
	else
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);

	return 0;
}

static void typecs_attachwait_snk_stop(struct tcpc_device *tcpc)
{
	typecs_unattached_cc(tcpc, TYPEC_CC_RP);
	typecs_next_state(tcpc, typec_unattached_src);
}

static int typecs_attachwait_snk_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc) &&
		typecs_check_wait_ps(tcpc, TYPECS_WAIT_PS_SNK_VSAFE5V))
		typecs_snk_to_attach(tcpc);

	return 0;
}

static int typecs_attachwait_snk_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		if (typecs_snk_is_cc_attach(tcpc))
			typecs_snk_try_attach(tcpc);
		else
			typecs_attachwait_snk_stop(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_try_snk_entry(struct tcpc_device *tcpc)
{
	tcpci_set_cc(tcpc, TYPEC_CC_RD);
	typecs_in_drp_try(tcpc);
	timer_enable(tcpc, TYPEC_TRY_TIMER_DRP_TRY);

	return 0;
}

static int typecs_try_snk_cc_change(struct tcpc_device *tcpc)
{
	if (typecs_is_drp_try(tcpc)) {
		TYPEC_DBG("[Try.SNK] in drp try ignore cc alert\n");
		return 0;
	}

	if (typecs_is_cc_attach(tcpc)) {
		tcpm_notify_sink_vbus_wait(&tcpc->notifier);
		timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
	} else {
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	}

	return 0;
}

static int typecs_try_snk_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc) &&
		typecs_check_wait_ps(tcpc, TYPECS_WAIT_PS_SNK_VSAFE5V))
		typecs_snk_to_attach(tcpc);

	return 0;
}

static void typecs_try_snk_drp_timeout(struct tcpc_device *tcpc)
{
	typecs_out_drp_try(tcpc);

	if (typec_is_drp_toggling()) {
		TYPEC_DBG("%s: Warning DRP Toggling\n", __func__);
		return;
	}

	if (!typec_is_cc_open())
		tcpm_notify_sink_vbus_wait(&tcpc->notifier);

	timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
}

static void typecs_try_snk_debounce_timeout(struct tcpc_device *tcpc)
{
	if (!typecs_snk_is_cc_attach(tcpc)) {
		typecs_next_state(tcpc, typec_trywait_src);
		return;
	}

	typecs_snk_try_attach(tcpc);
}

static int typecs_try_snk_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TRY_TIMER_DRP_TRY:
		typecs_try_snk_drp_timeout(tcpc);
		break;
	case TYPEC_TIMER_TRYCCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		typecs_try_snk_debounce_timeout(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_trywait_snk_entry(struct tcpc_device *tcpc)
{
	tcpci_set_vconn(tcpc, false);
	tcpci_set_cc(tcpc, TYPEC_CC_RD);
	typec_source_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_0V, 0);

	return 0;
}

static int typecs_trywait_snk_cc_change(struct tcpc_device *tcpc)
{
	if (typecs_is_cc_attach(tcpc))
		timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);
	else
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);

	return 0;
}

static int typecs_trywait_snk_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc) &&
		typecs_check_wait_ps(tcpc, TYPECS_WAIT_PS_SNK_VSAFE5V))
		typecs_snk_to_attach(tcpc);

	return 0;
}

static int typecs_trywait_snk_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:;
		if (typecs_snk_is_cc_attach(tcpc))
			typecs_snk_try_attach(tcpc);
		else
			typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
		break;
	default:
		break;
	}

	return 0;
}

static void typecs_to_wait_pe_idle(struct tcpc_device *tcpc)
{
	typecs_attach_state_change(tcpc, TYPEC_UNATTACHED);

	tcpc->fsm->wait_pe_idle = true;
	if (tcpc->pd_inited_flag)
		timer_enable(tcpc, TYPEC_RT_TIMER_PE_IDLE);
}

static int typecs_trywait_snk_pe_entry(struct tcpc_device *tcpc)
{
	typecs_to_wait_pe_idle(tcpc);
	return 0;
}

static int typecs_trywait_snk_pe_cc_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_trywait_snk_pe_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_trywait_snk_pe_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id != TYPEC_RT_TIMER_PE_IDLE)
		return 0;

	tcpc->fsm->wait_pe_idle = false;
	typecs_next_state(tcpc, typec_trywait_snk);
	return 0;
}

static int typecs_unattachwait_pe_entry(struct tcpc_device *tcpc)
{
	typecs_to_wait_pe_idle(tcpc);
	return 0;
}

static int typecs_unattachwait_pe_cc_change(struct tcpc_device *tcpc)
{
	if (!typecs_is_cc_attach(tcpc))
		return 0;

	TYPEC_INFO("%s: force pe idle\n", __func__);
	tcpc->fsm->wait_pe_idle = false;
	timer_disable(tcpc, TYPEC_RT_TIMER_PE_IDLE);
	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));

	return 0;
}

static int typecs_unattachwait_pe_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_unattachwait_pe_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id != TYPEC_RT_TIMER_PE_IDLE)
		return 0;

	tcpc->fsm->wait_pe_idle = false;
	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
	return 0;
}

static void typecs_to_open_state(struct tcpc_device *tcpc)
{
	tcpci_set_cc(tcpc, TYPEC_CC_OPEN);
	typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_DISABLE);
	tcpci_set_vconn(tcpc, false);
	typec_disable_vbus_control(tcpc);
}

static int typecs_disable_entry(struct tcpc_device *tcpc)
{
	typecs_to_open_state(tcpc);
	timer_enable(tcpc, TYPEC_RT_TIMER_STATE_CHANGE);

	return 0;
}

static int typecs_disable_cc_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_disable_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_disable_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id == TYPEC_RT_TIMER_STATE_CHANGE)
		typecs_attach_state_change(tcpc, TYPEC_UNATTACHED);

	return 0;
}

static int typecs_error_recovery_entry(struct tcpc_device *tcpc)
{
	typecs_to_open_state(tcpc);
	timer_enable(tcpc, TYPEC_TIMER_ERROR_RECOVERY);

	return 0;
}

static int typecs_error_recovery_cc_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_error_recovery_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_error_recovery_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id != TYPEC_TIMER_ERROR_RECOVERY)
		return 0;

	typecs_snk_detach(tcpc);
	return 0;
}

static int typecs_role_swap_snk_entry(struct tcpc_device *tcpc)
{
	tcpci_set_cc(tcpc, TYPEC_CC_OPEN);
	timer_enable(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_START);
	return 0;
}

static int typecs_role_swap_snk_cc_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_role_swap_snk_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_role_swap_snk_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_RT_TIMER_ROLE_SWAP_START:
		tcpci_set_cc(tcpc, TYPEC_CC_RP);
		timer_enable(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
		break;
	case TYPEC_RT_TIMER_ROLE_SWAP_STOP:
		typecs_unattached_cc(tcpc, TYPEC_CC_RP);
		typecs_next_state(tcpc, typec_unattached_src);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_attached_snk_entry(struct tcpc_device *tcpc)
{
	tcpc->typec_remote_rp_level = typec_get_cc_res();

	typecs_set_plug_orient(tcpc, TYPEC_CC_RD,
		!typec_check_cc2(TYPEC_CC_VOLT_OPEN));
	typecs_set_cc_short_detection(tcpc, typec_get_polarity() == 0,
		typec_get_polarity() == 1);
	typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
	typecs_attach_state_change(tcpc, TYPEC_ATTACHED_SNK);

	return 0;
}

static void typecs_attached_snk_cc_detach(struct tcpc_device *tcpc)
{
	timer_reset_typec_debt(tcpc);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	/*
	 * For Source detach during HardReset,
	 * However Apple TA may keep cc_open about 150 ms during HardReset
	 */
	if (tcpc->pd_wait_hard_reset_complete) {
#ifdef CONFIG_COMPATIBLE_APPLE_TA
		TYPEC_INFO("detach cc in hardreset, compatible apple ta\n");
		timer_enable(tcpc, TYPEC_TIMER_APPLE_CC_OPEN);
#else
		TYPEC_INFO("detach cc in hardreset\n");
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
#endif /* CONFIG_COMPATIBLE_APPLE_TA */
	} else if (tcpc->pd_port.pe_data.pd_prev_connected) {
		TYPEC_INFO("detach cc of pd\n");
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
}

static int typecs_attached_snk_cc_change(struct tcpc_device *tcpc)
{
	if (typecs_snk_is_cc_attach(tcpc)) {
#ifdef CONFIG_PD_WAIT_BC12
		tcpc->wait_bc12_cnt = 0;
#endif /* CONFIG_PD_WAIT_BC12 */
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
		TYPEC_DBG("rp level alert\n");
	} else {
		typecs_attached_snk_cc_detach(tcpc);
	}

	return 0;
}

static void typecs_attached_snk_vbus_absent(struct tcpc_device *tcpc)
{
	if (tcpci_is_pr_swaping(tcpc)) {
		TYPEC_DBG("%s: PR.Swap Ignore vbus_absent\n", __func__);
		return;
	}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	if (tcpc->pd_during_direct_charge && !typec_check_vsafe0v(tcpc)) {
		TYPEC_INFO("ignore vbus_absent in directcharge\n");
		return;
	}

	if (tcpc->pd_wait_hard_reset_complete) {
#ifdef CONFIG_COMPATIBLE_APPLE_TA
		TYPEC_DBG("ignore vbus_absent in apple hreset\n");
		return;
#else
		if (typec_get_cc_res() != TYPEC_CC_VOLT_OPEN) {
			TYPEC_DBG("ignore vbus_absent in hreset & cc != 0\n");
			return;
		}
#endif /* CONFIG_COMPATIBLE_APPLE_TA */
	}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	timer_disable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	timer_disable(tcpc, TYPEC_TIMER_APPLE_CC_OPEN);
	typecs_snk_detach(tcpc);
}

static int typecs_attached_snk_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc))
		return 0;

	typecs_attached_snk_vbus_absent(tcpc);
	return 0;
}

static void typecs_attached_snk_rplvl_change(struct tcpc_device *tcpc)
{
	uint8_t cc_res = typec_get_cc_res();

	if (cc_res == tcpc->typec_remote_rp_level)
		return;

	TYPEC_INFO("rp level change\n");
	tcpc->typec_remote_rp_level = cc_res;

	pd_notify_pe_rplvl_change(tcpc, cc_res);
	typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
}

#ifdef CONFIG_PD_WAIT_BC12
static void typecs_attached_snk_bc12_timeout(struct tcpc_device *tcpc_dev)
{
	unsigned int chg_type  = power_platform_get_charger_type();

	TYPEC_INFO("%s: chg_type = %d\n", __func__, chg_type);

	/* max wait time is 20 * 50ms = 1s */
	if ((chg_type != CHARGER_REMOVED) || (tcpc_dev->wait_bc12_cnt >= 20)) {
		pd_put_cc_attach_event(tcpc_dev, TYPEC_ATTACHED_SNK);
	} else {
		timer_enable(tcpc_dev, TYPEC_RT_TIMER_SINK_WAIT_BC12);
		tcpc_dev->wait_bc12_cnt++;
	}
}
#endif /* CONFIG_PD_WAIT_BC12 */

static int typecs_attached_snk_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
#ifdef CONFIG_COMPATIBLE_APPLE_TA
	case TYPEC_TIMER_APPLE_CC_OPEN:
#endif /* CONFIG_COMPATIBLE_APPLE_TA */
	case TYPEC_TIMER_PDDEBOUNCE:
		if (!typecs_snk_is_cc_attach(tcpc))
			typecs_snk_detach(tcpc);
		else
			typecs_attached_snk_rplvl_change(tcpc);
		break;
#ifdef CONFIG_PD_WAIT_BC12
	case TYPEC_RT_TIMER_SINK_WAIT_BC12:
		typecs_attached_snk_bc12_timeout(tcpc);
		break;
#endif /* CONFIG_PD_WAIT_BC12 */
	default:
		break;
	}

	return 0;
}

static int typecs_attached_cust_src_entry(struct tcpc_device *tcpc)
{
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();
	struct pd_dpm_typec_state typec_state;

	memset(&typec_state, 0, sizeof(typec_state));
	if ((cc1 == TYPEC_CC_VOLT_SNK_DFT) && (cc2 == TYPEC_CC_VOLT_SNK_DFT)) {
		typec_state.new_state = TYPEC_ATTACHED_CUSTOM_SRC;
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, &typec_state);
	}

	typecs_set_cc_short_detection(tcpc, true, true);
	typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V,
		tcpc->fsm->sink_curr);
	typecs_attach_state_change(tcpc, TYPEC_ATTACHED_CUSTOM_SRC);

	return 0;
}

static int typecs_attached_cust_src_cc_change(struct tcpc_device *tcpc)
{
	if (!typecs_snk_is_cc_attach(tcpc))
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);

	return 0;
}

static int typecs_attached_cust_src_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_attached_cust_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id != TYPEC_TIMER_PDDEBOUNCE)
		return 0;

	if (!typecs_snk_is_cc_attach(tcpc))
		typecs_snk_detach(tcpc);

	return 0;
}

static void typecs_snk_name_init(struct tcpc_device *tcpc)
{
	tcpc->fsm->objs[typec_disabled].name = "Disabled";
	tcpc->fsm->objs[typec_errorrecovery].name = "ErrorRecovery";
	tcpc->fsm->objs[typec_role_swap_snk].name = "RoleSwap.SNK";
	tcpc->fsm->objs[typec_unattached_snk].name = "Unattached.SNK";
	tcpc->fsm->objs[typec_attachwait_snk].name = "AttachWait.SNK";
	tcpc->fsm->objs[typec_try_snk].name = "Try.SNK";
	tcpc->fsm->objs[typec_trywait_snk].name = "TryWait.SNK";
	tcpc->fsm->objs[typec_trywait_snk_pe].name = "TryWait.SNK.PE";
	tcpc->fsm->objs[typec_unattachwait_pe].name = "UnattachWait.PE";
	tcpc->fsm->objs[typec_attached_snk].name = "Attached.SNK";
	tcpc->fsm->objs[typec_attached_custom_src].name = "Custom.SRC";
}

int typecs_snk_init(struct tcpc_device *tcpc)
{
	typecs_snk_name_init(tcpc);

	typecs_obj_construct(typec_disabled, disable);
	typecs_obj_construct(typec_errorrecovery, error_recovery);
	typecs_obj_construct(typec_role_swap_snk, role_swap_snk);
	typecs_obj_construct(typec_unattached_snk, unattached_snk);
	typecs_obj_construct(typec_attachwait_snk, attachwait_snk);
	typecs_obj_construct(typec_try_snk, try_snk);
	typecs_obj_construct(typec_trywait_snk, trywait_snk);
	typecs_obj_construct(typec_trywait_snk_pe, trywait_snk_pe);
	typecs_obj_construct(typec_unattachwait_pe, unattachwait_pe);
	typecs_obj_construct(typec_attached_snk, attached_snk);
	typecs_obj_construct(typec_attached_custom_src, attached_cust_src);

	return 0;
}
