/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is performance statistics for
 *              data transmission head file.
 * Author: lianchaofeng@huawei.com
 * Create: 2019-12-10
 */

#ifndef HW_PACKET_TRACKER_H
#define HW_PACKET_TRACKER_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "netlink_handle.h"

#include <net/sock.h>
#define PROC_BITMASK_SETTED 0xa0
#define PROC_BITMASK_SET_MASK 0xff
#define PROC_BITMASK_RNIC_UL_OUT (1 << 1)

struct hw_kernel_delay_info {
	/* kernel send data to modem cost more time than threshold */
	u32 uplink_delay_count;
	u32 uplink_data_count;
	u32 uplink_time_cost_ave;

	u32 downlink_delay_cnt;
	u32 downlink_data_count;
	u32 downlink_time_cost_ave;

	/* downlink data stay too long in receive buffer */
	u32 downlink_out_wait_cnt;
	u32 uplink_abnormal_not_report_cnt;
	u32 downlink_abnormal_not_report_cnt;
};

void hw_pt_set_skb_stamp(struct sk_buff *skb);
void hw_pt_dev_uplink(struct sk_buff *skb, struct net_device *dev);
void hw_pt_l4_downlink_in(struct sock *skt, struct sk_buff *skb, int eaten);
void hw_pt_l4_downlink_out(struct sock *skt, struct sk_buff *skb);
msg_process* __init hw_pt_init(notify_event *notify);

#endif
