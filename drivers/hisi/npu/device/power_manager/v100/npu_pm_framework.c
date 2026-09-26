/*
 * npu_pm_framework.c
 *
 * about npu pm
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
#include "npu_pm_framework.h"

#include <linux/hisi/rdr_pub.h>
#include <linux/timer.h>
#include <dsm/dsm_pub.h>

#include "npu_log.h"
#include "npu_platform.h"
#include "npu_shm.h"
#include "npu_dpm.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_heart_beat.h"
#include "npu_adapter.h"

static int npu_pm_check_workmode(u32 curr_work_mode, u32 workmode)
{
	if (workmode >= NPU_ISPNN_SEPARATED)
		return -1;
	if (bitmap_get(curr_work_mode, NPU_NONSEC) && (workmode == NPU_SEC))
		return -1;
	if (bitmap_get(curr_work_mode, NPU_SEC) && (workmode == NPU_NONSEC))
		return -1;

	if (bitmap_get(curr_work_mode, workmode) != 0)
		return 1;

	return 0;
}

int npu_pm_enter_workmode(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 workmode)
{
	int ret;
	u32 curr_work_mode;

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	curr_work_mode = dev_ctx->pm.work_mode;
	npu_drv_warn("curr_work_mode_set = 0x%x secure_mode = 0x%x \n",
		curr_work_mode, workmode);

	ret = npu_pm_check_workmode(curr_work_mode, workmode);
	if (ret != 0) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		cond_return_error(ret < 0, ret, "not support workmode, curr_work_mode_set = 0x%x work_mode = 0x%x\n",
			curr_work_mode, workmode);
		npu_drv_info("workmode has already set, workmode %u\n", workmode);
		return 0;
	}

	dev_ctx->pm.work_mode = (1 << workmode);
	ret = npu_powerup(proc_ctx, dev_ctx, workmode);
	if (ret) {
		dev_ctx->pm.work_mode = curr_work_mode;
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		npu_drv_err("npu powerup failed\n");

		if (!dsm_client_ocuppy(npu_dsm_client)) {
			dsm_client_record(npu_dsm_client, "npu power up failed\n");
			dsm_client_notify(npu_dsm_client, DSM_AI_KERN_POWER_UP_ERR_NO);
			npu_drv_err("[I/DSM] %s dmd report\n",
				npu_dsm_client->client_name);
		}
		return ret;
	}

	if (workmode == NPU_SEC) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		npu_drv_warn("secure or ispnn npu power up,no need send mbx to tscpu,return directly\n");
		return 0;
	}
	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	mutex_lock(&proc_ctx->stream_mutex);
	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	if (dev_ctx->power_stage == NPU_PM_UP) {
		ret = npu_proc_send_alloc_stream_mailbox(proc_ctx);
		if (ret) {
			mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
			mutex_unlock(&proc_ctx->stream_mutex);
			npu_drv_err("npu send stream mailbox failed\n");
			return ret;
		}
	} else {
		npu_drv_warn("alloc stream no need to inform ts, power_stage %d, work_mode = %d\n",
			dev_ctx->power_stage, dev_ctx->pm.work_mode);
	}
	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
	mutex_unlock(&proc_ctx->stream_mutex);

	npu_rdr_exception_init();

	npu_drv_warn("npu dev %u powerup successfully!\n", dev_ctx->devid);
	return ret;
}

int npu_pm_exit_workmode(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 workmode)
{
	int ret;
	u32 curr_work_mode;

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	curr_work_mode = dev_ctx->pm.work_mode;
	npu_drv_warn("cur_secure_mode = 0x%x, workmode = 0x%x\n",
		curr_work_mode, workmode);
	ret = npu_pm_check_workmode(curr_work_mode, workmode);
	if (ret <= 0) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		npu_drv_err("not support workmode, curr_work_mode_set = 0x%x work_mode = 0x%x\n",
			curr_work_mode, workmode);
		return ret;
	}

	ret = npu_powerdown(proc_ctx, dev_ctx);
	if (ret != 0) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		npu_drv_err("npu powerdown failed\n");
		return ret;
	}

	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	return ret;
}

int npu_powerup(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret;
	unsigned long flags;
	u32 *ts_status = NULL;
	struct npu_platform_info *plat_info = NULL;

	npu_drv_warn("enter\n");

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -ENODEV, "get plat_ops failed\n");
	cond_return_error(
		atomic_cmpxchg(&dev_ctx->power_access, 1, 0) == 0, 0,
		"npu dev %d has already powerup!\n", dev_ctx->devid);

	ret = plat_info->adapter.pm_ops.npu_power_up(dev_ctx, work_mode);
	if (ret != 0) {
		atomic_inc(&dev_ctx->power_access);
		npu_drv_err("plat_ops npu_power_up failed\n");
		/* bbox : npu power up falied */
		if (npu_rdr_exception_is_count_exceeding(
				(u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL) == 0)
			rdr_system_error((u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL, 0, 0);
		return ret;
	}

	npu_proc_clear_sqcq_info(proc_ctx);

	// update inuse
	dev_ctx->inuse.devid = dev_ctx->devid;
	spin_lock_irqsave(&dev_ctx->ts_spinlock, flags);
	dev_ctx->inuse.ai_core_num = plat_info->spec.aicore_max;
	dev_ctx->inuse.ai_core_error_bitmap = 0;
	dev_ctx->inuse.ai_cpu_num = plat_info->spec.aicpu_max;
	dev_ctx->inuse.ai_cpu_error_bitmap = 0;
	spin_unlock_irqrestore(&dev_ctx->ts_spinlock, flags);
	dev_ctx->ts_work_status = 1;

	ts_status = npu_get_ts_work_status(dev_ctx->devid);
	if (ts_status != NULL)
		*ts_status = (u32)(work_mode != NPU_SEC ? NPU_TS_WORK :
			NPU_TS_SEC_WORK);
	npu_drv_warn("npu dev %u hardware powerup successfully!\n", dev_ctx->devid);

	if (work_mode != NPU_SEC) {
		 /* bbox heart beat init in non_secure mode */
		ret = npu_heart_beat_init(dev_ctx);
		if (ret != 0)
			npu_drv_err("npu_heart_beat_init failed, ret = %d\n", ret);

		ret = npu_sync_ts_time();
		if (ret != 0)
			npu_drv_warn("npu_sync_ts_time fail. ret = %d\n", ret);
	}

	atomic_dec(&dev_ctx->power_success);
	__pm_stay_awake(&dev_ctx->wakeup);
	return 0;
}

