/*
 * virtual_voice_proxy_extdev.c
 *
 * virtual voice proxy extdev driver
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include "huawei_platform/audio/virtual_voice_proxy.h"

#include <linux/miscdevice.h>
#include <linux/delay.h>

#define DTS_VIR_VOICE_PROXY_EXTDEV_NAME "hisilicon,virtual_voice_proxy_extdev"

#define VIRTUAL_PROXY_VOICE_WAKE_UP_READ _IO('V', 0x1)
#define VIRTUAL_PROXY_VOICE_SET_MODE     _IO('V', 0x2)
#define VIRTUAL_PROXY_VOICE_SET_DISABLE  _IO('V', 0x3)

#define IO_BLOCK_TIMEOUT 9000
#define VIRTUAL_VOICE_WR_ERR_LIMIT 20

enum virtual_voice_status {
	STATUS_CLOSE = 0,
	STATUS_OPEN,
	STATUS_BUTT
};

struct proxy_extdev_priv {
	spinlock_t read_lock;
	spinlock_t write_lock;
	wait_queue_head_t read_waitq;
	wait_queue_head_t write_waitq;
	int32_t read_wait_flag;
	int32_t write_wait_flag;
	/* this is used for counting the size of recv_rx_queue */
	int32_t rx_cnt;
	/* this is used for counting the size of send_tx_queue */
	int32_t tx_cnt;
	/* rx voice data confirm */
	bool rx_confirm;
	/* rx first voice data */
	bool first_rx;
	/* virtual proxy status by virtual modem processor and hifi */
	bool is_proxy_open;
	/* open dev ctl by virtual modem processor */
	bool is_dev_open;
	/* rx voice data time stamp */
	int64_t rx_stamp;
	/* call uplink */
	char *micdata_buf;
	/* down uplink */
	char *spkdata_buf;
	/* voice proxy mode */
	uint32_t proxy_mode;
	bool disable_virtual_call;
	struct virtual_voice_memory_pool tx_mem_pool;
	struct virtual_voice_memory_pool rx_mem_pool;
	int32_t tx_read_err_cnt;
	int32_t rx_write_err_cnt;
};

static struct proxy_extdev_priv proxye_priv;

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

LIST_HEAD(recv_voice_proxy_rx_queue);
LIST_HEAD(send_voice_proxy_tx_queue);

static void proxy_extdev_check_read_write_data_err_cnt(void);

static struct virtual_voice_proxy_data_node *virtual_voice_data_node_malloc(
	struct virtual_voice_memory_pool *mem_pool,
	uint32_t size)
{
	int ret;
	void *ptr = NULL;
	struct virtual_voice_proxy_data_node *node = NULL;
	struct vir_free_node *mem_node = NULL;

	if (mem_pool == NULL)
		return NULL;

	ptr = virtual_voice_malloc(mem_pool, size, &ret);
	if (ptr == NULL) {
		loge("node malloc in pool failed\n");
		return NULL;
	}

	if (ret == VIR_MEM_BLOCK_POOL_NOT_USE) {
		node = ptr;
		/* only for virtual_voice_free use the node_cmd->mem_node */
		node->mem_node = ptr;
	} else {
		mem_node = ptr;
		node = (struct virtual_voice_proxy_data_node *)(mem_node->data);
		if (node == NULL) {
			loge("node is NULL\n");
			virtual_voice_free(mem_pool,
				(void **)(&mem_node), VIR_MEM_BLOCK_POOL_USE);
			return NULL;
		}
		node->mem_node = mem_node;
	}
	node->mem_status = ret;
	return node;
}

static void virtual_voice_data_node_free(
	struct virtual_voice_memory_pool *mem_pool,
	struct virtual_voice_proxy_data_node **node)
{
	struct virtual_voice_proxy_data_node *node_data = NULL;

	if (node == NULL || *node == NULL || mem_pool == NULL)
		return;

	node_data = *node;
	virtual_voice_free(mem_pool,
		(void **)(&node_data->mem_node),
		node_data->mem_status);
}

static int32_t virtual_voice_proxy_create_data_node(
	struct virtual_voice_memory_pool *mem_pool,
	struct virtual_voice_proxy_data_node **node,
	const int8_t *data, int32_t size)
{
	struct virtual_voice_proxy_data_node *n = NULL;

	if (node == NULL || data == NULL || mem_pool == NULL) {
		loge("input parameter invalid");
		return -EINVAL;
	}

	n = virtual_voice_data_node_malloc(mem_pool,
		sizeof(struct virtual_voice_proxy_data_node) + size);
	if (n == NULL) {
		loge("kzalloc failed\n");
		*node = NULL;
		return -ENOMEM;
	}

	memcpy(n->list_data.data, data, size);
	n->list_data.size = size;
	*node = n;
	return 0;
}

