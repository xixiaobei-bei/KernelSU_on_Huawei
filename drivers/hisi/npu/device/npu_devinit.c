/*
 * npu_devinit.c
 *
 * about npu devinit
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
#include "npu_devinit.h"

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
#include <linux/uio_driver.h>
#include <linux/irq.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/pm_wakeup.h>
#include <linux/atomic.h>

#include "npu_shm.h"
#include "npu_stream.h"
#include "npu_sink_stream.h"
#include "npu_mailbox.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "npu_log.h"
#include "npu_pm.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_doorbell.h"
#include "npu_dfx_cq.h"
#include "npu_heart_beat.h"
#include "npu_hwts.h"
#include "npu_manager_common.h"
#include "npu_recycle.h"
#include "npu_ioctl_services.h"

#include <linux/hisi/hisi_svm.h>

static unsigned int npu_major;
static struct class *npu_class = NULL;

static int devdrv_resource_list_init(u8 dev_id)
{
	int ret;
	ret = devdrv_stream_list_init(dev_id);
	COND_RETURN_ERROR(ret != 0, -1, "npu dev id = %u stream list init failed\n", dev_id);

	ret = devdrv_sink_stream_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, sink_list_init_failed, ret, ret,
		"npu dev id = %u sink stream list init failed\n", dev_id);

	ret = devdrv_sq_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, sq_list_init_failed, ret, -1, "npu dev id = %d sq list init failed\n", dev_id);

	ret = devdrv_cq_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, cq_list_init_failed, ret, -1, "npu dev id = %d cq list init failed\n", dev_id);

	ret = devdrv_event_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, event_list_init_failed, ret, -1, "npu dev id = %d event list init failed\n", dev_id);

	ret = devdrv_model_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, model_list_init_failed, ret, -1, "npu dev id = %d model list init failed\n", dev_id);

	ret = devdrv_task_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, task_list_init_failed, ret, -1, "npu dev id = %d task list init failed\n", dev_id);

	ret = devdrv_mailbox_init(dev_id);
	COND_GOTO_ERROR(ret != 0, mailbox_init_failed, ret, -1, "npu dev id = %d mailbox init failed\n", dev_id);

	ret = devdrv_dev_doorbell_init();
	COND_GOTO_ERROR(ret != 0, doorbell_init_failed, ret, -1, "npu dev id = %d doorbell init failed\n", dev_id);

	return ret;
doorbell_init_failed:
	devdrv_mailbox_destroy(dev_id);
mailbox_init_failed:
	devdrv_task_list_destroy(dev_id);
task_list_init_failed:
	devdrv_model_list_destroy(dev_id);
model_list_init_failed:
	devdrv_event_list_destroy(dev_id);
event_list_init_failed:
	devdrv_cq_list_destroy(dev_id);
cq_list_init_failed:
	devdrv_sq_list_destroy(dev_id);
sq_list_init_failed:
	devdrv_sink_stream_list_destroy(dev_id);
sink_list_init_failed:
	devdrv_stream_list_destroy(dev_id);
	return ret;
}

static void devdrv_resource_list_destroy(u8 dev_id)
{
	devdrv_model_list_destroy(dev_id);
	devdrv_task_list_destroy(dev_id);
	devdrv_event_list_destroy(dev_id);
	devdrv_cq_list_destroy(dev_id);
	devdrv_sq_list_destroy(dev_id);
	devdrv_stream_list_destroy(dev_id);
	devdrv_sink_stream_list_destroy(dev_id);
	devdrv_mailbox_destroy(dev_id);
}

static void devdrv_synchronous_resource_init(struct devdrv_dev_ctx *dev_ctx)
{
	spin_lock_init(&dev_ctx->spinlock);
	spin_lock_init(&dev_ctx->event_spinlock);
	spin_lock_init(&dev_ctx->model_spinlock);
	spin_lock_init(&dev_ctx->task_spinlock);
	spin_lock_init(&dev_ctx->notify_spinlock);
	spin_lock_init(&dev_ctx->ts_spinlock);

	mutex_init(&dev_ctx->npu_wake_lock_mutex);
	mutex_init(&dev_ctx->npu_power_up_off_mutex);
	mutex_init(&dev_ctx->npu_open_release_mutex);

	mutex_init(&dev_ctx->cq_mutex_t);
	mutex_init(&dev_ctx->stream_mutex_t);
	mutex_init(&dev_ctx->event_mutex_t);
	mutex_init(&dev_ctx->model_mutex_t);
	mutex_init(&dev_ctx->notify_mutex_t);
	mutex_init(&dev_ctx->cm_mutex_t);

	atomic_set(&dev_ctx->power_access, 1);
	atomic_set(&dev_ctx->power_success, 1);
	atomic_set(&dev_ctx->open_access, 1);
	atomic_set(&dev_ctx->open_success, 1);

	snprintf(dev_ctx->wakelock_name, DEVDRV_WAKELOCK_SIZE,
		"npu_power_wakelock%d%c", dev_ctx->devid, '\0');
	wakeup_source_init(&dev_ctx->wakelock, dev_ctx->wakelock_name);
}

static void devdrv_synchronous_resource_destroy(struct devdrv_dev_ctx *dev_ctx)
{
	wakeup_source_trash(&dev_ctx->wakelock);

	mutex_destroy(&dev_ctx->npu_wake_lock_mutex);
	mutex_destroy(&dev_ctx->npu_power_up_off_mutex);
	mutex_destroy(&dev_ctx->npu_open_release_mutex);

	mutex_destroy(&dev_ctx->cq_mutex_t);
	mutex_destroy(&dev_ctx->stream_mutex_t);
	mutex_destroy(&dev_ctx->event_mutex_t);
	mutex_destroy(&dev_ctx->model_mutex_t);
	mutex_destroy(&dev_ctx->notify_mutex_t);
	mutex_destroy(&dev_ctx->cm_mutex_t);
}

void npu_vm_open(struct vm_area_struct *vma)
{
	COND_RETURN_VOID(vma == NULL, "davinci munmap vma is null\n");

	NPU_DRV_DEBUG("davinci munmap: vma=0x%lx, vm_start=0x%lx, vm_end=0x%lx\n",
		vma, vma->vm_start, vma->vm_end);
}

void npu_vm_close(struct vm_area_struct *vma)
{
	struct npu_vma_mmapping *npu_vma_map = NULL;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	COND_RETURN_VOID(vma == NULL, "davinci munmap vma is null\n");

	NPU_DRV_DEBUG("davinci munmap: vma=0x%lx, vm_start=0x%lx, vm_end=0x%lx\n",
		vma, vma->vm_start, vma->vm_end);
	npu_vma_map = (struct npu_vma_mmapping *)vma->vm_private_data;
	COND_RETURN_VOID(npu_vma_map == NULL, "davinci mmap npu_vma_map is null\n");

	proc_ctx = (struct devdrv_proc_ctx *)npu_vma_map->proc_ctx;
	COND_RETURN_VOID(proc_ctx == NULL, "proc_ctx is null\n");

	if(npu_vma_map->map_type > MAP_RESERVED && npu_vma_map->map_type < MAP_MAX) {
		mutex_lock(&proc_ctx->map_mutex);
		list_del (&npu_vma_map->list);
		mutex_unlock(&proc_ctx->map_mutex);
	}
	kfree(npu_vma_map);
	vma->vm_private_data = NULL;
}

const struct vm_operations_struct npu_vm_ops = {
	.open  = npu_vm_open,
	.close = npu_vm_close,
};

static int devdrv_npu_map(struct file *filep, struct vm_area_struct *vma)
{
	unsigned long vm_pgoff;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	struct npu_vma_mmapping *npu_vma_map = NULL;
	u32 map_type;
	u32 share_num;
	u8 dev_id;
	int ret;

	if ((vma == NULL) || (filep == NULL)) {
		NPU_DRV_ERR("invalid vma or filep\n");
		return -EFAULT;
	}

	proc_ctx = (struct devdrv_proc_ctx *)filep->private_data;
	if (proc_ctx == NULL) {
		NPU_DRV_ERR("proc_ctx is NULL\n");
		return -EFAULT;
	}

	npu_vma_map = (struct npu_vma_mmapping *)kzalloc (sizeof(struct npu_vma_mmapping), GFP_KERNEL);
	COND_RETURN_ERROR(npu_vma_map == NULL, -EINVAL, "alloc npu_vma_map fail\n");

	dev_id = proc_ctx->devid;
	vm_pgoff = vma->vm_pgoff;
	map_type = MAP_GET_TYPE(vm_pgoff);

	mutex_lock(&proc_ctx->map_mutex);
	NPU_DRV_WARN("map_type = %d memory mmap start. vm_pgoff=0x%lx, vm_start=0x%lx, vm_end=0x%lx\n",
		map_type, vm_pgoff, vma->vm_start, vma->vm_end);
	switch (map_type) {
	case MAP_RESERVED:
		ret = -EINVAL;
		break;
	case MAP_L2_BUFF:
		ret = devdrv_map_l2_buff(filep, vma, dev_id);
		list_add(&npu_vma_map->list, &proc_ctx->l2_vma_list);
		break;
	case MAP_CONTIGUOUS_MEM:
		share_num = MAP_GET_SHARE_NUM(vm_pgoff);
		ret = devdrv_map_cm(proc_ctx, vma, share_num, dev_id);
		list_add(&npu_vma_map->list, &proc_ctx->cma_vma_list);
		break;
	case MAP_INFO_SQ_CQ_MEM:
		ret = devdrv_info_sq_cq_mmap(dev_id, filep, vma);
		list_add(&npu_vma_map->list, &proc_ctx->sqcq_vma_list);
		break;
	default:
		NPU_DRV_ERR("map_type is error\n");
		ret = -EINVAL;
		break;
	}
	npu_vma_map->map_type = map_type;
	npu_vma_map->proc_ctx = proc_ctx;
	npu_vma_map->vma = vma;

	vma->vm_flags |= VM_DONTCOPY;
	vma->vm_ops = &npu_vm_ops;
	vma->vm_private_data = (void *)npu_vma_map;
	vma->vm_ops->open(vma);

	mutex_unlock(&proc_ctx->map_mutex);
	if (ret != 0)
		NPU_DRV_ERR("map_type = %d memory mmap failed\n", map_type);

	return ret;
}

static int devdrv_npu_open(struct inode *inode, struct file* filep)
{
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	unsigned int minor = iminor(inode);
	struct devdrv_ts_cq_info *cq_info = NULL;
	// stub
	const u8 dev_id = 0;  // should get from manager info
	int ret;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return -EINVAL;
	}

	NPU_DRV_WARN("devdrv_open start. minor = %d \n", minor);
	COND_RETURN_ERROR(minor >= DEVDRV_MAX_DAVINCI_NUM, -ENODEV, "invalid npu minor num, minor = %d\n", minor);

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	COND_RETURN_ERROR(cur_dev_ctx == NULL, -ENODEV, "cur_dev_ctx %d is null\n", dev_id);

	cur_dev_ctx->will_powerdown = false;

	mutex_lock(&cur_dev_ctx->npu_open_release_mutex);

	proc_ctx = kzalloc(sizeof(struct devdrv_proc_ctx), GFP_KERNEL);
	if (proc_ctx == NULL) {
		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
		NPU_DRV_ERR("alloc memory for proc_ctx failed\n");
		return -ENOMEM;
	}

	ret = devdrv_proc_ctx_init(proc_ctx);
	if (ret != 0) {
		devdrv_proc_ctx_destroy(&proc_ctx);
		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
		NPU_DRV_ERR("init proc ctx failed\n");
		return ret;
	}

	proc_ctx->devid = cur_dev_ctx->devid;
	proc_ctx->pid = current->tgid;
	filep->private_data = (void *)proc_ctx;
	proc_ctx->filep = filep;

	// alloc cq for current process
	mutex_lock(&cur_dev_ctx->cq_mutex_t);
	cq_info = devdrv_proc_alloc_cq(proc_ctx);
	if (cq_info == NULL) {
		mutex_unlock(&cur_dev_ctx->cq_mutex_t);
		NPU_DRV_ERR("alloc persistent cq for proc_context failed\n");
		ret = -ENOMEM;
		goto proc_alloc_cq_failed;
	}
	NPU_DRV_DEBUG("alloc persistent cq for proc_context success\n");
	mutex_unlock(&cur_dev_ctx->cq_mutex_t);

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		ret = devdrv_proc_alloc_proc_ctx_sub(proc_ctx);
		if (ret != 0) {
			NPU_DRV_ERR("alloc prox_ctx_sub failed\n");
			goto proc_alloc_prox_ctx_sub_failed;
		}
	}

	// add proc_ctx to cur dev_ctx proc list
	(void)devdrv_add_proc_ctx(&proc_ctx->dev_ctx_list, dev_id);

	// bind proc_ctx to cq report int ctx
	devdrv_bind_proc_ctx_with_cq_int_ctx(proc_ctx);

	ret = npu_open(cur_dev_ctx);
	if (ret != 0) {
		NPU_DRV_ERR("npu open failed\n");
		goto npu_open_failed;
	}

	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
	devdrv_cm_resource_recycle(cur_dev_ctx);

	NPU_DRV_DEBUG("devdrv_open succeed\n");
	return 0;

npu_open_failed:
	devdrv_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)devdrv_remove_proc_ctx(&proc_ctx->dev_ctx_list, proc_ctx->devid);
	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1)
		(void)devdrv_proc_free_proc_ctx_sub(proc_ctx);

proc_alloc_prox_ctx_sub_failed:
	(void)devdrv_proc_free_cq(proc_ctx);
proc_alloc_cq_failed:
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
	devdrv_proc_ctx_destroy(&proc_ctx);
	return ret;
}

static int devdrv_npu_release(struct inode *inode, struct file *filep)
{
	int ret = 0;
	u8 dev_id;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	COND_RETURN_ERROR(plat_info == NULL, -EINVAL, "devdrv_plat_get_info\n");

	proc_ctx = (struct devdrv_proc_ctx*)filep->private_data;
	COND_RETURN_ERROR(proc_ctx == NULL, -EINVAL, "get proc_ctx fail\n");

	NPU_DRV_DEBUG("devdrv_release start. minor = %d, dev_id = %d\n", iminor(inode), proc_ctx->devid);
	dev_id = proc_ctx->devid; // should get from manager info
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	COND_RETURN_ERROR(cur_dev_ctx == NULL, -EINVAL, "cur_dev_ctx %d is null\n", dev_id);

	/* device will be power down,so some message donot need to inform ts */
	cur_dev_ctx->will_powerdown = true;

	devdrv_cm_resource_recycle(cur_dev_ctx);

	// callback char dev release rs before npu powerdown
	devdrv_release_npu_callback_proc(dev_id);

	mutex_lock(&cur_dev_ctx->npu_open_release_mutex);

	// resource leak happened
	if (devdrv_is_proc_resource_leaks(proc_ctx) == true) {
		devdrv_resource_leak_print(proc_ctx);
		// to be done, should judge is ts alive to decide whether inform ts
		devdrv_recycle_npu_resources(proc_ctx);
		mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

		ret = npu_powerdown(proc_ctx, cur_dev_ctx);
		if (ret != -EBUSY && ret != 0)
			NPU_DRV_ERR("npu powerdown failed\n");

		mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
		ret = npu_release(cur_dev_ctx);
		if (ret != 0)
			NPU_DRV_ERR("npu release failed\n");

		devdrv_proc_ctx_destroy(&proc_ctx);

		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);

		return 0;
	}

	// normal case
	devdrv_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)devdrv_remove_proc_ctx(&proc_ctx->dev_ctx_list, proc_ctx->devid);

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1)
		(void)devdrv_proc_free_proc_ctx_sub(proc_ctx);

	(void)devdrv_proc_free_cq(proc_ctx);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	ret = npu_powerdown(proc_ctx, cur_dev_ctx);

	if (ret != -EBUSY && ret != 0)
		NPU_DRV_ERR("npu powerdown failed\n");

	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	ret = npu_release(cur_dev_ctx);
	if (ret != 0)
		NPU_DRV_ERR("npu release failed\n");

	mutex_lock(&cur_dev_ctx->npu_wake_lock_mutex);
	__pm_relax(&cur_dev_ctx->wakelock);
	mutex_unlock(&cur_dev_ctx->npu_wake_lock_mutex);

	devdrv_proc_ctx_destroy(&proc_ctx);
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);

	NPU_DRV_DEBUG("devdrv_npu_release success\n");
	return ret;
}

