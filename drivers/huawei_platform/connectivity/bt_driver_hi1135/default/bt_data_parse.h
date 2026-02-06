/*
 * bt_data_parse.h
 *
 * api for bt data handle
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef BT_DATA_PARSE_H
#define BT_DATA_PARSE_H

#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "bt_common.h"
#include "bt_type.h"

void bt_send_data(uint8 channel, uint8 sid, uint8 cid, uint8 *data, uint16 len);
void bt_skb_purge(struct bt_core_s *bt_core_d);
int32 bt_skb_enqueue(struct bt_core_s *bt_core_d, struct sk_buff *skb, uint8 type);
struct sk_buff *bt_skb_dequeue(struct bt_core_s *bt_core_d, uint8 type);
void bt_core_tx_work(struct work_struct *work);
int32 bt_recv_data_cb(unsigned char service_id, unsigned char command_id, unsigned char *data, int data_len);
#endif
