/*
 * npu_model.c
 *
 * about npu model
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
#include "npu_model.h"

#include "npu_common.h"
#include "npu_log.h"
#include "npu_pm.h"
#include "devdrv_user_common.h"

static u32 devdrv_get_available_model_num(struct devdrv_dev_ctx *dev_ctx)
{
	u32 model_id_num;

	spin_lock(&dev_ctx->model_spinlock);
	model_id_num = dev_ctx->model_id_num;
	spin_unlock(&dev_ctx->model_spinlock);

	return model_id_num;
}

static struct devdrv_model_info *devdrv_get_one_model_id(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_model_info *model_info = NULL;

	if (list_empty_careful(&dev_ctx->model_available_list))
		return NULL;

	model_info = list_first_entry(&dev_ctx->model_available_list, struct devdrv_model_info, list);
	list_del(&model_info->list);
	dev_ctx->model_id_num--;
	return model_info;
}

static struct devdrv_model_info *devdrv_find_one_model(struct devdrv_dev_ctx *dev_ctx, int model_id)
{
	struct devdrv_model_info *model_info = NULL;
	model_info = (struct devdrv_model_info *)(dev_ctx->model_addr +
		(long)sizeof(struct devdrv_model_info) * model_id);

	return model_info->id != model_id ? NULL : model_info;
}

int devdrv_model_software_ops_register(u8 dev_ctx_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	return devdrv_dev_software_register(dev_ctx, devdrv_model_list_init,
		devdrv_model_list_init, devdrv_model_list_init, devdrv_model_list_init);
}

int devdrv_model_list_init(u8 dev_ctx_id)
{
	u32 i;
	u32 model_num = DEVDRV_MAX_MODEL_ID;
	unsigned long size;
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_model_info *model_info = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	INIT_LIST_HEAD(&dev_ctx->model_available_list);
	if (!list_empty_careful(&dev_ctx->model_available_list)) {
		NPU_DRV_ERR("available model list is not empty\n");
		return -EEXIST;
	}

	size = (long)(unsigned)sizeof(struct devdrv_model_info) * model_num;
	model_info = vmalloc(size);
	if (model_info == NULL) {
		NPU_DRV_ERR("model_info vmalloc failed\n");
		return -ENOMEM;
	}

	dev_ctx->model_id_num = 0;
	for (i = 0; i < model_num; i++) {
		model_info[i].id = i;
		model_info[i].devid = dev_ctx->devid;
		list_add_tail(&model_info[i].list, &dev_ctx->model_available_list);
		dev_ctx->model_id_num++;
	}
	dev_ctx->model_addr = model_info;

	return 0;
}
EXPORT_SYMBOL(devdrv_model_list_init);

struct devdrv_model_info *devdrv_alloc_model(u8 dev_ctx_id)
{
	u32 model_num;

	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_model_info *model_info = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return NULL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return NULL;
	}

	model_num = devdrv_get_available_model_num(dev_ctx);
	if (model_num == 0) {
		NPU_DRV_ERR("no available model\n");
		return NULL;
	}

	spin_lock(&dev_ctx->model_spinlock);
	model_info = devdrv_get_one_model_id(dev_ctx);
	if (model_info == NULL) {
		spin_unlock(&dev_ctx->model_spinlock);
		NPU_DRV_ERR("get one model info by dev_ctx failed\n");
		return NULL;
	}
	spin_unlock(&dev_ctx->model_spinlock);

	return model_info;
}
EXPORT_SYMBOL(devdrv_alloc_model);

int devdrv_free_model_id(u8 dev_ctx_id, int model_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_model_info *model_info = NULL;

	if ((dev_ctx_id >= NPU_DEV_NUM) || (model_id < 0) || (model_id >= DEVDRV_MAX_MODEL_ID)) {
		NPU_DRV_ERR("invalid input argument\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	spin_lock(&dev_ctx->model_spinlock);
	model_info = devdrv_find_one_model(dev_ctx, model_id);
	if (model_info == NULL) {
		spin_unlock(&dev_ctx->model_spinlock);
		NPU_DRV_ERR("can not find model by device context\n");
		return -ENODATA;
	}

	list_del(&model_info->list);
	list_add(&model_info->list, &dev_ctx->model_available_list);
	dev_ctx->model_id_num++;
	spin_unlock(&dev_ctx->model_spinlock);

	return 0;
}

int devdrv_model_list_destroy(u8 dev_ctx_id)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL || dev_ctx->model_id_num == 0) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	if (!list_empty_careful(&dev_ctx->model_available_list)) {
		list_for_each_safe(pos, n, &dev_ctx->model_available_list) {
			dev_ctx->model_id_num--;
			list_del(pos);
		}
	}
	vfree(dev_ctx->model_addr);
	dev_ctx->model_addr = NULL;

	return 0;
}
EXPORT_SYMBOL(devdrv_model_list_destroy);
