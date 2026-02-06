/*
 *               (C) COPYRIGHT 2014 - 2016 SYNOPSYS, INC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "hdcp_driver.h"
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include "esm/host_lib_driver_linux_if.h"

#define ESM_DEVICE_MAJOR 58

#define SOFT_AUTH_ENABLE 0

#define ESM_DEVICE_NAME "esm"
#define ESM_DEVICE_CLASS "elliptic"

/* Linux device, class and range */
static int g_device_created;
static struct device *g_device;
static int g_device_range_registered;
static int g_device_class_created;
static struct class *g_device_class;
static struct mutex g_hdcp_lock;

static long cmd_hdcp_common_cmd(struct hdcp_common_cmd_ioctl *request)
{
	long ret;
	struct hdcp_common_cmd_ioctl krequest = {0};

	if (request == NULL) {
		DPU_FB_ERR("%s request is null pointer\n", HDCP_TAG);
		return -1;
	}

	ret = copy_from_user(&krequest, request, sizeof(struct hdcp_common_cmd_ioctl));
	if (ret) {
		DPU_FB_ERR("%s copy_from_user failed!ret = %ld\n", HDCP_TAG, ret);
		krequest.returned_status = ret;
		goto Exit;
	}
	krequest.returned_status = HDCP_DRIVER_SUCCESS;
	switch (krequest.cmd) {
	case AUTH_START:
		hdcp_set_counter(HDCP_CNT_SET);
		break;
	case IRQ_IN:
		hdcp_set_counter(HDCP_CNT_INCREASE);
		break;
	case AUTH_END:
		hdcp_set_counter(HDCP_CNT_RESET);
		break;
	case IRQ_OUT:
		hdcp_set_counter(HDCP_CNT_DECREASE);
		break;
	case SOFT_AUTH_EN:
		krequest.soft_auth_enable = SOFT_AUTH_ENABLE;
		DPU_FB_INFO("%s soft_auth_en = %u\n", HDCP_TAG, krequest.soft_auth_enable);
		break;
	default:
		DPU_FB_ERR("%s hdcp common cmd, unknown cmd\n", HDCP_TAG);
		krequest.returned_status = HDCP_DRIVER_INVALID_PARAM;
		break;
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(struct hdcp_common_cmd_ioctl));
	if (ret) {
		DPU_FB_ERR("%s copy_to_user failed!ret = %ld\n", HDCP_TAG, ret);
		return ret;
	}

	return 0;
}

/* The device has been opened */
static int device_open(struct inode *inode, struct file *filp)
{
	DPU_FB_INFO("%s Device opened\n", HDCP_TAG);
	if (filp == NULL) {
		DPU_FB_ERR("%s filp is null pointer\n", HDCP_TAG);
		return -1;
	}

	filp->private_data = NULL;
	return 0;
}

/* The device has been closed */
static int device_release(struct inode *inode, struct file *filp)
{
	DPU_FB_INFO("%s Device released\n", HDCP_TAG);
	return 0;
}

static long device_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	long ret = HDCP_DRIVER_IOCTL_FAILED;
	bool is_esm_cmd = false;

	if ((f == 0) || (arg == 0))
		return ret;

	mutex_lock(&g_hdcp_lock);
	switch (cmd) {
	case HDCP_GENERAL_CMD:
		ret = cmd_hdcp_common_cmd(
			(struct hdcp_common_cmd_ioctl *)(uintptr_t)arg);
		break;
	default:
		is_esm_cmd = true;
		break;
	}
	if (is_esm_cmd)
		ret = esm_ioctl(f, cmd, arg);
	mutex_unlock(&g_hdcp_lock);

	return ret;
}


static const struct file_operations device_file_operations = {
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
	.owner = THIS_MODULE,
};


