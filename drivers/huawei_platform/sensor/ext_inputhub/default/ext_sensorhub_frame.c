/*
 * ext_sensorhub_frame.c
 *
 * code for make external sensorhub frame
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

#include "ext_sensorhub_frame.h"

#include <asm/memory.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/hwlog_kernel.h>

#include "securec.h"
#include "crc16.h"
#include "ext_sensorhub_route.h"
#include "ext_sensorhub_commu.h"
#include "ext_sensorhub_spi.h"
#include "ext_sensorhub_uart.h"

#define HWLOG_TAG ext_sensorhub_frame
HWLOG_REGIST();
#define SENSOR_SERVICE_ID  0x15
#define SENSOR_DATA_COMMAND_ID 0x81
#define SENSOR_TYPE_INDEX 2
#define FRAME_RETRY_TIME 2
#define PERIOD_RECORD_STAT_COUNT 60
#define STAT_TIMER_PERIOD 60
#define HANDSHAKE_RETRY_INTERVAL 3500
#define HANDSHAKE_RETRY_INTERVAL_S 3
#define COMMU_COMMON_COMMAND_LEN 2
#define COMMAND_UART_UPDATE_MODE 0x01
#define COMMAND_UART_EXIT_UPDATE_MODE 0x02
#define COMMAND_UART_SET_SPEED 0x03
#define COMMAND_UART_SET_SPEED_LEN 6
#define COMMAND_UPGRADE_SET_MODE 0x04
#define COMMAND_UPGRADE_SET_BOOT 0x05
#define COMMAND_UPGRADE_SET_MCU_RST 0x06
#define COMMAND_UPGRADE_RST_MCU 0x08
#define COMMAND_UPGRADE_DATA_LEN 3
#define COMMAND_UPGRADE_MAX_DATA_LEN 1024
#define MAX_REHANDSHAKE_RETRY_COUNT 20
#define MAX_REHANDSHAKE_RESET_COUNT 3

struct dubai_data {
	enum commu_driver driver;
	bool record_dubai;
	unsigned char service_id;
	unsigned char command_id;
	unsigned char sensor_type;
};

struct stat_count {
	unsigned int count;
	unsigned int read_count;
	unsigned int write_count;
	/* first recorded time in second */
	time_t first_record_ts;
	unsigned long uptime;
};

struct stat_record {
	struct rb_node node;
	unsigned short key; /* sidcid */
	struct stat_count period_count;
};

struct handshake {
	enum commu_driver driver;
	bool handshake_flag;
	struct work_struct handshake_work;
	int rehandshake_try_count;
	int write_fail_count;
	int write_fail_retry_count;
};

struct resp_recv {
	/* recode fsn when control not 0 */
	struct mutex decode_mutex;
	unsigned char expected_fsn;
	struct frame_decode_data frame_recv;
};

static const int time_factor = 1000;
static const int one_byte_bit_count = 8;
static struct handshake g_handshake[COMMU_DRIVER_COUNT];
static struct work_struct dubai_work;
static struct dubai_data g_dubai_data;
/* for received resp */
static struct resp_recv g_resp_recv[COMMU_DRIVER_COUNT];
/* mutex for red black tree */
static struct mutex rb_op_lock;
static int period_stat_record;
static struct timer_list stat_timer;
static enum commu_driver g_default_driver;
static struct commu_data *g_commu_data[COMMU_DRIVER_COUNT];
static bool g_frame_init = false;
static struct timer_list rehandshake_timer;
static unsigned int g_rehandshake_reset_count;
static unsigned int g_write_reset_count;
static bool enter_recovery;
static bool enter_erecovery;

#define RTC_SID     0x01
#define RTC_CID     0x06

/* red-black tree root node */
static struct rb_root stat_rb_root = RB_ROOT;

static inline unsigned short make_rb_key(unsigned char sid, unsigned char cid)
{
	return (sid << one_byte_bit_count) | cid;
}

/* get current time in second timestamp */
static inline time_t get_current_time(void)
{
	struct timespec ts = current_kernel_time();

	return ts.tv_sec;
}

static inline unsigned long get_uptime(void)
{
	struct timespec uptime;

	get_monotonic_boottime(&uptime);
	return uptime.tv_sec;
}

