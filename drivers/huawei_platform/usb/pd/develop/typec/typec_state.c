
/*
 * typec_state.c
 *
 * typec state
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
#include "inc/tcpc_core.h"
#include "inc/timer.h"
#include "inc/typec_vbus.h"
#include "typec_lpm.h"
#include "typec_common.h"
#include "typec_legacy_cable.h"

void typecs_set_cc_short_detection(struct tcpc_device *tcpc,
	bool cc1, bool cc2)
{
	if (tcpci_is_support(tcpc, TCPC_FLAGS_VBUS_CC_SHORT_DETECTION))
		tcpci_set_vbus_cc_short_detection(tcpc, cc1, cc2);
}

void typecs_enable_low_power(struct tcpc_device *tcpc, uint8_t pull)
{
	int ret;

	ret = tcpci_set_cc(tcpc, pull);
	if (ret) {
		TYPEC_ERR("%s: set cc=%d fail\n", __func__, pull);
		return;
	}

	if (tcpc->typec_legacy_cable) {
		TYPEC_DBG("legacy cable only not enable lpm\n");
		return;
	}
	typec_enable_lpm(tcpc->lpm, pull);
}

void typecs_set_plug_orient(struct tcpc_device *tcpc, uint8_t res, bool polarity)
{
	int ret;

	tcpc->typec_polarity = polarity;
	ret = tcpci_set_polarity(tcpc, polarity);
	if (ret)
		TYPEC_ERR("%s: set polarity fail ret=%d\n", __func__, ret);

	ret = tcpci_set_cc(tcpc, res);
	if (ret)
		TYPEC_ERR("%s: set cc fail ret=%d\n", __func__, ret);

	TYPEC_DBG("%s: res=%u, polarity=%u\n", __func__, res, polarity);
}

bool typecs_is_fake_ra_rp30(struct tcpc_device *tcpc)
{
	if ((typec_get_local_cc() != TYPEC_CC_RP_3_0) &&
		(typec_get_local_cc() != TYPEC_CC_DRP_3_0))
		return false;

	tcpci_set_cc(tcpc, TYPEC_CC_RP_DFT);
	/* sleep 1ms */
	usleep_range(1000, 2000);

	return tcpci_get_cc(tcpc);
}

bool typecs_is_cc_attach(struct tcpc_device *tcpc)
{
	bool cc_attach = false;
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();

	if ((cc1 != TYPEC_CC_VOLT_OPEN) || (cc2 != TYPEC_CC_VOLT_OPEN))
		cc_attach = true;

	/* Cable Only, no device */
	if ((cc1 + cc2) == TYPEC_CC_VOLT_RA) {
		if (typecs_is_fake_ra_rp30(tcpc)) {
			TYPEC_INFO("[Cable] Fake Ra\n");
			if ((cc1 + cc2) == TYPEC_CC_VOLT_RD)
				return true;
		}
		cc_attach = false;
		typec_set_cable_only(tcpc->lpm, true);
		TYPEC_INFO("[Cable] Ra Only\n");
	}

	return cc_attach;
}

bool typecs_is_act_as_sink_role(struct tcpc_device *tcpc)
{
	bool as_sink = true;
	uint8_t cc_sum;

	/* 0x7: cc mask */
	switch (typec_get_local_cc() & 0x07) {
	case TYPEC_CC_RP:
		as_sink = false;
		break;
	case TYPEC_CC_RD:
		as_sink = true;
		break;
	case TYPEC_CC_DRP:
		cc_sum = typec_get_cc1() + typec_get_cc2();
		as_sink = (cc_sum >= TYPEC_CC_VOLT_SNK_DFT);
		break;
	}

	TYPEC_DBG("%s: as_sink=%d\n", __func__, as_sink);
	return as_sink;
}

static void typecs_set_dual_role(struct tcpc_device *tcpc,
	enum typec_attach_type type)
{
	switch (type) {
	case TYPEC_ATTACHED_SNK:
		tcpm_dual_role_set_sink(tcpc);
		break;
	case TYPEC_ATTACHED_SRC:
		tcpm_dual_role_set_src(tcpc);
		break;
	case TYPEC_UNATTACHED:
		tcpm_dual_role_set_dft(tcpc);
		break;
	default:
		break;
	}
}

static void typecs_notify_attach_state(struct tcpc_device *tcpc,
	enum typec_attach_type type)
{
	if (tcpc->fsm->attach_type == TYPEC_UNATTACHED) {
		typecs_attach_type(tcpc, type);
		typec_set_wake_lock_pd(tcpc->lpm, true);
		if (!tcpc->fsm->disable_pe)
			pd_notify_usb_port_attach(tcpc, type);
		return;
	}

