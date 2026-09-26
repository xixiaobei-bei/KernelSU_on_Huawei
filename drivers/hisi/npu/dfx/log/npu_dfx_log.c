/*
 * npu_dfx_log.c
 *
 * about npu dfx log
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_dfx_log.h"

#include <linux/uaccess.h>
#include <linux/list.h>
#include "npu_log.h"
#include "npu_dfx.h"
#include "npu_dfx_cq.h"
#include "npu_dfx_sq.h"
#include "npu_common.h"
#include "npu_manager.h"
#include "npu_platform.h"
#include "npu_ioctl_services.h"
#include "npu_manager_common.h"
#include "npu_mailbox_msg.h"
#include <securec.h>
#include "npu_adapter.h"

#ifndef STATIC
#define STATIC static
#endif

log_char_dev_global_info_t g_log_dev_info;

extern int npu_get_devids(unsigned int *devices);

int g_flag_test;

int log_get_channel_index(int channel_id, unsigned int *channel_idx)
{
	int ret = 0;

	if (channel_id == LOG_CHANNEL_TS_ID)
		*channel_idx = LOG_CHANNEL_TS_IDX;
	else if (channel_id == LOG_CHANNEL_AICPU_ID)
		*channel_idx = LOG_CHANNEL_AICPU_IDX;
	else
		ret = -1;

	return ret;
}

STATIC char *log_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

static int log_get_channel_share_memory(struct log_channel_info_t *channel_info)
{
	struct npu_dfx_desc *dfx_desc = NULL;
	struct npu_platform_info *plat_info = NULL;
	unsigned int can_use_size;

	if ((channel_info->channel_id != LOG_CHANNEL_TS_ID) &&
		(channel_info->channel_id != LOG_CHANNEL_AICPU_ID)) {
		npu_drv_err("channel_id(%u) is invalid\n", channel_info->channel_id);
		return IDE_DRV_ERROR;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return IDE_DRV_ERROR;
	}

	dfx_desc = &plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_LOG];
	channel_info->phy_addr = dfx_desc->bufs->base;
	// a half memory use ts, other half memory use ai core
	can_use_size = (dfx_desc->bufs->len) >> 1;
	if (channel_info->channel_id == LOG_CHANNEL_AICPU_ID)
		channel_info->phy_addr += can_use_size;

	channel_info->buf_size = can_use_size;
	channel_info->vir_addr = (unsigned char *)ioremap_wc(
		channel_info->phy_addr, can_use_size);
	if (channel_info->vir_addr == NULL) {
		npu_drv_err("ioremap_nocache failed\n");
		return IDE_DRV_ERROR;
	}

	npu_drv_debug("phy_addr = %pK, vir_addr = %pK, buf_size : %x\n",
		(void *)(uintptr_t)channel_info->phy_addr,
		(void *)(uintptr_t)channel_info->vir_addr,
		channel_info->buf_size);

	return IDE_DRV_OK;
}

int log_channel_init(void)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;
	log_buf_ptr_t *buf_ptr = NULL;
	int ret = 0;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx++) {
		channel_info =
			(log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

		mutex_lock(&channel_info->cmd_mutex);
		if (channel_idx == LOG_CHANNEL_TS_IDX)
			channel_info->channel_id = LOG_CHANNEL_TS_ID;
		else if (channel_idx == LOG_CHANNEL_AICPU_IDX)
			channel_info->channel_id = LOG_CHANNEL_AICPU_ID;

		if (channel_info->vir_addr == NULL) {
			ret = log_get_channel_share_memory(channel_info);
			if (ret != IDE_DRV_OK) {
				npu_drv_err("get shm_memory failed!\n");
				mutex_unlock(&channel_info->cmd_mutex);
				return IDE_DRV_ERROR;
			}
		}

		channel_info->channel_state = LOG_CHANNEL_ENABLE;
		channel_info->poll_flag = POLL_INVALID;
		channel_info->channel_type = 0;
		buf_ptr = (log_buf_ptr_t *)(channel_info->vir_addr);
		buf_ptr->buf_read  = 0;
		buf_ptr->buf_write = 0;
		buf_ptr->buf_len = channel_info->buf_size;
		buf_ptr->log_level = LOG_LEVEL_DEFAULT;

		mutex_unlock(&channel_info->cmd_mutex);
	}

	return IDE_DRV_OK;
}

void log_channel_release(void)
{
	unsigned int channel_idx;
	log_channel_info_t *channel_info = NULL;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx++) {
		channel_info =
			(log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

		mutex_lock(&channel_info->cmd_mutex);
		channel_info->channel_state = LOG_CHANNEL_DISABLE;
		if (channel_info->vir_addr != NULL) {
			iounmap((void *)channel_info->vir_addr);
			channel_info->vir_addr = NULL;
		}
		mutex_unlock(&channel_info->cmd_mutex);
	}
}

int log_set(log_ioctl_para_t *log_para)
{
	log_channel_info_t *channel_info = NULL;
	int ret;
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_buf_ptr_t *buf_ptr = NULL;

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		npu_drv_err("para invalid\n");
		return IDE_DRV_ERROR;
	}

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		npu_drv_err("log_get_channel_index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info =
		(log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
		npu_drv_err("Channel:%d not create\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	if (channel_info->vir_addr == NULL) {
		npu_drv_err("Channel:%d vir addr error\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	buf_ptr = (log_buf_ptr_t *)(channel_info->vir_addr);
	buf_ptr->log_level = log_para->attrib.log_level;

	(void)memcpy_s((void *)&channel_info->ioctl_para.attrib,
		sizeof(attribute_t), (void *)&log_para->attrib,
		sizeof(attribute_t));

	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;
}

int log_read_ringbuffer(log_ioctl_para_t *log_para, unsigned int buf_read,
	unsigned int buf_write, log_channel_info_t *channel_info)
{
	unsigned char *base = NULL;
	unsigned int buf_len = 0;
	int size;
	log_buf_ptr_t *buf_ptr = NULL;
	char *out_buf = log_para->out_buf;

	cond_return_error((out_buf == NULL) || (channel_info->vir_addr == NULL),
		IDE_DRV_ERROR, "para->out_buf is null\n");

	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;

	if (buf_write > buf_read) {
		buf_len = (buf_write - buf_read) -
			((buf_write - buf_read) % LOG_MEMORY_ALIGN);
		base = channel_info->vir_addr + DATA_BUF_HEAD + buf_read;
		cond_goto_debug(buf_len == 0, fail, size, 0);

		if (copy_to_user_safe((void __user *)out_buf,
			(void *)base, buf_len) != 0) {
			npu_drv_warn("copy log fail\n");
			return IDE_DRV_ERROR;
		}

		atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
		log_para->buf_size = buf_len;
		log_para->ret = IDE_DRV_OK;
		return IDE_DRV_OK;
	}

	if (buf_read < (channel_info->buf_size - DATA_BUF_HEAD)) {
		buf_len = (channel_info->buf_size - DATA_BUF_HEAD - buf_read) -
			((channel_info->buf_size - DATA_BUF_HEAD - buf_read) %
			LOG_MEMORY_ALIGN);
		base = channel_info->vir_addr + DATA_BUF_HEAD + buf_read;

		cond_goto_debug(buf_len == 0, fail, size, 0);

		if (copy_to_user_safe((void __user *)out_buf,
			(void *)base, buf_len) != 0) {
			npu_drv_warn("copy log fail\n");
			return IDE_DRV_ERROR;
		}

		cond_goto_debug(buf_write == 0, fail_atomic_set, size, buf_len);
	}

	/* notice user buffer's offset;
	 * copy_to_user_safe() is defined in device manager ko
	 */
	base = channel_info->vir_addr + DATA_BUF_HEAD;
	if (copy_to_user_safe((void *)(out_buf + buf_len),
		(void *)base, buf_write) != 0) {
		npu_drv_warn("copy log fail\n");
		return IDE_DRV_ERROR;
	}

	atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
	log_para->buf_size = buf_len + buf_write;
	log_para->ret = IDE_DRV_OK;
	return IDE_DRV_OK;
