/*
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
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

#include <linux/delay.h>
#include <linux/cpu.h>
#include <linux/hisi/usb/hisi_tcpm.h>

#include "include/tcpci.h"
#include "include/tcpci_typec.h"
#include "include/tcpci_timer.h"

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
#undef	CONFIG_TYPEC_CAP_TRY_STATE
#define CONFIG_TYPEC_CAP_TRY_STATE
#endif

#define TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS

enum TYPEC_WAIT_PS_STATE {
	TYPEC_WAIT_PS_DISABLE = 0,
	TYPEC_WAIT_PS_SNK_VSAFE5V,
	TYPEC_WAIT_PS_SRC_VSAFE0V,
	TYPEC_WAIT_PS_SRC_VSAFE5V,
};

static const char * const typec_wait_ps_name[] = {
	"Disable",
	"SNK_VSafe5V",
	"SRC_VSafe0V",
	"SRC_VSafe5V",
};

enum TYPEC_ROLE_SWAP_STATE {
	TYPEC_ROLE_SWAP_NONE = 0,
	TYPEC_ROLE_SWAP_TO_SNK,
	TYPEC_ROLE_SWAP_TO_SRC,
};

static void typec_wait_ps_change(struct tcpc_device *tcpc_dev,
		enum TYPEC_WAIT_PS_STATE state)
{
	D("wait state %u\n", state);

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
	if (state == TYPEC_WAIT_PS_SRC_VSAFE0V)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_TOUT);
#endif

	if (tcpc_dev->typec_wait_ps_change == TYPEC_WAIT_PS_SRC_VSAFE0V
			&& state != TYPEC_WAIT_PS_SRC_VSAFE0V) {
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_DELAY);

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_TOUT);
#endif
	}

	tcpc_dev->typec_wait_ps_change = (uint8_t) state;
}

#define typec_get_cc1()			tcpc_dev->typec_remote_cc[0]
#define typec_get_cc2()			tcpc_dev->typec_remote_cc[1]
#define typec_get_cc_res() \
		(tcpc_dev->typec_polarity ? typec_get_cc2() : typec_get_cc1())

#define typec_check_cc1(cc)		(typec_get_cc1() == (cc))
#define typec_check_cc2(cc)		(typec_get_cc2() == (cc))
#define typec_check_cc(cc1, cc2) \
		(typec_check_cc1(cc1) && typec_check_cc2(cc2))
#define typec_check_cc_both(res)	(typec_check_cc(res, res))
#define typec_check_cc_any(res) \
		(typec_check_cc1(res) || typec_check_cc2(res))
#define typec_is_drp_toggling() \
		(typec_get_cc1() == TYPEC_CC_DRP_TOGGLING)
#define typec_is_cc_open()		typec_check_cc_both(TYPEC_CC_VOLT_OPEN)

#define typec_is_sink_with_emark() \
		(typec_get_cc1() + typec_get_cc2() == \
			TYPEC_CC_VOLT_RA + TYPEC_CC_VOLT_RD)

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
#define typec_is_cc_ra()	typec_check_cc_both(TYPEC_CC_VOLT_RA)
#define typec_is_cc_no_res()	(typec_is_drp_toggling() || typec_is_cc_open())
#endif

static inline int typec_enable_vconn(struct tcpc_device *tcpc_dev)
{
	if (typec_is_sink_with_emark())
		return tcpci_set_vconn(tcpc_dev, true);

	return 0;
}

/*
 * TYPEC Connection State Definition
 */
enum TYPEC_CONNECTION_STATE {
	typec_disabled = 0,
	typec_errorrecovery,

	typec_unattached_snk,
	typec_unattached_src,

	typec_attachwait_snk,
	typec_attachwait_src,

	typec_attached_snk,
	typec_attached_src,

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	typec_try_snk,
	typec_trywait_src,
	typec_trywait_src_pe,
#endif

	typec_audioaccessory,
	typec_debugaccessory,

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	typec_attached_dbgacc_snk,
#endif

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	typec_attached_custom_src,
#endif

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	typec_role_swap,
#endif
	typec_unattachwait_pe,
};

static const char * const typec_state_name[] = {
	"Disabled",
	"ErrorRecovery",

	"Unattached.SNK",
	"Unattached.SRC",

	"AttachWait.SNK",
	"AttachWait.SRC",

	"Attached.SNK",
	"Attached.SRC",

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	"Try.SNK",
	"TryWait.SRC",
	"TryWait.SRC.PE",
#endif

	"AudioAccessory",
	"DebugAccessory",

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	"DBGACC.SNK",
#endif

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	"Custom.SRC",
#endif

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	"RoleSwap",
#endif

	"UnattachWait.PE",
};

static inline void typec_transfer_state(struct tcpc_device *tcpc_dev,
		enum TYPEC_CONNECTION_STATE state)
{
	TYPEC_INFO("** %s\n", typec_state_name[state]);
	tcpc_dev->typec_state = (uint8_t) state;
}

#define TYPEC_NEW_STATE(state)	(typec_transfer_state(tcpc_dev, state))

static inline int typec_handle_vbus_present(struct tcpc_device *tcpc_dev);
static inline int typec_set_polarity(struct tcpc_device *tcpc_dev,
		bool polarity)
{
	tcpc_dev->typec_polarity = polarity;
	return tcpci_set_polarity(tcpc_dev, polarity);
}

static int typec_set_plug_orient(struct tcpc_device *tcpc_dev,
		uint8_t res, bool polarity)
{
	int rv;

	rv = typec_set_polarity(tcpc_dev, polarity);
	if (rv)
		return rv;

	tcpci_tcpc_print_pd_fsm_state(tcpc_dev);

	tcpci_set_cc(tcpc_dev, res);

	return 0;
}

