/*
 * npu_ioctl_service.c
 *
 * about npu ioctl service
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
#include "npu_ioctl_services.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/hisi/rdr_pub.h>
#include <dsm/dsm_pub.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/sched.h>
#include <securec.h>
#include <linux/hisi/hisi_svm.h>
#include <linux/pid.h>
#include <linux/sched/task.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
#include <linux/sched/mm.h>
#endif

#include "npu_common.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "npu_event.h"
#include "npu_hwts_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_heart_beat.h"
#include "npu_adapter.h"
#include "npu_calc_sq.h"
#include "npu_comm_sqe_fmt.h"
#include "npu_sink_sqe_fmt.h"
#include "npu_pool.h"
#include "npu_iova.h"
#include "npu_svm.h"
#include "npu_doorbell.h"
#include "npu_cache.h"
#include "npu_message.h"
#include "npu_pm_framework.h"
#include "npu_rt_task.h"
#include "npu_sink_task_verify.h"

static int npu_verify_model_desc(npu_model_desc_t *model_desc);

int npu_ioctl_alloc_stream(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	npu_stream_alloc_ioctl_info_t *para =
		(npu_stream_alloc_ioctl_info_t *)((uintptr_t)arg);
	npu_stream_alloc_ioctl_info_t para_1 = {0};
	struct npu_stream_info *stream_info = NULL;

	npu_drv_debug("enter\n");
	mutex_lock(&proc_ctx->stream_mutex);

	if (copy_from_user_safe(&para_1, (void __user *)(uintptr_t)arg,
		sizeof(npu_stream_alloc_ioctl_info_t))) {
		npu_drv_err("copy from user safe error\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -EFAULT;
	}

	ret = npu_proc_alloc_stream(proc_ctx, &stream_info, para_1.strategy,
		para_1.priority);
	if (ret != 0) {
		npu_drv_err("npu_alloc_stream failed\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -ENOKEY;
	}

	if (copy_to_user_safe((void __user *)(&(para->stream_id)),
		&stream_info->id, sizeof(int))) {
		npu_drv_err("copy to user safe stream_id = %d error\n",
			stream_info->id);
		ret = npu_proc_free_stream(proc_ctx, stream_info->id);
		if (ret != 0)
			npu_drv_err("npu_ioctl_free_stream_id = %d error\n",
				stream_info->id);
		mutex_unlock(&proc_ctx->stream_mutex);
		return -EFAULT;
	}
	bitmap_set(proc_ctx->stream_bitmap, stream_info->id, 1);
	mutex_unlock(&proc_ctx->stream_mutex);

	npu_drv_debug("end\n");
	return 0;
}

int npu_proc_get_occupy_stream_id(struct npu_proc_ctx *proc_ctx,
	struct npu_occupy_stream_id *stream_id, unsigned long arg)
{
	int ret;
	struct npu_id_entity *stream_sub_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	stream_id->count = 0;

	mutex_lock(&proc_ctx->stream_mutex);
	list_for_each_safe(pos, n, &proc_ctx->sink_stream_list) {
		stream_sub_info = list_entry(pos, struct npu_id_entity, list);
		if (stream_id->count >= NPU_MAX_STREAM_ID) {
			npu_drv_err("stream_id->count: %u out of range\n",
				stream_id->count);
			mutex_unlock(&proc_ctx->stream_mutex);
			return -1;
		}
		stream_id->id[stream_id->count] = stream_sub_info->id;
		stream_id->count++;
	}

	list_for_each_safe(pos, n, &proc_ctx->stream_list) {
		stream_sub_info = list_entry(pos, struct npu_id_entity, list);
		if (stream_id->count >= NPU_MAX_STREAM_ID) {
			npu_drv_err("stream_id->count: %u out of range\n",
				stream_id->count);
			mutex_unlock(&proc_ctx->stream_mutex);
			return -1;
		}
		stream_id->id[stream_id->count] = stream_sub_info->id;
		stream_id->count++;
	}

	ret = copy_to_user_safe((void *)(uintptr_t)arg, stream_id,
		sizeof(struct npu_occupy_stream_id));
	if (ret != 0) {
		npu_drv_err("copy_to_user_safe fail\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -1;
	}

	mutex_unlock(&proc_ctx->stream_mutex);
	return 0;
}

int npu_ioctl_get_occupy_stream_id(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	struct npu_occupy_stream_id stream_id;

	npu_drv_debug("enter\n");

	stream_id.sqcq_strategy = 0;

	ret = copy_from_user_safe(&stream_id, (void *)(uintptr_t)arg,
		sizeof(struct npu_occupy_stream_id));
	cond_return_error(ret != 0, ret, "copy from user failed, ret = %d\n", ret);
	npu_drv_debug("sqcq_strategy = %d\n",  stream_id.sqcq_strategy);

	ret = npu_proc_get_occupy_stream_id(proc_ctx, &stream_id, arg);
	if (ret != 0) {
		npu_drv_err("get occupy stream id fail ret %d\n", ret);
		return ret;
	}

	return 0;
}

int npu_ioctl_alloc_event(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	u32 event_id = 0;
	u32 max_event_id = 0;
	int ret;
	u16 strategy;
	npu_event_alloc_ioctl_info_t para = {0};
	npu_event_alloc_ioctl_info_t *para_1 =
		(npu_event_alloc_ioctl_info_t *)((uintptr_t)arg);
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	mutex_lock(&proc_ctx->event_mutex);
	if (copy_from_user_safe(&para, (void __user *)(uintptr_t)arg,
		sizeof(npu_event_alloc_ioctl_info_t))) {
		npu_drv_err("copy from user safe error\n");
		mutex_unlock(&proc_ctx->event_mutex);
		return -1;
	}

	if (para.strategy >= EVENT_STRATEGY_TS) {
		npu_drv_err("proc alloc event failed, invalid input strategy: %u\n",
			para.strategy);
		mutex_unlock(&proc_ctx->event_mutex);
		return -EINVAL;
	} else if (para.strategy == EVENT_STRATEGY_SINK &&
		plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		strategy = EVENT_STRATEGY_HWTS;
		max_event_id = NPU_MAX_HWTS_EVENT_ID;
	} else {
		strategy = EVENT_STRATEGY_TS;
		max_event_id = NPU_MAX_EVENT_ID;
	}

	ret = npu_proc_alloc_event(proc_ctx, &event_id, strategy);
	if (ret != 0) {
		npu_drv_err("proc alloc event failed, event id: %u\n", event_id);
		mutex_unlock(&proc_ctx->event_mutex);
		return -1;
	}

	if (copy_to_user_safe((void __user *)(&(para_1->event_id)), &event_id,
		sizeof(int))) {
		npu_drv_err("copy to user safe event_id = %u error\n", event_id);
		if (event_id != max_event_id) {
			ret = npu_proc_free_event(proc_ctx, event_id, strategy);
			if (ret != 0) {
				npu_drv_err("proc free event id failed, event id: %u\n",
					event_id);
				mutex_unlock(&proc_ctx->event_mutex);
				return -1;
			}
			mutex_unlock(&proc_ctx->event_mutex);
			return -1;
		}
	}

	if (strategy == EVENT_STRATEGY_HWTS)
		bitmap_set(proc_ctx->hwts_event_bitmap, event_id, 1);
	else
		bitmap_set(proc_ctx->event_bitmap, event_id, 1);

	mutex_unlock(&proc_ctx->event_mutex);
	return 0;
}

int npu_ioctl_alloc_model(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	u32 model_id = 0;
	int ret;

	mutex_lock(&proc_ctx->model_mutex);
	ret = npu_proc_alloc_model(proc_ctx, &model_id);
	if (ret != 0) {
		npu_drv_err("proc alloc model failed, model id: %d\n", model_id);
		mutex_unlock(&proc_ctx->model_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void *)(uintptr_t)arg, &model_id, sizeof(int))) {
		npu_drv_err("copy to user safe model_id = %d error\n", model_id);
		if (model_id != NPU_MAX_MODEL_ID) {
			ret = npu_proc_free_model(proc_ctx, model_id);
			if (ret != 0) {
				npu_drv_err("proc free model id failed, model id: %d\n",
					model_id);
				mutex_unlock(&proc_ctx->model_mutex);
				return -EFAULT;
			}
			mutex_unlock(&proc_ctx->model_mutex);
			return -EFAULT;
		}
	}

	bitmap_set(proc_ctx->model_bitmap, model_id, 1);
	mutex_unlock(&proc_ctx->model_mutex);
	return 0;
}

int npu_ioctl_alloc_task(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	u32 task_id = 0;
	int ret;

	mutex_lock(&proc_ctx->task_mutex);
	ret = npu_proc_alloc_task(proc_ctx, &task_id);
	if (ret != 0) {
		npu_drv_err("proc alloc task failed, task id: %d\n", task_id);
		mutex_unlock(&proc_ctx->task_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void *)(uintptr_t)arg, &task_id, sizeof(int))) {
		npu_drv_err("copy to user safe task_id = %d error\n", task_id);
		if (task_id != NPU_MAX_SINK_TASK_ID) {
			ret = npu_proc_free_task(proc_ctx, task_id);
			if (ret != 0) {
				npu_drv_err("proc free task id failed, task id: %d\n",
					task_id);
				mutex_unlock(&proc_ctx->task_mutex);
				return -EFAULT;
			}
			mutex_unlock(&proc_ctx->task_mutex);
			return -EFAULT;
		}
	}

	bitmap_set(proc_ctx->task_bitmap, task_id, 1);
	mutex_unlock(&proc_ctx->task_mutex);
	return 0;
}

static int npu_check_ioctl_free_stream_para(struct npu_proc_ctx *proc_ctx,
	u32 stream_id)
{
	DECLARE_BITMAP(tmp_bitmap, NPU_MAX_STREAM_ID);

	mutex_lock(&proc_ctx->stream_mutex);
	if (stream_id >= NPU_MAX_STREAM_ID) {
		mutex_unlock(&proc_ctx->stream_mutex);
		npu_drv_err("free_stream_id %d\n", stream_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->stream_bitmap, NPU_MAX_STREAM_ID);
	bitmap_set(tmp_bitmap, stream_id, 1);
	// if tmp_bitmap == proc_ctx->stream_bitmap ,
	// bitmap_equal return 1, otherwise return 0
	if (bitmap_equal(tmp_bitmap, proc_ctx->stream_bitmap,
		NPU_MAX_STREAM_ID) == 0) {
		mutex_unlock(&proc_ctx->stream_mutex);
		npu_drv_err("invalidate id %d\n", stream_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->stream_mutex);

	return 0;
}

static int npu_check_ioctl_free_event_para(struct npu_proc_ctx *proc_ctx,
	u32 event_id, u16 input_strategy)
{
	u32 max_event_id = 0;
	u16 strategy;
	DECLARE_BITMAP(tmp_bitmap, NPU_MAX_HWTS_EVENT_ID);
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	mutex_lock(&proc_ctx->event_mutex);

	if (input_strategy >= EVENT_STRATEGY_TS) {
		npu_drv_err("strategy %u\n", input_strategy);
		mutex_unlock(&proc_ctx->event_mutex);
		return -EINVAL;
	} else if (input_strategy == EVENT_STRATEGY_SINK &&
		plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		strategy = EVENT_STRATEGY_HWTS;
		max_event_id = NPU_MAX_HWTS_EVENT_ID;
	} else {
		strategy = EVENT_STRATEGY_TS;
		max_event_id = NPU_MAX_EVENT_ID;
	}

	if (event_id >= max_event_id) {
		mutex_unlock(&proc_ctx->event_mutex);
		npu_drv_err("free_event_id %d\n", event_id);
		return -1;
	}

	if (strategy == EVENT_STRATEGY_TS) {
		bitmap_copy(tmp_bitmap, proc_ctx->event_bitmap, max_event_id);
		bitmap_set(tmp_bitmap, event_id, 1);
		if (bitmap_equal(tmp_bitmap, proc_ctx->event_bitmap,
			max_event_id) == 0) {
			mutex_unlock(&proc_ctx->event_mutex);
			npu_drv_err("invalidate id %u\n", event_id);
			return -1;
		}
	} else {
		bitmap_copy(tmp_bitmap, proc_ctx->hwts_event_bitmap, max_event_id);
		bitmap_set(tmp_bitmap, event_id, 1);
		if (bitmap_equal(tmp_bitmap, proc_ctx->hwts_event_bitmap,
			max_event_id) == 0) {
			mutex_unlock(&proc_ctx->event_mutex);
			npu_drv_err("npu_check_ioctl_free_hwts_event_para invalidate id %u\n",
				event_id);
			return -1;
		}
	}

	mutex_unlock(&proc_ctx->event_mutex);

	return 0;
}

static int npu_check_ioctl_free_model_para(struct npu_proc_ctx *proc_ctx,
	unsigned long arg, u32 *model_id)
{
	DECLARE_BITMAP(tmp_bitmap, NPU_MAX_MODEL_ID);

	mutex_lock(&proc_ctx->model_mutex);
	if (copy_from_user_safe(model_id, (void *)(uintptr_t)arg, sizeof(int))) {
		mutex_unlock(&proc_ctx->model_mutex);
		npu_drv_err("error\n");
		return -EFAULT;
	}

	if (*model_id >= NPU_MAX_MODEL_ID) {
		mutex_unlock(&proc_ctx->model_mutex);
		npu_drv_err("free_stream_id %d\n", *model_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->model_bitmap, NPU_MAX_MODEL_ID);
	bitmap_set(tmp_bitmap, *model_id, 1);
	// if tmp_bitmap == proc_ctx->model_bitmap ,
	// bitmap_equal will return 1, otherwise return 0
	if (bitmap_equal(tmp_bitmap, proc_ctx->model_bitmap,
		NPU_MAX_MODEL_ID) == 0) {
		mutex_unlock(&proc_ctx->model_mutex);
		npu_drv_err("invalidate model id %u\n", *model_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->model_mutex);

	return 0;
}

static int npu_check_ioctl_free_task_para(struct npu_proc_ctx *proc_ctx,
	unsigned long arg, u32 *task_id)
{
	DECLARE_BITMAP(tmp_bitmap, NPU_MAX_SINK_TASK_ID);

	mutex_lock(&proc_ctx->task_mutex);
	if (copy_from_user_safe(task_id, (void *)(uintptr_t)arg, sizeof(int))) {
		mutex_unlock(&proc_ctx->task_mutex);
		npu_drv_err("error\n");
		return -EFAULT;
	}

	if (*task_id >= NPU_MAX_SINK_TASK_ID) {
		mutex_unlock(&proc_ctx->task_mutex);
		npu_drv_err("free_task_id %d\n", *task_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->task_bitmap, NPU_MAX_SINK_TASK_ID);
	bitmap_set(tmp_bitmap, *task_id, 1);
	if (bitmap_equal(tmp_bitmap, proc_ctx->task_bitmap,
		NPU_MAX_SINK_TASK_ID) == 0) {
		mutex_unlock(&proc_ctx->task_mutex);
		npu_drv_err("invalidate id %d\n", *task_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->task_mutex);

	return 0;
}

static int npu_proc_ioctl_free_stream(struct npu_proc_ctx *proc_ctx,
	struct npu_stream_free_ioctl_info *para)
{
	int ret;

	npu_drv_debug("enter sqcq_strategy %d", para->sqcq_strategy);

	mutex_lock(&proc_ctx->stream_mutex);
	ret = npu_proc_free_stream(proc_ctx, para->stream_id);
	mutex_unlock(&proc_ctx->stream_mutex);
	if (ret != 0) {
		npu_drv_err("npu_ioctl_free_stream_id = %d error\n",
			para->stream_id);
		return -EINVAL;
	}

	return ret;
}

static int npu_proc_ioctl_free_event(struct npu_proc_ctx *proc_ctx,
	u32 free_event_id, u16 input_strategy)
{
	int ret;
	u16 strategy;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	mutex_lock(&proc_ctx->event_mutex);
	if (input_strategy == EVENT_STRATEGY_SINK &&
		plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1)
		strategy = EVENT_STRATEGY_HWTS;
	else
		strategy = EVENT_STRATEGY_TS;

	ret = npu_proc_free_event(proc_ctx, free_event_id, strategy);
	mutex_unlock(&proc_ctx->event_mutex);
	if (ret != 0) {
		npu_drv_err("free event id = %d error\n", free_event_id);
		ret = -EINVAL;
	}

	return ret;
}

static int npu_proc_ioctl_free_model(struct npu_proc_ctx *proc_ctx,
	u32 free_model_id)
{
	int ret;

	mutex_lock(&proc_ctx->model_mutex);
	ret = npu_proc_free_model(proc_ctx, free_model_id);
	mutex_unlock(&proc_ctx->model_mutex);
	if (ret != 0) {
		npu_drv_err("free model id = %d error\n", free_model_id);
		ret = -EINVAL;
	}

	return ret;
}

static int npu_proc_ioctl_free_task(struct npu_proc_ctx *proc_ctx,
	u32 free_task_id)
{
	int ret;

	mutex_lock(&proc_ctx->task_mutex);
	ret = npu_proc_free_task(proc_ctx, free_task_id);
	mutex_unlock(&proc_ctx->task_mutex);
	if (ret != 0) {
		npu_drv_err("free task id = %d error\n", free_task_id);
		ret = -EINVAL;
	}

	return ret;
}

int npu_ioctl_free_stream(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	struct npu_stream_free_ioctl_info para = {0};

	if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(para))) {
		npu_drv_err("npu_check_ioctl_free_stream_para error\n");
		return -EFAULT;
	}

	ret = npu_check_ioctl_free_stream_para(proc_ctx, para.stream_id);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		return -EFAULT;
	}

	return npu_proc_ioctl_free_stream(proc_ctx, &para);
}

int npu_ioctl_free_event(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	npu_event_free_ioctl_info_t para = {0};

	if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(para))) {
		npu_drv_err("copy from user safe error\n");
		return -1;
	}

	ret = npu_check_ioctl_free_event_para(proc_ctx, para.event_id,
		para.strategy);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		return -EFAULT;
	}

	return npu_proc_ioctl_free_event(proc_ctx, para.event_id, para.strategy);
}

int npu_ioctl_free_model(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	int model_id = 0;

	ret = npu_check_ioctl_free_model_para(proc_ctx, arg, &model_id);
	if (ret != 0) {
		npu_drv_err("check para fail\n");
		return -EFAULT;
	}

	return npu_proc_ioctl_free_model(proc_ctx, model_id);
}

int npu_ioctl_free_task(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	int task_id = 0;

	ret = npu_check_ioctl_free_task_para(proc_ctx, arg, &task_id);
	if (ret) {
		npu_drv_err("check para fail\n");
		return -EFAULT;
	}

	return npu_proc_ioctl_free_task(proc_ctx, task_id);
}

static int npu_ioctl_get_chip_info(u64 arg)
{
	int ret;
	struct npu_chip_info info = {0};
	struct npu_platform_info *plat_info = NULL;
	struct npu_mem_desc *l2_desc = NULL;

	npu_drv_debug("arg = 0x%llx\n", arg);

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_info failed\n");
		return -EFAULT;
	}

	l2_desc = &plat_info->dts_info.reg_desc[NPU_REG_L2BUF_BASE];
	if (l2_desc == NULL) {
		npu_drv_err("npu_plat_get_reg_desc failed\n");
		return -EFAULT;
	}

	info.l2_size = l2_desc->len + 1; // becasue of dts will minus 1

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &info, sizeof(info));
	if (ret != 0) {
		npu_drv_err("fail to copy chip_info params to user space,ret = %d\n",
			ret);
		return -EINVAL;
	}

	return ret;
}

static int hisi_svm_get_ttbr(u64 *ttbr, u64 *tcr, pid_t process_id)
{
	unsigned long asid;
	struct mm_struct *mm = NULL;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;

	if ((ttbr == NULL) || (tcr == NULL)) {
		npu_drv_err("ttbr or tcr is null\n");
		return -EINVAL;
	}

	pid_struct = find_get_pid(process_id);
	if (pid_struct == NULL) {
		npu_drv_err("pid_struct is null\n");
		return -ESRCH;
	}

	task = get_pid_task(pid_struct, PIDTYPE_PID);
	if (task == NULL) {
		put_pid(pid_struct);
		npu_drv_err("get pid task failed\n");
		return -ESRCH;
	}

	mm = get_task_mm(task);
	if (mm == NULL) {
		put_task_struct(task);
		put_pid(pid_struct);
		npu_drv_err("get mm is null\n");
		return -ESRCH;
	}

	asid = ASID(mm);
	*ttbr = virt_to_phys(mm->pgd) | (asid << 48);
	*tcr  = read_sysreg(tcr_el1);
	npu_drv_debug("pgdaddr:0x:%pK, context:0x%pK, pa:0x%pK\n",
		mm->pgd, (u64 *)(mm->pgd), (void *)(uintptr_t)virt_to_phys(mm->pgd));

	mmput(mm);
	npu_drv_debug("asid:%lu ,ttbr:0x%pK, tcr:0x%pK\n", asid,
		(void *)(uintptr_t)*ttbr, (void *)(uintptr_t)*tcr);

	put_task_struct(task);
	put_pid(pid_struct);
	return 0;
}


static int npu_ioctl_get_svm_ssid(struct npu_dev_ctx *dev_ctx, u64 arg)
{
	int ret;
	struct process_info info = {0};
	u16 ssid = 0;

	ret = copy_from_user_safe(&info, (void __user *)(uintptr_t)arg,
		sizeof(info));
	if (ret != 0) {
		npu_drv_err("fail to copy process_info params, ret = %d\n",
			ret);
		return -EINVAL;
	}

	ret = npu_get_ssid_bypid(dev_ctx->devid, current->tgid,
		info.vpid, &ssid, &info.ttbr, &info.tcr);
	if (ret != 0) {
		npu_drv_err("fail to get ssid, ret = %d\n", ret);
		return ret;
	}

	info.pasid = ssid;
	ret = hisi_svm_get_ttbr(&info.ttbr, &info.tcr, info.vpid);
	if (ret != 0) {
		npu_drv_err("fail to get process info, ret = %d\n", ret);
		return ret;
	}

	npu_drv_debug("pid=%d get ssid 0x%x ttbr 0x%llx tcr 0x%llx\n",
		current->pid, info.pasid, info.ttbr, info.tcr);

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &info, sizeof(info));
	if (ret != 0) {
		npu_drv_err("fail to copy process info params to user space,"
			"ret = %d\n", ret);
		return -EINVAL;
	}

	return ret;
}

int npu_ioctl_attach_syscache(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret = 0;
	struct npu_attach_sc msg = {0};

	ret = copy_from_user_safe(&msg, (void __user *)(uintptr_t)arg, sizeof(msg));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy npu sc attach params, ret = %d\n", ret);

	/* syscahce attach interface with offset */
	ret = npu_plat_attach_sc(msg.fd, msg.offset, (size_t)msg.size);
	cond_return_error(ret != 0, -EINVAL,
		"fail to devdrV_plat_attach_sc, ret = %d\n", ret);

	return ret;
}

