/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: booster common interface.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2020-03-16
 */

#ifndef HW_BOOSTER_COMMON_H
#define HW_BOOSTER_COMMON_H

#include <linux/netdevice.h>
#include <linux/uidgid.h>
#include <net/sock.h>

#define IFNAME_LEN_DOUBLE 2
#define IFNAME_LEN_TRIPLE 3

#define assign_short(p, val) (*(s16 *)(p) = (val))
#define assign_int(p, val) (*(s32 *)(p) = (val))
#define assign_uint(p, val) (*(u32 *)(p) = (val))
#define assign_long(p, val) (*(s64 *)(p) = (val))
#define skip_byte(p, num) ((p) = (p) + (num))

enum ifname_array_index_enum {
	ARRAY_INDEX_0 = 0,
	ARRAY_INDEX_1 = 1,
	ARRAY_INDEX_2 = 2,
	ARRAY_INDEX_3 = 3,

	ARRAY_INDEX_BUTT
};

bool is_ds_rnic(const struct net_device *dev);
uid_t hw_get_sock_uid(struct sock *sk);

#endif
