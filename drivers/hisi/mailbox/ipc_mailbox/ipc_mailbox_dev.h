/*
 *
 * IPC mailbox device driver implement.
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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
#ifndef __IPC_MAILBOX_DEV_H__
#define __IPC_MAILBOX_DEV_H__

enum ipc_version {
	IPC_VERSION_APPLE = 0,
	IPC_VERSION_BOY
};
#define IPC_REG_SPACE       6
#define IPC_REG_SPACE_EX    7

#define ipc_bitmask(n) (1 << (n))
#define ipc_mbx_source(mdev, spc) ((mdev) << (spc))
#define ipc_mbx_dset(mdev, spc) (((mdev) << (spc)) + 0x04)
#define ipc_mbx_dclr(mdev, spc) (((mdev) << (spc)) + 0x08)
#define ipc_mbx_dstatus(mdev, spc) (((mdev) << (spc)) + 0x0C)
#define ipc_mbx_mode(mdev, spc) (((mdev) << (spc)) + 0x10)
#define ipc_mbx_imask(mdev, spc) (((mdev) << (spc)) + 0x14)
#define ipc_mbx_iclr(mdev, spc) (((mdev) << (spc)) + 0x18)
#define ipc_mbx_send(mdev, spc) (((mdev) << (spc)) + 0x1C)
#define ipc_mbx_data(mdev, index, spc) \
	(((mdev) << (spc)) + 0x20 + ((index) << 2))
#define ipc_lock(ipc_version) \
	(((ipc_version) == IPC_VERSION_BOY) ? 0xA000 : 0xA00)

#define ipc_cpux_imst(cpu) (((cpu) << 3) + 0x800)
#define ipc_cpux_irst(cpu) (((cpu) << 3) + 0x804)
#define ipc_cpux_imst_ex(cpu, region) (((cpu) << 3) + 0x8000 + (region << 8))
#define ipc_cpux_irst_ex(cpu, mdev) \
	(((cpu) << 3) + 0x8004 + (((mdev) / 32) << 8))

#endif /* __IPC_MAILBOX_DEV_H__ */