static void virtual_voice_data_memory_pool_free(void)
{
	if (list_empty_careful(&recv_voice_proxy_rx_queue)) {
		if (virtual_voice_memory_pool_free_node_full_idle(
			&proxye_priv.rx_mem_pool)) {
			logi("voice rx data mem pool free\n");
			virtual_voice_memory_pool_free(&proxye_priv.rx_mem_pool);
			return;
		}
	}

	if (list_empty_careful(&send_voice_proxy_tx_queue)) {
		if (virtual_voice_memory_pool_free_node_full_idle(
			&proxye_priv.tx_mem_pool)) {
			logi("voice tx data mem pool free\n");
			virtual_voice_memory_pool_free(&proxye_priv.tx_mem_pool);
			return;
		}
	}
}

static void proxy_extdev_clear_queue(struct virtual_voice_memory_pool *mem_pool,
	struct list_head *queue)
{
	uint32_t cnt = 0;
	struct virtual_voice_proxy_data_node *node = NULL;

	while (!list_empty_careful(queue)) {
		cnt++;
		if (cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
			loge("clear queue fail, cnt is %u\n", cnt);
			break;
		}
		node = list_first_entry(queue,
					struct virtual_voice_proxy_data_node,
					list_node);
		if (node == NULL) {
			loge("node is NULL\n");
			break;
		}
		list_del_init(&node->list_node);
		virtual_voice_data_node_free(mem_pool,
			(struct virtual_voice_proxy_data_node **)(&node));
		node = NULL;
	}

	logi("proxy_extdev:clear queue cnt is %u\n", cnt);
}

static int proxy_extdev_read_check(struct file *file, char __user *user_buf,
	size_t size)
{
	if (proxye_priv.disable_virtual_call)
		return -EOPNOTSUPP;

	if (!proxye_priv.is_proxy_open) {
		logw("the proxy not open or voice call finished, not read\n");
		return -ECANCELED;
	}

	if (!proxye_priv.is_dev_open) {
		logw("the dev not open not read\n");
		return -ECANCELED;
	}

	if (proxye_priv.proxy_mode == VIRTUAL_CALL_STATUS_OFF) {
		logw("the proxy_mode is off not read\n");
		return -EREMOTEIO;
	}

	if (user_buf == NULL || file == NULL) {
		loge("input error: user_buf or file is NULL\n");
		return -EINVAL;
	}

	if (file->f_flags & O_NONBLOCK) {
		loge("file->f_flags & O_NONBLOCK(%d) fail\n",
			file->f_flags & O_NONBLOCK);
		return -EINVAL;
	}

	if (size != VOICE_PROXY_LIMIT_PARAM_SIZE) {
		loge("param err, size(%zd) < PROXY_EXTDEV_TX_DATA_SIZE:%d\n",
			size, VOICE_PROXY_LIMIT_PARAM_SIZE);
		return -EINVAL;
	}

	return 0;
}

static int proxy_extdev_voice_read_data(char __user *user_buf,
	const void *buffer, int32_t size)
{
	int ret;

	if (size != VOICE_PROXY_LIMIT_PARAM_SIZE) {
		loge("copy size is not right, size: %d\n", size);
		return -ENOEXEC;
	}

	ret = copy_to_user(user_buf, buffer, size);
	if (ret != 0) {
		loge("copy_to_user fail, ret: %d\n", ret);
		proxye_priv.tx_read_err_cnt++;
		ret = -ENOEXEC;
	} else {
		proxye_priv.tx_read_err_cnt = 0;
		ret = size;
	}
	return ret;
}

static int proxy_extdev_readcheck_node_size(
	struct virtual_voice_proxy_data_node *node,
	size_t size)
{
	if (size < node->list_data.size) {
		loge("size:%zd < node->list_data.size:%d\n",
			size, node->list_data.size);
		virtual_voice_data_node_free(&proxye_priv.tx_mem_pool,
			(struct virtual_voice_proxy_data_node **)(&node));
		return -EAGAIN;
	}
	return 0;
}

