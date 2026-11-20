/*
 * typec_lpm.h
 *
 * typec low power mode
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

#ifndef _TYPEC_LPM_H_
#define _TYPEC_LPM_H_

#include <linux/types.h>

struct typec_lpm;
struct tcpc_device;

void typec_init_lpm(struct typec_lpm **lpm, struct tcpc_device *tcpc);
void typec_deinit_lpm(struct typec_lpm **lpm);
void typec_set_cable_only(struct typec_lpm *lpm, bool only);
void typec_set_watchdog(struct typec_lpm *lpm, bool en);
void typec_attach_wake_lock(struct typec_lpm *lpm);
void typec_set_wake_lock_pd(struct typec_lpm *lpm, bool pd_lock);
void typec_enable_lpm_timer(struct typec_lpm *lpm);
void typec_handle_lpm_timeout(struct typec_lpm *lpm);
void typec_enable_lpm(struct typec_lpm *lpm, uint8_t pull);
void typec_disable_lpm(struct typec_lpm *lpm);
void typec_enter_lpm_again(struct typec_lpm *lpm);
void typec_enable_wakeup_timer(struct typec_lpm *lpm, bool en);

#endif /* _TYPEC_LPM_H_ */
