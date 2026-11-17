/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Himos TCP stats head file.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2018-07-19
 */

#ifndef _HW_HIMOS_TCP_STATS_H
#define _HW_HIMOS_TCP_STATS_H

#include <net/sock.h>

#include <linux/socket.h>

void himos_tcp_stats(struct sock *sk, struct msghdr *old, struct msghdr *new,
	int inbytes, int outbytes);


#endif