	typecs_attach_type(tcpc, type);
	if (type == TYPEC_UNATTACHED) {
		if (!tcpc->fsm->disable_pe)
			pd_notify_usb_port_detach(tcpc);
		typec_set_wake_lock_pd(tcpc->lpm, false);
		return;
	}

	TYPEC_INFO("warning: tcpc attach again\n");
}

static const char *const typecs_attach_name[] = {
	"NULL",
	"SINK",
	"SOURCE",
	"AUDIO",
	"DEBUG",
	"DBGACC_SNK",
	"CUSTOM_SRC",
	"NORP_SRC",
};

void typecs_attach_state_change(struct tcpc_device *tcpc,
	enum typec_attach_type type)
{
	if (type >= ARRAY_SIZE(typecs_attach_name))
		return;

	typec_legacy_attach_state_change(tcpc);
	if (tcpc->fsm->attach_type == type) {
		TYPEC_DBG("Repeat: Attached->%s\n", typecs_attach_name[type]);
		return;
	}

	TYPEC_INFO("Attached-> %s\n", typecs_attach_name[type]);

	tcpm_notify_typec_state(&tcpc->notifier, typec_get_polarity(),
		tcpc->fsm->attach_type, type, typec_get_local_rp_lvl());
	typecs_set_dual_role(tcpc, type);
	typecs_notify_attach_state(tcpc, type);
}

static const char *const typecs_wait_ps_name[] = {
	"Disable",
	"SNK_VSafe5V",
	"SRC_VSafe0V",
	"SRC_VSafe5V",
};

void typecs_wait_ps_change(struct tcpc_device *tcpc, enum TYPECS_WAIT_PS_STATE state)
{
	if (state != tcpc->fsm->wait_ps)
		TYPEC_INFO("%s: wait_ps=%s\n", __func__, typecs_wait_ps_name[state]);

	if (state == TYPECS_WAIT_PS_SRC_VSAFE0V)
		timer_enable(tcpc, TYPEC_RT_TIMER_SAFE0V_TOUT);

	 if ((tcpc->fsm->wait_ps == TYPECS_WAIT_PS_SRC_VSAFE0V) &&
	 	(state != TYPECS_WAIT_PS_SRC_VSAFE0V))
		timer_disable(tcpc, TYPEC_RT_TIMER_SAFE0V_TOUT);

	tcpc->fsm->wait_ps = (uint8_t)state;
}

int typecs_get_unattached_cc(struct tcpc_device *tcpc)
{
	int pull = tcpc->fsm->unattached_cc;

	if (pull != TYPEC_CC_INVALID) {
		typecs_unattached_cc(tcpc, TYPEC_CC_INVALID);
		return pull;
	}

	switch (typec_get_role()) {
	case TYPEC_ROLE_SNK:
		return TYPEC_CC_RD;
	case TYPEC_ROLE_SRC:
		return TYPEC_CC_RP;
	default:
		return TYPEC_CC_DRP;
	}
}

int typecs_get_unattached_state(struct tcpc_device *tcpc)
{
	switch (typec_get_role()) {
	case TYPEC_ROLE_SNK:
		return typec_unattached_snk;
	case TYPEC_ROLE_SRC:
		return typec_unattached_src;
	default:
		return typec_unattached_snk;
	}
}

static bool typecs_init_with_charge(struct tcpc_device *tcpc)
{
	tcpci_get_cc(tcpc);
	if (tcpc->typec_role == TYPEC_ROLE_SRC)
		return false;

	if (!typec_check_vbus_valid(tcpc))
		return false;

	TYPEC_INFO("phone power on with chage plugin\n");
	return true;
}

int typecs_init(struct tcpc_device *tcpc)
{
	int ret;

	ret = typecs_snk_init(tcpc);
	ret += typecs_src_init(tcpc);
	if (ret) {
		TYPEC_DBG("%s: typec state object init fail\n", __func__);
		return ret;
	}

	typecs_unattached_cc(tcpc, TYPEC_CC_INVALID);
	tcpc->fsm->init_with_charge = typecs_init_with_charge(tcpc);
	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));

	TYPEC_INFO("%s: init_with_charge=%d\n", __func__,
		tcpc->fsm->init_with_charge);
	return ret;
}

int typecs_deinit(struct tcpc_device *tcpc)
{
	return 0;
}
