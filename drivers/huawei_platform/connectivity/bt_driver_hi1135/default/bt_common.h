/*
 * bt_common.h
 *
 * general macro and data structure for bt hci driver adapter
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

#ifndef BT_COMMON_H
#define BT_COMMON_H

#include <linux/ioctl.h>
#include <linux/time.h>
#include <linux/timer.h>
#include "bt_type.h"

/* invalid parameter */
#ifdef EINVAL
#undef EINVAL
#endif
#define EINVAL 22
#define EOK 0

#define BT_NV_IO 0x31
#define BT_OPERATION_NV _IOWR(BT_NV_IO, 0x1, bt_trans_t)
#define BT_OPERATION_XO_CAP _IOWR(BT_NV_IO, 0x2, bt_trans_t)

#define BT_TX_MAX_FRAME 3000  /* tx bt data max lenth */
#define BT_RX_MAX_FRAME 3000  /* RX bt data max lenth */
#define RX_BT_QUE_MAX_NUM 50
#define TX_BT_QUE_MAX_NUM 50
#define BT_SERVICE_ID 0x8B
#define BT_COMMAND_ID 0x0A
#define BT_CID_RESET_TYPE 0x0C
#define BT_SWITCH_STATE_COMMAND_ID 0x0D
#define BT_SWITCH_STATE_TYPE 0x01
#define BT_SWITCH_STATE_TLV_LEN 0x04
#define BT_SWITCH_STATE_VALUE_LEN 0x01
#define BT_SWITCH_STATE_VALUE_INDEX 0x03
#define BT_SWITCH_ON 0x01
#define BT_SWITCH_OFF 0x00
#define BT_FACTORY_TEST_ID 0x13
#define BT_NV_RW_ID 0x30
#define BT_XO_CAP_RW_ID 0x32
#define BT_MCU_CHANNEL BT_A_CAHNNEL
#define MAX_RETRY_COUNTS 5

enum data_queue {
	TX_BT_QUEUE,
	RX_BT_QUEUE,
};

struct bt_core_s {
	struct sk_buff_head rx_queue;
	wait_queue_head_t rx_wait; /* wait queue for rx packet */
	struct sk_buff_head tx_queue;
	struct workqueue_struct *bt_tx_workqueue; /* define for new workqueue */
	struct work_struct tx_skb_work; /* define new work struct */
	uint64 rx_pkt_num;
	uint64 tx_pkt_num;
	struct timer_list data_timer;
	struct work_struct timer_work; /* timer expire work queque */
	uint8 handshark_times;
};

struct bt_data_combination {
	uint8 len;
	uint8 type;
	uint16 resverd;
};

struct bt_sdio_state_s {
	uint64 sdio_tx_pkt_num;
	uint64 sdio_rx_pkt_num;
};

typedef struct bt_trans {
	unsigned char operation;
	unsigned char data_id;
	unsigned char length_l;
	unsigned char length_h;
	unsigned char data[];
} bt_trans_t;

#endif