static struct stat_record *rbtree_search(struct rb_root *root,
					 unsigned short key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct stat_record *data =
		container_of(node, struct stat_record, node);

		if (data->key > key)
			node = node->rb_left;
		else if (data->key < key)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

/* insert data */
static int rbtree_insert(struct rb_root *root, struct stat_record *data)
{
	struct rb_node **node = &root->rb_node;
	struct rb_node *parent = NULL;

	/* find node to insert */
	while (*node) {
		struct stat_record *this =
		container_of(*node, struct stat_record, node);
		parent = *node;
		if (this->key > data->key)
			node = &((*node)->rb_left);
		else if (this->key < data->key)
			node = &((*node)->rb_right);
		else
			return -1;
	}

	/* insert the new node */
	rb_link_node(&data->node, parent, node);
	rb_insert_color(&data->node, root);

	return 0;
}

static void rbtree_period_record(void)
{
	struct stat_record *node = NULL;
	struct stat_record *next = NULL;

	mutex_lock(&rb_op_lock);
	period_stat_record = 1;
	mutex_unlock(&rb_op_lock);
	rbtree_postorder_for_each_entry_safe(node, next, &stat_rb_root, node) {
		if (!node)
			continue;

		if (node->period_count.count >=
		    PERIOD_RECORD_STAT_COUNT)
			pr_info("period statistics: sid|cid:0x%04x, count:%d, read:%d, write:%d, begin ts|uptime:%ld|%ld, current ts|uptime:%ld|%ld",
				node->key,
				node->period_count.count,
				node->period_count.read_count,
				node->period_count.write_count,
				node->period_count.first_record_ts,
				node->period_count.uptime,
				get_current_time(),
				get_uptime());

		kfree(node);
	}
	stat_rb_root.rb_node = NULL;
	mutex_lock(&rb_op_lock);
	period_stat_record = 0;
	mutex_unlock(&rb_op_lock);
}

static void init_stat_count(struct stat_record *record)
{
	record->period_count.count = 0;
	record->period_count.read_count = 0;
	record->period_count.write_count = 0;
}

static void increase_stat_count(struct stat_record *record, bool is_read)
{
	/* record first time */
	if (record->period_count.count == 0) {
		record->period_count.first_record_ts = get_current_time();
		record->period_count.uptime = get_uptime();
	}
	record->period_count.count++;
	if (is_read)
		record->period_count.read_count++;
	else
		record->period_count.write_count++;
}

static struct stat_record *record_stat(unsigned char sid, unsigned char cid,
				       bool is_read)
{
	struct stat_record *record_node = NULL;
	unsigned short key = make_rb_key(sid, cid);

	mutex_lock(&rb_op_lock);
	if (period_stat_record != 0)
		goto unlock;
	record_node = rbtree_search(&stat_rb_root, key);
	if (record_node) {
		increase_stat_count(record_node, is_read);
		goto unlock;
	}
	/* kfree after period record */
	record_node = kmalloc(sizeof(*record_node), GFP_KERNEL);
	if (!record_node)
		goto unlock;
	init_stat_count(record_node);
	increase_stat_count(record_node, is_read);
	record_node->key = key;
	record_node->node.rb_left = NULL;
	record_node->node.rb_right = NULL;
	rbtree_insert(&stat_rb_root, record_node);
unlock:
	mutex_unlock(&rb_op_lock);
	return record_node;
}

static void record_stat_work(struct work_struct *work)
{
	hwlog_info("%s get stat timer work", __func__);
	rbtree_period_record();
	stat_timer.expires = jiffies + STAT_TIMER_PERIOD * HZ;
	add_timer(&stat_timer);
}

static void stat_timer_callback(unsigned long arg)
{
	static DECLARE_WORK(stat_work, record_stat_work);

	schedule_work(&stat_work);
}

static void stat_timer_init(void)
{
	init_timer(&stat_timer);
	stat_timer.function = stat_timer_callback;
	stat_timer.expires = jiffies + STAT_TIMER_PERIOD * HZ;
	add_timer(&stat_timer);
}

static void stat_timer_exit(void)
{
	del_timer(&stat_timer);
}

static void convert_16_data_b2l(unsigned char *ptr_data_in,
				unsigned short *ptr_data_out)
{
	*ptr_data_out = ((unsigned short)(*ptr_data_in++) <<
	one_byte_bit_count);
	*ptr_data_out |= (unsigned short)(*ptr_data_in);
}

static void calc_frame_length(struct frame_request *request,
			      unsigned char *control,
			      unsigned short frame_index, int payload_bufsz)
{
	if (request->frame_count == 1) {
		/* none FSN */
		request->frames[frame_index].frame_length =
			FRAME_CTRL_HEADER_LEN  +
			FRAME_OPCODE_HEADER_LEN + payload_bufsz;
		*control = FRAME_CTL_NONE; /* none FSN */
	} else if ((frame_index + 1) == request->frame_count) {
		/* with FSN, left buffer size + frame header - tranid & crc */
		request->frames[frame_index].frame_length = payload_bufsz -
			SPLIT_FRAME_MAX_PAYLOAD_LEN * frame_index +
			FRAME_OPCODE_HEADER_LEN + FRAME_CTRL_HEADER_LEN +
			FRAME_FSN_HEADER_LEN;
		*control = FRAME_CTL_END; /* end FSN */
	} else {
		request->frames[frame_index].frame_length =
		FRAME_LEN_BODY_MAX_LEN;
		if (frame_index == 0)
			*control = FRAME_CTL_FIRST; /* first FSN */
		else
			*control = FRAME_CTL_DOING; /* doing FSN */
	}
	request->frames[frame_index].frame_total_length =
		request->frames[frame_index].frame_length +
		FRAME_TRAN_HEADER_LEN + FRAME_CRC16_HEADER_LEN;
}

static int encode_frame_crc(struct frame_request *request,
			    unsigned short offset, int i)
{
	unsigned short crc;
	unsigned short frame_crc = 0;
	int ret;

	crc = crc16_calc(request->frames[i].frame_body + FRAME_TRAN_ID_LEN,
			 request->frames[i].frame_total_length -
			 FRAME_TRAN_ID_LEN - FRAME_CRC16_HEADER_LEN);
	convert_16_data_b2l((unsigned char *)&crc, &frame_crc);
	ret = memcpy_s(request->frames[i].frame_body + offset,
		       MAX_FRAME_BUF_LEN - offset,
		       &frame_crc, sizeof(frame_crc));

	return ret;
}

static int encode_frame_header(unsigned char tid,
			       struct frame_request *request,
			       unsigned short *offset,
			       int payload_bufsz, int i)
{
	unsigned char control = 0;
	unsigned short frame_len = 0;
	int ret;

	/* 1. Fill tran id */
	request->frames[i].frame_body[*offset] = tid;
	*offset = *offset + 1;

	/* 1. Fill SOF */
	request->frames[i].frame_body[*offset] = START_OF_FRAME;
	*offset = *offset + 1;

	/* 2. Fill frame length:contain control, fsn(or not), opcode, payload */
	calc_frame_length(request, &control, i, payload_bufsz);

	convert_16_data_b2l((unsigned char *)&request->frames[i].frame_length,
			    &frame_len);
	ret = memcpy_s(request->frames[i].frame_body + *offset,
		       MAX_FRAME_BUF_LEN - *offset,
		       &frame_len, sizeof(unsigned short));
	if (ret < 0)
		return ret;

	*offset = *offset + sizeof(unsigned short);

	/* 3. Fill control and FSN */
	request->frames[i].frame_body[*offset] = control;
	*offset = *offset + 1;
	/* If have FSN, fill it; range[0 - 255], but current max is 4 */
	if (control != 0x00) {
		/* fsn */
		request->frames[i].frame_body[*offset] = i;
		*offset = *offset + 1;
	}
	/* 4. Fill OPCode, only first frame */
	if (i == 0) {
		request->frames[i].frame_body[*offset] = request->service_id;
		*offset = *offset + 1;
		request->frames[i].frame_body[*offset] = request->command_id;
		*offset = *offset + 1;
	}

	return 0;
}

static int encode_req_frame_body(unsigned char tid,
				 struct frame_request *request,
				 unsigned char *payload_buf, int payload_bufsz)
{
	unsigned short i = 0;
	unsigned short offset = 0;
	unsigned short payload_offset = 0;
	int ret = 0;

	for (i = 0; i < request->frame_count; i++) {
		/* memset to 0x00 */
		memset_s(request->frames[i].frame_body, MAX_FRAME_BUF_LEN,
			 0, MAX_FRAME_BUF_LEN);

		offset = 0;
		ret = encode_frame_header(
			tid, request, &offset, payload_bufsz, i);
		if (ret < 0)
			return ret;
		/* Fill payload */
		if (request->frame_count == 1) {
			ret = memcpy_s(request->frames[i].frame_body + offset,
				       MAX_FRAME_BUF_LEN - offset,
				       payload_buf, payload_bufsz);
			offset += payload_bufsz;
		} else if ((i + 1) == request->frame_count) {
			ret = memcpy_s(request->frames[i].frame_body + offset,
				       MAX_FRAME_BUF_LEN - offset,
				       payload_buf + payload_offset,
				       /* first - FRAME_OPCODE_HEADER_LEN */
				       payload_bufsz -
				       SPLIT_FRAME_MAX_PAYLOAD_LEN * i +
				       FRAME_OPCODE_HEADER_LEN);

			offset += (payload_bufsz -
				SPLIT_FRAME_MAX_PAYLOAD_LEN * i +
				FRAME_OPCODE_HEADER_LEN);
		} else if (i == 0) {
			/* first frame need contain opcode */
			ret = memcpy_s(request->frames[i].frame_body + offset,
				       MAX_FRAME_BUF_LEN - offset,
				       payload_buf + payload_offset,
				       SPLIT_FRAME_MAX_PAYLOAD_LEN -
				       FRAME_OPCODE_HEADER_LEN);
			payload_offset += SPLIT_FRAME_MAX_PAYLOAD_LEN -
			FRAME_OPCODE_HEADER_LEN;
			offset += SPLIT_FRAME_MAX_PAYLOAD_LEN -
			FRAME_OPCODE_HEADER_LEN;
		} else {
			/* frame except first, will not contain opcode */
			ret = memcpy_s(request->frames[i].frame_body + offset,
				       MAX_FRAME_BUF_LEN - offset,
				       payload_buf + payload_offset,
				       SPLIT_FRAME_MAX_PAYLOAD_LEN);
			payload_offset += SPLIT_FRAME_MAX_PAYLOAD_LEN;
			offset += SPLIT_FRAME_MAX_PAYLOAD_LEN;
		}
		if (ret < 0)
			return ret;

		/* Fill crc, from sof to payload, ie: exclude tran id and crc */
		ret = encode_frame_crc(request, offset, i);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int send_frame_data_retry(enum commu_driver driver,
				 unsigned char *buf, size_t len)
{
	int ret;
	int retry_count = 0;

	ret = commu_write(g_commu_data[driver], buf, len);
	while (ret < 0) {
		if (retry_count >= FRAME_RETRY_TIME)
			break;
		ret = commu_write(g_commu_data[driver], buf, len);
		retry_count++;
	}

	return ret;
}

static int handle_common_command(unsigned char *payload_buf, int payload_bufsz)
{
	int offset = 0;
	unsigned int speed;
	int ret = 0;

	if (!g_commu_data[UART_DRIVER] ||
	    payload_bufsz < COMMU_COMMON_COMMAND_LEN)
		return -EINVAL;

	if (payload_buf[offset++] != 0)
		return -EINVAL;

	switch (payload_buf[offset++]) {
	case COMMAND_UART_UPDATE_MODE:
		/* here to open uart */
		ext_sensorhub_commu_uart_open();
		set_uart_commu_mode(1);
		g_handshake[UART_DRIVER].handshake_flag = true;
		break;
	case COMMAND_UART_EXIT_UPDATE_MODE:
		/* rehandshake */
		g_handshake[UART_DRIVER].handshake_flag = false;
		set_uart_commu_mode(0);
		/* here to close uart */
		ext_sensorhub_commu_uart_close();
		schedule_work(&g_handshake[UART_DRIVER].handshake_work);
		break;
	case COMMAND_UART_SET_SPEED:
		if (payload_bufsz < COMMAND_UART_SET_SPEED_LEN)
			return -EINVAL;
		speed = *(unsigned int *)(payload_buf + offset);
		set_uart_speed(speed);
		break;
	case COMMAND_UPGRADE_SET_MODE:
		if (payload_bufsz <= offset)
			return -EINVAL;
		/* mode */
		if (payload_buf[offset] == 0) {
			/* rehandshake */
			g_handshake[SPI_DRIVER].handshake_flag = false;
			ret = ext_sensorhub_set_force_upgrade_mode(0);
			schedule_work(&g_handshake[SPI_DRIVER].handshake_work);
		} else {
			ret = ext_sensorhub_set_force_upgrade_mode(1);
			g_handshake[SPI_DRIVER].handshake_flag = true;
		}
		break;
	case COMMAND_UPGRADE_SET_BOOT:
		if (payload_bufsz <= offset)
			return -EINVAL;
		ret = ext_sensorhub_mcu_boot(payload_buf[offset]);
		break;
	case COMMAND_UPGRADE_SET_MCU_RST:
		if (payload_bufsz <= offset)
			return -EINVAL;
		ret = ext_sensorhub_mcu_reset(payload_buf[offset]);
		break;
	case COMMAND_UPGRADE_RST_MCU:
		ret = ext_sensorhub_reset();
		break;
	default:
		return -EINVAL;
	}

	if (ret < 0)
		return ret;

	return payload_bufsz;
}

static int handle_force_upgrade(unsigned char *payload_buf, int payload_bufsz)
{
	int ret;
	u8 *tmp_buff = NULL;
	u8 *tx_buff = NULL;
	int tx_len = (int)payload_buf[0];

	hwlog_info("%s tx_len: %d", __func__, tx_len);
	if (tx_len != 0) {
		tx_buff = kmalloc((tx_len + payload_bufsz - 1), GFP_KERNEL);
		if (tx_buff == NULL) {
			hwlog_warn("%s malloc fail", __func__);
			return -ENOMEM;
		}
		if (memset_s(tx_buff, (tx_len + payload_bufsz - 1), 0,
			     (tx_len + payload_bufsz - 1)) != EOK)
			return -EFAULT;

		tmp_buff = kmalloc((tx_len + payload_bufsz - 1), GFP_KERNEL);
		if (tmp_buff == NULL) {
			hwlog_warn("%s malloc fail", __func__);
			return -ENOMEM;
		}
		if (memset_s(tmp_buff, (tx_len + payload_bufsz - 1), 0,
			     (tx_len + payload_bufsz - 1)) != EOK)
			return -EFAULT;

		if (memcpy_s(tmp_buff, payload_bufsz - 1,
			     payload_buf + 1, payload_bufsz - 1) != EOK)
			return -EFAULT;
	} else {
		tmp_buff = payload_buf + 1;
	}
	ret = ext_sensorhub_spi_direct_write(tmp_buff, payload_bufsz - 1,
					     tx_len, tx_buff);
	hwlog_info("%s spi write result: %d", __func__, ret);
	if (ret == 0)
		ret = payload_bufsz;

	if (tx_len != 0) {
		notify_mcu_spi_irq((tx_buff + payload_bufsz - 1), tx_len);
		hwlog_info("%s notify mcu spi irq ok", __func__);
		kfree(tx_buff);
		tx_buff = NULL;
		kfree(tmp_buff);
		tmp_buff = NULL;
	}

	return ret;
}

/* if result == 0, do next */
static int handle_inner_cmd(unsigned char service_id, unsigned char command_id,
			    unsigned char *payload_buf, int payload_bufsz)
{
	int ret;

	/* 1. 1132 upgrade data */
	if (service_id == SERVICE_ID_COMMU &&
	    command_id == SERVICE_UPGRADE_1132) {
		hwlog_info("%s is a update 1132 cmd, send direct", __func__);
		if (get_uart_commu_mode() == 0) {
			hwlog_warn("%s has not changed uart mode", __func__);
			return -EBUSY;
		}
		/* here to open uart, COMMAND_UART_UPDATE_MODE already open */
		ext_sensorhub_commu_uart_open();
		ret = ext_sensorhub_uart_write(payload_buf, payload_bufsz);
		hwlog_info("%s uart write result: %d", __func__, ret);
		if (ret == 0) {
			ret = -EFAULT;
			hwlog_err("%s uart write result is 0", __func__);
		}
		return ret;
	}

	if (service_id == SERVICE_ID_COMMU &&
	    command_id == SERVICE_FORCE_UPGRADE_MCU) {
		hwlog_info("%s is force update mcu, send direct", __func__);
		if (get_spi_commu_mode() != 1) {
			hwlog_warn("%s has not changed spi mode", __func__);
			return -EBUSY;
		}
		return handle_force_upgrade(payload_buf, payload_bufsz);
	}

	if (service_id == SERVICE_ID_COMMU &&
	    command_id == SERVICE_COMMU_COMMON) {
		hwlog_info("%s is a common commu command", __func__);
		return handle_common_command(payload_buf, payload_bufsz);
	}

	return 0;
}

static void set_write_ret(int ret, enum commu_driver driver)
{
	if (ret >= 0) {
		g_handshake[driver].write_fail_retry_count = 0;
		g_handshake[driver].write_fail_count = 0;
	}
	if (g_handshake[driver].handshake_flag &&
	    (ret == -ETIME || ret == -ETIMEDOUT)) {
		g_handshake[driver].write_fail_count++;
	} else {
		g_handshake[driver].write_fail_count = 0;
	}
}

int send_frame_data(unsigned char tid,
		    unsigned char service_id, unsigned char command_id,
		    unsigned char *payload_buf, int payload_bufsz)
{
	struct frame_request request;
	int ret;
	int i;
	unsigned char frame_count;
	enum commu_driver driver = SDIO_DRIVER;

	request.frames = NULL;
	if (g_default_driver != SDIO_DRIVER) {
		if (tid == TRANSACTION_ID_AP_TO_MCU)
			driver = SPI_DRIVER;
		else
			driver = UART_DRIVER;
	}
	ret = handle_inner_cmd(service_id, command_id,
			       payload_buf, payload_bufsz);
	if (ret != 0) {
		hwlog_info("%s handle inner cmd return %d", __func__, ret);
		return ret;
	}

	if (!g_handshake[driver].handshake_flag &&
	    (service_id != SERVICE_ID_COMMU ||
	     command_id != SERVICE_MCU_HANDSHAKE_REQ)) {
		hwlog_err("%s handshake has not done", __func__);
		return -EREMOTEIO;
	}

	hwlog_debug("%s start in service_id = %d, command_id = %d\n", __func__,
		    service_id, command_id);
	request.service_id = service_id;
	request.command_id = command_id;
	/* div into frame, may multiple frames, if one frame, without fsn */
	/* uart driver only one frame */
	if (driver == UART_DRIVER) {
		frame_count = 1;
	} else if (payload_bufsz <=
		   FRAME_BUF_LEN - FRAME_HEADER_WITHOUT_FSN_LEN) {
		frame_count = 1;
	} else {
		/* calculate frame count, only first frame need opcode */
		frame_count = (payload_bufsz + FRAME_OPCODE_HEADER_LEN - 1) /
		SPLIT_FRAME_MAX_PAYLOAD_LEN + 1;
		/* check max count */
		if (frame_count > MAX_FRAME_COUNT)
			return -EINVAL;
	}

	request.frame_count = frame_count;
	request.frames = kmalloc_array(request.frame_count,
				       sizeof(struct frame_content),
				       GFP_KERNEL);
	if (!request.frames)
		return -ENOMEM;

	ret = encode_req_frame_body(tid, &request, payload_buf, payload_bufsz);
	if (ret < 0) {
		hwlog_err("%s, encode request frame error, ret:%d",
			  __func__, ret);
		return ret;
	}

	hwlog_debug("%s frame count: %d, service_id = %d, command_id = %d\n",
		    __func__, request.frame_count, service_id, command_id);
	/* send request to external sensorhub */
	for (i = 0; i < request.frame_count; ++i) {
		ret = send_frame_data_retry(
			driver,
			request.frames[i].frame_body,
			request.frames[i].frame_total_length);
		if (ret < 0) {
			hwlog_err("%s commu write error, driver:%d, ret:%d",
				  __func__, driver, ret);
			break;
		}
	}
	set_write_ret(ret, driver);
	kfree(request.frames);
	request.frames = NULL;
	return ret;
}

static int handle_debug_frame(unsigned char *payload_buf, int payload_bufsz,
			      debug_frame_handler handler)
{
	int ret;
	unsigned short crc;
	unsigned short frame_crc = 0;
	unsigned char *frame_buffer = NULL;
	unsigned char tid;
	enum commu_driver driver = SDIO_DRIVER;

	if (payload_bufsz <= 0 ||
	    payload_bufsz + FRAME_CRC16_HEADER_LEN > MAX_FRAME_BUF_LEN)
		return -EINVAL;

	frame_buffer = kmalloc(MAX_FRAME_BUF_LEN, GFP_KERNEL);
	memset_s(frame_buffer, MAX_FRAME_BUF_LEN, 0, MAX_FRAME_BUF_LEN);
	if (!frame_buffer)
		return -ENOMEM;

	/* Fill crc, from sof to payload, ie: exclude tran id and crc */
	crc = crc16_calc(payload_buf + FRAME_TRAN_ID_LEN,
			 payload_bufsz - FRAME_TRAN_ID_LEN);
	convert_16_data_b2l((unsigned char *)&crc, &frame_crc);

	ret = memcpy_s(frame_buffer, MAX_FRAME_BUF_LEN,
		       payload_buf, payload_bufsz);
	if (ret < 0)
		goto err;
	ret = memcpy_s(frame_buffer + payload_bufsz,
		       MAX_FRAME_BUF_LEN - payload_bufsz,
		       &frame_crc, sizeof(frame_crc));
	if (ret < 0)
		goto err;
	/* get tid */
	tid = frame_buffer[0];
	if (g_default_driver == SPI_DRIVER) {
		driver = SPI_DRIVER;
		if (tid == TRANSACTION_ID_AP_TO_BT)
			driver = UART_DRIVER;
	}
	hwlog_info("%s commu write debug, driver:%d", __func__, driver);
	ret = handler(frame_buffer, payload_bufsz + FRAME_CRC16_HEADER_LEN,
		      driver);
	if (ret < 0)
		hwlog_err("%s commu write error, driver:%d ret:%d", __func__,
			  driver, ret);

err:
	kfree(frame_buffer);
	frame_buffer = NULL;
	return ret;
}

static int debug_write(unsigned char *payload_buf,
		       unsigned short payload_bufsz, enum commu_driver driver)
{
	return send_frame_data_retry(driver, payload_buf, payload_bufsz);
}

int send_debug_frame(unsigned char *payload_buf, int payload_bufsz)
{
	return handle_debug_frame(payload_buf, payload_bufsz, debug_write);
}

int mock_debug_frame(unsigned char *payload_buf, int payload_bufsz)
{
	return handle_debug_frame(payload_buf, payload_bufsz,
				  decode_frame_resp);
}

static int handle_resp_tran_header(unsigned char *read_buf,
				   struct frame_decode_data *decode_data)
{
	int ret = -1;
	/* contian:control [fsn] + opcode + payload */
	unsigned short frame_length = 0;
	struct frame_tran_header *tran_header = NULL;

	do {
		if (!read_buf || !decode_data) {
			hwlog_err("%s parameter is invalid", __func__);
			break;
		}

		/* check transaction_id, sof */
		tran_header = (struct frame_tran_header *)(read_buf);
		if (!tran_header) {
			hwlog_err("%s, ptran header is NULL", __func__);
			break;
		}

		/* check dst:get bit4 - bit7, if 0x1x, is AP; and check SOF */
		if (((tran_header->tran_id & 0xF0) !=
		     TRANSACTION_ID_AP_IS_DST) ||
		    tran_header->sof != START_OF_FRAME) {
			hwlog_err("%s, dst isn't AP or SOF is error. %02x",
				  __func__, tran_header->tran_id);
			break;
		}

		/* convert frame length */
		convert_16_data_b2l(&tran_header->frame_length_msb,
				    &frame_length);

		/* frame total length > MAX_FRAME_BUF_LEN, is invalid */
		if ((frame_length + FRAME_TRAN_HEADER_LEN +
		     FRAME_CRC16_HEADER_LEN) > MAX_FRAME_BUF_LEN) {
			hwlog_err("%s,frame_length is error, %d",
				  __func__, frame_length);
			break;
		}
		decode_data->frame_length = frame_length;

		ret = 0;
	} while (0);

	return ret;
}

static int handle_resp_crc_header(unsigned char *read_buf,
				  unsigned short frame_length)
{
	int ret = -1;
	unsigned short crc_offset = 0;
	unsigned short crc_value = 0;
	unsigned short crc_check = 0;
	struct frame_crc16_header *crc_header_ptr = NULL;

	do {
		if (!read_buf)
			break;

		crc_offset = frame_length + FRAME_TRAN_HEADER_LEN;

		/* get CRC value */
		crc_header_ptr = (struct frame_crc16_header *)(read_buf +
			crc_offset);

		convert_16_data_b2l(&crc_header_ptr->crc16_msb, &crc_value);

		/* check CRC, read_buf contain tansaction_id, shift 1 byte */
		crc_check = crc16_calc(read_buf + FRAME_TRAN_ID_LEN,
				       (frame_length + FRAME_TRAN_HEADER_LEN -
					FRAME_TRAN_ID_LEN));
		if (crc_check != crc_value)
			break;

		/* CRC pass */
		ret = 0;
	} while (0);

	return ret;
}

static int handle_resp_opcode_header(unsigned char *read_buf,
				     unsigned short offset,
				     unsigned char *command_id,
				     unsigned char *service_id)
{
	int ret = -1;
	struct frame_opcode_header *opcode_ptr = NULL;

	do {
		if (!read_buf || !command_id || !service_id)
			break;

		opcode_ptr = (struct frame_opcode_header *)(read_buf + offset);
		*service_id = opcode_ptr->service_id;
		*command_id = opcode_ptr->command_id;

		ret = 0;
	} while (0);

	return ret;
}

static int handle_resp_control_none(unsigned char *read_buf,
				    struct frame_decode_data *raw_data)
{
	int ret = -1;
	unsigned char command_id = 0;
	unsigned char service_id = 0;
	unsigned short opcode_offset = 0;
	unsigned short payload_offset = 0;
	unsigned char *payload_ptr = NULL;

	if (!read_buf || !raw_data) {
		hwlog_err("%s, parameter is invalid", __func__);
		return ret;
	}

	/* handle opcode */
	opcode_offset = FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN;
	if (handle_resp_opcode_header(read_buf, opcode_offset,
				      &command_id, &service_id) != 0) {
		hwlog_err("%s, opcode_offset is error", __func__);
		return ret;
	}

	raw_data->service_id = service_id;
	raw_data->command_id = command_id;
	raw_data->payload_length =
		(raw_data->frame_length) - FRAME_CTRL_HEADER_LEN -
		FRAME_OPCODE_HEADER_LEN;

	/* copy payload, according to payload_length */
	payload_offset =
		FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN +
		FRAME_OPCODE_HEADER_LEN;
	payload_ptr = (unsigned char *)(read_buf + payload_offset);
	if (!payload_ptr) {
		hwlog_err("%s, payload_ptr is error", __func__);
		return ret;
	}
	ret = memcpy_s(raw_data->payload, MAX_FRAME_BUF_LEN,
		       payload_ptr,
		       ext_sensorhub_get_min(raw_data->payload_length,
					     MAX_FRAME_BUF_LEN));

	return ret;
}

static int handle_resp_fsn_header(unsigned char *read_buf,
				  unsigned char expected_fsn)
{
	int ret = -1;
	int fsn_offset = 0;
	unsigned char fsn_value = 0;
	struct frame_fsn_header *fsn_header_ptr = NULL;

	if (!read_buf)
		return ret;

	fsn_offset = FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN;

	fsn_header_ptr = (struct frame_fsn_header *)(read_buf +
		fsn_offset);

	ret = memcpy_s(&fsn_value, sizeof(fsn_value),
		       fsn_header_ptr, sizeof(fsn_value));

	/* current frame max connt is 5, so FSN (0 - 4) */
	if (ret < 0 || expected_fsn != fsn_value ||
	    fsn_value >= MAX_FRAME_COUNT)
		hwlog_err("%s fsn value invalid, fsn=%d",
			  __func__, fsn_value);

	return ret;
}

static int handle_resp_control_first(unsigned char *read_buf,
				     struct frame_decode_data *raw_data,
				     enum commu_driver driver)
{
	int ret = -1;
	unsigned char command_id = 0;
	unsigned short offset = 0;
	unsigned char *payload_ptr = NULL;

	if (!read_buf || !raw_data)
		return ret;

	g_resp_recv[driver].expected_fsn = 0;

	/* check FSN */
	if (handle_resp_fsn_header(read_buf, 0) != 0)
		return ret;

	/* hand opcode */
	offset = FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN +
	FRAME_FSN_HEADER_LEN;
	if (handle_resp_opcode_header(read_buf, offset,
				      &command_id,
				      &raw_data->service_id) != 0)
		return ret;

	raw_data->command_id = command_id;

	/* copy payload to raw_data->payload */
	offset = FRAME_HEADER_LEN - FRAME_CRC16_HEADER_LEN;
	raw_data->payload_length = (raw_data->frame_length) -
	FRAME_CTRL_HEADER_LEN - FRAME_FSN_HEADER_LEN -
	FRAME_OPCODE_HEADER_LEN;
	payload_ptr = (unsigned char *)(read_buf + offset);

	ret = memcpy_s(raw_data->payload, MAX_FRAME_BUF_LEN,
		       payload_ptr,
		       ext_sensorhub_get_min(
			       raw_data->payload_length,
			       MAX_FRAME_BUF_LEN));

	if (ret < 0)
		return ret;

	g_resp_recv[driver].expected_fsn++;
	return 0;
}

static int handle_resp_control_doing(unsigned char *read_buf,
				     struct frame_decode_data *raw_data,
				     enum commu_driver driver)
{
	int ret = -1;
	unsigned short offset = 0;
	unsigned short payload_len = 0;
	unsigned char *payload_ptr = NULL;

	if (!read_buf || !raw_data)
		return ret;

	/* check FSN */
	if (handle_resp_fsn_header(read_buf,
				   g_resp_recv[driver].expected_fsn) != 0)
		return ret;

	/* hand opcode */
	offset = FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN +
	FRAME_FSN_HEADER_LEN;

	/* copy payload to raw_data->payload, doing or end frame,
	 * payload only raw data, no opcode
	 */
	payload_len = raw_data->frame_length -
	FRAME_CTRL_HEADER_LEN - FRAME_FSN_HEADER_LEN;
	payload_ptr = (unsigned char *)(read_buf +
		(FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN +
			FRAME_FSN_HEADER_LEN));

	offset = raw_data->payload_length;
	ret = memcpy_s(raw_data->payload + offset,
		       MAX_FRAME_BUF_LEN - offset, payload_ptr,
		       ext_sensorhub_get_min(
			payload_len, MAX_FRAME_BUF_LEN - offset));

	if (ret < 0)
		return ret;
	/* change raw_data->payload_length */
	raw_data->payload_length += payload_len;
	g_resp_recv[driver].expected_fsn++;
	return 0;
}

static int handle_resp_control_end(unsigned char *read_buf,
				   struct frame_decode_data *raw_data,
				   enum commu_driver driver)
{
	int ret;

	ret = handle_resp_control_doing(read_buf, raw_data, driver);
	if (ret == 0) {
		if (raw_data->payload_length > MAX_FRAME_BUF_LEN)
			ret = -1;
	}

	return ret;
}

static int handle_resp_frame_data(unsigned char *read_buf,
				  struct frame_decode_data *raw_data,
				  enum commu_driver driver)
{
	int ret = -1;
	unsigned char control = 0;
	struct frame_ctrl_header *ctrl_header_ptr = NULL;

	do {
		if (!read_buf || !raw_data) {
			hwlog_err("%s, parameter is invalid", __func__);
			break;
		}

		/* Get control */
		ctrl_header_ptr =
		   (struct frame_ctrl_header *)(read_buf +
			FRAME_TRAN_HEADER_LEN);
		if (!ctrl_header_ptr) {
			hwlog_err("%s, ctrl_header_ptr is null", __func__);
			break;
		}

		control = (ctrl_header_ptr->control) & FRAME_FSN_CTL;
		raw_data->control = control;

		/* handle control */
		switch (raw_data->control) {
		case FRAME_CTL_NONE:
			ret = handle_resp_control_none(read_buf, raw_data);
			break;

		case FRAME_CTL_FIRST:
			ret = handle_resp_control_first(read_buf,
							raw_data,
							driver);
			break;

		case FRAME_CTL_DOING:
			ret = handle_resp_control_doing(read_buf,
							raw_data,
							driver);
			break;

		case FRAME_CTL_END:
			ret = handle_resp_control_end(read_buf,
						      raw_data,
						      driver);
			break;

		default:
			ret = -1;
			break;
		}

	} while (0);

	return ret;
}

static void record_dubai_data(struct frame_decode_data *raw_data)
{
	g_dubai_data.record_dubai = false;
	g_dubai_data.service_id = raw_data->service_id;
	g_dubai_data.command_id = raw_data->command_id;

	// check if sensor data and record sensor type
	if (raw_data->service_id == SENSOR_SERVICE_ID &&
	    raw_data->command_id == SENSOR_DATA_COMMAND_ID &&
	    raw_data->payload_length > SENSOR_TYPE_INDEX) {
		g_dubai_data.sensor_type = raw_data->payload[SENSOR_TYPE_INDEX];
		hwlog_info("%s get sensor:0x%02x data to record dubai",
			   __func__, g_dubai_data.sensor_type);
	} else {
		g_dubai_data.sensor_type = -1;
	}

	schedule_work(&dubai_work);
}

void notify_commu_wakeup(bool is_wakeup, enum commu_driver driver)
{
	g_dubai_data.record_dubai = is_wakeup;
	g_dubai_data.driver = driver;
}

void notify_mcu_force_upgrade_irq(void)
{
	u8 *buffer = NULL;
	int ret;
	int offset;
	struct ext_sensorhub_buf_list *resp_list = NULL;

	buffer = kmalloc(COMMAND_UPGRADE_DATA_LEN, GFP_KERNEL);
	if (!buffer)
		return;
	offset = 0;
	/* upgrade irq data */
	buffer[offset++] = 0x00;
	buffer[offset++] = 0x07;
	buffer[offset++] = 0x01;

	pr_info("%s direct mode irq data", __func__);
	/* resp_list free in ext_sensorhub_route_append after used */
	resp_list = kmalloc(sizeof(*resp_list), GFP_KERNEL);
	if (!resp_list) {
		kfree(buffer);
		return;
	}

	resp_list->service_id = SERVICE_ID_COMMU;
	resp_list->command_id = SERVICE_COMMU_COMMON;
	resp_list->read_cnt = COMMAND_UPGRADE_DATA_LEN;
	resp_list->buffer = buffer;
	/* buffer also kfree after used */
	ext_sensorhub_route_append(resp_list);
}

void notify_mcu_spi_irq(unsigned char *payload_buf, int payload_bufsz)
{
	int ret;
	int offset;
	struct ext_sensorhub_buf_list *resp_list = NULL;
	u8 *buffer = NULL;

	if (payload_bufsz < 0 || payload_bufsz > COMMAND_UPGRADE_MAX_DATA_LEN)
		return;

	buffer = kmalloc(payload_bufsz, GFP_KERNEL);
	if (!buffer)
		return;
	if (memset_s(buffer, payload_bufsz, 0, payload_bufsz) != EOK)
		return;
	if (memcpy_s(buffer, payload_bufsz, payload_buf, payload_bufsz) != EOK)
		return;

	pr_info("%s direct mode irq data", __func__);
	/* resp_list free in ext_sensorhub_route_append after used */
	resp_list = kmalloc(sizeof(*resp_list), GFP_KERNEL);
	if (!resp_list)
		return;

	for (int i = 0; i < payload_bufsz; i++)
		pr_info("%s payload_buf[%d] = 0x%02x", __func__,
			i, payload_buf[i]);

	resp_list->service_id = SERVICE_ID_COMMU;
	resp_list->command_id = SERVICE_FORCE_UPGRADE_MCU;
	resp_list->read_cnt = payload_bufsz;
	resp_list->buffer = buffer;
	/* buffer also kfree after used */
	ext_sensorhub_route_append(resp_list);
}

static void notify_commu_event(enum commu_driver driver,
			       enum ext_sensorhub_event event)
{
	if (driver == SDIO_DRIVER) {
		notify_event(event, TRANSACTION_ID_AP_TO_MCU);
		notify_event(event, TRANSACTION_ID_AP_TO_BT);
	}
	if (driver == SPI_DRIVER)
		notify_event(event, TRANSACTION_ID_AP_TO_MCU);

	if (driver == UART_DRIVER)
		notify_event(event, TRANSACTION_ID_AP_TO_BT);
}

static void close_bt_uart(enum commu_driver driver)
{
	if (driver != UART_DRIVER)
		return;
	ext_sensorhub_frame_close_uart();
}

/* check spi driver read data len */
static void check_spi_read_len(struct frame_decode_data *raw_data,
			       enum commu_driver driver)
{
	u16 read_len;

	if (driver != SPI_DRIVER)
		return;

	if (raw_data->payload_length >= sizeof(u16)) {
		convert_16_data_b2l(raw_data->payload, &read_len);
		pr_info("%s get spi read len:%d from handshake",
			__func__, read_len);
		set_spi_read_len(read_len);
	} else {
		pr_info("%s use default spi read len", __func__);
		set_spi_read_len(DEFAULT_SPI_READ_LEN);
	}
}

/* handle commu message and return if need return response */
static bool handle_commu_msg(struct frame_decode_data *raw_data,
			     enum commu_driver driver)
{
	/* check if is a handshake command */
	if (raw_data->service_id == SERVICE_ID_COMMU &&
	    raw_data->command_id == SERVICE_MCU_HANDSHAKE_REQ) {
		if (!get_commu_data(driver)) {
			pr_err("%s handshake get commu data error", __func__);
			return false;
		}
		if (!get_commu_data(driver)->active_handshake) {
			hwlog_debug("%s get handshake from mcu", __func__);
			schedule_work(&g_handshake[driver].handshake_work);
		} else {
			check_spi_read_len(raw_data, driver);
			/* ap active handshake, here receive resp */
			pr_info("%s handshake with success, driver:%d",
				__func__, driver);
			g_handshake[driver].handshake_flag = true;
			notify_commu_event(driver, COMMU_HANDSHAKE);
			/* have get handshake msg, close uart */
			close_bt_uart(driver);
		}
		return false;
	}

	return true;
}

static int malloc_resp(struct frame_decode_data *raw_data,
		       enum commu_driver driver)
{
	struct ext_sensorhub_buf_list *resp_list = NULL;

	if (!raw_data)
		return -EINVAL;

	/* record dubai */
	if (g_dubai_data.record_dubai && g_dubai_data.driver == driver) {
		hwlog_debug("%s will record dubai", __func__);
		record_dubai_data(raw_data);
	}
	record_stat(raw_data->service_id, raw_data->command_id, true);
	/* handle commu msg, if not data send for other module, just return */
	if (!handle_commu_msg(raw_data, driver))
		return 0;

	/* resp_list free in ext_sensorhub_route_append after used */
	resp_list = kmalloc(sizeof(*resp_list), GFP_KERNEL);
	if (!resp_list)
		return -ENOMEM;

	resp_list->service_id = raw_data->service_id;
	resp_list->command_id = raw_data->command_id;
	resp_list->read_cnt = raw_data->payload_length;
	resp_list->buffer = kmalloc(resp_list->read_cnt, GFP_KERNEL);

	if (memcpy_s(resp_list->buffer, resp_list->read_cnt,
		     raw_data->payload, resp_list->read_cnt) < 0)
		return -EFAULT;

	hwlog_debug("resp_list sid: %d, cid: %d",
		    resp_list->service_id, resp_list->command_id);

	ext_sensorhub_route_append(resp_list);

	return 0;
}

int decode_frame_resp(unsigned char *read_buf, unsigned short read_bufsz,
		      enum commu_driver driver)
{
	int ret;
	struct frame_decode_data *decode_data = NULL;

	if (read_bufsz <= 0)
		return -EINVAL;

	mutex_lock(&g_resp_recv[driver].decode_mutex);
	decode_data = &g_resp_recv[driver].frame_recv;
	ret = handle_resp_tran_header(read_buf, decode_data);
	if (ret < 0) {
		memset_s(decode_data, sizeof(struct frame_decode_data), 0,
			 sizeof(struct frame_decode_data));
		hwlog_err("check tran id error %s", __func__);
		goto err;
	}

	ret = handle_resp_crc_header(read_buf, decode_data->frame_length);
	if (ret < 0) {
		memset_s(decode_data, sizeof(struct frame_decode_data), 0,
			 sizeof(struct frame_decode_data));
		hwlog_err("check crc error %s", __func__);
		goto err;
	}

	ret = handle_resp_frame_data(read_buf, decode_data, driver);
	if (ret < 0) {
		memset_s(decode_data, sizeof(struct frame_decode_data), 0,
			 sizeof(struct frame_decode_data));
		hwlog_err("check frame data error %s", __func__);
		goto err;
	}

	/* a complete frame reveived,copy and append to data list */
	if (decode_data->control == FRAME_CTL_NONE ||
	    decode_data->control == FRAME_CTL_END) {
		malloc_resp(decode_data, driver);
		hwlog_debug("get one frame %s", __func__);
		/* reset global revice */
		memset_s(decode_data, sizeof(struct frame_decode_data), 0,
			 sizeof(struct frame_decode_data));
	}
	ret = 0;
err:
	mutex_unlock(&g_resp_recv[driver].decode_mutex);
	return ret;
}

static void convert_64_l2b(char *ptr_data_in, char *ptr_data_out)
{
	int i;

	for (i = 0; i < one_byte_bit_count; ++i)
		*ptr_data_out++ = *(ptr_data_in + (one_byte_bit_count - 1 - i));
}

static int get_rtc_time(void)
{
	unsigned char buff[4] = {0x01, 0x00, 0x02, 0x00};

	return send_frame_data(TRANSACTION_ID_AP_TO_MCU,
			      RTC_SID, RTC_CID,
			      buff, sizeof(buff));
}

static void handshake_resp_work(struct work_struct *work)
{
	int ret;
	long long int msec;
	long long int big_value = 0;
	struct timeval tv;
	struct handshake *handshake = container_of(work, struct handshake,
		handshake_work);
	unsigned char tid = TRANSACTION_ID_AP_TO_MCU;

	/* already handshake successfully */
	if (handshake->handshake_flag)
		return;
	if (handshake->driver == UART_DRIVER)
		tid = TRANSACTION_ID_AP_TO_BT;
	hwlog_info("%s begin to send handshake, driver:%d, tid:0x%02x",
		   __func__, handshake->driver, tid);
	do_gettimeofday(&tv);
	msec = (long long int)tv.tv_sec * time_factor +
	tv.tv_usec / time_factor;
	convert_64_l2b((char *)(&msec), (char *)(&big_value));
	/* send handshake resp */
	ret = send_frame_data(tid,
			      SERVICE_ID_COMMU, SERVICE_MCU_HANDSHAKE_REQ,
			      (unsigned char *)&big_value, sizeof(big_value));
	if (ret < 0) {
		/* send handshake failed, retry every 3S */
		hwlog_err("%s send handshake error, driver:%d, ret: %d",
			  __func__, handshake->driver, ret);
		hwlog_info("%s will retry handshake in timer, driver:%d",
			   __func__, handshake->driver);
		return;
	}
	/* send handshake message then close uart */
	close_bt_uart(handshake->driver);
	/* mcu active handshake, and send success */
	if (get_commu_data(handshake->driver) &&
	    !get_commu_data(handshake->driver)->active_handshake) {
		hwlog_info("%s has handshake with mcu success",
			   __func__);
		handshake->handshake_flag = true;
		notify_commu_event(handshake->driver, COMMU_HANDSHAKE);
	}

	/* get init RTC from MCU, just in sdio commu */
	if (get_commu_mode() == 0) {
		if (get_rtc_time() < 0) {
			hwlog_err("%s get init rtc from mcu error, ret: %d",
				  __func__, ret);
		} else {
			hwlog_info("%s get init rtc from mcu success",
				   __func__);
		}
	}
}

static bool handshake_reset_mcu(void)
{
	if (g_handshake[SPI_DRIVER].handshake_flag == false)
		g_handshake[SPI_DRIVER].rehandshake_try_count++;
	else
		g_handshake[SPI_DRIVER].rehandshake_try_count = 0;

	if (g_handshake[UART_DRIVER].handshake_flag == false)
		g_handshake[UART_DRIVER].rehandshake_try_count++;
	else
		g_handshake[UART_DRIVER].rehandshake_try_count = 0;

	if (g_handshake[SPI_DRIVER].handshake_flag &&
	    g_handshake[UART_DRIVER].handshake_flag)
		g_rehandshake_reset_count = 0;

	/* spi or uart rehandshake more than 30, then reset MCU */
	if (g_handshake[SPI_DRIVER].rehandshake_try_count >
	    MAX_REHANDSHAKE_RETRY_COUNT ||
	    g_handshake[UART_DRIVER].rehandshake_try_count >
		MAX_REHANDSHAKE_RETRY_COUNT) {
		hwlog_info("%s spi handshake try count max, spi:%d, uart:%d",
			   __func__,
			   g_handshake[SPI_DRIVER].rehandshake_try_count,
			   g_handshake[UART_DRIVER].rehandshake_try_count);
		g_handshake[SPI_DRIVER].rehandshake_try_count = 0;
		g_handshake[UART_DRIVER].rehandshake_try_count = 0;
		if (g_rehandshake_reset_count < MAX_REHANDSHAKE_RESET_COUNT) {
			g_rehandshake_reset_count++;
			ext_sensorhub_reset();
		} else {
			hwlog_info("%s handshake reset sensorhub continuous 3",
				   __func__);
		}
		return true;
	}
	return false;
}

static void write_reset_mcu(void)
{
	if (g_handshake[SPI_DRIVER].write_fail_count > 0)
		g_handshake[SPI_DRIVER].write_fail_retry_count++;
	else
		g_handshake[SPI_DRIVER].write_fail_retry_count = 0;

	if (g_handshake[UART_DRIVER].write_fail_count > 0)
		g_handshake[UART_DRIVER].write_fail_retry_count++;
	else
		g_handshake[UART_DRIVER].write_fail_retry_count = 0;

	if (g_handshake[UART_DRIVER].write_fail_count == 0 &&
	    g_handshake[UART_DRIVER].write_fail_count == 0)
		g_write_reset_count = 0;

	if (g_handshake[SPI_DRIVER].write_fail_retry_count >
	    MAX_REHANDSHAKE_RETRY_COUNT ||
	    g_handshake[UART_DRIVER].write_fail_retry_count >
	    MAX_REHANDSHAKE_RETRY_COUNT) {
		hwlog_info("%s spi write try count max, spi:%d, uart:%d",
			   __func__,
			   g_handshake[SPI_DRIVER].write_fail_retry_count,
			   g_handshake[UART_DRIVER].write_fail_retry_count);
		g_handshake[SPI_DRIVER].write_fail_retry_count = 0;
		g_handshake[UART_DRIVER].write_fail_retry_count = 0;
		if (g_write_reset_count < MAX_REHANDSHAKE_RESET_COUNT) {
			g_write_reset_count++;
			ext_sensorhub_reset();
		} else {
			hwlog_info("%s write reset sensorhub continuous 3",
				    __func__);
		}
	}
}

static int __init ext_early_parse_recovery_cmdline(char *p)
{
	if (p != NULL) {
		enter_recovery = (strncmp(p, "1",
			strlen("1")) == 0) ? true : false;
		hwlog_info("%s mode!(in ext sensorhub)\n", enter_recovery ?
			   "recovery" : "normal?");
	}

	return 0;
}
early_param("enter_recovery", ext_early_parse_recovery_cmdline);

static int __init ext_early_parse_erecovery_cmdline(char *p)
{
	if (p != NULL) {
		enter_erecovery = (strncmp(p, "1",
			strlen("1")) == 0) ? true : false;
		hwlog_info("%s mode!(in ext sensorhub)\n", enter_erecovery ?
			   "erecovery" : "normal?");
	}

	return 0;
}
early_param("enter_erecovery", ext_early_parse_erecovery_cmdline);

/* rehandshake for uart and spi */
static void rehandshake_timer_work(struct work_struct *work)
{
	hwlog_info("%s rehandshake handshake work", __func__);
	if (g_handshake[SPI_DRIVER].handshake_flag == false) {
		hwlog_info("%s spi need to rehandshake", __func__);
		g_handshake[SPI_DRIVER].driver = SPI_DRIVER;
		if (!schedule_work(&g_handshake[SPI_DRIVER].handshake_work)) {
			hwlog_warn("%s schedule spi handshake fail", __func__);
			cancel_work(&g_handshake[SPI_DRIVER].handshake_work);
		}
	}

	if (g_handshake[UART_DRIVER].handshake_flag == false) {
		hwlog_info("%s uart need to rehandshake", __func__);
		g_handshake[UART_DRIVER].driver = UART_DRIVER;
		if (!schedule_work(&g_handshake[UART_DRIVER].handshake_work)) {
			hwlog_warn("%s schedule uart handshake fail", __func__);
			cancel_work(&g_handshake[UART_DRIVER].handshake_work);
		}
	}
	if (enter_recovery || enter_erecovery) {
		hwlog_info("%s ap not in normal mode", __func__);
		return;
	}

	if (handshake_reset_mcu())
		return;

	write_reset_mcu();
}

static void rehandshake_timer_callback(unsigned long arg)
{
	static DECLARE_WORK(rehandshake_work, rehandshake_timer_work);

	hwlog_info("%s rehandshake timer callback in", __func__);
	schedule_work(&rehandshake_work);
	rehandshake_timer.expires = jiffies + HANDSHAKE_RETRY_INTERVAL_S * HZ;
	add_timer(&rehandshake_timer);
}

static void rehandshake_timer_init(void)
{
	hwlog_info("%s init timer", __func__);
	init_timer(&rehandshake_timer);
	rehandshake_timer.function = rehandshake_timer_callback;
	rehandshake_timer.expires = jiffies + 1 * HZ; // init state
	add_timer(&rehandshake_timer);
}

static void rehandshake_timer_exit(void)
{
	hwlog_info("%s delete timer", __func__);
	del_timer(&rehandshake_timer);
}

static void record_dubai_work(struct work_struct *work)
{
	hwlog_info("%s begin to record dubai log, sid:0x%02x, cid:0x%02x, sensor:0x%02x",
		   __func__, g_dubai_data.service_id, g_dubai_data.command_id,
		   g_dubai_data.sensor_type);
	HWDUBAI_LOGE("DUBAI_TAG_WATCH_MCU_WAKEUP",
		     "serviceId=%d commandId=%d type=%d",
		     g_dubai_data.service_id, g_dubai_data.command_id,
		     g_dubai_data.sensor_type);
	hwlog_info("%s record dubai log end", __func__);
}

int ext_sensorhub_frame_init(void)
{
	int i;
	int ret;

	mutex_init(&rb_op_lock);
	stat_timer_init();
	for (i = 0; i < COMMU_DRIVER_COUNT; ++i) {
		g_commu_data[i] = NULL;
		g_handshake[i].driver = i;
		g_handshake[i].rehandshake_try_count = 0;
		g_handshake[i].write_fail_count = 0;
		g_handshake[i].write_fail_retry_count = 0;
		INIT_WORK(&g_handshake[i].handshake_work, handshake_resp_work);
		mutex_init(&g_resp_recv[i].decode_mutex);
	}
	g_rehandshake_reset_count = 0;
	INIT_WORK(&dubai_work, record_dubai_work);

	ext_sensorhub_commu_init();
	/* default sdio commu driver */
	if (get_commu_mode() == 0) {
		pr_info("%s use mode 0 commu driver", __func__);
		g_default_driver = SDIO_DRIVER;
		g_commu_data[SDIO_DRIVER] = get_commu_data(SDIO_DRIVER);
		if (!g_commu_data[SDIO_DRIVER])
			return -EFAULT;
	} else {
		pr_info("%s use mode 1 commu drivers", __func__);
		g_default_driver = SPI_DRIVER;
		/* use spi and uart commu driver */
		g_commu_data[SPI_DRIVER] = get_commu_data(SPI_DRIVER);
		if (!g_commu_data[SPI_DRIVER])
			return -EFAULT;

		g_commu_data[UART_DRIVER] = get_commu_data(UART_DRIVER);
		if (!g_commu_data[UART_DRIVER])
			return -EFAULT;
	}

	/* init each used commu driver */
	for (i = 0; i < COMMU_DRIVER_COUNT; ++i) {
		if (!g_commu_data[i])
			continue;
		/* commu init */
		ret = commu_init(g_commu_data[i]);
		if (ret < 0)
			return -EFAULT;
		if (g_commu_data[i]->active_handshake) {
			pr_info("%s active handshake, driver:%d", __func__, i);
			schedule_work(&g_handshake[i].handshake_work);
		}
		if (g_commu_data[i]->driver == UART_DRIVER)
			rehandshake_timer_init();
	}
	g_frame_init = true;
	return 0;
}

void ext_sensorhub_frame_exit(void)
{
	int i;

	g_frame_init = false;
	stat_timer_exit();
	/* stop communication */
	for (i = 0; i < COMMU_DRIVER_COUNT; ++i) {
		if (g_commu_data[i]->driver == SPI_DRIVER)
			rehandshake_timer_exit();
		commu_exit(g_commu_data[i]);
		cancel_work_sync(&g_handshake[i].handshake_work);
	}

	ext_sensorhub_commu_exit();
	cancel_work_sync(&dubai_work);
}

void notify_mcu_reboot(enum commu_driver driver)
{
	g_handshake[driver].handshake_flag = false;
	/* after mcu reboot, ap need active handshake again */
	if (g_commu_data[driver] && g_commu_data[driver]->active_handshake) {
		pr_info("%s ap active handshake", __func__);
		schedule_work(&g_handshake[driver].handshake_work);
	}

	if (!g_frame_init) {
		pr_warn("%s frame has not inited, do nothing", __func__);
		return;
	}
	/* for spi driver, additional 1132 need handshake again */
	if (driver == SPI_DRIVER) {
		g_handshake[UART_DRIVER].handshake_flag = false;
		pr_info("%s active uart handshake", __func__);
		schedule_work(&g_handshake[UART_DRIVER].handshake_work);
	}
}

int max_send_data_len(void)
{
	if (get_commu_mode() == 0)
		return SDIO_MAX_SEND_BUFFER_LEN;

	return SPI_MAX_SEND_BUFFER_LEN;
}

/* mode: 0 default sdio */
int get_commu_mode(void)
{
	struct device_node *status_node = NULL;
	int status;
	static int ret;
	static int once;

	if (once)
		return ret;

	status_node = of_find_compatible_node(NULL, NULL,
					      "huawei,ext_sensorhub_status");
	if (!status_node) {
		hwlog_err("%s, can not find node ext_sensorhub_status node",
			  __func__);
		return 0;
	}

	status = of_property_read_u32(status_node, "commu-mode", &ret);
	if (status) {
		hwlog_err("%s, can't find property commu mode, status:%d, ret: %d\n",
			  __func__, status, ret);
		once = 1;
		return 0;
	}

	once = 1;
	return ret;
}

bool check_commu_handshake(unsigned char tid)
{
	/* default mode 0 only check sdio driver flag */
	if (get_commu_mode() == 0)
		return g_handshake[SDIO_DRIVER].handshake_flag;

	if ((tid & 0xF0) == (TRANSACTION_ID_AP_TO_MCU & 0xF0))
		return g_handshake[SPI_DRIVER].handshake_flag;

	if ((tid & 0xF0) == (TRANSACTION_ID_AP_TO_BT & 0xF0))
		return g_handshake[UART_DRIVER].handshake_flag;

	return false;
}

int ext_sensorhub_frame_close_uart(void)
{
	int ret;

	if (get_commu_mode() == 0) {
		pr_info("%s commu is 0, do nothing", __func__);
		return true;
	}

	if (!g_commu_data[UART_DRIVER]) {
		pr_err("%s uart driver get null", __func__);
		return false;
	}
	/* direct mode, cannot set */
	if (get_uart_commu_mode() == 1) {
		pr_warn("%s uart in direct mode, cannot close", __func__);
		return -EACCES;
	}

	ret = ext_sensorhub_commu_uart_close();
	pr_info("%s bt close uart, ret:%d", __func__, ret);

	return ret;
}

void relax_commu_wake_lock(void)
{
	if (g_commu_data[SDIO_DRIVER]) {
		pr_debug("%s relax sdio wake lock", __func__);
		__pm_relax(&g_commu_data[SDIO_DRIVER]->user_wake_lock);
	}

	if (g_commu_data[SPI_DRIVER]) {
		pr_debug("%s relax spi wake lock", __func__);
		__pm_relax(&g_commu_data[SPI_DRIVER]->user_wake_lock);
	}
}

void hold_commu_wake_lock(unsigned int msec)
{
	if (g_commu_data[SDIO_DRIVER]) {
		pr_debug("%s hold sdio wake lock", __func__);
		__pm_wakeup_event(&g_commu_data[SDIO_DRIVER]->user_wake_lock,
				  msec);
	}

	if (g_commu_data[SPI_DRIVER]) {
		pr_debug("%s hold spi wake lock", __func__);
		__pm_wakeup_event(&g_commu_data[SPI_DRIVER]->user_wake_lock,
				  msec);
	}
}
