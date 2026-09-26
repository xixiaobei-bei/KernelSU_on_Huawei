/*
 * npu_devinit.c
 *
 * about npu devinit
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
#include "npu_devinit.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
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
#include <linux/hisi/hisi_svm.h>
#include <securec.h>

#include "npu_shm.h"
#include "npu_stream.h"
#include "npu_sink_stream.h"
#include "npu_mailbox.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "npu_log.h"
#include "npu_pm_framework.h"
#include "npu_event.h"
#include "npu_hwts_event.h"
#include "npu_model.h"
#include "npu_doorbell.h"
#include "npu_dfx_cq.h"
#include "npu_heart_beat.h"
#include "npu_hwts.h"
#include "npu_manager_common.h"
#include "npu_recycle.h"
#include "npu_ioctl_services.h"
#include "npu_svm.h"
#include "npu_task_message.h"

static unsigned int npu_major;
static struct class *npu_class;

static int npu_resource_list_init(struct npu_dev_ctx *dev_ctx)
{
	int ret;
	u8 dev_id;
	struct npu_id_allocator *id_allocator = NULL;

	cond_return_error(dev_ctx == NULL, -1, "dev_ctx is null\n");

	dev_id = dev_ctx->devid;
	ret = npu_stream_list_init(dev_ctx);
	cond_return_error(ret != 0, -1,
		"npu dev id = %u stream list init failed\n", dev_id);

	ret = npu_sink_stream_list_init(dev_ctx);
	cond_goto_error(ret != 0, sink_list_init_failed, ret, ret,
		"npu dev id = %u sink stream list init failed\n", dev_id);

	ret = npu_sq_list_init(dev_id);
	cond_goto_error(ret != 0, sq_list_init_failed, ret, -1,
		"npu dev id = %u sq list init failed\n", dev_id);

	ret = npu_cq_list_init(dev_id);
	cond_goto_error(ret != 0, cq_list_init_failed, ret, -1,
		"npu dev id = %u cq list init failed\n", dev_id);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_EVENT]);
	ret = npu_id_allocator_create(id_allocator, 0, NPU_MAX_EVENT_ID, 0);
	cond_goto_error(ret != 0, event_list_init_failed, ret, -1,
		"npu dev id = %u event list init failed\n", dev_id);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_HWTS_EVENT]);
	ret = npu_id_allocator_create(id_allocator, 0, NPU_MAX_HWTS_EVENT_ID, 0);
	cond_goto_error(ret != 0, hwts_event_list_init_failed, ret, -1,
		"npu dev id = %u hwts event list init failed\n", dev_id);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_MODEL]);
	ret = npu_id_allocator_create(id_allocator, 0, NPU_MAX_MODEL_ID, 0);
	cond_goto_error(ret != 0, model_list_init_failed, ret, -1,
		"npu dev id = %u model list init failed\n", dev_id);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_TASK]);
	ret = npu_id_allocator_create(id_allocator, 0, NPU_MAX_SINK_TASK_ID, 0);
	cond_goto_error(ret != 0, task_list_init_failed, ret, -1,
		"npu dev id = %u task list init failed\n", dev_id);

	ret = npu_task_set_init(dev_id);
	cond_goto_error(ret != 0, task_set_init_failed, ret, -1,
		"npu dev id = %u task set init failed\n", dev_id);

	ret = npu_svm_manager_init(dev_id);
	cond_goto_error(ret != 0, svm_manager_init_failed, ret, -1,
		"npu dev id = %u doorbell init failed\n", dev_id);

	ret = npu_mailbox_init(dev_id);
	cond_goto_error(ret != 0, mailbox_init_failed, ret, -1,
		"npu dev id = %u mailbox init failed\n", dev_id);

	ret = npu_dev_doorbell_init();
	cond_goto_error(ret != 0, doorbell_init_failed, ret, -1,
		"npu dev id = %u doorbell init failed\n", dev_id);

	return ret;
doorbell_init_failed:
	npu_mailbox_destroy(dev_id);
mailbox_init_failed:
	npu_svm_manager_destroy(dev_id);
svm_manager_init_failed:
	npu_task_set_destroy(dev_id);
task_set_init_failed:
	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_TASK]);
	npu_id_allocator_destroy(id_allocator);
task_list_init_failed:
	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_MODEL]);
	npu_id_allocator_destroy(id_allocator);
model_list_init_failed:
	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_HWTS_EVENT]);
	npu_id_allocator_destroy(id_allocator);
hwts_event_list_init_failed:
	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_EVENT]);
	npu_id_allocator_destroy(id_allocator);
event_list_init_failed:
	npu_cq_list_destroy(dev_id);
cq_list_init_failed:
	npu_sq_list_destroy(dev_id);
sq_list_init_failed:
	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_STREAM]);
	npu_id_allocator_destroy(id_allocator);
	if (NPU_MAX_SINK_LONG_STREAM_ID > 0) {
		id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_LONG_STREAM]);
		npu_id_allocator_destroy(id_allocator);
	}
sink_list_init_failed:
	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_STREAM]);
	npu_id_allocator_destroy(id_allocator);
	return ret;
}

static void npu_resource_list_destroy(struct npu_dev_ctx *dev_ctx)
{
	u8 dev_id;
	struct npu_id_allocator *id_allocator = NULL;

	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	dev_id = dev_ctx->devid;

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_MODEL]);
	npu_id_allocator_destroy(id_allocator);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_TASK]);
	npu_id_allocator_destroy(id_allocator);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_EVENT]);
	npu_id_allocator_destroy(id_allocator);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_HWTS_EVENT]);
	npu_id_allocator_destroy(id_allocator);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_STREAM]);
	npu_id_allocator_destroy(id_allocator);

	id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_STREAM]);
	npu_id_allocator_destroy(id_allocator);

	if (NPU_MAX_SINK_LONG_STREAM_ID > 0) {
		id_allocator = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_LONG_STREAM]);
		npu_id_allocator_destroy(id_allocator);
	}

	npu_cq_list_destroy(dev_id);
	npu_sq_list_destroy(dev_id);
	npu_svm_manager_destroy(dev_id);
	npu_mailbox_destroy(dev_id);
	npu_task_set_destroy(dev_id);
}

static void npu_synchronous_resource_init(struct npu_dev_ctx *dev_ctx)
{
	static char ws_name[NPU_WAKEUP_SIZE] = {0};

	spin_lock_init(&dev_ctx->spinlock);
	spin_lock_init(&dev_ctx->notify_spinlock);
	spin_lock_init(&dev_ctx->ts_spinlock);

	mutex_init(&dev_ctx->npu_power_up_off_mutex);
	mutex_init(&dev_ctx->npu_open_release_mutex);

	mutex_init(&dev_ctx->cq_mutex_t);
	mutex_init(&dev_ctx->notify_mutex_t);

	atomic_set(&dev_ctx->power_access, 1);
	atomic_set(&dev_ctx->power_success, 1);
	atomic_set(&dev_ctx->open_access, 1);
	atomic_set(&dev_ctx->open_success, 1);

	init_rwsem(&dev_ctx->pm.exception_lock);
	dev_ctx->pm.npu_exception_status = NPU_STATUS_NORMAL;
	mutex_init(&dev_ctx->pm.task_set_lock);

	snprintf_s(ws_name, NPU_WAKEUP_SIZE, NPU_WAKEUP_SIZE - 1,
		"npu_power_wakeup%d%c", dev_ctx->devid, '\0');
	wakeup_source_init(&dev_ctx->wakeup, ws_name);
}

static void npu_synchronous_resource_destroy(struct npu_dev_ctx *dev_ctx)
{
	wakeup_source_trash(&dev_ctx->wakeup);

	mutex_destroy(&dev_ctx->npu_power_up_off_mutex);
	mutex_destroy(&dev_ctx->npu_open_release_mutex);

	mutex_destroy(&dev_ctx->cq_mutex_t);
	mutex_destroy(&dev_ctx->notify_mutex_t);
	mutex_destroy(&dev_ctx->pm.task_set_lock);
}

void npu_vm_open(struct vm_area_struct *vma)
{
	cond_return_void(vma == NULL, "npu munmap vma is null\n");

	npu_drv_debug("npu munmap: vma=0x%lx, vm_start=0x%lx, vm_end=0x%lx\n",
		(uintptr_t)vma, vma->vm_start, vma->vm_end);
}

void npu_vm_close(struct vm_area_struct *vma)
{
	struct npu_vma_mmapping *npu_vma_map = NULL;
	struct npu_proc_ctx *proc_ctx = NULL;

	cond_return_void(vma == NULL, "npu munmap vma is null\n");

	npu_drv_debug("npu munmap: vma=0x%lx, vm_start=0x%lx, vm_end=0x%lx\n",
		(uintptr_t)vma, vma->vm_start, vma->vm_end);
	npu_vma_map = (struct npu_vma_mmapping *)vma->vm_private_data;
	cond_return_void(npu_vma_map == NULL,
		"npu mmap npu_vma_map is null\n");

	proc_ctx = (struct npu_proc_ctx *)npu_vma_map->proc_ctx;
	cond_return_void(proc_ctx == NULL, "proc_ctx is null\n");

	if (npu_vma_map->map_type > MAP_RESERVED &&
		npu_vma_map->map_type < MAP_MAX) {
		mutex_lock(&proc_ctx->map_mutex);
		list_del(&npu_vma_map->list);
		mutex_unlock(&proc_ctx->map_mutex);
	}
	kfree(npu_vma_map);
	vma->vm_private_data = NULL;
}

int npu_vm_split(struct vm_area_struct *vma, unsigned long addr)
{
	npu_drv_debug("npu munmap: vma=0x%lx, vm_start=0x%lx, vm_end=0x%lx addr\n",
		(uintptr_t)vma, vma->vm_start, vma->vm_end);
	return -1;
}

const struct vm_operations_struct npu_vm_ops = {
	.open  = npu_vm_open,
	.close = npu_vm_close,
	.split = npu_vm_split,
};

static int npu_map(struct file *filep, struct vm_area_struct *vma)
{
	unsigned long vm_pgoff;
	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_vma_mmapping *npu_vma_map = NULL;
	u32 map_type;
	u8 dev_id;
	int ret;

	if ((vma == NULL) || (filep == NULL)) {
		npu_drv_err("invalid vma or filep\n");
		return -EFAULT;
	}

	proc_ctx = (struct npu_proc_ctx *)filep->private_data;
	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is NULL\n");
		return -EFAULT;
	}

	npu_vma_map = (struct npu_vma_mmapping *)kzalloc(
		sizeof(struct npu_vma_mmapping), GFP_KERNEL);
	cond_return_error(npu_vma_map == NULL, -EINVAL,
		"alloc npu_vma_map fail\n");

	dev_id = proc_ctx->devid;
	vm_pgoff = vma->vm_pgoff;
	map_type = map_get_type(vm_pgoff);

	mutex_lock(&proc_ctx->map_mutex);
	npu_drv_warn("map_type = %d memory mmap start. vm_pgoff=0x%lx\n",
		map_type, vm_pgoff);
	switch (map_type) {
	case MAP_RESERVED:
		ret = -EINVAL;
		break;
	case MAP_L2_BUFF:
		ret = npu_map_l2_buff(filep, vma, dev_id);
		list_add(&npu_vma_map->list, &proc_ctx->l2_vma_list);
		break;
	default:
		npu_drv_err("map_type is error\n");
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
		npu_drv_err("map_type = %d memory mmap failed\n", map_type);

	return ret;
}

int npu_dev_open(struct npu_dev_ctx *dev_ctx)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;

	npu_drv_warn("enter\n");

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_ops failed\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->open_access, 1, 0) == 0) {
		npu_drv_err("npu dev %d has already open!\n", dev_ctx->devid);
		return -EBUSY;
	}

	ret = plat_info->adapter.pm_ops.npu_open(dev_ctx->devid);
	if (ret != 0) {
		atomic_inc(&dev_ctx->open_access);
		npu_drv_err("plat_ops npu_open failed\n");
		return ret;
	}

	npu_drv_warn("npu dev %d open success!\n", dev_ctx->devid);
	atomic_dec(&dev_ctx->open_success);

	dev_ctx->pid = current->tgid;

	return 0;
}

int npu_dev_release(struct npu_dev_ctx *dev_ctx)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;

	npu_drv_debug("enter\n");

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_ops failed\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->open_success, 0, 1) == 1) {
		npu_drv_err("npu dev %d has already release!\n", dev_ctx->devid);
		return -EBUSY;
	}

	ret = plat_info->adapter.pm_ops.npu_release(dev_ctx->devid);
	if (ret != 0) {
		atomic_dec(&dev_ctx->open_success);
		npu_drv_err("plat_ops npu_release failed\n");
		return ret;
	}

	atomic_inc(&dev_ctx->open_access);
	npu_drv_warn("npu dev %d release success!\n", dev_ctx->devid);

	return 0;
}

static int npu_open(struct inode *inode, struct file *filep)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_proc_ctx *proc_ctx = NULL;
	unsigned int minor = iminor(inode);
	struct npu_ts_cq_info *cq_info = NULL;
	// stub
	const u8 dev_id = 0;  // should get from manager info
	int ret;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return -EINVAL;
	}

	npu_drv_warn("start. minor = %d\n", minor);
	cond_return_error(minor >= NPU_MAX_DEVICE_NUM, -ENODEV,
		"invalid npu minor num, minor = %d\n", minor);

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODEV,
		"cur_dev_ctx %d is null\n", dev_id);

	cur_dev_ctx->will_powerdown = false;

	mutex_lock(&cur_dev_ctx->npu_open_release_mutex);

	ret = npu_dev_open(cur_dev_ctx);
	if (ret != 0) {
		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
		npu_drv_err("npu open ret %d failed\n", ret);
		return -EBUSY;
	}

	proc_ctx = kzalloc(sizeof(struct npu_proc_ctx), GFP_KERNEL);
	if (proc_ctx == NULL) {
		npu_drv_err("alloc memory for proc_ctx failed\n");
		goto alloc_proc_ctx_failed;
	}

	ret = npu_proc_ctx_init(proc_ctx);
	if (ret != 0) {
		npu_drv_err("init proc ctx failed\n");
		goto proc_ctx_init_failed;
	}

	proc_ctx->devid = cur_dev_ctx->devid;
	proc_ctx->pid = current->tgid;
	filep->private_data = (void *)proc_ctx;
	proc_ctx->filep = filep;

	// alloc cq for current process
	mutex_lock(&cur_dev_ctx->cq_mutex_t);
	cq_info = npu_proc_alloc_cq(proc_ctx);
	if (cq_info == NULL) {
		mutex_unlock(&cur_dev_ctx->cq_mutex_t);
		npu_drv_err("alloc persistent cq for proc_context failed\n");
		ret = -ENOMEM;
		goto proc_alloc_cq_failed;
	}
	npu_drv_debug("alloc persistent cq for proc_context success\n");
	mutex_unlock(&cur_dev_ctx->cq_mutex_t);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		ret = npu_proc_alloc_proc_ctx_sub(proc_ctx);
		if (ret != 0) {
			npu_drv_err("alloc prox_ctx_sub failed\n");
			goto proc_alloc_prox_ctx_sub_failed;
		}
	}

	// add proc_ctx to cur dev_ctx proc list
	(void)npu_add_proc_ctx(&proc_ctx->dev_ctx_list, dev_id);

	// bind proc_ctx to cq report int ctx
	npu_bind_proc_ctx_with_cq_int_ctx(proc_ctx);
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);

	npu_drv_debug("succeed\n");
	return 0;

proc_alloc_prox_ctx_sub_failed:
	(void)npu_proc_free_cq(proc_ctx);
proc_alloc_cq_failed:
	filep->private_data = NULL;
proc_ctx_init_failed:
	npu_proc_ctx_destroy(&proc_ctx);
alloc_proc_ctx_failed:
	(void)npu_dev_release(cur_dev_ctx);
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
	return ret;
}

static int npu_release(struct inode *inode, struct file *filep)
{
	int ret = 0;
	u8 dev_id;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	proc_ctx = (struct npu_proc_ctx *)filep->private_data;
	cond_return_error(proc_ctx == NULL, -EINVAL, "get proc_ctx fail\n");

	npu_drv_debug("start. minor = %d, dev_id = %d\n",
		iminor(inode), proc_ctx->devid);
	dev_id = proc_ctx->devid; // should get from manager info
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -EINVAL,
		"cur_dev_ctx %d is null\n", dev_id);

	/* device will be power down,so some message donot need to inform ts */
	cur_dev_ctx->will_powerdown = true;

	// callback char dev release rs before npu powerdown
	npu_release_npu_callback_proc(dev_id);

	npu_pm_delete_idle_timer(cur_dev_ctx);
	down_write(&cur_dev_ctx->pm.exception_lock);
	npu_task_set_clear(cur_dev_ctx);
	cur_dev_ctx->pm.npu_exception_status = NPU_STATUS_NORMAL;
	up_write(&cur_dev_ctx->pm.exception_lock);

	mutex_lock(&cur_dev_ctx->npu_open_release_mutex);

	// resource leak happened
	if (npu_is_proc_resource_leaks(proc_ctx) == true) {
		npu_resource_leak_print(proc_ctx);
		// to be done, should judge is ts alive to decide whether inform ts
		npu_recycle_npu_resources(proc_ctx);
		mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

		ret = npu_powerdown(proc_ctx, cur_dev_ctx);
		if (ret != -EBUSY && ret != 0)
			npu_drv_err("npu powerdown failed\n");

		mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
		ret = npu_dev_release(cur_dev_ctx);
		if (ret != 0)
			npu_drv_err("npu release failed\n");

		npu_proc_ctx_destroy(&proc_ctx);

		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
		return 0;
	}

	// normal case
	npu_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)npu_remove_proc_ctx(&proc_ctx->dev_ctx_list, proc_ctx->devid);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1)
		(void)npu_proc_free_proc_ctx_sub(proc_ctx);

	(void)npu_proc_free_cq(proc_ctx);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	ret = npu_powerdown(proc_ctx, cur_dev_ctx);
	if (ret != -EBUSY && ret != 0)
		npu_drv_err("npu powerdown failed\n");

	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	ret = npu_dev_release(cur_dev_ctx);
	if (ret != 0)
		npu_drv_err("npu release failed\n");

	npu_proc_ctx_destroy(&proc_ctx);
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);

	npu_drv_debug("success\n");
	return ret;
}

