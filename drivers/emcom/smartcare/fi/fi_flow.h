/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: define macros and structs for fi_flow.c
* Author: liyouyong liyouyong@huawei.com
* Create: 2018-09-10
*/

#ifndef __HW_FI_FLOW_H__
#define __HW_FI_FLOW_H__

#include <linux/spinlock.h>
#include "fi.h"

#define FI_FLOW_TABLE_SIZE 0x10                     /* hash bucket size 16 */
#define FI_FLOW_TABLE_MASK (FI_FLOW_TABLE_SIZE - 1) /* hash bucket mask */
#define FI_FLOW_AGING_TIME 60000                    /* aging time ms */
#define FI_FLOW_NODE_LIMIT 128                      /* flow node up limit */

#define FI_BH3_SEQ_CACHE_NUM 4 /* cache num of bh3 */

#define FI_SHIFT_8 8
#define FI_SHIFT_4 4
#define FI_SHIFT_12 12
#define FI_SHIFT_16 16
#define FI_SHIFT_6 6
#define FI_SHIFT_10 10
#define FI_SHIFT_14 14
#define FI_SHIFT_7 7

struct fi_pkt {
	uint8_t *data;          /* payload data */
	uint16_t len;           /* payload total len, bufdatalen + extradata */
	uint16_t bufdatalen;    /* datalen in buffer (not in list) */
	uint16_t sport;         /* network byte order, big-endian */
	uint16_t dport;         /* network byte order, big-endian */
	uint8_t proto;          /* tcp or udp */
	uint8_t dir : 2;        /* SA_DIR_UP or SA_DIR_DOWN */
	uint8_t mptcp : 1;      /* mptcp or not */
	uint8_t rev : 5;
	uint16_t rev1;
	uint32_t rev2;
	uint32_t sip;           /* network byte order, big-endian */
	uint32_t dip;           /* network byte order, big-endian */
	uint32_t seq;           /* tcp seq num */
	uint32_t ack;           /* tcp ack num */
	int64_t msec;           /* time stamp of this pkt, millisecond */
};

/* only for bh3 */
struct fi_flow_bh3 {
	uint16_t seq;
	uint16_t verify;
	uint32_t time;
};

/* flow context for rtt cal */
struct fi_flowctx {
	uint8_t flowtype;
	uint8_t appid;
	uint8_t battle_times;
	uint8_t btflow_times;
	uint16_t uppktnum;
	uint16_t downpktnum;
	uint16_t hbupnum;
	uint16_t hbdownnum;
	uint32_t hbstudytime;
	uint32_t uppktbytes;
	uint32_t seq;
	int64_t studystarttime;
	int64_t flowstarttime;
	int64_t uppkttime;

	union {
		struct fi_flow_bh3 flow_bh3[FI_BH3_SEQ_CACHE_NUM];
	};
};

struct fi_flow_node {
	struct list_head list;
	uint32_t sip;
	uint32_t dip;
	uint16_t sport;
	uint16_t dport;
	uint32_t updatetime;
	struct fi_flowctx flowctx;
};

/* flow table header */
struct fi_flow_head {
	struct list_head list;
};

/* fi flow struct */
struct fi_flow {
	struct fi_flow_head flow_table[FI_FLOW_TABLE_SIZE];
	atomic_t nodenum;
	spinlock_t flow_lock;
};

void fi_flow_init(void);
void fi_flow_age(void);
void fi_flow_clear(void);
void fi_flow_lock(void);
void fi_flow_unlock(void);
uint32_t fi_flow_hash(uint32_t sip, uint32_t dip, uint32_t sport,
		uint32_t dport, uint32_t proto);
struct fi_flow_node *fi_flow_get(struct fi_pkt *pktinfo,
		struct fi_flow_head *head, uint32_t addflow);
struct fi_flow_head *fi_flow_header(uint32_t index);

#endif /* __HW_FI_FLOW_H__ */