/*
 * CC alert after vbus_only timer timeout.
 */
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
static void typec_clear_norp_state(struct tcpc_device *tcpc_dev)
{
	switch (tcpc_dev->typec_attach_new) {
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case TYPEC_ATTACHED_DBGACC_SNK:
#endif
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case TYPEC_ATTACHED_CUSTOM_SRC:
#endif
	case TYPEC_ATTACHED_SNK:
		if (tcpc_dev->no_rpsrc_state) {
			D("clear norp state\n");
			tcpc_dev->no_rpsrc_state = 0;
		}
		break;

	default:
		break;
	}
}
#endif /* CONFIG_TYPEC_CAP_NORP_SRC */

/*
 * Handle state change.
 */
static int typec_alert_attach_state_change(struct tcpc_device *tcpc_dev)
{
	int ret;

	if (tcpc_dev->typec_attach_old == tcpc_dev->typec_attach_new) {
		TYPEC_INFO("Attached-> %s(repeat)\n",
			typec_attach_type_name(tcpc_dev->typec_attach_new));
		return 0;
	}

	TYPEC_INFO("Attached-> %s\n",
			typec_attach_type_name(tcpc_dev->typec_attach_new));

	/* cc after vbus too long */
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	typec_clear_norp_state(tcpc_dev);
#endif

	ret = hisi_tcpci_report_usb_port_changed(tcpc_dev);

	/*
	 * The typec_attach_old have three assignment:
	 * 1.here; 2.prs; 3.init
	 */
	tcpc_dev->typec_attach_old = tcpc_dev->typec_attach_new;
	D("set typec_attach_old %u\n", tcpc_dev->typec_attach_old);

	return ret;
}

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
static bool typec_try_enter_norp_src(struct tcpc_device *tcpc_dev)
{
	static bool vusb_uv_det_dis = false;

	if (tcpci_tcpc_vusb_uv_det_masked(tcpc_dev)) {
		D("vusb_uv_det disabled, ignore vbus_only\n");
		vusb_uv_det_dis = true;
		return false;
	}

	if (vusb_uv_det_dis) {
		D("vusb_uv_det disabled, ignore vbus_only\n");
		vusb_uv_det_dis = false;
		return false;
	}

	if (tcpci_check_vbus_valid(tcpc_dev)) {
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_NORP_SRC);
		return true;
	} else if (!tcpci_check_vsafe0v(tcpc_dev, false) &&
				tcpc_dev->typec_during_direct_charge) {
		D("ps_change&direct_chg = 1\n");
		return true;
	}

	return false;
}

static bool typec_try_exit_norp_src(struct tcpc_device *tcpc_dev)
{
	if (!typec_is_cc_ra() && !typec_is_cc_no_res()) {
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TIMER_NORP_SRC);
		return true;
	}

	return false;
}

static bool typec_norp_src_detached_entry(struct tcpc_device *tcpc_dev)
{
	bool ret = false;

	if (tcpc_dev->no_rpsrc_state == 1) {
		tcpc_dev->no_rpsrc_state = 0;
		tcpc_dev->typec_attach_new = TYPEC_DETTACHED_VBUS_ONLY;
		tcpci_notify_typec_state(tcpc_dev);
		ret = true;
	} else {
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TIMER_NORP_SRC);
		ret = false;
	}

	return ret;
}

static inline int typec_norp_src_attached_entry(struct tcpc_device *tcpc_dev)
{
	if (tcpc_dev->no_rpsrc_state == 0) {
		tcpc_dev->no_rpsrc_state = 1;
		tcpc_dev->typec_attach_new = TYPEC_ATTACHED_VBUS_ONLY;
		tcpci_notify_typec_state(tcpc_dev);
	}

	return 0;
}

static inline int typec_handle_norp_src_debounce_timeout(
		struct tcpc_device *tcpc_dev)
{
	if ((typec_is_cc_no_res() || typec_is_cc_ra()) &&
			tcpci_check_vbus_valid(tcpc_dev))
		typec_norp_src_attached_entry(tcpc_dev);

	return 0;
}
#endif /* CONFIG_TYPEC_CAP_NORP_SRC */

static void typec_unattached_power_entry(struct tcpc_device *tcpc_dev)
{
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (tcpc_dev->typec_power_ctrl) {
		tcpci_set_vconn(tcpc_dev, false);
		tcpci_disable_vbus_control(tcpc_dev);
		hisi_tcpci_report_power_control(tcpc_dev, false);
	}
}

static inline void typec_unattached_cc_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");

	tcpc_dev->typec_during_direct_charge = false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap) {
		TYPEC_NEW_STATE(typec_role_swap);
		return;
	}
#endif

	switch (tcpc_dev->typec_role) {
	case TYPEC_ROLE_SNK:
		D("TYPEC_ROLE_SNK\n");
		TYPEC_NEW_STATE(typec_unattached_snk);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
		break;

	case TYPEC_ROLE_SRC:
		D("TYPEC_ROLE_SRC\n");
		TYPEC_NEW_STATE(typec_unattached_src);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
		break;

	default:
		/* DRP */
		D("typec_state: %s\n", typec_state_name[tcpc_dev->typec_state]);
		switch (tcpc_dev->typec_state) {
		case typec_attachwait_snk:
		case typec_audioaccessory:
			TYPEC_NEW_STATE(typec_unattached_src);
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_DFT);
			/* Don't DRP now, but wait 20ms. */
			hisi_tcpc_enable_timer(tcpc_dev,
					TYPEC_TIMER_DRP_SRC_TOGGLE);
			break;

		default:
			TYPEC_NEW_STATE(typec_unattached_snk);

			if (tcpci_tcpc_chip_version_v610(tcpc_dev))
				if (!tcpc_dev->vbus_detect) {
					D("enable vbus_detect\n");
					tcpci_set_vbus_detect(tcpc_dev, true);
				}

			tcpci_tcpc_print_pd_fsm_state(tcpc_dev);
			D("typec_local_cc %d\n", tcpc_dev->typec_local_cc);
			tcpci_reset_pd_fsm(tcpc_dev);
			tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
			hisi_tcpc_poll_cc_status(tcpc_dev);
			break;
		}
		break;
	}

	D("-\n");
}

