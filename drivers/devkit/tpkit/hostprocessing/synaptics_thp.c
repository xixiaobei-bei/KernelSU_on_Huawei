/*
 * Thp driver code for synaptics
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
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
#include "huawei_thp.h"

#include <linux/time.h>
#include <linux/syscalls.h>

#define SYNAPTICS_IC_NAME "synaptics"
#define THP_SYNA_DEV_NODE_NAME "synaptics_thp"
#define SYNA_FRAME_SIZE 1092

#define MESSAGE_HEADER_SIZE 4
#define MESSAGE_MARKER 0xa5
#define UBL_FN_NUMBER 0x35
#define MESSAGE_PADDING 0x5a
#define FRAME_LENGTH   (2*18*30)
#define MESSAGE_DATA_NUM 32
#define VCC_DELAY 12
#define IOVDD_DELAY 30
#define POWER_OFF_DELAY_FOR_3909 5
#define NO_DELAY 0
#define SYNA_CMD_LEN 5
#define SYNA_ONE_CMD_LEN 6
#define SYNA_CMD_GESTURE_LEN 8
#define DOUBLE_TAP_FLAG 2
#define SYNA_CMD_GESTURE_MAX 24
#define NEED_WORK_IN_SUSPEND 1
#define NO_NEED_WORK_IN_SUSPEND 0
#define FIRST_FRAME_USEFUL_LEN 2

#define BOOT_CONFIG_SIZE 8
#define BOOT_CONFIG_SLOTS 16
#define ID_PART_NUM_LEN 16
#define ID_BUILD_LEN 4
#define ID_WRITE_LEN 2
#define BOOT_DATA 2
#define BOOT_START 4
#define REFLASH_READ_LEN 9
#define IC_PROJECT_ID_START 4
#define REFLASH_CMD_LEN_LOW 0x06
#define REFLASH_CMD_LEN_HIGH 0x00
#define CHIP_DETECT_TMP_BUF 30
#define SYNA_COMMAMD_LEN 3

#define RMI_ADDR_FIRST 0x80
#define RMI_ADDR_SECOND 0xEE
#define TOUCH_EVENT_TYPE 0xff

#define SPI_READ_WRITE_SIZE 256
#define RMI_CMD_LEN 2
#define FRAME_HEAD_LEN 4
#define MOVE_8BIT 8
#define MOVE_16BIT 16
#define MOVE_24BIT 24
#define FRAME_CMD_LEN 4
#define DYNAMIC_CMD_LEN 6

#define CMD_SET_DYNAMIC_CONFIG 0x24
#define CMD_DYNAMIC_CONFIG_LEN 3
#define SYNA_ENTER_GUESTURE_MODE 1
#define SYNA_EXIT_GUESTURE_MODE 1
#define SYNA_RETRY_TIMES 5
#define TOUCH_REPORT_CONFIG_SIZE 128
#define GESTURE_REPORT_SIZE 7
#define SYNA_RESPONSE_LEN 10
#define SPI_RETRY_TIMES 20
#define SPI_DELAY_MS 5

#define WAIT_FOR_SPI_BUS_READ_DELAY 5
#define SUPPORT_GET_FRAME_READ_ONCE 1
#define TOUCH_GESTURE_CMD 4

#define THP_GESTURE_DOUBLE_CLICK (1 << 1)
#define THP_GESTURE_FINGER (1 << 3)

#define FP_VALID_AREA_FINGER_DOWN (1 << 0)
#define FP_VALID_AREA_FINGER_UP (1 << 1)
#define FP_CORE_AREA_FINGER_DOWN (1 << 2)
#define FP_CORE_AREA_FINGER_UP (1 << 3)

#define SYNA_GESTURE_ONE_CMD 0x20
#define SYNA_SINGLE_CLICK_ONE_CMD 0x10
#define SYNA_FINGER_ONE_CMD 0x08
#define DEBUG_OPEN_VALUE 0x64
#define DEBUG_LOG_LENGTH 102

enum dynamic_config_id {
	DC_UNKNOWN = 0x00,
	DC_NO_DOZE,
	DC_DISABLE_NOISE_MITIGATION,
	DC_INHIBIT_FREQUENCY_SHIFT,
	DC_REQUESTED_FREQUENCY,
	DC_DISABLE_HSYNC,
	DC_REZERO_ON_EXIT_DEEP_SLEEP,
	DC_CHARGER_CONNECTED,
	DC_NO_BASELINE_RELAXATION,
	DC_IN_WAKEUP_GESTURE_MODE,
	DC_STIMULUS_FINGERS,
	DC_GRIP_SUPPRESSION_ENABLED,
	DC_ENABLE_THICK_GLOVE,
	DC_ENABLE_LOZE = 216,
	DC_SCENE_SWITCH = 0xCB,
};

enum status_code {
	STATUS_IDLE = 0x00,
	STATUS_OK = 0x01,
	STATUS_BUSY = 0x02,
	STATUS_CONTINUED_READ = 0x03,
	STATUS_RECEIVE_BUFFER_OVERFLOW = 0x0c,
	STATUS_PREVIOUS_COMMAND_PENDING = 0x0d,
	STATUS_NOT_IMPLEMENTED = 0x0e,
	STATUS_ERROR = 0x0f,
	STATUS_INVALID = 0xff,
};

enum report_type {
	REPORT_IDENTIFY = 0x10,
	REPORT_TOUCH = 0x11,
	REPORT_DELTA = 0x12,
	REPORT_RAW = 0x13,
	REPORT_PRINTF = 0x82,
	REPORT_STATUS = 0x83,
	REPORT_FRAME = 0xC0,
	REPORT_HDL = 0xfe,
};

enum report_touch_type {
	NO_GESTURE_DETECT = 0x00,
	DOUBLE_TAP,
	SWIPE,
	SINGLE_TAP = 0x80,
};

enum boot_mode {
	MODE_APPLICATION = 0x01,
	MODE_BOOTLOADER = 0x0b,
};

enum command {
	CMD_GET_BOOT_INFO = 0x10,
	CMD_READ_FLASH = 0x13,
	CMD_RUN_BOOTLOADER_FIRMWARE = 0x1f,
};

enum syna_ic_type {
	SYNA3909 = 1,
};

struct syna_tcm_identification {
	unsigned char version;
	unsigned char mode;
	unsigned char part_number[ID_PART_NUM_LEN];
	unsigned char build_id[ID_BUILD_LEN];
	unsigned char max_write_size[ID_WRITE_LEN];
};
struct syna_tcm_boot_info {
	unsigned char version;
	unsigned char status;
	unsigned char asic_id[BOOT_DATA];
	unsigned char write_block_size_words;
	unsigned char erase_page_size_words[BOOT_DATA];
	unsigned char max_write_payload_size[BOOT_DATA];
	unsigned char last_reset_reason;
	unsigned char pc_at_time_of_last_reset[BOOT_DATA];
	unsigned char boot_config_start_block[BOOT_DATA];
	unsigned char boot_config_size_blocks[BOOT_DATA];
	unsigned char display_config_start_block[BOOT_START];
	unsigned char display_config_length_blocks[BOOT_DATA];
	unsigned char backup_display_config_start_block[BOOT_START];
	unsigned char backup_display_config_length_blocks[BOOT_DATA];
	unsigned char custom_otp_start_block[BOOT_DATA];
	unsigned char custom_otp_length_blocks[BOOT_DATA];
};

static unsigned char *tx_buf;
static unsigned char *spi_read_buf;
static unsigned char *spi_write_buf;

static struct spi_transfer *spi_xfer;
static unsigned int get_project_id_flag;
static unsigned int need_power_off;
#ifdef CONFIG_HUAWEI_THP_MTK
static struct udfp_mode_status ud_mode_status;
#endif

static int pt_mode_set_for_3909(const struct thp_device *tdev);

static int touch_driver_spi_alloc_mem(struct spi_device *client,
	unsigned int count, unsigned int size)
{
	static unsigned int buf_size;
	static unsigned int xfer_count;

	if (count > xfer_count) {
		kfree(spi_xfer);
		spi_xfer = kcalloc(count, sizeof(*spi_xfer), GFP_KERNEL);
		if (!spi_xfer) {
			thp_log_err("Failed to allocate memory for xfer\n");
			xfer_count = 0;
			return -ENOMEM;
		}
		xfer_count = count;
	} else {
		memset(spi_xfer, 0, count * sizeof(*spi_xfer));
	}

	if (size > buf_size) {
		if (buf_size)
			kfree(tx_buf);
		tx_buf = kmalloc(size, GFP_KERNEL);
		if (!tx_buf) {
			thp_log_err("Failed to allocate memory for buf\n");
			buf_size = 0;
			return -ENOMEM;
		}
		buf_size = size;
	}

	return 0;
}

static int touch_driver_spi_read(struct spi_device *client, unsigned char *data,
	unsigned int length)
{
	int retval;
#ifdef CONFIG_HUAWEI_THP_MTK
	int retry = 0;
#endif
	struct spi_message msg;

	spi_message_init(&msg);

	retval = touch_driver_spi_alloc_mem(client, 1, length);
	if (retval < 0) {
		thp_log_err("Failed to allocate memory\n");
		goto exit;
	}

	memset(tx_buf, 0xff, length);
	spi_xfer[0].len = length;
	spi_xfer[0].tx_buf = tx_buf;
	spi_xfer[0].rx_buf = data;
	spi_message_add_tail(&spi_xfer[0], &msg);
#ifdef CONFIG_HUAWEI_THP_MTK
	while (retry < SPI_RETRY_TIMES) {
		retval = thp_spi_sync(client, &msg);
		if (retval == 0) {
			retval = length;
			break;
		} else {
			thp_log_err("SPI transfer failed, error = %d\n",
				retval);
			retry++;
			msleep(SPI_DELAY_MS);
			continue;
		}
	}
#else
	retval = thp_spi_sync(client, &msg);
	if (retval == 0) {
		retval = length;
	} else {
		thp_log_err("Failed to complete SPI transfer, error = %d\n",
				retval);
	}
#endif

exit:

	return retval;
}

static int touch_driver_spi_write(struct spi_device *client,
	unsigned char *data, unsigned int length)
{
	int retval;
	struct spi_message msg;

	spi_message_init(&msg);

	retval = touch_driver_spi_alloc_mem(client, 1, 0);
	if (retval < 0) {
		thp_log_err("Failed to allocate memory\n");
		goto exit;
	}

	spi_xfer[0].len = length;
	spi_xfer[0].tx_buf = data;
	spi_message_add_tail(&spi_xfer[0], &msg);
	retval = thp_spi_sync(client, &msg);
	if (retval == 0) {
		retval = length;
	} else {
		thp_log_err("Failed to complete SPI transfer, error = %d\n",
				retval);
	}

exit:

	return retval;
}

static int touch_driver_send_cmd(struct spi_device *client,
	char *buf, unsigned int length)
{
	int ret;
	int retry_times = SYNA_RETRY_TIMES;
	uint8_t *resp_buf = spi_read_buf;

	if (resp_buf == NULL) {
		thp_log_err("resp_buf is NULL\n");
		return -ENOMEM;
	}
	memset(resp_buf, 0, SYNA_RESPONSE_LEN);
	if (thp_bus_lock() < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}

	ret = touch_driver_spi_write(client, buf, length);
	if (ret != length) {
		thp_log_err("%s, Failed to write command\n", __func__);
		goto exit;
	}
	while (retry_times) {
		msleep(50); /* delay 50ms for make sure fw response cmd */
		ret = touch_driver_spi_read(client, resp_buf,
			SYNA_RESPONSE_LEN);
		if ((ret != SYNA_RESPONSE_LEN) ||
			(resp_buf[0] != MESSAGE_MARKER)) {
			thp_log_err("Fail to read response %x\n", resp_buf[0]);
			goto exit;
		}
		thp_log_info("%s, 0x%02x 0x%02x 0x%02x 0x%02x\n", __func__,
			resp_buf[0], resp_buf[1], resp_buf[2], resp_buf[3]);
		/* resp_buf[1]: fw response status */
		if (resp_buf[1] == STATUS_OK) {
			ret = NO_ERR;
			break;
		}

		retry_times--;
	}