int npu_ioctl_set_sc_prio(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	u32 prio = 0;

	ret = copy_from_user_safe(&prio, (void __user *)(uintptr_t)arg, sizeof(prio));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy npu sc prio params, ret = %d\n", ret);

	ret = npu_plat_set_sc_prio(prio);
	cond_return_error(ret != 0, -EINVAL,
		"fail to npu_plat_set_sc_prio, ret = %d\n", ret);

	return ret;
}

int npu_ioctl_switch_sc(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	u32 switch_sc = 0;

	ret = copy_from_user_safe(&switch_sc,
		(void __user *)(uintptr_t)arg, sizeof(switch_sc));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy npu sc enable tag params, ret = %d\n", ret);

	ret = npu_plat_switch_sc(switch_sc);
	cond_return_error(ret != 0, -EINVAL,
		"fail to npu_plat_switch_sc, ret = %d\n", ret);

	return ret;
}

int npu_ioctl_enter_workwode(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	npu_work_mode_info_t work_mode_info = {0};
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	ret = copy_from_user_safe(&work_mode_info,
		(void __user *)(uintptr_t)arg, sizeof(npu_work_mode_info_t));
	if (ret != 0) {
		npu_drv_err("fail to copy sec_mode_info params, ret = %d\n", ret);
		return -EINVAL;
	}
	cond_return_error(work_mode_info.work_mode >= NPU_WORKMODE_MAX, -EINVAL,
		"invalid work_mode = %u\n", work_mode_info.work_mode);

	dev_ctx->pm.work_mode_flags = work_mode_info.flags;

	npu_drv_debug("work mode %u flags 0x%x\n",
		work_mode_info.work_mode, work_mode_info.flags);
	ret = npu_pm_enter_workmode(proc_ctx, dev_ctx, work_mode_info.work_mode);
	if (ret != 0) {
		npu_drv_err("fail to enter workwode: %u, ret = %d\n",
			work_mode_info.work_mode, ret);
		return -EINVAL;
	}

	return ret;
}

