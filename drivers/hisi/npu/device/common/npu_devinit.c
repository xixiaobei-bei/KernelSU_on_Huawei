/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
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
#include  <linux/mm.h>
#include  <linux/list.h>
#include <linux/wakelock.h>
#include <linux/atomic.h>
#include <linux/hisi/hisi_svm.h>

#include "npu_shm.h"
#include "npu_stream.h"
#include "npu_sink_stream.h"
#include "npu_mailbox.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "drv_log.h"
#include "npu_pm.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_doorbell.h"
#include "npu_dfx_cq.h"
#include "npu_heart_beat.h"

static unsigned int npu_major;
static struct class *npu_class = NULL;

static int devdrv_resource_list_init(u8 dev_id)
{
	int ret = 0;
	ret = devdrv_stream_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %u stream list init failed\n",
			       dev_id);
		return -1;
	}

	ret = devdrv_sink_stream_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %u sink stream list init failed\n",
			       dev_id);
		goto sink_list_init_failed;
	}

	ret = devdrv_sq_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d sq list init failed\n", dev_id);
		ret = -1;
		goto sq_list_init_failed;
	}

	ret = devdrv_cq_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d cq list init failed\n", dev_id);
		ret = -1;
		goto cq_list_init_failed;
	}

	ret = devdrv_event_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d event list init failed\n",
			       dev_id);
		ret = -1;
		goto event_list_init_failed;
	}

	ret = devdrv_model_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d model list init failed\n",
			       dev_id);
		ret = -1;
		goto model_list_init_failed;
	}

	ret = devdrv_task_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d task list init failed\n",
			       dev_id);
		ret = -1;
		goto task_list_init_failed;
	}

	ret = devdrv_mailbox_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d mailbox init failed\n", dev_id);
		ret = -1;
		goto mailbox_init_failed;
	}

	ret = devdrv_dev_doorbell_init();
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %d doorbell init failed\n",dev_id);
		ret = -1;
		goto doorbell_init_failed;
	}
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

	mutex_init(&dev_ctx->npu_wake_lock_mutex);
	mutex_init(&dev_ctx->npu_power_up_off_mutex);
	mutex_init(&dev_ctx->npu_open_release_mutex);

	mutex_init(&dev_ctx->cq_mutex_t);
	mutex_init(&dev_ctx->stream_mutex_t);
	mutex_init(&dev_ctx->event_mutex_t);
	mutex_init(&dev_ctx->model_mutex_t);
	mutex_init(&dev_ctx->notify_mutex_t);

	atomic_set(&dev_ctx->power_access, 1);
	atomic_set(&dev_ctx->power_success, 1);
	atomic_set(&dev_ctx->open_access, 1);
	atomic_set(&dev_ctx->open_success, 1);

	snprintf(dev_ctx->wakelock_name, DEVDRV_WAKELOCK_SIZE,
			"npu_power_wakelock%d%c", dev_ctx->devid, '\0');
	wake_lock_init(&dev_ctx->wakelock, WAKE_LOCK_SUSPEND,
					dev_ctx->wakelock_name);
}

static void devdrv_synchronous_resource_destroy(struct devdrv_dev_ctx *dev_ctx)
{
	wake_lock_destroy(&dev_ctx->wakelock);

	mutex_destroy(&dev_ctx->npu_wake_lock_mutex);
	mutex_destroy(&dev_ctx->npu_power_up_off_mutex);
	mutex_destroy(&dev_ctx->npu_open_release_mutex);

	mutex_destroy(&dev_ctx->cq_mutex_t);
	mutex_destroy(&dev_ctx->stream_mutex_t);
	mutex_destroy(&dev_ctx->event_mutex_t);
	mutex_destroy(&dev_ctx->model_mutex_t);
	mutex_destroy(&dev_ctx->notify_mutex_t);
}

