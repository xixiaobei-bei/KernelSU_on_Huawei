/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for communicating
 *              with dev/hwdps.
 * Create: 2020-06-16
 */

#include <huawei_platform/hwdps/hwdps_ioctl.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <securec.h>
#include <huawei_platform/hwdps/hwdps_error.h>
#include <huawei_platform/hwdps/hwdps_fs_hooks.h>
#include <huawei_platform/hwdps/hwdps_limits.h>
#include "inc/base/hwdps_utils.h"
#include "inc/data/hwdps_data.h"
#include "inc/fek/hwdps_fs_callbacks.h"
#include "inc/policy/hwdps_policy.h"
#include "inc/tee/hwdps_keyinfo.h"
#include "inc/tee/hwdps_adapter.h"
#include "inc/tee/hwdps_tee.h"
#include "inc/ioctl/hwdps_ioctl_impl.h"

#define HWDPS_FIRST_MINOR 0
#define HWDPS_MINOR_CNT 1
#define SYNC_PKG_CNT_MAX 200

static dev_t g_dev;
static struct cdev g_cdev;
static struct class *g_cdev_class;

/*
 * This function init tee environment
 * @return 0 for success
 */
static s32 handle_init_tee(struct hwdps_init_tee_t __user *udata)
{
	s32 ret = 0;
	struct hwdps_init_tee_t kdata = {0};

	if (!udata)
		return -EINVAL;
	if (copy_from_user(&kdata, udata,
		sizeof(struct hwdps_init_tee_t)) != 0) {
		hwdps_pr_err("Failed while copying from user space!\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}
	kdata.ret = HWDPS_SUCCESS;
	kdata.ret = hwdps_init_tee();
	if (kdata.ret != HWDPS_SUCCESS) {
		hwdps_pr_err("hwdps_init_tee failed!\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}

do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret,
		sizeof(hwdps_result_t)) != 0) {
		hwdps_pr_err("Failed while copying to user space!\n");
		if (ret == 0)
			ret = -EFAULT;
	}

	return ret;
}

/*
 * This function copy ce key desc to kernel
 * @param udata      [in] ce key desc from user space
 */
static s32 handle_init_user(struct hwdps_init_user_t __user *udata)
{
	s32 ret = 0;
	u8 *addr_key_desc = NULL;
	struct hwdps_init_user_t kdata = {0};

	if (!udata)
		return -EINVAL;
	if (copy_from_user(&kdata, udata,
		sizeof(struct hwdps_init_user_t)) != 0) {
		hwdps_pr_err("Failed while copying from user space!\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}
	kdata.ret = HWDPS_SUCCESS;
	if (kdata.key_desc_len != HWDPS_USER_KEY_DESC_MAX) {
		ret = -EINVAL;
		kdata.ret = HWDPS_ERR_INVALID_ARGS;
		goto do_copy_ret;
	}

	addr_key_desc = kdata.key_desc;

	kdata.key_desc = kzalloc(kdata.key_desc_len, GFP_KERNEL);
	if (!kdata.key_desc) {
		ret = -ENOMEM;
		kdata.ret = HWDPS_ERR_NO_MEMORY;
		goto do_copy_ret;
	}
	if (copy_from_user(kdata.key_desc, addr_key_desc,
		sizeof(u8) * kdata.key_desc_len) != 0) {
		hwdps_pr_err("Failed while copying to user space!\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_free_ret;
	}
	hwdps_init_user(&kdata);

do_free_ret:
	kzfree(kdata.key_desc);
	kdata.key_desc = NULL;

do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret,
		sizeof(hwdps_result_t)) != 0) {
		hwdps_pr_err("Failed while copying to user space!\n");
		if (ret == 0)
			ret = -EFAULT;
	}

	return ret;
}

static s32 handle_clear_key(struct hwdps_clear_key_t __user *udata)
{
	s32 ret;
	struct hwdps_clear_key_t kdata;

	(void)memset_s(&kdata, sizeof(kdata), 0, sizeof(kdata));
	if (!udata)
		return -EINVAL;
	if (copy_from_user(&kdata, udata,
		sizeof(struct hwdps_clear_key_t)) != 0) {
		hwdps_pr_err("Failed while copying policy from user space!\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}

	ret = kernel_clear_credential(kdata.ausn);
	if (ret != 0) {
		hwdps_pr_err("Failed clear_credential!\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}

	hwdps_pr_info("clear user(%llu) key\n", kdata.ausn);
do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwdps_result_t))) {
		hwdps_pr_err("Failed while copying to user space!\n");
		if (ret == 0)
			ret = -EFAULT;
	}
	return ret;
}

static void handle_install_package_init(struct hwdps_install_package_t *kdata,
	struct hwdps_install_package_t *kdata_store)
{
	(void)memset_s(kdata, sizeof(struct hwdps_install_package_t), 0,
		sizeof(struct hwdps_install_package_t));
	(void)memset_s(kdata_store, sizeof(struct hwdps_install_package_t), 0,
		sizeof(struct hwdps_install_package_t));
}

static s32 handle_install_package(struct hwdps_install_package_t __user *udata)
{
	s32 ret = 0;

	struct hwdps_install_package_t kdata;
	struct hwdps_install_package_t kdata_store_addr;

	if (!udata)
		return -EINVAL;

	handle_install_package_init(&kdata, &kdata_store_addr);

	if (copy_from_user(&kdata, udata,
		sizeof(kdata)) != 0 ||
		copy_from_user(&kdata_store_addr, udata,
		sizeof(kdata_store_addr)) != 0) {
		hwdps_pr_err("%s copying from user fail\n", __func__);
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}

	if (hwdps_utils_copy_package_info_from_user(&kdata.pinfo,
		&kdata_store_addr.pinfo, true) != 0) {
		hwdps_pr_err("%s copying from packaged fail\n", __func__);
		ret = -EINVAL;
		kdata.ret = HWDPS_ERR_INVALID_ARGS;
		goto do_copy_ret;
	}

	hwdps_install_package(&kdata);
	hwdps_utils_free_package_info(&kdata.pinfo);

do_copy_ret:
	if (copy_to_user(
		&udata->ret, &kdata.ret, sizeof(hwdps_result_t)) != 0) {
		hwdps_pr_err("%s copying to user fail\n", __func__);
		if (ret == 0)
			ret = -EFAULT;
	}

	return ret;
}

static bool handle_uninstall_package_init(
	struct hwdps_uninstall_package_t *kdata,
	struct hwdps_uninstall_package_t *kdata_store)
{
	if (memset_s(kdata, sizeof(struct hwdps_uninstall_package_t), 0,
		sizeof(struct hwdps_uninstall_package_t)) != EOK)
		return false;
	if (memset_s(kdata_store, sizeof(struct hwdps_uninstall_package_t), 0,
		sizeof(struct hwdps_uninstall_package_t)) != EOK)
		return false;
	return true;
}

static s32 handle_uninstall_package(
	struct hwdps_uninstall_package_t __user *udata)
{
	s32 ret = 0;
	struct hwdps_uninstall_package_t kdata;
	struct hwdps_uninstall_package_t kdata_store_addr;

	if (!udata)
		return -EINVAL;
	if (!handle_uninstall_package_init(&kdata, &kdata_store_addr))
		return -EINVAL;
	if (copy_from_user(&kdata, udata,
		sizeof(struct hwdps_uninstall_package_t)) != 0 ||
		copy_from_user(&kdata_store_addr, udata,
		sizeof(struct hwdps_uninstall_package_t)) != 0) {
		hwdps_pr_err("Failed while copying from user space\n");
		ret = -EFAULT;
		kdata.ret = HWDPS_ERR_INTERNAL;
		goto do_copy_ret;
	}
	if (hwdps_utils_copy_package_info_from_user(&kdata.pinfo,
		&kdata_store_addr.pinfo, true) != 0) {
		ret = -EINVAL;
		kdata.ret = HWDPS_ERR_INVALID_ARGS;
		goto do_copy_ret;
	}

	hwdps_uninstall_package(&kdata);
	hwdps_utils_free_package_info(&kdata.pinfo);
do_copy_ret:
	if (copy_to_user(
		&udata->ret, &kdata.ret, sizeof(hwdps_result_t)) != 0) {
		hwdps_pr_err("Failed while copying to user space\n");
		if (ret == 0)
			ret = -EFAULT;
	}

	return ret;
}

static void free_kdata_packages(
	const struct hwdps_sync_packages_t *kdata)
{
	u32 i;

	if (kdata->packages) {
		for (i = 0; i < kdata->package_count; i++)
			hwdps_utils_free_package_info(&kdata->packages[i]);
		kzfree(kdata->packages);
	}
}

static s32 copy_sync_pkgs_level1(struct hwdps_sync_packages_t *kdata,
	struct hwdps_sync_packages_t __user *udata,
	struct hwdps_package_info_t **packages_user_addr)
{
	s32 ret = 0;

	if (memset_s(kdata, sizeof(*kdata), 0, sizeof(*kdata)) != EOK) {
		kdata->ret = HWDPS_ERR_INTERNAL;
		kdata->package_count = 0;
		ret = -EFAULT;
		goto out;
	}
	if (copy_from_user(kdata, udata,
		sizeof(struct hwdps_sync_packages_t)) != 0) {
		hwdps_pr_err("Failed while copying from user space\n");
		kdata->ret = HWDPS_ERR_INTERNAL;
		kdata->package_count = 0;
		ret = -EFAULT;
		goto out;
	}
	if ((kdata->package_count <= 0) ||
		(kdata->package_count > SYNC_PKG_CNT_MAX)) {
		kdata->ret = HWDPS_ERR_INVALID_ARGS;
		kdata->package_count = 0;
		ret = -EFAULT;
		goto out;
	}
	*packages_user_addr = kdata->packages;
out:
	kdata->packages = NULL;
	return ret;
}

static s32 copy_sync_pkgs_level2(
	struct hwdps_package_info_t **packages_transform,
	struct hwdps_package_info_t *packages_user_addr,
	struct hwdps_sync_packages_t *kdata)
{
	 /* outside free */
	*packages_transform = kcalloc(kdata->package_count,
		sizeof(struct hwdps_package_info_t), GFP_KERNEL);
	if (!(*packages_transform)) {
		kdata->ret = HWDPS_ERR_INVALID_ARGS;
		kdata->package_count = 0;
		return -ENOMEM;
	}
	if (copy_from_user(*packages_transform, packages_user_addr,
		sizeof(struct hwdps_package_info_t) *
		kdata->package_count) != 0) {
		hwdps_pr_err("Failed copying addr_pkgs from user space\n");
		kdata->ret = HWDPS_ERR_INVALID_ARGS;
		kdata->package_count = 0;
		return -EFAULT;
	}
	return 0;
}

static s32 copy_sync_pkgs_level3(struct hwdps_sync_packages_t *kdata,
	struct hwdps_package_info_t *packages_transform)
{
	s32 ret;

	kdata->packages = kcalloc(kdata->package_count,
		sizeof(struct hwdps_package_info_t), GFP_KERNEL);
	if (!kdata->packages) {
		kdata->ret = HWDPS_ERR_NO_MEMORY;
		kdata->package_count = 0;
		return -ENOMEM;
	}
	ret = hwdps_utils_copy_packages_from_user(kdata->packages,
		packages_transform, kdata->package_count);
	if (ret != 0) {
		if (ret == -ENOMEM)
			kdata->ret = HWDPS_ERR_NO_MEMORY;
		else if (ret == -EINVAL)
			kdata->ret = HWDPS_ERR_INVALID_ARGS;
		else
			kdata->ret = HWDPS_ERR_INTERNAL;
	}
	return ret;
}

static s32 handle_sync_installed_packages(
	struct hwdps_sync_packages_t __user *udata)
{
	s32 ret;
	struct hwdps_sync_packages_t kdata = {0};
	struct hwdps_package_info_t *packages_user_addr = NULL;
	struct hwdps_package_info_t *packages_transform = NULL;

	hwdps_pr_info("%s enter\n", __func__);

	if (!udata)
		return -EINVAL;
	/* copy 1st level data */
	ret = copy_sync_pkgs_level1(&kdata, udata, &packages_user_addr);
	if (ret != 0)
		goto do_copy_ret;
	/* copy 2nd level data */
	ret = copy_sync_pkgs_level2(&packages_transform, packages_user_addr,
		&kdata);
	if (ret != 0)
		goto do_free_pkg;
	/* copy 3rd level data */
	ret = copy_sync_pkgs_level3(&kdata, packages_transform);
	if (ret != 0)
		goto do_free_pkg;
	hwdps_sync_installed_packages(&kdata);

do_free_pkg:
	free_kdata_packages(&kdata);
	kfree(packages_transform);
do_copy_ret:
	if (copy_to_user(
		&udata->ret, &kdata.ret, sizeof(hwdps_result_t)) != 0) {
		hwdps_pr_err("Failed while copying to user space\n");
		ret = -EFAULT;
	}
	return ret;
}

/*
 * we have to use long instead of s64 to avoid warnings
 */
static long hwdps_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	s32 ret;

	(void)fd;
	switch (cmd) {
	case HWDPS_INIT_TEE:
		ret = handle_init_tee((struct hwdps_init_tee_t *)arg);
		break;
	case HWDPS_INITIALIZE_USER:
		ret = handle_init_user((struct hwdps_init_user_t *)arg);
		break;
	case HWDPS_CLEAR_KEY:
		ret = handle_clear_key((struct hwdps_clear_key_t *)arg);
		break;
	case HWDPS_INSTALL_PACKAGE:
		ret = handle_install_package(
			(struct hwdps_install_package_t *)arg);
		break;
	case HWDPS_UNINSTALL_PACKAGE:
		ret = handle_uninstall_package(
			(struct hwdps_uninstall_package_t *)arg);
		break;
	case HWDPS_SYNC_INSTALLED_PACKAGES:
		ret = handle_sync_installed_packages(
			(struct hwdps_sync_packages_t *)arg);
		break;
	default:
		hwdps_pr_warn("Invalid command: %u\n", cmd);
		ret = -EINVAL;
	}
	return (long)ret;
}

static const struct file_operations G_HWDPS_FOPS = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hwdps_ioctl
};

static s32 __init hwdps_init(void)
{
	s32 ret;
	struct device *dev_ret = NULL;

	hwdps_pr_info("Installing %s\n", HWDPS_DEVICE_NAME);
	ret = alloc_chrdev_region(&g_dev, HWDPS_FIRST_MINOR,
		HWDPS_MINOR_CNT, HWDPS_DEVICE_NAME);
	if (ret < 0)
		return ret;

	cdev_init(&g_cdev, &G_HWDPS_FOPS);
	ret = cdev_add(&g_cdev, g_dev, HWDPS_MINOR_CNT);
	if (ret < 0)
		return ret;

	g_cdev_class = class_create(THIS_MODULE, HWDPS_DEVICE_NAME);
	if (IS_ERR(g_cdev_class)) {
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWDPS_MINOR_CNT);
		return PTR_ERR(g_cdev_class);
	}

	dev_ret = device_create(g_cdev_class, NULL, g_dev, NULL,
		HWDPS_DEVICE_NAME);
	if (IS_ERR(dev_ret)) {
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWDPS_MINOR_CNT);
		return PTR_ERR(dev_ret);
	}
	hwdps_data_init();
	hwdps_register_fs_callbacks_proxy();
	init_list();
	if (!init_hwdps_work_queue()) {
		hwdps_pr_err("create_singlethread_workqueue failed");
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWDPS_MINOR_CNT);
		purge_phase1_key();
		return -EINVAL;
	}
	hwdps_pr_info("%s installed\n", HWDPS_DEVICE_NAME);
	return 0;
}

static void __exit hwdps_exit(void)
{
	destory_hwdps_work_queue();
	purge_phase1_key();
	hwdps_data_deinit();
	hwdps_unregister_fs_callbacks_proxy();
	device_destroy(g_cdev_class, g_dev);
	class_destroy(g_cdev_class);
	cdev_del(&g_cdev);
	unregister_chrdev_region(g_dev, HWDPS_MINOR_CNT);
}

module_init(hwdps_init);
module_exit(hwdps_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei, 2020");
MODULE_DESCRIPTION("Huawei Data Protect Service");
MODULE_VERSION("0.1");
