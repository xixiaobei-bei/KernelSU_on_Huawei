/*
 * voice_proxy_fusion.c - HW voice proxy fusion in kernel, it is used for pass
 * through voice data between audio hal and hifi for fusion(wifi or cell).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "voice_proxy_fusion.h"
#include "voice_proxy.h"
#include <linux/miscdevice.h>

#define LOG_TAG "voice_proxy_fusion"
#define FUSION_RX_DATA_SIZE sizeof(struct vpeer_voice_rx_rtp_ntf)
#define FUSION_TX_DATA_SIZE sizeof(struct voice_vpeer_tx_rtp_ntf)
#define DTS_COMP_VOICE_PROXY_FUSION_NAME "hisilicon,voice_proxy_fusion"

#define FUSION_HEAD_LENGTH (sizeof(struct proxy_fusion_voice_ntf))
#define VOICE_PROXY_CTRL_SIZE_MAX 100
#define VOICE_PROXY_WAKE_UP_FUSION_READ  _IO('P',  0x1)

enum STATUS {
	STATUS_CLOSE,
	STATUS_OPEN,
	STATUS_BUTT
};

#ifndef unused_parameter
#define unused_parameter(x) (void)(x)
#endif

LIST_HEAD(recv_fusion_ctrl_queue);
LIST_HEAD(recv_fusion_rx_queue);
LIST_HEAD(send_fusion_tx_queue);

struct fusion_priv {
	spinlock_t fusion_read_lock;

	spinlock_t fusion_write_lock;

	wait_queue_head_t fusion_read_waitq;

	int32_t fusion_read_wait_flag;

	/* this is used for counting the size of recv_fusion_rx_queue */
	int32_t fusion_rx_cnt;

	/* this is used for counting the size of recv_fusion_ctrl_queue */
	int32_t fusion_ctrl_cnt;

	/* this is used for counting the size of send_fusion_tx_queue */
	int32_t fusion_tx_cnt;

	/* fusion rx voice data confirm */
	bool fusion_rx_cnf;

	/* fusion rx first voice data */
	bool first_fusion_rx;

	/* fusion receive tx open msg from hifi */
	bool fusion_is_tx_open;

	/* fusion rx voice data time stamp */
	int64_t fusion_rx_stamp;

	struct mutex ioctl_mutex;
};

static struct fusion_priv g_priv;

static void fusion_sign_init(void)
{
	g_priv.fusion_rx_cnf = false;
	g_priv.first_fusion_rx = true;
	g_priv.fusion_is_tx_open = false;
}

static void fusion_clear_queue(struct list_head *queue, uint32_t buf_size)
{
	uint32_t cnt = 0;
	struct voice_proxy_data_node *node = NULL;

	while (!list_empty_careful(queue)) {
		cnt++;
		if (cnt > buf_size) {
			AUDIO_LOGE("fusion:clear queue abnormal, cnt is %u", cnt);
			break;
		}

		node = list_first_entry(queue, struct voice_proxy_data_node, list_node);
		list_del_init(&node->list_node);
		kfree(node);
		node = NULL;
	}

	AUDIO_LOGI("fusion:clear queue cnt is %u", cnt);
}

static int32_t fusion_add_tx_data(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&g_priv.fusion_read_lock);
	if (g_priv.fusion_tx_cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
		g_priv.fusion_read_wait_flag++;
		spin_unlock_bh(&g_priv.fusion_read_lock);
		wake_up(&g_priv.fusion_read_waitq);

		return -ENOMEM;
	}
	spin_unlock_bh(&g_priv.fusion_read_lock);

	ret = voice_proxy_create_data_node(&node, rev_buf, (int)buf_size);
	if (ret) {
		AUDIO_LOGE("data_node kzalloc failed");
		return ret;
	}

	spin_lock_bh(&g_priv.fusion_read_lock);
	list_add_tail(&node->list_node, &send_fusion_tx_queue);
	g_priv.fusion_tx_cnt++;
	g_priv.fusion_read_wait_flag++;
	spin_unlock_bh(&g_priv.fusion_read_lock);
	wake_up(&g_priv.fusion_read_waitq);

	return 0;
}

