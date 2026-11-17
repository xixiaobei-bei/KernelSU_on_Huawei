/*
 * tcpm_dual_role.h
 *
 * tcpm dual role
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

#ifndef _TCPM_DUAL_ROLE_H_
#define _TCPM_DUAL_ROLE_H_

#include <linux/device.h>
#include <linux/usb/class-dual-role.h>

#define TCPM_DUAL_ROLE_EXT_NAME_LEN 11

struct tcpc_device;

struct tcpm_dual_role {
	struct dual_role_phy_instance *phy;
	uint8_t supported_modes;
	uint8_t mode;
	uint8_t pr;
	uint8_t dr;
	uint8_t vconn_supply;
};

#ifdef CONFIG_HW_DUAL_ROLE_USB_INTF
int tcpm_dual_role_init(struct tcpc_device *tcpc);
void tcpm_dual_role_deinit(struct tcpc_device *tcpc);
void tcpm_dual_role_set_mode(struct tcpc_device *tcpc, uint8_t mode);
void tcpm_dual_role_set_pr(struct tcpc_device *tcpc, uint8_t pr);
void tcpm_dual_role_set_vconn(struct tcpc_device *tcpc, uint8_t vconn);
void tcpm_dual_role_set_sink(struct tcpc_device *tcpc);
void tcpm_dual_role_set_src(struct tcpc_device *tcpc);
void tcpm_dual_role_set_dft(struct tcpc_device *tcpc);
void tcpm_dual_role_changed(struct tcpc_device *tcpc);
#else
static inline int tcpm_dual_role_init(struct tcpc_device *tcpc)
{
	return 0;
}

static inline void tcpm_dual_role_deinit(struct tcpc_device *tcpc)
{
}

static inline void tcpm_dual_role_set_mode(struct tcpc_device *tcpc,
	uint8_t mode)
{
}

static inline void tcpm_dual_role_set_pr(struct tcpc_device *tcpc, uint8_t pr)
{
}

static inline void tcpm_dual_role_set_vconn(struct tcpc_device *tcpc,
	uint8_t vconn)
{
}

static inline void tcpm_dual_role_set_sink(struct tcpc_device *tcpc)
{
}

static inline void tcpm_dual_role_set_src(struct tcpc_device *tcpc)
{
}

static inline void tcpm_dual_role_set_dft(struct tcpc_device *tcpc)
{
}

static inline void tcpm_dual_role_changed(struct tcpc_device *tcpc)
{
}
#endif /* CONFIG_HW_DUAL_ROLE_USB_INTF */

#endif /* _TCPM_DUAL_ROLE_H_ */
