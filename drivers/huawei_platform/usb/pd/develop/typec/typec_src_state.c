
/*
 * typec_src_state.c
 *
 * typec src state
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
#include "inc/tcpc_core.h"
#include "inc/timer.h"
#include "inc/typec_vbus.h"
#include "typec_common.h"
#include "typec_legacy_cable.h"

static void typecs_enable_vconn(struct tcpc_device *tcpc)
{
	int ret;

#ifndef CONFIG_HW_USB_POWER_DELIVERY
	if (!typec_is_sink_with_emark())
		return;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

#ifdef CONFIG_TCPC_VCONN_SUPPLY_MODE
	if (tcpc->tcpc_vconn_supply == TCPC_VCONN_SUPPLY_NEVER)
		return;
#endif /* CONFIG_TCPC_VCONN_SUPPLY_MODE */

	ret = tcpci_set_vconn(tcpc, true);
	TYPEC_INFO("%s: open vconn ret=%d\n", __func__, ret);
}

static bool typecs_src_to_src(struct tcpc_device *tcpc)
{
	/* If Port Partner act as Sink with low VBUS, wait vSafe0v */
	if (!typec_check_vsafe0v(tcpc)) {
		typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_SRC_VSAFE0V);
		return true;
	}

	if ((typec_get_role() == TYPEC_ROLE_TRY_SNK) &&
		(tcpc->fsm->cur_state == typec_attachwait_src)) {
		typecs_next_state(tcpc, typec_try_snk);
		return true;
	}

	typecs_next_state(tcpc, typec_attached_src);
	return true;
}

static bool typecs_src_to_audio(struct tcpc_device *tcpc)
{
	if (typecs_is_fake_ra_rp30(tcpc)) {
		TYPEC_INFO("%s: fake both ra\n", __func__);
		if (typec_check_cc_any(TYPEC_CC_VOLT_RD))
			return typecs_src_to_src(tcpc);

		return false;
	}

	typecs_next_state(tcpc, typec_audioaccessory);
	return true;
}

static bool typecs_src_try_attach(struct tcpc_device *tcpc)
{
	bool attach = true;

	if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
		typecs_next_state(tcpc, typec_debugaccessory);
	else if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
		attach = typecs_src_to_audio(tcpc);
	else if (typec_check_cc_any(TYPEC_CC_VOLT_RD))
		attach = typecs_src_to_src(tcpc);

	return attach;
}

static void typecs_src_detach(struct tcpc_device *tcpc)
{
	if (pd_is_pe_running(tcpc)) {
		typecs_next_state(tcpc, typec_unattachwait_pe);
		return;
	}

	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
}

static bool typecs_src_is_cc_attach(struct tcpc_device *tcpc)
{
	if (typecs_is_act_as_sink_role(tcpc))
		return false;

	if (typec_check_cc_both(TYPEC_CC_VOLT_RA) ||
		typec_check_cc_any(TYPEC_CC_VOLT_RD))
		return true;

	return false;
}

static void typec_src_ps_reach_vsafe0v(struct tcpc_device *tcpc)
{
	if (typec_is_drp_toggling()) {
		TYPEC_DBG("%s: Warning DRP Toggling\n", __func__);
		return;
	}

	if (tcpci_is_pr_swaping(tcpc)) {
		TYPEC_DBG("%s: PR.Swap Ignore vsafe0v\n", __func__);
		return;
	}

	if (!typecs_check_wait_ps(tcpc, TYPECS_WAIT_PS_SRC_VSAFE0V))
		return;

	typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_DISABLE);
	timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);
}

static void typec_src_ps_vsafe0v_timeout(struct tcpc_device *tcpc)
{
	if (!typec_check_vbus_valid_from_ic(tcpc)) {
		typec_src_ps_reach_vsafe0v(tcpc);
		return;
	}

	TYPEC_ERR("%s: vbus reach vsafeov timeout\n", __func__);
}