static void typec_unattached_entry(struct tcpc_device *tcpc_dev)
{
	typec_unattached_cc_entry(tcpc_dev);
	typec_unattached_power_entry(tcpc_dev);
}

/*
 * Wait for PD Engine enter Idle state.
 * If PD is enabled, a timer will call typec_unattached_entry later.
 */
static void typec_unattach_wait_pe_idle_entry(struct tcpc_device *tcpc_dev)
{
	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->typec_attach_old) {
		TYPEC_NEW_STATE(typec_unattachwait_pe);
		return;
	}
#endif

	typec_unattached_entry(tcpc_dev);
}

/*
 * This function called when vbus present 5V.
 */
static void typec_source_attached_with_vbus_entry(struct tcpc_device *tcpc_dev)
{
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_SRC;
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);
}

static void typec_source_attached_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_NEW_STATE(typec_attached_src);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_SRC_VSAFE5V);

	/*
	 * Assign typec_attach_new after vbus valid.
	 */
	hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap) {
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		hisi_tcpc_disable_timer(tcpc_dev,
				TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}
#endif

	typec_set_plug_orient(tcpc_dev, tcpc_dev->typec_local_rp_level,
			typec_check_cc2(TYPEC_CC_VOLT_RD));

	hisi_tcpci_report_power_control(tcpc_dev, true);
	typec_enable_vconn(tcpc_dev);
	tcpci_source_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC,
			TCPC_VBUS_SOURCE_5V, -1);

	/*
	 * Vusb_uv_det is disabled under wireless charging,
	 * so Vbus always present.
	 * Boost_5v stables by delaying 100ms in it's driver.
	 */
	if (tcpci_tcpc_vusb_uv_det_masked(tcpc_dev))
		typec_handle_vbus_present(tcpc_dev);
}

static void typec_sink_attached_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_NEW_STATE(typec_attached_snk);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_SNK;

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
	if (tcpc_dev->typec_role >= TYPEC_ROLE_DRP)
		hisi_tcpc_reset_typec_try_timer(tcpc_dev);
#endif

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap) {
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		hisi_tcpc_disable_timer(tcpc_dev,
				TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}
#endif

	typec_set_plug_orient(tcpc_dev, TYPEC_CC_RD,
			!typec_check_cc2(TYPEC_CC_VOLT_OPEN));
	tcpc_dev->typec_remote_rp_level = typec_get_cc_res();

	hisi_tcpci_report_power_control(tcpc_dev, true);
	tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
}

static void typec_custom_src_attached_entry(struct tcpc_device *tcpc_dev)
{
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();

	if (cc1 == TYPEC_CC_VOLT_SNK_DFT && cc2 == TYPEC_CC_VOLT_SNK_DFT) {
		TYPEC_NEW_STATE(typec_attached_custom_src);
		tcpc_dev->typec_attach_new = TYPEC_ATTACHED_CUSTOM_SRC;
		return;
	}
#endif

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	I("[Warning] Same Rp: %u\n", typec_get_cc1());
#else
	I("[Warning] CC Both Rp\n");
#endif
}

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
static inline uint8_t typec_get_sink_dbg_acc_rp_level(int cc1, int cc2)
{
	if (cc2 == TYPEC_CC_VOLT_SNK_DFT)
		return (uint8_t)cc1;
	else
		return TYPEC_CC_VOLT_SNK_DFT;
}

static void typec_sink_dbg_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	bool polarity = false;
	uint8_t rp_level;
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();

	if (cc1 == cc2) {
		typec_custom_src_attached_entry(tcpc_dev);
		return;
	}

	TYPEC_NEW_STATE(typec_attached_dbgacc_snk);
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_DBGACC_SNK;

	polarity = cc2 > cc1;
	if (polarity)
		rp_level = typec_get_sink_dbg_acc_rp_level(cc2, cc1);
	else
		rp_level = typec_get_sink_dbg_acc_rp_level(cc1, cc2);

	tcpc_dev->typec_remote_rp_level = rp_level;

	hisi_tcpci_report_power_control(tcpc_dev, true);
	tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
}
#else
static inline void typec_sink_dbg_acc_attached_entry(
		struct tcpc_device *tcpc_dev)
{
	typec_custom_src_attached_entry(tcpc_dev);
}
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
static bool typec_role_is_try_sink(struct tcpc_device *tcpc_dev)
{
	I("typec_role %u, typec_during_role_swap %u\n", tcpc_dev->typec_role,
			tcpc_dev->typec_during_role_swap);

	if (tcpc_dev->typec_role != TYPEC_ROLE_TRY_SNK)
		return false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap)
		return false;
#endif

	return true;
}

static void typec_try_snk_entry(struct tcpc_device *tcpc_dev)
{
	I("\n");

	TYPEC_NEW_STATE(typec_try_snk);
	tcpc_dev->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);
}

static void typec_trywait_src_entry(struct tcpc_device *tcpc_dev)
{
	I("\n");

	TYPEC_NEW_STATE(typec_trywait_src);
	tcpc_dev->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_DFT);
	tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_0V, 0);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);
}
#endif

static void typec_cc_snk_detect_vsafe5v_entry(struct tcpc_device *tcpc_dev)
{
	I("\n");

	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (!typec_check_cc_any(TYPEC_CC_VOLT_OPEN)) {
		typec_sink_dbg_acc_attached_entry(tcpc_dev);
		return;
	}

	typec_sink_attached_entry(tcpc_dev);
}

/*
 * If Port Partner act as Source without VBUS, wait vSafe5V
 */
static inline void typec_cc_snk_detect_entry(struct tcpc_device *tcpc_dev)
{
	D("\n");

	if (tcpci_check_vbus_valid(tcpc_dev))
		typec_cc_snk_detect_vsafe5v_entry(tcpc_dev);
	else
		typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_SNK_VSAFE5V);
}