const struct file_operations npu_dev_fops = {
	.owner = THIS_MODULE,
	.open = devdrv_npu_open,
	.release = devdrv_npu_release,
	.unlocked_ioctl	= devdrv_npu_ioctl,
	.mmap = devdrv_npu_map,
};

static int devdrv_register_npu_chrdev_to_kernel(struct module *owner, struct devdrv_dev_ctx *dev_ctx,
                                                const struct file_operations *devdrv_fops)
{
	struct device *i_device = NULL;
	int ret;
	dev_t devno;

	devno = MKDEV(npu_major, dev_ctx->devid);

	dev_ctx->devdrv_cdev.owner = owner;
	cdev_init(&dev_ctx->devdrv_cdev, devdrv_fops);
	ret = cdev_add(&dev_ctx->devdrv_cdev, devno, DEVDRV_MAX_DAVINCI_NUM);
	if (ret != 0) {
		NPU_DRV_ERR("npu cdev_add error\n");
		return -1;
	}

	i_device = device_create(npu_class, NULL, devno, NULL, "davinci%d", dev_ctx->devid);
	if (i_device == NULL) {
		NPU_DRV_ERR("device_create error\n");
		ret = -ENODEV;
		goto device_fail;
	}
	dev_ctx->npu_dev = i_device;
	return ret;
device_fail:
	cdev_del(&dev_ctx->devdrv_cdev);
	return ret;
}