static ssize_t proxy_extdev_read(struct file *file, char __user *user_buf,
				size_t size, loff_t *ppos)
{
	int ret = 0;
	struct virtual_voice_proxy_data_node *node = NULL;

	UNUSED_PARAMETER(ppos);
	ret = proxy_extdev_read_check(file, user_buf, size);
	if (ret < 0)
		return ret;

	spin_lock_bh(&proxye_priv.read_lock);
	if (list_empty_careful(&send_voice_proxy_tx_queue)) {
		spin_unlock_bh(&proxye_priv.read_lock);
		ret = wait_event_interruptible_timeout(proxye_priv.read_waitq,
			proxye_priv.read_wait_flag > 0,
			msecs_to_jiffies(IO_BLOCK_TIMEOUT));
		if (ret <= 0) {
			loge("wait event fail, 0x%x.\n", ret);
			return -EBUSY;
		}
		spin_lock_bh(&proxye_priv.read_lock);
	}
	proxye_priv.read_wait_flag = 0;
	if (!list_empty_careful(&send_voice_proxy_tx_queue)) {
		node = list_first_entry(&send_voice_proxy_tx_queue,
					struct virtual_voice_proxy_data_node,
					list_node);
		if (node == NULL) {
			loge("node is NULL\n");
			spin_unlock_bh(&proxye_priv.read_lock);
			return -EAGAIN;
		}
		list_del_init(&node->list_node);
		if (proxye_priv.tx_cnt > 0)
			proxye_priv.tx_cnt--;
		ret = proxy_extdev_readcheck_node_size(node, size);
		if (ret < 0) {
			spin_unlock_bh(&proxye_priv.read_lock);
			return ret;
		}
		ret = proxy_extdev_voice_read_data(user_buf,
			(void *)(node->list_data.data), node->list_data.size);
		virtual_voice_data_node_free(&proxye_priv.tx_mem_pool,
			(struct virtual_voice_proxy_data_node **)(&node));
		node = NULL;
		spin_unlock_bh(&proxye_priv.read_lock);
		proxy_extdev_check_read_write_data_err_cnt();
	} else {
		spin_unlock_bh(&proxye_priv.read_lock);
		ret = -EAGAIN;
		logw("list is empty, read again\n");
	}
	return ret;
}

static int32_t virtual_voice_add_rx_data(int8_t *data, uint32_t size)
{
	int32_t ret;
	struct virtual_voice_proxy_data_node *node = NULL;

	if (data == NULL) {
		loge("input parameter data invalid");
		return -EINVAL;
	}

	spin_lock_bh(&proxye_priv.write_lock);
	if (proxye_priv.rx_cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
		spin_unlock_bh(&proxye_priv.write_lock);
		loge("out of queue, rx cnt:%d> %d\n", proxye_priv.rx_cnt,
			VOICE_PROXY_QUEUE_SIZE_MAX);
		return -ENOMEM;
	}
	spin_unlock_bh(&proxye_priv.write_lock);

	ret = virtual_voice_proxy_create_data_node(&proxye_priv.rx_mem_pool,
		&node, data, (int)size);
	if (ret != 0) {
		loge("node kzalloc failed\n");
		return -ENOMEM;
	}
	spin_lock_bh(&proxye_priv.write_lock);
	list_add_tail(&node->list_node, &recv_voice_proxy_rx_queue);
	proxye_priv.rx_cnt++;
	spin_unlock_bh(&proxye_priv.write_lock);

	return (int)size;
}

static int proxy_extdev_write_check(struct file *filp, const char __user *buff,
	size_t size)
{
	if (proxye_priv.disable_virtual_call)
		return -EOPNOTSUPP;

	if (!proxye_priv.is_proxy_open) {
		logw("the dev not open or voice call finished, not write\n");
		return -ECANCELED;
	}

	if (!proxye_priv.is_dev_open) {
		logw("the dev not open not write\n");
		return -ECANCELED;
	}

	if (proxye_priv.proxy_mode == VIRTUAL_CALL_STATUS_OFF) {
		logw("the proxy_mode is off not write\n");
		return -EREMOTEIO;
	}

	if (buff == NULL || filp == NULL) {
		loge("input error: buff or filp is NULL\n");
		return -EINVAL;
	}

	if (filp->f_flags & O_NONBLOCK) {
		loge("file->f_flags & O_NONBLOCK(%d) fail\n",
			filp->f_flags & O_NONBLOCK);
		return -EINVAL;
	}

	if (size != VOICE_PROXY_LIMIT_PARAM_SIZE) {
		loge("para error, size:%zd > %d\n", size,
			VOICE_PROXY_LIMIT_PARAM_SIZE);
		return -EINVAL;
	}

	return 0;
}

static int proxy_extdev_voice_write_data(void *buffer,
	const char __user *user_buf, int32_t size)
{
	int ret;

	if (size != VOICE_PROXY_LIMIT_PARAM_SIZE) {
		loge("copy size is not right, size: %d\n", size);
		return -ENOEXEC;
	}

	ret = copy_from_user(buffer, user_buf, size);
	if (ret != 0) {
		loge("copy_to_user fail, ret: 0x%x\n", ret);
		proxye_priv.rx_write_err_cnt++;
		ret = -ENOEXEC;
	} else {
		proxye_priv.rx_write_err_cnt = 0;
		ret = size;
	}
	return ret;
}

