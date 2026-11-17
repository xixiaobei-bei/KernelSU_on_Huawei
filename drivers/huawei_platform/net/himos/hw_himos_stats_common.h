/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Himos stats common head file.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2018-07-19
 */

#ifndef _HW_HIMOS_STATS_COMMON_H
#define _HW_HIMOS_STATS_COMMON_H

#include <linux/types.h>
#include <linux/list.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/ktime.h>

#define HIMOS_US_MS 1000
#define HIMOS_BYTE_OFFSET 44

struct himos_aweme_id {
	__s32 uid;
	__u32 portid;
};

struct himos_stats_common {
	__u32 portid; /* user portid */
	possible_net_t net; /* for reply */
	struct list_head node;
	int type; /* the value of 'enum HIMOS_STATS_TYPE' */
	__s32 uid;
};

int himos_stats_common_init(void) __init;

struct himos_stats_common *himos_get_stats_info_by_uid(__s32 uid);
void himos_stats_common_reset(struct himos_stats_common *info);

#endif
