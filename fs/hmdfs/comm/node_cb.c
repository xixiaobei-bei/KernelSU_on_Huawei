// SPDX-License-Identifier: GPL-2.0
/*
 * node_cb.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: houtao1@huawei.com
 * Author: yebin10@huawei.com
 * Create: 2021-01-08
 *
 */
#include <linux/list.h>

#include "node_cb.h"
#include "connection.h"

static struct list_head cb_head[NODE_EVT_NR][NODE_EVT_TYPE_NR];

static const char *evt_str_tbl[NODE_EVT_NR] = {
	"add", "online", "offline", "del",
};

static inline bool hmdfs_is_valid_node_evt(int evt)
{
	return (evt >= 0 && evt < NODE_EVT_NR);
}

static const char *hmdfs_evt_str(int evt)
{
	if (!hmdfs_is_valid_node_evt(evt))
		return "unknown";
	return evt_str_tbl[evt];
}

void hmdfs_node_evt_cb_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cb_head); i++) {
		int j;

		for (j = 0; j < ARRAY_SIZE(cb_head[0]); j++)
			INIT_LIST_HEAD(&cb_head[i][j]);
	}
}

void hmdfs_node_add_evt_cb(struct hmdfs_node_cb_desc *desc, int nr)
{
	int i;

	for (i = 0; i < nr; i++) {
		int evt = desc[i].evt;
		bool sync = desc[i].sync;

		if (!hmdfs_is_valid_node_evt(evt))
			continue;

		list_add_tail(&desc[i].list, &cb_head[evt][sync]);
	}
}

void hmdfs_node_call_evt_cb(struct hmdfs_peer *conn, int evt, bool sync,
			    unsigned int seq)
{
	struct hmdfs_node_cb_desc *desc = NULL;

	hmdfs_info("node 0x%x:0x%llx call %s %s cb seq %u",
		   conn->owner, conn->device_id, hmdfs_evt_str(evt),
		   sync ? "sync" : "async", seq);

	if (!hmdfs_is_valid_node_evt(evt))
		return;

	list_for_each_entry(desc, &cb_head[evt][sync], list) {
		if (conn->version < desc->min_version)
			continue;

		desc->fn(conn, evt, seq);
	}
}
