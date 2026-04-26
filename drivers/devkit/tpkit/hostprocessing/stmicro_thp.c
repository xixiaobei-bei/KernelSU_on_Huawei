/*
 * Thp driver code for stmicro
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/syscalls.h>
#include <linux/inet.h>
#include "huawei_thp.h"

#define STMICRO_IC_NAME "stmicro"
#define THP_STMICRO_DEV_NODE_NAME "stmicro_thp"
#define WRITE_CHUNK 1024
#define READ_CHUNK (1024 * 8)
#define READ_CHUNK_FOR_BA80Y (1024 * 8)
#define ADDRESS_SIZE 4
#define ALIX_HW_ID_0 0x36
#define ALIX_HW_ID_1 0x50
#define ALIX_HW_ID_1_STU56A 0x48
#define ALIX_HW_ID_ADDRESS 0x20000000
#define ALIX_SYSTEM_RESET_ADDRESS 0x20000024
#define ALIX_FRAME_BUFFER_ADDRESS 0x20010000
#define ALIX_INTERRUPT_ADDRESS 0x20000029
#define PROJECT_ID_ADDRESS 0x00041800
#define PROJECT_ID_ADDRESS_STU56A 0x00043800
#define FLASH_SET_ADDRESS 0x20000068
#define ADDR_GPIO_DIR 0x20000032
#define VAL_GPIO_DIR 0x10
#define ADDR_GPIO_RES 0x20000034
#define VAL_GPIO_RES 0x02
#define ADDR_GPIO_REG2_FOR_BA80Y 0x2000003F
#define VAL_GPIO_REG2_FOR_BA80Y 0x07
#define ADDR_GPIO_REG2 0x2000003E
#define VAL_GPIO_REG2 0x70
#define ADDR_GPIO_REG0 0x2000003D
#define VAL_GPIO_REG0 0x30
#define ADDR_GPIO_SPI4_MASK 0x2000002D
#define VAL_GPIO_SPI4_MASK 0x02
#define ALIX_OPCODE_READ 0xFB
#define ALIX_OPCODE_WRITE 0xFA
#define ALIX_CMD_SCAN_MODE 0xA0
#define ALIX_SCAN_MODE_ACTIVE 0x00
#define ALIX_HEADER_SIGNATURE 0xA5
#define ALIX_HEADER_TYPE 0x3A
#define FIFO_EVENT_SIZE 8
#define ST_RBUFF_LEN 9
#define ST_WBUFF_LEN 6
#ifndef CONFIG_HUAWEI_THP_MTK
#define TIMEOUT_RESOLUTION 1
#else
#define TIMEOUT_RESOLUTION 5
#define FW_READY 0x03
#define ESD_EVENT 0xF2
#define ABNORMAL_EVENT_TIME 9
#define AOD_WINDOW_MIN_X 0
#define AOD_WINDOW_MIN_Y 0
#define AOD_WINDOW_MAX_X 1080
#define AOD_WINDOW_MAX_Y 2400
#endif
#define EVT_TYPE_STATUS_ECHO 0x01
#define EVT_ID_STATUS_UPDATE 0x43
#define SPI_WRITE_DELAY_US 10
#define SPI_READ_UX_BUFF_LEN 5
#define POLL_FOR_EVENT_CMD 0x87
#define FIFO_EVENT_LEN 9
#define FIFO_EVENT_LEN 9
#define POLL_FOR_EVENT_CHECK_0 0x00
#define POLL_FOR_EVENT_CHECK_1 0xFF
#define POLL_FOR_EVENT_CHECK_2 0xFF
#define CHECK_STEP 2
#define CHECK_SIZE_OFFSET 4
#define CHECK_WAIT_TIME 100
#define ADDRESS_SAFE_SIZE 1
#define ALIX_CMD_SCAN_ON 0x03
#define ALIX_SENSE_CMD_LEN 3
#define FRAME_HEADER_SIZE 60
#define FRAME_HEADER_INFO_LEN 52
#define HEADER_SIGNATURE_INDEX 0
#define HEADER_TYPE_INDEX 1
#define HEADER_CHECK_LEN 2
#define FORCE_NODE_INDEX 48
#define SENSE_NODE_INDEX 49
#define EACH_DATA_SIZE 2
#define FRAME_SAFE_SIZE 64
#define TAIL_FRAME_NUMBER_SIZE 2
#define CLEAR_IRQ_CMD 0x02
#define GESTURE_CMD_LEN 3
#define GESTURE_HEAD 0x53
#define GESTURE_CMD_0 0x02
#define GESTURE_CMD_1 0X05
#define SUSPEND_CMD_LEN 6
#define CLEAR_IRQ_CMD_LEN 1
#define SCAN_MODE_1 0x01
#define SCAN_MODE_CMD_LEN 2
#define SET_GESTURE_MODE_CMD 0xA2
#define GESTURE_MODE_DATA 0x03
#define DOUBLE_CLICK_CMD 0x20
#define LONG_PRESS_CMD 0x01
#define DEFAULT_VALUE_CMD 0x00
#define ST_CHIP_ID_LEN 10
#define ID_BUFF_SIZE 384
#define DISABLE_FLASH_SLEEP_CMD 0x08
#define ID_OFFSET 16
#define ID_CHECK_STEP 4
#define ID_HEAD 0x70
#define ID_HEAD_OFFSET 2
#define CHIP_DETECT_RETRY_TIMES 3
#define DOUBLE_TAP_FLAG 2
#define WAIT_FOR_SPI_BUS_READ_DELAY 5
#define FP_EVENT 0x50
#define KEEP_HIGH_BIT 0xF0
#define KEEP_LOW_BIT 0x0F
#define KEEP_HIGH8_BIT 0xFF00
#define KEEP_LOW8_BIT 0x00FF
#define CHECKSUM_LENGTH 7
#define GESTURE_STATUS_LEN 4
#define SENSE_OFF_LEN 3
#define GESTURE_LOWPOWER_LEN 2
#define AOD_CMD_LENGTH 11
#define FP_STATUS_CMD_LENGTH 3
#define ENABLE 0x01
#define DISABLE 0x00
#define SPI_RETRY 20
#define FW_RETRY_MAX 3
#define POWER_OFF 0
#define POWER_ON 1
#define WORK_IN_SUSPEND 1
#define POWER_OFF_IN_SUSPEND 0

static unsigned int g_st_thp_udfp_status;
#ifdef CONFIG_HUAWEI_THP_MTK
struct mutex poll_event_mutex;
#endif
static unsigned int g_power_off_staus;

static u8 *st_rbuff;
static u8 *st_wbuff;

enum st_ic_type {
	STU56A = 1,
	ST_BA80Y = 2,
};

static int spi_driver_write_read(struct spi_device *client, uint8_t *cmd,
	int cmd_length, uint8_t *read_data, int read_count);
static int spi_driver_write(struct spi_device *client, uint8_t *cmd,
	int cmd_length);
static int read_u8_ux(struct spi_device *client, uint8_t op_code,
	uint32_t address, uint8_t *read_buff, uint32_t read_cnt);
static int write_u8_ux(struct spi_device *client, uint8_t op_code,
	uint32_t address, uint8_t *write_buff, uint32_t write_cnt);
static int alix_read(struct spi_device *client, uint32_t address, uint8_t *data,
	uint32_t count);
static int alix_write(struct spi_device *client, uint32_t address,
	uint8_t *data, uint32_t count);
static int touch_driver_init(struct thp_device *tdev);
static int touch_driver_alloc_mem(void);
static int touch_driver_check_chip_id(struct spi_device *sdev);
static int touch_driver_chip_detect(struct thp_device *tdev);
static void touch_driver_release_mem(void);
static int read_project_id_in_chip(struct thp_device *tdev, uint8_t *data,
	uint32_t count);
static int touch_driver_get_project_id(struct thp_device *tdev, char *buf,
	unsigned int len);
static int thp_set_frame_size(struct thp_device *tdev, uint32_t *size);
static int touch_driver_get_frame(struct thp_device *tdev, char *frame_buf,
	unsigned int len);
static int touch_driver_resume(struct thp_device *tdev);
static int touch_driver_suspend_ap(struct thp_device *tdev);
static int touch_driver_suspend(struct thp_device *tdev);
#ifdef CONFIG_HUAWEI_THP_MTK
static int parse_event_info(const uint8_t *read_buff,
	struct thp_udfp_data *udfp_data, unsigned int len);
static void reset_ctrl(struct thp_device *tdev);
static int touch_driver_set_fp_status(struct thp_device *tdev, uint8_t status);
static void esd_abnormal_handle(struct thp_device *tdev);
static int aod_event_report_ctrl(struct thp_device *tdev, u8 status,
	struct thp_aod_window window);
#endif
static void touch_driver_exit(struct thp_device *tdev);
static int alix_sense_off(struct spi_device *client);
static int fw_write(struct spi_device *client, uint8_t *cmd, int cmd_length);
static int poll_for_event(struct spi_device *client, uint8_t *event_to_search,
	int event_bytes, uint8_t *read_data, int time_to_wait);
static int check_echo(struct spi_device *client, uint8_t *cmd, int size);

static int spi_driver_write_read(struct spi_device *client, uint8_t *cmd,
	int cmd_length, uint8_t *read_data, int read_count)
{
	int ret;
	int retry = 0;
	struct spi_message msg;
	/* 2 transfer used for read and write */
	struct spi_transfer transfer[2] = { {0}, {0} };
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return -EINVAL;
	}
	spi_message_init(&msg);
	transfer[0].len = cmd_length;
	transfer[0].tx_buf = cmd;
	transfer[0].rx_buf = NULL;
	spi_message_add_tail(&transfer[0], &msg);
	transfer[1].len = read_count;
	transfer[1].delay_usecs = SPI_WRITE_DELAY_US;
	transfer[1].tx_buf = NULL;
	transfer[1].rx_buf = read_data;
	spi_message_add_tail(&transfer[1], &msg);

	while (retry < SPI_RETRY) {
		ret = thp_bus_lock();
		if (ret < 0) {
			thp_log_err("%s:get lock failed:%d\n", __func__, ret);
			return ret;
		}
		ret = thp_spi_sync(client, &msg);
		thp_bus_unlock();
		if (ret < 0) {
			thp_log_err("%s:Failed to complete SPI transfer, error= %d\n",
				__func__, ret);
			if (cd->support_vendor_ic_type == ST_BA80Y)
				return ret;
			retry++;
			msleep(TIMEOUT_RESOLUTION);
			continue;
		}
		break;
	}
	if (cd->support_vendor_ic_type == ST_BA80Y)
		return 0;
	return ret;
}

