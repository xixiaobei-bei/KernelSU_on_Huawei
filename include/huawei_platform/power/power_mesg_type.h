/*
 * power_mesg_type.h
 *
 * power netlink message head file
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _POWER_MESG_TYPE_
#define _POWER_MESG_TYPE_

#include <linux/list.h>
#include <net/genetlink.h>

#define POWER_NODE_NAME_MAX_LEN     16

enum power_mesg_target_port {
	TARGET_PORT_BEGIN,
	POWERCT_PORT = TARGET_PORT_BEGIN,
	TARGET_PORT_END,
};
#define TARGET_PORT_MAX             (TARGET_PORT_END - 1)

/* cbs is simplified identifier with callbacks */
struct power_mesg_easy_cbs {
	unsigned char cmd;
	int (*doit)(unsigned char version, void *data, int length);
};

/* cbs is simplified identifier with callbacks */
struct power_mesg_cbs {
	unsigned char cmd;
	int (*doit)(struct sk_buff *skb_in, struct genl_info *info);
};

struct power_mesg_node {
	struct list_head node;
	const unsigned int target;
	const char name[POWER_NODE_NAME_MAX_LEN];
	const struct power_mesg_easy_cbs *ops;
	const unsigned char n_ops; /* n_ops means there is n ops valid */
	const struct power_mesg_cbs *cbs;
	const unsigned char n_cbs; /* n_cbs means there is n cbs valid */
	int (*srv_on_cb)(void);
};

#endif /* _POWER_MESG_TYPE_ */