static int devdrv_register_npu_chrdev_to_kernel(struct module *owner,
						struct devdrv_dev_ctx *dev_ctx,
						const struct file_operations *devdrv_fops)
{
	struct device *i_device = NULL;
	int ret = 0;
	dev_t devno;

	devno = MKDEV(npu_major, dev_ctx->devid);

	dev_ctx->devdrv_cdev.owner = owner;
	cdev_init(&dev_ctx->devdrv_cdev, devdrv_fops);
	ret = cdev_add(&dev_ctx->devdrv_cdev, devno, DEVDRV_MAX_DAVINCI_NUM);
	if (ret != 0) {
		devdrv_drv_err("npu cdev_add error.\n");
		return -1;
	}

	i_device = device_create(npu_class, NULL, devno, NULL,
				 "davinci%d", dev_ctx->devid);
	if (i_device == NULL) {
		devdrv_drv_err("device_create error.\n");
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

/**
* devdrv_drv_register - register a new devdrv device
* @devdrv_info: devdrv device info
*
* returns zero on success
*/
static int devdrv_drv_register(struct module *owner,
								u8 dev_id,
								const struct file_operations *devdrv_fops)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	int ret = 0;

	devdrv_drv_debug("dev %u devdrv_drv_register started\n", dev_id);

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %u\n", dev_id);
		return -1;
	}

	dev_ctx = kzalloc(sizeof(struct devdrv_dev_ctx), GFP_KERNEL);
	if (dev_ctx == NULL) {
		devdrv_drv_err("kmalloc devid = %u dev_ctx failed\n", dev_id);
		return -ENOMEM;
	}

	set_dev_ctx_with_dev_id(dev_ctx, dev_id);
	dev_ctx->devid = dev_id;

	ret = devdrv_shm_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("dev %d shm init failed\n", dev_id);
		ret = -ENODEV;
		goto shm_init_fail;
	}

	ret = devdrv_resource_list_init(dev_id);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %u resource list init failed\n",
			       dev_id);
		ret = -ENODEV;
		goto resource_list_init;
	}
	// init proc_ctx list
	INIT_LIST_HEAD(&dev_ctx->proc_ctx_list);
	INIT_LIST_HEAD(&dev_ctx->rubbish_context_list);
	devdrv_synchronous_resource_init(dev_ctx);

	devdrv_heart_beat_resource_init(dev_ctx);

	if (devdrv_dfx_cqsq_init(dev_ctx)) {
		devdrv_drv_err("devdrv_init_functional_cqsq fail.\n");
		ret = -ENODEV;
		goto npu_chr_dev_register_failed;
	}

	ret = devdrv_register_npu_chrdev_to_kernel(owner, dev_ctx, devdrv_fops);
	if (ret != 0) {
		devdrv_drv_err("npu dev id = %u chrdev register failed \n",
			       dev_id);
		goto npu_chr_dev_init_dfx_cqsq_failed;
	}

	devdrv_drv_debug("devdrv_drv_register succeed\n");

	return 0;

npu_chr_dev_init_dfx_cqsq_failed:
	devdrv_destroy_dfx_cqsq(dev_ctx);
npu_chr_dev_register_failed:
	devdrv_heart_beat_resource_destroy(dev_ctx);
	devdrv_synchronous_resource_destroy(dev_ctx);
	devdrv_resource_list_destroy(dev_id);
resource_list_init:
	devdrv_shm_destroy(dev_id);
shm_init_fail:
	kfree(dev_ctx);
	dev_ctx = NULL;
	set_dev_ctx_with_dev_id(NULL, dev_id);
	return ret;
}

/**
* devdrv_drv_unregister - unregister a devdrv device
* @devdrv_info: devdrv device info
*
* returns zero on success
*/
void devdrv_drv_unregister(u8 dev_id)
{

	struct devdrv_dev_ctx *dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %d\n", dev_id);
		return;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return;
	};

	devdrv_unregister_npu_chrdev_from_kernel(dev_ctx);
	devdrv_destroy_dfx_cqsq(dev_ctx);
	devdrv_heart_beat_resource_destroy(dev_ctx);
	devdrv_synchronous_resource_destroy(dev_ctx);
	devdrv_resource_list_destroy(dev_id);
	devdrv_shm_destroy(dev_id);

	kfree(dev_ctx);
	dev_ctx = NULL;
}

static char *devdrv_drv_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

int devdrv_devinit(struct module *owner, u8 dev_id,
				const struct file_operations *devdrv_fops)
{
	dev_t npu_dev = 0;
	int ret = 0;

	devdrv_drv_debug("npu dev %u init start\n", dev_id);

	dev_ctx_array_init();

	ret = alloc_chrdev_region(&npu_dev, 0, DEVDRV_MAX_DAVINCI_NUM,
				"npu-cdev");
	if (ret != 0) {
		devdrv_drv_err("alloc npu chr dev region error.\n");
		return ret;
	}

	npu_major = MAJOR(npu_dev);
	npu_class = class_create(owner, "npu-class");
	if (IS_ERR(npu_class)) {
		devdrv_drv_err("class_create error.\n");
		ret = -EINVAL;
		goto class_fail;
	}
	npu_class->devnode = devdrv_drv_devnode;

	ret = devdrv_drv_register(owner, dev_id, devdrv_fops);
	if (ret != 0) {
		devdrv_drv_err("npu %d devdrv_drv_register failed \n", dev_id);
		ret = -EINVAL;
		goto devdrv_drv_register_fail;
	}

	devdrv_drv_debug("npu dev %d init succeed \n", dev_id);
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