static void devdrv_unregister_npu_chrdev_from_kernel(struct devdrv_dev_ctx *dev_ctx)
{
	cdev_del(&dev_ctx->devdrv_cdev);
	device_destroy(npu_class, MKDEV(npu_major, dev_ctx->devid));
}

/*
 * devdrv_drv_register - register a new devdrv device
 * @devdrv_info: devdrv device info
 * returns zero on success
 */
static int devdrv_drv_register(struct module *owner, u8 dev_id, const struct file_operations *devdrv_fops)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	int ret;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	COND_RETURN_ERROR(plat_info == NULL, -EINVAL, "devdrv_plat_get_info\n");

	NPU_DRV_DEBUG("dev %u devdrv_drv_register started\n", dev_id);
	COND_RETURN_ERROR(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id = %u\n", dev_id);

	dev_ctx = kzalloc(sizeof(struct devdrv_dev_ctx), GFP_KERNEL);
	COND_RETURN_ERROR(dev_ctx == NULL, -ENOMEM, "kmalloc devid = %u dev_ctx failed\n", dev_id);

	set_dev_ctx_with_dev_id(dev_ctx, dev_id);
	dev_ctx->devid = dev_id;
	dev_ctx->secure_state = NPU_NONSEC;
	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		ret = devdrv_shm_v200_init(dev_id);
	} else {
		ret = devdrv_shm_init(dev_id);
	}
	if (ret != 0) {
		NPU_DRV_ERR("dev %d shm init failed\n", dev_id);
		ret = -ENODEV;
		goto shm_init_fail;
	}
	COND_GOTO_ERROR(ret != 0, shm_init_fail, ret, -ENODEV, "dev %d shm init failed\n", dev_id);

	ret = devdrv_resource_list_init(dev_id);
	COND_GOTO_ERROR(ret != 0, resource_list_init, ret, -ENODEV,
		"npu dev id = %u resource list init failed\n", dev_id);

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		ret = devdrv_dev_ctx_sub_init(dev_id);
		COND_GOTO_ERROR(ret != 0, ctx_sub_init_failed, ret, -ENODEV,
			"npu dev id = %u resource list init failed\n", dev_id);
	}

	// init proc_ctx list
	INIT_LIST_HEAD(&dev_ctx->proc_ctx_list);
	INIT_LIST_HEAD(&dev_ctx->rubbish_context_list);
	devdrv_synchronous_resource_init(dev_ctx);

	COND_GOTO_ERROR(devdrv_heart_beat_resource_init(dev_ctx), npu_chr_dev_register_failed, ret,
		-ENODEV, "devdrv_heart_beat_resource_init fail\n");

	COND_GOTO_ERROR(devdrv_dfx_cqsq_init(dev_ctx), npu_heart_beat_init_failed, ret, -ENODEV,
		"devdrv_init_functional_cqsq fail\n");

	ret = devdrv_register_npu_chrdev_to_kernel(owner, dev_ctx, devdrv_fops);
	COND_GOTO_ERROR(ret != 0, npu_chr_dev_init_dfx_cqsq_failed, ret, ret,
		"npu dev id = %u chrdev register failed \n", dev_id);

	NPU_DRV_DEBUG("devdrv_drv_register succeed\n");
	return 0;
