/*
 * typec_legacy_cable.h
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

#ifndef _TYPEC_LEGACY_CABLE_H_
#define _TYPEC_LEGACY_CABLE_H_

#include <linux/types.h>

#define TYPEC_LEGACY_CABLE_RETRYS      2
#define TYPEC_LEGACY_CABLE_SUSPECT_THD 2

enum typec_legacy_cable_type {
	TYPEC_LEGACY_NONE = 0,
	TYPEC_LEGACY_CABLE1,
	TYPEC_LEGACY_CABLE2,
};

struct tcpc_device;

void typec_legacy_init(struct tcpc_device *tcpc);
bool typec_legacy_check_cable(struct tcpc_device *tcpc);
bool typec_legacy_cc_change(struct tcpc_device *tcpc);
bool typec_legacy_ps_change(struct tcpc_device *tcpc);
void typec_legacy_attach_state_change(struct tcpc_device *tcpc);
void typec_legacy_handle_detach(struct tcpc_device *tcpc);
void typec_legacy_timeout(struct tcpc_device *tcpc, uint32_t timer_id);

#endif /* _TYPEC_LEGACY_CABLE_H_ */
