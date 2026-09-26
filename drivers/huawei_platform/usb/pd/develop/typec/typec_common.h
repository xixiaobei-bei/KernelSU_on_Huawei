/*
 * typec_common.h
 *
 * typec common driver
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

#ifndef _TYPEC_COMMON_H_
#define _TYPEC_COMMON_H_

#include "inc/tcpc_core.h"

#define typec_get_role() tcpc->typec_role
#define typec_get_polarity() tcpc->typec_polarity
#define typec_get_local_cc() tcpc->typec_local_cc
#define typec_get_local_rp_lvl() tcpc->typec_local_rp_level

#define typec_get_cc1() tcpc->typec_remote_cc[0]
#define typec_get_cc2() tcpc->typec_remote_cc[1]

#define typec_get_cc_res() (tcpc->typec_polarity ? \
	typec_get_cc2() : typec_get_cc1())

#define typec_check_cc1(cc) (typec_get_cc1() == (cc))
#define typec_check_cc2(cc) (typec_get_cc2() == (cc))

#define typec_check_cc(cc1, cc2) (typec_check_cc1(cc1) && \
	typec_check_cc2(cc2))

#define typec_check_cc_both(res) (typec_check_cc(res, res))

#define typec_check_cc_any(res) (typec_check_cc1(res) || \
	typec_check_cc2(res))

#define typec_is_drp_toggling() (typec_get_cc1() == \
	TYPEC_CC_DRP_TOGGLING)

#define typec_is_cc_open() typec_check_cc_both(TYPEC_CC_VOLT_OPEN)

#define typec_is_cable_only() (typec_get_cc1() + \
	typec_get_cc2() == TYPEC_CC_VOLT_RA)

#define typec_is_sink_with_emark() (typec_get_cc1() + \
	typec_get_cc2() == (TYPEC_CC_VOLT_RA + TYPEC_CC_VOLT_RD))

#define typec_is_cc_no_res() (typec_is_drp_toggling() || \
	typec_is_cc_open())

enum typec_connection_state {
	typec_disabled = 0,
	typec_errorrecovery,

	typec_unattached_snk,
	typec_unattached_src,

	typec_attachwait_snk,
	typec_attachwait_src,

	typec_attached_snk,
	typec_attached_src,

	/*
	 * Require : Assert Rp
	 * Exit(-> Attached.SRC) : Detect Rd (tPDDebounce).
	 * Exit(-> TryWait.SNK) : Not detect Rd after tDRPTry
	 */
	typec_try_src,
	/*
	 * Require : Assert Rd
	 * Exit(-> Attached.SNK) : Detect Rp (tCCDebounce) and Vbus present.
	 * Exit(-> Unattached.SNK) : Not detect Rp (tPDDebounce)
	 */
	typec_trywait_snk,
	typec_trywait_snk_pe,

	/*
	 * Require : Assert Rd
	 * Wait for tDRPTry and only then begin monitoring CC.
	 * Exit (-> Attached.SNK) : Detect Rp (tPDDebounce) and Vbus present.
	 * Exit (-> TryWait.SRC) : Not detect Rp for tPDDebounce.
	 */
	typec_try_snk,
	/*
	 * Require : Assert Rp
	 * Exit (-> Attached.SRC) : Detect Rd (tCCDebounce)
	 * Exit (-> Unattached.SNK) : Not detect Rd after tDRPTry
	 */
	typec_trywait_src,

	typec_audioaccessory,
	typec_debugaccessory,
	typec_attached_custom_src,
	typec_attached_norp_src,

	typec_role_swap_snk,
	typec_role_swap_src,

	/* Wait Policy Engine go to Idle */
	typec_unattachwait_pe,

	typec_state_max,
};

static inline void typec_lock(struct tcpc_device *tcpc)
{
	mutex_lock(&tcpc->typec_lock);
}

static inline void typec_unlock(struct tcpc_device *tcpc)
{
	mutex_unlock(&tcpc->typec_lock);
}

#endif /* _TYPEC_COMMON_H_ */