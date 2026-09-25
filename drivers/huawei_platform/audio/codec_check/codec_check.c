/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: codec reg check in call
 * Author: lijinkui
 * Create: 2020-01-10
 */

#include "huawei_platform/audio/codec_check.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/pm_wakeup.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/hisi/hi64xx/hi64xx_utils.h>
#include <dsm/dsm_pub.h>
#include <dsm_audio/dsm_audio.h>

#define HWLOG_TAG codec_check
HWLOG_REGIST();

#define MAX_SCENE_NUM    20
#define RETRY_COUNT      3
#define ERR_REG_NUM      5

struct codec_check_priv {
	struct mutex ioctl_mutex;
	struct codec_check_data *para[MAX_SCENE_NUM];
};

static struct codec_check_priv *codec_check_priv_data;
static int check_scene = MAX_SCENE_NUM;

static unsigned long copy_param_from_user(void *to,
	const void __user *from, unsigned long n)
{
	int try_times = 0;
	unsigned long ret = 0;

	while (try_times < RETRY_COUNT) {
		ret = copy_from_user(to, from, n);
		if (ret == 0)
			break;

		try_times++;
		hwlog_info("%s: copy from user fail ret %lu, rety %d times\n",
			__func__, ret, try_times);
	}

	return ret;
}

static int codec_check_get_input_param(unsigned int usr_para_size,
	const void *usr_para_addr)
{
	void *para_in = NULL;
	int scene;

	if (!codec_check_priv_data) {
		hwlog_err("%s: codec_check_priv_data is null\n", __func__);
		return -ENOMEM;
	}

	if (usr_para_size == 0 ||
		usr_para_size > sizeof(struct codec_check_data)) {
		hwlog_err("%s: invalid usr_para_size\n", __func__);
		return -EINVAL;
	}

	para_in = kzalloc(sizeof(struct codec_check_data), GFP_KERNEL);
	if (!para_in)
		return -ENOMEM;

	if (usr_para_addr) {
		if (copy_param_from_user(para_in,
			usr_para_addr, usr_para_size)) {
			hwlog_err("%s: copy from user fail\n", __func__);
			goto error;
		}
	} else {
		hwlog_err("%s: usr_para_addr is null\n", __func__);
		goto error;
	}

	scene = ((struct codec_check_data *)para_in)->scene_num;
	if (scene >= MAX_SCENE_NUM || scene < 0) {
		hwlog_err("%s: invalid scene %d\n", __func__, scene);
		goto error;
	}

	codec_check_priv_data->para[scene] = para_in;

	return 0;
error:
	kfree(para_in);
	para_in = NULL;
	return -EINVAL;
}

static int drv_async_cmd(uintptr_t arg)
{
	int ret = -1;
	struct drv_io_async_param param;
	void __user *para_addr_in = NULL;

	if (copy_param_from_user(&param, (void *)arg, sizeof(param))) {
		hwlog_err("%s: copy from user fail\n", __func__);
		goto end;
	}

	para_addr_in = int_to_addr(param.para_in_l, param.para_in_h);

	ret = codec_check_get_input_param(param.para_size_in, para_addr_in);
	if (ret != 0) {
		hwlog_err("%s: get input param fail\n", __func__);
		goto end;
	}

	ret = copy_to_user(int_to_addr(param.para_out_l, param.para_out_h),
		&ret, sizeof(int));
	if (ret != 0) {
		hwlog_err("%s: sync cmd send err\n", __func__);
		goto end;
	}

end:
	return ret;
}

static void dsm_report_codec_check(struct error_reg *reg, int len)
{
	if (!reg) {
		hwlog_err("%s: reg is null\n", __func__);
		return;
	}
	if (len / sizeof(reg[0]) != ERR_REG_NUM) {
		hwlog_err("%s: invalid input\n", __func__);
		return;
	}
#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC,
		DSM_CODEC_CHECK_ERR,
		"reg fail: %d, %x:%x:%x, %x:%x:%x, %x:%x:%x, %x:%x:%x, %x:%x:%x",
		check_scene,                                       // err scene
		reg[0].reg & 0xFFFF, reg[0].val, reg[0].real_val,  // err dump
		reg[1].reg & 0xFFFF, reg[1].val, reg[1].real_val,  // err dump
		reg[2].reg & 0xFFFF, reg[2].val, reg[2].real_val,  // err dump
		reg[3].reg & 0xFFFF, reg[3].val, reg[3].real_val,  // err dump
		reg[4].reg & 0xFFFF, reg[4].val, reg[4].real_val); // err dump
#endif
}

static int codec_reg_check(unsigned int reg, unsigned int value,
	unsigned int mask, unsigned int *real_val)
{
	if (!real_val) {
		hwlog_err("%s: real_val null\n", __func__);
		return 0;
	}

	*real_val = hi64xx_utils_reg_read(reg);

	if ((*real_val & mask) == (value & mask))
		return 1;

	return 0;
}

