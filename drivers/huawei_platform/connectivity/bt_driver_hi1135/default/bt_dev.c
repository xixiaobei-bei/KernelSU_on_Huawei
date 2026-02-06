/*
 * bt_dev.c
 *
 * code for bt hci driver adapter
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

#include "bt_dev.h"

#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/miscdevice.h>

#include "bt_common.h"
#include "bt_debug.h"
#include "bt_data_parse.h"
#include "ext_sensorhub_api.h"

#define BT_DEV "hwbt"
#define DEBUG_BUFFER_LEN 100
#define BT_TYPE_DATA_LEN 1
#define BT_DATA_SLEEP_TIME 2000

STATIC struct bt_core_s *g_bt_tl_drv_adt_d = NULL;
STATIC struct bt_data_combination g_bt_data_combination = {0};
STATIC struct sid_cid g_bt_sid_cid[] = {
	{ BT_SERVICE_ID, BT_COMMAND_ID },
	{ BT_FACTORY_TEST_ID, BT_NV_RW_ID }
};
STATIC struct sid_cid g_bt_mcu_sid_cid[] = {
	{ BT_FACTORY_TEST_ID, BT_XO_CAP_RW_ID }
};
STATIC struct subscribe_cmds g_bt_cmds = { g_bt_sid_cid, sizeof(g_bt_sid_cid) / sizeof(g_bt_sid_cid[0]) };
STATIC struct subscribe_cmds g_bt_mcu_cmds = {
	g_bt_mcu_sid_cid, sizeof(g_bt_mcu_sid_cid) / sizeof(g_bt_mcu_sid_cid[0])
};
/* register rev data CB status */
STATIC int32 g_register_status = 0x00;
/* for debug use */
STATIC uint8 g_debug_buffer[DEBUG_BUFFER_LEN] = {0};
STATIC int32 g_bt_loglevel = BT_LOG_DEBUG;

int32 bt_dev_log_level(void)
{
	return g_bt_loglevel;
}

void data_str_printf(int32 len, const uint8 *ptr)
{
	uint8 index;
	int print_len = len;

	if (g_bt_loglevel != BT_LOG_DEBUG)
		return;

	BT_PRINT_DBG("len = %d\n", len);
	/* 1: one bytes */
	if (print_len >= (DEBUG_BUFFER_LEN -1))
		print_len = DEBUG_BUFFER_LEN - 1;

	for (index = 0; index < print_len; index++)
		g_debug_buffer[index] = ptr[index];

	g_debug_buffer[index] = '\0';
	BT_PRINT_DBG("data = %s\n", g_debug_buffer);
}

STATIC void bt_data_timer_expire(uint64 data)
{
	struct bt_core_s *bt_core_d = NULL;
	bt_get_core_reference(&bt_core_d);

	if (bt_core_d != NULL)
		queue_work(bt_core_d->bt_tx_workqueue, &bt_core_d->timer_work);
	BT_PRINT_INFO("bt goto sleep\n");
}

STATIC void bt_data_timer_expire_work(struct work_struct *work)
{
	notify_close_bt_uart();
}

STATIC void notify_bt_switch_state(uint8 value)
{
	uint8 send_data[BT_SWITCH_STATE_TLV_LEN] = { BT_SWITCH_STATE_TYPE, 0x00, BT_SWITCH_STATE_VALUE_LEN, 0x00 };

	send_data[BT_SWITCH_STATE_VALUE_INDEX] = value;
	BT_PRINT_INFO("notify_bt_switch_state, value = %d\n", value);
	bt_send_data(BT_CHANNEL, BT_SERVICE_ID, BT_SWITCH_STATE_COMMAND_ID, send_data, BT_SWITCH_STATE_TLV_LEN);
}

STATIC struct sk_buff *bt_alloc_skb(uint16 len)
{
	struct sk_buff *skb = NULL;

	BT_PRINT_FUNCTION_NAME;

	skb = alloc_skb(len, GFP_KERNEL);
	if (skb == NULL) {
		BT_PRINT_WARNING("can't allocate mem for new skb, len=%d\n", len);
		return NULL;
	}

	skb_put(skb, len);

	return skb;
}

STATIC int32 bt_restore_skbqueue(struct bt_core_s *bt_core_d, struct sk_buff *skb, uint8 type)
{
	BT_PRINT_FUNCTION_NAME;

	if (unlikely((skb == NULL) || (bt_core_d == NULL))) {
		BT_PRINT_ERR(" skb or ps_core_d is NULL\n");
		return -EINVAL;
	}

	switch (type) {
	case TX_BT_QUEUE:
		skb_queue_head(&bt_core_d->tx_queue, skb);
		break;
	case RX_BT_QUEUE:
		skb_queue_head(&bt_core_d->rx_queue, skb);
		break;
	default:
		BT_PRINT_ERR("queue type is error, type=%d\n", type);
		break;
	}

	return 0;
}

