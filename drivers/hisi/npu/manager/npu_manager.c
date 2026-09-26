/*
 * npu_manager.c
 *
 * about npu manager
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
#include "npu_manager.h"

#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/atomic.h>
#include <dsm/dsm_pub.h>

#include "npu_proc_ctx.h"
#include "npu_manager_ioctl_services.h"
#include "npu_ioctl_services.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_common.h"
#include "npu_devinit.h"
#include "npu_log.h"
#include "npu_pm_framework.h"
#include "npu_recycle.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_mailbox_msg.h"
#include "npu_manager_common.h"
#include "npu_hwts.h"
#include "npu_svm.h"
#include "npu_adapter.h"

static unsigned int g_npu_manager_major;
static struct class *g_npu_manager_class;
static struct npu_manager_info *g_dev_manager_info;

static int (*npu_ioctl_call[NPU_MAX_CMD])(struct npu_proc_ctx *proc_ctx,
	unsigned long arg) = { NULL };
static int (*npu_feature_call[NPU_MAX_FEATURE_ID])(
	struct npu_proc_ctx *proc_ctx, uint32_t flag) = { NULL };

struct dsm_dev dev_npu = {
	.name = "dsm_ai",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};
struct dsm_client *npu_dsm_client;

static int npu_manager_open(struct inode *inode, struct file *filep)
{
	int ret;
	uint32_t pid;

	npu_drv_debug("start\n");
	ret = npu_insert_item_bypid(0, current->tgid, current->tgid);
	if (ret != 0) {
		npu_drv_err("npu_insert_item_bypid failed\n");
		return ret;
	}
	pid = (uint32_t)current->tgid;
	filep->private_data = (void *)(uintptr_t)pid;
	return 0;
}

static int npu_manager_release(struct inode *inode, struct file *filep)
{
	int ret;
	pid_t pid;

	pid = (pid_t)(uintptr_t)filep->private_data;
	npu_drv_debug("start\n");
	ret = npu_release_item_bypid(0, pid, pid);
	if (ret != 0) {
		npu_drv_err("npu_release_item_bypid failed\n");
		return ret;
	}

	return 0;
}

struct npu_manager_info *npu_get_manager_info(void)
{
	return g_dev_manager_info; /* dev_manager_info */
}
EXPORT_SYMBOL(npu_get_manager_info);

const struct file_operations npu_manager_fops = {
	.owner = THIS_MODULE,
	.open = npu_manager_open,
	.release = npu_manager_release,
	.unlocked_ioctl = npu_manager_ioctl,
};

static char *npu_manager_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

static int npu_register_manager_chrdev(dev_t npu_manager_dev, dev_t devno)
{
	struct device *i_device = NULL;
	int ret;

	ret = alloc_chrdev_region(&npu_manager_dev, 0, NPU_MAX_DEVICE_NUM,
		"npu_manager");
	if (ret != 0) {
		npu_drv_err("npu manager alloc_chrdev_region error\n");
		return -1;
	}

	g_dev_manager_info->cdev.owner = THIS_MODULE;
	g_npu_manager_major = MAJOR(npu_manager_dev);
	devno = MKDEV(g_npu_manager_major, 0);

	g_npu_manager_class = class_create(THIS_MODULE, "npu_manager");
	if (IS_ERR(g_npu_manager_class)) {
		npu_drv_err("npu_manager_class create error\n");
		ret = -EINVAL;
		goto class_fail;
	}

	g_npu_manager_class->devnode = npu_manager_devnode;

	cdev_init(&g_dev_manager_info->cdev, &npu_manager_fops);
	ret = cdev_add(&g_dev_manager_info->cdev, devno, NPU_MAX_DEVICE_NUM);
	if (ret != 0) {
		npu_drv_err("npu manager cdev_add error\n");
		goto cdev_setup_fail;
	}

	i_device = device_create(g_npu_manager_class, NULL, devno, NULL,
		"npu_manager");
	if (i_device == NULL) {
		npu_drv_err("npu manager device_create error\n");
		ret = -ENODEV;
		goto device_create_fail;
	}

	return ret;
device_create_fail:
	cdev_del(&g_dev_manager_info->cdev);
cdev_setup_fail:
	class_destroy(g_npu_manager_class);
class_fail:
	unregister_chrdev_region(npu_manager_dev, NPU_MAX_DEVICE_NUM);
	return ret;
}