static void fusion_get_rx_data(int8_t *data, uint32_t *size)
{
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&g_priv.fusion_write_lock);
	if (!list_empty_careful(&recv_fusion_rx_queue)) {
		node = list_first_entry(&recv_fusion_rx_queue, struct voice_proxy_data_node, list_node);

		list_del_init(&node->list_node);

		if (g_priv.fusion_rx_cnt > 0)
			g_priv.fusion_rx_cnt--;

		if (*size < (uint32_t)node->list_data.size) {
			AUDIO_LOGE("invalid size = %d, list_data.size = %d", *size, node->list_data.size);
			kfree(node);
			node = NULL;
			*size = 0;
			spin_unlock_bh(&g_priv.fusion_write_lock);
			return;
		}

		*size = (uint32_t)node->list_data.size;
		memcpy(data, node->list_data.data, (size_t)*size);

		kfree(node);
		node = NULL;
		spin_unlock_bh(&g_priv.fusion_write_lock);

		g_priv.first_fusion_rx = false;
		g_priv.fusion_rx_cnf = false;
	} else {
		spin_unlock_bh(&g_priv.fusion_write_lock);
		*size = 0;
	}
}

static void fusion_get_fusioncall_data(int8_t *data, uint32_t *size)
{
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&g_priv.fusion_write_lock);
	if (!list_empty_careful(&recv_fusion_ctrl_queue)) {
		node = list_first_entry(&recv_fusion_ctrl_queue, struct voice_proxy_data_node, list_node);

		list_del_init(&node->list_node);

		if (g_priv.fusion_ctrl_cnt > 0)
			g_priv.fusion_ctrl_cnt--;

		if (*size < (uint32_t)node->list_data.size) {
			AUDIO_LOGE("invalid size = %d, list_data.size = %d", *size, node->list_data.size);
			kfree(node);
			node = NULL;
			*size = 0;
			spin_unlock_bh(&g_priv.fusion_write_lock);
			return;
		}

		*size = (uint32_t)node->list_data.size;
		memcpy(data, node->list_data.data, (size_t)*size);

		kfree(node);
		node = NULL;
		spin_unlock_bh(&g_priv.fusion_write_lock);
	} else {
		spin_unlock_bh(&g_priv.fusion_write_lock);
		*size = 0;
	}
}

static void fusion_receive_ctrl_ind(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;

	if (rev_buf == NULL) {
		AUDIO_LOGE("%s fail, param is NULL", __FUNCTION__);
		return;
	}

	ret = fusion_add_tx_data(rev_buf, buf_size);
	if (ret) {
		AUDIO_LOGE("send ctrl ind to read func failed");
		return;
	}
}

static void fusion_receive_tx_ntf(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;

	if (rev_buf == NULL) {
		AUDIO_LOGE("%s fail, param rev_buf is NULL", __FUNCTION__);
		return;
	}

	ret = fusion_add_tx_data(rev_buf, buf_size);
	if (ret) {
		AUDIO_LOGE("send fusion tx data to read func failed");
		return;
	}
}

static void fusion_receive_rx_cnf(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;
	unused_parameter(rev_buf);

	ret = voice_proxy_add_cmd(ID_VOICE_PROXY_VPEER_CTRL_CNF);
	if (ret != 0)
		AUDIO_LOGE("send fusion rx data cnf failed");
}

static void fusion_handle_rx_ntf(int8_t *data, uint32_t *size, uint16_t *msg_id)
{
	if (data == NULL || size == NULL || msg_id == NULL) {
		AUDIO_LOGE("%s fail, param is NULL", __FUNCTION__);
		return;
	}

	voice_proxy_set_send_sign(g_priv.first_fusion_rx, &g_priv.fusion_rx_cnf, &g_priv.fusion_rx_stamp);

	if (g_priv.first_fusion_rx || g_priv.fusion_rx_cnf)
		fusion_get_rx_data(data, size);
	else
		*size = 0;
	*msg_id = ID_PROXY_VOICE_VPEER_CTRL_NTF;
}

static void fusion_handle_fusioncall_ctrl_ind(int8_t *data, uint32_t *size, uint16_t *msg_id)
{
	if (data == NULL || size == NULL || msg_id == NULL) {
		AUDIO_LOGE("%s fail, param is NULL", __FUNCTION__);
		return;
	}

	fusion_get_fusioncall_data(data, size);
	*msg_id = ID_PROXY_VOICE_FUSIONCALL_CTRL_IND;
}