int npu_ioctl_exit_workwode(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	uint32_t workmode;
	struct npu_power_down_info power_down_info = {{0}, 0};
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	ret = copy_from_user_safe(&power_down_info,
		(void __user *)(uintptr_t)arg, sizeof(power_down_info));
	if (ret != 0) {
		npu_drv_err("fail to copy sec_mode_info params, ret = %d\n", ret);
		return -EINVAL;
	}

	workmode = power_down_info.secure_info.work_mode;
	npu_drv_debug("workmode = %d\n", workmode);
	cond_return_error(workmode >= NPU_WORKMODE_MAX, -EINVAL,
		"invalid workmode = %u\n", workmode);

	dev_ctx->pm.work_mode_flags = 0; /* clear the flag */

	ret = npu_pm_exit_workmode(proc_ctx, dev_ctx, workmode);
	if (ret != 0) {
		npu_drv_err("fail to exit workwode: %d, ret = %d\n", workmode, ret);
		return ret;
	}
	return ret;
}

int npu_ioctl_set_limit(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	u32 value;
	struct npu_limit_time_info limit_time_info = {0, 0};
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	ret = copy_from_user_safe(&limit_time_info,
		(void __user *)(uintptr_t)arg, sizeof(limit_time_info));
	if (ret != 0) {
		npu_drv_err("fail to copy sec_mode_info params, ret = %d\n", ret);
		return -EINVAL;
	}

	value = limit_time_info.time_out;
	if (limit_time_info.type == NPU_LOW_POWER_TIMEOUT) {
		cond_return_error(value < NPU_IDLE_TIME_OUT_MIN_VALUE,
			-EINVAL, "value :%u is too small\n", value);
		dev_ctx->pm.npu_idle_time_out = value;
	} else if (limit_time_info.type == NPU_STREAM_SYNC_TIMEOUT) {
		cond_return_error(value < NPU_TASK_TIME_OUT_MIN_VALUE,
			-EINVAL, "value :%u is too small\n", value);
		cond_return_error(value > NPU_TASK_TIME_OUT_MAX_VALUE,
			-EINVAL, "value :%u is too big\n", value);
		dev_ctx->pm.npu_task_time_out = value;
	} else {
		ret = -1;
		npu_drv_err("limit type wrong!, type = %u\n", limit_time_info.type);
	}

	return ret;
}

