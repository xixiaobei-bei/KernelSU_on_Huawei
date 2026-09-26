/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: flow table of fi module
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
 */
#include "fi_flow.h"
#include "securec.h"

struct fi_flow g_fi_flow;

void fi_flow_lock(void)
{
	spin_lock_bh(&(g_fi_flow.flow_lock));
}

void fi_flow_unlock(void)
{
	spin_unlock_bh(&(g_fi_flow.flow_lock));
}

uint32_t fi_flow_hash(uint32_t sip, uint32_t dip, uint32_t sport,
	uint32_t dport, uint32_t proto)
{
	uint32_t v1;
	uint32_t v2;
	uint32_t h1;

	v1 = sip ^ dip;
	v2 = sport ^ dport;

	h1 = v1 << FI_SHIFT_8;
	h1 ^= v1 >> FI_SHIFT_4;
	h1 ^= v1 >> FI_SHIFT_12;
	h1 ^= v1 >> FI_SHIFT_16;
	h1 ^= v2 << FI_SHIFT_6;
	h1 ^= v2 << FI_SHIFT_10;
	h1 ^= v2 << FI_SHIFT_14;
	h1 ^= v2 << FI_SHIFT_7;
	h1 += proto;

	return h1 & FI_FLOW_TABLE_MASK;
}

static bool fi_is_same_flow(struct fi_flow_node *node, struct fi_pkt *pktinfo)
{
	if (node->sip == pktinfo->sip &&
		node->dip == pktinfo->dip &&
		node->sport == pktinfo->sport &&
		node->dport == (pktinfo)->dport)
		return true;

	if (node->sip == pktinfo->dip &&
		node->dip == pktinfo->sip &&
		node->sport == pktinfo->dport &&
		node->dport == pktinfo->sport)
		return true;
	return false;
}

struct fi_flow_node *fi_flow_find(struct fi_pkt *pktinfo,
	struct fi_flow_head *head)
{
	struct fi_flow_node *node = NULL;

	list_for_each_entry(node, &(head->list), list) {
		if (fi_is_same_flow(node, pktinfo))
			return node;
	}

	return NULL;
}

struct fi_flow_node *fi_flow_add(struct fi_pkt *pktinfo,
	struct fi_flow_head *head)
{
	struct fi_flow_node *newnode = NULL;

	/* flow size is out of limit */
	if (atomic_read(&g_fi_flow.nodenum) > FI_FLOW_NODE_LIMIT) {
		fi_logd(" : FI flow node out of limit");
		return NULL;
	}

	newnode = fi_malloc(sizeof(*newnode));
	if (newnode == NULL) {
		fi_logd(" : FI failed to malloc for new fi flow node");
		return NULL;
	}

	newnode->sip = pktinfo->sip;
	newnode->dip = pktinfo->dip;
	newnode->sport = pktinfo->sport;
	newnode->dport = pktinfo->dport;
	newnode->updatetime = jiffies_to_msecs(jiffies);

	list_add(&(newnode->list), &(head->list));
	atomic_inc(&g_fi_flow.nodenum);

	fi_logd(" : FI new flow node, port %u:%u",
		pktinfo->sport, pktinfo->dport);

	return newnode;
}

struct fi_flow_node *fi_flow_get(struct fi_pkt *pktinfo,
	struct fi_flow_head *head, uint32_t addflow)
{
	struct fi_flow_node *node = NULL;

	node = fi_flow_find(pktinfo, head);
	if (node != NULL) {
		node->updatetime = jiffies_to_msecs(jiffies);
		return node;
	}

	if (addflow)
		return fi_flow_add(pktinfo, head);
	else
		return NULL;
}

struct fi_flow_head *fi_flow_header(uint32_t index)
{
	index = index & FI_FLOW_TABLE_MASK;
	return &(g_fi_flow.flow_table[index]);
}

void fi_flow_init(void)
{
	int i;

	(void)memset_s(&g_fi_flow, sizeof(g_fi_flow), 0, sizeof(g_fi_flow));
	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++)
		INIT_LIST_HEAD(&(g_fi_flow.flow_table[i].list));
	spin_lock_init(&g_fi_flow.flow_lock);
}

void fi_flow_age(void)
{
	int i;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	uint32_t curtime = jiffies_to_msecs(jiffies);

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_flow.flow_table + i;
		if (list_empty(&(head->list)))
			continue;

		fi_flow_lock();
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			if (curtime - node->updatetime > FI_FLOW_AGING_TIME) {
				list_del(&(node->list));
				fi_free(node, sizeof(*node));
				atomic_dec(&g_fi_flow.nodenum);
			}
		}
		fi_flow_unlock();
	}
}

void fi_flow_clear(void)
{
	int i;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;

	/* Traversing hash bucket */
	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_flow.flow_table + i;

		/* Traversing flow node */
		if (list_empty(&(head->list)))
			continue;

		fi_flow_lock();
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			list_del(&(node->list));
			fi_free(node, sizeof(*node));
			atomic_dec(&g_fi_flow.nodenum);
		}
		fi_flow_unlock();
	}
}