exit:
	thp_bus_unlock();
#if defined(CONFIG_HUAWEI_DSM)
	if (ret)
		thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_GESTURE_EXP1,
			"%s, 0x%02x 0x%02x 0x%02x 0x%02x\n", __func__,
			resp_buf[0], resp_buf[1], resp_buf[2], resp_buf[3]);
#endif
	return ret;
}

static int touch_driver_disable_frame(struct thp_device *tdev, int value)
{
	const uint8_t cmd_disable[FRAME_CMD_LEN] = { 0x06, 0x01, 0x00, 0xC0 };
	const uint8_t cmd_enable[FRAME_CMD_LEN] = { 0x05, 0x01, 0x00, 0xC0 };
	uint8_t *cmd_buf = spi_write_buf;

	thp_log_info("%s, input value is %d\n", __func__, value);

	if (cmd_buf == NULL) {
		thp_log_err("cmd_buf is NULL\n");
		return -ENOMEM;
	}
	if (value) /* value 1: disable frame */
		memcpy(cmd_buf, cmd_disable, FRAME_CMD_LEN);
	else
		memcpy(cmd_buf, cmd_enable, FRAME_CMD_LEN);

	return touch_driver_send_cmd(
		tdev->thp_core->sdev,
		cmd_buf,
		FRAME_CMD_LEN);
}

static int touch_driver_set_dynamic_config(struct thp_device *tdev,
	enum dynamic_config_id id, unsigned int value)
{
	int index = 0;
	uint8_t *out_buf = spi_write_buf;

	if (out_buf == NULL) {
		thp_log_err("out_buf is NULL\n");
		return -ENOMEM;
	}
	out_buf[index++] = CMD_SET_DYNAMIC_CONFIG;
	out_buf[index++] = (unsigned char)CMD_DYNAMIC_CONFIG_LEN;
	out_buf[index++] = (unsigned char)(CMD_DYNAMIC_CONFIG_LEN >> MOVE_8BIT);
	out_buf[index++] = (unsigned char)id;
	out_buf[index++] = (unsigned char)value;
	out_buf[index++] = (unsigned char)(value >> MOVE_8BIT);

	thp_log_info("%s, type: %d, value: %u\n", __func__, id, value);
	return touch_driver_send_cmd(tdev->thp_core->sdev,
		out_buf, DYNAMIC_CMD_LEN);
}

static int touch_driver_parse_gesture_data(struct thp_device *tdev,
	const char *data, unsigned int *gesture_wakeup_value)
{
	unsigned int data_length;

	 /* data[2]: length's low ,data[3]: length's high */
	data_length = (unsigned int)((data[3] << MOVE_8BIT) | data[2]);
	thp_log_info("%s, data length is %u\n", __func__, data_length);
	/* data[4]: report touch type */
	if (data_length == GESTURE_REPORT_SIZE && data[4] == DOUBLE_TAP) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
			tdev->thp_core->easy_wakeup_info.off_motion_on = false;
			*gesture_wakeup_value = TS_DOUBLE_CLICK;
		}
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		return NO_ERR;
	}

	return -EINVAL;
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

static int touch_driver_gesture_report(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	unsigned char *data = spi_read_buf;
	int retval;
	u32 i;

	thp_log_info("%s enter\n", __func__);
	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_err("%s: input dev is null\n", __func__);
		return -EINVAL;
	}
	if ((gesture_wakeup_value == NULL) ||
		(!tdev->thp_core->support_gesture_mode)) {
		thp_log_err("%s, gesture not support\n", __func__);
		return -EINVAL;
	}
	if (!data) {
		thp_log_err("%s:data is NULL\n", __func__);
		return -EINVAL;
	}
	memset(data, 0, SPI_READ_WRITE_SIZE);
	retval = thp_bus_lock();
	if (retval < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	/* wait spi bus resume */
	for (i = 0; i < tdev->thp_core->gesture_retry_times; i++) {
		retval = touch_driver_spi_read(tdev->thp_core->sdev,
			data, TOUCH_REPORT_CONFIG_SIZE);
		if (retval == TOUCH_REPORT_CONFIG_SIZE)
			break;
		thp_log_info("%s: spi not work normal, ret %d retry\n",
			__func__, retval);
		msleep(WAIT_FOR_SPI_BUS_READ_DELAY);
	}
	thp_bus_unlock();

	if ((data[0] != MESSAGE_MARKER) ||
		(data[1] != REPORT_TOUCH)) { /* data[1]: fw response status */
		thp_log_err("%s, data0~1: 0x%02x 0x%02x\n", __func__,
			data[0], data[1]);
		return -EINVAL;
	}
	retval = touch_driver_parse_gesture_data(tdev,
		data, gesture_wakeup_value);

	thp_log_info("%s exit\n", __func__);
	return retval;
}

int touch_driver_parse_ic_feature_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	thp_log_debug("%s:Enter!\n", __func__);
	cd->support_get_frame_read_once = 0;
	rc = of_property_read_u32(thp_node, "support_get_frame_read_once",
		&value);
	if (!rc) {
		cd->support_get_frame_read_once = value;
		thp_log_info("%s:support_get_frame_read_once configed %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "support_reset_low_pinctrl_for_mtk",
		&value);
	if (!rc) {
		cd->support_reset_low_pinctrl_for_mtk = value;
		thp_log_info("%s:support_reset_low_pinctrl_for_mtk %u\n",
			__func__, value);
	}
	cd->get_frame_size_max = 0;
	rc = of_property_read_u32(thp_node, "get_frame_size_max",
		&value);
	if (!rc) {
		cd->get_frame_size_max = value;
		thp_log_info("%s:get_frame_size_max configed %u\n",
			__func__, value);
	}
	cd->send_one_cmd_for_ap = 0;
	rc = of_property_read_u32(thp_node, "send_one_cmd_for_ap", &value);
	if (!rc) {
		cd->send_one_cmd_for_ap = value;
		thp_log_info("%s:send_one_cmd_for_ap %u\n", __func__, value);
	}
	return 0;
}