int npu_ioctl_enable_feature(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	uint32_t feature_id;

	ret = copy_from_user_safe(&feature_id,
		(void __user *)(uintptr_t)arg, sizeof(uint32_t));
	if (ret != 0) {
		npu_drv_err("copy_from_user_safe error\n");
		return -EINVAL;
	}
	ret = npu_feature_enable(proc_ctx, feature_id, 1);

	return ret;
}

int npu_ioctl_disable_feature(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	uint32_t feature_id;

	ret = copy_from_user_safe(&feature_id,
		(void __user *)(uintptr_t)arg, sizeof(uint32_t));
	if (ret != 0) {
		npu_drv_err("copy_from_user_safe error\n");
		return -EINVAL;
	}

	ret = npu_feature_enable(proc_ctx, feature_id, 0);
	return ret;
}

int npu_ioctl_reboot(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u64 arg)
{
	unused(proc_ctx);
	unused(dev_ctx);
	unused(arg);
	return 0;
}

static int npu_verify_model_task_info(npu_model_desc_t *model_desc, u8 idx)
{
	if (model_desc->stream_tasks[idx] > NPU_MAX_SINK_TASK_ID) {
		npu_drv_err("user sink long stream_tasks[%u]= %u invalid\n",
			idx, model_desc->stream_tasks[idx]);
		return -1;
	}
	return 0;
}

