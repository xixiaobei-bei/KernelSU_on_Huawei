/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *  Description: security_info driver
 *  Create : 2021/07/06
 */
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hisi/efuse_driver.h>
#include <linux/hisi/security_info.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <securec.h>

/* node name */
#define SECURITY_INFO_NODE_NAME "security_info"
static bool g_dieid_init;
static u8 dieid[EFUSE_DIEID_LENGTH_BYTES];

static s32 security_info_get_dieid_hash(u32 cmd, uintptr_t arg)
{
	s32 ret;
	void __user *argp = (void __user *)arg;
	struct dieid_hash dieid_hash = {0};

	/* if declare and define argp, static check warning */
	if (argp == NULL) {
		pr_err("argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_GET_DIEID_HASH) {
		pr_err("cmd 0x%08x error\n", cmd);
		return -EFAULT;
	}

	if (!g_dieid_init) {
		ret = get_efuse_dieid_value(dieid, sizeof(dieid), EFUSE_TIMEOUT_SECOND);
		if (ret != 0) {
			pr_err("get efuse dieid err\n");
			return -EFAULT;
		}
		g_dieid_init = true;
	}

	ret = security_info_compute_sha256(dieid, sizeof(dieid),
		dieid_hash.hash, SHA256_BYTES);
	if (ret != 0) {
		pr_err("compute_sha256 dieid err\n");
		return -EFAULT;
	}
	dieid_hash.hash_bytes = SHA256_BYTES;

	ret = copy_to_user(argp, &dieid_hash, sizeof(struct dieid_hash));
	if (ret != 0) {
		pr_err("copy dieid hash err\n");
		return -EFAULT;
	}

	return ret;
}

static const struct ioctl_security_info_func_t g_security_info_tbl[] = {
	{
		SECURUTY_INFO_VERIFY_BYPASS_NET_CERT,
		"verify bypass net",
		security_info_verify_bypass_net_cert,
	},
	{
		SECURUTY_INFO_GET_DIEID_HASH,
		"get dieid hash",
		security_info_get_dieid_hash,
	}
};

/*
 * Function name:security_info_ioctl.
 * Discription:
 *    security_info ioctl
 * return value:
 *    @ 0 : success.
 *    @ -EFAULT : failure.
 */
static long security_info_ioctl(struct file *file, u32 cmd, unsigned long arg)
{
	u32 i;
	s32 ret;
	const struct ioctl_security_info_func_t *item = NULL;

	pr_err("enter func %s, cmd=0x%08x", __func__, cmd);

	for (i = 0; i < ARRAY_SIZE(g_security_info_tbl); i++) {
		if (g_security_info_tbl[i].cmd == cmd) {
			item =  &g_security_info_tbl[i];
			break;
		}
	}

	if (item == NULL || !item->func) {
		pr_err("item or itm->func is null");
		return -EFAULT;
	}

	if (item->name)
		pr_err("process %s\n", item->name);

	ret = item->func(cmd, arg);
	if (ret != 0) {
		pr_err("ioctl failed %d, cmd 0x%08x\n", ret, cmd);
		return -EFAULT;
	}

	return ret;
}

static long security_info_compat_ioctl(
	struct file *file, u32 cmd, unsigned long arg)
{
	void __user *arg64 = compat_ptr(arg);

	if (file->f_op == NULL) {
		pr_err("%s,file->f_op is NULL\n", __func__);
		return -ENOTTY;
	}

	if (!file->f_op->unlocked_ioctl) {
		pr_err("%s,file->f_op->unlocked_ioctl is NULL\n", __func__);
		return -ENOTTY;
	}

	return file->f_op->unlocked_ioctl(
		file, cmd, (unsigned long)(uintptr_t)arg64);
}

/*
 * 32bit app & 32bit kernel, app ioctl will callback unlocked_ioctl
 * 64bit app & 64bit kernel, app ioctl will callback unlocked_ioctl
 * 32bit app & 64bit kernel, app ioctl will callback compat_ioctl
 * 64bit app & 32bit kernel, illegal
 */
static const struct file_operations g_security_info_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = security_info_ioctl,
	.compat_ioctl = security_info_compat_ioctl,
};

static s32 __init security_info_driver_init(void)
{
	s32 ret;
	s32 major;
	struct class *pclass = NULL;
	struct device *pdevice = NULL;

	pr_err("enter func %s", __func__);

	major = register_chrdev(
		0, SECURITY_INFO_NODE_NAME, &g_security_info_fops);
	if (major <= 0) {
		ret = -EFAULT;
		pr_err("unable to get major.\n");
		goto error0;
	}

	pclass = class_create(THIS_MODULE, SECURITY_INFO_NODE_NAME);
	if (IS_ERR(pclass)) {
		ret = -EFAULT;
		pr_err("class_create error.\n");
		goto error1;
	}

	pdevice = device_create(pclass, NULL, MKDEV((u32)major, 0),
				NULL, SECURITY_INFO_NODE_NAME);
	if (IS_ERR(pdevice)) {
		ret = -EFAULT;
		pr_err("device_create error.\n");
		goto error2;
	}

	pr_info("success\n");
	return ret;

error2:
	class_destroy(pclass);
error1:
	unregister_chrdev(major, SECURITY_INFO_NODE_NAME);
error0:
	return ret;
}

rootfs_initcall(security_info_driver_init);

MODULE_DESCRIPTION("security_info module");
MODULE_LICENSE("GPL");

