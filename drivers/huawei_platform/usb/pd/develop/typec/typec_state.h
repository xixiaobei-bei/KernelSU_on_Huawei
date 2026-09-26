/*
 * typec_state.h
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

#ifndef _TYPEC_STATE_H_
#define _TYPEC_STATE_H_

#include <linux/types.h>
#include "typec_common.h"

enum TYPECS_WAIT_PS_STATE {
	TYPECS_WAIT_PS_DISABLE = 0,
	TYPECS_WAIT_PS_SNK_VSAFE5V,
	TYPECS_WAIT_PS_SRC_VSAFE0V,
	TYPECS_WAIT_PS_SRC_VSAFE5V,
};

struct typec_state_object {
	const char *name;
	int (*entry)(struct tcpc_device *tcpc);
	int (*cc_change)(struct tcpc_device *tcpc);
	int (*ps_change)(struct tcpc_device *tcpc);
	int (*timeout)(struct tcpc_device *tcpc, uint32_t timer_id);
};

struct typec_fsm {
	uint8_t cur_state;
	uint8_t next_state;
	uint8_t attach_type;
	uint8_t rp_present;
	uint8_t unattached_cc;
	uint8_t wait_ps;
	bool wait_drp_try;
	bool wait_pe_idle;
	bool init_with_charge;
	bool disable_pe;
	int sink_curr;
	struct typec_state_object objs[typec_state_max];
};

#define typecs_next_state(tcpc, state) ((tcpc)->fsm->next_state = (state))
#define typecs_unattached_cc(tcpc, cc) ((tcpc)->fsm->unattached_cc = (cc))
#define typecs_attach_type(tcpc, type) ((tcpc)->fsm->attach_type = (type))
#define typecs_check_wait_ps(tcpc, state) ((tcpc)->fsm->wait_ps == (state))
#define typecs_in_drp_try(tcpc) ((tcpc)->fsm->wait_drp_try = true)
#define typecs_out_drp_try(tcpc) ((tcpc)->fsm->wait_drp_try = false)
#define typecs_is_drp_try(tcpc) ((tcpc)->fsm->wait_drp_try)

#define typecs_obj_construct(state, flag) \
do { \
	tcpc->fsm->objs[state].entry = typecs_##flag##_entry; \
	tcpc->fsm->objs[state].cc_change = typecs_##flag##_cc_change; \
	tcpc->fsm->objs[state].ps_change = typecs_##flag##_ps_change; \
	tcpc->fsm->objs[state].timeout = typecs_##flag##_timeout; \
} while (0)

void typecs_set_cc_short_detection(struct tcpc_device *tcpc, bool cc1, bool cc2);
void typecs_enable_low_power(struct tcpc_device *tcpc, uint8_t pull);
void typecs_set_plug_orient(struct tcpc_device *tcpc, uint8_t res, bool polarity);
bool typecs_is_fake_ra_rp30(struct tcpc_device *tcpc);
bool typecs_is_cc_attach(struct tcpc_device *tcpc);
bool typecs_is_act_as_sink_role(struct tcpc_device *tcpc);
int typecs_get_unattached_cc(struct tcpc_device *tcpc);
int typecs_get_unattached_state(struct tcpc_device *tcpc);
void typecs_attach_state_change(struct tcpc_device *tcpc, enum typec_attach_type type);
void typecs_wait_ps_change(struct tcpc_device *tcpc, enum TYPECS_WAIT_PS_STATE state);

int typecs_snk_init(struct tcpc_device *tcpc);
int typecs_src_init(struct tcpc_device *tcpc);
int typecs_init(struct tcpc_device *tcpc);
int typecs_deinit(struct tcpc_device *tcpc);

#endif /* _TYPEC_STATE_H_ */
