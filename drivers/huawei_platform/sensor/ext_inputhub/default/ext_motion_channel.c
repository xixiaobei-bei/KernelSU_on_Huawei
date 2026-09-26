/*
 * drivers/sensors/ext_inputhub/default/ext_motion_channel.c
 *
 * External Motion Hub Channel driver
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
#include "ext_motion_channel.h"

#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <huawei_platform/log/hw_log.h>

#include "securec.h"
#include "ext_sensorhub_api.h"

#define HWLOG_TAG ext_motion_channel
HWLOG_REGIST();

/* ioctl cmd define */
#define MHBIO 0xB1
#define MHB_IOCTL_MOTION_START _IOW(MHBIO, 0x01, short)
#define MHB_IOCTL_MOTION_STOP _IOW(MHBIO, 0x02, short)
/* use for motion cmd */
#define MOTION_SERVICE_ID 0x15
#define MOTION_START_COMMAND_ID 0x01
#define MOTION_STOP_COMMAND_ID 0x02
#define MOTION_REPORT_COMMAND_ID 0x81
#define MOTION_PAYLOAD_MIN_LEN 8
#define OPEN_SENSOR_CMD_LEN 11
#define SENSOR_CLOSE_CMD_LEN 8
/* for payload */
#define STRUCT_TAG 0x81
#define SENSOR_TYPE_TAG 0x02
#define SENSOR_ID_TAG 0x03
#define SENSOR_OPTION_TAG 0x04
#define TLV_LEN_ONE 0x01
#define DEFAULT_SENSOR_ID 0x00
#define DEFAULT_OPTION 0x00
#define OPEN_SENSOR_STRUCT_LEN 0x09
#define MOTION_DATA_PAYLOAD_LEN 0x09
#define STRUCT_COMMON_LEN 0x06
#define SENSOR_COMMON_LEN 1
/* report payload */
#define REPORT_SENSOR_TYPE_TAG 0x01
#define REPORT_SENSOR_ID_TAG 0x02
#define REPORT_DATA_TAG 0x03
/* resp */
#define COMMON_RESP_LEN 14
#define STRUCT_RESULT_LEN 0x0c
#define RESULT_TAG 0x7f
#define RESULT_LEN 0x04
#define WAIT_RESP_TIMEOUT_MS 4000
#define CMD_SUCCESS_CODE 100000
#define MOTION_RESULT_LEN 10
#define MOTION_LIST_LEN 5
#define MHB_PKG_TIMESTAMP_BYTE 8

struct cmd_wait {
	bool is_waiting;
	bool data_flag;
	unsigned char command_id;
	unsigned char service_id;
	int motion_type;
	int cmd_ret;
	wait_queue_head_t resp_wait_queue;
	/* lock for wait response */
	struct mutex resp_lock;
};

static wait_queue_head_t read_wait;
static struct mutex cmd_lock;
static struct cmd_wait wait_resp;

struct sensor_data {
	u8 sensor;
	u8 sensor_id;
	int result;
	u8 data;
};

struct motion_sensor {
	int motion_type;
	int sensor_type;
};

static struct motion_sensor motion_sensor_tbl[] = {
	{MOTION_TYPE_WRIST_TILT, MOTION_SENSOR_TYPE},
	{MOTION_TYPE_FINGER_GESTURE, MOTION_FINGER_SENSOR_TYPE},
};

#define MOTION_TBL_LEN \
(sizeof(motion_sensor_tbl) / sizeof(struct motion_sensor))

struct motion_result_table {
	u8 motion;
	u8 result; /* "1" recognize successful, "0" failed */
	s8 status; /* motion direction or activity state */
	u8 data_len;
	s32 data[MOTION_RESULT_LEN];
};

struct ext_motion_data_list {
	struct list_head   list;
	struct motion_result_table motion_result;
};

static struct ext_motion_data_list *motion_idle_head;
static struct mutex idle_lock;
static struct ext_motion_data_list *motion_data_head;
static struct mutex read_lock;
static struct sid_cid cmd_list[] = {
	{MOTION_SERVICE_ID, MOTION_START_COMMAND_ID},
	{MOTION_SERVICE_ID, MOTION_STOP_COMMAND_ID},
	{MOTION_SERVICE_ID, MOTION_REPORT_COMMAND_ID},
};

#define MOTION_CMD_LEN \
(sizeof(cmd_list) / sizeof(struct sid_cid))

static struct subscribe_cmds motion_cmds = {cmd_list, MOTION_CMD_LEN};
static int motion_callback(unsigned char service_id, unsigned char command_id,
			   unsigned char *data, int data_len);