static ssize_t proxy_extdev_write(struct file *filp, const char __user *buff,
			size_t size, loff_t *offp)
{
	int32_t ret;
	int8_t *data = NULL;
	struct virtual_voice_proxy_data_node *node = NULL;

	UNUSED_PARAMETER(offp);
	ret = proxy_extdev_write_check(filp, buff, size);
	if (ret < 0)
		return ret;

	node = virtual_voice_data_node_malloc(&proxye_priv.rx_mem_pool,
		sizeof(*node) + size);
	if (node == NULL) {
		loge("kzalloc rev node failed\n");
		return -ENOMEM;
	}
	data = (int8_t *)node->list_data.data;
	ret = proxy_extdev_voice_write_data((void *)data, buff, size);
	if (ret < 0)
		goto extdev_write_err;

	ret = wait_event_interruptible_timeout(proxye_priv.write_waitq,
		proxye_priv.write_wait_flag > 0,
		msecs_to_jiffies(IO_BLOCK_TIMEOUT));
	if (ret <= 0) {
		if (ret != -ERESTARTSYS)
			loge("wait event interruptible fail, 0x%x.\n", ret);

		ret = -EBUSY;
		goto extdev_write_err;
	}
	spin_lock_bh(&proxye_priv.write_lock);
	proxye_priv.write_wait_flag = 0;
	spin_unlock_bh(&proxye_priv.write_lock);

	ret = virtual_voice_proxy_add_data(virtual_voice_add_rx_data,
		data, (uint32_t)size, ID_VIRTUAL_VOICE_PROXY_RX_NTF);
	if (ret <= 0) {
		loge("call voice_proxy_add_data fail\n");
		ret = -EAGAIN;
		goto extdev_write_err;
	}
	ret = (int)size;
extdev_write_err:
	virtual_voice_data_node_free(&proxye_priv.rx_mem_pool,
		(struct virtual_voice_proxy_data_node **)(&node));
	proxy_extdev_check_read_write_data_err_cnt();
	node = NULL;
	return ret;
}

static int proxy_extdev_set_mode_to_hifi(struct virtual_voice_proxy_mode mode_msg)
{
	int ret;

	logi("mode_msg.mode [%d]\n", mode_msg.mode);
	if (mode_msg.mode == VIRTUAL_CALL_STATUS_ON) {
		proxy_extdev_clear_queue(&proxye_priv.rx_mem_pool,
			&recv_voice_proxy_rx_queue);
		proxye_priv.rx_cnt = 0;
	}

	ret = voice_proxy_mailbox_send_msg_cb(
		MAILBOX_MAILCODE_ACPU_TO_HIFI_VIRTUAL_VOICE,
		mode_msg.msg_id, (void *)&mode_msg,
		(uint32_t)sizeof(mode_msg));
	if (ret) {
		loge("send_mailbox_msg fail, ret:%d\n", ret);
		return -EFAULT;
	}
	proxye_priv.proxy_mode = mode_msg.mode;

	return 0;
}

static void proxy_extdev_check_read_write_data_err_cnt(void)
{
	struct virtual_voice_proxy_mode mode_msg;

	if (proxye_priv.tx_read_err_cnt > VIRTUAL_VOICE_WR_ERR_LIMIT ||
		proxye_priv.rx_write_err_cnt > VIRTUAL_VOICE_WR_ERR_LIMIT) {
		loge("err_cnt exceed limit, set off mde rd_cnt:%d, wr_cnt:%d\n",
			proxye_priv.tx_read_err_cnt,
			proxye_priv.rx_write_err_cnt);
		mode_msg.mode = VIRTUAL_CALL_STATUS_OFF;
		mode_msg.msg_id = ID_VIRTUAL_VOICE_PROXY_SET_MODE;
		proxy_extdev_set_mode_to_hifi(mode_msg);
		proxye_priv.is_proxy_open = false;
		proxye_priv.proxy_mode = VIRTUAL_CALL_STATUS_OFF;
		proxye_priv.read_wait_flag++;
		proxye_priv.write_wait_flag++;
		wake_up(&proxye_priv.read_waitq);
		wake_up(&proxye_priv.write_waitq);
		proxy_extdev_clear_queue(&proxye_priv.rx_mem_pool,
			&recv_voice_proxy_rx_queue);
		proxy_extdev_clear_queue(&proxye_priv.tx_mem_pool,
			&send_voice_proxy_tx_queue);
	}
}

static int proxy_extdev_set_mode(uintptr_t arg)
{
	int ret;
	struct virtual_voice_proxy_mode mode_msg;

	if (copy_from_user(&mode_msg.mode, (void *)arg,
		sizeof(mode_msg.mode))) {
		loge("copy_from_user fail.\n");
		return -EIO;
	}
	logi("set mode_msg.mode:%d\n", mode_msg.mode);
	mode_msg.msg_id = ID_VIRTUAL_VOICE_PROXY_SET_MODE;
	ret = proxy_extdev_set_mode_to_hifi(mode_msg);
	if (mode_msg.mode == VIRTUAL_CALL_STATUS_OFF) {
		proxye_priv.read_wait_flag++;
		proxye_priv.write_wait_flag++;
		wake_up(&proxye_priv.read_waitq);
		wake_up(&proxye_priv.write_waitq);
	}

	return ret;
}