static int npu_load_model(npu_model_desc_t *model_desc, u8 devid)
{
	int ret;
	int stream_idx;
	void *stream_buf_addr = NULL;
	int stream_len;
	u16 first_task_id = MAX_UINT16_NUM;

	npu_drv_debug("stream_cnt %u\n", model_desc->stream_cnt);
	cond_return_error(devid >= NPU_DEV_NUM, -1, "invalid device");
	ret = npu_verify_model_desc(model_desc);
	npu_drv_debug("stream_cnt %u\n", model_desc->stream_cnt);
	cond_return_error(ret != 0, -1,
		"npu_verify_model_desc fail, ret= %d\n", ret);

	for (stream_idx = model_desc->stream_cnt - 1; stream_idx >= 0; stream_idx--) {
		stream_len = model_desc->stream_tasks[stream_idx] * NPU_RT_TASK_SIZE;
		stream_buf_addr = vmalloc(stream_len);
		cond_return_error(stream_buf_addr == NULL, -ENOMEM,
			"vmalloc stream_buf memory size= %d failed\n", stream_len);

		ret = copy_from_user_safe(stream_buf_addr,
			(void __user *)(uintptr_t)model_desc->stream_addr[stream_idx],
			stream_len);
		if (ret != 0) {
			vfree(stream_buf_addr);
			npu_drv_err("fail to copy stream_buf, ret= %d\n", ret);
			return -EINVAL;
		}

		// verify tasks
		ret = npu_verify_rt_tasks(stream_buf_addr, stream_len);
		if (ret != 0) {
			vfree(stream_buf_addr);
			npu_drv_err("npu_verify_hwts_sqe fail, length= %d, ret= %d\n",
				stream_len, ret);
			return -1;
		}

		npu_drv_debug("stream_cnt %u, stream_idx = %d\n", model_desc->stream_cnt, stream_idx);
		ret = npu_format_sink_sqe(model_desc, stream_buf_addr, &first_task_id, devid, stream_idx);
		if (ret != 0) {
			vfree(stream_buf_addr);
			npu_drv_err("npu_verify_hwts_sqe fail, length= %d, ret= %d\n",
				stream_len, ret);
			return -1;
		}

		vfree(stream_buf_addr);
		stream_buf_addr = NULL;
	}

	return 0;
}

