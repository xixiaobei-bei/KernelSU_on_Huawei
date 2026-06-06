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

#include <linux/hisi/hisi_svm.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
#include <linux/sched/mm.h>
#endif
#include "npu_ioctl_services.h"
#include "npu_proc_ctx.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "drv_log.h"
#include "npu_mailbox.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_pm.h"
#include "bbox/npu_black_box.h"
#include "npu_heart_beat.h"

int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{

	if (from == NULL || n == 0){
		devdrv_drv_err("user pointer is NULL.\n");
		return -EINVAL;
	}

	if (copy_from_user(to, (void *)from, n))
		return -ENODEV;

	return 0;
}

int copy_to_user_safe(void __user *to, const void *from, unsigned long len)
{

	if (to == NULL || len == 0){
		devdrv_drv_err("user pointer is NULL or len(%lu) is 0.\n", len);
		return -EINVAL;
	}

	if (copy_to_user(to, (void *)from, len))
		return -ENODEV;

	return 0;
}

int devdrv_ioctl_alloc_stream(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	u32 stream_id = 0;
	int ret = 0;
	int strategy = 0;
	struct devdrv_stream_strategy_ioctl_info *para =
			(struct devdrv_stream_strategy_ioctl_info*)((uintptr_t)arg);

	mutex_lock(&proc_ctx->stream_mutex);

	if (copy_from_user_safe(&strategy, (void *)(&para->strategy), sizeof(int)))
	{
		devdrv_drv_err("copy from user safe error.\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -EFAULT;
	}

	ret = devdrv_proc_alloc_stream(proc_ctx, &stream_id, strategy);
	if(ret != 0) {
		devdrv_drv_err("devdrv_alloc_stream failed.\n");
		mutex_unlock(&proc_ctx->stream_mutex);
		return -ENOKEY;
	}

	if (copy_to_user_safe((void *)(&para->stream_id), &stream_id, sizeof(int))) {
		devdrv_drv_err("copy to user safe stream_id = %d error.\n",stream_id);
		ret = devdrv_proc_free_stream(proc_ctx, stream_id);
		if (ret != 0) {
			devdrv_drv_err("devdrv_ioctl_free_stream_id = %d error.\n",stream_id);
		}
		mutex_unlock(&proc_ctx->stream_mutex);
		return -EFAULT;
	}
	bitmap_set(proc_ctx->stream_bitmap, stream_id, 1);
	mutex_unlock(&proc_ctx->stream_mutex);
	return 0;
}

int devdrv_ioctl_get_occupy_stream_id(struct devdrv_proc_ctx *proc_ctx,
													unsigned long arg)
{
	struct devdrv_occupy_stream_id *stream_id = NULL;
	struct devdrv_stream_sub_info *stream_sub_info = NULL;
	struct list_head *pos = NULL, *n = NULL;
	int ret;

	stream_id = kzalloc(sizeof(struct devdrv_occupy_stream_id), GFP_KERNEL);
	if (stream_id == NULL) {
		devdrv_drv_err("kmalloc fail.\n");
		return -1;
	}

	stream_id->count = 0;

	list_for_each_safe(pos, n, &proc_ctx->sink_stream_list) {
		stream_sub_info = list_entry(pos, struct devdrv_stream_sub_info, list);
		stream_id->id[stream_id->count] = stream_sub_info->id;
		stream_id->count++;
	}

	list_for_each_safe(pos, n, &proc_ctx->stream_list) {
		stream_sub_info = list_entry(pos, struct devdrv_stream_sub_info, list);
		stream_id->id[stream_id->count] = stream_sub_info->id;
		stream_id->count++;
	}

	ret = copy_to_user_safe((void *)(uintptr_t)arg, stream_id,
						sizeof(struct devdrv_occupy_stream_id));
	if (ret != 0) {
		devdrv_drv_err("copy_to_user_safe fail.\n");
		kfree(stream_id);
		stream_id = NULL;
		return -1;
	}

	kfree(stream_id);
	stream_id = NULL;
	return 0;
}

int devdrv_ioctl_alloc_event(struct devdrv_proc_ctx *proc_ctx,
							unsigned long arg)
{
	u32 event_id = 0;
	int ret;

	mutex_lock(&proc_ctx->event_mutex);
	ret = devdrv_proc_alloc_event(proc_ctx, &event_id);
	if (ret != 0) {
		devdrv_drv_err("proc alloc event failed, event id: %d\n",
													event_id);
		mutex_unlock(&proc_ctx->event_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void *)(uintptr_t)arg, &event_id, sizeof(int))) {
		devdrv_drv_err("copy to user safe event_id = %d error.\n",
															event_id);
		if (event_id != DEVDRV_MAX_EVENT_ID) {
			ret = devdrv_proc_free_event(proc_ctx, event_id);
			if (ret != 0) {
				devdrv_drv_err("proc free event id failed, event id: %d\n",
															event_id);
				mutex_unlock(&proc_ctx->event_mutex);
				return -EFAULT;
			}
			mutex_unlock(&proc_ctx->event_mutex);
			return -EFAULT;
		}
	}
	bitmap_set(proc_ctx->event_bitmap, event_id, 1);
	mutex_unlock(&proc_ctx->event_mutex);
	return 0;
}