static int typecs_unattached_src_entry(struct tcpc_device *tcpc)
{
	tcpc->fsm->sink_curr = CONFIG_TYPEC_SNK_CURR_DFT;
	tcpci_set_vconn(tcpc, false);
	typec_disable_vbus_control(tcpc);
	typecs_set_cc_short_detection(tcpc, false, false);

	if (typec_get_role() == TYPEC_ROLE_SRC) {
		typecs_enable_low_power(tcpc, typecs_get_unattached_cc(tcpc));
	} else {
		tcpci_set_cc(tcpc, typecs_get_unattached_cc(tcpc));
		timer_enable(tcpc, TYPEC_TIMER_DRP_SRC_TOGGLE);
	}

	typecs_attach_state_change(tcpc, TYPEC_UNATTACHED);
	return 0;
}

static int typecs_unattached_src_cc_change(struct tcpc_device *tcpc)
{
	bool as_src = true;

	if (!typecs_is_cc_attach(tcpc))
		return 0;

	as_src = !typecs_is_act_as_sink_role(tcpc);
	if (as_src && typec_legacy_check_cable(tcpc))
		return 0;

	if (as_src)
		typecs_next_state(tcpc, typec_attachwait_src);
	else
		typecs_next_state(tcpc, typec_attachwait_snk);
	return 0;
}

static int typecs_unattached_src_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_unattached_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TIMER_DRP_SRC_TOGGLE:
		typecs_next_state(tcpc, typec_unattached_snk);
		break;
	case TYPEC_TIMER_PDDEBOUNCE:
		typecs_unattached_src_cc_change(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_attachwait_src_entry(struct tcpc_device *tcpc)
{
	tcpm_notify_source_vbus_wait(&tcpc->notifier);
	/* Advertise Rp level before Attached.SRC Ellisys 3.1.6359 */
	tcpci_set_cc(tcpc, typec_get_local_rp_lvl());
	timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);

	return 0;
}

static int typecs_attachwait_src_cc_change(struct tcpc_device *tcpc)
{
	if (typecs_src_is_cc_attach(tcpc))
		timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);
	else
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);

	return 0;
}

static int typecs_attachwait_src_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vsafe0v(tcpc))
		typec_src_ps_reach_vsafe0v(tcpc);

	return 0;
}

static int typecs_attachwait_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		if (!typecs_src_is_cc_attach(tcpc) ||
			!typecs_src_try_attach(tcpc))
			typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
		break;
	case TYPEC_RT_TIMER_SAFE0V_TOUT:
		typec_src_ps_vsafe0v_timeout(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_norp_src_entry(struct tcpc_device *tcpc)
{
	struct pd_dpm_typec_state tc_state;

#ifdef CONFIG_TYPEC_CAP_A2C_C2C
	tcpc->typec_a2c_cable = true;
#endif /* CONFIG_TYPEC_CAP_A2C_C2C */

	/* 500ma */
	typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, 500);
	typecs_attach_state_change(tcpc, TYPEC_ATTACHED_NORP_SRC);

	memset(&tc_state, 0, sizeof(tc_state));
	tc_state.new_state = PD_DPM_TYPEC_ATTACHED_NORP_SRC;
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void *)&tc_state);

	return 0;
}

static void typecs_norp_src_exit(struct tcpc_device *tcpc)
{
	struct pd_dpm_typec_state tc_state;

	TYPEC_DBG("%s+\n", __func__);
	memset(&tc_state, 0, sizeof(tc_state));
	tc_state.new_state = PD_DPM_TYPEC_ATTACHED_NORP_SRC;
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void *)&tc_state);

	typecs_next_state(tcpc, typec_unattached_snk);
}

static void typecs_try_exit_norp_src(struct tcpc_device *tcpc)
{
	if (typec_check_vbus_valid(tcpc) && typec_is_cc_no_res())
		return;

	typecs_norp_src_exit(tcpc);
}