static void typec_cc_src_detect_vsafe0v_entry(struct tcpc_device *tcpc_dev)
{
	D("\n");

	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (typec_role_is_try_sink(tcpc_dev)) {
		if (tcpc_dev->typec_state == typec_attachwait_src) {
			D("to try SNK\n");
			typec_try_snk_entry(tcpc_dev);
			return;
		}
	}
#endif

	typec_source_attached_entry(tcpc_dev);
}

/*
 * If Port Partner act as Sink with low VBUS, wait vSafe0v
 */
static void typec_cc_src_detect_entry(struct tcpc_device *tcpc_dev)
{
	bool vbus_absent = false;

	D("\n");

	vbus_absent = tcpci_check_vsafe0v(tcpc_dev, true);
	if (vbus_absent)
		typec_cc_src_detect_vsafe0v_entry(tcpc_dev);
	else
		if (tcpci_tcpc_vusb_uv_det_masked(tcpc_dev))
			typec_cc_src_detect_vsafe0v_entry(tcpc_dev);
		else
			typec_wait_ps_change(tcpc_dev,
					TYPEC_WAIT_PS_SRC_VSAFE0V);
}

static void typec_cc_src_remove_entry(struct tcpc_device *tcpc_dev)
{
	D("\n");

	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);
	typec_unattach_wait_pe_idle_entry(tcpc_dev);
}

static void typec_cc_snk_remove_entry(struct tcpc_device *tcpc_dev)
{
	D("\n");

	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (tcpc_dev->typec_state == typec_try_snk) {
		typec_trywait_src_entry(tcpc_dev);
		return;
	}
#endif

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
}

static bool typec_debug_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_NEW_STATE(typec_debugaccessory);
	TYPEC_DBG("[Debug] CC1&2 Both Rd\n");
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_DEBUG;
	return true;
}

static bool typec_audio_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	D("\n");

	TYPEC_NEW_STATE(typec_audioaccessory);
	TYPEC_INFO("[Audio] CC1&2 Both Ra\n");
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_AUDIO;

	return true;
}

static bool typec_cc_change_source_entry(struct tcpc_device *tcpc_dev)
{
	bool src_remove = false;

	D("\n");
	switch (tcpc_dev->typec_state) {
	case typec_attached_src:
		if (typec_get_cc_res() != TYPEC_CC_VOLT_RD)
			src_remove = true;
		break;
	case typec_audioaccessory:
		if (!typec_check_cc_both(TYPEC_CC_VOLT_RA))
			src_remove = true;
		break;
	case typec_debugaccessory:
		if (!typec_check_cc_both(TYPEC_CC_VOLT_RD))
			src_remove = true;
		break;
	default:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
			typec_debug_acc_attached_entry(tcpc_dev);
		else if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
			typec_audio_acc_attached_entry(tcpc_dev);
		else if (typec_check_cc_any(TYPEC_CC_VOLT_RD))
			typec_cc_src_detect_entry(tcpc_dev);
		else
			src_remove = true;
		break;
	}

	if (src_remove)
		typec_cc_src_remove_entry(tcpc_dev);

	return true;
}

static bool typec_attached_snk_cc_change(struct tcpc_device *tcpc_dev)
{
	uint8_t cc_res = typec_get_cc_res();

	D("\n");

	if (cc_res != tcpc_dev->typec_remote_rp_level) {
		TYPEC_INFO("RpLvl Change\n");
		tcpc_dev->typec_remote_rp_level = cc_res;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
		if (tcpc_dev->pd_port.pd_prev_connected)
			return true;
#endif
		tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC,
				TCPC_VBUS_SINK_5V, -1);
	}

	return true;
}

static bool typec_cc_change_sink_entry(struct tcpc_device *tcpc_dev)
{
	bool snk_remove = false;

	D("typec_state %u(%s)\n", tcpc_dev->typec_state,
		typec_state_name[tcpc_dev->typec_state]);

	switch (tcpc_dev->typec_state) {
	case typec_attached_snk:
		if (typec_get_cc_res() == TYPEC_CC_VOLT_OPEN)
			snk_remove = true;
		else
			typec_attached_snk_cc_change(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case typec_attached_dbgacc_snk:
		if (typec_get_cc_res() == TYPEC_CC_VOLT_OPEN)
			snk_remove = true;
		break;
#endif

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case typec_attached_custom_src:
		if (typec_check_cc_any(TYPEC_CC_VOLT_OPEN))
			snk_remove = true;
		break;
#endif

	default:
		if (!typec_is_cc_open())
			typec_cc_snk_detect_entry(tcpc_dev);
		else
			snk_remove = true;
	}

	if (snk_remove) {
		D("sink removed\n");
		typec_cc_snk_remove_entry(tcpc_dev);
	}

	return true;
}

static bool typec_is_act_as_sink_role(struct tcpc_device *tcpc_dev)
{
	bool as_sink = true;
	uint8_t cc_sum;

	switch (tcpc_dev->typec_local_cc & 0x07) {
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
	default:
		break;
	}

	D("as_sink %u\n", as_sink);

	return as_sink;
}

static bool typec_handle_cc_changed_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_INFO("[CC_Change] %u/%u\n", typec_get_cc1(), typec_get_cc2());

	tcpc_dev->typec_attach_new = tcpc_dev->typec_attach_old;

	if (typec_is_act_as_sink_role(tcpc_dev))
		typec_cc_change_sink_entry(tcpc_dev);
	else
		typec_cc_change_source_entry(tcpc_dev);

	hisi_usb_typec_cc_status_change(typec_get_cc1(), typec_get_cc2());

	typec_alert_attach_state_change(tcpc_dev);

	return true;
}

