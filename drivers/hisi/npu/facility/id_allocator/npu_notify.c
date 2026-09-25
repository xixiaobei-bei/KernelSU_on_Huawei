/*
 * npu_notify.c
 *
 * about npu notify
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
#include "npu_notify.h"

#include "linux/uaccess.h"

#include "npu_common.h"
#include "npu_log.h"
#include "npu_pm.h"
#include "devdrv_user_common.h"
#include "npu_ioctl_services.h"

static struct devdrv_notify_info *devdrv_find_one_notify_id(struct devdrv_dev_ctx *dev_ctx, int id)
{
	struct devdrv_notify_info *notify_info = NULL;

	notify_info = (struct devdrv_notify_info *)(dev_ctx->notify_addr +
		(long)sizeof(struct devdrv_notify_info) * id);
	return notify_info;
}

static u32 devdrv_get_available_notify_num(struct devdrv_dev_ctx *dev_ctx)
{
	u32 notify_num;

	spin_lock(&dev_ctx->notify_spinlock);
	notify_num = dev_ctx->notify_id_num;
	spin_unlock(&dev_ctx->notify_spinlock);

	return notify_num;
}

static struct devdrv_notify_info *devdrv_get_one_notify_id(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_notify_info *notify_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (!list_empty_careful(&dev_ctx->notify_available_list)) {
		list_for_each_safe(pos, n, &dev_ctx->notify_available_list) {
			notify_info = list_entry(pos, struct devdrv_notify_info, list);
			list_del(&notify_info->list);
			dev_ctx->notify_id_num--;
			return notify_info;
		}
	}

	return NULL;
}

int devdrv_notify_ts_msg(struct devdrv_dev_ctx *dev_ctx, int notify_id)
{
	struct devdrv_ts_notify_msg msg;
	int result;
	int ret;

	msg.header.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
	msg.header.cmd_type = DEVDRV_MAILBOX_RESET_NOTIFY_ID;
	msg.header.result = 0;
	msg.notify_id = notify_id;

	ret = devdrv_mailbox_message_send_for_res((uintptr_t)&dev_ctx->mailbox, (u8 *) &msg, sizeof(msg), &result);
	if (ret != 0 || result != 0) {
		NPU_DRV_ERR("notify alloc inform ts failed\n");
		return -EFAULT;
	};
	return 0;
}

int devdrv_free_one_notify_id(struct devdrv_dev_ctx *dev_ctx, int id, int inform_type)
{
	int ret;
	struct devdrv_notify_info *notify_info = NULL;

	if (id < 0 || id >= DEVDRV_MAX_NOTIFY_ID || dev_ctx == NULL) {
		NPU_DRV_ERR("invalid inpu argument\n");
		return -EINVAL;
	}

	spin_lock(&dev_ctx->notify_spinlock);
	notify_info = devdrv_find_one_notify_id(dev_ctx, id);
	if (notify_info == NULL) {
		spin_unlock(&dev_ctx->notify_spinlock);
		NPU_DRV_ERR("find notify id failed\n");
		return -EINVAL;
	}

	list_del(&notify_info->list);
	list_add(&notify_info->list, &dev_ctx->notify_available_list);
	dev_ctx->notify_id_num++;
	spin_unlock(&dev_ctx->notify_spinlock);

	if (inform_type == DEVDRV_NOTIFY_INFORM_TS) {
		ret = devdrv_notify_ts_msg(dev_ctx, notify_info->id);
		if (ret != 0) {
			NPU_DRV_ERR("send ts notify alloc msg failed, " "notify id = %d\n", notify_info->id);
			goto notify_ts_msg_failed;
		}
	}

	return 0;

notify_ts_msg_failed:
	spin_lock(&dev_ctx->notify_spinlock);
	list_del(&notify_info->list);   // question
	dev_ctx->notify_id_num--;
	spin_unlock(&dev_ctx->notify_spinlock);

	return ret;
}

int devdrv_notify_software_register(int dev_ctx_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if ((dev_ctx_id < 0) || (dev_ctx_id >= NPU_DEV_NUM)) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	return devdrv_dev_software_register(dev_ctx, devdrv_notify_list_init,
		devdrv_notify_list_init, devdrv_notify_list_init, devdrv_notify_list_init);
}

int devdrv_notify_list_init(u8 dev_ctx_id)
{
	u32 i;
	u32 notify_num = DEVDRV_MAX_NOTIFY_ID;
	unsigned long size;

	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_notify_info *notify_info = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	spin_lock(&dev_ctx->notify_spinlock);
	INIT_LIST_HEAD(&dev_ctx->notify_available_list);
	if (!list_empty_careful(&dev_ctx->notify_available_list)) {
		spin_unlock(&dev_ctx->notify_spinlock);
		NPU_DRV_ERR("available notify list is not empty\n");
		return -EEXIST;
	}
	spin_unlock(&dev_ctx->notify_spinlock);

	size = (long)(unsigned)sizeof(struct devdrv_notify_info) * notify_num;
	notify_info = vmalloc(size);
	if (notify_info == NULL) {
		NPU_DRV_ERR("notify_info vmalloc failed\n");
		return -ENOMEM;
	}

	spin_lock(&dev_ctx->notify_spinlock);
	dev_ctx->notify_id_num = 0;
	for (i = 0; i < notify_num; i++) {
		notify_info[i].id = i;
		notify_info[i].devid = dev_ctx->devid;
		spin_lock_init(&notify_info[i].spinlock);
		list_add_tail(&notify_info[i].list, &dev_ctx->notify_available_list);
		dev_ctx->notify_id_num++;
	}
	dev_ctx->notify_addr = notify_info;
	spin_unlock(&dev_ctx->notify_spinlock);

	return 0;
}

int devdrv_alloc_notify_id(int dev_ctx_id, unsigned long arg)
{
	u32 notify_num;
	int ret;

	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_notify_info *notify_info = NULL;
	COND_RETURN_ERROR((dev_ctx_id < 0) || (dev_ctx_id >= NPU_DEV_NUM), -EINVAL, "device id is illegal\n");

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	COND_RETURN_ERROR(dev_ctx == NULL, -ENODATA, "get device context by device id failed\n");

	mutex_lock(&dev_ctx->notify_mutex_t);
	notify_num = devdrv_get_available_notify_num(dev_ctx);
	if (notify_num == 0) {
		NPU_DRV_ERR("no available model\n");
		mutex_unlock(&dev_ctx->notify_mutex_t);
		return -ENODATA;
	}
	mutex_unlock(&dev_ctx->notify_mutex_t);

	spin_lock(&dev_ctx->notify_spinlock);
	notify_info = devdrv_get_one_notify_id(dev_ctx);
	if (notify_info == NULL) {
		spin_unlock(&dev_ctx->notify_spinlock);
		NPU_DRV_ERR("get one notify info by dev_ctx failed\n");
		return -EINVAL;
	}
	spin_unlock(&dev_ctx->notify_spinlock);

	ret = devdrv_notify_ts_msg(dev_ctx, notify_info->id);
	COND_GOTO_ERROR(ret != 0, notify_ts_msg_failed, ret, ret,
		"send ts notify alloc msg failed, notify id = %d\n", notify_info->id);

	COND_GOTO_ERROR(copy_to_user_safe((void *)(uintptr_t)arg, &notify_info->id, sizeof(int)),
		copy_to_user_safe_failed, ret, ret, "copy_to_user_safe failed\n");

	return 0;

notify_ts_msg_failed:
	COND_RETURN_ERROR(devdrv_free_one_notify_id(dev_ctx, notify_info->id, DEVDRV_NOTIFY_NOT_INFORM_TS),
		-EINVAL, "free one notify id failed, id = %d\n", notify_info->id);
	return -EFAULT;
copy_to_user_safe_failed:
	COND_RETURN_ERROR(devdrv_free_one_notify_id(dev_ctx, notify_info->id, DEVDRV_NOTIFY_INFORM_TS),
		-EINVAL, "free one notify id failed, id = %d\n", notify_info->id);
	return -EFAULT;
}

int devdrv_free_notify_id(int dev_ctx_id, unsigned long arg)
{
	int ret;
	int id;
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if ((dev_ctx_id < 0) || (dev_ctx_id >= NPU_DEV_NUM)) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n", __func__);
		return -ENODATA;
	}

	if (copy_to_user_safe(&id, (void *)(uintptr_t)arg, sizeof(int))) {
		NPU_DRV_ERR("copy_to_user_saft error\n");
		return -EFAULT;
	}

	ret = devdrv_free_one_notify_id(dev_ctx, id, DEVDRV_NOTIFY_INFORM_TS);
	if (ret != 0) {
		NPU_DRV_ERR("invalid input argument\n");
		return ret;
	}

	return 0;
}
