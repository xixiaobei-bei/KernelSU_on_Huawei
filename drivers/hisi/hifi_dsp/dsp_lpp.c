/*
 * dsp lpp.c
 *
 * dsp misc lpp driver.
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#include "dsp_lpp.h"

#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/gfp.h>
#include <linux/pm_wakeup.h>
#include <linux/errno.h>
#include <linux/of_address.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/suspend.h>
#include <linux/fb.h>

#include <asm/memory.h>
#include <dsm/dsm_pub.h>

#include "drv_mailbox_msg.h"
#include "bsp_drv_ipc.h"
#include "dsp_om.h"
#include "om_debug.h"
#include "om_bigdata.h"

#include "hisi_lb.h"

#define FILE_PROC_DIRECTORY "hifidsp"

struct dsp_lpp_priv {
	int wait_flag;
	char *to_user_buf;
	unsigned int to_user_buf_len;

	wait_queue_head_t pcm_read_waitq;
	wait_queue_head_t para_write_waitq;
	int pcm_read_wait_flag;
	int para_write_wait_flag;

	struct mutex proc_read_mutex;
	spinlock_t pcm_read_lock;
	spinlock_t para_write_lock;
	spinlock_t recv_proc_lock;
	wait_queue_head_t proc_waitq;
	struct wakeup_source update_buff_wakelock;
	struct wakeup_source wakelock;
	unsigned char *dsp_priv_base_virt;
	struct proc_dir_entry *entry_dsp;
	struct proc_dir_entry *entry_dsp_pcm_read;
	struct proc_dir_entry *entry_dsp_volume_meter;
};

static struct dsp_lpp_priv g_lpp_priv;

LIST_HEAD(recv_proc_work_queue_head);

void rcv_dsp_play_done_msg(struct list_head *node)
{
	logi("lock 5s\n");
	/* only mesg ID_AUDIO_AP_PLAY_DONE_IND lock 5s */
	__pm_wakeup_event(&g_lpp_priv.update_buff_wakelock, jiffies_to_msecs(5 * HZ));

	spin_lock_bh(&g_lpp_priv.recv_proc_lock);
	list_add_tail(node, &recv_proc_work_queue_head);
	g_lpp_priv.wait_flag++;
	spin_unlock_bh(&g_lpp_priv.recv_proc_lock);

	wake_up(&g_lpp_priv.proc_waitq);
}

void release_update_buff_wakelock(void)
{
	__pm_relax(&g_lpp_priv.update_buff_wakelock);
}

int soc_dsp_wakeup_read_thread(uintptr_t arg)
{
	struct recv_request *recv = NULL;
	struct misc_recmsg_param *recmsg = NULL;
	struct list_head *pos = NULL;
	unsigned int wake_cmd = (unsigned int)arg;
	unsigned int node_count = 0;

	spin_lock_bh(&g_lpp_priv.recv_proc_lock);
	list_for_each(pos, &recv_proc_work_queue_head) {
		node_count++;
	}
	spin_unlock_bh(&g_lpp_priv.recv_proc_lock);

	if (node_count >= MAX_NODE_COUNT) {
		loge("too much work left in proc work queue node count: %u\n",
			node_count);
		return -EBUSY;
	}

	recv = kmalloc(sizeof(*recv), GFP_ATOMIC);
	if (!recv) {
		loge("recv kmalloc failed\n");
		return -ENOMEM;
	}
	memset(recv, 0, sizeof(*recv));

	__pm_wakeup_event(&g_lpp_priv.wakelock, jiffies_to_msecs(HZ));

	recv->rev_msg.mail_buff_len = sizeof(*recmsg) + SIZE_CMD_ID;
	recv->rev_msg.mail_buff = kmalloc(recv->rev_msg.mail_buff_len, GFP_ATOMIC);
	if (!recv->rev_msg.mail_buff) {
		kfree(recv);
		loge("mail buff kmalloc failed\n");
		return -ENOMEM;
	}
	memset(recv->rev_msg.mail_buff, 0, recv->rev_msg.mail_buff_len);

	recmsg = (struct misc_recmsg_param *)recv->rev_msg.mail_buff;
	recmsg->msg_id = ID_AUDIO_AP_PLAY_DONE_IND;
	recmsg->play_status = (unsigned short)wake_cmd;

	spin_lock_bh(&g_lpp_priv.recv_proc_lock);
	list_add_tail(&recv->recv_node, &recv_proc_work_queue_head);
	g_lpp_priv.wait_flag++;
	spin_unlock_bh(&g_lpp_priv.recv_proc_lock);

	wake_up(&g_lpp_priv.proc_waitq);

	return OK;
}