static int init_motion_data_list(void)
{
	int i;
	struct ext_motion_data_list *buff_node = NULL;
	int ret;

	hwlog_info("%s init begin\n", __func__);
	motion_idle_head = kmalloc(sizeof(*motion_idle_head), GFP_KERNEL);
	if (!motion_idle_head)
		return -ENOMEM;
	INIT_LIST_HEAD(&motion_idle_head->list);

	motion_data_head = kmalloc(sizeof(*motion_data_head), GFP_KERNEL);
	if (!motion_data_head)
		return -ENOMEM;
	INIT_LIST_HEAD(&motion_data_head->list);

	/* initial data list */
	for (i = 0; i < MOTION_LIST_LEN; i++) {
		buff_node = kmalloc(sizeof(*motion_idle_head), GFP_KERNEL);
		if (buff_node)
			list_add_tail(&buff_node->list,
				      &motion_idle_head->list);
	}
	ret = register_data_callback(MOTION_CHANNEL, &motion_cmds,
				     motion_callback);
	if (ret < 0)
		hwlog_err("%s register callback error", __func__);

	hwlog_info("%s init end, list len: %d\n", __func__, MOTION_LIST_LEN);

	return 0;
}

static void release_motion_data(void)
{
	struct ext_motion_data_list *buf_node = NULL;
	struct ext_motion_data_list *tmp_node = NULL;

	mutex_lock(&idle_lock);
	list_for_each_entry_safe(buf_node, tmp_node,
				 &motion_idle_head->list, list) {
		/* data buffer in idle node already free */
		list_del(&buf_node->list);
		kfree(buf_node);
		buf_node = NULL;
	}
	kfree(motion_idle_head);
	motion_idle_head = NULL;

	list_for_each_entry_safe(buf_node, tmp_node,
				 &motion_data_head->list, list) {
		/* data buffer in idle node already free */
		list_del(&buf_node->list);
		kfree(buf_node);
		buf_node = NULL;
	}
	kfree(motion_data_head);
	motion_data_head = NULL;
	mutex_unlock(&idle_lock);
}

static int motion_to_sensor(int motion)
{
	int index;

	for (index = 0; index < MOTION_TBL_LEN; index++) {
		if (motion_sensor_tbl[index].motion_type == motion)
			return motion_sensor_tbl[index].sensor_type;
	}

	return -1;
}

static int sensor_to_motion(int sensor)
{
	int index;

	for (index = 0; index < MOTION_TBL_LEN; index++) {
		if (motion_sensor_tbl[index].sensor_type == sensor)
			return motion_sensor_tbl[index].motion_type;
	}

	return -1;
}

static int parse_cmd_result(unsigned char *payload, int len,
			    struct sensor_data *data)
{
	/* result struct TL: T(0x81)L(0x12) */
	int offset = 0;
	int data_ret;
	int ret;

	if (!payload || !data || len < MOTION_PAYLOAD_MIN_LEN)
		return -EINVAL;

	if (payload[offset++] != STRUCT_TAG)
		return -1;

	if (payload[offset++] != STRUCT_RESULT_LEN)
		return -1;

	/* sensor type TLV: T(0x02)L(0x01)V(sensor) */
	if (payload[offset++] != SENSOR_TYPE_TAG)
		return -1;

	if (payload[offset++] != TLV_LEN_ONE)
		return -1;

	data->sensor = payload[offset++];
	/* sensor id TLV: T(0x03)L(0x01)V(0x00) */
	if (payload[offset++] != SENSOR_ID_TAG)
		return -1;

	if (payload[offset++] != TLV_LEN_ONE)
		return -1;

	data->sensor_id = payload[offset++];
	/* result TLV: T(0x7f)L(0x04)V(ret) */
	if (payload[offset++] != RESULT_TAG)
		return -1;

	if (payload[offset++] != RESULT_LEN)
		return -1;

	ret = memcpy_s(&data_ret, sizeof(data_ret),
		       payload + offset, sizeof(int));
	if (ret < 0)
		return ret;

	data->result = c32_b2l(data_ret);
	return 0;
}

static int parse_motion_report(unsigned char *payload, int len,
			       struct sensor_data *data)
{
	/* result struct TL: T(0x81)L(0x12) */
	int offset = 0;

	if (!payload || !data || len < MOTION_DATA_PAYLOAD_LEN)
		return -EINVAL;

	/* sensor id TLV: T(0x01)L(0x01)V(0x00) */
	if (payload[offset++] != REPORT_SENSOR_TYPE_TAG)
		return -1;

