/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the internal header file for the
 *              network slice management module.
 * Author: yuxiaoliang@huawei.com
 * Create: 2019-06-20
 */

#ifndef _NETWORK_SLICE_MANAGEMENT_H
#define _NETWORK_SLICE_MANAGEMENT_H

#include <linux/spinlock.h>
#include <linux/skbuff.h>

#include "netlink_handle.h"

struct uid_process_bind_req {
	u16 type;
	u16 len;
	u32 net_id;
	u32 uid;
};

struct del_uid_bind_req {
	u16 type;
	u16 len;
	u32 uid;
};

struct ip_para_rpt_ctrl_req {
	u16 type;
	u16 len;
	u32 enable;
};

msg_process *network_slice_management_init(notify_event *fn);
void slice_ip_para_report(struct sk_buff *skb, unsigned short family);

#endif
