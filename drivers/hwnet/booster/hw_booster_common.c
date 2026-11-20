/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: booster common interface.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2020-03-16
 */

#include "hw_booster_common.h"

#include <linux/highuid.h>

/* when modify mobile_ifaces array, plz modify the IFACE_NUM simultaneously */
#define IFACE_NUM 15
static const char mobile_ifaces[IFACE_NUM][IFNAMSIZ] __attribute__((aligned(sizeof(long)))) = {
	/* hisi, sort by frequency of use:rmnet0 > rmnet3 > rmnet1 > ... */
	"rmnet0", "rmnet3", "rmnet1", "rmnet4", "rmnet2", "rmnet5", "rmnet6",
	/* qcom */
	"rmnet_data0", "rmnet_data1", "rmnet_data2", "rmnet_data3",
	/* mtk */
	"ccmni0", "ccmni1", "ccmni2", "ccmni3",
};

/* reference to ifname_compare_aligned in x_tables.h */
static bool ifname_equal(const char *if1, size_t if1_len,
	const char *if2, size_t if2_len)
{
	const unsigned long *p = (const unsigned long *)if1;
	const unsigned long *q = (const unsigned long *)if2;
	unsigned long ret;

	if (if1_len != if2_len)
		return false;
	if (if1_len == 0)
		return false;

	ret = (p[ARRAY_INDEX_0] ^ q[ARRAY_INDEX_0]);
	if (ret != 0)
		return false;
	if (if1_len > sizeof(unsigned long))
		ret = (p[ARRAY_INDEX_1] ^ q[ARRAY_INDEX_1]);
	if (ret != 0)
		return false;
	if (if1_len > IFNAME_LEN_DOUBLE * sizeof(unsigned long))
		ret = (p[ARRAY_INDEX_2] ^ q[ARRAY_INDEX_2]);
	if (ret)
		return false;
	if (if1_len > IFNAME_LEN_TRIPLE * sizeof(unsigned long))
		ret = (p[ARRAY_INDEX_3] ^ q[ARRAY_INDEX_3]);
	return (ret == 0L);
}

bool is_ds_rnic(const struct net_device *dev)
{
	int i;

	if (!dev)
		return false;
	for (i = 0; i < IFACE_NUM; ++i) {
		if (ifname_equal(dev->name, sizeof(dev->name),
			mobile_ifaces[i], sizeof(mobile_ifaces[i])))
			return true;
	}
	return false;
}

uid_t hw_get_sock_uid(struct sock *sk)
{
	kuid_t kuid;

	if (!sk || !sk_fullsock(sk))
		return overflowuid;
	kuid = sock_net_uid(sock_net(sk), sk);
	return from_kuid_munged(sock_net(sk)->user_ns, kuid);
}
