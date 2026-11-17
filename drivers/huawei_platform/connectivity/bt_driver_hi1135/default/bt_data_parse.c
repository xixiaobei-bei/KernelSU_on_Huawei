/*
 * bt_data_parse.c
 *
 * code for bt hci driver rev and send data parse
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

#include "bt_data_parse.h"

#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include "securec.h"
#include "bt_common.h"
#include "bt_debug.h"
#include "ext_sensorhub_api.h"

STATIC struct bt_sdio_state_s g_sdio_state = {0};

void bt_send_data(uint8 channel, uint8 sid, uint8 cid, uint8 *data, uint16 len)
{
	int32 ret;
	int8 retry_count = 0;
	struct command command_data;

	command_data.service_id = sid;
	command_data.command_id = cid;
	command_data.send_buffer = data;
	command_data.send_buffer_len = len;
	while (((ret = send_command(channel, &command_data, 0, NULL)) < 0) && (retry_count <= MAX_RETRY_COUNTS)) {
		BT_PRINT_ERR("bt_send_data FAIL! REASON = %d\n", ret);
		/* send fail, send continue later */
		msleep(1);
		retry_count++;
		continue;
	}
}

void bt_skb_purge(struct bt_core_s *bt_core_d)
{
	if (unlikely(bt_core_d == NULL)) {
		BT_PRINT_ERR("bt_skb_purge bt_core_d is NULL\n");
		return;
	}
	BT_PRINT_INFO("hw_bt_release: bt_skb_purge\n");
	skb_queue_purge(&bt_core_d->tx_queue);
	skb_queue_purge(&bt_core_d->rx_queue);
}

int32 bt_skb_enqueue(struct bt_core_s *bt_core_d, struct sk_buff *skb, uint8 type)
{
	if (unlikely(bt_core_d == NULL)) {
		BT_PRINT_ERR(" bt_core_d is NULL\n");
		return -EINVAL;
	}

	if (unlikely(skb == NULL)) {
		BT_PRINT_ERR(" skb is NULL\n");
		return -EINVAL;
	}

	switch (type) {
	case TX_BT_QUEUE:
		skb_queue_tail(&bt_core_d->tx_queue, skb);
		break;
	case RX_BT_QUEUE:
		skb_queue_tail(&bt_core_d->rx_queue, skb);
		break;
	default:
		BT_PRINT_ERR("queue type is error, type=%d\n", type);
		break;
	}

	return 0;
}

struct sk_buff *bt_skb_dequeue(struct bt_core_s *bt_core_d, uint8 type)
{
	struct sk_buff *curr_skb = NULL;

	if (bt_core_d == NULL) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return curr_skb;
	}

	switch (type) {
	case TX_BT_QUEUE:
		curr_skb = skb_dequeue(&bt_core_d->tx_queue);
		break;
	case RX_BT_QUEUE:
		curr_skb = skb_dequeue(&bt_core_d->rx_queue);
		break;
	default:
		BT_PRINT_ERR("queue type is error, type = %d\n", type);
		break;
	}

	return curr_skb;
}

void bt_core_tx_work(struct work_struct *work)
{
	struct sk_buff *skb = NULL;
	int8 retry_count = 0;
	int32 ret;

	struct bt_core_s *bt_core_d = NULL;

	BT_PRINT_FUNCTION_NAME;

	bt_get_core_reference(&bt_core_d);
	if (unlikely(bt_core_d == NULL)) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return;
	}

	BT_PRINT_DBG("enter tx work\n");
	while (bt_core_d->tx_queue.qlen) {
		skb = bt_skb_dequeue(bt_core_d, TX_BT_QUEUE);
		/* no skb and exit */
		if (skb == NULL)
			break;

		if (skb->len) {
			/* for debug */
			BT_PRINT_DBG("start to send data,skb->len=[%d]\n", skb->len);
			data_str_printf(skb->len, &skb->data[0]);
			bt_send_data(BT_CHANNEL, BT_SERVICE_ID, BT_COMMAND_ID, skb->data, skb->len);
			/* send finish */
			skb->len = 0;
		}
		kfree_skb(skb);
	}
	return;
}

STATIC int32 data_check(const uint8 *data, int32 count)
{
	if (unlikely(data == NULL)) {
		BT_PRINT_ERR(" received null from SDIO\n");
		return 0;
	}

	if (count < 0) {
		BT_PRINT_ERR(" received count from SDIO err\n");
		return 0;
	}

	return 1;
}

int32 bt_recv_data_cb(uint8 service_id, uint8 command_id, uint8 *data, int32 data_len)
{
	struct bt_core_s *bt_core_d = NULL;
	struct sk_buff *skb = NULL;
	int32 ret;

	BT_PRINT_FUNCTION_NAME;
	g_sdio_state.sdio_rx_pkt_num++;
	ret = data_check(data, data_len);
	if (!ret)
		return ret;

	/* for debug */
	data_str_printf(data_len, &data[0]);

	bt_get_core_reference(&bt_core_d);
	/* If the cached data reaches its maximum value, the new data is discarded */
	if (bt_core_d->rx_queue.qlen >= RX_BT_QUE_MAX_NUM) {
		BT_PRINT_WARNING("%s rx queue too large! qlen=%d\n",
			RX_BT_QUE_MAX_NUM, bt_core_d->rx_queue.qlen);
		wake_up_interruptible(&bt_core_d->rx_wait);
		return -EINVAL;
	}

	skb = alloc_skb(data_len, GFP_ATOMIC);
	if (skb == NULL) {
		BT_PRINT_ERR("can't allocate mem for new skb, len=%d\n", data_len);
		return -EINVAL;
	}

	skb_put(skb, data_len);
	ret = memcpy_s(skb->data, data_len, data, data_len);
	if (ret != EOK) {
		BT_PRINT_ERR("skb data copy failed\n");
		kfree_skb(skb);
		return -EINVAL;
	}
	bt_skb_enqueue(bt_core_d, skb, RX_BT_QUEUE);

	BT_PRINT_DBG("%s rx done! qlen=%d\n", "BT", bt_core_d->rx_queue.qlen);
	wake_up_interruptible(&bt_core_d->rx_wait);
	return 0;
}