static int touch_driver_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *syna_node = of_get_child_by_name(cd->thp_node,
						THP_SYNA_DEV_NODE_NAME);

	thp_log_info("%s: called\n", __func__);
	if (!syna_node) {
		thp_log_info("%s: dev not config in dts\n", __func__);
		return -ENODEV;
	}
	rc = thp_parse_spi_config(syna_node, cd);
	if (rc)
		thp_log_err("%s: spi config parse fail\n", __func__);
	rc = thp_parse_timing_config(syna_node, &tdev->timing_config);
	if (rc)
		thp_log_err("%s: timing config parse fail\n", __func__);
	rc = thp_parse_feature_config(syna_node, cd);
	if (rc)
		thp_log_err("%s: feature_config fail\n", __func__);
	rc = touch_driver_parse_ic_feature_config(syna_node, cd);
	if (rc)
		thp_log_err("%s: ic_feature_config fail\n", __func__);
	rc = thp_parse_trigger_config(syna_node, cd);
	if (rc)
		thp_log_err("%s: trigger_config fail\n", __func__);
	rc = of_property_read_u32(syna_node, "support_deepsleep_mode",
		&cd->support_deepsleep_mode);
	if (rc)
		cd->support_deepsleep_mode = 0;
	thp_log_info("%s: support_deepsleep_mode: %u\n", __func__,
		cd->support_deepsleep_mode);
	return 0;
}

static int touch_driver_power_init(void)
{
	int ret;

	ret = thp_power_supply_get(THP_VCC);
	if (ret)
		thp_log_err("%s: fail to get vcc power\n", __func__);
	ret = thp_power_supply_get(THP_IOVDD);
	if (ret)
		thp_log_err("%s: fail to get power\n", __func__);
	return 0;
}

static int touch_driver_power_release(void)
{
	int ret;

	ret = thp_power_supply_put(THP_VCC);
	if (ret)
		thp_log_err("%s: fail to release vcc power\n", __func__);
	ret = thp_power_supply_put(THP_IOVDD);
	if (ret)
		thp_log_err("%s: fail to release power\n", __func__);
	return ret;
}

static int touch_driver_power_on(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s:called\n", __func__);

	if (!tdev || !cd) {
		thp_log_err("%s: tdev or cd null\n", __func__);
		return -EINVAL;
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_direction_input(tdev->gpios->irq_gpio);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 1); /* delay 1ms */
	if (ret)
		thp_log_err("%s:power ctrl fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON,
		cd->iovdd_power_on_delay_ms);
	if (ret)
		thp_log_err("%s:power ctrl vddio fail\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#else
	pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.reset_low);
	if (!cd->support_no_cs_pinctrl_for_mtk)
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.cs_low);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 1); /* delay 1ms */
	if (ret)
		thp_log_err("%s:power ctrl fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 1); /* delay 1ms */
	if (ret)
		thp_log_err("%s:power ctrl vddio fail\n", __func__);
	if (!cd->support_no_cs_pinctrl_for_mtk)
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.cs_high);
	pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.reset_high);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#endif
	return ret;
}

static int touch_driver_power_off_for_3909(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_debug("%s: in\n", __func__);
	if ((!tdev) || (!tdev->gpios) || !cd) {
		thp_log_err("%s: have null ptr\n", __func__);
		return -EINVAL;
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	mdelay(POWER_OFF_DELAY_FOR_3909);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	mdelay(POWER_OFF_DELAY_FOR_3909);
#else
	if (cd->support_control_cs_off) {
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->pins_idle);
	} else {
		if (!cd->support_no_cs_pinctrl_for_mtk)
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.cs_low);
		mdelay(POWER_OFF_DELAY_FOR_3909);
		pinctrl_select_state(tdev->thp_core->pctrl,
			tdev->thp_core->mtk_pinctrl.reset_low);
	}
	mdelay(POWER_OFF_DELAY_FOR_3909);
#endif
	if (cd->support_deepsleep_mode) {
		thp_log_err("%s: no power_off, in deepsleep mode\n", __func__);
		return pt_mode_set_for_3909(tdev);
	}
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF,
		POWER_OFF_DELAY_FOR_3909);
	if (ret)
		thp_log_err("%s:power ctrl fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF,
		POWER_OFF_DELAY_FOR_3909);
	if (ret)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);

	return ret;
}