static void npu_unregister_manager_chrdev(dev_t npu_manager_dev, dev_t devno)
{
	device_destroy(g_npu_manager_class, devno);
	cdev_del(&g_dev_manager_info->cdev);
	class_destroy(g_npu_manager_class);
	unregister_chrdev_region(npu_manager_dev, NPU_MAX_DEVICE_NUM);
}

static void npu_ioctl_call_init(void)
{
	int i;

	for (i = 0; i < NPU_MAX_CMD; i++)
		npu_ioctl_call[i] = NULL;

	npu_ioctl_call[_IOC_NR(NPU_ALLOC_STREAM_ID)] = npu_ioctl_alloc_stream;
	npu_ioctl_call[_IOC_NR(NPU_FREE_STREAM_ID)] = npu_ioctl_free_stream;
	npu_ioctl_call[_IOC_NR(NPU_GET_OCCUPY_STREAM_ID)] =
		npu_ioctl_get_occupy_stream_id;
	npu_ioctl_call[_IOC_NR(NPU_ALLOC_EVENT_ID)] = npu_ioctl_alloc_event;
	npu_ioctl_call[_IOC_NR(NPU_FREE_EVENT_ID)] = npu_ioctl_free_event;
	npu_ioctl_call[_IOC_NR(NPU_ALLOC_MODEL_ID)] = npu_ioctl_alloc_model;
	npu_ioctl_call[_IOC_NR(NPU_FREE_MODEL_ID)] = npu_ioctl_free_model;
	npu_ioctl_call[_IOC_NR(NPU_REQUEST_SEND)] = npu_ioctl_send_request;
	npu_ioctl_call[_IOC_NR(NPU_RESPONSE_RECEIVE)] = npu_ioctl_receive_response;
	npu_ioctl_call[_IOC_NR(NPU_ALLOC_TASK_ID)] = npu_ioctl_alloc_task;
	npu_ioctl_call[_IOC_NR(NPU_FREE_TASK_ID)] = npu_ioctl_free_task;
	npu_ioctl_call[_IOC_NR(NPU_GET_TS_TIMEOUT_ID)] = npu_ioctl_get_ts_timeout;
	npu_ioctl_call[_IOC_NR(NPU_ATTACH_SYSCACHE)] = npu_ioctl_attach_syscache;
	npu_ioctl_call[_IOC_NR(NPU_SVM_BIND_PID)] = npu_ioctl_svm_bind_pid;
	npu_ioctl_call[_IOC_NR(NPU_SVM_UNBIND_PID)] = npu_ioctl_svm_unbind_pid;
	npu_ioctl_call[_IOC_NR(NPU_CUSTOM_IOCTL)] = npu_ioctl_custom;
	npu_ioctl_call[_IOC_NR(NPU_ENTER_WORKMODE)] = npu_ioctl_enter_workwode;
	npu_ioctl_call[_IOC_NR(NPU_EXIT_WORKMODE)] = npu_ioctl_exit_workwode;
	npu_ioctl_call[_IOC_NR(NPU_SET_LIMIT)] = npu_ioctl_set_limit;
	npu_ioctl_call[_IOC_NR(NPU_ENABLE_FEATURE)] = npu_ioctl_enable_feature;
	npu_ioctl_call[_IOC_NR(NPU_DISABLE_FEATURE)] = npu_ioctl_disable_feature;
	npu_ioctl_call[_IOC_NR(NPU_SET_SC_PRIO)] = npu_ioctl_set_sc_prio;
	npu_ioctl_call[_IOC_NR(NPU_SWITCH_SC)] = npu_ioctl_switch_sc;
}

