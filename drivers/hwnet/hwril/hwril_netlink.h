/*
 * hwril_netlink.h
 *
 * generic netlink for hwril
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

#ifndef __HWRIL_NETLINK_H
#define __HWRIL_NETLINK_H
#include <linux/types.h>

#define HWRIL_GENL_FAMILY "hwril"

enum {
	HWRIL_CMD_UNSPEC,
	HWRIL_CMD_VSIM_SET_IFNAME,
	HWRIL_CMD_VSIM_RESET_IFNAME,
	HWRIL_CMD_VSIM_RESET_COUNTER,
	HWRIL_CMD_VSIM_GET_FLOW,

	HWRIL_CMD_VSIM_NOTIFY,
	__HWRIL_CMD_MAX,
};

#define HWRIL_CMD_MAX (__HWRIL_CMD_MAX - 1)

enum {
	HWRIL_ATTR_UNSPEC,
	HWRIL_ATTR_VSIM_IFNAME,
	HWRIL_ATTR_VSIM_TX_BYTES,
	HWRIL_ATTR_VSIM_RX_BYTES,
	HWRIL_ATTR_VSIM_TEE_TASK,
	HWRIL_ATTR_PAD,
	__HWRIL_ATTR_MAX,
};

#define HWRIL_ATTR_MAX (__HWRIL_ATTR_MAX - 1)

int hwril_nl_vsim_flow_notify(u64 tx_bytes, u64 rx_bytes);
int hwril_nl_vsim_tee_notify(u32 task_type);
#endif	/* __HWRIL_NETLINK_H */