static void typec_attach_wait_entry(struct tcpc_device *tcpc_dev)
{
	bool as_sink = false;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	bool pd_en = tcpc_dev->pd_port.pd_prev_connected;
#else
	bool pd_en = false;
#endif

	D("\n");
	if (tcpc_dev->typec_attach_old == TYPEC_ATTACHED_SNK && !pd_en) {
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		D("RpLvl Alert\n");
		return;
	}

	if ((tcpc_dev->typec_attach_old != TYPEC_UNATTACHED)
			|| (tcpc_dev->typec_state == typec_attached_src)) {
		hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
		D("Attached, Ignore cc_attach\n");
		return;
	}

	I("typec_state %u(%s)\n", tcpc_dev->typec_state,
			typec_state_name[tcpc_dev->typec_state]);
	switch (tcpc_dev->typec_state) {
#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_try_snk:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return;

	case typec_trywait_src:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return;
#endif

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	case typec_unattachwait_pe:
		TYPEC_INFO("Force PE Idle\n");
		tcpc_dev->pd_wait_pe_idle = false;
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_PE_IDLE);
		typec_unattached_power_entry(tcpc_dev);
		break;
#endif

	default:
		break;
	}

	as_sink = typec_is_act_as_sink_role(tcpc_dev);
	if (as_sink)
		TYPEC_NEW_STATE(typec_attachwait_snk);
	else
		TYPEC_NEW_STATE(typec_attachwait_src);

	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_CCDEBOUNCE);
}

#ifdef TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS
static int typec_attached_snk_cc_detach(struct tcpc_device *tcpc_dev)
{
	int vbus_valid = tcpci_check_vbus_valid(tcpc_dev);
	bool detach_by_cc = false;

	if (!tcpci_tcpc_cc_is_realdetach(tcpc_dev)) {
		D("Not Real detach by CC");
		return 0;
	}

	if (tcpc_dev->typec_during_direct_charge) {
		D("direct charging, detach by cc\n");
		detach_by_cc = true;
	}

	if (tcpci_tcpc_chip_version_v610(tcpc_dev)) {
		if (!tcpc_dev->vbus_detect) {
			D("vbus detect disabled, detach by cc\n");
			detach_by_cc = true;
			tcpci_set_vbus_detect(tcpc_dev, true);
		}
	}

	I("vbus_valid %d\n", vbus_valid);
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	/* For Source detach during HardReset */
	if ((!vbus_valid) && tcpc_dev->pd_wait_hard_reset_complete) {
		detach_by_cc = true;
		D("Detach_CC (HardReset)\n");
	}
#endif

	if (detach_by_cc)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);

	return 0;
}
#endif /* TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS */

static void typec_detach_wait_entry(struct tcpc_device *tcpc_dev)
{
	switch (tcpc_dev->typec_state) {
#ifdef TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS
	case typec_attached_snk:
		typec_attached_snk_cc_detach(tcpc_dev);
		break;
#endif

	case typec_audioaccessory:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_CCDEBOUNCE);
		break;

#ifdef TYPEC_EXIT_ATTACHED_SRC_NO_DEBOUNCE
	case typec_attached_src:
		TYPEC_INFO("Exit Attached.SRC immediately\n");
		hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);

		/* force to terminate TX */
		hisi_tcpc_tcpci_init(tcpc_dev, true);

		typec_cc_src_remove_entry(tcpc_dev);
		typec_alert_attach_state_change(tcpc_dev);
		break;
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_trywait_src:
		if (tcpc_dev->typec_drp_try_timeout)
			hisi_tcpc_enable_timer(tcpc_dev,
					TYPEC_TIMER_PDDEBOUNCE);
		else {
			hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
			D("[Try] Igrone cc_detach\n");
		}
		break;
#endif
	default:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		break;
	}
}

static bool typec_is_cc_attach(struct tcpc_device *tcpc_dev)
{
	bool cc_attach = false;
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();
	int cc_res = typec_get_cc_res();

	tcpc_dev->typec_cable_only = false;

	switch (tcpc_dev->typec_attach_old) {
	case TYPEC_ATTACHED_SNK:
	case TYPEC_ATTACHED_SRC:

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case TYPEC_ATTACHED_CUSTOM_SRC:
#endif

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case TYPEC_ATTACHED_DBGACC_SNK:
#endif
		if ((cc_res != TYPEC_CC_VOLT_OPEN) &&
				(cc_res != TYPEC_CC_VOLT_RA))
			cc_attach = true;
		break;

	case TYPEC_ATTACHED_AUDIO:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
			cc_attach = true;
		break;

	case TYPEC_ATTACHED_DEBUG:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
			cc_attach = true;
		break;

	default:
		if (cc1 != TYPEC_CC_VOLT_OPEN)
			cc_attach = true;

		if (cc2 != TYPEC_CC_VOLT_OPEN)
			cc_attach = true;

		/* Cable Only, no device */
		if ((cc1 + cc2) == TYPEC_CC_VOLT_RA) {
			cc_attach = false;
			tcpc_dev->typec_cable_only = true;
			D("[Cable] Ra Only\n");
		}
		break;
	}

	D("%u\n", cc_attach);
	return cc_attach;
}

int hisi_tcpc_typec_handle_cc_change(struct tcpc_device *tcpc_dev)
{
	int ret;

	I("\n");
	ret = tcpci_get_cc(tcpc_dev);
	if (ret < 0)
		return ret;

	if (typec_is_drp_toggling()) {
		D("DRP Toggling\n");
		return 0;
	}

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	typec_try_exit_norp_src(tcpc_dev);
#endif

	D("[CC_Alert] cc1:%u, cc2:%u\n", typec_get_cc1(), typec_get_cc2());

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_pr_swap_complete) {
		D("[PR.Swap] Ignore CC_Alert\n");
		return 0;
	}

	if (tcpc_dev->pd_wait_error_recovery) {
		D("[Error Recovery] Ignore CC_Alert\n");
		return 0;
	}
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if ((tcpc_dev->typec_state == typec_try_snk)
			&& (!tcpc_dev->typec_drp_try_timeout)) {
		D("[Try.SNK] Ignore CC_Alert\n");
		return 0;
	}

	if (tcpc_dev->typec_state == typec_trywait_src_pe) {
		D("[Try.PE] Ignore CC_Alert\n");
		return 0;
	}