static int touch_driver_power_off(struct thp_device *tdev)
{
	int ret;

	if ((!tdev) || (!tdev->thp_core) || (!tdev->gpios)) {
		thp_log_err("%s: have null ptr\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->support_vendor_ic_type == SYNA3909)
		return touch_driver_power_off_for_3909(tdev);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	mdelay(tdev->timing_config.suspend_reset_after_delay_ms);
	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, VCC_DELAY);
	if (ret)
		thp_log_err("%s:power ctrl vcc fail\n", __func__);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, IOVDD_DELAY);
	if (ret)
		thp_log_err("%s:power ctrl fail\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);

	return ret;
}

static int touch_driver_get_flash_cmd(struct syna_tcm_boot_info *boot_info,
	unsigned char *cmd_buf, unsigned int cmd_len)
{
	int index = 0;
	unsigned int addr_words;
	unsigned int length_words;
	unsigned char *start_block = boot_info->boot_config_start_block;

	if (cmd_len != REFLASH_READ_LEN) {
		thp_log_err("%s:input invalidl\n", __func__);
		return -EINVAL;
	}
	addr_words = ((unsigned int)start_block[0] & 0x000000FF) |
		(((unsigned int)start_block[1] << MOVE_8BIT) & 0x0000FF00);
	addr_words *= boot_info->write_block_size_words;
	length_words = BOOT_CONFIG_SIZE * BOOT_CONFIG_SLOTS;
	cmd_buf[index++] = CMD_READ_FLASH;
	cmd_buf[index++] = REFLASH_CMD_LEN_LOW;
	cmd_buf[index++] = REFLASH_CMD_LEN_HIGH;
	cmd_buf[index++] = (unsigned char)addr_words;
	cmd_buf[index++] = (unsigned char)(addr_words >> MOVE_8BIT);
	cmd_buf[index++] = (unsigned char)(addr_words >> MOVE_16BIT);
	cmd_buf[index++] = (unsigned char)(addr_words >> MOVE_24BIT);
	cmd_buf[index++] = (unsigned char)length_words;
	cmd_buf[index++] = (unsigned char)(length_words >> MOVE_8BIT);
	return 0;
}

#define ENTER_BOOTLOADER_MODE_DELAY  50
#define GET_BOOT_INFO_DELAY  10
#define GET_PROJECTID_DELAY  20

static void touch_driver_enter_bootloader_mode(struct thp_device *tdev,
	struct syna_tcm_identification *id_info)
{
	struct spi_device *sdev = tdev->thp_core->sdev;
	int retval;
	unsigned char cmd = CMD_RUN_BOOTLOADER_FIRMWARE;
	unsigned char *temp_buf = spi_read_buf;

	if (id_info->mode == MODE_APPLICATION) {
		retval = touch_driver_spi_write(sdev, &cmd, sizeof(cmd));
		if (retval < 0)
			thp_log_err("%s:spi write failed\n", __func__);
		msleep(ENTER_BOOTLOADER_MODE_DELAY);
		retval = touch_driver_spi_read(sdev, temp_buf,
			BOOT_CONFIG_SIZE * BOOT_CONFIG_SLOTS * 2);
		if (retval < 0)
			thp_log_err("%s:spi read failed\n", __func__);
		if (temp_buf[1] == REPORT_IDENTIFY)
			memcpy(id_info, &temp_buf[MESSAGE_HEADER_SIZE],
				sizeof(*id_info));
		thp_log_info("%s: value = 0x%x,expect value = 0x10\n",
			__func__, temp_buf[1]);
	}
}

static int touch_driver_get_boot_info(struct thp_device *tdev,
	struct syna_tcm_boot_info *pboot_info)
{
	struct spi_device *sdev = tdev->thp_core->sdev;
	int retval;
	unsigned char cmd;
	unsigned char *temp_buf = spi_read_buf;

	cmd = CMD_GET_BOOT_INFO;
	retval = touch_driver_spi_write(sdev, &cmd, sizeof(cmd));
	if (retval < 0)
		thp_log_err("%s:spi write failed\n", __func__);
	msleep(GET_BOOT_INFO_DELAY);
	touch_driver_spi_read(sdev, temp_buf,
		(BOOT_CONFIG_SIZE * BOOT_CONFIG_SLOTS * 2));
	if (retval < 0)
		thp_log_err("%s:spi read failed\n", __func__);
	if (temp_buf[1] != STATUS_OK) {
		thp_log_err("%s:fail to get boot info\n", __func__);
		return -EINVAL;
	}
	memcpy(pboot_info, &temp_buf[MESSAGE_HEADER_SIZE], sizeof(*pboot_info));
	return 0;
}

static void touch_driver_reflash_read_boot_config(struct thp_device *tdev,
	struct syna_tcm_identification *id_info)
{
	int retval;
	unsigned char *temp_buf = spi_read_buf;
	unsigned char *out_buf = spi_write_buf;
	struct syna_tcm_boot_info boot_info;
	struct spi_device *sdev = tdev->thp_core->sdev;

	retval = thp_bus_lock();
	if (retval < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return;
	}
	touch_driver_enter_bootloader_mode(tdev, id_info);

	thp_log_info("%s:id_info.mode = %d\n", __func__, id_info->mode);
	if (id_info->mode == MODE_BOOTLOADER) {
		retval = touch_driver_get_boot_info(tdev, &boot_info);
		if (retval < 0) {
			thp_log_err("%s:fail to get boot info\n", __func__);
			goto exit;
		}
	}
	memcpy(&boot_info, &temp_buf[MESSAGE_HEADER_SIZE], sizeof(boot_info));
	retval = touch_driver_get_flash_cmd(&boot_info,
		out_buf, REFLASH_READ_LEN);
	if (retval < 0) {
		thp_log_err("%s:fail to get flash cmd\n", __func__);
		goto exit;
	}
	retval = touch_driver_spi_write(sdev, out_buf, REFLASH_READ_LEN);
	if (retval < 0)
		thp_log_err("%s:reflash read failed\n", __func__);
	msleep(GET_PROJECTID_DELAY);
	retval = touch_driver_spi_read(sdev, temp_buf,
		(BOOT_CONFIG_SIZE * BOOT_CONFIG_SLOTS * 2));
	if (retval < 0) {
		thp_log_err("%s:fail to read boot config\n", __func__);
		goto exit;
	}
	/* success get project iD from tp ic */
	get_project_id_flag = 1;
	memcpy(tdev->thp_core->project_id,
		&temp_buf[IC_PROJECT_ID_START],
		THP_PROJECT_ID_LEN);
	thp_log_info("%s: IC project id is %s\n",
		__func__, tdev->thp_core->project_id);
exit:
	thp_bus_unlock();
}

static int touch_driver_chip_detect_for_tddi(struct thp_device *tdev)
{
	unsigned char *rmiaddr = spi_write_buf;
	unsigned char fnnum = 0;
	int rc;

	thp_log_info("%s: called\n", __func__);
	rc = thp_bus_lock();
	if (rc < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		rc = -EINVAL;
		goto exit;
	}
	rmiaddr[0] = RMI_ADDR_FIRST;
	rmiaddr[1] = RMI_ADDR_SECOND;
	rc = touch_driver_spi_write(tdev->thp_core->sdev, rmiaddr, RMI_CMD_LEN);
	if (rc < 0)
		thp_log_err("%s: spi write failed\n", __func__);
	rc = touch_driver_spi_read(tdev->thp_core->sdev, &fnnum, sizeof(fnnum));
	if (rc < 0)
		thp_log_err("%s:spi read failed\n", __func__);
	thp_bus_unlock();
	if ((fnnum != UBL_FN_NUMBER) && (fnnum != MESSAGE_MARKER)) {
		thp_log_err("%s: fnnum error: 0x%02x\n", __func__, fnnum);
		rc = -ENODEV;
		goto exit;
	}
	thp_log_err("%s: fnnum error: 0x%02x\n", __func__, fnnum);
	return 0;
exit:
	return rc;
}

static int touch_driver_chip_detect_3909(struct thp_device *tdev)
{
	int ret;
	unsigned char *tx_buf = spi_read_buf;
	struct syna_tcm_identification id_info;
	int i;

	thp_log_info("%s: called\n", __func__);
	ret = touch_driver_power_init();
	if (ret)
		thp_log_err("%s: power init failed\n", __func__);
	ret = touch_driver_power_on(tdev);
	if (ret)
		thp_log_err("%s: power on failed\n", __func__);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	ret = touch_driver_spi_read(tdev->thp_core->sdev, tx_buf,
		MESSAGE_DATA_NUM);
	if (ret < 0) {
		thp_log_err("%s: failed to read data\n", __func__);
		thp_bus_unlock();
		return -ENODEV;
	}
	thp_bus_unlock();

	if (tx_buf[0] != MESSAGE_MARKER) {
		thp_log_err("%s: message_marker error\n", __func__);
		for (i = 0; i < MESSAGE_DATA_NUM; i++)
			thp_log_info("buf[i] = %d\n", tx_buf[i]);
		ret = touch_driver_power_off(tdev);
		if (ret)
			thp_log_err("%s: power off failed\n", __func__);
		ret = touch_driver_power_release();
		if (ret < 0) {
			thp_log_err("%s: power ctrl Failed\n", __func__);
			return ret;
		}
		return -ENODEV;
	}
	thp_log_info("%s:device detected\n", __func__);

	memcpy(&id_info, &tx_buf[MESSAGE_HEADER_SIZE], sizeof(id_info));
	touch_driver_reflash_read_boot_config(tdev, &id_info);
	thp_log_info("%s: message_marker succ\n", __func__);
	return 0;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	int ret = -EINVAL;

	thp_log_info("%s: called\n", __func__);
	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	spi_read_buf = kzalloc(SPI_READ_WRITE_SIZE, GFP_KERNEL);
	if (!spi_read_buf) {
		thp_log_err("%s:spi_read_buf fail\n", __func__);
		return -EINVAL;
	}
	spi_write_buf = kzalloc(SPI_READ_WRITE_SIZE, GFP_KERNEL);
	if (!spi_write_buf) {
		thp_log_err("%s:spi_write_buf fail\n", __func__);
		goto  exit;
	}

	if (tdev->thp_core->self_control_power) {
		ret = touch_driver_chip_detect_3909(tdev);
		if (ret < 0) {
			thp_log_err("%s: fail\n", __func__);
			goto  exit;
		}
	} else {
		ret = touch_driver_chip_detect_for_tddi(tdev);
		if (ret < 0) {
			thp_log_err("%s: fail\n", __func__);
			goto  exit;
		}
	}
	thp_log_info("%s: succ\n", __func__);
	return 0;
exit:
	kfree(spi_read_buf);
	spi_read_buf = NULL;
	kfree(spi_write_buf);
	spi_write_buf = NULL;
	if (tdev->thp_core->fast_booting_solution) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	return ret;
}

#define SYNA_FRAME_SIZE_MAX 2256
#define SYNA_FRAME_STATUS_ERROR 0xFF
static int touch_driver_get_frame_read_once(
	struct thp_device *tdev, char *frame_buf, unsigned int len)
{
	unsigned int length = SYNA_FRAME_SIZE_MAX;
	int retval;

	if (length > len) {
		thp_log_err("%s:frame len error\n", __func__);
		return -EINVAL;
	}
	retval = thp_bus_lock();
	if (retval < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->get_frame_size_max)
		length = tdev->thp_core->get_frame_size_max;
	retval = touch_driver_spi_read(tdev->thp_core->sdev,
		frame_buf, length); /* read packet */
	if (retval < 0) {
		thp_log_err("%s: Failed to read length\n", __func__);
		goto error;
	}
	if (frame_buf[1] == SYNA_FRAME_STATUS_ERROR) { /* 1: frame status flag */
		thp_log_err("%s: should ignore this irq\n", __func__);
		retval = -ENODATA;
		goto error;
	}
	if (frame_buf[0] != MESSAGE_MARKER) { /* 0: frame head flag */
		thp_log_err("%s: incorrect marker: 0x%02x\n", __func__,
			frame_buf[0]);
		if (frame_buf[1] == STATUS_CONTINUED_READ) { /* 1: frame status flag */
			/* just in case */
			thp_log_err("%s: continued Read\n", __func__);
			touch_driver_spi_read(tdev->thp_core->sdev,
				tdev->rx_buff,
				THP_MAX_FRAME_SIZE); /* drop one transaction */
		}
		retval = -ENODATA;
		goto error;
	}
	thp_bus_unlock();
	return 0;
error:
	thp_bus_unlock();
	return retval;
}

static int touch_driver_get_frame(struct thp_device *tdev,
	char *frame_buf, unsigned int len)
{
	unsigned char *data = spi_read_buf;
	unsigned int length;
	int retval;

	if (tdev->thp_core->support_get_frame_read_once ==
		SUPPORT_GET_FRAME_READ_ONCE)
		return touch_driver_get_frame_read_once(tdev, frame_buf, len);

	if (!data) {
		thp_log_err("%s:data is NULL\n", __func__);
		return -EINVAL;
	}
	retval = thp_bus_lock();
	if (retval < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	retval = touch_driver_spi_read(tdev->thp_core->sdev,
		data, FRAME_HEAD_LEN);
	if (retval < 0) {
		thp_log_err("%s: Failed to read length\n", __func__);
		goto ERROR;
	}
	if (data[1] == 0xFF) {
		thp_log_err("%s: should ignore this irq\n", __func__);
		 retval = -ENODATA;
		goto ERROR;
	}
	if (data[0] != MESSAGE_MARKER) {
		thp_log_err("%s: incorrect marker: 0x%02x\n", __func__,
			data[0]);
		if (data[1] == STATUS_CONTINUED_READ) {
			// just in case
			thp_log_err("%s: continued Read\n", __func__);
			touch_driver_spi_read(tdev->thp_core->sdev,
				tdev->rx_buff,
				THP_MAX_FRAME_SIZE); /* drop one transaction */
		}
		retval = -ENODATA;
		goto ERROR;
	}

	length = (data[3] << 8) | data[2];
	if (length > (THP_MAX_FRAME_SIZE - FIRST_FRAME_USEFUL_LEN)) {
		thp_log_info("%s: out of length\n", __func__);
		length = THP_MAX_FRAME_SIZE - FIRST_FRAME_USEFUL_LEN;
	}
	if (length) {
#ifndef CONFIG_HUAWEI_THP_MTK
		retval = touch_driver_spi_read(tdev->thp_core->sdev,
			frame_buf + FIRST_FRAME_USEFUL_LEN,
			length + FIRST_FRAME_USEFUL_LEN); /* read packet */
		if (retval < 0) {
			thp_log_err("%s: Failed to read length\n", __func__);
			goto ERROR;
		}
#else
		/* MTK6873 byte alignment */
		retval = touch_driver_spi_read(tdev->thp_core->sdev,
			frame_buf,
			length + FIRST_FRAME_USEFUL_LEN); /* read packet */
		if (retval < 0) {
			thp_log_err("%s: Failed to read length\n", __func__);
			goto ERROR;
		}
		memmove(frame_buf + FIRST_FRAME_USEFUL_LEN, frame_buf,
			length + FIRST_FRAME_USEFUL_LEN);
#endif
	}
	thp_bus_unlock();
	memcpy(frame_buf, data, FRAME_HEAD_LEN);
	return 0;

ERROR:
	thp_bus_unlock();
	return retval;
}

static void touch_driver_gesture_mode_enable_switch(
	struct thp_device *tdev, unsigned int value)
{
	int ret_disable_frame;
	int ret_set_config;

	ret_disable_frame = touch_driver_disable_frame(tdev, value);
	ret_set_config = touch_driver_set_dynamic_config(tdev,
		DC_IN_WAKEUP_GESTURE_MODE, value);
	if (ret_disable_frame || ret_set_config)
		thp_log_err("%s, ret is %d %d\n", __func__,
			ret_disable_frame, ret_set_config);
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	tdev->thp_core->easy_wakeup_info.off_motion_on = true;
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
}

static int touch_driver_resume(struct thp_device *tdev)
{
	int ret;
	struct spi_device *sdev = NULL;
	/* report irq to ap cmd */
	char report_to_ap_cmd[SYNA_CMD_LEN] = { 0xC7, 0x02, 0x00, 0x2E, 0x00 };
	enum ts_sleep_mode gesture_status;

	thp_log_info("%s: called\n", __func__);
	if (!tdev || !tdev->thp_core || !tdev->thp_core->sdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	sdev = tdev->thp_core->sdev;
	gesture_status = tdev->thp_core->easy_wakeup_info.sleep_mode;
	if (tdev->thp_core->self_control_power) {
		if (is_pt_test_mode(tdev)) {
			gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
			mdelay(tdev->timing_config.resume_reset_after_delay_ms);
			gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
		} else if (need_power_off == NEED_WORK_IN_SUSPEND) {
			thp_log_info("%s:change irq to AP\n", __func__);
#ifdef CONFIG_HUAWEI_THP_MTK
			if (tdev->thp_core->use_ap_gesture)
				report_to_ap_cmd[3] = 0x4C; /* udfp cmd 0x4C */
#endif
			ret = thp_bus_lock();
			if (ret < 0) {
				thp_log_err("%s:get lock failed\n", __func__);
				return -EINVAL;
			}
			memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
			memcpy(spi_write_buf, report_to_ap_cmd, SYNA_CMD_LEN);
			ret = touch_driver_spi_write(sdev, spi_write_buf,
				SYNA_CMD_LEN);
			if (ret < 0)
				thp_log_err("%s: send cmd failed\n", __func__);
			thp_bus_unlock();
		} else {
			ret = touch_driver_power_on(tdev);
			if (ret < 0) {
				thp_log_err("%s: power on Failed\n", __func__);
				return ret;
			}
		}
	} else {
#ifndef CONFIG_HUAWEI_THP_MTK
		if (gesture_status == TS_GESTURE_MODE &&
			tdev->thp_core->lcd_gesture_mode_support) {
			thp_log_info("gesture mode exit\n");
			gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_LOW);
			mdelay(10); /* 10ms sequence delay */
			gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
		} else {
			gpio_set_value(tdev->gpios->cs_gpio, 1);
			/* keep TP rst  high before LCD  reset hign */
			gpio_set_value(tdev->gpios->rst_gpio, 1);
		}
#else
		if (tdev->thp_core->support_pinctrl == 0) {
			thp_log_info("%s: not support pinctrl\n", __func__);
			return -EINVAL;
		}
		if (gesture_status == TS_GESTURE_MODE &&
			tdev->thp_core->lcd_gesture_mode_support) {
			thp_log_info("gesture mode exit\n");
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.reset_low);
			mdelay(10); /* 10ms sequence delay */
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.reset_high);
		} else {
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.cs_high);
			/* keep TP rst  high before LCD  reset hign */
			if (tdev->thp_core->support_reset_low_pinctrl_for_mtk) {
				pinctrl_select_state(tdev->thp_core->pctrl,
					tdev->thp_core->mtk_pinctrl.reset_low);
				mdelay(10);  /* 10ms sequence delay */
			}
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.reset_high);
		}