int soc_dsp_wakeup_pcm_read_thread(uintptr_t arg)
{
	spin_lock_bh(&g_lpp_priv.pcm_read_lock);
	g_lpp_priv.pcm_read_wait_flag = true;
	spin_unlock_bh(&g_lpp_priv.pcm_read_lock);

	wake_up(&g_lpp_priv.pcm_read_waitq);

	return OK;
}

int soc_dsp_wakeup_para_write_thread(uintptr_t arg)
{
	spin_lock_bh(&g_lpp_priv.para_write_lock);
	g_lpp_priv.para_write_wait_flag = true;
	spin_unlock_bh(&g_lpp_priv.para_write_lock);

	wake_up(&g_lpp_priv.para_write_waitq);

	return OK;
}

static int try_fill_user_buf(char __user *buf, size_t count)
{
	static int retry_cnt;
	int ret;
	struct misc_recmsg_param *recmsg = NULL;
	unsigned int len = g_lpp_priv.to_user_buf_len;

	recmsg = (struct misc_recmsg_param *)g_lpp_priv.to_user_buf;
	logi("msgid: 0x%x, len: %u\n", recmsg->msg_id, len);
	logi("play status: %u, 0: done normal, 1: done complete, 2: done abnormal, 3: reset\n",
		recmsg->play_status);

	if (len > count) {
		loge("copy len: %u bigger than count: %zu\n", len, count);
		kzfree(g_lpp_priv.to_user_buf);
		g_lpp_priv.to_user_buf = NULL;
		g_lpp_priv.to_user_buf_len = 0;
		return -EINVAL;
	}

	ret = (int)try_copy_to_user(buf, g_lpp_priv.to_user_buf, len);
	if (ret != 0) {
		loge("copy to user fail, ret: %d, len: %u, retry cnt: %d\n",
			ret, len, retry_cnt);
		retry_cnt++;
		ret = -EFAULT;
	}

	if ((ret == 0) || (retry_cnt >= RETRY_COUNT)) {
		kzfree(g_lpp_priv.to_user_buf);
		g_lpp_priv.to_user_buf = NULL;
		retry_cnt = 0;
		g_lpp_priv.to_user_buf_len = 0;
	}

	return ret;
}

static int recv_msg_get(void)
{
	struct recv_request *recv = NULL;
	struct misc_recmsg_param *recmsg = NULL;
	unsigned int len;
	int ret = -EAGAIN;

	if (likely(g_lpp_priv.wait_flag > 0))
		g_lpp_priv.wait_flag--;

	if (list_empty(&recv_proc_work_queue_head)) {
		loge("recv proc list is empty\n");
		return ret;
	}

	recv = list_entry(recv_proc_work_queue_head.next,
		struct recv_request, recv_node);
	len = recv->rev_msg.mail_buff_len;
	recmsg = (struct misc_recmsg_param *)recv->rev_msg.mail_buff;

	if (unlikely(len >= MAIL_LEN_MAX || len <= SIZE_CMD_ID || !recmsg)) {
		loge("buff size invalid: %u or recmsg is null\n", len);
		ret = -EINVAL;
	} else {
		len -= SIZE_CMD_ID;
		g_lpp_priv.to_user_buf = kzalloc(len, GFP_ATOMIC);
		if (!g_lpp_priv.to_user_buf) {
			loge("kzalloc failed\n");
			return -ENOMEM;
		}
		memcpy(g_lpp_priv.to_user_buf, recv->rev_msg.mail_buff, len);
		g_lpp_priv.to_user_buf_len = len;
	}

	list_del(&recv->recv_node);
	kfree(recv->rev_msg.mail_buff);
	kfree(recv);

	return ret;
}

