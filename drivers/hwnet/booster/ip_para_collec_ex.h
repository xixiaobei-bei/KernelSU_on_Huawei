/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: This file is the internal header file for the
 *              TCP/IP parameter collection module.
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#ifndef _IP_PARA_COLLEC_EX_H
#define _IP_PARA_COLLEC_EX_H

#include <linux/skbuff.h>

#include "netlink_handle.h"

/* call by tcp/ip stack */
void collec_update_buf_time(struct sk_buff *skb, u8 protocal);
/* initialization function for external modules */
msg_process *ip_para_collec_init(notify_event *fn);
void ip_para_collec_exit(void);
void send_buf_to_chr(u32 tcp_buf, u32 udp_buf);

#endif // _IP_PARA_COLLEC_EX_H
