/*
 * typec_legacy_cable.c
 *
 * typec legacy cable
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

#include "typec_legacy_cable.h"
#include "inc/tcpci.h"
#include "inc/timer.h"
#include "inc/tcpc_core.h"
#include "inc/typec_vbus.h"
#include "inc/debug.h"
#include "typec_common.h"
#include "typec_lpm.h"

void typec_legacy_init(struct tcpc_device *tcpc)
{
	tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
	tcpc->typec_legacy_cable_suspect = 0;
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	tcpc->typec_legacy_retry_wk = 0;
 #endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
}

static void typec_legacy_enable_discharge(struct tcpc_device *tcpc, bool en)
{
	if (tcpci_is_support(tcpc, TCPC_FLAGS_PREFER_LEGACY2))
		return;

	tcpci_enable_ext_discharge(tcpc, en);
}

static void typec_legacy_charge(struct tcpc_device *tcpc)
{
	int i, vbus_level;

	TYPEC_INFO("LC->Charge\n");
	typec_source_vbus(tcpc, TCP_VBUS_CTRL_TYPEC,
		TCPC_VBUS_SOURCE_5V, 100); /* 100ma */

	/* retry 6 times */
	for (i = 0; i < 6; i++) {
		vbus_level = typec_inquire_vbus_level(tcpc, true);
		if (vbus_level >= TCPC_VBUS_VALID)
			return;
		msleep(50); /* 50ms */
	}

	TYPEC_INFO("LC->Charge Failed\n");
}

static void typec_legacy_discharge(struct tcpc_device *tcpc)
{
	int i, vbus_level;

	TYPEC_INFO("LC->Discharge\n");
	typec_source_vbus(tcpc, TCP_VBUS_CTRL_TYPEC,
		TCPC_VBUS_SOURCE_0V, 0);

	/* retry 6 times */
	for (i = 0; i < 6; i++) {
		vbus_level = typec_inquire_vbus_level(tcpc, true);
		if (vbus_level < TCPC_VBUS_VALID)
			return;
		msleep(50); /* 50ms */
	}
	TYPEC_INFO("LC->Discharge Failed\n");
}

static bool typec_legacy_suspect(struct tcpc_device *tcpc)
{
	int i = 0;
	int vbus_level;

	TYPEC_INFO("LC->Suspect\n");
	tcpc->typec_legacy_cable_suspect = 0;

	while (1) {
		vbus_level = typec_inquire_vbus_level(tcpc, true);
		if (vbus_level < TCPC_VBUS_VALID)
			break;

		i++;
		/* retry 3 times */
		if (i > 3) {
			TYPEC_INFO("LC->TAIn\n");
			return false;
		}
		msleep(50); /* 50ms */
	};

	tcpci_set_cc(tcpc, TYPEC_CC_RP_1_5);
	usleep_range(1000, 2000); /* 1ms */
	return tcpci_get_cc(tcpc) != 0;
}

static void typec_legacy_stable1(struct tcpc_device *tcpc)
{
	typec_legacy_charge(tcpc);
	typec_legacy_discharge(tcpc);
	TYPEC_INFO("LC->Stable\n");
	timer_enable(tcpc, TYPEC_RT_TIMER_LEGACY_STABLE);
}

#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
static void typec_legacy_keep_default_rp(struct tcpc_device *tcpc, bool en)
{
	typec_legacy_enable_discharge(tcpc, en);

	if (en) {
		tcpci_set_cc(tcpc, TYPEC_CC_RD);
		usleep_range(1000, 2000); /* 1ms */
		tcpci_set_cc(tcpc, TYPEC_CC_RP);
		usleep_range(1000, 2000); /* 1ms */
	}
}

static bool typec_is_run_legacy_stable2(struct tcpc_device *tcpc)
{
	bool run_legacy2 = false;
	uint8_t retry_max = TYPEC_LEGACY_CABLE_RETRYS;

	run_legacy2 = tcpci_is_support(tcpc, TCPC_FLAGS_PREFER_LEGACY2);

	tcpc->typec_legacy_retry_wk++;
	TYPEC_INFO("LC->Retry%d\n", tcpc->typec_legacy_retry_wk);

	if (tcpc->typec_legacy_retry_wk <= retry_max)
		return run_legacy2;
	/* zoom in twice */
	if (tcpc->typec_legacy_retry_wk > (retry_max * 2))
		tcpc->typec_legacy_retry_wk = 0;

	return !run_legacy2;
}

static void typec_legacy_stable2(struct tcpc_device *tcpc)
{
	tcpc->typec_legacy_cable = TYPEC_LEGACY_CABLE2;
	TYPEC_INFO("LC->Stable2\n");
	typec_legacy_keep_default_rp(tcpc, true);

	timer_enable(tcpc, TYPEC_RT_TIMER_LEGACY_STABLE);

#ifdef CONFIG_TYPEC_LEGACY2_AUTO_RECYCLE
	timer_enable(tcpc, TYPEC_RT_TIMER_LEGACY_RECYCLE);
#endif /* CONFIG_TYPEC_LEGACY2_AUTO_RECYCLE */
}
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */

static void typec_legacy_confirm(struct tcpc_device *tcpc)
{
	TYPEC_INFO("LC->Confirm\n");
	tcpc->typec_legacy_cable = TYPEC_LEGACY_CABLE1;
	timer_disable(tcpc, TYPEC_RT_TIMER_NOT_LEGACY);

#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	if (typec_is_run_legacy_stable2(tcpc)) {
		typec_legacy_stable2(tcpc);
		return;
	}
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */

	typec_legacy_stable1(tcpc);
}