static ssize_t dsp_lpp_proc_read(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	int ret = -EAGAIN;

	if (!buf) {
		loge("input error: buf is NULL\n");
		return -EINVAL;
	}

	if (!is_dsp_img_loaded()) {
		loge("dsp is not loaded\n");
		return -ENXIO;
	}

	mutex_lock(&g_lpp_priv.proc_read_mutex);

	if (g_lpp_priv.to_user_buf) {
		ret = try_fill_user_buf(buf, count);
		mutex_unlock(&g_lpp_priv.proc_read_mutex);
		return ret;
	}

	if (list_empty(&recv_proc_work_queue_head)) {
		/*
		 * wait wake up function has to be called,
		 * after change the result of the wait condition
		 */
		ret = wait_event_interruptible(g_lpp_priv.proc_waitq,  /*lint !e578*/
			g_lpp_priv.wait_flag != 0);
		if (ret != 0) {
			logi("wait interrupted by a signal, ret: %d\n", ret);
			mutex_unlock(&g_lpp_priv.proc_read_mutex);
			return ret;
		}
		logi("wait event interruptible success\n");
	}

	spin_lock_bh(&g_lpp_priv.recv_proc_lock);
	ret = recv_msg_get();
	spin_unlock_bh(&g_lpp_priv.recv_proc_lock);

	if (g_lpp_priv.to_user_buf)
		ret = try_fill_user_buf(buf, count);

	mutex_unlock(&g_lpp_priv.proc_read_mutex);
	return ret;
}

static ssize_t dsp_lpp_pcm_read(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	int ret;

	if (!is_dsp_img_loaded()) {
		loge("dsp is not loaded\n");
		return -ENXIO;
	}

	if (!buf) {
		loge("param err, buf is null\n");
		return -EINVAL;
	}

	if ((count == 0) || (count > DSP_PCM_UPLOAD_BUFFER_SIZE)) {
		loge("param err, count: %zu\n", count);
		return -EINVAL;
	}

	/*
	 * wait_event_interruptible(wq, condition);
	 * The function will return -ERESTARTSYS if it was interrupted by a signal,
	 * and 0 if condition evaluated to true.
	 */
	ret = wait_event_interruptible(g_lpp_priv.pcm_read_waitq, /*lint !e578*/
		g_lpp_priv.pcm_read_wait_flag);
	if (ret != 0) {
		loge("pcm read wait interrupted, 0x%x\n", ret);
		return -EBUSY;
	}

	spin_lock_bh(&g_lpp_priv.pcm_read_lock);
	g_lpp_priv.pcm_read_wait_flag = false;
	spin_unlock_bh(&g_lpp_priv.pcm_read_lock);

	if (try_copy_to_user(buf, g_lpp_priv.dsp_priv_base_virt +
		(DSP_PCM_UPLOAD_BUFFER_ADDR - DSP_UNSEC_BASE_ADDR), count)) {
		loge("pcm read copy to user fail\n");
		return -EFAULT;
	}

	return (ssize_t)count;
}

static ssize_t dsp_lpp_wired_para_write(struct file *file, const char __user *buf,
	size_t count, loff_t *ppos)
{
	int ret;

	if (!is_dsp_img_loaded()) {
		loge("dsp is not loaded\n");
		return -ENXIO;
	}

	if (!buf) {
		loge("param err, buf is null\n");
		return -EINVAL;
	}

	if (count == 0 || count > DSP_PARA_DOWNLOAD_BUFFER_SIZE) {
		loge("param err, count = %zu\n", count);
		return -EINVAL;
	}

	ret = wait_event_interruptible(g_lpp_priv.para_write_waitq, /*lint !e578*/
		g_lpp_priv.para_write_wait_flag);
	if (ret != 0) {
		loge("para write wait interrupted, 0x%x\n", ret);
		return -EBUSY;
	}

	spin_lock_bh(&g_lpp_priv.para_write_lock);
	g_lpp_priv.para_write_wait_flag = false;
	spin_unlock_bh(&g_lpp_priv.para_write_lock);
	if (try_copy_from_user(g_lpp_priv.dsp_priv_base_virt + (HIFI_WIRED_HEADSET_PARA_ADDR -
		DSP_UNSEC_BASE_ADDR), buf, count)) {
		loge("para write copy_from_user fail\n");
		return -EFAULT;
	}

	return (ssize_t)count;
}