int npu_powerdown(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;
	u32 *ts_status = NULL;
	u32 is_secure = 0;

	npu_drv_warn("enter\n");
	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_ops failed\n");
		return -1;
	}

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_AUTO_POWER_DOWN] == NPU_FEATURE_OFF) {
		npu_drv_info("npu auto power down switch off\n");
		dev_ctx->pm.work_mode = 0;
		return 0;
	}

	if (atomic_cmpxchg(&dev_ctx->power_success, 0, 1) == 1) {
		npu_drv_warn("npu dev %d has already powerdown!\n", dev_ctx->devid);
		dev_ctx->pm.work_mode = 0;
		return 0;
	}

	if (!bitmap_get(dev_ctx->pm.work_mode, NPU_SEC)) {
		/* bbox heart beat exit */
		npu_heart_beat_exit(dev_ctx);
	}

	if (bitmap_get(dev_ctx->pm.work_mode, NPU_SEC))
		is_secure = NPU_SEC;

	ret = plat_info->adapter.pm_ops.npu_power_down(dev_ctx->devid,
		is_secure, &dev_ctx->power_stage);
	if (ret != 0) {
		npu_drv_err("plat_ops npu_power_down failed\n");
		/* bbox : npu power down falied */
		if (npu_rdr_exception_is_count_exceeding(
			(u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL) == 0)
			rdr_system_error((u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL, 0, 0);
	} else {
		npu_drv_warn("npu dev %d powerdown success!\n", dev_ctx->devid);
	}
	npu_proc_clear_sqcq_info(proc_ctx);

	dev_ctx->ts_work_status = 0;
	ts_status = npu_get_ts_work_status(dev_ctx->devid);
	if (ts_status != NULL)
		*ts_status = NPU_TS_DOWN;
	dev_ctx->pm.work_mode = 0;
	atomic_inc(&dev_ctx->power_access);
	__pm_relax(&dev_ctx->wakeup);

	return ret;
}

int npu_ctrl_core(u32 dev_id, u32 core_num)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_ops failed\n");
		return -EINVAL;
	}

	if (dev_id > NPU_DEV_NUM) {
		npu_drv_err("invalid device id %d\n", dev_id);
		return -EINVAL;
	}

	if ((core_num == 0) || (core_num > plat_info->spec.aicore_max)) {
		npu_drv_err("invalid core num %d\n", core_num);
		return -EINVAL;
	}

	if (NULL == plat_info->adapter.res_ops.npu_ctrl_core) {
		npu_drv_err("do not support ctrl core num %d\n", core_num);
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		npu_drv_err("get device %d ctx fail\n", dev_id);
		return -EINVAL;
	}

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	ret = plat_info->adapter.res_ops.npu_ctrl_core(dev_ctx, core_num);
	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	if (ret != 0)
		npu_drv_err("ctrl device %d core num %d fail ret %d\n", dev_id,
			core_num, ret);
	else
		npu_drv_warn("ctrl device %d core num %d success\n", dev_id, core_num);

	return ret;
}
EXPORT_SYMBOL(npu_ctrl_core);

int npu_pm_reboot(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx)
{
	return 0;
}

void npu_pm_resource_init(struct npu_proc_ctx *proc_ctx,
	struct npu_power_manage *handle)
{
}

void npu_pm_delete_idle_timer(struct npu_dev_ctx *dev_ctx)
{
}

void npu_pm_add_idle_timer(struct npu_dev_ctx *dev_ctx)
{
}

int npu_interframe_enable(struct npu_proc_ctx *proc_ctx, uint32_t enable)
{
	return 0;
}

void npu_pm_adapt_init(struct npu_dev_ctx *dev_ctx)
{
}

int npu_pm_exception_powerdown(struct npu_proc_ctx *proc_ctx, struct npu_dev_ctx *dev_ctx)
{
	return 0;
}
