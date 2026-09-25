/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: This module is a map utils for FI
 * Author: jupeng.zhang@huawei.com
 */

#ifndef _FI_UTILS_H
#define _FI_UTILS_H

#include <linux/types.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/spinlock_types.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG EMCOM_FI_KERNEL

HWLOG_REGIST();

#define IP_DEBUG 0

#define IP6_ADDR_BLOCK1(ip6_addr) (ntohs((ip6_addr).s6_addr16[0]) & 0xffff)
#define IP6_ADDR_BLOCK2(ip6_addr) (ntohs((ip6_addr).s6_addr16[1]) & 0xffff)
#define IP6_ADDR_BLOCK3(ip6_addr) (ntohs((ip6_addr).s6_addr16[2]) & 0xffff)
#define IP6_ADDR_BLOCK4(ip6_addr) (ntohs((ip6_addr).s6_addr16[3]) & 0xffff)
#define IP6_ADDR_BLOCK5(ip6_addr) (ntohs((ip6_addr).s6_addr16[4]) & 0xffff)
#define IP6_ADDR_BLOCK6(ip6_addr) (ntohs((ip6_addr).s6_addr16[5]) & 0xffff)
#define IP6_ADDR_BLOCK7(ip6_addr) (ntohs((ip6_addr).s6_addr16[6]) & 0xffff)
#define IP6_ADDR_BLOCK8(ip6_addr) (ntohs((ip6_addr).s6_addr16[7]) & 0xffff)

#if IP_DEBUG
#define IPV4_FMT "%u.%u.%u.%u"
#define IPV4_INFO(addr) \
    ((unsigned char *)&(addr))[0], \
    ((unsigned char *)&(addr))[1], \
    ((unsigned char *)&(addr))[2], \
    ((unsigned char *)&(addr))[3]

#define IPV6_FMT "%x:%x:%x:%x:%x:%x:%x:%x"
#define IPV6_INFO(addr) \
    IP6_ADDR_BLOCK1(addr), \
    IP6_ADDR_BLOCK2(addr), \
    IP6_ADDR_BLOCK3(addr), \
    IP6_ADDR_BLOCK4(addr), \
    IP6_ADDR_BLOCK5(addr), \
    IP6_ADDR_BLOCK6(addr), \
    IP6_ADDR_BLOCK7(addr), \
    IP6_ADDR_BLOCK8(addr)

#else
#define IPV4_FMT "*.*.%u.%u"
#define IPV4_INFO(addr) \
    ((unsigned char *)&(addr))[2], \
    ((unsigned char *)&(addr))[3]

#define IPV6_FMT "%x:***:%x"
#define IPV6_INFO(addr) \
    IP6_ADDR_BLOCK1(addr), \
    IP6_ADDR_BLOCK8(addr)
#endif


#define fi_logd(fmt, ...) \
	do { \
		hwlog_debug("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

#define fi_logi(fmt, ...) \
	do { \
		hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

#define fi_loge(fmt, ...) \
	do { \
		hwlog_err("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

struct sk_buff* fi_get_netlink_skb(int type, int len, char **data);
void fi_enqueue_netlink_skb(struct sk_buff *pskb);
int fi_netlink_thread(void* data);
void fi_empty_netlink_skb_queue(void);

#endif //_FI_UTILS_H