static int spi_driver_write(struct spi_device *client, uint8_t *cmd,
	int cmd_length)
{
	int ret;
	struct spi_message msg;
	/* 1 transfer used for write */
	struct spi_transfer transfer[1] = { {0} };

	spi_message_init(&msg);
	transfer[0].len = cmd_length;
	transfer[0].delay_usecs = SPI_WRITE_DELAY_US;
	transfer[0].tx_buf = cmd;
	transfer[0].rx_buf = NULL;
	spi_message_add_tail(&transfer[0], &msg);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed:%d\n", __func__, ret);
		return ret;
	}
	ret = thp_spi_sync(client, &msg);
	thp_bus_unlock();
	if (ret < 0) {
		thp_log_err("%s:Failed to complete SPI transfer, error= %d\n",
			__func__, ret);
		return ret;
	}
	return 0;
}

static int read_u8_ux(struct spi_device *client, uint8_t op_code,
	uint32_t address, uint8_t *read_buff, uint32_t read_cnt)
{
	int err;
	int read_already = 0;
	int to_read_now;
	uint32_t address_now;
	struct thp_core_data *cd = thp_get_core_data();
	uint8_t write_buff[SPI_READ_UX_BUFF_LEN] = {0};
	uint8_t *temp_read_buff = cd->thp_dev->rx_buff;
	uint32_t read_chunk;

	thp_log_debug("%s:vendor_ic_type = %d\n", __func__,
		cd->support_vendor_ic_type);
	if (cd->support_vendor_ic_type == ST_BA80Y)
		read_chunk = READ_CHUNK_FOR_BA80Y;
	else
		read_chunk = READ_CHUNK;

	memset(temp_read_buff, 0, ADDRESS_SAFE_SIZE + read_chunk *
		sizeof(uint8_t));
	write_buff[0] = op_code;

	while (read_cnt > 0) {
		address_now = address + read_already;
		/* change address to net address, copy 4 bytes to buff + 1 */
		*(int *)(write_buff + 1) = htonl(address_now);
		to_read_now = (read_cnt > read_chunk) ? read_chunk : read_cnt;
		err = spi_driver_write_read(client, write_buff,
			ADDRESS_SIZE + ADDRESS_SAFE_SIZE, temp_read_buff,
			to_read_now + ADDRESS_SAFE_SIZE);
		if (err != 0) {
			thp_log_err("%s: spi transfer error\n", __func__);
			return err;
		}

		memcpy(read_buff + read_already, temp_read_buff +
			ADDRESS_SAFE_SIZE, to_read_now);
		read_already += to_read_now;
		read_cnt -= to_read_now;
	}
	return 0;
}

static int write_u8_ux(struct spi_device *client, uint8_t op_code,
	uint32_t address, uint8_t *write_buff, uint32_t write_cnt)
{
	int err;
	int written_already = 0;
	int to_write_now;
	uint32_t address_now;
	struct thp_core_data *cd = thp_get_core_data();
	uint8_t *temp_buff = cd->thp_dev->tx_buff;

	memset(temp_buff, 0, ADDRESS_SAFE_SIZE + ADDRESS_SIZE +
		WRITE_CHUNK * sizeof(uint8_t));
	temp_buff[0] = op_code;

	while (write_cnt > 0) {
		address_now = address + written_already;
		/* change address to net address, copy 4 bytes to buff + 1 */
		*(int *)(temp_buff + 1) = htonl(address_now);
		to_write_now = (write_cnt > WRITE_CHUNK) ? WRITE_CHUNK :
			write_cnt;
		memcpy(temp_buff + ADDRESS_SAFE_SIZE + ADDRESS_SIZE,
			write_buff + written_already, to_write_now);

		err = spi_driver_write(client, temp_buff, to_write_now +
			ADDRESS_SAFE_SIZE + ADDRESS_SIZE);
		if (err != 0) {
			thp_log_err("%s: spi transfer error\n", __func__);
			return err;
		}

		written_already += to_write_now;
		write_cnt -= to_write_now;
	}
	return 0;
}