int npu_proc_npu_ioctl_call(struct npu_proc_ctx *proc_ctx,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	if (cmd < _IO(NPU_ID_MAGIC, 1) || cmd >= _IO(NPU_ID_MAGIC, NPU_MAX_CMD)) {
		npu_drv_err("parameter,arg = 0x%lx, cmd = %d\n", arg, cmd);
		return -EINVAL;
	}

	npu_drv_debug("IOC_NR = %d  cmd = %d\n", _IOC_NR(cmd), cmd);

	if (npu_ioctl_call[_IOC_NR(cmd)] == NULL) {
		npu_drv_err("invalid cmd = %d\n", cmd);
		return -EINVAL;
	}

	// porcess ioctl
	ret = npu_ioctl_call[_IOC_NR(cmd)](proc_ctx, arg);
	if (ret != 0) {
		npu_drv_err("process failed,arg = %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}

int npu_feature_enable(struct npu_proc_ctx *proc_ctx, uint32_t feature_id,
	uint32_t enable)
{
	int ret = 0;

	if (feature_id >= NPU_MAX_FEATURE_ID) {
		npu_drv_err("feature id %u is invalid\n", feature_id);
		return -EINVAL;
	}
	if (npu_feature_call[feature_id] == NULL) {
		npu_drv_err("npu feature call invalid feature id = %d\n", feature_id);
		return -EINVAL;
	}
	ret = npu_feature_call[feature_id](proc_ctx, enable);
	if (ret != 0) {
		npu_drv_err("feature id = %u, enable = %u failed\n", feature_id,
			enable);
		return -EINVAL;
	}

	return ret;
}

static void npu_feature_call_init(void)
{
	int i;

	for (i = 0; i < NPU_MAX_FEATURE_ID; i++)
		npu_feature_call[i] = NULL;

	npu_feature_call[NPU_INTERFRAME_FEATURE_ID] = npu_interframe_enable;
}

static int __init npu_manager_init(void)
{
	const dev_t npu_manager_dev = 0;
	const dev_t npu_manager_devno = 0;
	const u8 dev_id = 0;
	int ret;
	struct npu_platform_info *plat_info = NULL;

	npu_drv_warn("npu dev %d drv_manager_init start\n", dev_id);

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return -1;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -ENODEV,
		"npu_plat_get_info failed\n");

	/* bbox black box init */
	ret = npu_black_box_init();
	cond_return_error(ret != 0, ret,
		"Failed npu_black_box_init! ret = %d\n", ret);

	ret = npu_noc_register();
	if (ret != 0)
		npu_drv_err("Failed npu_noc_register ! ret = %d\n", ret);

	g_dev_manager_info = kzalloc(sizeof(struct npu_manager_info),
		GFP_KERNEL);
	cond_goto_error(g_dev_manager_info == NULL, alloc_manager_info_failed,
		ret, -ENOMEM, "kzalloc npu g_dev_manager_info failed\n");

	g_dev_manager_info->plat_info = NPU_MANAGER_DEVICE_ENV;
	ret = npu_register_manager_chrdev(npu_manager_dev, npu_manager_devno);
	cond_goto_error(ret != 0, register_manager_chrdev_failed, ret, -ENODEV,
		"register npu manager chrdev failed ret = %d\n", ret);

	npu_ioctl_call_init();
	npu_feature_call_init();

	// init npu powerup or powerdown register info
	npu_register_callbak_info_init();

	if (npu_dsm_client == NULL) {
		npu_dsm_client = dsm_register_client(&dev_npu);
		if (npu_dsm_client == NULL)
			npu_drv_err("dsm_register_client register fail\n");
	}
	// npu device resoure init
	ret = npu_devinit(THIS_MODULE, dev_id);
	cond_goto_error(ret != 0, npu_devinit_failed, ret, -ENODEV,
		"npu dev %d npu_devinit failed\n", dev_id);

	ret = npu_request_cq_report_irq_bh();
	cond_goto_error(ret != 0, request_cq_report_irq_failed, ret, ret,
		"npu_request_cq_report_irq_bh failed\n");

	npu_drv_warn("success\n");
	return ret;

request_cq_report_irq_failed:
npu_devinit_failed:
	npu_unregister_manager_chrdev(npu_manager_dev, npu_manager_devno);
register_manager_chrdev_failed:
	kfree(g_dev_manager_info);
	g_dev_manager_info = NULL;
alloc_manager_info_failed:
	(void)npu_black_box_exit();
	npu_drv_err("failed\n");
	return ret;
}

module_init(npu_manager_init);

static void __exit npu_manager_exit(void)
{
	dev_t npu_manager_dev = 0;
	dev_t npu_manager_devno = 0;
	const u8 dev_id = 0; // default npu 0 on lite plat

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return ;

	if (npu_dsm_client != NULL) {
		dsm_unregister_client(npu_dsm_client, &dev_npu);
		npu_dsm_client = NULL;
	}
	// free irq and kill tasklet
	(void)npu_free_cq_report_irq_bh();
	// dev resource unregister
	(void)npu_devexit(dev_id);
	// unregister npu manager char dev
	npu_unregister_manager_chrdev(npu_manager_dev, npu_manager_devno);
	// free mem
	if (g_dev_manager_info != NULL) {
		kfree(g_dev_manager_info);
		g_dev_manager_info = NULL;
	}

	/* bbox black box uninit */
	(void)npu_black_box_exit();
}

module_exit(npu_manager_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NPU driver");
MODULE_VERSION("V1.0");