#endif
	}
	thp_log_info("%s: called end\n", __func__);
	return 0;
}

#define SYNA_READ_PT_MODE_FLAG_RETRY_TIMES 10
#define SYNA_PT_PKG_HEAD 0xa5
#define SYNA_PT_PKG_HEAD_OFFSET 1
#define SYNA_PT_PKG_LEN 0

static int pt_mode_set_for_3909(const struct thp_device *tdev)
{
	int ret;
	int i;
	unsigned char *data = spi_read_buf;
	/* pt station cmd */
	u8 syna_sleep_cmd[SYNA_COMMAMD_LEN] = { 0x2C, 0x00, 0x00 };

	if (tdev == NULL) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
	memcpy(spi_write_buf, syna_sleep_cmd, SYNA_COMMAMD_LEN);
	ret = touch_driver_spi_write(tdev->thp_core->sdev,
		spi_write_buf, SYNA_COMMAMD_LEN);
	if (ret < 0)
		thp_log_err("Failed to send active command\n");
	thp_log_info("%s write ret = %d\n", __func__, ret);

	/* TP chip needs to read redundant frame,then goto idle */
	for (i = 0; i < SYNA_READ_PT_MODE_FLAG_RETRY_TIMES; i++) {
		ret = touch_driver_spi_read(tdev->thp_core->sdev,
			data, FRAME_HEAD_LEN);
		if (ret < 0)
			thp_log_err("%s: Failed to read length\n", __func__);
		thp_log_debug("data: %*ph\n", FRAME_HEAD_LEN, data);
		ret = touch_driver_spi_read(tdev->thp_core->sdev, tdev->rx_buff,
			THP_MAX_FRAME_SIZE);
		if (ret < 0)
			thp_log_err("%s: Failed to read length\n", __func__);
		thp_log_debug("rx_buff: %*ph\n", FRAME_HEAD_LEN, tdev->rx_buff);
		/* read 0xa5 0x1 0x0 0x0 is success from tp ic */
		if ((data[0] == SYNA_PT_PKG_HEAD) &&
			(data[1] == SYNA_PT_PKG_HEAD_OFFSET) &&
			(data[2] == SYNA_PT_PKG_LEN) &&
			(data[3] == SYNA_PT_PKG_LEN)) {
			thp_log_info("%s :get flag success\n", __func__);
			break;
		}
		thp_log_err("%s :get flag failed\n", __func__);
		ret = -EINVAL;
	}
	thp_bus_unlock();
	return ret;
}