#endif

	/* for cc alert monitoring, wakeup AP */
	hisi_usb_typec_cc_alert(typec_get_cc1(), typec_get_cc2());

	if (tcpc_dev->typec_state == typec_attachwait_snk
			|| tcpc_dev->typec_state == typec_attachwait_src)
		typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (typec_is_cc_attach(tcpc_dev))
		typec_attach_wait_entry(tcpc_dev);
	else
		typec_detach_wait_entry(tcpc_dev);

	return 0;
}

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
static inline int typec_handle_drp_try_timeout(struct tcpc_device *tcpc_dev)
{
	bool src_detect = false;
	bool en_timer = false;

	D("\n");
	tcpc_dev->typec_drp_try_timeout = true;
	hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);

	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore debounce timeout.\n");
		return 0;
	}

	if (typec_check_cc1(TYPEC_CC_VOLT_RD) ||
			typec_check_cc2(TYPEC_CC_VOLT_RD))
		src_detect = true;

	switch (tcpc_dev->typec_state) {
#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_trywait_src:
		en_timer = !src_detect;
		break;

	case typec_try_snk:
		en_timer = true;
		break;
#endif

	default:
		en_timer = false;
		break;
	}

	if (en_timer)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);

	return 0;
}
#endif /* CONFIG_TYPEC_CAP_TRY_STATE */

static int typec_handle_debounce_timeout(struct tcpc_device *tcpc_dev)
{
	D("\n");

	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore debounce timeout.\n");
		return 0;
	}

	typec_handle_cc_changed_entry(tcpc_dev);

	return 0;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static int typec_handle_error_recovery_timeout(struct tcpc_device *tcpc_dev)
{
	D("\n");

	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_error_recovery = false;
	mutex_unlock(&tcpc_dev->access_lock);

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
	typec_alert_attach_state_change(tcpc_dev);

	return 0;
}

static int typec_handle_pe_idle(struct tcpc_device *tcpc_dev)
{
	switch (tcpc_dev->typec_state) {
	case typec_unattachwait_pe:
		typec_unattached_entry(tcpc_dev);
		break;

	default:
		D("Dummy pe_idle\n");
		break;
	}

	return 0;
}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

static int typec_handle_src_reach_vsafe0v(struct tcpc_device *tcpc_dev)
{
	D("\n");

	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore vsafe0V\n");
		return 0;
	}

	typec_cc_src_detect_vsafe0v_entry(tcpc_dev);

	return 0;
}

static int typec_handle_src_toggle_timeout(struct tcpc_device *tcpc_dev)
{
	D("\n");

	if (tcpc_dev->typec_state == typec_unattached_src) {
		TYPEC_NEW_STATE(typec_unattached_snk);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
	}

	return 0;
}

static int hisi_tcpc_typec_handle_vsafe0v(struct tcpc_device *tcpc_dev)
{
	if (tcpc_dev->typec_wait_ps_change == TYPEC_WAIT_PS_SRC_VSAFE0V) {
#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_DELAY);
#else
		typec_handle_src_reach_vsafe0v(tcpc_dev);
#endif
	}

	return 0;
}

static int typec_handle_safe0v_tout(struct tcpc_device *tcpc_dev)
{
	int ret;
	uint16_t power_status = 0;

	D("\n");

	if (!tcpci_check_vbus_valid(tcpc_dev))
		return  hisi_tcpc_typec_handle_vsafe0v(tcpc_dev);

	TYPEC_INFO("VBUS still Valid!!\n");
	ret = tcpci_get_power_status(tcpc_dev, &power_status);
	if (ret)
		TYPEC_INFO("TCPCI get power status: %d!!\n", ret);

	hisi_tcpci_vbus_level_init(tcpc_dev, power_status);

	TCPC_INFO("Safe0V TOUT: ps=%u\n", tcpc_dev->vbus_level);

	if (!tcpci_check_vbus_valid(tcpc_dev))
		ret = hisi_tcpc_typec_handle_vsafe0v(tcpc_dev);
	else
		D("wait safe0v timeout\n");

	return ret;
}

