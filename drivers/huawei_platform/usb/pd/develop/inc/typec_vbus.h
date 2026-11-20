/*
 * typec_vbus.h
 *
 * typec vbus
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

#ifndef _TYPEC_VBUS_H_
#define _TYPEC_VBUS_H_

#include <linux/types.h>

#define TCPC_VBUS_SOURCE_0V  0
#define TCPC_VBUS_SOURCE_5V  5000
#define TCPC_VBUS_SINK_0V    0
#define TCPC_VBUS_SINK_5V    5000

struct tcpc_device;

int typec_refresh_power_status(struct tcpc_device *tcpc);
int typec_inquire_vbus_level(struct tcpc_device *tcpc_dev, bool from_ic);
int typec_source_vbus(struct tcpc_device *tcpc, uint8_t type, int mv, int ma);
int typec_sink_vbus(struct tcpc_device *tcpc, uint8_t type, int mv, int ma);
int typec_disable_vbus_control(struct tcpc_device *tcpc);
int typec_handle_auto_discharge_timeout(struct tcpc_device *tcpc);
bool typec_check_vbus_valid(struct tcpc_device *tcpc);
bool typec_check_vbus_valid_from_ic(struct tcpc_device *tcpc);
bool typec_check_vsafe0v(struct tcpc_device *tcpc);

#endif /* _TYPEC_VBUS_H_ */