static int proxy_extdev_set_driver_status_to_hifi(void)
{
	int ret;
	struct virtual_voice_proxy_driver_status status_msg;

	status_msg.msg_id = ID_VIRTUAL_VOICE_PROXY_SET_PROXY_STATUS;
	status_msg.status = VIRTUAL_CALL_STATUS_REGISTER;

	ret = voice_proxy_mailbox_send_msg_cb(
		MAILBOX_MAILCODE_ACPU_TO_HIFI_VIRTUAL_VOICE,
		status_msg.msg_id, (void *)&status_msg,
		(uint32_t)sizeof(status_msg));
	if (ret) {
		loge("send_mailbox_msg fail, ret:%d\n", ret);
		return -EFAULT;
	}

	return 0;
}

static int proxy_extdev_open(struct inode *finode, struct file *fd)
{
	UNUSED_PARAMETER(finode);
	UNUSED_PARAMETER(fd);
	if (proxye_priv.disable_virtual_call) {
		logw("virtual vioce dev is disable, not open\n");
		return -EOPNOTSUPP;
	}

	logi("open dev by virtual processor\n");
	proxy_extdev_set_driver_status_to_hifi();
	spin_lock_bh(&proxye_priv.write_lock);
	proxye_priv.is_dev_open = true;
	proxye_priv.is_proxy_open = true;
	proxye_priv.tx_read_err_cnt = 0;
	proxye_priv.rx_write_err_cnt = 0;
	proxye_priv.rx_cnt = 0;
	proxye_priv.write_wait_flag = 0;
	proxye_priv.tx_cnt = 0;
	proxye_priv.read_wait_flag = 0;
	proxy_extdev_clear_queue(&proxye_priv.rx_mem_pool,
		&recv_voice_proxy_rx_queue);
	proxy_extdev_clear_queue(&proxye_priv.tx_mem_pool,
		&send_voice_proxy_tx_queue);
	spin_unlock_bh(&proxye_priv.write_lock);

	return 0;
}

static int proxy_extdev_close(struct inode *node, struct file *filp)
{
	struct virtual_voice_proxy_mode mode_msg;

	UNUSED_PARAMETER(node);
	UNUSED_PARAMETER(filp);
	if (proxye_priv.disable_virtual_call) {
		logw("virtual vioce dev is disable\n");
		return -EOPNOTSUPP;
	}

	logi("close dev by virtual processor\n");
	spin_lock_bh(&proxye_priv.read_lock);
	proxye_priv.read_wait_flag++;
	proxye_priv.is_proxy_open = false;
	proxye_priv.is_dev_open = false;
	proxye_priv.write_wait_flag++;
	spin_unlock_bh(&proxye_priv.read_lock);
	wake_up(&proxye_priv.read_waitq);
	wake_up(&proxye_priv.write_waitq);
	mode_msg.mode = VIRTUAL_CALL_STATUS_OFF;
	mode_msg.msg_id = ID_VIRTUAL_VOICE_PROXY_SET_MODE;
	proxy_extdev_set_mode_to_hifi(mode_msg);
	proxy_extdev_clear_queue(&proxye_priv.rx_mem_pool,
		&recv_voice_proxy_rx_queue);
	proxy_extdev_clear_queue(&proxye_priv.tx_mem_pool,
		&send_voice_proxy_tx_queue);
	virtual_voice_data_memory_pool_free();
	virtual_voice_cmd_memory_pool_free();
	return 0;
}

static void proxy_extdev_wake_up_read(void)
{
	spin_lock_bh(&proxye_priv.read_lock);
	proxye_priv.read_wait_flag++;
	spin_unlock_bh(&proxye_priv.read_lock);
	wake_up(&proxye_priv.read_waitq);
}