static void codec_check_by_scene(int scene)
{
	int i = 0;
	int real_val = 0;
	int ret;
	int index = 0;
	struct codec_check_node *node = NULL;
	struct error_reg reg[ERR_REG_NUM];

	hwlog_info("%s: ++\n", __func__);

	memset(reg, 0, sizeof(reg));

	if (!codec_check_priv_data) {
		hwlog_err("%s: codec_check_priv_data is null\n", __func__);
		return;
	}

	if (scene >= MAX_SCENE_NUM || scene < 0) {
		hwlog_err("%s: invalid input, scene %d\n", __func__, scene);
		return;
	}

	check_scene = scene;

	if (!codec_check_priv_data->para[scene]) {
		hwlog_err("%s: no param, scene %d\n", __func__, scene);
		return;
	}

	node = codec_check_priv_data->para[scene]->node;

	while (node[i].reg != 0 && i < MAX_REG_NUM) {
		ret = codec_reg_check(node[i].reg,
			node[i].val, node[i].mask, &real_val);
		if (ret == 0) {
			hwlog_err("%s: reg 0x%x, exp_val 0x%x, real_val 0x%x\n",
				__func__, node[i].reg, node[i].val, real_val);
			reg[index].reg = node[i].reg;
			reg[index].val = node[i].val;
			reg[index].real_val = real_val;
			index++;
			if (index >= ERR_REG_NUM)
				break;
		}
		i++;
	}
	if (index > 0)
		dsm_report_codec_check(reg, sizeof(reg));
}

static int codec_check_cmd(uintptr_t arg)
{
	int scene = -1;

	if (copy_param_from_user(&scene, (void *)arg, sizeof(scene))) {
		hwlog_err("%s: copy from user fail\n", __func__);
		return -1;
	}

	codec_check_by_scene(scene);

	return 0;
}

static long codec_check_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	int ret;
	void __user *data32 = (void __user *)(uintptr_t)arg;

	if (!data32) {
		hwlog_err("%s: input buff is null\n", __func__);
		return (long)-EINVAL;
	}

	switch (cmd) {
	case IOCTL_CODEC_CHECK_SET_PARAM:
		mutex_lock(&codec_check_priv_data->ioctl_mutex);
		ret = drv_async_cmd((uintptr_t)data32);
		mutex_unlock(&codec_check_priv_data->ioctl_mutex);
		break;
	case IOCTL_CODEC_CHECK_TEST:
		mutex_lock(&codec_check_priv_data->ioctl_mutex);
		ret = codec_check_cmd((uintptr_t)data32);
		mutex_unlock(&codec_check_priv_data->ioctl_mutex);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static long codec_check_ioctl32(struct file *fd,
	unsigned int cmd, unsigned long arg)
{
	void *user_ptr = compat_ptr(arg);

	return codec_check_ioctl(fd, cmd, (uintptr_t)user_ptr);
}

static const struct file_operations codec_check_fops = {
	.owner           = THIS_MODULE,
	.open            = simple_open,
	.unlocked_ioctl = codec_check_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl    = codec_check_ioctl32,
#endif
};

static struct miscdevice codec_check_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "codec_check",
	.fops =     &codec_check_fops,
};

static const struct of_device_id codec_check_of_match[] = {
	{ .compatible = "huawei,codec_check", },
	{},
};
MODULE_DEVICE_TABLE(of, codec_check_of_match);

static int codec_check_probe(struct platform_device *pdev)
{
	int ret;

	codec_check_priv_data = kzalloc(
		sizeof(*codec_check_priv_data), GFP_KERNEL);
	if (!codec_check_priv_data)
		return -ENOMEM;

	mutex_init(&codec_check_priv_data->ioctl_mutex);

	ret = misc_register(&codec_check_miscdev);
	if (ret) {
		hwlog_err("%s: register codec_check_miscdev fail, ret:%d\n",
			__func__, ret);
		goto err_out;
	}

	hwlog_info("%s: success\n", __func__);

	return 0;

err_out:
	mutex_destroy(&codec_check_priv_data->ioctl_mutex);
	kfree(codec_check_priv_data);
	codec_check_priv_data = NULL;
	return ret;
}

static int codec_check_remove(struct platform_device *pdev)
{
	if (!codec_check_priv_data)
		return 0;

	misc_deregister(&codec_check_miscdev);
	mutex_destroy(&codec_check_priv_data->ioctl_mutex);
	kfree(codec_check_priv_data);
	codec_check_priv_data = NULL;
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static struct platform_driver codec_check_driver = {
	.driver = {
		.name   = "codec_check",
		.owner  = THIS_MODULE,
		.of_match_table = codec_check_of_match,
	},
	.probe  = codec_check_probe,
	.remove = codec_check_remove,
};

static int __init codec_check_init(void)
{
	return platform_driver_register(&codec_check_driver);
}

static void __exit codec_check_exit(void)
{
	platform_driver_unregister(&codec_check_driver);
}

late_initcall(codec_check_init);
module_exit(codec_check_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("codec reg check in call");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