static int tcpc_typec_handle_timeout(struct tcpc_device *tcpc_dev,
		uint32_t timer_id)
{
	int ret = 0;

	switch (timer_id) {
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		ret = typec_handle_debounce_timeout(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	case TYPEC_TIMER_NORP_SRC:
		ret = typec_handle_norp_src_debounce_timeout(tcpc_dev);
		break;
#endif

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	case TYPEC_TIMER_ERROR_RECOVERY:
		ret = typec_handle_error_recovery_timeout(tcpc_dev);
		break;

	case TYPEC_RT_TIMER_PE_IDLE:
		ret = typec_handle_pe_idle(tcpc_dev);
		break;
#endif

	case TYPEC_RT_TIMER_SAFE0V_DELAY:
		ret = typec_handle_src_reach_vsafe0v(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
	case TYPEC_RT_TIMER_SAFE0V_TOUT:
		ret = typec_handle_safe0v_tout(tcpc_dev);
		break;
#endif

	case TYPEC_TIMER_DRP_SRC_TOGGLE:
		ret = typec_handle_src_toggle_timeout(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	case TYPEC_RT_TIMER_ROLE_SWAP_START:
		if (tcpc_dev->typec_during_role_swap ==
				TYPEC_ROLE_SWAP_TO_SNK) {
			D("Role Swap to Sink\n");
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
			hisi_tcpc_enable_timer(tcpc_dev,
					TYPEC_RT_TIMER_ROLE_SWAP_STOP);
		} else if (tcpc_dev->typec_during_role_swap ==
				TYPEC_ROLE_SWAP_TO_SRC) {
			D("Role Swap to Source\n");
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
			hisi_tcpc_enable_timer(tcpc_dev,
					TYPEC_RT_TIMER_ROLE_SWAP_STOP);
		}
		break;

	case TYPEC_RT_TIMER_ROLE_SWAP_STOP:
		if (tcpc_dev->typec_during_role_swap) {
			D("TypeC Role Swap Failed\n");
			tcpc_dev->typec_during_role_swap = false;
			hisi_tcpc_enable_timer(tcpc_dev,
					TYPEC_TIMER_PDDEBOUNCE);
		}
		break;
#endif

	default:
		break;
	}

	return ret;
}

int hisi_tcpc_typec_handle_timeout(struct tcpc_device *tcpc_dev,
		uint32_t timer_id)
{
	I("timer_id %u\n", timer_id);

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
	if (timer_id == TYPEC_TRY_TIMER_DRP_TRY)
		return typec_handle_drp_try_timeout(tcpc_dev);
#endif

	if (timer_id >= TYPEC_TIMER_START_ID)
		hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
	else if (timer_id >= TYPEC_RT_TIMER_START_ID)
		hisi_tcpc_disable_timer(tcpc_dev, timer_id);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_pr_swap_complete) {
		D("[PR.Swap] Igrone timer_evt\n");
		return 0;
	}

	if (tcpc_dev->pd_wait_error_recovery &&
			(timer_id != TYPEC_TIMER_ERROR_RECOVERY)) {
		D("[Error Recovery] Igrone timer_evt\n");
		return 0;
	}
#endif

	return tcpc_typec_handle_timeout(tcpc_dev, timer_id);
}

static int typec_handle_vbus_present(struct tcpc_device *tcpc_dev)
{
	I("\n");

	switch (tcpc_dev->typec_wait_ps_change) {
	case TYPEC_WAIT_PS_SNK_VSAFE5V:
		typec_cc_snk_detect_vsafe5v_entry(tcpc_dev);
		typec_alert_attach_state_change(tcpc_dev);
		break;

	case TYPEC_WAIT_PS_SRC_VSAFE5V:
		typec_source_attached_with_vbus_entry(tcpc_dev);
		typec_alert_attach_state_change(tcpc_dev);
		break;

	default:
		break;
	}

	return 0;
}

static int typec_attached_snk_vbus_absent(struct tcpc_device *tcpc_dev)
{
	I("\n");

#ifdef TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS
	if (tcpc_dev->typec_during_direct_charge &&
			!tcpci_check_vsafe0v(tcpc_dev, true)) {
		TYPEC_DBG("Ignore vbus_absent(snk), DirectCharge\n");
		return 0;
	}
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_hard_reset_complete ||
			tcpc_dev->pd_hard_reset_event_pending) {
		if (typec_get_cc_res() != TYPEC_CC_VOLT_OPEN) {
			TYPEC_DBG("Ignore vbus_absent(snk), HReset & CC!=0\n");
			return 0;
		}
	}
#endif

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
	typec_alert_attach_state_change(tcpc_dev);
#endif

	return 0;
}


static inline int typec_handle_vbus_absent(struct tcpc_device *tcpc_dev)
{
	I("\n");

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_pr_swap_complete) {
		D("[PR.Swap] Igrone vbus_absent\n");
		return 0;
	}

	if (tcpc_dev->pd_wait_error_recovery) {
		if (typec_get_cc_res() == TYPEC_CC_VOLT_OPEN) {
			D("CC Open, go through\n");
		} else {
			D("[Error Recovery] CC not open, Igrone vbus_absent\n");
			return 0;
		}
	}
#endif

	switch (tcpc_dev->typec_state) {
	case typec_attached_snk:
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case typec_attached_dbgacc_snk:
#endif

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case typec_attached_custom_src:
#endif
		typec_attached_snk_vbus_absent(tcpc_dev);
		break;
	}

#ifndef CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT
	hisi_tcpc_typec_handle_vsafe0v(tcpc_dev);
#endif

	return 0;
}

/*
 * Power status change interrupt service route.
 */
int hisi_tcpc_typec_handle_ps_change(struct tcpc_device *tcpc_dev,
		int vbus_level)
{
	I("\n");

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	if (typec_is_cc_no_res() || typec_is_cc_ra()) {
		if (!typec_try_enter_norp_src(tcpc_dev))
			typec_norp_src_detached_entry(tcpc_dev);
	}
#endif

	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore power status change.\n");
		return 0;
	}

	if (vbus_level >= TCPC_VBUS_VALID)
		return typec_handle_vbus_present(tcpc_dev);
	else
		return typec_handle_vbus_absent(tcpc_dev);
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
int hisi_tcpc_typec_advertise_explicit_contract(struct tcpc_device *tcpc_dev)
{
	if (tcpc_dev->typec_local_rp_level == TYPEC_CC_RP_DFT)
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_1_5);

	return 0;
}

int hisi_tcpc_typec_handle_pe_pr_swap(struct tcpc_device *tcpc_dev)
{
	I("\n");

	mutex_lock(&tcpc_dev->typec_lock);
	switch (tcpc_dev->typec_state) {
	case typec_attached_snk:
		TYPEC_NEW_STATE(typec_attached_src);
		tcpc_dev->typec_attach_old = TYPEC_ATTACHED_SRC;
		D("set typec_attach_old %d\n", tcpc_dev->typec_attach_old);
		tcpci_snk_src_set_cc(tcpc_dev, tcpc_dev->typec_local_rp_level);
		break;

	case typec_attached_src:
		TYPEC_NEW_STATE(typec_attached_snk);
		tcpc_dev->typec_attach_old = TYPEC_ATTACHED_SNK;
		D("set typec_attach_old %d\n", tcpc_dev->typec_attach_old);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
		tcpci_tcpc_print_pd_fsm_state(tcpc_dev);
		break;

	default:
		break;
	}
	mutex_unlock(&tcpc_dev->typec_lock);

	return 0;
}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
int hisi_tcpc_typec_swap_role(struct tcpc_device *tcpc_dev)
{
	I("\n");

	if (tcpc_dev->typec_role < TYPEC_ROLE_DRP)
		return -1;

	if (tcpc_dev->typec_during_role_swap)
		return -1;

	switch (tcpc_dev->typec_attach_old) {
	case TYPEC_ATTACHED_SNK:
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_TO_SRC;
		break;
	case TYPEC_ATTACHED_SRC:
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_TO_SNK;
		break;
	default:
		break;
	}

	if (tcpc_dev->typec_during_role_swap) {
		TYPEC_INFO("TypeC Role Swap Start\n");
		tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
		hisi_tcpc_enable_timer(tcpc_dev,
				TYPEC_RT_TIMER_ROLE_SWAP_START);
		return 0;
	}

	return -1;
}
#endif /* CONFIG_TYPEC_CAP_ROLE_SWAP */

