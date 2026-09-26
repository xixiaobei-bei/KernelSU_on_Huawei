/*
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
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

#ifndef _TCPCI_TYPEC_H
#define _TCPCI_TYPEC_H

#include "include/tcpci.h"

struct tcpc_device;

#define HISI_TCPC_ALERT_STATUS_ZERO	0x5A5A
#define HISI_TCPC_ALERT_I2C_TIMEOUT	0x5B5B
#define HISI_TCPC_OTHER_IRQ_MAX_LOOPS	3000
int hisi_tcpc_typec_handle_cc_change(struct tcpc_device *tcpc_dev);
int hisi_tcpc_typec_handle_ps_change(struct tcpc_device *tcpc_dev,
		int vbus_level);
int hisi_tcpc_typec_handle_timeout(struct tcpc_device *tcpc_dev,
		uint32_t timer_id);
int hisi_tcpc_typec_set_rp_level(struct tcpc_device *tcpc_dev, uint8_t res);
int hisi_tcpc_typec_change_role(struct tcpc_device *tcpc_dev,
		uint8_t typec_role);
void hisi_tcpc_typec_force_unattach(struct tcpc_device *tcpc_dev);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
int hisi_tcpc_typec_advertise_explicit_contract(struct tcpc_device *tcpc_dev);
int hisi_tcpc_typec_handle_pe_pr_swap(struct tcpc_device *tcpc_dev);
#endif

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
int hisi_tcpc_typec_swap_role(struct tcpc_device *tcpc_dev);
#endif

int hisi_tcpc_typec_set_direct_charge(struct tcpc_device *tcpc_dev,
		bool direct_charge);

int hisi_tcpc_typec_init(struct tcpc_device *tcpc, uint8_t typec_role);

#endif