npu_chr_dev_init_dfx_cqsq_failed:
	devdrv_destroy_dfx_cqsq(dev_ctx);
npu_heart_beat_init_failed:
	devdrv_heart_beat_resource_destroy(dev_ctx);
npu_chr_dev_register_failed:
	devdrv_synchronous_resource_destroy(dev_ctx);
	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1)
		devdrv_dev_ctx_sub_destroy(dev_id);
ctx_sub_init_failed:
	devdrv_resource_list_destroy(dev_id);
resource_list_init:
	devdrv_shm_destroy(dev_id);
shm_init_fail:
	kfree(dev_ctx);
	dev_ctx = NULL;
	set_dev_ctx_with_dev_id(NULL, dev_id);

	return ret;
}

/*
 * devdrv_drv_unregister - unregister a devdrv device
 * @devdrv_info: devdrv device info
 * returns zero on success
 */
void devdrv_drv_unregister(u8 dev_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return;
	}

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d\n", dev_id);
		return;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return;
	};

	devdrv_unregister_npu_chrdev_from_kernel(dev_ctx);
	devdrv_destroy_dfx_cqsq(dev_ctx);
	devdrv_heart_beat_resource_destroy(dev_ctx);
	devdrv_synchronous_resource_destroy(dev_ctx);
	devdrv_resource_list_destroy(dev_id);
	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		devdrv_dev_ctx_sub_destroy(dev_id);
		devdrv_shm_v200_destroy(dev_id);
	} else {
		devdrv_shm_destroy(dev_id);
	}

	kfree(dev_ctx);
	dev_ctx = NULL;
}