static int npu_verify_model_desc(npu_model_desc_t *model_desc)
{
	u8 idx;
	int ret;
	cond_return_error(model_desc == NULL, -1, "model_desc is invalid\n");

	if (model_desc->model_id >= NPU_MAX_MODEL_ID) {
		npu_drv_err("user model_id= %u invalid\n", model_desc->model_id);
		return -1;
	}
	if (model_desc->stream_cnt > NPU_MODEL_STREAM_NUM ||
		model_desc->stream_cnt == 0) {
		npu_drv_err("user stream_cnt= %u invalid\n", model_desc->stream_cnt);
		return -1;
	}

	for (idx = 0; idx < model_desc->stream_cnt; idx++) {
		if (model_desc->stream_id[idx] < NPU_MAX_NON_SINK_STREAM_ID ||
			model_desc->stream_id[idx] >= NPU_MAX_STREAM_ID) {
			npu_drv_err("user sink stream_id[%u]= %u invalid\n",
				idx, model_desc->stream_id[idx]);
			return -1;
		}

		if (model_desc->stream_addr[idx] == NULL) {
			npu_drv_err("user sink stream_addr[%u] is NULL invalid\n", idx);
			return -1;
		}

		ret = npu_verify_model_task_info(model_desc, idx);
		cond_return_error(ret != 0, -1, "model_desc is invalid\n");
	}
	return 0;
}