void bt_get_core_reference(struct bt_core_s **core_data)
{
	if (core_data != NULL)
		*core_data = g_bt_tl_drv_adt_d;
}

STATIC int32 hw_bt_handshark_event(enum ext_sensorhub_event event, uint8 tid, uint8 *data, int32 data_len)
{
	struct bt_core_s *bt_core_d = NULL;

	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL))) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return -EINVAL;
	}
	/* 2 is the number of times you receive a handshake, and if it is >= 2, need to restart Bt controller */
	if (++bt_core_d->handshark_times >= 2) {
		uint8 need_reset = (uint8)true;
		bt_send_data(BT_CHANNEL, BT_SERVICE_ID, BT_CID_RESET_TYPE, &need_reset, sizeof(uint8));
	}
	BT_PRINT_INFO("hw_bt_handshark_event handshark times: %d (>=2 will reset BT)\n", bt_core_d->handshark_times);
	return 0;
}

STATIC void hw_bt_clear_handshark(void)
{
	struct bt_core_s *bt_core_d = NULL;
	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL))) {
		BT_PRINT_ERR("hw_bt_clear_handshark bt_core_d is NULL\n");
		return;
	}
	bt_core_d->handshark_times = 0;
	unregister_event_callback(COMMU_HANDSHAKE, TRANSACTION_ID_AP_TO_BT, hw_bt_handshark_event);
}

STATIC int32 hw_bt_open(struct inode *inode, struct file *filp)
{
	int32 ret;
	BT_PRINT_FUNCTION_NAME;
	if (unlikely((inode == NULL) || (filp == NULL))) {
		BT_PRINT_ERR("%s param is error", __func__);
		return -EINVAL;
	}
	ret = register_data_callback(BT_CHANNEL, &g_bt_cmds, bt_recv_data_cb);
	if (ret >= 0x00) {
		ret = register_data_callback(BT_MCU_CHANNEL, &g_bt_mcu_cmds, bt_recv_data_cb);
		if (ret >= 0x00)
			/* 0x01: register CB success */
			g_register_status = 0x01;
	}
	if (ret < 0) {
		BT_PRINT_ERR("%s register_data_callback err %d", __func__, ret);
		return -EINVAL;
	}
	register_event_callback(COMMU_HANDSHAKE, TRANSACTION_ID_AP_TO_BT, hw_bt_handshark_event);
	notify_bt_switch_state(BT_SWITCH_ON);

	return 0;
}

STATIC ssize_t hw_bt_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
	struct bt_core_s *bt_core_d = NULL;
	struct sk_buff *skb = NULL;
	uint16 count1;

	BT_PRINT_FUNCTION_NAME;

	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return -EINVAL;
	}

	mod_timer(&bt_core_d->data_timer, jiffies + msecs_to_jiffies(BT_DATA_SLEEP_TIME));
	if ((skb = bt_skb_dequeue(bt_core_d, RX_BT_QUEUE)) == NULL) {
		BT_PRINT_WARNING("bt read skb queue is null!\n");
		return 0;
	}

	/* read min value from skb->len or count */
	count1 = min_t(size_t, skb->len, count);
	if (copy_to_user(buf, skb->data, count1)) {
		BT_PRINT_ERR("copy_to_user is err!\n");
		bt_restore_skbqueue(bt_core_d, skb, RX_BT_QUEUE);
		return -EFAULT;
	}
	/* have read count1 byte */
	skb_pull(skb, count1);

	/* if skb->len = 0: read is over */
	if (skb->len == 0)
		/* curr skb data have read to user */
		kfree_skb(skb);
	else
		/* if don,t read over; restore to skb queue */
		bt_restore_skbqueue(bt_core_d, skb, RX_BT_QUEUE);

	return count1;
}

