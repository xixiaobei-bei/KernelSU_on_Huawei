/*
 * npu_dfx_profiling.c
 *
 * about npu dfx profiling
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "npu_dfx_profiling.h"

#include <linux/err.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/barrier.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#include "npu_log.h"
#include "npu_dfx.h"
#include "npu_cache.h"
#include "npu_dfx_cq.h"
#include "npu_dfx_sq.h"
#include "npu_manager_common.h"
#include "npu_mailbox_msg.h"
#include "npu_ioctl_services.h"
#ifdef PROFILING_USE_RESERVED_MEMORY
#include <linux/io.h>
#endif
#include <securec.h>
#include "npu_platform_resource.h"

static struct prof_buff_desc *g_prof_buff = NULL;

static struct mutex g_prof_ioctrl_mutex;

static struct char_device g_prof_char_dev;

STATIC char* prof_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;

	return NULL;
}

inline u32 get_ring_buff_size(struct ring_buff_manager *ring_buff, u32 circle_len)
{
	return (ring_buff->write + circle_len - ring_buff->read ) % circle_len;
}

int move_ring_buff_to_user(struct ring_buff_manager *ring_buff, u32 circle_len, char *src_addr, char __user *dest)
{
	int tmp_len = 0;
	int ret = 0;
	COND_RETURN_ERROR(src_addr == NULL || ring_buff == NULL || dest == NULL || circle_len == 0,
		PROF_ERROR,
		"addr is null. src_addr:0x%lx, ring_buff:0x%lx, dest:0x%lx, circle_len:%u\n", src_addr, ring_buff,
		dest, circle_len);
	COND_RETURN_ERROR((ring_buff->write >= circle_len) || (ring_buff->read >= circle_len), PROF_ERROR,
		"write or read index is larger than the buffer length:0x%x. write:0x%x, read:0x%x\n",
		circle_len, ring_buff->write, ring_buff->read);

	NPU_DRV_WARN("copy_to_user, src_addr:0x%lx, dest:0x%lx, buff_len:0x%x, write:0x%x, read:0x%x\n",
		src_addr, dest, circle_len, ring_buff->write, ring_buff->read);

	if (ring_buff->read < ring_buff->write) {
		ret = copy_to_user (dest, src_addr + ring_buff->read, get_ring_buff_size(ring_buff, circle_len));
		COND_RETURN_ERROR(ret != 0, PROF_ERROR,
			"copy_to_user fail, src_addr:0x%lx, dest:0x%lx, write:0x%x, read:0x%x\n",
			src_addr, dest, ring_buff->write, ring_buff->read);
		return PROF_OK;
	}
	else if (ring_buff->read > ring_buff->write) {
		tmp_len = circle_len - ring_buff->read;
		ret = copy_to_user (dest, src_addr + ring_buff->read, tmp_len);
		COND_RETURN_ERROR(ret != 0, PROF_ERROR,
			"copy_to_user fail, src_addr:0x%lx, dest:0x%lx, write:0x%x, read:0x%x\n",
			src_addr, dest, ring_buff->write, ring_buff->read);
		ret = copy_to_user (dest + tmp_len, src_addr, ring_buff->write);
		COND_RETURN_ERROR(ret != 0, PROF_ERROR,
			"copy_to_user fail, src_addr:0x%lx, dest:0x%lx, write:0x%x, read:0x%x\n",
			src_addr, dest, ring_buff->write, ring_buff->read);
	}
	return PROF_OK;
}

static inline struct prof_buff_desc *get_prof_mem ()
{
	return g_prof_buff;
}

static char *get_prof_channel_data_addr (u32 channel)
{
	struct prof_buff_desc *prof_buff = get_prof_mem ();
	COND_RETURN_ERROR(prof_buff == NULL, NULL, "prof_buff is null\n");
	switch (channel) {
	case PROF_CHANNEL_TSCPU:
		return (char *)prof_buff->tscpu_data;
	case PROF_CHANNEL_AICPU:
		return (char *)prof_buff->aicpu_data;
	case PROF_CHANNEL_AICORE:
		return (char *)prof_buff->aicore_data;
	default:
		NPU_DRV_ERR("channel: %u error\n", channel);
	}
	return NULL;
}

static u32 get_prof_channel_data_size(u32 channel)
{
	struct prof_buff_desc *prof_buff = get_prof_mem ();
	COND_RETURN_ERROR(prof_buff == NULL, 0, "prof_buff is null\n");
	switch (channel) {
	case PROF_CHANNEL_TSCPU:
		return PROF_TSCPU_DATA_SIZE;
	case PROF_CHANNEL_AICPU:
		return PROF_AICPU_DATA_SIZE;
	case PROF_CHANNEL_AICORE:
		return PROF_AICORE_DATA_SIZE;
	default:
		NPU_DRV_ERR("channel: %u error\n", channel);
	}
	return 0;
}

int npu_prof_setting (unsigned long arg)
{
	void __user *parg = NULL;
	struct prof_setting_info *setting_info = NULL;
	struct prof_buff_desc *prof_buff = get_prof_mem ();
	COND_RETURN_ERROR(prof_buff == NULL, PROF_ERROR, "prof_buff is null\n");

	setting_info = &prof_buff->head.manager.cfg.info;

	parg = (void __user *)(uintptr_t)arg;

	COND_RETURN_ERROR(parg == NULL, PROF_ERROR, "user arg error.parg is null\n");

	if (copy_from_user(setting_info, parg, sizeof(struct prof_setting_info))) {
		NPU_DRV_ERR("copy_from_user error\n");
		return PROF_ERROR;
	}

	NPU_DRV_WARN("aicore.event_num = %u\n", setting_info->aicore.event_num);
	return PROF_OK;
}

int npu_prof_start(void)
{
	struct prof_buff_desc *prof_buff = get_prof_mem ();
	NPU_DRV_WARN("Enter.\n");
	COND_RETURN_ERROR(prof_buff == NULL, PROF_ERROR, "prof_buff is null\n");
	prof_buff->head.manager.start_flag = 1;
	return PROF_OK;
}

int npu_prof_read(unsigned long arg)
{
	void __user *parg = NULL;
	struct prof_read_config read_info = {0};
	struct ring_buff_manager tmp_ring_buff = {0};
	struct ring_buff_manager *channel_ring_buff = NULL;
	char *data_addr = NULL;
	u32 channel_data_size = 0;
	u32 ring_buff_content_size = 0;
	int ret = 0;
	COND_RETURN_ERROR(get_prof_mem () == NULL, PROF_ERROR, "prof_buff is null\n");

	parg = (void __user *)(uintptr_t)arg;

	COND_RETURN_ERROR(parg == NULL, PROF_ERROR, "user arg error.parg is null\n");


	if (copy_from_user(&read_info, parg, sizeof(struct prof_read_config))) {
		NPU_DRV_ERR("copy_from_user error\n");
		return PROF_ERROR;
	}

	COND_RETURN_ERROR(read_info.channel >= PROF_CHANNEL_MAX, PROF_ERROR,
		"prof channel number:%u is error\n", read_info.channel);

	NPU_DRV_WARN("prof channel number:%u dest_buff:0x%lx, dest_size:0x%x\n",
		read_info.channel, read_info.dest_buff, read_info.dest_size);

	data_addr = get_prof_channel_data_addr (read_info.channel);
	channel_data_size = get_prof_channel_data_size(read_info.channel);

	channel_ring_buff = &(get_prof_mem ()->head.manager.ring_buff[read_info.channel]);

	tmp_ring_buff.write = channel_ring_buff->write;
	tmp_ring_buff.read = channel_ring_buff->read;

	COND_RETURN_ERROR(channel_data_size == 0, PROF_ERROR, "channel_data_size is errer");
	ring_buff_content_size = get_ring_buff_size(&tmp_ring_buff, channel_data_size);

	if (ring_buff_content_size == 0) {
		NPU_DRV_WARN ("ring_buff is empty. channel = %u read = 0x%x write = 0x%x\n",
			read_info.channel, tmp_ring_buff.read, tmp_ring_buff.write);
		return PROF_OK;
	}
	ret = move_ring_buff_to_user (&tmp_ring_buff, channel_data_size, data_addr, read_info.dest_buff);
	COND_RETURN_ERROR(ret != PROF_OK, PROF_ERROR, "move_ring_buff_to_user fail\n");

	channel_ring_buff->read = tmp_ring_buff.write;
	read_info.src_size = ring_buff_content_size;
	NPU_DRV_WARN("move_ring_buff_to_user success. src_size:0x%x\n", read_info.src_size);

	ret = copy_to_user (parg, &read_info, sizeof(struct prof_read_config));
	COND_RETURN_ERROR (ret != PROF_OK, PROF_ERROR, "copy_to_user prof_read_config fail\n");

	return PROF_OK;
}

int npu_prof_stop(void)
{
	struct prof_buff_desc *prof_buff = NULL;
	NPU_DRV_WARN("Enter.npu_prof_stop\n");
	prof_buff = get_prof_mem ();
	COND_RETURN_ERROR(prof_buff == NULL, PROF_ERROR, "prof_buff is null\n");
	prof_buff->head.manager.start_flag = 0;
	return PROF_OK;
}

long npu_prof_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = PROF_OK;
	unsigned int prof_cmd;
	mutex_lock(&g_prof_ioctrl_mutex);

	prof_cmd = _IOC_NR(cmd);

	switch (prof_cmd) {
	case PROF_SETTING:
		ret = npu_prof_setting(arg);
		break;
	case PROF_START:
		ret = npu_prof_start();
		break;
	case PROF_STOP:
		ret = npu_prof_stop();
		break;
	case PROF_READ:
		ret = npu_prof_read(arg);
		break;
	default:
		NPU_DRV_ERR("prof ioctl cmd:%d illegal\n", prof_cmd);
		goto error;
	}

	if (ret != PROF_OK) {
		NPU_DRV_ERR("prof ioctl cmd error. prof_cmd = %d\n", prof_cmd);
		goto error;
	}
	mutex_unlock(&g_prof_ioctrl_mutex);
	return PROF_OK;
error:
	mutex_unlock(&g_prof_ioctrl_mutex);
	return PROF_ERROR;
}

int npu_prof_open(struct inode *inode, struct file *filp)
{
	NPU_DRV_WARN("Enter npu_prof_open\n");
	return PROF_OK;
}

int npu_prof_release(struct inode *inode, struct file *filp)
{
	NPU_DRV_WARN("Enter npu_prof_release\n");
	return PROF_OK;
}

const struct file_operations npu_prof_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = npu_prof_ioctl,
	.open = npu_prof_open,
	.release = npu_prof_release,
};

int npu_prof_register_cdev(void)
{
	int ret;
	unsigned int major;
	dev_t devno;
	struct char_device *priv = &g_prof_char_dev;

	priv->devno = 0;
	ret = alloc_chrdev_region(&priv->devno, 0, 1, CHAR_DRIVER_NAME);
	if (ret < 0) {
		NPU_DRV_ERR("alloc_chrdev_region error, ret = %d\n", ret);
		return PROF_ERROR;
	}

	/* init and add char device */
	major = MAJOR(priv->devno);
	devno = MKDEV(major, 0);
	cdev_init(&priv->cdev, &npu_prof_fops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops = &npu_prof_fops;

	if (cdev_add(&priv->cdev, devno, 1)) {
		NPU_DRV_ERR("cdev_add error, devno = %d\n", devno);
		unregister_chrdev_region(devno, 1);
		return PROF_ERROR;
	}

	priv->dev_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
	if (IS_ERR(priv->dev_class)) {
		NPU_DRV_ERR("class_create error, devno = %d\n", devno);
		unregister_chrdev_region(devno, 1);
		cdev_del(&priv->cdev);
		return PROF_ERROR;
	}
	if (priv->dev_class != NULL)
		priv->dev_class->devnode = prof_devnode;
	priv->device = device_create(priv->dev_class, NULL, devno, NULL, CHAR_DRIVER_NAME);

	return PROF_OK;
}