int npu_ioctl_load_model(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u64 arg)
{
	int ret;
	npu_model_desc_t model_desc = {0};

	npu_drv_debug("enter\n");
	ret = copy_from_user_safe(&model_desc, (void __user *)(uintptr_t)arg,
		sizeof(npu_model_desc_t));
	cond_return_error(ret != 0, -EINVAL, "fail to copy model_desc, ret= %d\n",
		ret);

	npu_drv_debug("model_id= %u, stream_cnt= %u\n",
		model_desc.model_id, model_desc.stream_cnt);

	ret = npu_load_model(&model_desc, proc_ctx->devid);
	cond_return_error(ret != 0, ret, "fail to load model, ret= %d\n",
		ret);

	npu_drv_debug("end\n");
	return 0;
}

int npu_check_ioctl_custom_para(struct npu_proc_ctx *proc_ctx,
	unsigned long arg,
	npu_custom_para_t *custom_para, struct npu_dev_ctx **dev_ctx)
{
	int ret;

	ret = copy_from_user_safe(custom_para, (void __user *)(uintptr_t)arg,
		sizeof(npu_custom_para_t));
	if (ret != 0) {
		npu_drv_err("copy_from_user_safe failed, ret = %d\n", ret);
		return -EINVAL;
	}

	if (custom_para->arg == 0) {
		npu_drv_err("invalid arg\n");
		return -EINVAL;
	}

	*dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if ((*dev_ctx) == NULL) {
		npu_drv_err("npu_proc_ioctl_custom %d of npu process %d is null\n",
			proc_ctx->devid, proc_ctx->pid);
		return -1;
	}

	return ret;
}

int npu_ioctl_send_request(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	npu_rt_task_t task = {0};
	int ret = 0;

	ret = copy_from_user_safe(&task, (void __user *)(uintptr_t)arg,
		sizeof(npu_rt_task_t));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy task, ret= %d\n", ret);

	return npu_send_request(proc_ctx, &task);
}