static void fusion_handle_rx_cnf(int8_t *data, uint32_t *size, uint16_t *msg_id)
{
	if (data == NULL || size == NULL || msg_id == NULL) {
		AUDIO_LOGE("%s fail, param is NULL", __FUNCTION__);
		return;
	}

	g_priv.fusion_rx_cnf = true;
	g_priv.fusion_rx_stamp = voice_proxy_get_time_ms();

	fusion_get_rx_data(data, size);
	*msg_id = ID_PROXY_VOICE_VPEER_CTRL_NTF;
}

static bool is_input_param_valid(struct file *file, size_t size)
{
	if (file == NULL) {
		AUDIO_LOGE("file is nul");
		return false;
	}

	if (file->f_flags & O_NONBLOCK) {
		AUDIO_LOGE("file->f_flags & O_NONBLOCK  fail");
		return false;
	}

	if (size < FUSION_TX_DATA_SIZE) {
		AUDIO_LOGE("param err, size %zd, little than FUSION_TX_DATA_SIZE %ld", size, FUSION_TX_DATA_SIZE);
		return false;
	}

	return true;
}

static ssize_t fusion_read(struct file *file, char __user *user_buf, size_t size, loff_t *ppos)
{
	struct voice_proxy_data_node *node = NULL;
	int ret = 0;

	if (!is_input_param_valid(file, size)) {
		AUDIO_LOGE("invalid input params");
		return -EINVAL;
	}

	spin_lock_bh(&g_priv.fusion_read_lock);
	if (list_empty_careful(&send_fusion_tx_queue)) {
		spin_unlock_bh(&g_priv.fusion_read_lock);
		ret = wait_event_interruptible(g_priv.fusion_read_waitq, g_priv.fusion_read_wait_flag > 0);
		if (ret) {
			if (ret != -ERESTARTSYS)
				AUDIO_LOGE("wait interruptible fail,0x%x", ret);
			return -EBUSY;
		}
		spin_lock_bh(&g_priv.fusion_read_lock);
	}

	g_priv.fusion_read_wait_flag = 0;

	if (!list_empty_careful(&send_fusion_tx_queue)) {
		node = list_first_entry(&send_fusion_tx_queue, struct voice_proxy_data_node, list_node);

		list_del_init(&node->list_node);
		if (g_priv.fusion_tx_cnt > 0)
			g_priv.fusion_tx_cnt--;

		if (size < node->list_data.size) {
			AUDIO_LOGE("size(%zd), little list_data.size(%d)", size, node->list_data.size);
			kfree(node);
			node = NULL;
			spin_unlock_bh(&g_priv.fusion_read_lock);
			return -EAGAIN;
		}

		if (copy_to_user(user_buf, node->list_data.data, node->list_data.size)) {
			AUDIO_LOGE("copy_to_user fail");
			ret = -EFAULT;
		} else {
			ret = node->list_data.size;
		}
		kfree(node);
		node = NULL;
		spin_unlock_bh(&g_priv.fusion_read_lock);
	} else {
		spin_unlock_bh(&g_priv.fusion_read_lock);
		ret = -EAGAIN;
		AUDIO_LOGE("list is empty, read again");
	}

	return ret;
}

static int32_t fusion_add_rx_data(int8_t *data, uint32_t size)
{
	int32_t ret;
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&g_priv.fusion_write_lock);
	if (g_priv.fusion_rx_cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
		spin_unlock_bh(&g_priv.fusion_write_lock);
		AUDIO_LOGE("out of queue, rx cnt %d greater than MAX %d", g_priv.fusion_rx_cnt, VOICE_PROXY_QUEUE_SIZE_MAX);
		return -ENOMEM;
	}
	spin_unlock_bh(&g_priv.fusion_write_lock);

	ret = voice_proxy_create_data_node(&node, data, (int)size);
	if (ret != 0) {
		AUDIO_LOGE("node kzalloc failed");
		return -EFAULT;
	}
	spin_lock_bh(&g_priv.fusion_write_lock);
	list_add_tail(&node->list_node, &recv_fusion_rx_queue);
	g_priv.fusion_rx_cnt++;
	spin_unlock_bh(&g_priv.fusion_write_lock);

	return (int32_t)size;
}

