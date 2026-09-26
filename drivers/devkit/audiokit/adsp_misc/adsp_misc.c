/*
 * adsp_misc driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "adsp_misc.h"

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/hw_log.h>

#ifdef CONFIG_ADSP_HUAWEI_SHARED_MEM
#include "adsp_huawei_shared_mem.h"
#endif

#define HWLOG_TAG adsp_misc
#define MAX_BUFFER_SIZE (30000)
HWLOG_REGIST();

struct adsp_priv {
	struct mutex do_ioctl_lock;
};

int __attribute__((weak)) mtk_spk_send_ipi_buf_to_dsp(void *data_buffer,
							uint32_t data_size)
{
	(void)data_buffer;
	(void)data_size;
	hwlog_info("%s: call weak func\n", __func__);
	return 0;
}

int __attribute__((weak)) mtk_spk_recv_ipi_buf_from_dsp(int8_t *buffer,
							int16_t size,
							uint32_t *buf_len)
{
	(void)buffer;
	(void)size;
	(void)buf_len;
	hwlog_info("%s: call weak func\n", __func__);
	return 0;
}

static struct adsp_priv *adsp_priv_data;

static int write_data_to_dsp(void *data, int len)
{
	hwlog_info("%s: send ipi buf to dsp\n", __func__);
	return mtk_spk_send_ipi_buf_to_dsp(data, len);
}

static int read_data_from_dsp(void *data, int len)
{
	hwlog_info("%s: recv ipi data from dsp\n", __func__);
	uint32_t real_len = len;
	int ret = -EFAULT;

	ret = mtk_spk_send_ipi_buf_to_dsp(data, len);
	if (ret < 0) {
		hwlog_err("%s: mtk_spk_send_ipi_buf_to_dsp fail, ret = %d\n",
								__func__, ret);
		return ret;
	}

	ret = mtk_spk_recv_ipi_buf_from_dsp(data, len, &real_len);
	if (ret < 0)
		hwlog_err("%s: mtk_spk_recv_ipi_buf_from_dsp fail, ret = %d\n",
								__func__, ret);

	return ret;
}

#ifdef CONFIG_ADSP_HUAWEI_SHARED_MEM
static int set_adsp_shared_mem(const void __user *arg,
				const void *data_ptr, int data_len)
{
	const void __user *buf = arg + data_len;
	unsigned int cmd = ((struct adsp_set_shared_mem *)data_ptr)->cmd;
	size_t buf_size = ((struct adsp_set_shared_mem *)data_ptr)->param_size;
	int ret = 0;

	switch (cmd) {
	case ADSP_SHARED_MEM_UPDATE:
		ret = update_adsp_shared_mem(buf, buf_size);
		break;
	case ADSP_SHARED_MEM_NOTIFY_ONLY:
		ret = notify_adsp_shared_mem(buf_size);
		break;
	default:
		break;
	}
	if (ret != 0) {
		hwlog_err("%s: cmd-%u failed\n", __func__, cmd);
		return -EFAULT;
	}
	hwlog_info("%s: cmd-%u success\n", __func__, cmd);
	return 0;
}
#endif

static int adsp_misc_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	if ((inode == NULL) || (filp == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ret = nonseekable_open(inode, filp);
	if (ret < 0) {
		hwlog_err("%s: nonseekable_open failed\n", __func__);
		return ret;
	}

	filp->private_data = (void *)adsp_priv_data;
	return ret;
}

static int adsp_misc_release(struct inode *inode, struct file *filp)
{
	if ((inode == NULL) || (filp == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	filp->private_data = NULL;
	return 0;
}

static int adsp_ctrl_do_ioctl_unlock(struct file *file, unsigned int cmd,
					void __user *arg, int compat_mode)
{
	int ret = 0;

	if ((_IOC_TYPE(cmd)) != (ADSP_IOCTL_MAGIC)) {
		hwlog_err("%s: cmd magic number invalid\n", __func__);
		return -EINVAL;
	}

	int16_t data_len = _IOC_SIZE(cmd);
	char *data_ptr = NULL;

	if (data_len > 0 && data_len < MAX_BUFFER_SIZE) {
		data_ptr = kzalloc(data_len, GFP_KERNEL);
		if (data_ptr == NULL) {
			return -ENOMEM;
		}
	} else {
		hwlog_err("%s: data len is zero or too large\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user((void *)data_ptr, arg, data_len)) {
		hwlog_err("%s: copy_from_user fail\n", __func__);
		ret = -EFAULT;
		goto EXIT;
	}

	switch (cmd) {
	case IPI_SET_BUF:
	case SET_PA_PARAM:
		ret = write_data_to_dsp(data_ptr, data_len);
		if (ret) {
			hwlog_err("cmd = %d, ret = %d\n", cmd, ret);
			ret = -EFAULT;
			goto EXIT;
		}
		break;
	case IPI_GET_BUF:
		ret = read_data_from_dsp(data_ptr, data_len);
		if (ret) {
			hwlog_err("ipi_get_buf, ret = %d\n", ret);
			ret = -EFAULT;
			goto EXIT;
		}

		if (copy_to_user((void __user *)arg, data_ptr, data_len)) {
			hwlog_err("%s: copy_to_user fail\n", __func__);
			ret = -EFAULT;
			goto EXIT;
		}
		break;
	case SET_ADSP_SHARED_MEM:
#ifdef CONFIG_ADSP_HUAWEI_SHARED_MEM
		ret = set_adsp_shared_mem(arg, data_ptr, data_len);
#else
		hwlog_warn("%s: not support SET_ADSP_SHARED_MEM\n", __func__);
#endif
		break;
	default:
		hwlog_err("%s: not support cmd = 0x%x\n", __func__, cmd);
		ret = -EIO;
		break;
	}

EXIT:
	kfree(data_ptr);
	return ret;
}

static long adsp_misc_ioctl(struct file *file, unsigned int command,
						unsigned long arg)
{
	if ((file == NULL) || (file->private_data == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	struct adsp_priv *adsp_priv_data = file->private_data;

	mutex_lock(&adsp_priv_data->do_ioctl_lock);
	int ret = adsp_ctrl_do_ioctl_unlock(file, command,
					(void __user *)(uintptr_t)arg, 0);
	mutex_unlock(&adsp_priv_data->do_ioctl_lock);

	return ret;
}

#ifdef CONFIG_COMPAT
static long adsp_misc_ioctl_compat(struct file *file,
	unsigned int command, unsigned long arg)
{
	if ((file == NULL) || (file->private_data == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	struct adsp_priv *adsp_priv_data = file->private_data;

	mutex_lock(&adsp_priv_data->do_ioctl_lock);
	int ret = adsp_ctrl_do_ioctl_unlock(file, command,
					(void __user *)(uintptr_t)arg, 1);
	mutex_unlock(&adsp_priv_data->do_ioctl_lock);
	return ret;
}
#endif

static const struct file_operations adsp_ctrl_fops = {
	.owner          = THIS_MODULE,
	.open           = adsp_misc_open,
	.release        = adsp_misc_release,
	.unlocked_ioctl = adsp_misc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = adsp_misc_ioctl_compat,
#endif
};

static struct miscdevice adsp_ctl_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "adsp_misc",
	.fops  = &adsp_ctrl_fops,
};

static int adsp_probe(struct platform_device *pdev)
{
	struct adsp_priv *adsp_priv = NULL;
	int ret = -EFAULT;

	hwlog_info("%s: enter\n", __func__);

	if (pdev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	adsp_priv = kzalloc(sizeof(*adsp_priv), GFP_KERNEL);
	if (adsp_priv == NULL)
		return -ENOMEM;

	adsp_priv_data = adsp_priv;
	platform_set_drvdata(pdev, adsp_priv);

	mutex_init(&adsp_priv->do_ioctl_lock);

	ret = misc_register(&adsp_ctl_miscdev);
	if (ret != 0) {
		hwlog_err("%s: register miscdev failed, %d\n", __func__, ret);
		goto err_out;
	}

	hwlog_info("%s: end success\n", __func__);
	return 0;

err_out:
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int adsp_remove(struct platform_device *pdev)
{
	if (pdev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	struct adsp_priv *adsp_priv = platform_get_drvdata(pdev);
	if (adsp_priv == NULL) {
		hwlog_err("%s: adsp_priv invalid argument\n", __func__);
		return -EINVAL;
	}

	kfree(adsp_priv);
	platform_set_drvdata(pdev, NULL);
	misc_deregister(&adsp_ctl_miscdev);
	return 0;
}

static const struct of_device_id adsp_match[] = {
	{ .compatible = "huawei,adsp_misc", },
	{},
};
MODULE_DEVICE_TABLE(of, adsp_match);

static struct platform_driver adsp_driver = {
	.driver = {
		.name           = "adsp_misc",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_match),
	},
	.probe    = adsp_probe,
	.remove   = adsp_remove,
};

static int __init adsp_misc_init(void)
{
	hwlog_info("%s: platform_driver_register\n", __func__);
	int ret = -EFAULT;

	ret = platform_driver_register(&adsp_driver);
	if (ret != 0)
		hwlog_err("%s: platform_driver_register failed, %d\n",
				  __func__, ret);

	return ret;
}

static void __exit adsp_misc_exit(void)
{
	platform_driver_unregister(&adsp_driver);
}

fs_initcall_sync(adsp_misc_init);
module_exit(adsp_misc_exit);

MODULE_DESCRIPTION("adsp misc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
