/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
* Description: This module is a map utils for FI
* Author: songqiubin songqiubin@huawei.com
* Create: 2019-09-18
*/

#ifndef _FI_UTILS_H
#define _FI_UTILS_H

#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/spinlock_types.h>
#include <linux/types.h>
#include "huawei_platform/log/hw_log.h"
#include "securec.h"

#undef HWLOG_TAG
#define HWLOG_TAG EMCOM_FI_KERNEL

HWLOG_REGIST();

#define IP_DEBUG 0

#define ip6_addr_block1(ip6_addr) (ntohs((ip6_addr).s6_addr16[0]) & 0xffff)
#define ip6_addr_block2(ip6_addr) (ntohs((ip6_addr).s6_addr16[1]) & 0xffff)
#define ip6_addr_block3(ip6_addr) (ntohs((ip6_addr).s6_addr16[2]) & 0xffff)
#define ip6_addr_block4(ip6_addr) (ntohs((ip6_addr).s6_addr16[3]) & 0xffff)
#define ip6_addr_block5(ip6_addr) (ntohs((ip6_addr).s6_addr16[4]) & 0xffff)
#define ip6_addr_block6(ip6_addr) (ntohs((ip6_addr).s6_addr16[5]) & 0xffff)
#define ip6_addr_block7(ip6_addr) (ntohs((ip6_addr).s6_addr16[6]) & 0xffff)
#define ip6_addr_block8(ip6_addr) (ntohs((ip6_addr).s6_addr16[7]) & 0xffff)

#if IP_DEBUG
#define IPV4_FMT "%u.%u.%u.%u"
#define ipv4_info(addr) \
	((unsigned char *)&(addr))[0], \
	((unsigned char *)&(addr))[1], \
	((unsigned char *)&(addr))[2], \
	((unsigned char *)&(addr))[3]

#define IPV6_FMT "%x:%x:%x:%x:%x:%x:%x:%x"
#define ipv6_info(addr) \
	ip6_addr_block1(addr), \
	ip6_addr_block2(addr), \
	ip6_addr_block3(addr), \
	ip6_addr_block4(addr), \
	ip6_addr_block5(addr), \
	ip6_addr_block6(addr), \
	ip6_addr_block7(addr), \
	ip6_addr_block8(addr)

#else
#define IPV4_FMT "%u.%u.*.*"
#define ipv4_info(addr) \
	((unsigned char *)&(addr))[0], \
	((unsigned char *)&(addr))[1]

#define IPV6_FMT "%x:***:%x"
#define ipv6_info(addr) \
	ip6_addr_block1(addr), \
	ip6_addr_block8(addr)
#endif


#define fi_logd(fmt, ...) do \
		hwlog_debug("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	while (0)

#define fi_logi(fmt, ...) do \
		hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	while (0)

#define fi_loge(fmt, ...) do \
		hwlog_err("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	while (0)

struct sk_buff* fi_get_netlink_skb(int type, int len, char **data);
void fi_enqueue_netlink_skb(struct sk_buff *pskb);
int fi_netlink_thread(void* data);
void fi_empty_netlink_skb_queue(void);

#endif // _FI_UTILS_H