int npu_ioctl_receive_response(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	struct npu_receive_response_info report_info = {0};
	int ret = 0;

	npu_drv_debug("npu_ioctl_receive_report enter\n");
	ret = copy_from_user_safe(&report_info, (void __user *)(uintptr_t)arg,
		sizeof(struct npu_receive_response_info));
	if (ret != 0) {
		npu_drv_err("fail to copy comm_report, ret= %d\n", ret);
		return -EINVAL;
	}

	ret = npu_receive_response(proc_ctx, &report_info);
	if (ret != 0)
		npu_drv_err("fail to receive_response ret= %d\n", ret);

	if (copy_to_user_safe((void *)(uintptr_t)arg, &report_info,
		sizeof(report_info))) {
		npu_drv_err("ioctl_receive_response copy_to_user_safe error\n");
		return -EINVAL;
	}

	npu_drv_debug("npu_ioctl_receive_report exit\n");

	return ret;
}

static int npu_proc_ioctl_custom(struct npu_proc_ctx *proc_ctx,
	const npu_custom_para_t *custom_para, struct npu_dev_ctx *dev_ctx)
{
	int ret;

	switch (custom_para->cmd) {
	case NPU_IOC_GET_SVM_SSID:
		ret = npu_ioctl_get_svm_ssid(dev_ctx, custom_para->arg);
		break;
	case NPU_IOC_GET_CHIP_INFO:
		ret = npu_ioctl_get_chip_info(custom_para->arg);
		break;
	case NPU_IOC_REBOOT:
		ret = npu_ioctl_reboot(proc_ctx, dev_ctx, custom_para->arg);
		break;
	case NPU_IOC_LOAD_MODEL_BUFF: // for sink stream(v200)
		ret = npu_ioctl_load_model(proc_ctx, dev_ctx, custom_para->arg);
		break;
	default:
		npu_drv_err("invalid custom cmd 0x%x\n", custom_para->cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int npu_ioctl_custom(struct npu_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret;
	npu_custom_para_t custom_para = {0};
	struct npu_dev_ctx *dev_ctx = NULL;

	ret = npu_check_ioctl_custom_para(proc_ctx, arg, &custom_para, &dev_ctx);
	if (ret != 0) {
		npu_drv_err("npu_check_ioctl_custom_para failed, ret = %d\n", ret);
		return -EINVAL;
	}

	ret = npu_proc_ioctl_custom(proc_ctx, &custom_para, dev_ctx);
	if (ret != 0) {
		npu_drv_err("npu_proc_ioctl_custom failed, ret = %d\n", ret);
		return -EINVAL;
	}

	return ret;
}

/* new add for TS timeout function */
int npu_ioctl_get_ts_timeout(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	uint64_t exception_code = 0;
	int ret = 0;

	npu_drv_debug("enter\n");
	if (copy_from_user_safe(&exception_code, (void *)(uintptr_t)arg,
		sizeof(uint64_t))) {
		npu_drv_err("copy_from_user_safe error\n");
		return -EFAULT;
	}

	if (exception_code < (uint64_t)MODID_NPU_START || exception_code >
		(uint64_t)MODID_NPU_EXC_END) {
		npu_drv_err("expection code %llu out of npu range\n", exception_code);
		return -1;
	}

	if (npu_rdr_exception_is_count_exceeding(exception_code) == 0) {
		/* receive TS exception */
		npu_drv_warn("call rdr_system_error\n");
		rdr_system_error(exception_code, 0, 0);
	}

	return ret;
}

/* ION memory map */
int npu_ioctl_svm_bind_pid(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	pid_t pid;

	ret = copy_from_user_safe(&pid,
		(void __user *)(uintptr_t)arg, sizeof(pid));
	cond_return_error(ret != 0, -EINVAL, "fail to copy svm params, ret = %d\n",
		ret);

	ret = npu_insert_item_bypid(proc_ctx->devid, current->tgid, pid);
	cond_return_error(ret != 0, -EINVAL,
		"npu_insert_item_bypid fail, ret = %d\n", ret);

	return ret;
}

/* ION memory map */
int npu_ioctl_svm_unbind_pid(struct npu_proc_ctx *proc_ctx,
	unsigned long arg)
{
	int ret;
	pid_t pid;

	ret = copy_from_user_safe(&pid,
		(void __user *)(uintptr_t)arg, sizeof(pid));
	cond_return_error(ret != 0, -EINVAL, "fail to copy svm params, ret = %d\n",
		ret);

	ret = npu_release_item_bypid(proc_ctx->devid, current->tgid, pid);
	cond_return_error(ret != 0, -EINVAL,
		"npu_release_item_bypid fail, ret = %d\n", ret);

	return ret;
}

long npu_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct npu_proc_ctx *proc_ctx = NULL;
	int ret;

	proc_ctx = (struct npu_proc_ctx *)filep->private_data;
	if (proc_ctx == NULL || arg == 0) {
		npu_drv_err("invalid parameter,arg = 0x%lx,cmd = %d\n",
			arg, cmd);
		return -EINVAL;
	}

	ret = npu_proc_npu_ioctl_call(proc_ctx, cmd, arg);
	if (ret != 0) {
		npu_drv_err("process failed,arg = %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}