static int alix_read(struct spi_device *client, uint32_t address, uint8_t *data,
	uint32_t count)
{
	if (!client || !data) {
		thp_log_err("%s:data is null\n", __func__);
		return -EINVAL;
	}
	return read_u8_ux(client, ALIX_OPCODE_READ, address, data, count);
}

static int alix_write(struct spi_device *client, uint32_t address,
	uint8_t *data, uint32_t count)
{
	if (!client || !data) {
		thp_log_err("%s:data is null\n", __func__);
		return -EINVAL;
	}
	return write_u8_ux(client, ALIX_OPCODE_WRITE, address, data, count);
}

#ifdef CONFIG_HUAWEI_THP_MTK
static int clear_irq(struct thp_core_data *cd)
{
	uint8_t cmd = CLEAR_IRQ_CMD;

	return alix_write(cd->sdev, ALIX_INTERRUPT_ADDRESS, &cmd, sizeof(cmd));
}
#endif

static int poll_for_event(struct spi_device *client, uint8_t *event_to_search,
	int event_bytes, uint8_t *read_data, int time_to_wait)
{
	int i;
	bool find = false;
	int retry = 0;
	int time_to_count;
	uint8_t cmd = POLL_FOR_EVENT_CMD;
#ifdef CONFIG_HUAWEI_THP_MTK
	uint8_t last_fifo_data[FIFO_EVENT_LEN] = {0};
	int every_byte_same;
	int same_event = 0;
#endif

	time_to_count = time_to_wait / TIMEOUT_RESOLUTION;
	while (!find && retry < time_to_count) {
		if (spi_driver_write_read(client, &cmd, sizeof(cmd), read_data,
			FIFO_EVENT_LEN) != 0) {
			thp_log_info("%s: SPI error, break\n", __func__);
			break;
		}
		if ((read_data[1] != POLL_FOR_EVENT_CHECK_0) &&
			(read_data[1] != POLL_FOR_EVENT_CHECK_1))
			thp_log_info("%s: Read Event: %*ph\n", __func__,
				FIFO_EVENT_LEN, read_data);
		find = true;
#ifdef CONFIG_HUAWEI_THP_MTK
		if ((event_to_search[0] & KEEP_HIGH_BIT) == FP_EVENT &&
			(read_data[1] & KEEP_HIGH_BIT) == FP_EVENT) {
			thp_log_info("%s: find event success\n", __func__);
			return NO_ERR;
		}
		if (read_data[1] == ESD_EVENT) {
			thp_log_info("%s: ESD EVENT, need reset\n", __func__);
			return ESD_EVENT;
		}
		for (i = 0; i < FIFO_EVENT_SIZE; i++) {
			if (last_fifo_data[i] != read_data[i + 1]) {
				every_byte_same = -1;
				same_event = 0;
				break;
			}
			every_byte_same = 0;
		}
		if (every_byte_same == 0) {
			same_event++;
			if (same_event > ABNORMAL_EVENT_TIME) {
				thp_log_info("%s:retry %d, same %d, reset\n",
					__func__, retry, same_event);
				return ESD_EVENT;
			}
		}
#endif
		for (i = 0; i < event_bytes; ++i) {
			if (event_to_search[i] != POLL_FOR_EVENT_CHECK_2 &&
				read_data[i + 1] != event_to_search[i]) {
				find = false;
				break;
			}
		}
#ifdef CONFIG_HUAWEI_THP_MTK
		for (i = 0; i < FIFO_EVENT_SIZE; i++)
			last_fifo_data[i] = read_data[i + 1];
#endif
		retry++;
		mdelay(TIMEOUT_RESOLUTION);
	}

	if ((retry >= time_to_count) && !find) {
		thp_log_err("%s: Could not fetch the event\n", __func__);
		return -ENODEV;
	} else if (find) {
		thp_log_info("%s:Found Event:%*ph\n", __func__, FIFO_EVENT_LEN,
			read_data);
		return 0;
	}
	thp_log_err("%s: spi communication error\n", __func__);
	return -ENODEV;
}

static int check_echo(struct spi_device *client, uint8_t *cmd, int size)
{
	int ret;
	int i;
	uint8_t event_to_search[FIFO_EVENT_SIZE] = {0};
	uint8_t *read_data = st_rbuff;
#ifdef CONFIG_HUAWEI_THP_MTK
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return -EINVAL;
	}
#endif
	if ((size + CHECK_SIZE_OFFSET) > FIFO_EVENT_SIZE)
		size = FIFO_EVENT_SIZE - CHECK_SIZE_OFFSET;
	event_to_search[0] = EVT_ID_STATUS_UPDATE;
	event_to_search[1] = EVT_TYPE_STATUS_ECHO;
	for (i = CHECK_STEP; i < size + CHECK_STEP; i++)
		event_to_search[i] = cmd[i - CHECK_STEP];
	thp_log_info("%s:event_to_search: %*ph\n", __func__, FIFO_EVENT_SIZE,
		event_to_search);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_lock(&poll_event_mutex);
#endif
	ret = poll_for_event(client, event_to_search, size + CHECK_STEP,
		read_data, CHECK_WAIT_TIME);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_unlock(&poll_event_mutex);
	if (ret == ESD_EVENT)
		esd_abnormal_handle(cd->thp_dev);
#endif
	if (ret != 0) {
		thp_log_err("%s: echo event not found\n", __func__);
		return ret;
	}
	thp_log_info("%s echo ok\n", __func__);
	return ret;
}

static int fw_write(struct spi_device *client, uint8_t *cmd, int cmd_length)
{
	int err;
	int retry = 0;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd || !client || !cmd || (cmd_length <= 0)) {
		thp_log_err("%s:input is invalid\n", __func__);
		return -EINVAL;
	}

	while (retry < FW_RETRY_MAX) {
		err = spi_driver_write(client, cmd, cmd_length);
		if (err != 0) {
			thp_log_err("%s: error writing firmware command %d\n",
				__func__, err);
			if (cd->support_vendor_ic_type == ST_BA80Y)
				return err;
			retry++;
			continue;
		}
		err = check_echo(client, cmd, cmd_length);
		if (err != 0) {
			thp_log_err("%s: could not get echo event %d\n",
				__func__, err);
			if (cd->support_vendor_ic_type == ST_BA80Y)
				return err;
			retry++;
			continue;
		}
		break;
	}
	if (cd->support_vendor_ic_type == ST_BA80Y)
		return 0;
	return err;
}

static int alix_sense_off(struct spi_device *client)
{
	int err;
	uint8_t *cmd = st_wbuff;

	memset(cmd, 0, ALIX_SENSE_CMD_LEN);
	cmd[0] = ALIX_CMD_SCAN_MODE;
	err = fw_write(client, cmd, ALIX_SENSE_CMD_LEN);
	if (err != 0) {
		thp_log_err("%s: write failed\n", __func__);
		return err;
	}
	thp_log_info("%s: sense off\n", __func__);
	return err;
}

static int thp_set_frame_size(struct thp_device *tdev, uint32_t *size)
{
	uint8_t force_node;
	uint8_t sense_node;
	uint16_t ms_node_data_size;
	struct thp_core_data *cd = thp_get_core_data();

	force_node = cd->st_force_node_default;
	sense_node = cd->st_sense_node_default;
	ms_node_data_size = force_node * sense_node;
	*size = EACH_DATA_SIZE * (ms_node_data_size + force_node + sense_node) +
		FRAME_SAFE_SIZE + TAIL_FRAME_NUMBER_SIZE;

	return 0;
}