fail_atomic_set:
	atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
fail:
	log_para->buf_size = size;
	log_para->ret = IDE_DRV_OK;
	return IDE_DRV_OK;
}

int log_read(log_ioctl_para_t *log_para)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	unsigned int buf_read;
	unsigned int buf_write;
	int channel_id;
	int ret = log_check_ioctl_para(log_para);
	log_buf_ptr_t *buf_ptr = NULL;
	log_channel_info_t *channel_info = NULL;

	cond_return_error(ret != IDE_DRV_OK, IDE_DRV_ERROR, "para invalid\n");

	channel_id = log_para->channel_id;
	log_para->ret = IDE_DRV_ERROR;

	cond_return_error(
		log_get_channel_index(log_para->channel_id, &channel_idx),
		IDE_DRV_ERROR, "log_get_channel_index fail. channel_id: %u\n",
		channel_id);

	channel_info = (
		log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	cond_goto_error(channel_info->channel_state != LOG_CHANNEL_ENABLE,
		fail_free, ret, IDE_DRV_ERROR,
		"Channel:%d not create\n", log_para->channel_id);

	cond_goto_error(channel_info->vir_addr == NULL, fail_free, ret,
		IDE_DRV_ERROR, "Channel:%d vir_addr is null\n", log_para->channel_id);

	log_para->buf_size = 0;
	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;
	buf_read = buf_ptr->buf_read;
	buf_write = (unsigned int)atomic_read((atomic_t *)&buf_ptr->buf_write);
	/* print_num is set to control log's print for every channel; and always ret; */
	if (buf_write > (channel_info->buf_size - DATA_BUF_HEAD) ||
		buf_read > (channel_info->buf_size - DATA_BUF_HEAD)) {
		if (channel_info->print_num < LOG_PRINT_MAX) {
			npu_drv_err("log's buffer is error. channel_id = %d, len = %lu, read = %u, write = %u\n",
				channel_id, (channel_info->buf_size - DATA_BUF_HEAD), buf_read,
				buf_write);
			channel_info->print_num++;
			}
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	cond_goto_nolog(buf_write == buf_read, fail_setlog_free, ret, IDE_DRV_OK);

	ret = log_read_ringbuffer(log_para, buf_read, buf_write, channel_info);
	if (ret != IDE_DRV_OK) {
		npu_drv_info("log_read_ringbuffer error, ret = %d, channel_id = %d, buf_read = %u, buf_write = %u\n",
			ret, channel_id, buf_read, buf_write);
		mutex_unlock(&channel_info->cmd_mutex);
		return ret;
	}

	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;
fail_setlog_free:
	log_para->ret = IDE_DRV_OK;
fail_free:
	mutex_unlock(&channel_info->cmd_mutex);
	return ret;
}

int log_check_ioctl_para(const struct log_ioctl_para_t *log_para)
{
	struct npu_platform_info *plat_info = NULL;
	struct npu_dfx_desc *dfx_desc = NULL;
	int i;

	if (log_para == NULL) {
		npu_drv_err("NULL para\n");
		return IDE_DRV_ERROR;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info fail\n");
		return IDE_DRV_ERROR;
	}

	dfx_desc = &plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_LOG];
	if (dfx_desc->channel_num > NPU_DFX_CHANNEL_MAX_RESOURCE) {
		npu_drv_err("channel_num:%d cofig error\n", dfx_desc->channel_num);
		return IDE_DRV_ERROR;
	}

	for (i = 0; i < dfx_desc->channel_num; i++) {
		if (dfx_desc->channels[i] == log_para->channel_id)
			break;
	}

	if (i == dfx_desc->channel_num) {
		npu_drv_err("channel_id:%d create invalid\n", log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	return 0;
}

long log_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct log_ioctl_para_t log_para = {0};
	unsigned int drv_cmd;
	void __user *parg = NULL;
	int ret = 0;

	drv_cmd = _IOC_NR(cmd);
	parg = (void __user *)(uintptr_t)arg;
	if (parg == NULL) {
		npu_drv_err("user arg error.parg is null\n");
		return IDE_DRV_ERROR;
	}

	if (copy_from_user(&log_para, parg, sizeof(log_para))) {
		npu_drv_err("copy_from_user error\n");
		return IDE_DRV_ERROR;
	}

	switch (drv_cmd) {
	case LOG_SET:
		ret = log_set((log_ioctl_para_t *)&log_para);
		break;
	case LOG_READ:
		ret = log_read((log_ioctl_para_t *)&log_para);
		break;
	default:
		npu_drv_err("log ioctl cmd:%d illegal\n", drv_cmd);
		return IDE_DRV_ERROR;
	}

	if (copy_to_user_safe(parg, (void *)&log_para, sizeof(log_para)) != 0) {
		npu_drv_err("copy_to_user error\n");
		return IDE_DRV_ERROR;
	}

	return ret;
}

int log_drv_release(struct inode *inode, struct file *filp)
{
	if (g_flag_test-- > 0)
		return IDE_DRV_OK;

	return IDE_DRV_OK;
}

int log_drv_init(unsigned int dev_id)
{
	int channel_idx;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	log_dev_info->device_state = DEV_UNUSED;
	log_dev_info->cmd_verify = 1;
	log_dev_info->device_id = dev_id;
	log_dev_info->poll_head = 0;
	log_dev_info->poll_tail = 0;

	g_log_dev_info.poll_box = (log_poll_info_t *)kzalloc(
		sizeof(log_poll_info_t) * LOG_POLL_DEPTH, GFP_KERNEL);
	if (g_log_dev_info.poll_box == NULL) {
		npu_drv_err("pollbox kzalloc is error\n");
		return IDE_DRV_ERROR;
	}

	(void)memset_s(g_log_dev_info.log_channel,
		sizeof(log_channel_info_t) * LOG_CHANNEL_NUM,
		0, sizeof(log_channel_info_t) * LOG_CHANNEL_NUM);

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx++)
		mutex_init(&g_log_dev_info.log_channel[channel_idx].cmd_mutex);
	g_log_dev_info.op_cb_idx = NPU_CALLBACK_REG_NUM;
	g_log_dev_info.rl_cb_idx = NPU_CALLBACK_REG_NUM;

	return IDE_DRV_OK;
}