static int typecs_norp_src_cc_change(struct tcpc_device *tcpc)
{
	typecs_try_exit_norp_src(tcpc);
	return 0;
}

static int typecs_norp_src_ps_change(struct tcpc_device *tcpc)
{
	typecs_try_exit_norp_src(tcpc);
	return 0;
}

static int typecs_norp_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	return 0;
}

static int typecs_try_src_entry(struct tcpc_device *tcpc)
{
	tcpci_set_cc(tcpc, TYPEC_CC_RP);
	typecs_in_drp_try(tcpc);
	timer_enable(tcpc, TYPEC_TRY_TIMER_DRP_TRY);

	return 0;
}

static int typecs_try_src_cc_change(struct tcpc_device *tcpc)
{
	if (typecs_is_cc_attach(tcpc)) {
		timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
		return 0;
	}

	if (!typecs_is_drp_try(tcpc)) {
		timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	} else {
		timer_reset_typec_debt(tcpc);
		TYPEC_DBG("%s: ignore cc_detach\n", __func__);
	}

	return 0;
}

static int typecs_try_src_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vsafe0v(tcpc))
		typec_src_ps_reach_vsafe0v(tcpc);

	return 0;
}

static void typec_try_src_drp_timeout(struct tcpc_device *tcpc)
{
	typecs_out_drp_try(tcpc);

	if (typec_is_drp_toggling()) {
		TYPEC_DBG("%s: Warning DRP Toggling\n", __func__);
		return;
	}

	if (!typec_check_cc_any(TYPEC_CC_VOLT_RD))
		timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
}

static void typecs_try_src_debounce_timeout(struct tcpc_device *tcpc)
{
	if (typecs_src_is_cc_attach(tcpc)) {
		if (!typecs_src_try_attach(tcpc))
			typecs_next_state(tcpc, typec_trywait_snk);
	} else {
		typecs_next_state(tcpc, typec_trywait_snk);
	}
}

static int typecs_try_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TRY_TIMER_DRP_TRY:
		typec_try_src_drp_timeout(tcpc);
		break;
	case TYPEC_TIMER_TRYCCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		typecs_try_src_debounce_timeout(tcpc);
		break;
	case TYPEC_RT_TIMER_SAFE0V_TOUT:
		typec_src_ps_vsafe0v_timeout(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_trywait_src_entry(struct tcpc_device *tcpc)
{
	tcpci_set_cc(tcpc, TYPEC_CC_RP);
	typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_0V, 0);
	typecs_in_drp_try(tcpc);
	timer_enable(tcpc, TYPEC_TRY_TIMER_DRP_TRY);

	return 0;
}

static int typecs_trywait_src_cc_change(struct tcpc_device *tcpc)
{
	if (typecs_src_is_cc_attach(tcpc)) {
		timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
		return 0;
	}

	if (!typecs_is_drp_try(tcpc)) {
		timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
	} else {
		timer_reset_typec_debt(tcpc);
		TYPEC_DBG("%s: ignore cc_detach\n", __func__);
	}

	return 0;
}

static int typecs_trywait_src_ps_change(struct tcpc_device *tcpc)
{
	if (typec_check_vsafe0v(tcpc))
		typec_src_ps_reach_vsafe0v(tcpc);

	return 0;
}