static int touch_driver_get_frame(struct thp_device *tdev, char *frame_buf,
	unsigned int len)
{
	int ret;
	uint32_t frame_size;

	if ((tdev == NULL) || (frame_buf == NULL)) {
		thp_log_info("%s: have null ptr\n", __func__);
		return -ENOMEM;
	}

	memset(frame_buf, 0, len);
	ret = thp_set_frame_size(tdev, &frame_size);
	if (ret != 0) {
		thp_log_err("%s:get frame size error\n", __func__);
		goto exit;
	}
	thp_log_debug("%s:frame_size: %u\n", __func__, frame_size);

	ret = alix_read(tdev->thp_core->sdev, ALIX_FRAME_BUFFER_ADDRESS,
		frame_buf, frame_size);
	if (ret != 0)
		thp_log_err("%s: error reading the data frame %d\n",
			__func__, ret);
exit:
	if ((frame_buf[0] != ALIX_HEADER_SIGNATURE) ||
		(frame_buf[1] != ALIX_HEADER_TYPE))
		thp_log_err("%s: wrong header information found %02X %02X\n",
			__func__, frame_buf[0], frame_buf[1]);
	return ret;
}

static int touch_driver_wrong_touch(struct thp_device *tdev)
{
	if ((!tdev) || (!tdev->thp_core)) {
		thp_log_err("%s: input dev is null\n", __func__);
		return -EINVAL;
	}

	if (tdev->thp_core->support_gesture_mode) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		thp_log_info("%s, done\n", __func__);
	}
	return 0;
}

static int thp_st_chip_gesture_report(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	int err;
	uint8_t echo[GESTURE_CMD_LEN] = {GESTURE_HEAD, GESTURE_CMD_0,
		GESTURE_CMD_1};
	uint8_t cmd = CLEAR_IRQ_CMD;
	uint8_t *read_data = NULL;

	thp_log_info("%s: called\n", __func__);
	if (!tdev || !tdev->rx_buff || !gesture_wakeup_value) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}

	read_data = tdev->rx_buff;
	memset(read_data, 0, FIFO_EVENT_LEN);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_lock(&poll_event_mutex);
#endif
	err = poll_for_event(tdev->thp_core->sdev, echo, GESTURE_CMD_LEN,
		read_data, CHECK_WAIT_TIME);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_unlock(&poll_event_mutex);
#endif
	if (err == 0) {
		thp_log_info("found valid gesture type\n");
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
			tdev->thp_core->easy_wakeup_info.off_motion_on = false;
			*gesture_wakeup_value = TS_DOUBLE_CLICK;
		}
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
	} else {
		thp_log_err("%s: Error fetching the event for gesture %d\n",
			__func__, err);
	}
	if (alix_write(tdev->thp_core->sdev, ALIX_INTERRUPT_ADDRESS, &cmd,
		sizeof(cmd)) != 0)
		thp_log_err("%s: Error clearing the interrupt\n", __func__);

	return err;
}

#ifdef CONFIG_HUAWEI_THP_MTK
static int checksum(const uint8_t *event, uint32_t size)
{
	uint8_t cur;
	uint8_t parity = 0;
	/* shift operation used for calibration */
	uint8_t fw_parity = (event[size] & 0x40) >> 6;
	uint32_t i;

	for (i = 0; i < size; i++) {
		cur = event[i];
		while (cur) {
			/* 0x01 parity check */
			parity ^= 0x01;
			cur &= (cur - 1);
		}
	}
	if (parity != fw_parity) {
		thp_log_err("%s incorrect!\n", __func__);
		return -EINVAL;
	}
	return NO_ERR;
}

static int parse_event_info(const uint8_t *read_buff,
	struct thp_udfp_data *udfp_data, unsigned int len)
{
	if (len < FIFO_EVENT_LEN) {
		thp_log_err("%s: invalid length\n", __func__);
		return -EINVAL;
	}
	if (checksum(&read_buff[1], CHECKSUM_LENGTH)) {
		thp_log_err("%s: event info checksum error!\n", __func__);
		return -EINVAL;
	}
	/* Huawei format: Byte 0 = version,
	 * ST format: byte2[7:4] = version_major, byte2[3:0] = version,
	 * byte3[7:4] = version_patch
	 */
	udfp_data->tpud_data.version = ((read_buff[2] & KEEP_HIGH_BIT) << 16) +
		((read_buff[2] & KEEP_LOW_BIT) << 8) +
		((read_buff[3] & KEEP_HIGH_BIT));
	/* Huawei format: byte 1 = ud_fp_event,
	 * ST format: byte2[3:0] = ud_fp_event
	 */
	udfp_data->tpud_data.udfp_event = (read_buff[3] & KEEP_LOW_BIT);
	/* Huawei format: byte 8 = tp_x, byte 9 = tp_y
	 * ST format: byte4[7:0] = x_coor [7:0], byte5[7:4] = y_coor [3:0],
	 * byte5[3:0] = x_coor [11:8], byte6[7:0] = y_coor [11:4]
	 */
	udfp_data->tpud_data.tp_x = read_buff[4] +
		((read_buff[5] & KEEP_LOW_BIT) << 8);
	udfp_data->tpud_data.tp_y = (read_buff[5] >> 4) + (read_buff[6] << 4);
	/* Huawei format: byte 14 = aod_event, byte 15 = key_event
	 * ST format: byte1[3:2] = aod_event, byte7[7:4] = key_event
	 */
	udfp_data->aod_event = ((read_buff[1] & KEEP_LOW_BIT) >> 2);
	udfp_data->key_event = (read_buff[7] >> 4);

	thp_log_info("%s:version:%x, udfp:%d, x:%d, y:%d, aod:%d, key:%d\n",
		__func__, udfp_data->tpud_data.version,
		udfp_data->tpud_data.udfp_event, udfp_data->tpud_data.tp_x,
		udfp_data->tpud_data.tp_y,
		udfp_data->aod_event, udfp_data->key_event);
	return NO_ERR;
}

static void double_tap_handle(struct thp_core_data *cd,
	unsigned int *double_tap_wakeup_value)
{
	if (*double_tap_wakeup_value != DOUBLE_TAP_FLAG)
		return;
	thp_log_info("%s: found valid gesture type\n", __func__);
	mutex_lock(&cd->thp_wrong_touch_lock);
	if (cd->easy_wakeup_info.off_motion_on == true) {
		cd->easy_wakeup_info.off_motion_on = false;
		*double_tap_wakeup_value = TS_DOUBLE_CLICK;
	}
	mutex_unlock(&cd->thp_wrong_touch_lock);
}

static void esd_abnormal_handle(struct thp_device *tdev)
{
	int ret;
	uint8_t fw_ready_search = FW_READY;
	uint8_t read_data[FIFO_EVENT_LEN] = {0};
	uint8_t aod_report_enable = ENABLE;
	struct thp_aod_window window;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	if (!cd->suspended) {
		thp_log_info("%s: resumed, return\n", __func__);
		return;
	}
	thp_log_err("%s: called\n", __func__);
	reset_ctrl(tdev->thp_core->thp_dev);
	mutex_lock(&poll_event_mutex);
	ret = poll_for_event(tdev->sdev, &fw_ready_search,
		sizeof(fw_ready_search), read_data, CHECK_WAIT_TIME);
	mutex_unlock(&poll_event_mutex);
	if (ret) {
		thp_log_err("%s:fw ready flag not found %d\n", __func__, ret);
		return;
	}
	touch_driver_suspend_ap(tdev);
	if (cd->aod_event_report_status) {
		window.x0 = AOD_WINDOW_MIN_X;
		window.y0 = AOD_WINDOW_MIN_Y;
		window.x1 = cd->aod_window_max_x;
		window.y1 = cd->aod_window_max_y;
		aod_event_report_ctrl(tdev, aod_report_enable, window);
	}
}

