
#include "huawei_platform/hwsdp/hwsdp_ioctl.h"
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <securec.h>

#include "inc/hwsdp_ioctl_teec.h"
#include "inc/hwsdp_utils.h"

#define HWSDP_FIRST_MINOR 0
#define HWSDP_MINOR_CNT 1
#define HWSDP_DEVICE_IOCTL "hwsdp_ioctl"

/* device name */
#ifndef HWSDP_DEVICE_NAME
#define HWSDP_DEVICE_NAME "hwsdp"
#endif

static dev_t g_dev;
static struct cdev g_cdev;
static struct class *g_cdev_class;

/*
 * This function init tee environment
 * @return: success is 0, else -EINVAL; teec operation return value is set to udata->ret
 */
static s32 handle_init_tee(hwsdp_ioctl_data_t __user *udata)
{
	s32 ret = 0;
	hwsdp_ioctl_data_t kdata = {0};

	if (udata == NULL) {
		hwsdp_log_err("User buffer is invalid!");
		return -EINVAL;
	}
	ret = copy_from_user(&kdata, udata, sizeof(hwsdp_ioctl_data_t));
	if (ret != 0) {
		hwsdp_log_err("handle_init_tee, failture in copying from user space!");
		return -EFAULT;
	}
	kdata.ret = hwsdp_init_tee();

	ret = copy_to_user(udata, &kdata, sizeof(hwsdp_ioctl_data_t));
	if (ret != 0) {
		hwsdp_log_err("handle_init_tee, failure in copying to user space!");
		ret = -EFAULT;
	}

	return ret;
}

/*
 * This function invokes teec command with tee operation
 * @param udata [in] teec command with tee operation
 * @return: success is 0, else -EINVAL; teec operation return value is set to udata->ret
 */
static s32 handle_teec_ops(hwsdp_ioctl_data_t __user *udata)
{
	s32 ret = 0;
	hwsdp_ioctl_data_t kdata = {0};

	if (udata == NULL)
		return -EINVAL;

	ret = copy_from_user(&kdata, udata, sizeof(hwsdp_ioctl_data_t));
	if (ret != 0) {
		hwsdp_log_err("handle_teec_ops, failure in copying from user space!");
		return -EFAULT;
	}

	kdata.ret = hwsdp_teec_ops(&kdata);

	ret = copy_to_user(udata, &kdata, sizeof(hwsdp_ioctl_data_t));
	if (ret != 0) {
		hwsdp_log_err("handle_teec_ops, failure in copying to user space!");
		ret = -EFAULT;
	}

	return ret;
}

static void show_ioctl_cmd_info(u32 cmd)
{
	switch (cmd) {
	case HWSDP_CMD_INIT_TEE:
		hwsdp_log_info("hwsdp_ioctl HWSDP_CMD_INIT_TEE");
		break;
	case HWSDP_CMD_TEEC_OPS:
		hwsdp_log_info("hwsdp_ioctl HWSDP_CMD_TEEC_OPS");
		break;
	default:
		hwsdp_log_info("hwsdp_ioctl unknow CMD");
	}
	return;
}

/* we have to use long instead of s64 to avoid warnings, but i promise nowhere else */
static long hwsdp_ioctl(struct file *fptr, unsigned int cmd, unsigned long arg)
{
	s32 ret;
	show_ioctl_cmd_info(cmd);

	switch (cmd) {
	case HWSDP_CMD_INIT_TEE:
		ret = handle_init_tee((hwsdp_ioctl_data_t *)(uintptr_t)arg);
		break;
	case HWSDP_CMD_TEEC_OPS:
		ret = handle_teec_ops((hwsdp_ioctl_data_t *)(uintptr_t)arg);
		break;
	default:
		hwsdp_log_warn("Invalid command: %d", cmd);
		ret = -EINVAL;
	}
	(void)fptr;
	return (long)ret;
}

static const struct file_operations g_hwsdp_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hwsdp_ioctl
};

static s32 __init hwsdp_init(void)
{
	s32 ret;
	struct device *dev_ret = NULL;

	hwsdp_log_info("Installing "HWSDP_DEVICE_NAME" ...");
	ret = alloc_chrdev_region(&g_dev, HWSDP_FIRST_MINOR,
		HWSDP_MINOR_CNT, HWSDP_DEVICE_NAME);
	if (ret < 0) {
		hwsdp_log_err("hwsdp_init alloc_chrdev_region failed...");
		return ret;
	}

	cdev_init(&g_cdev, &g_hwsdp_fops);
	g_cdev.owner = THIS_MODULE;
	ret = cdev_add(&g_cdev, g_dev, HWSDP_MINOR_CNT);
	if (ret < 0) {
		hwsdp_log_err("hwsdp_init cdev_add failed...");
		return ret;
	}

	g_cdev_class = class_create(THIS_MODULE, HWSDP_DEVICE_NAME);
	if (IS_ERR(g_cdev_class)) {
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWSDP_MINOR_CNT);
		hwsdp_log_err("hwsdp_init class_create failed...");
		return PTR_ERR(g_cdev_class);
	}

	dev_ret = device_create(g_cdev_class, NULL, g_dev, NULL, HWSDP_DEVICE_NAME);
	if (IS_ERR(dev_ret)) {
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWSDP_MINOR_CNT);
		return PTR_ERR(dev_ret);
	}
	ret = init_hwsdp_work_queue();
	if (ret != HWSDP_IOCTL_SUCCESS) {
		hwsdp_log_err("init_hwsdp_work_queue failed...");
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWSDP_MINOR_CNT);
		return -EINVAL;
	}
	hwsdp_log_info(HWSDP_DEVICE_NAME" installed...");
	return 0;
}

static void __exit hwsdp_exit(void)
{
	destory_hwsdp_work_resource();
	device_destroy(g_cdev_class, g_dev);
	class_destroy(g_cdev_class);
	cdev_del(&g_cdev);
	unregister_chrdev_region(g_dev, HWSDP_MINOR_CNT);
	return;
}

module_init(hwsdp_init);
module_exit(hwsdp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei, 2020");
MODULE_DESCRIPTION("Huawei Sensitive Data Protection Module");
MODULE_VERSION("0.1");