void npu_prof_free_cdev(void)
{
	struct char_device *priv = &g_prof_char_dev;

	device_destroy(priv->dev_class, priv->devno);
	class_destroy(priv->dev_class);
	unregister_chrdev_region(priv->devno, 1);
	cdev_del(&priv->cdev);
}

int prof_buff_init (void)
{
	g_prof_buff = (struct prof_buff_desc *)ioremap_wc(NPU_NONSEC_RESERVED_PROF_BASE_ADDR,
														NPU_NONSEC_RESERVED_PROF_SIZE);
	COND_RETURN_ERROR(g_prof_buff == NULL, PROF_ERROR, "prof mem remap fail,addr=0x%lx, size = 0x%x\n",
		NPU_NONSEC_RESERVED_PROF_BASE_ADDR, NPU_NONSEC_RESERVED_PROF_SIZE);
	g_prof_buff->head.manager.start_flag = 0;
	memset (g_prof_buff, 0, PROF_HEAD_MANAGER_SIZE);
	return PROF_OK;
}

int __init npu_prof_module_init(void)
{
	int ret;
	mutex_init(&g_prof_ioctrl_mutex);

	ret = prof_buff_init();
	COND_RETURN_ERROR (ret != PROF_OK, PROF_ERROR, "prof buff init\n");

	ret = npu_prof_register_cdev();
	if (ret != PROF_OK) {
		NPU_DRV_ERR("create character device fail\n");
		return PROF_ERROR;
	}

	NPU_DRV_INFO("npu_prof load ok\n");

	return PROF_OK;
}

void __exit npu_prof_module_exit(void)
{
	npu_prof_free_cdev();

	NPU_DRV_INFO("npu_prof unload ok\n");
}

module_init(npu_prof_module_init);

module_exit(npu_prof_module_exit);

MODULE_DESCRIPTION("prof driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd");
MODULE_VERSION(DRV_MODE_VERSION);