static int pt_mode_set(const struct thp_device *tdev)
{
	int ret;
	unsigned char *data = spi_read_buf;
	/* pt station cmd */
	u8 syna_sleep_cmd[SYNA_COMMAMD_LEN] = { 0x2C, 0x00, 0x00 };

	if ((tdev == NULL) || (tdev->thp_core == NULL)) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->support_vendor_ic_type == SYNA3909)
		return pt_mode_set_for_3909(tdev);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
	memcpy(spi_write_buf, syna_sleep_cmd, SYNA_COMMAMD_LEN);
	ret = touch_driver_spi_write(tdev->thp_core->sdev,
		spi_write_buf, SYNA_COMMAMD_LEN);
	if (ret < 0)
		thp_log_err("Failed to send syna active command\n");
	ret = touch_driver_spi_read(tdev->thp_core->sdev, data, FRAME_HEAD_LEN);
	if (ret < 0)
		thp_log_err("%s: Failed to read length\n", __func__);
	ret = touch_driver_spi_read(tdev->thp_core->sdev, tdev->rx_buff,
		THP_MAX_FRAME_SIZE);
	if (ret < 0)
		thp_log_err("%s: Failed to read length\n", __func__);
	thp_bus_unlock();
	return ret;
}

#ifdef CONFIG_HUAWEI_SHB_THP
#define INPUTHUB_POWER_SWITCH_START_BIT 9
#define INPUTHUB_POWER_SWITCH_START_OFFSET 1
static void touch_driver_get_poweroff_status(void)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned int finger_status = !!(thp_get_status(THP_STATUS_UDFP));
	unsigned int stylus_status = (thp_get_status(THP_STATUS_STYLUS)) |
		(thp_get_status(THP_STATUS_STYLUS3));

	cd->poweroff_function_status =
		(cd->double_click_switch << THP_DOUBLE_CLICK_ON) |
		(finger_status << THP_TPUD_ON) |
		(cd->ring_setting_switch << THP_RING_SUPPORT) |
		(cd->ring_switch << THP_RING_ON) |
		(stylus_status << THP_PEN_MODE_ON) |
		(cd->phone_status << THP_PHONE_STATUS) |
		(cd->single_click_switch << THP_SINGLE_CLICK_ON) |
		(cd->volume_side_status << THP_VOLUME_SIDE_STATUS_LEFT);
	if (cd->aod_display_support)
		cd->poweroff_function_status |=
		(cd->aod_touch_status << THP_AOD_TOUCH_STATUS);
	if ((cd->power_switch >= POWER_KEY_OFF_CTRL) &&
		(cd->power_switch < POWER_MAX_STATUS))
	/*
	 * cd->poweroff_function_status 0~8 bit saved function flag
	 * eg:double_click, finger_status, ring_switch,and so on.
	 * cd->poweroff_function_status 9~16 bit saved screen-on-off reason flag
	 * cd->power_switch is a value(1~8) which stand for screen-on-off reason
	 */
		cd->poweroff_function_status |=
			(1 << (INPUTHUB_POWER_SWITCH_START_BIT +
			cd->power_switch - INPUTHUB_POWER_SWITCH_START_OFFSET));
}
#endif

static void need_work_in_suspend_for_shb(struct thp_device *tdev)
{
	int ret;
	char report_to_shb_cmd[SYNA_CMD_LEN] = { 0xC7, 0x02, 0x00, 0x2E, 0x01 };

	thp_log_info("%s:change irq to sensorhub\n", __func__);
	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return;
	}
	memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
	memcpy(spi_write_buf, report_to_shb_cmd, SYNA_CMD_LEN);
	ret = touch_driver_spi_write(tdev->thp_core->sdev, spi_write_buf,
		SYNA_CMD_LEN);
	if (ret < 0)
		thp_log_err("%s: send cmd failed\n", __func__);
	thp_bus_unlock();
}

#ifdef CONFIG_HUAWEI_THP_MTK
static void touch_driver_set_cmd(struct thp_device *tdev,
	unsigned char *report_to_ap_cmd, int len)
{
	int ret;
	int i;
	unsigned char *data = spi_read_buf;

	if (!tdev || !report_to_ap_cmd || (len <= 0)) {
		thp_log_err("%s: tdev or data null\n", __func__);
		return;
	}
	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return;
	}
	for (i = 0; i < len; i++)
		thp_log_debug("event %d 0x%02x\n", i, report_to_ap_cmd[i]);
	memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
	memcpy(spi_write_buf, report_to_ap_cmd, len);
	ret = touch_driver_spi_write(tdev->thp_core->sdev, spi_write_buf, len);
	if (ret < 0)
		thp_log_err("Failed to send active command\n");
	thp_log_info("%s write ret = %d\n", __func__, ret);

	for (i = 0; i < SYNA_READ_PT_MODE_FLAG_RETRY_TIMES; i++) {
		ret = touch_driver_spi_read(tdev->thp_core->sdev,
			data, FRAME_HEAD_LEN);
		if (ret < 0)
			thp_log_err("%s: Failed to read length\n", __func__);
		thp_log_debug("data: %*ph\n", FRAME_HEAD_LEN, data);
		ret = touch_driver_spi_read(tdev->thp_core->sdev, tdev->rx_buff,
			THP_MAX_FRAME_SIZE);
		if (ret < 0)
			thp_log_err("%s: Failed to read length\n", __func__);
		thp_log_debug("rx_buff: %*ph\n", FRAME_HEAD_LEN, tdev->rx_buff);
		/* read 0xa5 0x1 0x0 0x0 is success from tp ic */
		if ((data[0] == SYNA_PT_PKG_HEAD) &&
			(data[1] == SYNA_PT_PKG_HEAD_OFFSET) &&
			(data[2] == SYNA_PT_PKG_LEN) &&
			(data[3] == SYNA_PT_PKG_LEN)) {
			thp_log_info("%s :get flag success\n", __func__);
			break;
		}
		thp_log_info("%s get flag retry 0x%02x 0x%02x 0x%02x 0x%02x\n",
			__func__, data[0], data[1], data[2], data[3]);
	}
	thp_bus_unlock();
}

