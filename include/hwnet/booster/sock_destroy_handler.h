/*
 * sock_destroy_handler.h
 *
 * send tcp reset on iface
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#ifndef _SOCK_DESTROY_HANDLER_H
#define _SOCK_DESTROY_HANDLER_H

#include "netlink_handle.h"

msg_process *sock_destroy_handler_init(notify_event *fn);

#endif	/* _SOCK_DESTROY_HANDLER_H */
