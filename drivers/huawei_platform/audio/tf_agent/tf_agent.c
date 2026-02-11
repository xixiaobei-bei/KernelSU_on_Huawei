/*
 * tf_agent.c
 *
 * tfagent driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <huawei_platform/audio/tf_agent.h>

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>

#define DEBUG_LEVEL 0
#define HWLOG_TAG tf_agent

#if DEBUG_LEVEL
HWLOG_REGIST_LEVEL(HWLOG_ERR | HWLOG_WARNING | HWLOG_INFO | HWLOG_DEBUG);
#else
HWLOG_REGIST();
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define TF_AGENT_NAME "tf_agent"
#define VOICE_PACKET_SIZE_MAX 256

struct tf_agent {
	int packet_size_max;

	char read_packet[VOICE_PACKET_SIZE_MAX];
	char write_packet[VOICE_PACKET_SIZE_MAX];

	struct mutex read_lock;
	struct mutex write_lock;
};

static struct tf_agent *tf_agent_priv;
static bool tf_agent_support_flag;

static int __init early_parse_vendorcountry_cmdline(char *arg)
{
	if (arg == NULL) {
		hwlog_err("%s: invalid param\n", __func__);
		return -1;
	}
	hwlog_debug("%s: vendorcountry is %s\n", __func__, arg);

	tf_agent_support_flag = false;
	if (strcmp(arg, "cmcc/cn") == 0)
		tf_agent_support_flag = true;
	hwlog_debug("%s: tf_agent_support_flag is %d\n",
		__func__, tf_agent_support_flag);

	return 0;
}
early_param("vendorcountry", early_parse_vendorcountry_cmdline);

// The following interfaces from:
// vendor/hisi/ap/kernel/drivers/hisi/hifi_dsp/voice_proxy/voice_proxy_volte.c
__attribute__((weak)) int32_t proxy_push_data(void *data)
{
	UNUSED(data);
	hwlog_debug("%s: use weak\n", __func__);
	return 0;
}

__attribute__((weak)) void proxy_wake_up_pull(void)
{
	hwlog_debug("%s: use weak\n", __func__);
}

__attribute__((weak)) int32_t proxy_pull_data(int8_t *data, int32_t size)
{
	UNUSED(data);
	UNUSED(size);
	hwlog_debug("%s: use weak\n", __func__);
	return 0;
}

__attribute__((weak)) int32_t proxy_enable_sec_key_negotiation(int32_t enable)
{
	UNUSED(enable);
	hwlog_debug("%s: use weak\n", __func__);
	return 0;
}

static ssize_t tf_agent_read(struct file *file, char __user *buf,
			size_t count, loff_t *pos)
{
	struct tf_agent *priv = tf_agent_priv;
	int proxy_read;
	int ret;

	UNUSED(pos);
	if ((file == NULL) || (buf == NULL) || (priv == NULL)) {
		hwlog_err("%s: file, buf or priv is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&priv->read_lock);
	if ((count <= 0) || (count > priv->packet_size_max)) {
		hwlog_err("%s: invalid count %d,%d\n", __func__,
			(int)count, priv->packet_size_max);
		ret = -EINVAL;
		goto err_out;
	}

	if (file->f_flags & O_NONBLOCK) {
		hwlog_info("%s: not block, skip\n", __func__);
		ret = -EAGAIN;
		goto err_out;
	}

	memset(priv->read_packet, 0, sizeof(priv->read_packet));
	hwlog_debug("%s: read_packet len %ld\n", __func__,
		sizeof(priv->read_packet));
	// receive from voice proxy
	proxy_read = proxy_pull_data((int8_t *)priv->read_packet,
		(int32_t)count);
	if (proxy_read < 0) {
		hwlog_info("%s: read proxy FIFO failed %d\n",
			__func__, proxy_read);
		ret = -EFAULT;
		goto err_out;
	}

	if (copy_to_user(buf, priv->read_packet, count)) {
		hwlog_err("%s: copy_to_user failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	hwlog_debug("%s: read success %d,%d\n", __func__,
		(int)count, proxy_read);
	ret = proxy_read;

err_out:
	mutex_unlock(&priv->read_lock);
	return ret;
}

static ssize_t tf_agent_write(struct file *file, const char __user *buf,
			size_t count, loff_t *pos)
{
	struct tf_agent *priv = tf_agent_priv;
	int ret;

	UNUSED(file);
	UNUSED(pos);
	if ((buf == NULL) || (priv == NULL)) {
		hwlog_err("%s: buf or priv is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&priv->write_lock);
	if ((count <= 0) || (count > priv->packet_size_max)) {
		hwlog_err("%s: invalid count %d,%d\n", __func__,
			(int)count, priv->packet_size_max);
		ret = -EINVAL;
		goto err_out;
	}

	memset(priv->write_packet, 0, sizeof(priv->write_packet));
	hwlog_debug("%s: write_packet len %ld\n", __func__,
		sizeof(priv->write_packet));
	if (copy_from_user(priv->write_packet, buf, count)) {
		hwlog_err("%s: copy_from_user failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	// send to voice proxy
	ret = proxy_push_data((void *)priv->write_packet);
	if (ret < 0) {
		hwlog_err("%s: write proxy FIFO failed %d\n", __func__, ret);
		ret = -EFAULT;
		goto err_out;
	}

	hwlog_debug("%s: write success %d,%d\n", __func__,
		(int)count, ret);
	ret = count;

err_out:
	mutex_unlock(&priv->write_lock);
	return ret;
}

static long tf_agent_ioctl(struct file *file, unsigned int cmd,
			unsigned long arg)
{
	long ret = 0;

	UNUSED(file);
	UNUSED(arg);
	hwlog_debug("%s: cmd 0x%x begin\n", __func__, cmd);
	switch (cmd) {
	case CLOSE_READ:
		proxy_wake_up_pull();
		break;
	case SEC_KEY_NEG_START:
		ret = proxy_enable_sec_key_negotiation(0); // enable
		break;
	case SEC_KEY_NEG_END_SUCCESS:
		ret = proxy_enable_sec_key_negotiation(1); // succ_disable
		break;
	case SEC_KEY_NEG_END_FAIL:
		ret = proxy_enable_sec_key_negotiation(2); // fail_disable
		break;
	default:
		hwlog_info("%s: invalid cmd 0x%x\n", __func__, cmd);
		return -EINVAL;
	}

	hwlog_debug("%s: cmd 0x%x end %ld\n", __func__, cmd, ret);
	return ret;
}

static int tf_agent_close(struct inode *node, struct file *file)
{
	UNUSED(node);
	UNUSED(file);
	return 0;
}

static const struct file_operations tf_agent_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = tf_agent_read,
	.write = tf_agent_write,
	.unlocked_ioctl = tf_agent_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = tf_agent_ioctl,
#endif
	.release = tf_agent_close,
};

static struct miscdevice tf_agent_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = TF_AGENT_NAME,
	.fops = &tf_agent_fops,
};

static int tf_agent_probe(struct platform_device *pdev)
{
	struct tf_agent *priv = NULL;
	int size;
	int ret;

	hwlog_debug("%s: enter\n", __func__);
	if (pdev == NULL) {
		hwlog_err("%s: pdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	priv->packet_size_max = VOICE_PACKET_SIZE_MAX;
	if (!of_property_read_u32(pdev->dev.of_node,
		"voice_packet_size_max", &size)) {
		hwlog_info("get packet size success %d\n", size);
		if (size < VOICE_PACKET_SIZE_MAX)
			priv->packet_size_max = size;
	} else {
		hwlog_info("get packet size failed, use default\n");
	}

	ret = misc_register(&tf_agent_miscdev);
	if (ret) {
		hwlog_err("%s: misc register failed %d", __func__, ret);
		goto err_out;
	}

	mutex_init(&priv->read_lock);
	mutex_init(&priv->write_lock);

	tf_agent_priv = priv;
	hwlog_debug("%s: success\n", __func__);
	return 0;

err_out:
	if (priv != NULL)
		kfree(priv);
	return ret;
}

static int tf_agent_remove(struct platform_device *pdev)
{
	UNUSED(pdev);
	kfree(tf_agent_priv);
	tf_agent_priv = NULL;

	misc_deregister(&tf_agent_miscdev);
	return 0;
}

static const struct of_device_id tf_agent_match[] = {
	{ .compatible = "huawei,tf_agent", },
	{},
};
MODULE_DEVICE_TABLE(of, tf_agent_match);

static struct platform_driver tf_agent_driver = {
	.driver = {
		.name = TF_AGENT_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(tf_agent_match),
	},
	.probe = tf_agent_probe,
	.remove = tf_agent_remove,
};

static int __init tf_agent_init(void)
{
	if (!tf_agent_support_flag) {
		hwlog_info("%s: tf_agent not supported", __func__);
		return 0;
	}
	hwlog_info("%s: tf_agent supported", __func__);

	return platform_driver_register(&tf_agent_driver);
}

static void __exit tf_agent_exit(void)
{
	platform_driver_unregister(&tf_agent_driver);
}

module_init(tf_agent_init);
module_exit(tf_agent_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("tfagent driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

