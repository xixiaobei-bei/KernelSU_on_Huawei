/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Himos UDP stats head file.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2018-07-19
 */

#ifndef _HW_HIMOS_UDP_STATS_H
#define _HW_HIMOS_UDP_STATS_H

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/genetlink.h>
#include <net/genetlink.h>

int himos_start_udp_stats(struct genl_info *info);
int himos_stop_udp_stats(const struct genl_info *info);
int himos_keepalive_udp_stats(const struct genl_info *info);

void himos_udp_stats(const struct sock *sk, __u32 in_bytes, __u32 out_bytes);

int himos_udp_stats_init(void) __init;

#endif