static void typec_trywait_src_drp_timeout(struct tcpc_device *tcpc)
{
	typecs_out_drp_try(tcpc);

	if (typec_is_drp_toggling()) {
		TYPEC_DBG("%s: Warning DRP Toggling\n", __func__);
		return;
	}

	if (!typec_check_cc_any(TYPEC_CC_VOLT_RD))
		timer_enable(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
}

static void typecs_trywait_src_debounce_timeout(struct tcpc_device *tcpc)
{
	timer_disable(tcpc, TYPEC_TRY_TIMER_DRP_TRY);
	if (typecs_src_is_cc_attach(tcpc) && typecs_src_try_attach(tcpc))
		return;

	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
}

static int typecs_trywait_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TRY_TIMER_DRP_TRY:
		typec_trywait_src_drp_timeout(tcpc);
		break;
	case TYPEC_TIMER_TRYCCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		typecs_trywait_src_debounce_timeout(tcpc);
		break;
	case TYPEC_RT_TIMER_SAFE0V_TOUT:
		typec_src_ps_vsafe0v_timeout(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_role_swap_src_entry(struct tcpc_device *tcpc)
{
	tcpci_set_cc(tcpc, TYPEC_CC_OPEN);
	timer_enable(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_START);
	return 0;
}

static int typecs_role_swap_src_cc_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_role_swap_src_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_role_swap_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_RT_TIMER_ROLE_SWAP_START:
		tcpci_set_cc(tcpc, TYPEC_CC_RD);
		timer_enable(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
		break;
	case TYPEC_RT_TIMER_ROLE_SWAP_STOP:
		typecs_unattached_cc(tcpc, TYPEC_CC_RD);
		typecs_next_state(tcpc, typec_unattached_snk);
		break;
	default:
		break;
	}

	return 0;
}

static int typecs_attached_src_entry(struct tcpc_device *tcpc)
{
	typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_SRC_VSAFE5V);
	typecs_set_plug_orient(tcpc, tcpc->typec_local_rp_level,
		typec_check_cc2(TYPEC_CC_VOLT_RD));
	typecs_set_cc_short_detection(tcpc, tcpc->typec_polarity == 0,
		tcpc->typec_polarity == 1);
	typecs_enable_vconn(tcpc);
	typec_source_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_5V, -1);

	return 0;
}

static void typecs_attached_src_fake_ra(struct tcpc_device *tcpc)
{
	int cc_res = typec_get_cc_res();

	if ((cc_res != TYPEC_CC_VOLT_RA) ||
		(typec_get_local_rp_lvl() != TYPEC_CC_RP_DFT))
		return;

	tcpci_set_cc(tcpc, TYPEC_CC_RP_1_5);
	/* sleep 1ms */
	usleep_range(1000, 2000);
	tcpci_get_cc(tcpc);
}

static int typecs_attached_src_cc_change(struct tcpc_device *tcpc)
{
	typecs_attached_src_fake_ra(tcpc);
	if (typecs_src_is_cc_attach(tcpc)) {
		timer_reset_typec_debt(tcpc);
		TYPEC_DBG("%s: attached, ignore cc_attach\n", __func__);
		return 0;
	}

	if (tcpc->fsm->attach_type != TYPEC_ATTACHED_SRC)
		typec_legacy_handle_detach(tcpc);

	timer_enable(tcpc, TYPEC_TIMER_SRCDISCONNECT);
	return 0;
}

static int typecs_attached_src_ps_change(struct tcpc_device *tcpc)
{
	if (!typec_check_vbus_valid(tcpc) ||
		!typecs_check_wait_ps(tcpc, TYPECS_WAIT_PS_SRC_VSAFE5V))
		return 0;

	typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_DISABLE);
	if (typec_get_cc_res() != TYPEC_CC_VOLT_RD)
		timer_enable(tcpc, TYPEC_RT_TIMER_STATE_CHANGE);
	else
		typecs_attach_state_change(tcpc, TYPEC_ATTACHED_SRC);

	return 0;
}

static void typecs_attached_src_detach_timeout(struct tcpc_device *tcpc)
{
	if (typec_get_cc_res() == TYPEC_CC_VOLT_RD)
		return;

	if (typec_get_role() == TYPEC_ROLE_TRY_SRC) {
		typecs_next_state(tcpc, typec_trywait_snk_pe);
		return;
	}

	typecs_src_detach(tcpc);
}

static int typecs_attached_src_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_TIMER_SRCDISCONNECT:
		typecs_attached_src_detach_timeout(tcpc);
		break;
	case TYPEC_RT_TIMER_STATE_CHANGE:
		typecs_attach_state_change(tcpc, TYPEC_ATTACHED_SRC);
		break;
	default:
		break;
	}

	return 0;
}

