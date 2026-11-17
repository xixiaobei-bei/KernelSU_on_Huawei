/*
 * typec_state_machine.h
 *
 * typec state machine
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

#ifndef _TYPEC_STATE_MACHINE_H_
#define _TYPEC_STATE_MACHINE_H_

#include <linux/types.h>

struct tcpc_device;

int typecs_fsm_init(struct tcpc_device *tcpc);
int typecs_fsm_deinit(struct tcpc_device *tcpc);
int typecs_handle_cc_change(struct tcpc_device *tcpc);
int typecs_handle_ps_change(struct tcpc_device *tcpc_dev);
int typecs_handle_timeout(struct tcpc_device *tcpc, uint32_t timer_id);

void typecs_notfiy_pe_idle(struct tcpc_device *tcpc);
bool typecs_is_wait_pe_idle(struct tcpc_device *tcpc);
void typecs_set_sink_curr(struct tcpc_device *tcpc, int curr);
void typecs_typec_error_recovery(struct tcpc_device *tcpc);
void typecs_typec_disable(struct tcpc_device *tcpc);
void typecs_typec_enable(struct tcpc_device *tcpc);
void typecs_pe_disable(struct tcpc_device *tcpc);
void typecs_pe_enable(struct tcpc_device *tcpc);
void typecs_handle_pe_pr_swap(struct tcpc_device *tcpc);
void typecs_change_typec_role(struct tcpc_device *tcpc);
void typecs_swap_typec_role(struct tcpc_device *tcpc);
uint8_t typecs_get_typec_state(struct tcpc_device *tcpc);
uint8_t typecs_get_attach_type(struct tcpc_device *tcpc);

#endif /* _TYPEC_STATE_MACHINE_H_ */