static int touch_driver_set_ap_one_cmd(struct thp_device *tdev)
{
	unsigned char report_to_ap_cmd[SYNA_ONE_CMD_LEN] = { 0xC7, 0x03, 0x00,
		0x4C, 0x02, 0x00 };
	unsigned int finger_status;

	struct thp_core_data *cd = tdev->thp_core;
	finger_status = !!(thp_get_status(THP_STATUS_UDFP));
	/* cmd[5]: determine different status */
	if (tdev->thp_core->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE)
		report_to_ap_cmd[5] += SYNA_GESTURE_ONE_CMD;
	if (finger_status)
		report_to_ap_cmd[5] += SYNA_FINGER_ONE_CMD;
	if (cd->aod_touch_status)
		report_to_ap_cmd[5] += SYNA_SINGLE_CLICK_ONE_CMD;
	touch_driver_set_cmd(tdev, report_to_ap_cmd,
		sizeof(report_to_ap_cmd));
	return 0;
}
static int touch_driver_set_ap_state(struct thp_device *tdev)
{
	unsigned char report_to_ap_cmd[SYNA_CMD_LEN] = { 0xC7, 0x02, 0x00,
		0x4C, 0x01 };
	unsigned char report_to_ap_cmd_gesture[SYNA_CMD_GESTURE_LEN] = { 0xC7,
		0x05, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x00 };
	unsigned int finger_status;

	finger_status = !!(thp_get_status(THP_STATUS_UDFP));
	if (!tdev || !tdev->thp_core) {
		thp_log_err("%s:tdev null\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->send_one_cmd_for_ap) {
		touch_driver_set_ap_one_cmd(tdev);
		return 0;
	}
	touch_driver_set_cmd(tdev, report_to_ap_cmd,
		sizeof(report_to_ap_cmd));
	if (tdev->thp_core->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE)
		report_to_ap_cmd_gesture[TOUCH_GESTURE_CMD] |=
			THP_GESTURE_DOUBLE_CLICK;
	if (finger_status)
		report_to_ap_cmd_gesture[TOUCH_GESTURE_CMD] |=
			THP_GESTURE_FINGER;
	touch_driver_set_cmd(tdev, report_to_ap_cmd_gesture,
		sizeof(report_to_ap_cmd_gesture));
	return 0;
}
#endif

static void need_work_in_suspend_for_ap(struct thp_device *tdev)
{
	int ret;
	char report_to_ap_cmd[SYNA_CMD_LEN] = { 0xC7, 0x02, 0x00, 0x2A, 0x01 };

	thp_log_info("%s: called\n", __func__);
#ifdef CONFIG_HUAWEI_THP_MTK
	if (tdev->thp_core->use_ap_gesture) {
		ret = touch_driver_set_ap_state(tdev);
		if (ret)
			thp_log_err("failed to set ap state\n");
		return;
	}
#endif
	thp_log_info("%s:thp need work in suspend\n", __func__);
	if (tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) {
		thp_log_info("%s:thp gesture mode enter\n", __func__);
		ret = thp_bus_lock();
		if (ret < 0) {
			thp_log_err("%s:get lock fail,ret=%d\n", __func__, ret);
			return;
		}
		memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
		memcpy(spi_write_buf, report_to_ap_cmd,
			sizeof(report_to_ap_cmd));
		ret = touch_driver_spi_write(tdev->thp_core->sdev,
			spi_write_buf, SYNA_CMD_LEN);
		thp_bus_unlock();
		if (ret < 0)
			thp_log_err("%s:send cmd fail,ret=%d\n", __func__, ret);
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
	}
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int ret;
	struct spi_device *sdev = NULL;
	struct thp_core_data *cd = thp_get_core_data();
	unsigned int gesture_status;
	unsigned int finger_status;
	unsigned int stylus_status;

	thp_log_info("%s: called\n", __func__);
	if (!tdev || !tdev->thp_core || !tdev->thp_core->sdev || !cd) {
		thp_log_err("%s: tdev or cd is null\n", __func__);
		return -EINVAL;
	}
	sdev = tdev->thp_core->sdev;
	gesture_status = !!(tdev->thp_core->easy_wakeup_info.sleep_mode);
	finger_status = !!(thp_get_status(THP_STATUS_UDFP));
	stylus_status = (thp_get_status(THP_STATUS_STYLUS)) |
		(thp_get_status(THP_STATUS_STYLUS3));
	thp_log_info("%s:gesture_status:%u,finger_status:%u,stylus_status=%u\n",
		__func__, gesture_status, finger_status, stylus_status);
	thp_log_info(
		"%s:ring_support:%d,ring_switch:%u,phone_status:%u,ring_setting_switch:%u\n",
		__func__, cd->support_ring_feature, cd->ring_switch,
		cd->phone_status, cd->ring_setting_switch);
	thp_log_info("%s:aod_touch_status:%u\n", __func__,
		cd->aod_touch_status);

	if ((gesture_status == TS_GESTURE_MODE) || finger_status ||
		cd->ring_setting_switch || stylus_status ||
		cd->aod_touch_status) {
#ifdef CONFIG_HUAWEI_SHB_THP
		if (cd->support_shb_thp)
			touch_driver_get_poweroff_status();
		if (tdev->thp_core->tsa_event_to_udfp && finger_status) {
			ret = send_tp_ap_event(sizeof(finger_status),
				(void *)&finger_status,
				ST_CMD_TYPE_FINGERPRINT_SWITCH);
			thp_log_info("%s:tsa_event_to_udfp, ret = %d\n",
				__func__, ret);
		}
#endif
		need_power_off = NEED_WORK_IN_SUSPEND;
	} else {
		if (cd->support_shb_thp)
			/* 0:all function was closed */
			cd->poweroff_function_status = 0;
		need_power_off = NO_NEED_WORK_IN_SUSPEND;
	}
	if (tdev->thp_core->self_control_power) {
		if (is_pt_test_mode(tdev)) {
			thp_log_info("%s: suspend PT mode\n", __func__);
			ret = pt_mode_set(tdev);
			if (ret < 0)
				thp_log_err("%s: failed to set pt mode\n",
					__func__);
		} else if (need_power_off == NEED_WORK_IN_SUSPEND) {
			if (cd->support_shb_thp)
				need_work_in_suspend_for_shb(tdev);
			else
				need_work_in_suspend_for_ap(tdev);
		} else {
			ret = touch_driver_power_off(tdev);
			if (ret < 0) {
				thp_log_err("%s: power off Failed\n", __func__);
				return ret;
			}
		}
	} else {
		if (gesture_status == TS_GESTURE_MODE &&
			cd->lcd_gesture_mode_support) {
			thp_log_info("gesture mode enter\n");
			/*
			 * 120ms sequence delay,
			 * make sure gesture mode enable success.
			 */
			msleep(120);
			touch_driver_gesture_mode_enable_switch(tdev,
				SYNA_ENTER_GUESTURE_MODE);
		} else {
#ifndef CONFIG_HUAWEI_THP_MTK
			gpio_set_value(tdev->gpios->rst_gpio, 0);
			gpio_set_value(tdev->gpios->cs_gpio, 0);
#else
			if (tdev->thp_core->support_pinctrl == 0) {
				thp_log_info("%s: not support pinctrl\n",
					__func__);
				return -EINVAL;
			}
			pinctrl_select_state(tdev->thp_core->pctrl,
				tdev->thp_core->mtk_pinctrl.reset_low);
			if (!cd->support_no_cs_pinctrl_for_mtk)
				pinctrl_select_state(tdev->thp_core->pctrl,
					tdev->thp_core->mtk_pinctrl.cs_low);
#endif
		}
	}
	thp_log_info("%s: called end\n", __func__);
	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	if (tdev) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}

	kfree(spi_read_buf);
	spi_read_buf = NULL;
	kfree(spi_write_buf);
	spi_write_buf = NULL;

	kfree(tx_buf);
	tx_buf = NULL;
	kfree(spi_xfer);
	spi_xfer = NULL;
}

static int touch_driver_get_project_id(struct thp_device *tdev, char *buf,
	unsigned int len)
{
	if ((!buf) || (!tdev)) {
		thp_log_err("%s: tdev or buff null\n", __func__);
		return -EINVAL;
	}

	if (tdev->thp_core->self_control_power &&
		(!get_project_id_flag)) {
		strncpy(tdev->thp_core->project_id,
			tdev->thp_core->project_id_dummy,
			THP_PROJECT_ID_LEN);
		thp_log_info("%s:use dummy project id:%s\n",
			__func__, tdev->thp_core->project_id);
	}
	strncpy(buf, tdev->thp_core->project_id, len);

	return 0;
}

#ifdef CONFIG_HUAWEI_SHB_THP
static int touch_driver_switch_int_sh(struct thp_device *tdev)
{
	int ret;
	struct spi_device *sdev = NULL;
	/* report irq to sensorhub cmd */
	unsigned char report_to_shb_cmd[SYNA_CMD_LEN] = { 0xC7,
		0x02, 0x00, 0x2E, 0x01 };

	if ((tdev == NULL) || (tdev->thp_core == NULL) ||
		(tdev->thp_core->sdev == NULL)) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	sdev = tdev->thp_core->sdev;

	thp_log_info("%s: called\n", __func__);
	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s: get lock failed\n", __func__);
		return -EINVAL;
	}
	memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
	memcpy(spi_write_buf, report_to_shb_cmd, SYNA_CMD_LEN);
	ret = touch_driver_spi_write(sdev, spi_write_buf, SYNA_CMD_LEN);
	if (ret < 0)
		thp_log_err("%s: send cmd failed\n", __func__);
	thp_bus_unlock();
	return 0;
}

static int touch_driver_switch_int_ap(struct thp_device *tdev)
{
	int ret;
	struct spi_device *sdev = NULL;
	/* report irq to ap cmd */
	unsigned char report_to_ap_cmd[SYNA_CMD_LEN] = { 0xC7,
		0x02, 0x00, 0x2E, 0x00 };

	if ((tdev == NULL) || (tdev->thp_core == NULL) ||
		(tdev->thp_core->sdev == NULL)) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	sdev = tdev->thp_core->sdev;

	thp_log_info("%s: called\n", __func__);
	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s: get lock failed\n", __func__);
		return -EINVAL;
	}
	memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
	memcpy(spi_write_buf, report_to_ap_cmd, SYNA_CMD_LEN);
	ret = touch_driver_spi_write(sdev, spi_write_buf, SYNA_CMD_LEN);
	if (ret < 0)
		thp_log_err("%s: send cmd failed\n", __func__);
	thp_bus_unlock();
	return 0;
}
#endif

#ifdef CONFIG_HUAWEI_THP_MTK
static int touch_driver_set_lowpower_state(struct thp_device *tdev,
	u8 state)
{
	unsigned char syna_sleep_cmd[SYNA_COMMAMD_LEN] = { 0x2C, 0x00, 0x00 };
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: called state = %u\n", __func__, state);
	if (tdev == NULL) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (cd->work_status != SUSPEND_DONE) {
		thp_log_info("%s: resumed, not handle lp\n", __func__);
		return NO_ERR;
	}
	if (ud_mode_status.lowpower_mode == state) {
		thp_log_info("%s:don't repeat old status %u\n",
			__func__, state);
		return 0;
	}
	if (state)
		syna_sleep_cmd[0] = 0x2C; /* enable lowpower */
	else
		syna_sleep_cmd[0] = 0x2D; /* disable lowpower */
	touch_driver_set_cmd(tdev, syna_sleep_cmd, sizeof(syna_sleep_cmd));
	ud_mode_status.lowpower_mode = state;
	return 0;
}