void log_drv_uninit(void)
{
	(void)memset_s(g_log_dev_info.log_channel,
		sizeof(log_channel_info_t) * LOG_CHANNEL_NUM,
		0, sizeof(log_channel_info_t) * LOG_CHANNEL_NUM);

	if (g_log_dev_info.poll_box != NULL) {
		kfree((void *)g_log_dev_info.poll_box);
		g_log_dev_info.poll_box = NULL;
	}
}

const struct file_operations log_drv_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = log_drv_ioctl,
	.release = log_drv_release,
};

int log_drv_register_cdev(void)
{
	int ret;
	unsigned int major;
	dev_t devno;

	struct char_device *priv =
		(struct char_device *)&g_log_dev_info.char_dev_log;

	priv->devno = 0;
	ret = alloc_chrdev_region(&priv->devno, 0, 1, CHAR_DRIVER_NAME);
	if (ret < 0)
		return IDE_DRV_ERROR;

	/* init and add char device */
	major = MAJOR(priv->devno);
	devno = MKDEV(major, 0);
	cdev_init(&priv->cdev, &log_drv_fops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops = &log_drv_fops;

	if (cdev_add(&priv->cdev, devno, 1)) {
		unregister_chrdev_region(devno, 1);
		return IDE_DRV_ERROR;
	}

	priv->dev_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
	if (IS_ERR(priv->dev_class)) {
		(void)unregister_chrdev_region(devno, 1);
		(void)cdev_del(&priv->cdev);
		return IDE_DRV_ERROR;
	}
	if (priv->dev_class != NULL)
		priv->dev_class->devnode = log_devnode;

	g_log_dev_info.log_dev = device_create(priv->dev_class, NULL, devno,
		NULL, CHAR_DRIVER_NAME);
	if (g_log_dev_info.log_dev == NULL) {
		npu_drv_err("device_create error.devno = %d\n", devno);
		(void)class_destroy(priv->dev_class);
		(void)unregister_chrdev_region(devno, 1);
		(void)cdev_del(&priv->cdev);
		return IDE_DRV_ERROR;
	}

	return IDE_DRV_OK;
}

void log_drv_free_cdev(void)
{
	struct char_device *priv = &(g_log_dev_info.char_dev_log);
	(void)device_destroy(priv->dev_class, priv->devno);
	(void)class_destroy(priv->dev_class);
	(void)unregister_chrdev_region(priv->devno, 1);
	(void)cdev_del(&priv->cdev);
}

int __init log_drv_module_init(void)
{
	int ret = IDE_DRV_OK;
	const unsigned int dev_id = 0;

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return -1;

	(void)memset_s(&g_log_dev_info, sizeof(log_char_dev_global_info_t), 0,
		sizeof(log_char_dev_global_info_t));

	ret = log_drv_register_cdev();
	if (ret != IDE_DRV_OK) {
		npu_drv_err("create character device fail\n");
		return IDE_DRV_ERROR;
	}

	ret = log_drv_init(dev_id);
	if (ret != IDE_DRV_OK) {
		log_drv_free_cdev();
		npu_drv_err("log drv init fail\n");
		return IDE_DRV_ERROR;
	}

	ret = log_channel_init();
	if (ret != IDE_DRV_OK) {
		log_drv_uninit();
		log_drv_free_cdev();
		npu_drv_err("log drv init fail\n");
		return IDE_DRV_ERROR;
	}

	npu_drv_info("device (%u) log drv load ok\n", dev_id);

	return IDE_DRV_OK;
}

void __exit log_drv_module_exit(void)
{
	if (npu_plat_bypass_status() == NPU_BYPASS)
		return;

	log_channel_release();

	log_drv_uninit();

	/* char device free at last */
	log_drv_free_cdev();

	npu_drv_info("device (%u) log drv exit\n",
		g_log_dev_info.log_dev_info.device_id);
}

module_init(log_drv_module_init);

module_exit(log_drv_module_exit);

MODULE_DESCRIPTION("log driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(LOG_DRV_MODE_VERSION);