	if (payload[offset++] != SENSOR_COMMON_LEN)
		return -1;

	data->sensor = payload[offset++];
	/* sensor id TLV: T(0x02)L(0x01)V(0x00) */
	if (payload[offset++] != REPORT_SENSOR_ID_TAG)
		return -1;

	if (payload[offset++] != SENSOR_COMMON_LEN)
		return -1;

	data->sensor_id = payload[offset++];

	/* data */
	if (payload[offset++] != REPORT_DATA_TAG)
		return -1;

	if (payload[offset++] < SENSOR_COMMON_LEN)
		return -1;
	data->data = payload[offset];
	return 0;
}

static struct ext_motion_data_list *get_idle_buf_node()
{
	   struct ext_motion_data_list *buf_node = NULL;

	mutex_lock(&idle_lock);
	/* whole buffer pool is empty */
	if (list_empty(&motion_idle_head->list)) {
		/* No Buf in idle list, sleep 2ms and retry */
		hwlog_err("%s buf list is full\n", __func__);
		mutex_unlock(&idle_lock);
		return NULL;
	}
	buf_node = list_entry(motion_idle_head->list.prev,
			      struct ext_motion_data_list, list);

	mutex_unlock(&idle_lock);
	return buf_node;
}

static int parse_motion_data(unsigned char command_id,
			     unsigned char *payload, int len)
{
	int ret;
	struct ext_motion_data_list *node = NULL;
	struct sensor_data data;
	int motion_type;

	if (command_id != MOTION_REPORT_COMMAND_ID)
		return -EINVAL;

	ret = parse_motion_report(payload, len, &data);
	if (ret < 0) {
		hwlog_err("%s parse sensor report data error, ret: %d",
			  __func__, ret);
		return ret;
	}

	motion_type = sensor_to_motion(data.sensor);
	if (motion_type < 0)
		return -EINVAL;

	node = get_idle_buf_node();
	if (!node)
		return -1;

	node->motion_result.motion = motion_type;
	node->motion_result.result = 1;
	node->motion_result.status = 0;
	node->motion_result.data_len = 1;
	node->motion_result.data[0] = data.data;

	mutex_lock(&idle_lock);
	list_move_tail(&node->list, &motion_data_head->list);
	mutex_unlock(&idle_lock);
	wake_up_interruptible(&read_wait);

	return 0;
}

static int parse_cmd(unsigned char service_id, unsigned char command_id,
		     unsigned char *data, int data_len)
{
	int ret;
	int motion_type;
	struct sensor_data motion;

	ret = parse_cmd_result(data, data_len, &motion);
	if (ret < 0) {
		hwlog_err("%s: parseResult ret error, ret:%d\n", __func__, ret);
		return -EINVAL;
	}

	if (!wait_resp.is_waiting) {
		/* data report deal */
		return 0;
	}
	if (wait_resp.service_id == service_id &&
	    wait_resp.command_id == command_id) {
		motion_type = sensor_to_motion(motion.sensor);
		if (motion_type != wait_resp.motion_type) {
			hwlog_err("%s: get sensor: 0x%02x", motion.sensor);
			return 0;
		}
		wait_resp.is_waiting = false;
		wait_resp.cmd_ret = motion.result;
		wait_resp.data_flag = true;
		wake_up_interruptible(&wait_resp.resp_wait_queue);
	}

	return 0;
}

static int motion_callback(unsigned char service_id, unsigned char command_id,
			   unsigned char *data, int data_len)
{
	if (service_id != MOTION_SERVICE_ID)
		return -EINVAL;
	   /* need data report command and check sensor type */
	if (command_id == MOTION_START_COMMAND_ID ||
	    command_id == MOTION_STOP_COMMAND_ID)
		return parse_cmd(service_id, command_id, data, data_len);

	if (command_id == MOTION_REPORT_COMMAND_ID)
		return parse_motion_data(command_id, data, data_len);

	return 0;
}

static bool data_ready(void)
{
	/* read buf list is empty, data not ready */
	if (list_empty(&motion_data_head->list))
		return false;

	return true;
}