static int touch_driver_get_event_info(struct thp_device *tdev,
	struct thp_udfp_data *udfp_data)
{
	int ret;
	uint8_t search = FP_EVENT;
	uint8_t read_data[FIFO_EVENT_LEN] = {0};

	mutex_lock(&poll_event_mutex);
	ret = poll_for_event(tdev->sdev, &search, sizeof(search), read_data,
		CHECK_WAIT_TIME);
	mutex_unlock(&poll_event_mutex);
	if (ret == ESD_EVENT)
		esd_abnormal_handle(tdev);
	if (ret) {
		thp_log_err("%s: event info not found, %d\n", __func__, ret);
		goto err;
	}
	ret = parse_event_info(read_data, udfp_data, FIFO_EVENT_LEN);
	if (ret) {
		thp_log_err("%s: parse_event_info fail, %d\n", __func__, ret);
		goto err;
	}
	double_tap_handle(tdev->thp_core, &(udfp_data->key_event));
err:
	if (clear_irq(tdev->thp_core))
		thp_log_err("%s: clear irq fail, %d\n", __func__, ret);
	return ret;
}
#endif

static int touch_driver_power_init(void)
{
	int ret;

	thp_log_info("%s:called\n", __func__);
	ret = thp_power_supply_get(THP_VCC);
	if (ret)
		thp_log_err("%s: fail to get vcc power\n", __func__);
	ret = thp_power_supply_get(THP_IOVDD);
	if (ret)
		thp_log_err("%s: fail to get iovdd power\n", __func__);

	return 0;
}

static int touch_driver_power_on(struct thp_device *tdev)
{
	int ret;

	thp_log_info("%s:called\n", __func__);
	if (!tdev || !tdev->gpios) {
		thp_log_err("%s: have null ptr\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->support_vendor_ic_type == STU56A) {
#ifndef CONFIG_HUAWEI_THP_MTK
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
		/* delay 1 ms after power ctrl iovdd */
		ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 1);
		if (ret)
			thp_log_err("%s:power ctrl iovdd fail, ret = %d\n",
				__func__, ret);
		/* delay 2 ms after power ctrl vcc */
		ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 2);
		if (ret)
			thp_log_err("%s:power ctrl vcc fail, ret = %d\n",
				__func__, ret);
		gpio_direction_output(tdev->gpios->rst_gpio, GPIO_HIGH);
		thp_time_delay(tdev->timing_config.boot_reset_after_delay_ms);
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
#else
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.cs_high);
		/* delay 1 ms after power ctrl iovdd */
		ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 1);
		if (ret)
			thp_log_err("%s:power ctrl iovdd fail, ret = %d\n",
				__func__, ret);
		/* delay 2 ms after power ctrl vcc */
		ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 2);
		if (ret)
			thp_log_err("%s:power ctrl vcc fail, ret = %d\n",
				__func__, ret);
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.reset_high);
		thp_time_delay(tdev->timing_config.boot_reset_after_delay_ms);
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.cs_low);
#endif
		return ret;
	}
	/* keep TP rst  high before LCD  reset high */
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 1); /* delay 1ms */
	if (ret)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 5); /* delay 5ms */
	if (ret)
		thp_log_err("%s:power ctrl iovdd fail\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_HIGH);
	return ret;
}

static int touch_driver_power_off(struct thp_device *tdev)
{
	int ret;

	thp_log_info("%s:called\n", __func__);
	if (!tdev || !tdev->gpios) {
		thp_log_err("%s: have null ptr\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->support_vendor_ic_type == ST_BA80Y) {
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
		gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
		mdelay(tdev->timing_config.suspend_reset_after_delay_ms);
		/* vcc power off need delay 1 ms */
		ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, 1);
		if (ret)
			thp_log_err("%s:power ctrl vcc fail\n", __func__);
		/* iovdd power off need delay 1 ms */
		ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, 1);
		if (ret)
			thp_log_err("%s:power ctrl iovdd fail\n", __func__);
		return ret;
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	mdelay(tdev->timing_config.suspend_reset_after_delay_ms);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, 1); /* delay1ms */
	if (ret)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, 1); /* delay1ms */
	if (ret)
		thp_log_err("%s:power ctrl iovdd fail\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
#else
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.reset_low);
	mdelay(tdev->timing_config.suspend_reset_after_delay_ms);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, 1); /* delay1ms */
	if (ret)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, 1); /* delay1ms */
	if (ret)
		thp_log_err("%s:power ctrl iovdd fail\n", __func__);
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.cs_low);
#endif
	return ret;
}

#ifdef CONFIG_HUAWEI_THP_MTK
static void reset_ctrl(struct thp_device *tdev)
{
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.reset_low);
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
	pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->mtk_pinctrl.reset_high);
}
#endif

static int touch_driver_resume(struct thp_device *tdev)
{
	int ret;
	uint8_t cmd = CLEAR_IRQ_CMD;

	if ((tdev == NULL) || (tdev->thp_core == NULL)) {
		thp_log_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	ret = alix_write(tdev->thp_core->sdev, ALIX_INTERRUPT_ADDRESS,
		&cmd, sizeof(cmd));
	if (ret != 0)
		thp_log_err("%s: error clearing the interrupt\n", __func__);

	if (is_pt_test_mode(tdev)) {
		thp_log_info("%s: pt test mode\n", __func__);
#ifdef CONFIG_HUAWEI_THP_MTK
		reset_ctrl(tdev);
#endif
	} else if (g_power_off_staus == WORK_IN_SUSPEND) {
		thp_log_info("%s: gesture or fingerprint mode\n", __func__);
		if (tdev->thp_core->gesture_from_sensorhub) {
#ifdef CONFIG_HUAWEI_SHB_THP
			ret = send_thp_driver_status_cmd(POWER_ON,
				tdev->thp_core->multi_panel_index,
				ST_CMD_TYPE_MULTI_TP_UD_STATUS);
#endif
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->pins_default);
			thp_log_info("%s: call sensorhub gestrue disable\n",
				__func__);
		}
#ifdef CONFIG_HUAWEI_THP_MTK
		reset_ctrl(tdev);
#else
		gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
		mdelay(1);
		gpio_direction_output(tdev->gpios->rst_gpio, GPIO_HIGH);
		if (tdev->thp_core->gesture_from_sensorhub)
			thp_set_irq_status(tdev->thp_core, THP_IRQ_ENABLE);
#endif
	} else {
		thp_log_info("%s: power on\n", __func__);
		ret = touch_driver_power_on(tdev);
		if (ret < 0) {
			thp_log_err("%s: power on failed\n", __func__);
			return ret;
		}
		if (tdev->thp_core->gesture_from_sensorhub)
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->pins_default);
	}
	return ret;
}