/* Creates the device required to interface with the HLD driver */
static int create_device(void)
{
	DPU_FB_INFO("%s Creating device '%s'\n", HDCP_TAG, ESM_DEVICE_NAME);

	g_device = device_create(g_device_class, NULL,
		MKDEV(ESM_DEVICE_MAJOR, 0), NULL, ESM_DEVICE_NAME);

	if (IS_ERR(g_device)) {
		DPU_FB_ERR("%s Failed to create device '%s'\n",
			HDCP_TAG, ESM_DEVICE_NAME);
		return PTR_ERR(g_device);
	}

	g_device_created = 1;
	DPU_FB_INFO("%s Device '%s' has been created\n",
		HDCP_TAG, ESM_DEVICE_NAME);

	return 0;
}

static void end_device(void)
{
	if (g_device_created) {
		DPU_FB_INFO("%s Deleting device '%s'\n",
			HDCP_TAG, ESM_DEVICE_NAME);
		device_destroy(g_device_class, MKDEV(ESM_DEVICE_MAJOR, 0));
		g_device_created = 0;
	}
}

static int register_device_range(void)
{
	int ret;

	DPU_FB_INFO("%s Registering device range '%s'\n",
		HDCP_TAG, ESM_DEVICE_NAME);

	ret = register_chrdev(ESM_DEVICE_MAJOR, ESM_DEVICE_NAME,
		&device_file_operations);
	if (ret < 0) {
		DPU_FB_ERR("%s Failed to register device range '%s'\n",
			HDCP_TAG, ESM_DEVICE_NAME);
		return ret;
	}

	DPU_FB_INFO("%s Device range '%s' has been registered\n",
		HDCP_TAG, ESM_DEVICE_NAME);
	g_device_range_registered = 1;

	return 0;
}

static void unregister_device_range(void)
{
	if (g_device_range_registered) {
		DPU_FB_INFO("%s Unregistering device range '%s'\n",
			HDCP_TAG, ESM_DEVICE_NAME);
		unregister_chrdev(ESM_DEVICE_MAJOR, ESM_DEVICE_NAME);
		g_device_range_registered = 0;
	}
}

/* Creates the interface device class */
static int create_device_class(void)
{
	DPU_FB_INFO("%s Creating class /sys/class/%s\n",
		HDCP_TAG, ESM_DEVICE_CLASS);

	g_device_class = class_create(THIS_MODULE, ESM_DEVICE_CLASS);

	if (IS_ERR(g_device_class)) {
		DPU_FB_ERR("%s Failed to create device class /sys/class/%s\n",
			HDCP_TAG, ESM_DEVICE_CLASS);
		return PTR_ERR(g_device_class);
	}

	g_device_class_created = 1;
	DPU_FB_INFO("%s The class /sys/class/%s has been created\n",
		HDCP_TAG, ESM_DEVICE_CLASS);

	return 0;
}

/* Ends the device class */
static void end_device_class(void)
{
	if (g_device_class_created) {
		DPU_FB_INFO("%s Deleting the device class /sys/class/%s\n",
			HDCP_TAG, ESM_DEVICE_CLASS);
		class_destroy(g_device_class);
		g_device_class_created = 0;
	}
}

static int __init hdcp_driver_init(void)
{
	DPU_FB_INFO("%s Initializing\n", HDCP_TAG);

	mutex_init(&g_hdcp_lock);
	esm_hld_init();
	if ((register_device_range() == 0) && (create_device_class() == 0) &&
		(create_device() == 0))
		DPU_FB_INFO("%s Done initializing the HLD driver\n", HDCP_TAG);
	else
		DPU_FB_ERR("%s Failed to initialize the HLD driver\n", HDCP_TAG);

	return 0;
}

static void __exit hdcp_driver_exit(void)
{
	DPU_FB_INFO("%s Exiting\n", HDCP_TAG);
	mutex_destroy(&g_hdcp_lock);
	esm_end_device();
	end_device();
	end_device_class();
	unregister_device_range();
	DPU_FB_INFO("%s Done\n", HDCP_TAG);
}

module_init(hdcp_driver_init);
module_exit(hdcp_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Tech. Co., Ltd.");
MODULE_DESCRIPTION("ESM Linux Host Library Driver");