int devdrv_ioctl_alloc_model(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	u32 model_id = 0;
	int ret;

	mutex_lock(&proc_ctx->model_mutex);
	ret = devdrv_proc_alloc_model(proc_ctx, &model_id);
	if (ret != 0) {
		devdrv_drv_err("proc alloc model failed, model id: %d\n",
													model_id);
		mutex_unlock(&proc_ctx->model_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void *)(uintptr_t)arg, &model_id, sizeof(int))) {
		devdrv_drv_err("copy to user safe model_id = %d error.\n",
															model_id);
		if (model_id != DEVDRV_MAX_EVENT_ID) {
			ret = devdrv_proc_free_model(proc_ctx, model_id);
			if (ret != 0) {
				devdrv_drv_err("proc free model id failed, model id: %d\n",
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

int devdrv_ioctl_alloc_task(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	u32 task_id = 0;
	int ret;

	mutex_lock(&proc_ctx->task_mutex);
	ret = devdrv_proc_alloc_task(proc_ctx, &task_id);
	if (ret != 0) {
		devdrv_drv_err("proc alloc task failed, task id: %d\n",
													task_id);
		mutex_unlock(&proc_ctx->task_mutex);
		return -EFAULT;
	}

	if (copy_to_user_safe((void *)(uintptr_t)arg, &task_id, sizeof(int))) {
		devdrv_drv_err("copy to user safe task_id = %d error.\n",
															task_id);
		if (task_id != DEVDRV_MAX_TASK_ID) {
			ret = devdrv_proc_free_task(proc_ctx, task_id);
			if (ret != 0) {
				devdrv_drv_err("proc free task id failed, task id: %d\n",
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

static int devdrv_check_ioctl_free_stream_para(struct devdrv_proc_ctx *proc_ctx,
											 unsigned long arg, u32* pstream_id)
{
	mutex_lock(&proc_ctx->stream_mutex);
	DECLARE_BITMAP(tmp_bitmap, DEVDRV_MAX_STREAM_ID);

	if (copy_from_user_safe(pstream_id, (void *)(uintptr_t)arg, sizeof(int))) {
		mutex_unlock(&proc_ctx->stream_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_stream_para error\n");
		return -EFAULT;
	}

	if(*pstream_id >= DEVDRV_MAX_STREAM_ID) {
		mutex_unlock(&proc_ctx->stream_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_stream_para free_stream_id %d\n",
		*pstream_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->stream_bitmap, DEVDRV_MAX_STREAM_ID);
	bitmap_set(tmp_bitmap, *pstream_id, 1);
	// if tmp_bitmap == proc_ctx->stream_bitmap ,bitmap_equal return 1, or return 0
	if(bitmap_equal(tmp_bitmap, proc_ctx->stream_bitmap, DEVDRV_MAX_STREAM_ID) == 0){
		mutex_unlock(&proc_ctx->stream_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_stream_para invalidate id %d\n",
			*pstream_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->stream_mutex);

	return 0;
}

static int devdrv_check_ioctl_free_event_para(struct devdrv_proc_ctx *proc_ctx,
	unsigned long arg, u32* event_id)
{
	mutex_lock(&proc_ctx->event_mutex);
	DECLARE_BITMAP(tmp_bitmap, DEVDRV_MAX_EVENT_ID);
	if (copy_from_user_safe(event_id, (void *)(uintptr_t)arg, sizeof(int))) {
		mutex_unlock(&proc_ctx->event_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_event_para error\n");
		return -EFAULT;
	}

	if(*event_id >= DEVDRV_MAX_EVENT_ID) {
		mutex_unlock(&proc_ctx->event_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_event_para free_stream_id %d\n",
			*event_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->event_bitmap, DEVDRV_MAX_EVENT_ID);
	bitmap_set(tmp_bitmap, *event_id, 1);
	// if tmp_bitmap == proc_ctx->event_bitmap ,bitmap_equal return 1, or return 0
	if(bitmap_equal(tmp_bitmap, proc_ctx->event_bitmap, DEVDRV_MAX_EVENT_ID) == 0){
		mutex_unlock(&proc_ctx->event_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_event_para invalidate id %d\n",
			*event_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->event_mutex);

	return 0;
}

static int devdrv_check_ioctl_free_model_para(struct devdrv_proc_ctx *proc_ctx,
		unsigned long arg, u32* model_id)
{

	mutex_lock(&proc_ctx->model_mutex);
	DECLARE_BITMAP(tmp_bitmap, DEVDRV_MAX_MODEL_ID);

	if (copy_from_user_safe(model_id, (void *)(uintptr_t)arg, sizeof(int))) {
		mutex_unlock(&proc_ctx->model_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_model_para error\n");
		return -EFAULT;
	}

	if(*model_id >= DEVDRV_MAX_MODEL_ID) {
		mutex_unlock(&proc_ctx->model_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_model_para free_stream_id %d\n",
			*model_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->model_bitmap, DEVDRV_MAX_MODEL_ID);
	bitmap_set(tmp_bitmap, *model_id, 1);
	// if tmp_bitmap == proc_ctx->model_bitmap ,bitmap_equal return 1, or return 0
	if(bitmap_equal(tmp_bitmap, proc_ctx->model_bitmap, DEVDRV_MAX_MODEL_ID) == 0){
		mutex_unlock(&proc_ctx->model_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_event_para invalidate id %d\n",
			*model_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->model_mutex);

	return 0;
}

static int devdrv_check_ioctl_free_task_para(struct devdrv_proc_ctx *proc_ctx,
	unsigned long arg, u32* task_id)
{
	mutex_lock(&proc_ctx->task_mutex);
	DECLARE_BITMAP(tmp_bitmap, DEVDRV_MAX_TASK_ID);
	if (copy_from_user_safe(task_id, (void *)(uintptr_t)arg, sizeof(int))) {
		mutex_unlock(&proc_ctx->task_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_task_para error\n");
		return -EFAULT;
	}

	if(*task_id >= DEVDRV_MAX_TASK_ID) {
		mutex_unlock(&proc_ctx->task_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_task_para free_task_id %d\n",
		*task_id);
		return -EFAULT;
	}

	bitmap_copy(tmp_bitmap, proc_ctx->task_bitmap, DEVDRV_MAX_TASK_ID);
	bitmap_set(tmp_bitmap, *task_id, 1);
	if(bitmap_equal(tmp_bitmap, proc_ctx->task_bitmap, DEVDRV_MAX_TASK_ID) == 0){
		mutex_unlock(&proc_ctx->task_mutex);
		devdrv_drv_err("devdrv_check_ioctl_free_task_para invalidate id %d\n",
		*task_id);
		return -EFAULT;
	}
	mutex_unlock(&proc_ctx->task_mutex);

	return 0;
}

static int devdrv_proc_ioctl_free_stream(struct devdrv_proc_ctx *proc_ctx,
	u32 free_stream_id)
{
	int ret = 0;

	mutex_lock(&proc_ctx->stream_mutex);
	ret = devdrv_proc_free_stream(proc_ctx, free_stream_id);
	mutex_unlock(&proc_ctx->stream_mutex);
	if (ret != 0) {
		devdrv_drv_err("devdrv_ioctl_free_stream_id = %d error.\n",free_stream_id);
		return -EINVAL;
	}

	return ret;
}

static int devdrv_proc_ioctl_free_event(struct devdrv_proc_ctx *proc_ctx,
    u32 free_event_id)
{
	int ret = 0;

	mutex_lock(&proc_ctx->event_mutex);
	ret = devdrv_proc_free_event(proc_ctx, free_event_id);
	mutex_unlock(&proc_ctx->event_mutex);
	if (ret != 0) {
		devdrv_drv_err("free event id = %d error.\n", free_event_id);
		ret = -EINVAL;
	}

	return ret;
}

static int devdrv_proc_ioctl_free_model(struct devdrv_proc_ctx *proc_ctx,
    u32 free_model_id)
{
	int ret = 0;

	mutex_lock(&proc_ctx->model_mutex);
	ret = devdrv_proc_free_model(proc_ctx, free_model_id);
	mutex_unlock(&proc_ctx->model_mutex);
	if (ret != 0) {
		devdrv_drv_err("free model id = %d error.\n", free_model_id);
		ret = -EINVAL;
	}

	return ret;
}

static int devdrv_proc_ioctl_free_task(struct devdrv_proc_ctx *proc_ctx,
    u32 free_task_id)
{
	int ret = 0;

	mutex_lock(&proc_ctx->task_mutex);
	ret = devdrv_proc_free_task(proc_ctx, free_task_id);
	mutex_unlock(&proc_ctx->task_mutex);
	if (ret != 0) {
		devdrv_drv_err("free task id = %d error.\n", free_task_id);
		ret = -EINVAL;
	}

	return ret;
}

int devdrv_ioctl_free_stream(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	u32 stream_id = 0;

	ret = devdrv_check_ioctl_free_stream_para(proc_ctx, arg, &stream_id);
	if(ret != 0) {
		devdrv_drv_err("devdrv_ioctl_free_stream check para fail\n");
		return -EFAULT;
	}

	return devdrv_proc_ioctl_free_stream(proc_ctx, stream_id);
}

int devdrv_ioctl_free_event(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	int event_id = 0;

	ret = devdrv_check_ioctl_free_event_para(proc_ctx, arg, &event_id);
	if(ret != 0) {
		devdrv_drv_err("devdrv_ioctl_free_event check para fail\n");
		return -EFAULT;
	}

	return devdrv_proc_ioctl_free_event(proc_ctx, event_id);
}

int devdrv_ioctl_free_model(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	int model_id = 0;

	ret = devdrv_check_ioctl_free_model_para(proc_ctx, arg, &model_id);
	if(ret != 0) {
		devdrv_drv_err("devdrv_ioctl_free_model check para fail\n");
		return -EFAULT;
	}

	return devdrv_proc_ioctl_free_model(proc_ctx, model_id);
}

int devdrv_ioctl_free_task(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	int task_id = 0;

	ret = devdrv_check_ioctl_free_task_para(proc_ctx, arg, &task_id);
	if(ret) {
		devdrv_drv_err("devdrv_ioctl_free_task check para fail\n");
		return -EFAULT;
	}

	return devdrv_proc_ioctl_free_task(proc_ctx, task_id);
}

static int davinci_va_to_pa(struct davinci_area_info *params)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr = params->va;

	pgd_t *pgd = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *pte = NULL;

	pgd = pgd_offset(mm, addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return -EINVAL;

	pud = pud_offset(pgd, addr);
	if (pud_none(*pud) || pud_bad(*pud))
		return -EINVAL;

	pmd = pmd_offset(pud, addr); //lint !e666
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return -EINVAL;

	pte = pte_offset_map(pmd, addr); //lint !e666
	if (pte_none(*pte) || !pte_present(*pte))
		return -EINVAL;

	params->pa = PFN_PHYS(pte_pfn(*pte));

	return 0;
}

static int devdrv_ioctl_davinci_va_to_pa(u64 arg)
{
	int ret  = 0;
	struct davinci_area_info params;

	params.va = 0;
	params.pa = 0;
	params.len = 0;

	ret = copy_from_user_safe(&params, (void __user *)(uintptr_t)arg,sizeof(params));
	if (ret != 0) {
		devdrv_drv_err("fail to copy davinci_area_info params, ret = %d\n", ret);
		return -EINVAL;
	}
	ret = davinci_va_to_pa(&params);
	if (ret != 0) {
		devdrv_drv_err("fail to change the virtual pte, ret = %d\n", ret);
		return ret;
	}
	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &params,sizeof(params));
	if (ret != 0) {
		devdrv_drv_err("fail to copy phys params to user space, ret = %d\n",
			ret);
		return -EINVAL;
	}

	return ret;
}

static int devdrv_ioctl_get_chip_info(u64 arg)
{
	int ret  = 0;
	struct devdrv_chip_info info = {0};
	struct devdrv_mem_desc *l2_desc =
		devdrv_plat_get_reg_desc(DEVDRV_REG_L2BUF_BASE);

	if (l2_desc == NULL) {
		devdrv_drv_err("devdrv_plat_get_reg_desc failed.\n");
		return -EFAULT;
	}

	info.l2_size = l2_desc->len + 1; //becasue of dts will minus 1

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &info,sizeof(info));
	if (ret != 0) {
		devdrv_drv_err("fail to copy chip_info params to "
							"user space,ret = %d\n", ret);
		return -EINVAL;
	}

	return ret;
}

static int hisi_svm_get_ttbr(u64 *ttbr, u64 *tcr)
{
	unsigned long asid = 0;
	struct mm_struct *mm = NULL;
	if((ttbr == NULL) || (tcr == NULL))
	{
		devdrv_drv_err("invalid ttbr or tcr\n");
		return -EINVAL;
	}
	mm = get_task_mm(current);
	if(mm == NULL)
	{
		devdrv_drv_err("get mm is null.\n");
		return -EINVAL;
	}
	//flush cache? ion_flush_all_cpus_caches
	asid = ASID(mm);
	*ttbr = virt_to_phys(mm->pgd)|(asid << 48);
	*tcr  = read_sysreg(tcr_el1);

	devdrv_drv_debug("pgdaddr:0x:%pK, context:0x%pK, pa:0x%pK\n",
					mm->pgd, (u64 *)(mm->pgd),
					(void *)(uintptr_t)virt_to_phys(mm->pgd));

	mmput(mm);

	devdrv_drv_debug("asid:%lu ,ttbr:0x%pK, tcr:0x%pK.\n",
								asid, (void *)(uintptr_t)*ttbr,
								(void *)(uintptr_t)*tcr);

	return 0;
}


static int devdrv_ioctl_get_svm_ssid(struct devdrv_dev_ctx *dev_ctx, u64 arg)
{
	int ret  = 0;
	struct process_info info = {0};
	u16 ssid  = 0;

	ret = copy_from_user_safe(&info, (void __user *)(uintptr_t)arg,sizeof(info));
	if (ret != 0) {
		devdrv_drv_err("fail to copy process_info params, ret = %d\n",
			ret);
		return -EINVAL;
	}
	ret = hisi_svm_get_ssid((struct hisi_svm*)dev_ctx->hisi_svm,&ssid,
							&info.ttbr, &info.tcr);
	if (ret != 0) {
		devdrv_drv_err("fail to get ssid, ret = %d\n", ret);
		return ret;
	}

	info.pasid= ssid;

	ret = hisi_svm_get_ttbr(&info.ttbr, &info.tcr);
	if (ret != 0) {
		devdrv_drv_err("fail to get process info, ret = %d\n", ret);
		return ret;
	}

	devdrv_drv_info("get ssid 0x%x ttbr 0x%llx tcr 0x%llx \n",
					info.pasid, info.ttbr, info.tcr);

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &info,sizeof(info));
	if (ret != 0) {
		devdrv_drv_err("fail to copy process info params to user space,"
						"ret = %d\n", ret);
		return -EINVAL;
	}

	return ret;
}

int devdrv_ioctl_alloc_cm(struct devdrv_dev_ctx *dev_ctx, u64 arg)
{
	struct devdrv_cm_msg msg = {0};
	struct devdrv_cm_info info = {0};
	void* cpu_addr = NULL;
	struct devdrv_platform_info* plat_info = NULL;
	dma_addr_t dma_handle = 0;
	u32 share_num = 0;
	int ret = 0;
	u64 size =  0;

	if (dev_ctx == NULL)
	{
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	ret = copy_from_user_safe(&msg, (void __user *)(uintptr_t)arg,sizeof(msg));
	if (ret != 0) {
		devdrv_drv_err("fail to copy npu cm msg params, ret = %d\n",ret);
		return -EINVAL;
	}

	size = msg.size;

	if (devdrv_is_cm_available(dev_ctx,size))
	{
		devdrv_drv_err("exceed the limit, npu cm is not enough\n");
		return -ENOMEM;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed.\n");
		return -EINVAL;
	}

	if(dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid == 0 && dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].size != 0)
	{
		devdrv_drv_info("devdrv_ioctl_alloc_cm SHARE_NUM_OCCUPIED enter.\n");
		ret = devdrv_save_cm_info_occupied(dev_ctx, &share_num);
		if (ret != 0)
		{
			devdrv_drv_err("fail to save continue mem info ret %d\n", ret);
			return -ENOMEM;
		}

		msg.share_num = share_num;
	}
	else
	{
		cpu_addr = dma_alloc_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size,
				&dma_handle, GFP_KERNEL);
		if (cpu_addr == NULL)
		{
			devdrv_drv_err("fail to malloc cma mem size 0x%llx\n", size);
			return -ENOMEM;
		}

		info.cpu_addr = cpu_addr;
		info.dma_handle = dma_handle;
		info.size = size;

		ret = devdrv_save_cm_info(dev_ctx, &info, &share_num);
		if (ret != 0)
		{
			devdrv_drv_err("fail to save continue mem info ret %d\n", ret);
			dma_free_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size,
					cpu_addr, dma_handle);
			return -ENOMEM;
		}

		msg.share_num = share_num;
	}

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg, &msg, sizeof(msg));
	if (ret != 0)
	{
		dma_free_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size,
				cpu_addr, dma_handle);
		(void)devdrv_delete_cm_info(dev_ctx, share_num);
		devdrv_drv_err("fail to copy cm msg params to "
							"user space, ret = %d\n",ret);
		return -EINVAL;
	}

	(void)devdrv_inc_cm_total_size(dev_ctx,size);

	return 0;
}

int devdrv_ioctl_free_cm(struct devdrv_dev_ctx *dev_ctx, u64 arg)
{
	int ret  = 0;
	u32 share_num = 0 ;
	u64 size  = 0;
	struct devdrv_platform_info* plat_info = NULL;

	if (dev_ctx == NULL)
	{
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	ret = copy_from_user_safe(&share_num, (void __user *)(uintptr_t)arg,sizeof(share_num));
	if (ret != 0) {
		devdrv_drv_err("fail to copy share_num params, ret = %d\n",ret);
		return -EINVAL;
	}

	devdrv_drv_debug(" share_num = %d from user \n",share_num);

	ret = devdrv_is_cm_valid(dev_ctx,share_num);
	if (ret != 0){
		devdrv_drv_err("invalid share_num , size or cm.valid info \n");
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed.\n");
		return -EINVAL;
	}

	size = dev_ctx->cm->mem_info[share_num].size;
	if ((dev_ctx->cm->total_size) < size) {
		devdrv_drv_err("invalid size 0x%llx total size 0x%llx\n", size,
						dev_ctx->cm->total_size);
		return -EINVAL;
	}

	devdrv_drv_debug("valid size 0x%llx total size 0x%llx\n", size,
						dev_ctx->cm->total_size);

	if(SHARE_NUM_OCCUPIED == share_num) {
		devdrv_drv_info("devdrv_ioctl_free_cm SHARE_NUM_OCCUPIED enter.\n");
		(void)devdrv_delete_cm_info_occupied(dev_ctx);
	}
	else {
		dma_free_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size,
						dev_ctx->cm->mem_info[share_num].cpu_addr,
						dev_ctx->cm->mem_info[share_num].dma_handle);

		(void)devdrv_delete_cm_info(dev_ctx, share_num);
	}

	(void)devdrv_dec_cm_total_size(dev_ctx,size);

	return 0;
}

int devdrv_ioctl_powerup(struct devdrv_proc_ctx *proc_ctx,
									struct devdrv_dev_ctx *dev_ctx, u64 arg)
{
	int ret = 0;

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	ret = npu_powerup(dev_ctx);
	if (ret) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		devdrv_drv_err("npu powerup failed.\n");

		if (!dsm_client_ocuppy(davinci_dsm_client)) {
			dsm_client_record(davinci_dsm_client, "npu power up failed.\n");
			dsm_client_notify(davinci_dsm_client, DSM_AI_KERN_POWER_UP_ERR_NO);
			devdrv_drv_err("[I/DSM] %s dmd report.\n",
				davinci_dsm_client->client_name);
		}

		return ret;
	}

	ret = devdrv_proc_send_alloc_stream_mailbox(proc_ctx);
	if (ret) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		devdrv_drv_err("devdrv send stream mailbox failed.\n");
		return ret;
	}

	/* bbox heart beat init */
	ret = devdrv_heart_beat_init(dev_ctx);
	if (ret)
	{
		devdrv_drv_err("devdrv_heart_beat_init failed.\n");
	}

	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	return ret;
}

int devdrv_ioctl_powerdown(struct devdrv_proc_ctx *proc_ctx,
									struct devdrv_dev_ctx *dev_ctx, u64 arg)
{
	int ret = 0;

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);

	(void)devdrv_proc_clr_sqcq_info(proc_ctx);

	ret = npu_powerdown(dev_ctx);
	if (ret != 0) {
		mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		devdrv_drv_err("npu powerdown failed.\n");
		return ret;
	}

	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	return ret;
}

int devdrv_ioctl_reboot(struct devdrv_proc_ctx *proc_ctx,
									struct devdrv_dev_ctx *dev_ctx, u64 arg)
{
	return 0;
}

static int devdrv_check_ioctl_report_wait_para(struct devdrv_proc_ctx *proc_ctx,
									unsigned long arg, int* time_out)
{
	if (copy_from_user_safe(time_out, (void *)(uintptr_t)arg, sizeof(int))) {
		devdrv_drv_err("copy cq report time_out para from user error\n");
		return -EFAULT;
	}

	return 0;
}

static int devdrv_proc_ioctl_report_wait(struct devdrv_proc_ctx *proc_ctx,
									unsigned long arg, int time_out)
{
	int ret;

	//get report from cq_tail and compare it with cq_info->phase
	if (devdrv_proc_is_valid_report_received(proc_ctx) == true) {
		devdrv_drv_debug("get valid report now without sleep\n");
		ret = 1;
	}else {//will block cuurent thread
		devdrv_drv_debug("get invalid report and will sleep\n");
		proc_ctx->cq_tail_updated = CQ_HEAD_INITIAL_UPDATE_FLAG;
		ret = devdrv_proc_report_wait(proc_ctx, time_out);
//		if (devdrv_is_ts_work(cce_ctrl->dev_info) == 0) {
//			devdrv_drv_err("ts is down.\n");
//			ret = DEVDRV_BUS_DOWN;
//		}
		if (ret > 0) {//make cq_tail_updated flag restore to initial state
			proc_ctx->cq_tail_updated = CQ_HEAD_INITIAL_UPDATE_FLAG;
		}
	}

	if (copy_to_user_safe((void *)(uintptr_t)arg, &ret, sizeof(int))) {
		devdrv_drv_err("REPORT_POLLING copy_to_user_safe error\n");
		return -ENODEV;
	}
	return 0;
}

int devdrv_ioctl_report_wait(struct devdrv_proc_ctx *proc_ctx,
									unsigned long arg)
{
	int ret = 0;
	int time_out = 0;

	ret = devdrv_check_ioctl_report_wait_para(proc_ctx, arg, &time_out);
	if (ret != 0){
		devdrv_drv_err("devdrv_ioctl_report_wait fail\n");
		return -ENODEV;
	}

	ret = devdrv_proc_ioctl_report_wait(proc_ctx, arg, time_out);
	if (ret != 0){
		devdrv_drv_err("devdrv_ioctl_report_wait timeout = %d\n", time_out);
		return -ENODEV;
	}

	return 0;
}

int devdrv_check_ioctl_custom_para(struct devdrv_proc_ctx *proc_ctx,
									unsigned long arg,
									devdrv_custom_para_t* custom_para,
									struct devdrv_dev_ctx** dev_ctx)
{
    int ret = 0;

	ret = copy_from_user_safe(custom_para,
							  (void __user *)(uintptr_t)arg,
							  sizeof(devdrv_custom_para_t));
	if (ret != 0) {
		devdrv_drv_err("devdrv_check_ioctl_custom_para,ret = %d\n",ret);
		return -EINVAL;
	}

	if(custom_para->arg == 0){
		devdrv_drv_err("devdrv_check_ioctl_custom_para invalid arg\n");
		return -EINVAL;
	}

	*dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if((*dev_ctx) == NULL){
		devdrv_drv_err("devdrv_proc_ioctl_custom %d of npu process %d is null\n",
								proc_ctx->devid,proc_ctx->pid);
		return -1;
	}

	return ret;
}

static int devdrv_proc_ioctl_custom(struct devdrv_proc_ctx *proc_ctx,
									const devdrv_custom_para_t* custom_para,
									struct devdrv_dev_ctx *dev_ctx)
{
	int ret = 0;

	switch (custom_para->cmd) {
		case DEVDRV_IOC_VA_TO_PA:
			ret = devdrv_ioctl_davinci_va_to_pa(custom_para->arg);
			break;
		case DEVDRV_IOC_GET_SVM_SSID:
			ret = devdrv_ioctl_get_svm_ssid(dev_ctx, custom_para->arg);
			break;
		case DEVDRV_IOC_GET_CHIP_INFO:
			ret = devdrv_ioctl_get_chip_info(custom_para->arg);
			break;
		case DEVDRV_IOC_ALLOC_CONTIGUOUS_MEM:
			//need lock because cm_info global resoure
			ret = devdrv_ioctl_alloc_cm(dev_ctx, custom_para->arg);
			break;
		case DEVDRV_IOC_FREE_CONTIGUOUS_MEM:
			ret = devdrv_ioctl_free_cm(dev_ctx, custom_para->arg);
			break;
		case DEVDRV_IOC_POWERUP:
			ret = devdrv_ioctl_powerup(proc_ctx, dev_ctx, custom_para->arg);
			break;
		case DEVDRV_IOC_POWERDOWN:
			ret = devdrv_ioctl_powerdown(proc_ctx, dev_ctx, custom_para->arg);
			break;
		case DEVDRV_IOC_REBOOT:
			ret = devdrv_ioctl_reboot(proc_ctx, dev_ctx, custom_para->arg);
			break;
		default:
			devdrv_drv_err("invalid custom cmd 0x%x \n", custom_para->cmd);
			ret = -EINVAL;
			break;
	}

	return ret;
}

int devdrv_ioctl_custom(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	int ret = 0;
	devdrv_custom_para_t custom_para = {0};
	struct devdrv_dev_ctx* dev_ctx = NULL;

	ret = devdrv_check_ioctl_custom_para(proc_ctx, arg, &custom_para, &dev_ctx);
	if (ret != 0) {
		devdrv_drv_err("devdrv_ioctl_custom,ret = %d\n",ret);
		return -EINVAL;
	}

	ret = devdrv_proc_ioctl_custom(proc_ctx, &custom_para, dev_ctx);
	if (ret != 0) {
		devdrv_drv_err("devdrv_ioctl_custom call devdrv_proc_ioctl_custom,ret = %d\n",
			ret);
		return -EINVAL;
	}

	return ret;
}

/*
 * new add for TS timeout function
 */
int devdrv_ioctl_get_ts_timeout( struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	uint64_t exception_code = 0;
	int ret = 0;

	if (copy_from_user_safe(&exception_code, (void *)(uintptr_t)arg, sizeof(uint64_t))) {
		devdrv_drv_err("copy_from_user_safe error.\n");
		return -EFAULT;
	}

	if (exception_code < (uint64_t)MODID_NPU_START || exception_code > (uint64_t)MODID_NPU_EXC_END) {
		devdrv_drv_err("expection code %llu out of npu range\n", exception_code);
		return -1;
	}

	/* receive TS exception */
	devdrv_drv_err("call rdr_system_error.\n");
	rdr_system_error(exception_code, 0, 0);

	return ret;
}

static int devdrv_ioctl_message_send_one(struct devdrv_dev_ctx *dev_ctx,
										struct devdrv_mailbox_message *message)
{
	int result;
	int ret;

	ret = devdrv_mailbox_message_send(&dev_ctx->mailbox, message, &result);
	if (ret != 0){
		kfree(message->message_payload);
		message->message_payload = NULL;

		kfree(message);
		message = NULL;

		devdrv_drv_err("devdrv_mailbox_message_send_one failed.\n");
		return ret;
	}
	/* transmisson ok */
	else if (message->sync_type == DEVDRV_MAILBOX_SYNC) {
		kfree(message->message_payload);
		message->message_payload = NULL;
		kfree(message);
		message = NULL;
	}

	return ret;
}

int devdrv_ioctl_mailbox_send(struct devdrv_proc_ctx *proc_ctx, unsigned long arg)
{
	struct devdrv_mailbox_user_message *user_message = NULL;
	struct devdrv_mailbox_message *message = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;
	int ret;

	devdrv_drv_err("enter, arg: 0x%lx.\n", arg);

	if (proc_ctx->devid >= NPU_DEV_NUM) {
		devdrv_drv_err("device id is illegal.\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (dev_ctx == NULL) {
		devdrv_drv_err("get device context by device id failed.\n");
		return -ENODATA;
	}

	user_message = kzalloc(sizeof(struct devdrv_mailbox_user_message), GFP_KERNEL);
	if (user_message == NULL) {
		devdrv_drv_err("kmalloc failed.\n");
		return -ENOMEM;
	}

	ret = copy_from_user_safe(user_message, (void *)(uintptr_t)arg, sizeof(struct devdrv_mailbox_user_message));
	devdrv_drv_err("copy_from_user_safe.\n");
	if (ret != 0) {
		kfree(user_message);
		user_message = NULL;
		devdrv_drv_err("copy_from_user_safe error.\n");
		return -EFAULT;
	}

	devdrv_drv_err("user_message->message_length: %d.\n", user_message->message_length);
	if (user_message->message_length <= 0 ||
		user_message->message_length > DEVDRV_MAILBOX_PAYLOAD_LENGTH) {
		kfree(user_message);
		user_message = NULL;
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	message = kzalloc(sizeof(struct devdrv_mailbox_message), GFP_KERNEL);
	if (message == NULL) {
		kfree(user_message);
		user_message = NULL;
		devdrv_drv_err("kmalloc failed.\n");
		return -ENOMEM;
	}

	message->message_payload = kzalloc(DEVDRV_MAILBOX_PAYLOAD_LENGTH, GFP_KERNEL);
	if (message->message_payload == NULL) {
		kfree(message);
		message = NULL;
		kfree(user_message);
		user_message = NULL;
		devdrv_drv_err("kmalloc failed.\n");
		return -ENOMEM;
	}

	memcpy(message->message_payload,
			user_message->message_payload, user_message->message_length);
	message->message_length = user_message->message_length;

	message->process_result = 0;
	message->sync_type = user_message->sync_type;
	message->cmd_type= user_message->cmd_type;
	message->message_index = user_message->message_index;
	message->message_pid = user_message->message_pid;
	message->mailbox = &dev_ctx->mailbox;
	message->abandon = 0;

	kfree(user_message);
	user_message = NULL;

	return devdrv_ioctl_message_send_one(dev_ctx, message);

}

long devdrv_npu_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct devdrv_proc_ctx *proc_ctx = NULL;
	int ret = 0;

	proc_ctx = (struct devdrv_proc_ctx *)filep->private_data;
	if(proc_ctx == NULL || arg == 0) {
		devdrv_drv_err("invalid devdrv_npu_ioctl parameter,arg = 0x%lx,cmd = %d\n",
			arg, cmd);
		return -EINVAL;
	}

	ret = devdrv_proc_npu_ioctl_call(proc_ctx, cmd, arg);
	if(ret != 0) {
		devdrv_drv_err("devdrv_npu_ioctl process failed,arg = %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}

