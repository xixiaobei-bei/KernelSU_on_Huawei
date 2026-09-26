/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the internal header file for the
 *              TCP parameter collection module.
 * Author: liuleimin1@huawei.com
 * Create: 2019-04-18
 */

#ifndef _TCP_PARA_COLLEC_H
#define _TCP_PARA_COLLEC_H

#include <linux/netfilter.h>

#include "netlink_handle.h"

#define NOTIFY_BUF_LEN 512

void booster_update_tcp_statistics(u_int8_t af, struct sk_buff *skb,
	struct net_device *in, struct net_device *out);
msg_process *tcp_para_collec_init(notify_event *fn);
#endif