static int touch_driver_suspend_ap(struct thp_device *tdev)
{
	int ret;
	uint8_t *cmd = st_wbuff;
	struct spi_device *sdev = tdev->thp_core->sdev;
	/* double tap cmd, C0: address, 04 01: enable double tap */
	uint8_t cmd_double_tap[3] = { 0xC0, 0x04, 0x01 };
	/* single tap cmd, C0: address, 06 01: enable single tap */
	uint8_t cmd_single_tap[11] = { 0xC0, 0x06, 0x01, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	memset(cmd, 0, SUSPEND_CMD_LEN);
	cmd[0] = ALIX_CMD_SCAN_MODE;
	cmd[1] = SCAN_MODE_1;
	ret = fw_write(sdev, cmd, SCAN_MODE_CMD_LEN);
	if (ret != 0) {
		thp_log_err("%s:set scan mode fail\n", __func__);
		return ret;
	}

	if (tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) {
		thp_log_info("%s: enter gesture mode\n", __func__);
		ret = fw_write(sdev, cmd_double_tap,
			sizeof(cmd_double_tap));
		if (ret)
			thp_log_err("%s:set double tap cmd fail, %d\n",
					__func__, ret);
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
	}

	if ((tdev->thp_core->easy_wakeup_info.aod_mode == TS_TOUCH_AOD_OPEN) &&
		(tdev->thp_core->support_vendor_ic_type == ST_BA80Y)) {
		thp_log_info("%s: enter aod mode\n", __func__);
		ret = fw_write(sdev, cmd_single_tap,
			sizeof(cmd_single_tap));
		if (ret)
			thp_log_err("%s:set signle tap cmd fail, %d\n",
				__func__, ret);
	}

#ifdef CONFIG_HUAWEI_THP_MTK
	if (g_st_thp_udfp_status && tdev->thp_core->use_ap_gesture) {
		thp_log_info("%s: enable fp_status\n", __func__);
		ret = touch_driver_set_fp_status(tdev, ENABLE);
		if (ret)
			thp_log_err("%s: set_fp_status fail, ret %d\n",
				__func__, ret);
	}
#endif
	return ret;
}

static int touch_driver_suspend_ba80y(struct thp_device *tdev)
{
	int ret;
	uint8_t *cmd = st_wbuff;

	memset(cmd, 0, SUSPEND_CMD_LEN);
	cmd[0] = CLEAR_IRQ_CMD;
	ret = alix_write(tdev->thp_core->sdev, ALIX_INTERRUPT_ADDRESS,
		cmd, CLEAR_IRQ_CMD_LEN); /* clear irq */
	if (ret != 0)
		thp_log_err("clear irq failed\n");
	if (is_pt_test_mode(tdev)) {
		thp_log_info("enter pt test mode\n");
		ret = alix_sense_off(tdev->thp_core->sdev);
	} else if ((tdev->thp_core->easy_wakeup_info.aod_mode) ||
		(tdev->thp_core->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE)) {
		thp_log_info("gesture mode\n");
		g_power_off_staus = WORK_IN_SUSPEND;
		ret = touch_driver_suspend_ap(tdev);
#ifndef CONFIG_HUAWEI_THP_MTK
		if (tdev->thp_core->gesture_from_sensorhub) {
			thp_set_irq_status(tdev->thp_core, THP_IRQ_DISABLE);
#ifdef CONFIG_HUAWEI_SHB_THP
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->pins_idle);
			ret = send_thp_driver_status_cmd(POWER_OFF,
				tdev->thp_core->multi_panel_index,
				ST_CMD_TYPE_MULTI_TP_UD_STATUS);
#endif
			thp_log_info("ts call sensorhub gestrue enable\n");
		}
#endif
	} else {
		thp_log_info("ts power off\n");
		g_power_off_staus = POWER_OFF_IN_SUSPEND;
		ret = touch_driver_power_off(tdev);
	}
	if (ret)
		thp_log_err("ts suspend failed\n");
	return ret;
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int ret;
	uint8_t *cmd = st_wbuff;

	if ((tdev == NULL) || (tdev->thp_core == NULL) ||
		(tdev->thp_core->sdev == NULL)) {
		thp_log_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->support_vendor_ic_type == ST_BA80Y)
		return touch_driver_suspend_ba80y(tdev);
	memset(cmd, 0, SUSPEND_CMD_LEN);
	cmd[0] = CLEAR_IRQ_CMD;
	ret = alix_write(tdev->thp_core->sdev, ALIX_INTERRUPT_ADDRESS,
		cmd, CLEAR_IRQ_CMD_LEN); /* clear irq */
	if (ret != 0)
		thp_log_err("%s: clear irq failed\n", __func__);
	g_st_thp_udfp_status = thp_get_status(THP_STATUS_UDFP);
	if (is_pt_test_mode(tdev)) {
		thp_log_info("%s: pt test mode\n", __func__);
		ret = alix_sense_off(tdev->thp_core->sdev);
	} else if ((tdev->thp_core->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE) || g_st_thp_udfp_status ||
		(tdev->thp_core->aod_touch_status &&
		tdev->thp_core->use_ap_gesture)) {
		thp_log_info("%s: gesture or fingerprint mode\n", __func__);
		g_power_off_staus = WORK_IN_SUSPEND;
		ret = touch_driver_suspend_ap(tdev);
	} else {
		g_power_off_staus = POWER_OFF_IN_SUSPEND;
		if (tdev->thp_core->support_vendor_ic_type == STU56A) {
			thp_log_info("%s: no power off, only sense_off\n",
				__func__);
			return alix_sense_off(tdev->thp_core->sdev);
		}
		thp_log_info("%s: power off\n", __func__);
		ret = touch_driver_power_off(tdev);
	}
	if (ret)
		thp_log_err("%s: failed\n", __func__);
	return ret;
}

#ifdef CONFIG_HUAWEI_THP_MTK
static int lowpower_status_ctrl(struct thp_device *tdev, uint8_t status)
{
	uint8_t sense_off[SENSE_OFF_LEN] = { 0xA0, 0x00, 0x00 };
	uint8_t gesture[GESTURE_LOWPOWER_LEN] = { 0xA0, 0x01 };
	struct thp_core_data *cd = thp_get_core_data();

	if (cd->support_vendor_ic_type == STU56A)
		return NO_ERR;
	thp_log_info("%s in, status: %u\n", __func__, status);
	if (!cd->suspended) {
		thp_log_info("%s: resumed, not handle lp\n", __func__);
		return NO_ERR;
	}
	/* status : 0(exit LP, recover tp ud), 1(enter LP, sense_off) */
	if (status == 0) {
		thp_log_info("lowpower mode exit\n");
		if (fw_write(tdev->thp_core->sdev, gesture, sizeof(gesture))) {
			thp_log_err("%s: exit lp fail\n", __func__);
			return -EINVAL;
		}
		return NO_ERR;
	}
	thp_log_info("lowpower mode enter\n");
	if (fw_write(tdev->thp_core->sdev, sense_off, sizeof(sense_off))) {
		thp_log_err("%s: Error sense-off\n", __func__);
		return -EINVAL;
	}
	return NO_ERR;
}