static int32_t fusion_add_ctrl_data(int8_t *data, uint32_t size)
{
	int32_t ret;
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&g_priv.fusion_write_lock);
	if (g_priv.fusion_ctrl_cnt > VOICE_PROXY_CTRL_SIZE_MAX) {
		spin_unlock_bh(&g_priv.fusion_write_lock);
		AUDIO_LOGE("out of queue, cnt %d greater than %d", g_priv.fusion_ctrl_cnt, VOICE_PROXY_CTRL_SIZE_MAX);
		return -ENOMEM;
	}
	spin_unlock_bh(&g_priv.fusion_write_lock);

	ret = voice_proxy_create_data_node(&node, data, (int)size);
	if (ret != 0) {
		AUDIO_LOGE("node kzalloc failed");
		return -EFAULT;
	}
	spin_lock_bh(&g_priv.fusion_write_lock);
	list_add_tail(&node->list_node, &recv_fusion_ctrl_queue);
	g_priv.fusion_ctrl_cnt++;
	spin_unlock_bh(&g_priv.fusion_write_lock);

	return (int)size;
}

static int32_t fusion_dispatch_write_msg(int8_t *data, uint32_t size)
{
	int32_t ret;
	uint16_t msg_id = *((uint16_t*)data);

	switch(msg_id) {
	case ID_PROXY_VOICE_FUSIONCALL_CTRL_IND:
		ret = voice_proxy_add_data(fusion_add_ctrl_data, data, size, ID_PROXY_VOICE_FUSIONCALL_CTRL_IND);
		break;
	case ID_PROXY_VOICE_VPEER_CTRL_NTF:
		ret = voice_proxy_add_data(fusion_add_rx_data, data, size, ID_PROXY_VOICE_VPEER_CTRL_NTF);
		break;
	default:
		AUDIO_LOGE("%s fail msgid [0x%x]", __FUNCTION__, msg_id);
		ret = -EFAULT;
		break;
	}
	return ret;
}


static ssize_t fusion_write(struct file *filp, const char __user *buff, size_t size, loff_t *offp)
{
	int32_t ret;
	int8_t data[FUSION_RX_DATA_SIZE + FUSION_HEAD_LENGTH - 4] = {0}; /* member msg got 4 bytes, need to remove it */
	int32_t max_size = FUSION_RX_DATA_SIZE + FUSION_HEAD_LENGTH - 4;

	unused_parameter(filp);
	unused_parameter(offp);

	if (size > max_size) {
		AUDIO_LOGE("para error, size:%zd greater than max size %d", size, max_size);
		return -EINVAL;
	}

	if (copy_from_user(data, buff, size) != 0) {
		AUDIO_LOGE("copy_from_user fail");
		return -EFAULT;
	} else {
		if (size > max_size || size < FUSION_HEAD_LENGTH) {
			AUDIO_LOGE("copy_from_user fail size [%zd]", size);
			return -EFAULT;
		}
	}

	ret = fusion_dispatch_write_msg(data, size);
	if (ret <= 0) {
		AUDIO_LOGE("call fusion_dispatch_write_msg fail");
		return -EFAULT;
	}

	return (int)size;
}

static void fusion_wake_up_read(void)
{
	spin_lock_bh(&g_priv.fusion_read_lock);
	g_priv.fusion_read_wait_flag++;
	spin_unlock_bh(&g_priv.fusion_read_lock);
	wake_up(&g_priv.fusion_read_waitq);
}

static long fusion_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	unused_parameter(fd);
	unused_parameter(arg);

	mutex_lock(&g_priv.ioctl_mutex);
	switch (cmd) {
	case VOICE_PROXY_WAKE_UP_FUSION_READ:
		fusion_wake_up_read();
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&g_priv.ioctl_mutex);

	return ret;
}

static int fusion_open(struct inode *finode, struct file *fd)
{
	unused_parameter(finode);
	unused_parameter(fd);
	AUDIO_LOGI("%s start", __FUNCTION__);

	spin_lock_bh(&g_priv.fusion_write_lock);
	fusion_clear_queue(&recv_fusion_rx_queue, VOICE_PROXY_QUEUE_SIZE_MAX);
	g_priv.fusion_rx_cnt = 0;
	spin_unlock_bh(&g_priv.fusion_write_lock);

	spin_lock_bh(&g_priv.fusion_write_lock);
	fusion_clear_queue(&recv_fusion_ctrl_queue, VOICE_PROXY_CTRL_SIZE_MAX);
	g_priv.fusion_ctrl_cnt = 0;
	spin_unlock_bh(&g_priv.fusion_write_lock);

	return 0;
}