static ssize_t ext_motion_read(struct file *file, char __user *buf,
			       size_t count, loff_t *pos)
{
	int missing;
	int status;
	int ret;
	struct ext_motion_data_list *read_buf_node = NULL;

	mutex_lock(&read_lock);
	hwlog_err("%s wait for read data begin, count: %d", __func__, count);
	ret = wait_event_interruptible(read_wait, data_ready());
	if (ret < 0) {
		mutex_unlock(&read_lock);
		hwlog_err("%s wait for read data error", __func__);
		return ret;
	}
	hwlog_err("%s wait for read data end", __func__);
	read_buf_node = list_first_entry(&motion_data_head->list,
					 struct ext_motion_data_list, list);
	mutex_unlock(&read_lock);
	read_buf_node->motion_result;
	missing = copy_to_user(buf + MHB_PKG_TIMESTAMP_BYTE,
			       &read_buf_node->motion_result,
			       sizeof(read_buf_node->motion_result));
	mutex_lock(&idle_lock);
	list_move_tail(&read_buf_node->list,
		       &motion_idle_head->list);
	mutex_unlock(&idle_lock);
	if (missing != 0) {
		hwlog_err("%s, cannot copy to user for read data", __func__);
		status = -EFAULT;
	} else {
		status = MHB_PKG_TIMESTAMP_BYTE +
			 sizeof(read_buf_node->motion_result);
	}

	hwlog_err("%s, end read, status: %d", __func__, status);

	return status;
}

static ssize_t ext_motion_write(struct file *file, const char __user *data,
				size_t len, loff_t *ppos)
{
	return -1;
}

static int make_sensor_payload(u8 sensor, unsigned char tlvs_len,
			       unsigned char *payload, int len)
{
	int offset = 0;

	if (!payload || len < MOTION_PAYLOAD_MIN_LEN)
		return -EFAULT;

	/* struct TL: T(0x01)L(0x0x) */
	payload[offset++] = STRUCT_TAG;
	payload[offset++] = tlvs_len;
	/* sensor type TLV: T(0x02)L(0x01)V(sensor) */
	payload[offset++] = SENSOR_TYPE_TAG;
	payload[offset++] = TLV_LEN_ONE;
	payload[offset++] = sensor;
	/* sensor id TLV: T(0x03)L(0x01)V(0x00)  -- sensor id default 0x00 */
	payload[offset++] = SENSOR_ID_TAG;
	payload[offset++] = TLV_LEN_ONE;
	payload[offset++] = DEFAULT_SENSOR_ID;

	return offset;
}

static int make_open_sensor_payload(u8 sensor, unsigned char *payload, int len)
{
	int offset = make_sensor_payload(sensor, OPEN_SENSOR_STRUCT_LEN,
					 payload, len);
	if (offset < 0)
		return offset;

	if (offset < len)
		payload[offset++] = SENSOR_OPTION_TAG;

	if (offset < len)
		payload[offset++] = TLV_LEN_ONE;

	if (offset < len)
		payload[offset++] = DEFAULT_OPTION;

	return offset;
}

static void init_resp(unsigned char service_id,
		      unsigned char command_id, int motion)
{
	mutex_lock(&wait_resp.resp_lock);
	wait_resp.is_waiting = true;
	wait_resp.data_flag = false;
	wait_resp.service_id = service_id;
	wait_resp.command_id = command_id;
	wait_resp.motion_type = motion;
	wait_resp.cmd_ret = -1;
	mutex_unlock(&wait_resp.resp_lock);
}

static void clear_resp(void)
{
	mutex_lock(&wait_resp.resp_lock);
	wait_resp.is_waiting = false;
	wait_resp.data_flag = false;
	wait_resp.motion_type = -1;
	wait_resp.cmd_ret = -1;
	mutex_unlock(&wait_resp.resp_lock);
}

static int start_motion(int motion)
{
	int ret;
	unsigned char payload[OPEN_SENSOR_CMD_LEN];
	int sensor;

	struct command start = {
		.service_id = MOTION_SERVICE_ID,
		.command_id = MOTION_START_COMMAND_ID,
		.send_buffer_len = OPEN_SENSOR_CMD_LEN,
		.send_buffer = payload,
	};
	hwlog_info("%s begin in, motion:0x%02x", __func__, motion);
	/* send start command */
	sensor = motion_to_sensor(motion);
	make_open_sensor_payload(sensor, payload, OPEN_SENSOR_CMD_LEN);
	/* wait & read resp */
	ret = send_command(MOTION_CHANNEL, &start, false, NULL);
	init_resp(MOTION_SERVICE_ID, MOTION_START_COMMAND_ID, motion);
	ret = wait_event_interruptible_timeout(
		wait_resp.resp_wait_queue, wait_resp.data_flag,
		msecs_to_jiffies(WAIT_RESP_TIMEOUT_MS));
	if (ret < 0) {
		hwlog_err("%s command error, ret： %d", __func__, ret);
		return ret;
	}
	if (ret == 0) {
		hwlog_err("%s wait response timeout", __func__);
		return -1;
	}
	ret = wait_resp.cmd_ret;
	clear_resp();
	hwlog_info("%s get result: %d", __func__, ret);

	if (ret == CMD_SUCCESS_CODE)
		return 0;

	return -1;
}

