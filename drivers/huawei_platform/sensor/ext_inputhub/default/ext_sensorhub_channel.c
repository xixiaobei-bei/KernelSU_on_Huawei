/*
 * ext_sensorhub_channel.c
 *
 * code for external sensorhub channel driver
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

#include "ext_sensorhub_channel.h"

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <huawei_platform/log/hw_log.h>

#include "ext_sensorhub_route.h"
#include "ext_sensorhub_frame.h"

#define HWLOG_TAG ext_sensorhub_channel
HWLOG_REGIST();

/* do nothing in read function */
static ssize_t ext_sensorhub_read(struct file *file, char __user *buf,
				  size_t count, loff_t *pos)
{
	return count;
}

/* do nothing in write function */
static ssize_t ext_sensorhub_write(struct file *file, const char __user *data,
				   size_t len, loff_t *ppos)
{
	return len;
}

static long ioctl_send_cmd(unsigned long arg)
{
	int status;
	/* send command */
	struct ioctl_send_data send_data;

	/* arg is ioctl_send_data struct pointer
	 * send buffer and reveive buffer also need copy_from/to
	 */
	if (copy_from_user(&send_data, (struct ioctl_send_data *)arg,
			   sizeof(struct ioctl_send_data)))
		return -EFAULT;

	/* check if a relax wake lock cmd */
	if (send_data.service_id == SERVICE_ID_COMMU &&
		send_data.command_id == SERVICE_RELAX_WAKE_LOCK) {
		hwlog_info("%s try to relax lock", __func__);
		/* here try to relax wake lock */
		ext_sensorhub_lock_try_relax();
		return 0;
	}

	/* wait resp alreay copied */
	status = ext_sensorhub_route_write(send_data.port, &send_data);
	if (status < 0) {
		hwlog_err("%s write route error: %d\n", __func__, status);
		return status;
	}
	/* resp len */
	if (copy_to_user((struct ioctl_send_data *)arg, &send_data,
			 sizeof(struct ioctl_send_data)))
		return -EFAULT;

	hwlog_debug("%s send cmd end, sid: %d cid: %d, ret: %d\n",
		    __func__, send_data.service_id,
		    send_data.command_id, status);
	return status;
}

static long ioctl_read(unsigned long arg)
{
	/* read data */
	struct ioctl_read_data read_data;
	unsigned char service_id = 0;
	unsigned char command_id = 0;
	int status;

	/* receive buffer also need copy_from/to */
	if (copy_from_user(&read_data, (struct ioctl_read_data *)arg,
			   sizeof(struct ioctl_read_data)))
		return -EFAULT;

	hwlog_debug("%s read data get param success, service id: %d\n",
		    __func__, read_data.service_id);

	/* call sensorhub read */
	status = ext_sensorhub_route_read(
		read_data.port, read_data.receive_buffer,
		read_data.receive_buffer_len,
		&service_id, &command_id);
	if (status < 0) {
		hwlog_err("%s read route error: %d\n", __func__, status);
		return status;
	}

	read_data.service_id = service_id;
	read_data.command_id = command_id;
	/* receive buffer been copied already */
	if (copy_to_user((struct ioctl_read_data *)arg,
			 &read_data, sizeof(struct ioctl_read_data)))
		return -EFAULT;

	return status;
}

static long ioctl_config(unsigned long arg)
{
	/* config cmd for route(channel) */
	struct ioctl_config_cmd config_cmd;

	if (copy_from_user(&config_cmd, (struct ioctl_config_cmd *)arg,
			   sizeof(struct ioctl_config_cmd)))
		return -EFAULT;

	/* get buffer for config cmds */
	if (config_cmd.cmd_cnt < 0)
		return -EFAULT;

	return ext_sensorhub_route_config(config_cmd.port,
		config_cmd.service_ids, config_cmd.command_ids,
		config_cmd.cmd_cnt);
}

static long ioctl_query_max_len(void)
{
	return max_send_data_len();
}

static long ext_sensorhub_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	/* check file param */
	if (!file) {
		hwlog_err("%s check param file error\n", __func__);
		return -EFAULT;
	}

	switch (cmd) {
	case EXT_SENSORHUB_IOCTL_SEND_CMD:
		return ioctl_send_cmd(arg);
	case EXT_SENSORHUB_IOCTL_READ_DATA:
		return ioctl_read(arg);
	case EXT_SENSORHUB_IOCTL_CONFIG_CMD:
		return ioctl_config(arg);
	case EXT_SENSORHUB_IOCTL_QUERY_MAX_LEN:
		return ioctl_query_max_len();
	default:
		hwlog_info("%s unknown cmd", __func__);
		return -EINVAL;
	}
	return 0;
}

