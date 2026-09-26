/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "securec.h"
#include "product_config.h"
#include "NetMgrCtrlVcom.h"
#include "NetMgrCtrlInterface.h"
#include "NetMgrPrivate.h"


#if (FEATURE_ACPU_PHONE_MODE == FEATURE_ON)
STATIC struct class *nm_class;
STATIC struct nm_vcom_context nm_vcom_ctx;
STATIC unsigned int nm_bind_pid = 0;

/*
 * nm_set_channel - Set channel infomation.
 * @ch: channel context
 */
STATIC void nm_set_channel(struct nm_vcom_channel *ch)
{
	nm_vcom_ctx.ch = ch;
}

/*
 * nm_get_channel - Get channel infomation.
 *
 * Return pointer to channel context
 */
STATIC struct nm_vcom_channel *nm_get_channel(void)
{
	return nm_vcom_ctx.ch;
}

/*
 * nm_queue_empty - Check if messsage queue is empty.
 * @ch: channel context
 */
STATIC bool nm_queue_empty(struct nm_vcom_channel *ch)
{
	bool ret = true;

	mutex_lock(&ch->lock);
	ret = list_empty(&ch->hp_q.head) && list_empty(&ch->lp_q.head);
	mutex_unlock(&ch->lock);

	return ret;
}

/*
 * __nm_queue_flush - Flush message queue.
 * @q: message queue
 *
 * This must be called under mutex lock of nm channel.
 */
STATIC void __nm_queue_flush(struct nm_msg_queue *q)
{
	struct nm_msg_entry *entry = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	list_for_each_safe(pos, tmp, &q->head) {
		entry = list_entry(pos, struct nm_msg_entry, list);
		list_del(&entry->list);
		kfree(entry);
		entry = NULL;
	}
}

/*
 * nm_queue_flush - Flush message queue.
 * @q: message queue
 *
 * This can be called from any context except irq context.
 */
STATIC void nm_queue_flush(struct nm_msg_queue *q)
{
	mutex_lock(&q->ch->lock);
	__nm_queue_flush(q);
	mutex_unlock(&q->ch->lock);
}

/*
 * nm_enqueue_msg - Enqueue message to specific queue.
 * @q: message queue
 * @entry: message entry
 */
STATIC void nm_enqueue_msg(struct nm_msg_queue *q, struct nm_msg_entry *entry)
{
	mutex_lock(&q->ch->lock);
	if (q->single)
		__nm_queue_flush(q);
	list_add_tail(&entry->list, &q->head);
	mutex_unlock(&q->ch->lock);

	wake_up_interruptible(&q->ch->wait);
}

/*
 * nm_dequeue_msg - Dequeue message from specific queue.
 * @q: message queue
 *
 * Return pointer to message entry, or null if message queue is empty.
 */
STATIC struct nm_msg_entry *nm_dequeue_msg(struct nm_msg_queue *q)
{
	struct nm_msg_entry *entry = NULL;

	mutex_lock(&q->ch->lock);
	entry = list_first_entry_or_null(&q->head, struct nm_msg_entry, list);
	if (entry != NULL)
		list_del_init(&entry->list);
	mutex_unlock(&q->ch->lock);

	return entry;
}

/*
 * nm_send_msg - Send nm message to read buffer of nmctrlvcom.
 * @data: message data
 * @len: message length
 * @high_prio: high priority flag
 *
 * Return 0 on success, negative on failure.
 */
/*lint -save -e801*/
int nm_send_msg(const void *data, unsigned int len, bool high_prio)
{
	struct nm_vcom_channel *ch = nm_get_channel();
	struct nm_msg_entry *entry = NULL;
	int ret = 0;

	if (unlikely(ch == NULL)) {
		NM_CTRL_LOGE("device not exist.");
		ret = -ENODEV;
		goto out;
	}

	if (unlikely(data == NULL || len != sizeof(NM_MSG_STRU))) {
		NM_CTRL_LOGE("message is invalid.");
		ret = -EINVAL;
		goto out;
	}

	entry = kzalloc(sizeof(*entry) + len, GFP_KERNEL);
	if (entry == NULL) {
		NM_CTRL_LOGE("alloc entry failed.");
		ret = -ENOMEM;
		goto out;
	}

	entry->len = len;
	memcpy_s(entry->data, len, data, len);
	nm_enqueue_msg(high_prio ? &ch->hp_q : &ch->lp_q, entry);

out:
	return ret;
}
/*lint -restore +e801*/

/*
 * nm_vcom_read - Read function of cdev.
 * @filp: file handle
 * @buf: the user space buffer to write to
 * @size: the size of the user space buffer
 * @ppos : the current position in the buffer
 *
 * On success, the number of bytes read is returned, or negative value is
 * returned on error.
 */