static int fusion_close(struct inode *node, struct file *filp)
{
	unused_parameter(node);
	unused_parameter(filp);

	spin_lock_bh(&g_priv.fusion_read_lock);
	g_priv.fusion_read_wait_flag++;
	g_priv.fusion_is_tx_open = false;
	spin_unlock_bh(&g_priv.fusion_read_lock);
	wake_up(&g_priv.fusion_read_waitq);
	return 0;
}

static const struct file_operations fusion_misc_fops = {
	.owner = THIS_MODULE,
	.open = fusion_open,
	.read = fusion_read,
	.write = fusion_write,
	.release = fusion_close,
	.unlocked_ioctl = fusion_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = fusion_ioctl,
#endif
};

static struct miscdevice fusion_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "voice_proxy_fusion",
	.fops = &fusion_misc_fops,
};

static int fusion_probe(struct platform_device *pdev)
{
	int32_t ret;
	memset(&g_priv, 0, sizeof(g_priv));
	unused_parameter(pdev);
	AUDIO_LOGI("%s start", __FUNCTION__);

	g_priv.fusion_read_wait_flag = 0;
	spin_lock_init(&g_priv.fusion_read_lock);
	spin_lock_init(&g_priv.fusion_write_lock);
	init_waitqueue_head(&g_priv.fusion_read_waitq);
	mutex_init(&g_priv.ioctl_mutex);

	ret = misc_register(&fusion_misc_device);
	if (ret != 0) {
		AUDIO_LOGE("fusion misc register fail");
		return ret;
	}

	fusion_sign_init();

	voice_proxy_register_msg_callback(ID_VOICE_PROXY_FUSIONCALL_CTRL_IND, fusion_receive_ctrl_ind);
	voice_proxy_register_msg_callback(ID_VOICE_PROXY_VPEER_CTRL_NTF, fusion_receive_tx_ntf);
	voice_proxy_register_msg_callback(ID_VOICE_PROXY_VPEER_CTRL_CNF, fusion_receive_rx_cnf);

	voice_proxy_register_cmd_callback(ID_PROXY_VOICE_FUSIONCALL_CTRL_IND, fusion_handle_fusioncall_ctrl_ind);
	voice_proxy_register_cmd_callback(ID_PROXY_VOICE_VPEER_CTRL_NTF, fusion_handle_rx_ntf);
	voice_proxy_register_cmd_callback(ID_VOICE_PROXY_VPEER_CTRL_CNF, fusion_handle_rx_cnf);

	voice_proxy_register_sign_init_callback(fusion_sign_init);

	return ret;
}

static int fusion_remove(struct platform_device *pdev)
{

	unused_parameter(pdev);
	AUDIO_LOGI("%s start", __FUNCTION__);

	mutex_destroy(&g_priv.ioctl_mutex);
	misc_deregister(&fusion_misc_device);

	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_FUSIONCALL_CTRL_IND);
	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_VPEER_CTRL_NTF);
	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_VPEER_CTRL_CNF);

	voice_proxy_deregister_cmd_callback(ID_PROXY_VOICE_FUSIONCALL_CTRL_IND);
	voice_proxy_deregister_cmd_callback(ID_PROXY_VOICE_VPEER_CTRL_NTF);

	voice_proxy_deregister_sign_init_callback(fusion_sign_init);

	return 0;
}


static const struct of_device_id fusion_match_table[] = {
	{
		.compatible = DTS_COMP_VOICE_PROXY_FUSION_NAME,
		.data = NULL,
	},
	{
	}
};

static struct platform_driver fusion_driver = {
	.driver = {
		.name  = "voice proxy fusion",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(fusion_match_table),
	},
	.probe = fusion_probe,
	.remove = fusion_remove,
};

static int __init fusion_init(void)
{
	int32_t ret;
	AUDIO_LOGI("%s start", __FUNCTION__);

	ret = platform_driver_register(&fusion_driver);
	if (ret)
		AUDIO_LOGE("voice proxy fusion driver register fail %d", ret);

	return ret;
}

static void __exit fusion_exit(void)
{
	AUDIO_LOGI("%s start", __FUNCTION__);
	platform_driver_unregister(&fusion_driver);
}

module_init(fusion_init);
module_exit(fusion_exit);

MODULE_DESCRIPTION("voice proxy fusion driver");
MODULE_LICENSE("GPL");

