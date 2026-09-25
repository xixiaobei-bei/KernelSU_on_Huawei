/*
 * npu_pm.c
 *
 * about npu pm
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
#include "npu_pm.h"

#include <linux/hisi/rdr_pub.h>

#include "npu_log.h"
#include "npu_platform.h"
#include "npu_firmware.h"
#include "npu_shm.h"
#include "npu_dpm.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_heart_beat.h"

int npu_open(struct devdrv_dev_ctx *dev_ctx)
{
	int ret;
	struct devdrv_platform_info *plat_info = NULL;

	NPU_DRV_WARN("enter\n");

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("get plat_ops failed\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->open_access, 1, 0) == 0) {
		NPU_DRV_ERR("npu dev %d has already open!\n", dev_ctx->devid);
		return -EBUSY;
	}

	ret = DEVDRV_PLAT_GET_PM_OPEN(plat_info)();
	if (ret != 0) {
		atomic_inc(&dev_ctx->open_access);
		NPU_DRV_ERR("plat_ops npu_open failed\n");
		return ret;
	}

	NPU_DRV_WARN("npu dev %d open success!\n", dev_ctx->devid);
	atomic_dec(&dev_ctx->open_success);

	return 0;
}

int npu_release(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_platform_info *plat_info = NULL;

	NPU_DRV_DEBUG("enter\n");

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("get plat_ops failed\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->open_success, 0, 1) == 1) {
		NPU_DRV_ERR("npu dev %d has already release!\n", dev_ctx->devid);
		return -EBUSY;
	}

	atomic_inc(&dev_ctx->open_access);
	NPU_DRV_WARN("npu dev %d release success!\n", dev_ctx->devid);

	return 0;
}

int npu_powerup(struct devdrv_proc_ctx *proc_ctx, struct devdrv_dev_ctx *dev_ctx, u32 secure_state)
{
	int ret;
	unsigned long flags;
	u32 *ts_status = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_vma_mmapping *npu_vma_map = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	NPU_DRV_WARN("enter\n");

	plat_info = devdrv_plat_get_info();
	COND_RETURN_ERROR(plat_info == NULL, -ENODEV, "get plat_ops failed\n");
	COND_RETURN_ERROR(atomic_cmpxchg(&dev_ctx->power_access, 1, 0) == 0, 0,
		"npu dev %d has already powerup!\n", dev_ctx->devid);

	// dirty_code
	NPU_DRV_INFO("firmware is loading");
	if (plat_info->dts_info.feature_switch[DEVDRV_FEATURE_KERNEL_LOAD_IMG]) {
		ret = devdrv_load_cpu_fw();
		if (ret != 0) {
			atomic_inc(&dev_ctx->open_access);
			NPU_DRV_ERR("load cpu fw failed ret %d", ret);
			return ret;
		}
	} else {
		NPU_DRV_WARN("no need to load firmware, for npu/aicpu firmware is loaded in fastboot");
	}

	ret = DEVDRV_PLAT_GET_PM_POWER_UP(plat_info)(dev_ctx, secure_state);
	if (ret != 0) {
		atomic_inc(&dev_ctx->power_access);
		NPU_DRV_ERR("plat_ops npu_power_up failed\n");
		/* bbox : npu power up falied */
		rdr_system_error((u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL, 0, 0);
		return ret;
	}

	if ((!list_empty_careful(&proc_ctx->l2_vma_list)) && (dev_ctx->secure_state == NPU_NONSEC)) {
		list_for_each_safe(pos, n, &proc_ctx->l2_vma_list) {
			npu_vma_map = list_entry(pos, struct npu_vma_mmapping, list);
			l2_mem_swapin(npu_vma_map->vma);
		}
	} else {
		NPU_DRV_WARN("l2_vma_list is empty Or secure_state:%d is SEC. No Need to do l2_mem_swapin\n",
			dev_ctx->secure_state);
	}

	// update inuse
	dev_ctx->inuse.devid = dev_ctx->devid;
	spin_lock_irqsave(&dev_ctx->ts_spinlock, flags);
	dev_ctx->inuse.ai_core_num = DEVDRV_PLAT_GET_AICORE_MAX(plat_info);
	dev_ctx->inuse.ai_core_error_bitmap = 0;
	dev_ctx->inuse.ai_cpu_num = DEVDRV_PLAT_GET_AICPU_MAX(plat_info);
	dev_ctx->inuse.ai_cpu_error_bitmap = 0;
	spin_unlock_irqrestore(&dev_ctx->ts_spinlock, flags);
	dev_ctx->ts_work_status = 1;

	ts_status = devdrv_get_ts_work_status(dev_ctx->devid);
	if (ts_status != NULL)
		*ts_status = (u32)(secure_state == NPU_NONSEC ? DEVDRV_TS_WORK : DEVDRV_TS_SEC_WORK);
	NPU_DRV_WARN("npu dev %u powerup success!\n", dev_ctx->devid);

	if (secure_state == NPU_NONSEC) {
		 /* bbox heart beat init in non_secure mode */
		ret = devdrv_heart_beat_init(dev_ctx);
		if (ret != 0)
			NPU_DRV_ERR("devdrv_heart_beat_init failed, ret = %d\n", ret);
		ret = npu_sync_ts_time();
		if (ret != 0)
			NPU_DRV_WARN ("npu_sync_ts_time fail. ret = %d\n", ret);
	}

	atomic_dec(&dev_ctx->power_success);
	mutex_lock(&dev_ctx->npu_wake_lock_mutex);
	__pm_stay_awake(&dev_ctx->wakelock);
	mutex_unlock(&dev_ctx->npu_wake_lock_mutex);
	return 0;
}