STATIC ssize_t hw_bt_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
	int32 ret;
	struct bt_core_s *bt_core_d = NULL;
	struct sk_buff *skb = NULL;
	uint16 total_len;
	uint8 __user *puser = (uint8 __user *)buf;
	uint8 type = 0;

	BT_PRINT_FUNCTION_NAME;

	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		g_bt_data_combination.len = 0;
		return -EINVAL;
	}

	if (count > BT_TX_MAX_FRAME) {
		BT_PRINT_ERR("bt skb len is too large!\n");
		g_bt_data_combination.len = 0;
		return -EINVAL;
	}

	/* one packet of bt hci data send through two times, first datatype, one byte, then data, we should combine them together here */
	if (count == BT_TYPE_DATA_LEN) {
		get_user(type, puser);
		g_bt_data_combination.type = type;
		g_bt_data_combination.len = count;
		BT_PRINT_INFO("rev type: %d\n", type);
		return count;
	}

	/* if high queue num > MAX_NUM and don't write */
	if (bt_core_d->tx_queue.qlen > TX_BT_QUE_MAX_NUM) {
		BT_PRINT_ERR("bt tx queue len large than MAXNUM\n");
		g_bt_data_combination.len = 0;
		return 0;
	}

	total_len = count + g_bt_data_combination.len;

	skb = bt_alloc_skb(total_len);
	if (skb == NULL) {
		BT_PRINT_ERR("bt alloc skb mem fail\n");
		g_bt_data_combination.len = 0;
		return -EFAULT;
	}

	if (copy_from_user(&skb->data[g_bt_data_combination.len], buf, count)) {
		BT_PRINT_ERR("copy_from_user from bt is err\n");
		kfree_skb(skb);
		g_bt_data_combination.len = 0;
		return -EFAULT;
	}

	if (g_bt_data_combination.len == BT_TYPE_DATA_LEN) {
		/* first byte is type */
		skb->data[0] = g_bt_data_combination.type;
	}

	/* for debug */
	data_str_printf(total_len, &skb->data[0]);

	g_bt_data_combination.len = 0;

	bt_skb_enqueue(bt_core_d, skb, TX_BT_QUEUE);
	if (g_register_status == 0x00) {
		ret = register_data_callback(BT_CHANNEL, &g_bt_cmds, bt_recv_data_cb);
		if (ret >= 0) {
			g_register_status = 0x01;
		} else {
			BT_PRINT_ERR("register_data_callback fail, ret = %d\n", ret);
		}
	}
	queue_work(bt_core_d->bt_tx_workqueue, &bt_core_d->tx_skb_work);
	mod_timer(&bt_core_d->data_timer, jiffies + msecs_to_jiffies(BT_DATA_SLEEP_TIME));
	g_bt_tl_drv_adt_d->tx_pkt_num++;

	return count;
}

STATIC uint32 hw_bt_poll(struct file *filp, poll_table *wait)
{
	struct bt_core_s *bt_core_d = NULL;
	uint32 mask = 0;

	BT_PRINT_FUNCTION_NAME;

	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL) || (filp == NULL))) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return -EINVAL;
	}

	/* push curr wait event to wait queue */
	poll_wait(filp, &bt_core_d->rx_wait, wait);
	/* have data to read */
	if (bt_core_d->rx_queue.qlen)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static int64 hw_bt_trans_data(enum ext_channel_id channel_id, unsigned char service_id,
		unsigned char command_id, uint64 __user arg)
{
	int i;
	int ret;
	unsigned short len;
	struct command command;
	bt_trans_t trans_header;
	unsigned char *send_data = NULL;

	copy_from_user(&trans_header, arg, sizeof(bt_trans_t));
	len = trans_header.length_l | (trans_header.length_h << 8);
	send_data = kmalloc(sizeof(bt_trans_t) + len, GFP_KERNEL);
	if (send_data == NULL) {
		BT_PRINT_ERR("%s kmalloc err\n", __func__);
		return -EFAULT;
	}
	if (copy_from_user(send_data, arg, len + sizeof(bt_trans_t))) {
		kfree(send_data);
		BT_PRINT_ERR("%s copy_form_user err\n", __func__);
		return -EFAULT;
	}
	command.service_id = service_id;
	command.command_id = command_id;
	command.send_buffer = send_data;
	command.send_buffer_len = len + sizeof(bt_trans_t);

	for (i = 0; i < sizeof(bt_trans_t) + len; i++) {
		BT_PRINT_INFO("hw_bt_operation_nv[%d]: 0x%x \n", i, send_data[i]);
	}
	ret = send_command(channel_id, &command, false, NULL);
	if (ret < 0) {
		BT_PRINT_ERR("command ERR, ret = %d\n", ret);
	}
	kfree(send_data);
	return ret;
}

int64 hw_bt_operation_xo_cap(uint64 __user arg)
{
	return hw_bt_trans_data(BT_MCU_CHANNEL, BT_FACTORY_TEST_ID, BT_XO_CAP_RW_ID, arg);
}

int64 hw_bt_operation_nv(uint64 __user arg)
{
	return hw_bt_trans_data(BT_CHANNEL, BT_FACTORY_TEST_ID, BT_NV_RW_ID, arg);
}