static long proxy_extdev_ioctl(struct file *fd,
		unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	void __user *data = (void __user *)(uintptr_t)arg;

	if (proxye_priv.disable_virtual_call) {
		logw("virtual vioce dev is disable, not support\n");
		return -EOPNOTSUPP;
	}

	UNUSED_PARAMETER(fd);
	if ((void __user *)(uintptr_t)arg == NULL) {
		loge("input error: arg is NULL\n");
		return -EINVAL;
	}

	switch (cmd) {
	case VIRTUAL_PROXY_VOICE_WAKE_UP_READ:
		proxy_extdev_wake_up_read();
		break;
	case VIRTUAL_PROXY_VOICE_SET_MODE:
		ret = proxy_extdev_set_mode((uintptr_t)data);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations proxy_extdev_misc_fops = {
	.owner = THIS_MODULE,
	.open = proxy_extdev_open,
	.read = proxy_extdev_read,
	.write = proxy_extdev_write,
	.release = proxy_extdev_close,
	.unlocked_ioctl = proxy_extdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = proxy_extdev_ioctl,
#endif
};

static struct miscdevice proxy_extdev_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "voice_proxy_extdev",
	.fops = &proxy_extdev_misc_fops,
};

static long voice_proxy_ctl_ioctl(struct file *fd,
		unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	UNUSED_PARAMETER(fd);
	UNUSED_PARAMETER(arg);
	switch (cmd) {
	case VIRTUAL_PROXY_VOICE_SET_DISABLE:
		proxye_priv.disable_virtual_call = true;
		logi("disable virtual voice proxy extdev\n");
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations voice_proxy_ctl_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.unlocked_ioctl = voice_proxy_ctl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = voice_proxy_ctl_ioctl,
#endif
};

static struct miscdevice voice_proxy_ctl_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "voice_proxy_ctl_dev",
	.fops   = &voice_proxy_ctl_fops,
};

static void proxy_extdev_sign_init(void)
{
	proxye_priv.rx_confirm = false;
	proxye_priv.first_rx = true;
}

static int32_t proxy_extdev_add_tx_data(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;
	struct virtual_voice_proxy_data_node *node = NULL;

	UNUSED_PARAMETER(rev_buf);
	UNUSED_PARAMETER(buf_size);

	spin_lock_bh(&proxye_priv.read_lock);
	if (proxye_priv.tx_cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
		logw("tx_cnt[%d] is out of range\n", proxye_priv.tx_cnt);
		proxye_priv.read_wait_flag++;
		spin_unlock_bh(&proxye_priv.read_lock);
		wake_up(&proxye_priv.read_waitq);
		return -ENOMEM;
	}
	spin_unlock_bh(&proxye_priv.read_lock);

	if (proxye_priv.spkdata_buf == NULL) {
		loge("priv.spkdata_buffer is NULL\n");
		goto add_tx_data_err;
	}

	ret = virtual_voice_proxy_create_data_node(&proxye_priv.tx_mem_pool,
		&node, proxye_priv.spkdata_buf,
		(int)VOICE_PROXY_LIMIT_PARAM_SIZE);
	if (ret != 0) {
		loge("data_node kzalloc failed\n");
		ret = -ENOMEM;
		goto add_tx_data_err;
	}

	spin_lock_bh(&proxye_priv.read_lock);
	list_add_tail(&node->list_node, &send_voice_proxy_tx_queue);
	proxye_priv.tx_cnt++;
	proxye_priv.read_wait_flag++;
	spin_unlock_bh(&proxye_priv.read_lock);
	wake_up(&proxye_priv.read_waitq);

add_tx_data_err:
	return ret;
}

static void proxy_extdev_get_rx_data(uint32_t *size)
{
	struct virtual_voice_proxy_data_node *node = NULL;

	if (size == NULL) {
		loge("param size is NULL\n");
		return;
	}
	spin_lock_bh(&proxye_priv.write_lock);
	if (!list_empty_careful(&recv_voice_proxy_rx_queue)) {
		node = list_first_entry(&recv_voice_proxy_rx_queue,
					struct virtual_voice_proxy_data_node,
					list_node);
		if (node == NULL) {
			loge("node is NULL\n");
			spin_unlock_bh(&proxye_priv.write_lock);
			return;
		}
		list_del_init(&node->list_node);
		if (proxye_priv.rx_cnt > 0)
			proxye_priv.rx_cnt--;

		if (*size < (uint32_t)node->list_data.size) {
			loge("size is invalid,size = %d,list_data.size = %d\n",
				*size, node->list_data.size);
			virtual_voice_data_node_free(&proxye_priv.rx_mem_pool,
				(struct virtual_voice_proxy_data_node **)(&node));
			node = NULL;
			*size = 0;
			spin_unlock_bh(&proxye_priv.write_lock);
			return;
		}
		*size = (uint32_t)node->list_data.size;
		if (proxye_priv.micdata_buf != NULL)
			memcpy(proxye_priv.micdata_buf, node->list_data.data,
				(size_t)*size);
		else
			loge("priv.micdata_buffer is NULL\n");

		virtual_voice_data_node_free(&proxye_priv.rx_mem_pool,
			(struct virtual_voice_proxy_data_node **)(&node));
		node = NULL;
		spin_unlock_bh(&proxye_priv.write_lock);
		proxye_priv.first_rx = false;
		proxye_priv.rx_confirm = false;
	} else {
		spin_unlock_bh(&proxye_priv.write_lock);
		*size = 0;
	}
}

static int proxy_extdev_status_notify_check(int8_t *rev_buf,
		uint32_t buf_size)
{
	if (rev_buf == NULL) {
		loge("proxy_extdev:status notify param rev_buf is NULL\n");
		return -EINVAL;
	}

	if (buf_size < sizeof(struct virtual_voice_proxy_rev_msg)) {
		loge("msg size is %u error, it not less than %ld\n",
			buf_size, sizeof(struct virtual_voice_proxy_rev_msg));
		return -EINVAL;
	}
	return 0;
}

static void proxy_extdev_receive_status_notify(int8_t *rev_buf,
		uint32_t buf_size)
{
	struct virtual_voice_proxy_rev_msg *status_ind = NULL;

	if (proxy_extdev_status_notify_check(rev_buf, buf_size) < 0)
		return;

	status_ind = (struct virtual_voice_proxy_rev_msg *)rev_buf;
	logi("proxy_extdev:status indication, status is %u(1-open,0-close)\n",
		status_ind->voice_status);
	if (status_ind->voice_status == STATUS_OPEN) {
		spin_lock_bh(&proxye_priv.read_lock);
		if (!proxye_priv.is_proxy_open) {
			proxye_priv.tx_cnt = 0;
			proxye_priv.read_wait_flag = 0;
			proxye_priv.rx_cnt = 0;
			proxye_priv.write_wait_flag = 0;
		}
		proxye_priv.is_proxy_open = true;
		proxye_priv.first_rx = true;
		proxye_priv.rx_confirm = false;
		proxye_priv.tx_read_err_cnt = 0;
		proxye_priv.rx_write_err_cnt = 0;
		spin_unlock_bh(&proxye_priv.read_lock);
	} else if (status_ind->voice_status == STATUS_CLOSE) {
		spin_lock_bh(&proxye_priv.read_lock);
		if (proxye_priv.is_dev_open) {
			logi("set read and write wakeup waitq\n");
			proxye_priv.read_wait_flag++;
			proxye_priv.write_wait_flag++;
		}
		proxye_priv.is_proxy_open = false;
		proxye_priv.proxy_mode = VIRTUAL_CALL_STATUS_OFF;
		proxy_extdev_clear_queue(&proxye_priv.rx_mem_pool,
			&recv_voice_proxy_rx_queue);
		proxy_extdev_clear_queue(&proxye_priv.tx_mem_pool,
			&send_voice_proxy_tx_queue);
		spin_unlock_bh(&proxye_priv.read_lock);
		virtual_voice_data_memory_pool_free();
		virtual_voice_cmd_memory_pool_free();
	}
	if (status_ind->voice_status == STATUS_CLOSE &&
		proxye_priv.is_dev_open) {
		logi("the voice finished and dev still open, wakeup waitq\n");
		wake_up(&proxye_priv.read_waitq);
		wake_up(&proxye_priv.write_waitq);
	}
}

static void proxy_extdev_receive_tx_notify(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;

	if (rev_buf == NULL) {
		loge("receive_tx_ntf fail, param rev_buf is NULL\n");
		return;
	}

	ret = proxy_extdev_add_tx_data(rev_buf, buf_size);
	if (ret) {
		loge("send tx data to read func failed\n");
		return;
	}

	/* hifi only need channel id */
	virtual_voice_proxy_add_work_queue_cmd(ID_VIRTUAL_VOICE_PROXY_TX_CNF);
}

static void proxy_extdev_receive_rx_confirm(int8_t *rev_buf, uint32_t buf_size)
{

	UNUSED_PARAMETER(rev_buf);
	UNUSED_PARAMETER(buf_size);

	proxye_priv.rx_confirm = true;
}

static void proxy_extdev_handle_rx_notify(uint32_t *size, uint16_t *msg_id)
{

	if (size == NULL || msg_id == NULL) {
		loge("handle_rx_ntf fail, param is NULL\n");
		return;
	}

	virtual_voice_proxy_set_send_sign(proxye_priv.first_rx,
		&proxye_priv.rx_confirm,
		&proxye_priv.rx_stamp);

	if (proxye_priv.first_rx || proxye_priv.rx_confirm)
		proxy_extdev_get_rx_data(size);
	else
		*size = 0;

	*msg_id = ID_VIRTUAL_VOICE_PROXY_RX_NTF;
}

static void proxy_extdev_handle_request_micdata(int8_t *rev_buf,
		uint32_t buf_size)
{
	UNUSED_PARAMETER(rev_buf);
	UNUSED_PARAMETER(buf_size);

	spin_lock_bh(&proxye_priv.write_lock);
	proxye_priv.write_wait_flag++;
	spin_unlock_bh(&proxye_priv.write_lock);
	wake_up(&proxye_priv.write_waitq);
}

static void proxy_extdev_proxy_register_msg_cmd_callback(void)
{
	virtual_voice_proxy_register_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_STATUS_NTF,
		proxy_extdev_receive_status_notify);

	virtual_voice_proxy_register_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_TX_NTF,
		proxy_extdev_receive_tx_notify);

	virtual_voice_proxy_register_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_RX_CNF,
		proxy_extdev_receive_rx_confirm);

	virtual_voice_proxy_register_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_REQUEST_MIC_DATA,
		proxy_extdev_handle_request_micdata);

	virtual_voice_proxy_register_cmd_callback(
		ID_VIRTUAL_VOICE_PROXY_RX_NTF,
		proxy_extdev_handle_rx_notify);

	virtual_voice_proxy_register_sign_init_callback(proxy_extdev_sign_init);
}