/*lint -save -e801*/
STATIC ssize_t nm_vcom_read(struct file *filp, char __user *buf,
			    size_t count, loff_t *ppos)
{
	struct nm_vcom_channel *ch = filp->private_data;
	struct nm_msg_entry *entry = NULL;
	loff_t tmp = 0;
	ssize_t ret = 0;

	NM_CTRL_LOGI("size is %lu.", (unsigned long)count);

	if (filp->f_flags & O_NONBLOCK) {
	 	NM_CTRL_LOGE("NONBLOCK.");
	 	return -EAGAIN;
	}

	if (wait_event_interruptible(ch->wait, !nm_queue_empty(ch)))
		return -ERESTARTSYS;

	entry = nm_dequeue_msg(&ch->hp_q);
	if (entry != NULL)
		goto read;

	entry = nm_dequeue_msg(&ch->lp_q);
	if (entry == NULL)
		goto out;

read:
	ret = simple_read_from_buffer(buf, count, &tmp, entry->data, entry->len);
	if (ret < 0)
		NM_CTRL_LOGE("input buf read error, return: %zd.", ret);

	kfree(entry);
	entry = NULL;
out:
	return ret;
}
/*lint -restore +e801*/

/*
 * nm_vcom_poll - Poll function of cdev.
 * @filp: file handle
 * @wait: wait list
 */