static int touch_driver_set_aod_state(struct thp_device *tdev,
	u8 state, struct thp_aod_window window)
{
	return 0;
}

static void parse_touch_event_info(struct thp_device *tdev,
	const char *read_buff, struct thp_udfp_data *udfp_data, int len)
{
	unsigned int tmp_event;
	unsigned int finger_status;
	struct thp_core_data *cd = NULL;

	if ((!tdev) || (!read_buff) || (!udfp_data) || (len <= 0)) {
		thp_log_err("%s: invalid data\n", __func__);
		return;
	}
	cd = tdev->thp_core;
	finger_status = !!(thp_get_status(THP_STATUS_UDFP));
	/* syna format: byte[16:17] = x_coor byte[18:19] = y_coor */
	udfp_data->tpud_data.tp_x =
		read_buff[16] + (read_buff[17] << 8);
	udfp_data->tpud_data.tp_y =
		read_buff[18] + (read_buff[19] << 8);
	/* syna format: byte[20~23] = udfp_event */
	tmp_event = read_buff[20] + (read_buff[21] << 8) +
		(read_buff[22] << 16) + (read_buff[23] << 24);
	thp_log_info("touch event = %u\n", tmp_event);
	udfp_data->tpud_data.udfp_event = TP_FP_EVENT_MAX;
	if (finger_status) {
		if (tmp_event == FP_CORE_AREA_FINGER_DOWN)
			udfp_data->tpud_data.udfp_event = TP_EVENT_FINGER_DOWN;
		if (tmp_event == FP_CORE_AREA_FINGER_UP)
			udfp_data->tpud_data.udfp_event = TP_EVENT_FINGER_UP;
	}
	if (cd->aod_touch_status) {
		if ((tmp_event == FP_VALID_AREA_FINGER_DOWN) ||
			(tmp_event == FP_VALID_AREA_FINGER_UP))
			udfp_data->aod_event = AOD_VALID_EVENT;
	}
}

static int parse_event_info(struct thp_device *tdev,
	const char *read_buff, struct thp_udfp_data *udfp_data, int len)
{
	unsigned int udfp_event;
	int retval;
	unsigned char report_to_ap_cmd[SYNA_CMD_GESTURE_LEN] = { 0xC7, 0x05,
		0x00, 0x33, 0x00, 0x00, 0x00, 0x00 };

	if ((!tdev) || (!read_buff) || (!udfp_data) || (len <= 0)) {
		thp_log_err("%s: invalid data\n", __func__);
		return -EINVAL;
	}
	udfp_data->tpud_data.udfp_event = TP_FP_EVENT_MAX;
	/* read_buff[12~15] = 0xff touch event */
	if ((read_buff[12] == TOUCH_EVENT_TYPE) &&
		(read_buff[13] == TOUCH_EVENT_TYPE) &&
		(read_buff[14] == TOUCH_EVENT_TYPE) &&
		(read_buff[15] == TOUCH_EVENT_TYPE)) {
		parse_touch_event_info(tdev, read_buff, udfp_data, len);
	} else {
		/* read_buff[12~15] = gesture event */
		udfp_event = read_buff[12] + (read_buff[13] << 8) +
			(read_buff[14] << 16) + (read_buff[15] << 24);
		thp_log_info("gesture event = %d\n", udfp_event);
		if (udfp_event == DOUBLE_TAP_FLAG) {
			udfp_data->key_event = TS_DOUBLE_CLICK;
			/* cmd[4] event */
			report_to_ap_cmd[TOUCH_GESTURE_CMD] |=
				THP_GESTURE_DOUBLE_CLICK;
		}
		memset(spi_write_buf, 0, SPI_READ_WRITE_SIZE);
		memcpy(spi_write_buf, report_to_ap_cmd,
			sizeof(report_to_ap_cmd));
		retval = touch_driver_spi_write(tdev->thp_core->sdev,
			spi_write_buf, sizeof(report_to_ap_cmd));
		if (retval < 0)
			thp_log_err("%s:spi write failed\n", __func__);
	}
	return 0;
}

static int touch_driver_get_event_info(struct thp_device *tdev,
	struct thp_udfp_data *udfp_data)
{
	unsigned char *data = spi_read_buf;
	unsigned char *debug_data = spi_read_buf;
	int retval;
	int i;
	unsigned int length;

	thp_log_info("%s enter\n", __func__);
	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev) ||
		(!data)) {
		thp_log_err("%s: input dev is null\n", __func__);
		return -EINVAL;
	}
	retval = thp_bus_lock();
	memset(data, 0, SPI_READ_WRITE_SIZE);
	if (retval < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	retval = touch_driver_spi_read(tdev->thp_core->sdev, data,
		MESSAGE_HEADER_SIZE);
	if (retval < 0) {
		thp_log_err("%s: Failed to read length\n", __func__);
		goto error;
	}
	thp_log_info("data[0]=0x%02x data[1]=0x%02x data[2]=0x%02x data[3]=0x%02x\n",
		data[0], data[1], data[2], data[3]);
	length = (data[3] << 8) | data[2]; /* data[3:2] = event length */
	if (data[2] == DEBUG_OPEN_VALUE) {
		retval = touch_driver_spi_read(tdev->thp_core->sdev, debug_data,
			DEBUG_LOG_LENGTH);
		if (retval < 0) {
			thp_log_err("%s: Failed to read length\n", __func__);
			goto error;
		}
		for (i = 2; i < DEBUG_LOG_LENGTH; i++)
			thp_log_info("debug_info[%d] = 0x%02x", i - 2,
				debug_data[i]);
	}
	if (length > (THP_MAX_FRAME_SIZE - FIRST_FRAME_USEFUL_LEN)) {
		thp_log_info("%s: out of length\n", __func__);
		length = THP_MAX_FRAME_SIZE - FIRST_FRAME_USEFUL_LEN;
	}
	thp_log_info("%s: length %d\n", __func__, length);
	if (data[1] == 0xFF) {
		thp_log_err("%s: should ignore this irq\n", __func__);
		retval = -ENODATA;
		goto error;
	}
	if ((data[0] != MESSAGE_MARKER) || (length > SYNA_CMD_GESTURE_MAX)) {
		thp_log_err("%s: incorrect marker: 0x%02x\n",
			__func__, data[0]);
		if (data[1] == STATUS_CONTINUED_READ ||
			(length > SYNA_CMD_GESTURE_MAX)) {
			/* just in case */
			thp_log_err("%s: continued Read\n", __func__);
			touch_driver_spi_read(tdev->thp_core->sdev,
				tdev->rx_buff,
				THP_MAX_FRAME_SIZE); /* drop one transaction */
		}
		retval = -ENODATA;
		goto error;
	}
	if (length) {
		retval = touch_driver_spi_read(tdev->thp_core->sdev,
			data + FIRST_FRAME_USEFUL_LEN,
			length + FIRST_FRAME_USEFUL_LEN); /* read packet */
		if (retval < 0) {
			thp_log_err("%s: Failed to read length\n", __func__);
			goto error;
		}
		retval = parse_event_info(tdev, data, udfp_data,
			SYNA_CMD_GESTURE_MAX);
		if (retval) {
			thp_log_err("parse_event_info fail %d\n", retval);
			goto error;
		}
	} else {
		thp_log_debug("%s Invalid event\n", __func__);
		retval = -ENODATA;
	}
error:
	thp_bus_unlock();
	return retval;
}
#endif

struct thp_device_ops syna_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.get_project_id = touch_driver_get_project_id,
	.exit = touch_driver_exit,
	.chip_wrong_touch = touch_driver_wrong_touch,
	.chip_gesture_report = touch_driver_gesture_report,
#ifdef CONFIG_HUAWEI_SHB_THP
	.switch_int_sh = touch_driver_switch_int_sh,
	.switch_int_ap = touch_driver_switch_int_ap,
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
	.get_event_info = touch_driver_get_event_info,
	.tp_lowpower_ctrl = touch_driver_set_lowpower_state,
	.tp_aod_event_ctrl = touch_driver_set_aod_state,
#endif

};

static int __init touch_driver_module_init(void)
{
	int rc;
	struct thp_device *dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: called\n", __func__);
	if (!dev) {
		thp_log_err("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}
	tx_buf = NULL;
	spi_xfer = NULL;
	dev->ic_name = SYNAPTICS_IC_NAME;
	dev->dev_node_name = THP_SYNA_DEV_NODE_NAME;
	dev->ops = &syna_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(dev, NO_SYNC_TIMEOUT);
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
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
	if (dev) {
		kfree(dev->tx_buff);
		dev->tx_buff = NULL;

		kfree(dev->rx_buff);
		dev->rx_buff = NULL;

		kfree(dev);
		dev = NULL;
	}
	return rc;
}
static void __exit touch_driver_module_exit(void)
{
	thp_log_err("%s: called\n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);

