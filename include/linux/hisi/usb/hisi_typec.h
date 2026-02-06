/*
 * Copyright (C) 2018 Hisilicon
 * Author: Hisillicon <>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef HISI_TYPEC_H_
#define HISI_TYPEC_H_

#include <linux/kernel.h>
#include <linux/notifier.h>

enum typec_attach_type {
	TYPEC_UNATTACHED = 0,
	TYPEC_ATTACHED_SNK,
	TYPEC_ATTACHED_SRC,
	TYPEC_ATTACHED_AUDIO,
	TYPEC_ATTACHED_DEBUG,

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	TYPEC_ATTACHED_DBGACC_SNK, /* Rp, Rp */
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	TYPEC_ATTACHED_CUSTOM_SRC, /* Same Rp */
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */

	TYPEC_ATTACHED_VBUS_ONLY,
	TYPEC_DETTACHED_VBUS_ONLY,
};


enum {
	TCP_VBUS_CTRL_REMOVE		= 0,
	TCP_VBUS_CTRL_TYPEC		= 1,
	TCP_VBUS_CTRL_PD		= 2,

	TCP_VBUS_CTRL_HRESET		= TCP_VBUS_CTRL_PD,
	TCP_VBUS_CTRL_PR_SWAP		= 3,
	TCP_VBUS_CTRL_REQUEST		= 4,
	TCP_VBUS_CTRL_STANDBY		= 5,
	TCP_VBUS_CTRL_STANDBY_UP	= 6,
	TCP_VBUS_CTRL_STANDBY_DOWN	= 7,

	TCP_VBUS_CTRL_PD_DETECT		= (1 << 7),
	TCP_VBUS_CTRL_PD_HRESET		= TCP_VBUS_CTRL_HRESET | TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_PR_SWAP	= TCP_VBUS_CTRL_PR_SWAP | TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_REQUEST	= TCP_VBUS_CTRL_REQUEST | TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_STANDBY	= TCP_VBUS_CTRL_STANDBY | TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_STANDBY_UP	= TCP_VBUS_CTRL_STANDBY_UP | TCP_VBUS_CTRL_PD_DETECT,
	TCP_VBUS_CTRL_PD_STANDBY_DOWN	= TCP_VBUS_CTRL_STANDBY_DOWN | TCP_VBUS_CTRL_PD_DETECT,
};

struct tcp_ny_vbus_state {
	int mv;
	int ma;
	uint8_t type;
	uint8_t ext_power;
	uint8_t remote_rp_level;
};

#define TYPEC_CC_VOLT_ACT_AS_SINK (1 << 2)

enum tcpm_cc_voltage_status {
	TYPEC_CC_VOLT_OPEN = 0,
	TYPEC_CC_VOLT_RA = 1,
	TYPEC_CC_VOLT_RD = 2,

	TYPEC_CC_VOLT_SNK_DFT = 5,
	TYPEC_CC_VOLT_SNK_1_5 = 6,
	TYPEC_CC_VOLT_SNK_3_0 = 7,

	TYPEC_CC_DRP_TOGGLING = 15,
};

static inline char *tcpm_cc_voltage_status_string(uint8_t cc)
{
	if (cc == TYPEC_CC_VOLT_OPEN)
		return "OPEN";
	else if (cc == TYPEC_CC_VOLT_RA)
		return "RA";
	else if (cc == TYPEC_CC_VOLT_RD)
		return "RD";
	else if (cc == TYPEC_CC_VOLT_SNK_DFT)
		return "Default";
	else if (cc == TYPEC_CC_VOLT_SNK_1_5)
		return "1.5";
	else if (cc == TYPEC_CC_VOLT_SNK_3_0)
		return "3.0";
	else if (cc == TYPEC_CC_DRP_TOGGLING)
		return "DRP";
	else
		return "unknown";
}

enum tcpm_vbus_level {
#ifdef CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT
	TCPC_VBUS_SAFE0V = 0,
	TCPC_VBUS_INVALID,
	TCPC_VBUS_VALID,
#else
	TCPC_VBUS_INVALID = 0,
	TCPC_VBUS_VALID,
#endif
};

static inline char *typec_attach_type_name(uint8_t type)
{
	switch (type) {
	case TYPEC_UNATTACHED: return "UNATTACHED";
	case TYPEC_ATTACHED_SNK: return "SINK";
	case TYPEC_ATTACHED_SRC: return "SOURCE";
	case TYPEC_ATTACHED_AUDIO: return "AUDIO";
	case TYPEC_ATTACHED_DEBUG: return "DBGACC";
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case TYPEC_ATTACHED_DBGACC_SNK: return "DBGACC_SNK";
#endif
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case TYPEC_ATTACHED_CUSTOM_SRC: return "CUSTOM_SRC";
#endif
	case TYPEC_ATTACHED_VBUS_ONLY: return "ATTACHED_VBUS_ONLY";
	case TYPEC_DETTACHED_VBUS_ONLY: return "DETTACHED_VBUS_ONLY";
	default: return "uknown";
	}
}

enum typec_role_defination {
	TYPEC_ROLE_UNKNOWN = 0,
	TYPEC_ROLE_SNK,
	TYPEC_ROLE_SRC,
	TYPEC_ROLE_DRP,
	TYPEC_ROLE_TRY_SRC,
	TYPEC_ROLE_TRY_SNK,
	TYPEC_ROLE_NR,
};

static const char * const typec_role_name[] = {
	"UNKNOWN",
	"SNK",
	"SRC",
	"DRP",
	"TrySRC",
	"TrySNK",
};

enum pd_cable_current_limit {
	PD_CABLE_CURR_UNKNOWN = 0,
	PD_CABLE_CURR_1A5 = 1,
	PD_CABLE_CURR_3A = 2,
	PD_CABLE_CURR_5A = 3,
};

#endif /* HISI_TYPEC_H_ */