STATIC unsigned int nm_vcom_poll(struct file* filp, poll_table *wait)
{
	struct nm_vcom_channel *ch = filp->private_data;
	unsigned int mask = 0;

	/* put the queue into poll_table */
	poll_wait(filp, &ch->wait, wait);

	if (!nm_queue_empty(ch))
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

/*
 * nm_vcom_open - Open netmanager cdev.
 * @node: node
 * @filp: file handle
 *
 * Return 0 on success, negative on failure.
 */
STATIC int nm_vcom_open(struct inode *node, struct file *filp)
{
	struct nm_vcom_channel *ch = container_of(node->i_cdev,
						  struct nm_vcom_channel, cdev);

	filp->private_data = ch;

	return 0;
}

/*
 * nm_vcom_release - Release netmanager cdev.
 * @node: node
 * @filp: file handle
 *
 * Return 0 on success, negative on failure
 */
STATIC int nm_vcom_release(struct inode *node, struct file *filp)
{
	struct nm_vcom_channel *ch = filp->private_data;

	nm_queue_flush(&ch->hp_q);
	nm_queue_flush(&ch->lp_q);

	return 0;
}

STATIC const struct file_operations nm_vcom_fops = {
	.owner		= THIS_MODULE,
	.read		= nm_vcom_read,
	.poll		= nm_vcom_poll,
	.open		= nm_vcom_open,
	.release	= nm_vcom_release,
};

/*
 * nm_vcom_init - Module init function.
 *
 * Return 0 on success, negative on failure.
 */
/*lint -save -e801*/
STATIC int nm_vcom_init(void)
{
	struct nm_vcom_channel *ch = NULL;
	int ret;

	ch = kzalloc(sizeof(*ch), GFP_KERNEL);
	if (ch == NULL) {
		NM_CTRL_LOGE("alloc channel failed.");
		ret = -ENOMEM;
		goto err_alloc_cha;
	}

	ret = alloc_chrdev_region(&ch->devno, 0, 1, NM_VCOM_NAME);
	if (ret) {
		NM_CTRL_LOGE("alloc_chrdev_region failed.");
		goto err_alloc_chrdev;
	}

	cdev_init(&ch->cdev, &nm_vcom_fops);
	ch->cdev.owner = THIS_MODULE;

	ret = cdev_add(&ch->cdev, ch->devno, 1);
	if (ret) {
		NM_CTRL_LOGE("add cdev failed.");
		goto err_add_cdev;
	}

	nm_class = class_create(THIS_MODULE, NM_VCOM_NAME);
	ret = PTR_ERR_OR_ZERO(nm_class);
	if (ret) {
		NM_CTRL_LOGE("create class failed.");
		nm_class = NULL;
		goto err_create_class;
	}

	ch->class_device = device_create(nm_class, NULL, ch->devno,
					 NULL, NM_VCOM_NAME);
	ret = PTR_ERR_OR_ZERO(ch->class_device);
	if (ret) {
		NM_CTRL_LOGE("create device failed.");
		ch->class_device = NULL;
		goto err_create_device;
	}

	init_waitqueue_head(&ch->wait);
	mutex_init(&ch->lock);

	ch->hp_q.ch = ch;
	ch->hp_q.single = false;
	INIT_LIST_HEAD(&ch->hp_q.head);
	ch->lp_q.ch = ch;
	ch->lp_q.single = true;
	INIT_LIST_HEAD(&ch->lp_q.head);

	nm_set_channel(ch);

	return 0;

err_create_device:
	class_destroy(nm_class);
err_create_class:
	cdev_del(&ch->cdev);
err_add_cdev:
	unregister_chrdev_region(ch->devno, 1);
err_alloc_chrdev:
	kfree(ch);
	ch = NULL;
err_alloc_cha:
	return ret;
}
/*lint -restore +e801*/

/*
 * nm_vcom_exit - Module exit function.
 */
STATIC void nm_vcom_exit(void)
{
	struct nm_vcom_channel *ch = nm_get_channel();

	if (ch != NULL) {
		device_destroy(nm_class, ch->devno);
		class_destroy(nm_class);
		cdev_del(&ch->cdev);
		unregister_chrdev_region(ch->devno, 1);
		kfree(ch);
		ch = NULL;
		nm_set_channel(NULL);
	}
}

/*
 * nm_bind_pid_read - Read function of bind_pid proc file.
 * @file: file handle
 * @user_buf: the user space buffer to write to
 * @count: the size of the user space buffer
 * @ppos : the current position in the buffer
 *
 * On success, the number of bytes read is returned, or negative value is
 * returned on error.
 */
STATIC ssize_t nm_bind_pid_read(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	char buf[NM_BIND_PID_BUF_LEN];
	ssize_t bytes_read;

	if (*ppos > 0) {
		NM_CTRL_LOGE("only allow read from start.");
		return 0;
	}

	(void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
	scnprintf(buf, sizeof(buf), "%u", nm_bind_pid);

	bytes_read = simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
	if (bytes_read < 0)
		NM_CTRL_LOGE("input buf read error, return: %zd.", bytes_read);

	return bytes_read;
}

/*
 * nm_bind_pid_write - Write function of bind_pid file.
 * @file: file handle
 * @user_buf: the user space buffer to write to
 * @count: the size of the user space buffer
 * @ppos : the current position in the buffer
 *
 * On success, the number of bytes written is returned, or negative value is
 * returned on error.
 */
STATIC ssize_t nm_bind_pid_write(struct file *file, const char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	NM_MSG_STRU msg;
	char buf[NM_BIND_PID_BUF_LEN];
	size_t buf_size;
	ssize_t bytes_written;
	int pid_value = 0;

	if (*ppos > 0) {
		NM_CTRL_LOGE("only allow write from start.");
		return 0;
	}

	(void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
	buf_size = count < (sizeof(buf) - 1) ? count : (sizeof(buf) - 1);

	bytes_written = simple_write_to_buffer(buf, buf_size, ppos, user_buf, count);
	if (bytes_written <= 0) {
		NM_CTRL_LOGE("input buf read error: %d.", (int)bytes_written);
		return bytes_written;
	}

	if ((size_t)bytes_written != count) {
		NM_CTRL_LOGE("input buf too long: %s.", buf);
		return -EINVAL;
	}

	if(!isdigit(buf[0])) {
		NM_CTRL_LOGE("input buf is error: %s.", buf);
		return -EINVAL;
	}

	if (kstrtoint(&buf[0], 10, &pid_value)) {
		NM_CTRL_LOGE("convert to int fail: %s.", buf);
		return -EINVAL;
	}

	nm_bind_pid = (unsigned int)pid_value;

	memset_s(&msg, sizeof(msg), 0, sizeof(msg));
	msg.enMsgId = ID_NM_BIND_PID_CONFIG_IND;
	msg.ulMsgLen = sizeof(unsigned int);
	msg.ulBindPid = (unsigned int)pid_value;

	nm_send_msg(&msg, sizeof(msg), false);

	return bytes_written;
}

STATIC const struct file_operations nm_bind_pid_fops = {
	.owner = THIS_MODULE,
	.read  = nm_bind_pid_read,
	.write = nm_bind_pid_write,
};

/*
 * nm_bind_pid_init - Module init function.
 *
 * Return 0 on success, negative on failure.
 */
STATIC int nm_bind_pid_init(void)
{
	struct proc_dir_entry *proc_entry;

	proc_entry = proc_create(NM_PROC_FILE_BIND_PID, 0660, NULL,
				 &nm_bind_pid_fops);
	if (proc_entry == NULL) {
		NM_CTRL_LOGE("fail.");
		return -ENOMEM;
	}

	return 0;
}

/*
 * nm_bind_pid_exit - Module exit function.
 */
STATIC void nm_bind_pid_exit(void)
{
	remove_proc_entry(NM_PROC_FILE_BIND_PID, NULL);
	NM_CTRL_LOGI("succ.");
}

/*
 * nm_init - Module init function.
 *
 * Return 0 on success, negative on failure.
 */
/*lint -save -e801*/
int __init nm_init(void)
{
	int ret = 0;

	ret = nm_vcom_init();
	if (ret)
		goto err_vcom;

	ret = nm_bind_pid_init();
	if (ret)
		goto err_bind_pid;

	NM_CTRL_LOGH("succ.");

	return 0;

err_bind_pid:
	nm_vcom_exit();
err_vcom:
	return ret;
}
/*lint -restore +e801*/

/*
 * nm_exit - Module exit function.
 */
STATIC void __exit nm_exit(void)
{
	nm_vcom_exit();
	nm_bind_pid_exit();
	NM_CTRL_LOGI("succ.");
}
#else
int __init nm_init(void)
{
	return 0;
}

STATIC void __exit nm_exit(void)
{
}
#endif /* FEATURE_ACPU_PHONE_MODE == FEATURE_ON */

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(nm_init);
module_exit(nm_exit);
#endif

