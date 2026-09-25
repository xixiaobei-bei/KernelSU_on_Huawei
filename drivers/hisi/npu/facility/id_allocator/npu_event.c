/*
 * npu_event.c
 *
 * about npu event
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
#include "npu_event.h"

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/list.h>

#include "npu_common.h"
#include "npu_log.h"
#include "npu_pm.h"
#include "devdrv_user_common.h"

static u32 devdrv_get_available_event_num(struct devdrv_dev_ctx *dev_ctx)
{
	u32 event_num;

	spin_lock(&dev_ctx->event_spinlock);
	event_num = dev_ctx->event_num;
	spin_unlock(&dev_ctx->event_spinlock);
	return event_num;
}

static struct devdrv_event_info *devdrv_get_one_event(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_event_info *event_info = NULL;

	if (dev_ctx->event_num == 0)
		return NULL;

	if (list_empty_careful(&dev_ctx->event_available_list))
		return NULL;

	event_info = list_first_entry(&dev_ctx->event_available_list, struct devdrv_event_info, list);
	list_del(&event_info->list);
	dev_ctx->event_num--;
	return event_info;
}

static struct devdrv_event_info *devdrv_find_one_event(struct devdrv_dev_ctx *dev_ctx, u32 event_id)
{
	struct devdrv_event_info *event_info = NULL;

	event_info = (struct devdrv_event_info *)(dev_ctx->event_addr +
		(long)sizeof(struct devdrv_event_info) * event_id);

	return event_info->id != event_id ? NULL : event_info;
}

int devdrv_event_software_ops_register(u8 dev_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	return devdrv_dev_software_register(dev_ctx, devdrv_event_list_init, devdrv_event_list_init,
		devdrv_event_list_init, devdrv_event_list_init);
}

int devdrv_event_list_init(u8 dev_id)
{
	u32 i;
	u32 event_num = DEVDRV_MAX_EVENT_ID;
	unsigned long size;

	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_event_info *event_info = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	INIT_LIST_HEAD(&dev_ctx->event_available_list);
	if (!list_empty_careful(&dev_ctx->event_available_list)) {
		NPU_DRV_ERR("available list not empty\n");
		return -EEXIST;
	}

	size = (long)(unsigned)sizeof(*event_info) * event_num;
	event_info = vmalloc(size);
	if (event_info == NULL)
		return -ENOMEM;

	dev_ctx->event_num = 0;
	for (i = 0; i < event_num; i++) {
		event_info[i].id = i;
		event_info[i].devid = dev_ctx->devid;
		spin_lock_init(&event_info[i].spinlock);
		list_add_tail(&event_info[i].list, &dev_ctx->event_available_list);
		dev_ctx->event_num++;
	}
	dev_ctx->event_addr = (void *)event_info;
	return 0;
}
EXPORT_SYMBOL(devdrv_event_list_init);

struct devdrv_event_info *devdrv_alloc_event(u8 dev_id)
{
	u32 event_num;

	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_event_info *event_info = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return NULL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return NULL;
	}

	event_num = devdrv_get_available_event_num(dev_ctx);
	if (event_num == 0) {
		NPU_DRV_ERR("no available event\n");
		return NULL;
	}

	spin_lock(&dev_ctx->event_spinlock);
	event_info = devdrv_get_one_event(dev_ctx);
	if (event_info == NULL) {
		spin_unlock(&dev_ctx->event_spinlock);
		NPU_DRV_ERR("get one event info by dev_ctx failed\n");
		return NULL;
	}

	spin_unlock(&dev_ctx->event_spinlock);

	return event_info;
}

int devdrv_free_event_id(u8 dev_id, u32 event_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_event_info *event_info = NULL;

	if ((dev_id >= NPU_DEV_NUM) || (event_id >= DEVDRV_MAX_EVENT_ID)) {
		NPU_DRV_ERR("invalid input argument\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	spin_lock(&dev_ctx->event_spinlock);
	event_info = devdrv_find_one_event(dev_ctx, event_id);
	if (event_info == NULL) {
		spin_unlock(&dev_ctx->event_spinlock);
		NPU_DRV_ERR("can not find event by device context and event id\n");
		return -ENODATA;
	}

	list_del(&event_info->list);
	list_add(&event_info->list, &dev_ctx->event_available_list);
	dev_ctx->event_num++;
	spin_unlock(&dev_ctx->event_spinlock);

	return 0;
}

int devdrv_event_list_destroy(u8 dev_id)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	spin_lock(&dev_ctx->event_spinlock);
	if (!list_empty_careful(&dev_ctx->event_available_list)) {
		list_for_each_safe(pos, n, &dev_ctx->event_available_list) {
			list_del(pos);
		}
	}
	spin_unlock(&dev_ctx->event_spinlock);
	vfree(dev_ctx->event_addr);
	dev_ctx->event_addr = NULL;

	return 0;
}
EXPORT_SYMBOL(devdrv_event_list_destroy);