const struct file_operations npu_dev_fops = {
	.owner = THIS_MODULE,
	.open = npu_open,
	.release = npu_release,
	.unlocked_ioctl	= npu_ioctl,
	.mmap = npu_map,
};

static int npu_register_npu_chrdev_to_kernel(struct module *owner,
	struct npu_dev_ctx *dev_ctx, const struct file_operations *npu_fops)
{
	struct device *i_device = NULL;
	int ret;
	dev_t devno;

	devno = MKDEV(npu_major, dev_ctx->devid);

	dev_ctx->npu_cdev.owner = owner;
	cdev_init(&dev_ctx->npu_cdev, npu_fops);
	ret = cdev_add(&dev_ctx->npu_cdev, devno, NPU_MAX_DEVICE_NUM);
	if (ret != 0) {
		npu_drv_err("npu cdev_add error\n");
		return -1;
	}

	i_device = device_create(npu_class, NULL, devno, NULL, "npu%d",
		dev_ctx->devid);
	if (i_device == NULL) {
		npu_drv_err("device_create error\n");
		ret = -ENODEV;
		goto device_fail;
	}
	dev_ctx->npu_dev = i_device;
	return ret;
device_fail:
	cdev_del(&dev_ctx->npu_cdev);
	return ret;
}

static void npu_unregister_npu_chrdev_from_kernel(
	struct npu_dev_ctx *dev_ctx)
{
	cdev_del(&dev_ctx->npu_cdev);
	device_destroy(npu_class, MKDEV(npu_major, dev_ctx->devid));
}