static const struct file_operations dsp_proc_fops = {
	.owner = THIS_MODULE,
	.read = dsp_lpp_proc_read,
};

static const struct file_operations dsp_pcm_read_fops = {
	.owner = THIS_MODULE,
	.read = dsp_lpp_pcm_read,
};

static const struct file_operations dsp_volume_meter_fops = {
	.owner = THIS_MODULE,
	.write = dsp_lpp_wired_para_write,
};

int dsp_lpp_init(unsigned char *base_virt)
{
	struct proc_dir_entry *dsp_misc_dir = NULL;

	memset(&g_lpp_priv, 0, sizeof(g_lpp_priv));

	dsp_misc_dir = proc_mkdir(FILE_PROC_DIRECTORY, NULL);
	if (!dsp_misc_dir) {
		loge("unable to create /proc/hifidsp directory\n");
		return -EINVAL;
	}

	/* Creating read/write "status" entry */
	g_lpp_priv.entry_dsp = proc_create("hifi", 0440, dsp_misc_dir, &dsp_proc_fops);
	if (!g_lpp_priv.entry_dsp) {
		loge("unable to create /proc/hifidsp/hifi entry\n");
		remove_proc_entry(FILE_PROC_DIRECTORY, NULL);
		return -EINVAL;
	}

	g_lpp_priv.entry_dsp_pcm_read = proc_create("hifi_pcm_read", 0440, dsp_misc_dir,
		&dsp_pcm_read_fops);
	if (!g_lpp_priv.entry_dsp_pcm_read) {
		/* remove a /proc entry and free it if it's not currently in use */
		proc_remove(g_lpp_priv.entry_dsp);
		remove_proc_entry(FILE_PROC_DIRECTORY, NULL);
		logi("remove /proc/hifidsp directory\n");
		return -EINVAL;
	}

	g_lpp_priv.entry_dsp_volume_meter = proc_create("hifi_volume_meter", 0440, dsp_misc_dir,
		&dsp_volume_meter_fops);
	if (!g_lpp_priv.entry_dsp_volume_meter) {
		/* remove a /proc entry and free it if it's not currently in use */
		remove_proc_entry(FILE_PROC_DIRECTORY, NULL);
		logi("remove /proc/hifidsp directory\n");
	}

	g_lpp_priv.dsp_priv_base_virt = base_virt;

	spin_lock_init(&g_lpp_priv.recv_proc_lock);
	spin_lock_init(&g_lpp_priv.pcm_read_lock);
	spin_lock_init(&g_lpp_priv.para_write_lock);

	mutex_init(&g_lpp_priv.proc_read_mutex);
	/* file signal init */
	init_waitqueue_head(&g_lpp_priv.proc_waitq);
	g_lpp_priv.wait_flag = 0;

	init_waitqueue_head(&g_lpp_priv.pcm_read_waitq);
	g_lpp_priv.pcm_read_wait_flag = false;

	init_waitqueue_head(&g_lpp_priv.para_write_waitq);
	g_lpp_priv.para_write_wait_flag = false;

	wakeup_source_init(&g_lpp_priv.wakelock, "hifi_wakelock");
	wakeup_source_init(&g_lpp_priv.update_buff_wakelock, "update_buff_wakelock");

	return 0;
}

void dsp_lpp_deinit(void)
{
	/* wake lock destroy */
	wakeup_source_trash(&g_lpp_priv.wakelock);
	wakeup_source_trash(&g_lpp_priv.update_buff_wakelock);
	mutex_destroy(&g_lpp_priv.proc_read_mutex);

	if (g_lpp_priv.entry_dsp_pcm_read)
		proc_remove(g_lpp_priv.entry_dsp_pcm_read);

	if (g_lpp_priv.entry_dsp)
		proc_remove(g_lpp_priv.entry_dsp);

	remove_proc_entry(FILE_PROC_DIRECTORY, NULL);
}

