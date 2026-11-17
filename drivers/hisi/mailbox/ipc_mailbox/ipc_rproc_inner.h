/*
 * ipc_rproc_inner.h
 *
 * IPC driver inner interface.
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
#ifndef _IPC_RPROC_INNER_H_
#define _IPC_RPROC_INNER_H_

int ipc_rproc_init(struct hisi_mbox_device **mdev_list);

#endif