/*
 * npu_drv_register - register a new devdrv device
 * @npu_info: devdrv device info
 * returns zero on success
 */
static int npu_drv_register(struct module *owner, u8 dev_id,
	const struct file_operations *npu_fops)
{
	struct npu_dev_ctx *dev_ctx = NULL;
	int ret;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	npu_drv_debug("dev %u started\n", dev_id);
	cond_return_error(dev_id >= NPU_DEV_NUM, -1,
		"illegal npu dev id = %u\n", dev_id);

	dev_ctx = kzalloc(sizeof(struct npu_dev_ctx), GFP_KERNEL);
	cond_return_error(dev_ctx == NULL, -ENOMEM,
		"kmalloc devid = %u dev_ctx failed\n", dev_id);

	set_dev_ctx_with_dev_id(dev_ctx, dev_id);
	dev_ctx->devid = dev_id;
	dev_ctx->pm.work_mode = 0;
	dev_ctx->pm.cur_subip_set = 0;
	dev_ctx->pm.npu_idle_time_out = NPU_IDLE_TIME_OUT_DEAFULT_VALUE;
	dev_ctx->pm.npu_task_time_out = NPU_TASK_TIME_OUT_DEAFULT_VALUE;
	atomic_set(&dev_ctx->pm.interframe_idle_manager.enable, 1);
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1)
		ret = npu_shm_v200_init(dev_id);
	else
		ret = npu_shm_init(dev_id);

	cond_goto_error(ret != 0, resource_list_init, ret, -ENODEV,
		"dev %d shm init failed\n", dev_id);

	ret = npu_resource_list_init(dev_ctx);
	cond_goto_error(ret != 0, resource_list_init, ret, -ENODEV,
		"npu dev id = %u resource list init failed\n", dev_id);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		ret = npu_dev_ctx_sub_init(dev_id);
		cond_goto_error(ret != 0, ctx_sub_init_failed, ret, -ENODEV,
			"npu dev id = %u resource list init failed\n", dev_id);
	}

	// init proc_ctx list
	INIT_LIST_HEAD(&dev_ctx->proc_ctx_list);
	INIT_LIST_HEAD(&dev_ctx->rubbish_context_list);
	npu_synchronous_resource_init(dev_ctx);

	cond_goto_error(npu_heart_beat_resource_init(dev_ctx),
		npu_chr_dev_register_failed, ret,
		-ENODEV, "npu_heart_beat_resource_init fail\n");

	cond_goto_error(npu_dfx_cqsq_init(dev_ctx), npu_heart_beat_init_failed,
		ret, -ENODEV, "npu_init_functional_cqsq fail\n");

	ret = npu_register_npu_chrdev_to_kernel(owner, dev_ctx, npu_fops);
	cond_goto_error(ret != 0, npu_chr_dev_init_dfx_cqsq_failed, ret, ret,
		"npu dev id = %u chrdev register failed\n", dev_id);

	dev_ctx->plat_info = plat_info;
	npu_pm_adapt_init(dev_ctx);
	npu_drv_debug("succeed\n");
	return 0;