static void proxy_extdev_proxy_deregister_msg_cmd_callback(void)
{
	virtual_voice_proxy_deregister_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_TX_NTF);

	virtual_voice_proxy_deregister_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_RX_CNF);

	virtual_voice_proxy_deregister_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_STATUS_NTF);

	virtual_voice_proxy_deregister_msg_callback(
		ID_VIRTUAL_VOICE_PROXY_REQUEST_MIC_DATA);

	virtual_voice_proxy_deregister_cmd_callback(
		ID_VIRTUAL_VOICE_PROXY_RX_NTF);

	virtual_voice_proxy_deregister_sign_init_callback(
		proxy_extdev_sign_init);
}

static void proxy_extdev_priv_data_init(void)
{
	unsigned int size = sizeof(struct virtual_voice_proxy_data_node) +
		VOICE_PROXY_LIMIT_PARAM_SIZE;

	memset(&proxye_priv, 0, sizeof(proxye_priv));
	proxye_priv.read_wait_flag = 0;
	proxye_priv.write_wait_flag = 0;
	proxye_priv.disable_virtual_call = false;
	proxye_priv.is_dev_open = false;
	proxye_priv.is_proxy_open = false;
	proxye_priv.proxy_mode = VIRTUAL_CALL_STATUS_OFF;
	spin_lock_init(&proxye_priv.read_lock);
	spin_lock_init(&proxye_priv.write_lock);
	init_waitqueue_head(&proxye_priv.read_waitq);
	init_waitqueue_head(&proxye_priv.write_waitq);
	virtual_voice_memory_pool_init(&proxye_priv.tx_mem_pool, size);
	virtual_voice_memory_pool_init(&proxye_priv.rx_mem_pool, size);
}