bool typec_legacy_check_cable(struct tcpc_device *tcpc)
{
	bool check_legacy = false;

	if (tcpci_is_support(tcpc, TCPC_FLAGS_DISABLE_LEGACY))
		return false;

#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	if (tcpc->typec_legacy_cable == TYPEC_LEGACY_CABLE2)
		return true;
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */

	if (typec_check_cc(TYPEC_CC_VOLT_RD, TYPEC_CC_VOLT_OPEN) ||
		typec_check_cc(TYPEC_CC_VOLT_OPEN, TYPEC_CC_VOLT_RD))
		check_legacy = true;

	/*
	 * Confirm DUT is connected to legacy cable or not
	 * after suupect_counter > this threshold (0 = always check)
	 */
	if (tcpc->typec_legacy_cable_suspect <
		TYPEC_LEGACY_CABLE_SUSPECT_THD)
		check_legacy = false;

	if (check_legacy) {
		if (typec_legacy_suspect(tcpc)) {
			typec_legacy_confirm(tcpc);
			return true;
		}

		tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
		tcpci_set_cc(tcpc, TYPEC_CC_RP);
	}

	return false;
}

void typec_legacy_enable_low_power(struct tcpc_device *tcpc, uint8_t pull)
{
	int ret;

	ret = tcpci_set_cc(tcpc, pull);
	if (ret) {
		TYPEC_ERR("%s: set cc=%d fail\n", __func__, pull);
		return;
	}

	if (tcpc->typec_legacy_cable) {
		TYPEC_DBG("lpm legacy cable only\n");
		return;
	}
	typec_enable_lpm(tcpc->lpm, pull);
}

static inline void typec_legacy_reset_timer(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	if (tcpc->typec_legacy_cable == TYPEC_LEGACY_CABLE2)
		timer_disable(tcpc, TYPEC_RT_TIMER_LEGACY_RECYCLE);

	timer_disable(tcpc, TYPEC_RT_TIMER_LEGACY_STABLE);
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
}

static void typec_legacy_reach_vsafe5v(struct tcpc_device *tcpc)
{
	TYPEC_INFO("LC->Attached\n");
	tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
	tcpci_set_cc(tcpc, TYPEC_CC_RD);
	typec_legacy_reset_timer(tcpc);
}

static void typec_legacy_reach_vsafe0v(struct tcpc_device *tcpc)
{
	TYPEC_INFO("LC->Detached-PS\n");
	tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
	typec_legacy_enable_low_power(tcpc, TYPEC_CC_DRP);
	timer_disable(tcpc, TYPEC_RT_TIMER_LEGACY_STABLE);
}

bool typec_legacy_ps_change(struct tcpc_device *tcpc)
{
	if (!tcpc->typec_legacy_cable)
		return false;

#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	if (tcpc->typec_legacy_cable != TYPEC_LEGACY_CABLE1)
		return true;
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */

	if (typec_check_vbus_valid(tcpc))
		typec_legacy_reach_vsafe5v(tcpc);
	else if (typec_check_vsafe0v(tcpc))
		typec_legacy_reach_vsafe0v(tcpc);

	return true;
}

void typec_legacy_attach_state_change(struct tcpc_device *tcpc)
{
	if (tcpc->typec_legacy_cable)
		timer_disable(tcpc, TYPEC_RT_TIMER_NOT_LEGACY);
	else
		timer_restart(tcpc, TYPEC_RT_TIMER_NOT_LEGACY);
}

void typec_legacy_handle_detach(struct tcpc_device *tcpc)
{
	tcpc->typec_legacy_cable_suspect++;
	TYPEC_INFO("LC->Suspect: %d\n", tcpc->typec_legacy_cable_suspect);
}

static int typec_legacy_handle_cc_open(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	if (tcpc->typec_legacy_cable == TYPEC_LEGACY_CABLE2) {
		typec_legacy_keep_default_rp(tcpc, false);
		return 1;
	}
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */

	return 0;
}

static inline int typec_legacy_handle_cc_present(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	return tcpc->typec_legacy_cable == TYPEC_LEGACY_CABLE1;
#else
	return 1;
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
}

bool typec_legacy_cc_change(struct tcpc_device *tcpc)
{
	int ret;

	if (!tcpc->typec_legacy_cable)
		return false;

	if (typec_is_cc_open() || typec_is_cable_only())
		ret = typec_legacy_handle_cc_open(tcpc);
	else
		ret = typec_legacy_handle_cc_present(tcpc);

	if (!ret)
		return false;

	TYPEC_INFO("LC->Detached-CC\n");

	tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
	typec_legacy_enable_low_power(tcpc, TYPEC_CC_DRP);
	typec_legacy_reset_timer(tcpc);
	return true;
}

void typec_legacy_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	switch (timer_id) {
	case TYPEC_RT_TIMER_NOT_LEGACY:
		tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
		tcpc->typec_legacy_cable_suspect = 0;
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
		tcpc->typec_legacy_retry_wk = 0;
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
		break;

#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	case TYPEC_RT_TIMER_LEGACY_STABLE:
		if (tcpc->typec_legacy_cable)
			tcpc->typec_legacy_retry_wk--;
		break;

#ifdef CONFIG_TYPEC_LEGACY2_AUTO_RECYCLE
	case TYPEC_RT_TIMER_LEGACY_RECYCLE:
		if (tcpc->typec_legacy_cable == TYPEC_LEGACY_CABLE2) {
			TYPEC_INFO("LC->Recycle\n");
			tcpc->typec_legacy_cable = TYPEC_LEGACY_NONE;
			typec_legacy_keep_default_rp(tcpc, false);
			typec_legacy_enable_low_power(tcpc, TYPEC_CC_DRP);
		}
		break;
#endif /* CONFIG_TYPEC_LEGACY2_AUTO_RECYCLE */
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
	default:
		break;
	}
}