STATIC int64 hw_bt_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
	BT_PRINT_FUNCTION_NAME;
	struct bt_core_s *bt_core_d = NULL;
	int64 ret = -EPERM;

	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL))) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return -EINVAL;
	}
	mod_timer(&bt_core_d->data_timer, jiffies + msecs_to_jiffies(BT_DATA_SLEEP_TIME));

	switch (cmd) {
	case BT_OPERATION_NV:
		ret = hw_bt_operation_nv(arg);
		break;
	case BT_OPERATION_XO_CAP:
		ret = hw_bt_operation_xo_cap(arg);
		break;
	default:
		BT_PRINT_INFO("hw_bt_ioctl default\n");
		break;
	}

	return ret;
}

STATIC int32 hw_bt_release(struct inode *inode, struct file *filp)
{
	BT_PRINT_FUNCTION_NAME;
	struct bt_core_s *bt_core_d = NULL;
	bt_get_core_reference(&bt_core_d);
	if (unlikely((bt_core_d == NULL))) {
		BT_PRINT_ERR("bt_core_d is NULL\n");
		return -EINVAL;
	}
	unregister_data_callback(BT_CHANNEL);
	unregister_data_callback(BT_MCU_CHANNEL);
	g_register_status = 0;
	hw_bt_clear_handshark();
	bt_skb_purge(bt_core_d);
	notify_bt_switch_state(BT_SWITCH_OFF);
	return 0;
}

STATIC const struct file_operations hw_bt_fops = {
	.owner = THIS_MODULE,
	.open = hw_bt_open,
	.write = hw_bt_write,
	.read = hw_bt_read,
	.poll = hw_bt_poll,
	.unlocked_ioctl = hw_bt_ioctl,
	.compat_ioctl = hw_bt_ioctl,
	.release = hw_bt_release,
};

STATIC struct miscdevice hw_bt_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = BT_DEV,
	.fops = &hw_bt_fops,
};

STATIC int32 __init bt_hci_sdio_driver_adapter_init(void)
{
	int32 ret;
	if (is_ext_sensorhub_disabled())
		return -EINVAL;

	g_bt_tl_drv_adt_d = kzalloc(sizeof(struct bt_core_s), GFP_KERNEL);
	if (g_bt_tl_drv_adt_d == NULL) {
		BT_PRINT_ERR("no mem to allocate\n");
		return -ENOMEM;
	}
	/* initialize tx and rx queue head for actual transmission */
	skb_queue_head_init(&g_bt_tl_drv_adt_d->rx_queue);
	skb_queue_head_init(&g_bt_tl_drv_adt_d->tx_queue);
	/* create a singlethread work queue */
	g_bt_tl_drv_adt_d->bt_tx_workqueue = create_singlethread_workqueue("bt_tx_queue");
	/* init tx work */
	INIT_WORK(&g_bt_tl_drv_adt_d->tx_skb_work, bt_core_tx_work);
	INIT_WORK(&g_bt_tl_drv_adt_d->timer_work, bt_data_timer_expire_work);

	init_waitqueue_head(&g_bt_tl_drv_adt_d->rx_wait);

	/* init sleep timer */
	g_bt_tl_drv_adt_d->data_timer.function = bt_data_timer_expire;
	g_bt_tl_drv_adt_d->data_timer.data = &g_bt_tl_drv_adt_d;
	init_timer(&g_bt_tl_drv_adt_d->data_timer);
	mod_timer(&g_bt_tl_drv_adt_d->data_timer, jiffies + msecs_to_jiffies(BT_DATA_SLEEP_TIME));

	ret = misc_register(&hw_bt_device);
	if (ret != 0) {
		BT_PRINT_ERR("misc_register fail , ret = %d\n", ret);
		if (g_bt_tl_drv_adt_d != NULL) {
			kfree(g_bt_tl_drv_adt_d);
			g_bt_tl_drv_adt_d = NULL;
		}
		return ret;
	}
	return 0;
}

STATIC void __exit bt_hci_sdio_driver_adapter_exit(void)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;

	BT_PRINT_FUNCTION_NAME;

	destroy_workqueue(g_bt_tl_drv_adt_d->bt_tx_workqueue);
	del_timer_sync(&g_bt_tl_drv_adt_d->data_timer);
	misc_deregister(&hw_bt_device);
	if (g_bt_tl_drv_adt_d != NULL) {
		kfree(g_bt_tl_drv_adt_d);
		g_bt_tl_drv_adt_d = NULL;
	}
	unregister_data_callback(BT_CHANNEL);
}

module_init(bt_hci_sdio_driver_adapter_init);
module_exit(bt_hci_sdio_driver_adapter_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bt hci driver adapter");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