npu_chr_dev_init_dfx_cqsq_failed:
	npu_destroy_dfx_cqsq(dev_ctx);
npu_heart_beat_init_failed:
	npu_heart_beat_resource_destroy(dev_ctx);
npu_chr_dev_register_failed:
	npu_synchronous_resource_destroy(dev_ctx);
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1)
		npu_dev_ctx_sub_destroy(dev_id);
ctx_sub_init_failed:
	npu_resource_list_destroy(dev_ctx);

resource_list_init:
	// maybe one of both alloc failed also need free,
	// judge every addr whether need free
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1)
		npu_shm_v200_destroy(dev_id);
	else
		npu_shm_destroy(dev_id);

	kfree(dev_ctx);
	dev_ctx = NULL;
	set_dev_ctx_with_dev_id(NULL, dev_id);

	return ret;
}

/*
 * npu_drv_unregister - unregister a devdrv device
 * @npu_info: devdrv device info
 * returns zero on success
 */
void npu_drv_unregister(u8 dev_id)
{
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return;
	}

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id = %d\n", dev_id);
		return;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return;
	};

	npu_unregister_npu_chrdev_from_kernel(dev_ctx);
	npu_destroy_dfx_cqsq(dev_ctx);
	npu_heart_beat_resource_destroy(dev_ctx);
	npu_synchronous_resource_destroy(dev_ctx);
	npu_resource_list_destroy(dev_ctx);
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		npu_dev_ctx_sub_destroy(dev_id);
		npu_shm_v200_destroy(dev_id);
	} else {
		npu_shm_destroy(dev_id);
	}

	kfree(dev_ctx);
	dev_ctx = NULL;
}