static int proxy_extdev_probe(struct platform_device *pdev)
{
	int32_t ret;

	UNUSED_PARAMETER(pdev);
	proxy_extdev_priv_data_init();
	ret = misc_register(&proxy_extdev_misc_device);
	if (ret) {
		loge("voice proxy extdev misc register fail\n");
		return ret;
	}

	proxy_extdev_sign_init();
	proxy_extdev_proxy_register_msg_cmd_callback();

	ret = misc_register(&voice_proxy_ctl_device);
	if (ret) {
		loge("voice proxy extdev ctl misc register fail\n");
		goto micdata_err1;
	}

	proxye_priv.micdata_buf = ioremap_wc(HISI_AP_VIRTUAL_CALL_UPLINK_ADDR,
					HISI_AP_VIRTUAL_CALL_UPLINK_SIZE);
	if (proxye_priv.micdata_buf == NULL) {
		loge("micdata_buffer buffer ioremap err\n");
		goto micdata_err1;
	}

	proxye_priv.spkdata_buf = ioremap_wc(HISI_AP_VIRTUAL_CALL_DOWNLINK_ADDR,
					HISI_AP_VIRTUAL_CALL_DOWNLINK_SIZE);
	if (proxye_priv.spkdata_buf == NULL) {
		loge("spkdata_buffer buffer ioremap err\n");
		goto spkdata_err2;
	}
	proxy_extdev_set_driver_status_to_hifi();
	return ret;

spkdata_err2:
	if (proxye_priv.micdata_buf != NULL) {
		iounmap(proxye_priv.micdata_buf);
		proxye_priv.micdata_buf = NULL;
	}
micdata_err1:
	misc_deregister(&proxy_extdev_misc_device);
	proxy_extdev_proxy_deregister_msg_cmd_callback();
	misc_deregister(&voice_proxy_ctl_device);

	return -EFAULT;
}

static int proxy_extdev_remove(struct platform_device *pdev)
{
	UNUSED_PARAMETER(pdev);
	misc_deregister(&proxy_extdev_misc_device);
	proxy_extdev_proxy_deregister_msg_cmd_callback();
	misc_deregister(&voice_proxy_ctl_device);
	virtual_voice_data_memory_pool_free();

	if (proxye_priv.micdata_buf != NULL) {
		iounmap(proxye_priv.micdata_buf);
		proxye_priv.micdata_buf = NULL;
	}

	if (proxye_priv.spkdata_buf != NULL) {
		iounmap(proxye_priv.spkdata_buf);
		proxye_priv.spkdata_buf = NULL;
	}
	return 0;
}

static const struct of_device_id proxy_extdev_match_table[] = {
	{
		.compatible = DTS_VIR_VOICE_PROXY_EXTDEV_NAME,
		.data = NULL,
	},
	{}
};

static struct platform_driver proxy_extdev_driver = {
	.driver = {
		.name  = "virtual_voice_proxy_extdev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(proxy_extdev_match_table),
	},
	.probe = proxy_extdev_probe,
	.remove = proxy_extdev_remove,
};


static int __init proxy_extdev_init(void)
{
	int32_t ret;

	logi("proxy extdev init\n");

	ret = platform_driver_register(&proxy_extdev_driver);
	if (ret)
		loge("voice_proxy extdev register fail,ERROR is %d\n", ret);

	return ret;
}

static void __exit proxy_extdev_exit(void)
{
	logi("proxy extdev exit\n");

	platform_driver_unregister(&proxy_extdev_driver);
}

module_init(proxy_extdev_init);
module_exit(proxy_extdev_exit);

MODULE_DESCRIPTION("virtual voice proxy extdev driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