int hisi_tcpc_typec_set_direct_charge(struct tcpc_device *tcpc_dev,
		bool direct_charge)
{
	D("\n");

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->typec_during_direct_charge = direct_charge;
	mutex_unlock(&tcpc_dev->access_lock);

	return 0;
}

/*
 * For tcpm.
 */
int hisi_tcpc_typec_set_rp_level(struct tcpc_device *tcpc_dev, uint8_t res)
{
	D("\n");

	switch (res) {
	case TYPEC_CC_RP_DFT:
	case TYPEC_CC_RP_1_5:
	case TYPEC_CC_RP_3_0:
		TYPEC_INFO("TypeC-Rp: %u\n", res);
		tcpc_dev->typec_local_rp_level = res;
		break;
	default:
		TYPEC_INFO("TypeC-Unknown-Rp (%u)\n", res);
		return -1;
	}

	if ((tcpc_dev->typec_attach_old != TYPEC_UNATTACHED) &&
			(tcpc_dev->typec_attach_new != TYPEC_UNATTACHED))
		tcpci_set_cc(tcpc_dev, res);

	return 0;
}

/*
 * tcpm call this function, for force role swap!!!
 */
int hisi_tcpc_typec_change_role(struct tcpc_device *tcpc_dev,
		uint8_t typec_role)
{
	bool force_unattach = false;

	D("\n");
	if (typec_role == TYPEC_ROLE_UNKNOWN || typec_role >= TYPEC_ROLE_NR) {
		TYPEC_INFO("Wrong TypeC-Role: %d\n", typec_role);
		return -1;
	}

	if (typec_role == TYPEC_ROLE_TRY_SRC) {
		TYPEC_INFO("Try.SRC is not supported\n");
		return -1;
	}

	mutex_lock(&tcpc_dev->access_lock);

	/* The tcpc_dev->typec_role come from desc! */
	tcpc_dev->typec_role = typec_role;
	TYPEC_INFO("typec_new_role: %s\n", typec_role_name[typec_role]);

	if (tcpc_dev->typec_attach_new != TYPEC_UNATTACHED) {
		force_unattach = true;
	} else {
		switch (tcpc_dev->typec_role) {
		case TYPEC_ROLE_SNK:
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
			break;
		case TYPEC_ROLE_SRC:
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
			break;
		default:
			tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
			break;
		}
	}

	if (force_unattach) {
		TYPEC_DBG("force_unattach\n");
		tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
		mutex_unlock(&tcpc_dev->access_lock);
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return 0;
	}

	mutex_unlock(&tcpc_dev->access_lock);

	return 0;
}

void hisi_tcpc_typec_force_unattach(struct tcpc_device *tcpc_dev)
{
	D("\n");

	mutex_lock(&tcpc_dev->access_lock);
	tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
	mutex_unlock(&tcpc_dev->access_lock);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
}

#ifdef CONFIG_TYPEC_CAP_POWER_OFF_CHARGE
/* machine startup with charger */
static int typec_init_power_off_charge(struct tcpc_device *tcpc_dev)
{
	int ret;
	bool cc_open = false;

	I("\n");
	ret = tcpci_get_cc(tcpc_dev);
	if (ret < 0)
		return ret;

	if (tcpc_dev->typec_role == TYPEC_ROLE_SRC)
		return 0;

	cc_open = typec_is_cc_open();

#ifndef CONFIG_TYPEC_CAP_NORP_SRC
	if (cc_open) {
		D("cc both open\n");
		return 0;
	}
#endif

	if (!tcpci_check_vbus_valid(tcpc_dev)) {
		D("vbus not valid\n");
		return 0;
	}

	TYPEC_NEW_STATE(typec_unattached_snk);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (cc_open) {
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_NORP_SRC);
#endif
		tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
	} else {
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
	}

	return 1;
}
#endif /* CONFIG_TYPEC_CAP_POWER_OFF_CHARGE */

int hisi_tcpc_typec_init(struct tcpc_device *tcpc_dev, uint8_t typec_role)
{
	int ret = 0;

	if (typec_role >= TYPEC_ROLE_NR) {
		TYPEC_INFO("Wrong TypeC-Role: %u\n", typec_role);
		return -EINVAL;
	}

	D("typec_role: %s\n", typec_role_name[typec_role]);

	tcpc_dev->typec_role = typec_role;
	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;
	tcpc_dev->typec_attach_old = TYPEC_UNATTACHED;

	tcpc_dev->typec_remote_cc[0] = TYPEC_CC_VOLT_OPEN;
	tcpc_dev->typec_remote_cc[1] = TYPEC_CC_VOLT_OPEN;

	tcpc_dev->wake_lock_pd = 0;
	tcpc_dev->wake_lock_user = true;
	tcpc_dev->typec_usb_sink_curr = CONFIG_TYPEC_SNK_CURR_DFT;
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	tcpc_dev->no_rpsrc_state = 0;
#endif

#ifdef CONFIG_TYPEC_CAP_POWER_OFF_CHARGE
	ret = typec_init_power_off_charge(tcpc_dev);
	if (ret != 0) {
		pr_err("typec_init_power_off_charge error ret %d", ret);
		return ret;
	}
#endif

#ifdef CONFIG_TYPEC_POWER_CTRL_INIT
	tcpc_dev->typec_power_ctrl = true;
#endif

	typec_unattached_entry(tcpc_dev);

	return ret;
}