int npu_powerdown(struct devdrv_proc_ctx *proc_ctx, struct devdrv_dev_ctx *dev_ctx)
{
	int ret;
	struct devdrv_platform_info *plat_info = NULL;
	u32 *ts_status = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_vma_mmapping *npu_vma_map = NULL;

	NPU_DRV_WARN("enter\n");
	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("get plat_ops failed\n");
		return -1;
	}

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_AUTO_POWER_DOWN) == DEVDRV_FEATURE_OFF) {
		NPU_DRV_INFO("npu auto power down switch off\n");
		return 0;
	}

	if (atomic_cmpxchg(&dev_ctx->power_success, 0, 1) == 1) {
		NPU_DRV_WARN("npu dev %d has already powerdown!\n", dev_ctx->devid);
		return 0;
	}

	if (dev_ctx->secure_state == NPU_NONSEC) {
		/* bbox heart beat exit */
		devdrv_heart_beat_exit(dev_ctx);
	}

	if ((!list_empty_careful(&proc_ctx->l2_vma_list)) && (dev_ctx->secure_state == NPU_NONSEC)) {
		list_for_each_safe(pos, n, &proc_ctx->l2_vma_list) {
			npu_vma_map = list_entry(pos, struct npu_vma_mmapping, list);
			l2_mem_swapout(npu_vma_map->vma, dev_ctx->devid);
		}
	} else {
		NPU_DRV_WARN("l2_vma_list is empty Or secure_state:%d is SEC. No Need to do l2_mem_swapout\n",
			dev_ctx->secure_state);
	}

	ret = DEVDRV_PLAT_GET_PM_POWER_DOWN(plat_info)(dev_ctx->hisi_svm, dev_ctx->secure_state, &dev_ctx->power_stage);
	if (ret != 0) {
		NPU_DRV_ERR("plat_ops npu_power_down failed\n");
		/* bbox : npu power down falied */
		rdr_system_error((u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL, 0, 0);
	} else {
		NPU_DRV_WARN("npu dev %d powerdown success!\n", dev_ctx->devid);
	}

	dev_ctx->ts_work_status = 0;
	ts_status = devdrv_get_ts_work_status(dev_ctx->devid);
	if (ts_status != NULL)
		*ts_status = DEVDRV_TS_DOWN;
	atomic_inc(&dev_ctx->power_access);

	mutex_lock(&dev_ctx->npu_wake_lock_mutex);
	__pm_relax(&dev_ctx->wakelock);
	mutex_unlock(&dev_ctx->npu_wake_lock_mutex);
	return ret;
}

int devdrv_dev_software_register(struct devdrv_dev_ctx *dev_ctx, software_ops_func init,
                                 software_ops_func available, software_ops_func recycle,
                                 software_ops_func deinit)
{
	struct devdrv_dev_res_software_ops *software_ops = NULL;

	if (dev_ctx == NULL) {
		NPU_DRV_ERR("device context is null\n");
		return -1;
	}

	software_ops = kzalloc(sizeof(struct devdrv_dev_res_software_ops), GFP_KERNEL);
	if (software_ops == NULL) {
		NPU_DRV_ERR("kzalloc software_ops failed\n");
		return -1;
	}

	software_ops->devdrv_res_software_init = init;
	software_ops->devdrv_res_software_available = available;
	software_ops->devdrv_res_software_recycle = recycle;
	software_ops->devdrv_res_software_deinit = deinit;

	spin_lock_init(&dev_ctx->resource_software_spinlock);
	INIT_LIST_HEAD(&dev_ctx->resource_software_list);
	if (!list_empty_careful(&dev_ctx->resource_software_list)) {
		NPU_DRV_ERR("res_software_list is not empty\n");
		kfree(software_ops);
		software_ops = NULL;
		return -1;
	}

	spin_lock(&dev_ctx->resource_software_spinlock);
	list_add_tail(&software_ops->list, &dev_ctx->resource_software_list);
	spin_unlock(&dev_ctx->resource_software_spinlock);

	return 0;
}


int devdrv_dev_software_unregister(struct devdrv_dev_ctx *dev_ctx)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct devdrv_dev_res_software_ops *software_ops = NULL;

	if (!list_empty_careful(&dev_ctx->resource_software_list)) {
		list_for_each_safe(pos, n, &dev_ctx->resource_software_list) {
			software_ops = list_entry(pos, struct devdrv_dev_res_software_ops, list);
			kfree(software_ops);
			list_del(pos);
		}
	}

	return 0;
}

int devdrv_npu_ctrl_core(u32 dev_id, u32 core_num)
{
	int ret;
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("get plat_ops failed\n");
		return -EINVAL;
	}

	if (dev_id > NPU_DEV_NUM) {
		NPU_DRV_ERR("invalid device id %d\n", dev_id);
		return -EINVAL;
	}

	if ((core_num == 0) || (core_num > DEVDRV_PLAT_GET_AICORE_MAX(plat_info))) {
		NPU_DRV_ERR("invalid core num %d\n", core_num);
		return -EINVAL;
	}

	if (NULL == DEVDRV_PLAT_GET_RES_CTRL_CORE(plat_info)) {
		NPU_DRV_ERR("do not support ctrl core num %d\n", core_num);
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device %d ctx fail\n", dev_id);
		return -EINVAL;
	}

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	ret = DEVDRV_PLAT_GET_RES_CTRL_CORE(plat_info)(dev_ctx, core_num);
	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	if (ret != 0) {
		NPU_DRV_ERR("ctrl device %d core num %d fail ret %d\n", dev_id, core_num, ret);
	} else {
		NPU_DRV_WARN("ctrl device %d core num %d success\n", dev_id, core_num);
	}

	return ret;
}
EXPORT_SYMBOL(devdrv_npu_ctrl_core);