static int stop_motion(int motion)
{
	int ret;
	int sensor;
	unsigned char payload[SENSOR_CLOSE_CMD_LEN];

	struct command stop = {
		.service_id = MOTION_SERVICE_ID,
		.command_id = MOTION_STOP_COMMAND_ID,
		.send_buffer_len = SENSOR_CLOSE_CMD_LEN,
		.send_buffer = payload,
	};
	hwlog_err("%s begin in, motion:0x%02x", __func__, motion);
	sensor = motion_to_sensor(motion);
	make_sensor_payload(sensor, STRUCT_COMMON_LEN,
			    payload, SENSOR_CLOSE_CMD_LEN);
	ret = send_command(MOTION_CHANNEL, &stop, false, NULL);
	init_resp(MOTION_SERVICE_ID, MOTION_STOP_COMMAND_ID, motion);
	ret = wait_event_interruptible_timeout(
		wait_resp.resp_wait_queue, wait_resp.data_flag,
		msecs_to_jiffies(WAIT_RESP_TIMEOUT_MS));
	if (ret < 0) {
		hwlog_err("%s command error, ret： %d", __func__, ret);
		return ret;
	}
	if (ret == 0) {
		hwlog_err("%s wait response timeout", __func__);
		return -1;
	}
	ret = wait_resp.cmd_ret;
	clear_resp();
	hwlog_info("%s get result: %d", __func__, ret);

	if (ret == CMD_SUCCESS_CODE)
		return 0;

	return -1;
}

static int send_motion_cmd(int cmd, int motion)
{
	hwlog_err("%s send motion cmd", __func__);
	switch (cmd) {
	case MHB_IOCTL_MOTION_START:
		return start_motion(motion);
	case MHB_IOCTL_MOTION_STOP:
		return stop_motion(motion);
	default:
		return -ENOTTY;
	}
}

static long ext_motion_ioctl(struct file *file,
			     unsigned int cmd, unsigned long arg)
{
	int argvalue = 0;
	int ret;
	uintptr_t arg_tmp = (uintptr_t)arg;
	void __user *argp = (void __user *)arg_tmp;
	int sensor;

	switch (cmd) {
	case MHB_IOCTL_MOTION_START:
	case MHB_IOCTL_MOTION_STOP:
		break;
	default:
		hwlog_err("%s unknown cmd : %d\n", __func__, cmd);
		return -ENOTTY;
	}
	if (copy_from_user(&argvalue, argp, sizeof(argvalue)))
		return -EFAULT;

	/* check motion type */
	sensor = motion_to_sensor(argvalue);
	if (sensor < 0) {
		hwlog_err("%s cannot get motion by: %d\n", __func__, argvalue);
		return -EINVAL;
	}

	mutex_lock(&cmd_lock);
	ret = send_motion_cmd(cmd, argvalue);
	mutex_unlock(&cmd_lock);

	return ret;
}

static int ext_motion_open(struct inode *inode, struct file *file)
{
	hwlog_info("%s ok!\n", __func__);
	return 0;
}

static int ext_motion_release(struct inode *inode, struct file *file)
{
	hwlog_info("%s ok!\n", __func__);
	return 0;
}

static const struct file_operations ext_motion_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = ext_motion_read,
	.write = ext_motion_write,
	.unlocked_ioctl = ext_motion_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ext_motion_ioctl,
#endif
	.open = ext_motion_open,
	.release = ext_motion_release,
};

static struct miscdevice motionhub_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "motionhub",
	.fops = &ext_motion_fops,
};

static int __init ext_motionhub_init(void)
{
	int ret;

	hwlog_info("%s, begin init", __func__);
	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	ret = misc_register(&motionhub_miscdev);
	if (ret != 0)
		hwlog_err("cannot register miscdev err=%d\n", ret);

	init_waitqueue_head(&read_wait);
	init_waitqueue_head(&wait_resp.resp_wait_queue);
	mutex_init(&cmd_lock);
	mutex_init(&wait_resp.resp_lock);
	mutex_init(&idle_lock);
	mutex_init(&read_lock);
	init_motion_data_list();

	hwlog_info("%s, end init", __func__);
	return ret;
}

static void __exit ext_motionhub_exit(void)
{
	misc_deregister(&motionhub_miscdev);
	release_motion_data();
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(ext_motionhub_init);
module_exit(ext_motionhub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("External MotionHub driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