static int aod_event_report_ctrl(struct thp_device *tdev, u8 status,
	struct thp_aod_window window)
{
	uint8_t aod_cmd[AOD_CMD_LENGTH] = { 0xC0, 0x06, 0x01 };
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return -EINVAL;
	}
	if (!cd->suspended) {
		thp_log_info("%s: resumed, return\n", __func__);
		return NO_ERR;
	}
	thp_log_info("%s status %u, x0,y0,x1,y1: (%u, %u), (%u, %u)\n",
		__func__, status, window.x0, window.y0, window.x1, window.y1);
	aod_cmd[2] = status ? ENABLE : DISABLE;
	aod_cmd[3] = (window.x0 & KEEP_LOW8_BIT);
	aod_cmd[4] = (window.x0 & KEEP_HIGH8_BIT) >> 8;
	aod_cmd[5] = (window.y0 & KEEP_LOW8_BIT);
	aod_cmd[6] = (window.y0 & KEEP_HIGH8_BIT) >> 8;
	aod_cmd[7] = (window.x1 & KEEP_LOW8_BIT);
	aod_cmd[8] = (window.x1 & KEEP_HIGH8_BIT) >> 8;
	aod_cmd[9] = (window.y1 & KEEP_LOW8_BIT);
	aod_cmd[10] = (window.y1 & KEEP_HIGH8_BIT) >> 8;
	if (fw_write(tdev->thp_core->sdev, aod_cmd, sizeof(aod_cmd))) {
		thp_log_err("Error %s aod_event_ctrl\n",
			(status ? "enable" : "disable"));
		return -EINVAL;
	}
	return NO_ERR;
}

static int touch_driver_set_fp_status(struct thp_device *tdev, uint8_t status)
{
	uint8_t cmd_tp_ud[FP_STATUS_CMD_LENGTH] = { 0xC0, 0x05, 0x01 };

	thp_log_info("st_notify_fp_status in, status: %u\n", status);
	/* cmd_tp_ud[2] is upfp event report ctrl */
	cmd_tp_ud[2] = status ? ENABLE : DISABLE;
	if (fw_write(tdev->thp_core->sdev, cmd_tp_ud, sizeof(cmd_tp_ud))) {
		thp_log_err("Error %s touch report\n",
			(status ? "enable" : "disable"));
		return -EINVAL;
	}
	return NO_ERR;
}
#endif
static int thp_communication_config(struct spi_device *sdev)
{
	int err;
	uint8_t cmd;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: called\n", __func__);
	cmd = VAL_GPIO_DIR;
	err = alix_write(sdev, ADDR_GPIO_DIR, &cmd, sizeof(cmd));
	if (err != 0)
		goto exit;

	cmd = VAL_GPIO_RES;
	err = alix_write(sdev, ADDR_GPIO_RES, &cmd, sizeof(cmd));
	if (err != 0)
		goto exit;

	if (cd->support_vendor_ic_type == ST_BA80Y) {
		cmd = VAL_GPIO_REG2_FOR_BA80Y;
		err = alix_write(sdev, ADDR_GPIO_REG2_FOR_BA80Y,
			&cmd, sizeof(cmd));
	} else {
		cmd = VAL_GPIO_REG2;
		err = alix_write(sdev, ADDR_GPIO_REG2, &cmd, sizeof(cmd));
	}
	if (err != 0)
		goto exit;

	cmd = VAL_GPIO_REG0;
	err = alix_write(sdev, ADDR_GPIO_REG0, &cmd, sizeof(cmd));
	if (err != 0)
		goto exit;

	cmd = VAL_GPIO_SPI4_MASK;
	err = alix_write(sdev, ADDR_GPIO_SPI4_MASK, &cmd, sizeof(cmd));
	if (err != 0)
		goto exit;
	mdelay(1); /* delay 1 ms, after config */
	return 0;
exit:
	thp_log_err("%s: config fail, cmd: 0x%02X\n", __func__, cmd);
	return err;
}

static int touch_driver_alloc_mem(void)
{
	thp_log_info("%s: called\n", __func__);
	st_rbuff = kzalloc(ST_RBUFF_LEN, GFP_KERNEL);
	st_wbuff = kzalloc(ST_WBUFF_LEN, GFP_KERNEL);
	if (!st_rbuff || !st_wbuff) {
		thp_log_err("%s: kzalloc failed\n", __func__);
		kfree(st_rbuff);
		st_rbuff = NULL;
		kfree(st_wbuff);
		st_wbuff = NULL;
		return -ENOMEM;
	}
	return 0;
}