static void typec_attached_audio_sink_vbus(struct tcpc_device *tcpc, bool vbus_valid)
{
	if (vbus_valid)
		/* 500ma */
		typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, 500);
	else
		typec_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_0V, 0);
}

static int typecs_attached_audio_entry(struct tcpc_device *tcpc)
{
	typecs_attach_state_change(tcpc, TYPEC_ATTACHED_AUDIO);

	if (typec_check_vbus_valid(tcpc))
		typec_attached_audio_sink_vbus(tcpc, true);

	return 0;
}

static int typecs_attached_audio_cc_change(struct tcpc_device *tcpc)
{
	if (typec_check_cc_both(TYPEC_CC_VOLT_RA)) {
		timer_reset_typec_debt(tcpc);
		TYPEC_DBG("%s: attached, ignore cc_attach\n", __func__);
		return 0;
	}

	timer_enable(tcpc, TYPEC_TIMER_CCDEBOUNCE);
	return 0;
}

static int typecs_attached_audio_ps_change(struct tcpc_device *tcpc)
{
	typec_attached_audio_sink_vbus(tcpc, typec_check_vbus_valid(tcpc));
	return 0;
}

static int typecs_attached_audio_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id != TYPEC_TIMER_CCDEBOUNCE)
		return 0;

	if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
		return 0;

	typecs_src_detach(tcpc);
	return 0;
}

static int typecs_attached_debug_entry(struct tcpc_device *tcpc)
{
	typecs_attach_state_change(tcpc, TYPEC_ATTACHED_DEBUG);
	return 0;
}

static int typecs_attached_debug_cc_change(struct tcpc_device *tcpc)
{
	if (typec_check_cc_both(TYPEC_CC_VOLT_RD)) {
		timer_reset_typec_debt(tcpc);
		TYPEC_DBG("%s: attached, ignore cc_attach\n", __func__);
		return 0;
	}

	timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	return 0;
}

static int typecs_attached_debug_ps_change(struct tcpc_device *tcpc)
{
	return 0;
}

static int typecs_attached_debug_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	if (timer_id != TYPEC_TIMER_PDDEBOUNCE)
		return 0;

	if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
		return 0;

	typecs_src_detach(tcpc);
	return 0;
}

static void typecs_src_name_init(struct tcpc_device *tcpc)
{
	tcpc->fsm->objs[typec_unattached_src].name = "Unattached.SRC";
	tcpc->fsm->objs[typec_attachwait_src].name = "AttachWait.SRC";
	tcpc->fsm->objs[typec_attached_norp_src].name = "NoRp.SRC";
	tcpc->fsm->objs[typec_try_src].name = "Try.SRC";
	tcpc->fsm->objs[typec_trywait_src].name = "TryWait.SRC";
	tcpc->fsm->objs[typec_role_swap_src].name = "RoleSwap.SRC";
	tcpc->fsm->objs[typec_attached_src].name = "Attached.SRC";
	tcpc->fsm->objs[typec_audioaccessory].name = "AudioAccessory";
	tcpc->fsm->objs[typec_debugaccessory].name = "DebugAccessory";
}

int typecs_src_init(struct tcpc_device *tcpc)
{
	typecs_src_name_init(tcpc);

	typecs_obj_construct(typec_unattached_src, unattached_src);
	typecs_obj_construct(typec_attachwait_src, attachwait_src);
	typecs_obj_construct(typec_attached_norp_src, norp_src);
	typecs_obj_construct(typec_try_src, try_src);
	typecs_obj_construct(typec_trywait_src, trywait_src);
	typecs_obj_construct(typec_role_swap_src, role_swap_src);
	typecs_obj_construct(typec_attached_src, attached_src);
	typecs_obj_construct(typec_audioaccessory, attached_audio);
	typecs_obj_construct(typec_debugaccessory, attached_debug);

	return 0;
}