static char *npu_drv_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

int npu_devinit(struct module *owner, u8 dev_id)
{
	dev_t npu_dev = 0;
	int ret;

	npu_drv_warn("npu dev %u init start\n", dev_id);

	dev_ctx_array_init();

	ret = alloc_chrdev_region(&npu_dev, 0, NPU_MAX_DEVICE_NUM, "npu-cdev");
	if (ret != 0) {
		npu_drv_err("alloc npu chr dev region error\n");
		return ret;
	}

	npu_major = MAJOR(npu_dev);
	npu_class = class_create(owner, "npu-class");
	if (IS_ERR(npu_class)) {
		npu_drv_err("class_create error\n");
		ret = -EINVAL;
		goto class_fail;
	}
	npu_class->devnode = npu_drv_devnode;

	ret = npu_drv_register(owner, dev_id, &npu_dev_fops);
	if (ret != 0) {
		npu_drv_err("npu %d npu_drv_register failed\n", dev_id);
		ret = -EINVAL;
		goto npu_drv_register_fail;
	}

	npu_drv_warn("npu dev %d init succeed\n", dev_id);
	return ret;

npu_drv_register_fail:
	class_destroy(npu_class);
class_fail:
	unregister_chrdev_region(npu_dev, NPU_MAX_DEVICE_NUM);
	return ret;
}

void npu_devexit(u8 dev_id)
{
	npu_drv_unregister(dev_id);
	class_destroy(npu_class);
	unregister_chrdev_region(MKDEV(npu_major, 0), NPU_MAX_DEVICE_NUM);
}