static long ioctl_compat_send_cmd(unsigned long arg)
{
	int status = 0;
	struct ioctl_send_data32 data32;
	struct ioctl_send_data __user *data;

	if (copy_from_user(&data32, compat_ptr(arg),
			   sizeof(data32)))
		return -EFAULT;

	data = compat_alloc_user_space(sizeof(*data));
	if (!access_ok(VERIFY_WRITE, data, sizeof(*data)))
		return -EFAULT;

	status |= put_user(data32.port, &data->port);
	status |= put_user(data32.service_id, &data->service_id);
	status |= put_user(data32.command_id, &data->command_id);
	status |= put_user(data32.need_resp, &data->need_resp);
	status |= put_user(compat_ptr(data32.send_buffer), &data->send_buffer);
	status |= put_user(data32.send_buffer_len, &data->send_buffer_len);
	status |= put_user(compat_ptr(data32.receive_buffer),
			   &data->receive_buffer);
	status |= put_user(data32.receive_buffer_len,
			   &data->receive_buffer_len);
	if (status < 0)
		return status;

	return ioctl_send_cmd((unsigned long)data);
}

static long ioctl_compat_read(unsigned long arg)
{
	int status;
	int ret = 0;
	struct ioctl_read_data32 data32;
	struct ioctl_read_data __user *data;

	if (copy_from_user(&data32, compat_ptr(arg), sizeof(data32)))
		return -EFAULT;

	data = compat_alloc_user_space(sizeof(*data));
	if (!access_ok(VERIFY_WRITE, data, sizeof(*data)))
		return -EFAULT;

	ret |= put_user(data32.port, &data->port);
	ret |= put_user(data32.service_id, &data->service_id);
	ret |= put_user(data32.command_id, &data->command_id);
	ret |= put_user(compat_ptr(data32.receive_buffer),
			&data->receive_buffer);
	ret |= put_user(data32.receive_buffer_len, &data->receive_buffer_len);
	if (ret < 0)
		return ret;

	status = ioctl_read((unsigned long)data);

	ret |= get_user(data32.port, &data->port);
	ret |= get_user(data32.service_id, &data->service_id);
	ret |= get_user(data32.command_id, &data->command_id);
	ret |= get_user(data32.receive_buffer, &data->receive_buffer);
	ret |= get_user(data32.receive_buffer_len, &data->receive_buffer_len);
	if (ret < 0)
		return ret;

	if (copy_to_user(arg, &data32, sizeof(data32)))
		return -EFAULT;
	return status;
}

static long ioctl_compat_config(unsigned long arg)
{
	int status = 0;
	struct ioctl_config_cmd32 data32;
	struct ioctl_config_cmd __user *data;

	if (copy_from_user(&data32, compat_ptr(arg),
			   sizeof(data32)))
		return -EFAULT;

	data = compat_alloc_user_space(sizeof(*data));
	if (!access_ok(VERIFY_WRITE, data, sizeof(*data)))
		return -EFAULT;

	status |= put_user(data32.port, &data->port);
	status |= put_user(data32.cmd_cnt, &data->cmd_cnt);
	status |= put_user(compat_ptr(data32.service_ids), &data->service_ids);
	status |= put_user(compat_ptr(data32.command_ids), &data->command_ids);
	if (status < 0)
		return status;

	return ioctl_config((unsigned long)data);
}

static long ioctl_compat_query_max_len(void)
{
	return ioctl_query_max_len();
}

static long ext_sensorhub_compat_ioctl(struct file *file, unsigned int cmd,
				       unsigned long arg)
{
	/* check file param */
	if (!file) {
		hwlog_err("%s check param file error\n", __func__);
		return -EFAULT;
	}
	switch (cmd) {
	case EXT_SENSORHUB_IOCTL_SEND_CMD:
		return ioctl_compat_send_cmd(arg);
	case EXT_SENSORHUB_IOCTL_READ_DATA:
		return ioctl_compat_read(arg);
	case EXT_SENSORHUB_IOCTL_CONFIG_CMD:
		return ioctl_compat_config(arg);
	case EXT_SENSORHUB_IOCTL_QUERY_MAX_LEN:
		return ioctl_compat_query_max_len();
	default:
		hwlog_info("%s unknown cmd", __func__);
		return -EINVAL;
	}
	return 0;
}

/* do nothing in open function */
static int ext_sensorhub_open(struct inode *inode, struct file *file)
{
	return 0;
}

/* do nothing in close function */
static int ext_sensorhub_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations ext_sensorhub_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = ext_sensorhub_read,
	.write = ext_sensorhub_write,
	.unlocked_ioctl = ext_sensorhub_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ext_sensorhub_compat_ioctl,
#endif
	.open = ext_sensorhub_open,
	.release = ext_sensorhub_release,
};

static struct miscdevice ext_senorhub_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ext_sensorhub",
	.fops = &ext_sensorhub_fops,
};

static int __init ext_sensorhub_init(void)
{
	int ret;

	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	ret = misc_register(&ext_senorhub_miscdev);
	if (ret != 0) {
		hwlog_err("%s cannot register miscdev err=%d\n", __func__, ret);
		return ret;
	}
	/* init ext sensorhub */
	hwlog_info("%s ok\n", __func__);

	ext_sensorhub_route_init();
	return 0;
}

static void __exit ext_sensorhub_exit(void)
{
	if (is_ext_sensorhub_disabled())
		return;
	/* exit ext sensorhub route */
	ext_sensorhub_route_exit();
	misc_deregister(&ext_senorhub_miscdev);
	hwlog_info("%s\n", __func__);
}

module_init(ext_sensorhub_init);
module_exit(ext_sensorhub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("External sensorhub driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