static int touch_driver_check_chip_id(struct spi_device *sdev)
{
	int ret;
	uint8_t chip_id[ST_CHIP_ID_LEN];
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s:called\n", __func__);
	memset(chip_id, 0, sizeof(chip_id));
	ret = alix_read(sdev, ALIX_HW_ID_ADDRESS, chip_id,
		ST_CHIP_ID_LEN);
	if (ret != 0) {
		thp_log_err("%s: Error reading chip id\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s:chip_id:0x%02X%02X%d,fw version:0x%02X%02X\n",
		__func__, chip_id[0], chip_id[1], chip_id[2], chip_id[5],
		chip_id[4]);
	if (cd->support_vendor_ic_type == STU56A) {
		if ((chip_id[0] != ALIX_HW_ID_0) || (chip_id[1] !=
			ALIX_HW_ID_1_STU56A)) {
			thp_log_err("%s: chip id mismatch\n", __func__);
			return -EINVAL;
		}
		return 0;
	}
	if (chip_id[0] != ALIX_HW_ID_0 || chip_id[1] != ALIX_HW_ID_1) {
		thp_log_err("%s: chip id mismatch\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int chip_communication_retry(struct thp_device *tdev)
{
	int ret;

	thp_log_info("%s: called\n", __func__);
	ret = thp_communication_config(tdev->thp_core->sdev);
	if (ret)
		return ret;
	ret = touch_driver_check_chip_id(tdev->thp_core->sdev);
	if (ret)
		return ret;

	return 0;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	int ret;
	int i;

	thp_log_info("%s: called\n", __func__);
	if (!tdev || !tdev->thp_core || !tdev->thp_core->sdev) {
		thp_log_err("%s: have null ptr\n", __func__);
		return -EINVAL;
	}
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_init(&poll_event_mutex);
#endif
	ret = touch_driver_alloc_mem();
	if (ret)
		return -ENOMEM;

	ret = touch_driver_power_init();
	if (ret)
		goto err;
	/* check chip id for 3 times. if all failed, go to error */
	for (i = CHIP_DETECT_RETRY_TIMES; i > 0; --i) {
		ret = touch_driver_power_on(tdev);
		if (ret)
			continue;
		mdelay(100); /* delay 100 ms to wait sensor ready */

		ret = chip_communication_retry(tdev);
		if (!ret) {
			thp_log_info("%s: succeed\n", __func__);
			return 0;
		}
		ret = touch_driver_power_off(tdev);
		if (ret)
			thp_log_err("%s: power off failed\n", __func__);
	}
err:
	thp_log_err("%s: failed\n", __func__);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_destroy(&poll_event_mutex);
#endif
	touch_driver_release_mem();
	if (tdev->thp_core->fast_booting_solution) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	return -ENODEV;
}

static void touch_driver_release_mem(void)
{
	thp_log_info("%s: called\n", __func__);
	kfree(st_rbuff);
	st_rbuff = NULL;
	kfree(st_wbuff);
	st_wbuff = NULL;
}

static int thp_parse_frame_size_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	if (of_property_read_u32(thp_node, "st_force_node_default",
		&cd->st_force_node_default)) {
		thp_log_err("%s: force_node parse fail\n", __func__);
		return -EINVAL;
	}
	if (of_property_read_u32(thp_node, "st_sense_node_default",
		&cd->st_sense_node_default)) {
		thp_log_err("%s: sense_node parse fail\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: force node = %u, sense node = %u\n", __func__,
		cd->st_force_node_default, cd->st_sense_node_default);
	return NO_ERR;
}

#ifdef CONFIG_HUAWEI_THP_MTK
static void thp_parse_aod_click_window(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	if (of_property_read_u32(thp_node, "aod_window_max_x",
		&cd->aod_window_max_x)) {
		thp_log_err("%s: aod_window_max_x parse fail, use default\n",
			__func__);
		cd->aod_window_max_x = AOD_WINDOW_MAX_X;
	}
	if (of_property_read_u32(thp_node, "aod_window_max_y",
		&cd->aod_window_max_y)) {
		thp_log_err("%s: aod_window_max_y parse fail, use default\n",
			__func__);
		cd->aod_window_max_y = AOD_WINDOW_MAX_Y;
	}
	thp_log_info("%s: aod_window_max_x = %u, aod_window_max_y = %u\n",
		__func__, cd->aod_window_max_x, cd->aod_window_max_y);
}
#endif

static int thp_parse_feature_ic_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	if (of_property_read_string(thp_node, "multi_vendor_name",
		&cd->thp_dev->multi_vendor_name))
		cd->thp_dev->multi_vendor_name = "null";

	return NO_ERR;
}

static int touch_driver_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data();
	struct device_node *stmicro_node = NULL;

	thp_log_info("%s: called\n", __func__);
	if (!tdev || !cd) {
		thp_log_err("%s: have null ptr\n", __func__);
		return -EINVAL;
	}

	stmicro_node = of_get_child_by_name(cd->thp_node,
		THP_STMICRO_DEV_NODE_NAME);
	if (!stmicro_node) {
		thp_log_info("%s:node not config in dts\n", __func__);
		return -ENODEV;
	}
	rc = thp_parse_spi_config(stmicro_node, cd);
	if (rc)
		thp_log_err("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(stmicro_node, &tdev->timing_config);
	if (rc)
		thp_log_err("%s: timing config parse fail\n", __func__);

	rc = thp_parse_feature_config(stmicro_node, cd);
	if (rc)
		thp_log_err("%s: feature_config parse fail\n", __func__);

	rc = thp_parse_frame_size_config(stmicro_node, cd);
	if (rc)
		thp_log_err("%s: frame size config parse fail %d\n", __func__,
			rc);
	rc = thp_parse_feature_ic_config(stmicro_node, cd);
	if (rc)
		thp_log_err("%s: special_config parse fail\n", __func__);
#ifdef CONFIG_HUAWEI_THP_MTK
	if (cd->use_ap_gesture)
		thp_parse_aod_click_window(stmicro_node, cd);
#endif
	if (cd->support_gesture_mode) {
		cd->easy_wakeup_info.sleep_mode = TS_POWER_OFF_MODE;
		cd->easy_wakeup_info.easy_wakeup_gesture = false;
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		cd->easy_wakeup_info.off_motion_on = false;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
	}
	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	kfree(st_rbuff);
	st_rbuff = NULL;
	kfree(st_wbuff);
	st_wbuff = NULL;

	kfree(tdev->tx_buff);
	tdev->tx_buff = NULL;
	kfree(tdev->rx_buff);
	tdev->rx_buff = NULL;
	kfree(tdev);
	tdev = NULL;
}

static int read_project_id_in_chip(struct thp_device *tdev, uint8_t *data,
	uint32_t count)
{
	int err;
	uint8_t cmd = DISABLE_FLASH_SLEEP_CMD;
	uint8_t tmp = 0;

	err = alix_read(tdev->thp_core->sdev, FLASH_SET_ADDRESS, &tmp,
		sizeof(tmp));
	if (err != 0) {
		thp_log_err("%s:Error getting flash settings\n", __func__);
		return -ENODEV;
	}
	err = alix_write(tdev->thp_core->sdev, FLASH_SET_ADDRESS, &cmd,
		sizeof(cmd));
	if (err != 0) {
		thp_log_err("%s: Error disabling flash sleep\n", __func__);
		return -ENODEV;
	}
	if (tdev->thp_core->support_vendor_ic_type == STU56A)
		err = alix_read(tdev->thp_core->sdev, PROJECT_ID_ADDRESS_STU56A,
			data, count);
	else
		err = alix_read(tdev->thp_core->sdev, PROJECT_ID_ADDRESS,
			data, count);
	if (err != 0) {
		thp_log_err("%s: Error reading project ID\n", __func__);
		return -ENODEV;
	}
	err = alix_write(tdev->thp_core->sdev, FLASH_SET_ADDRESS, &tmp,
		sizeof(tmp));
	if (err != 0) {
		thp_log_err("%s: Error restoring flash settings\n", __func__);
		return -ENODEV;
	}
	thp_log_info("%s: succeed\n", __func__);
	return 0;
}

static int touch_driver_get_project_id(struct thp_device *tdev, char *buf,
	unsigned int len)
{
	int err;
	uint8_t data[ID_BUFF_SIZE];
	int i;
	int j;

	if (!tdev || !buf || !tdev->thp_core) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}

	err = read_project_id_in_chip(tdev, data, ID_BUFF_SIZE);
	if (err) {
		thp_log_err("%s: failed\n", __func__);
		return err;
	}

	for (i = ID_BUFF_SIZE - ID_OFFSET; i >= 0; i -= ID_CHECK_STEP) {
		if (data[i] == (uint8_t)len &&
			data[i + ID_HEAD_OFFSET] == ID_HEAD) {
			for (j = 0; j < len; j++)
				buf[j] = data[i + ID_CHECK_STEP + j];
			thp_log_info("%s:ProjectID:%*ph\n", __func__, len, buf);
			return 0;
		}
	}
	thp_log_err("%s: no project id in IC\n", __func__);
	return -EINVAL;
}

struct thp_device_ops stmicro_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
	.chip_wrong_touch = touch_driver_wrong_touch,
	.chip_gesture_report = thp_st_chip_gesture_report,
	.get_project_id = touch_driver_get_project_id,
#ifdef CONFIG_HUAWEI_THP_MTK
	.get_event_info = touch_driver_get_event_info,
	.tp_lowpower_ctrl = lowpower_status_ctrl,
	.tp_aod_event_ctrl = aod_event_report_ctrl,
#endif
};

static int __init touch_driver_module_init(void)
{
	int rc;
	struct thp_device *dev = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: called\n", __func__);
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		thp_log_err("%s: thp device malloc failed\n", __func__);
		return -ENOMEM;
	}
	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = STMICRO_IC_NAME;
	dev->dev_node_name = THP_STMICRO_DEV_NODE_NAME;
	dev->ops = &stmicro_dev_ops;
	if (cd && cd->fast_booting_solution) {
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
		thp_send_detect_cmd(dev, NO_SYNC_TIMEOUT);
		return 0;
	}
	rc = thp_register_dev(dev);
	if (rc) {
		thp_log_err("%s: register fail\n", __func__);
		goto err;
	}
	thp_log_info("%s: register success\n", __func__);
	return rc;
err:
	kfree(dev->tx_buff);
	dev->tx_buff = NULL;
	kfree(dev->rx_buff);
	dev->rx_buff = NULL;
	kfree(dev);
	dev = NULL;
	return rc;
}

static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s: called\n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);