static char *devdrv_drv_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

int devdrv_devinit(struct module *owner, u8 dev_id)
{
	dev_t npu_dev = 0;
	int ret;

	NPU_DRV_DEBUG("npu dev %u init start\n", dev_id);

	dev_ctx_array_init();

	ret = alloc_chrdev_region(&npu_dev, 0, DEVDRV_MAX_DAVINCI_NUM, "npu-cdev");
	if (ret != 0) {
		NPU_DRV_ERR("alloc npu chr dev region error\n");
		return ret;
	}

	npu_major = MAJOR(npu_dev);
	npu_class = class_create(owner, "npu-class");
	if (IS_ERR(npu_class)) {
		NPU_DRV_ERR("class_create error\n");
		ret = -EINVAL;
		goto class_fail;
	}
	npu_class->devnode = devdrv_drv_devnode;

	ret = devdrv_drv_register(owner, dev_id, &npu_dev_fops);
	if (ret != 0) {
		NPU_DRV_ERR("npu %d devdrv_drv_register failed \n", dev_id);
		ret = -EINVAL;
		goto devdrv_drv_register_fail;
	}

	NPU_DRV_DEBUG("npu dev %d init succeed \n", dev_id);
	return ret;

devdrv_drv_register_fail:
	class_destroy(npu_class);
class_fail:
	unregister_chrdev_region(npu_dev, DEVDRV_MAX_DAVINCI_NUM);
	return ret;
}

void devdrv_devexit(u8 dev_id)
{
	devdrv_drv_unregister(dev_id);
	class_destroy(npu_class);
	unregister_chrdev_region(MKDEV(npu_major, 0), DEVDRV_MAX_DAVINCI_NUM);
}
