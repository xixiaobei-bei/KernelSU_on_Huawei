/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: send tcp reset on iface head file.
 * Author: zhuweichen@huawei.com
 * Create: 2019-10-24
 */

#ifndef _SOCK_DESTROY_HANDLER_H
#define _SOCK_DESTROY_HANDLER_H

#include "netlink_handle.h"

msg_process *sock_destroy_handler_init(notify_event *fn);

#endif /* _SOCK_DESTROY_HANDLER_H */
