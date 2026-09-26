/*
 * Thp driver code for himax
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
#include <asm/byteorder.h>
#include "huawei_thp.h"

#include <linux/firmware.h>
#if defined(CONFIG_HMX_DB)
#include <linux/regulator/consumer.h>
#endif
#define THP_HX83112_DEV_NODE_NAME "himax-hx83112"
#define HX83112_IC_NAME "himax"
#define THP_HIMAX_INCELL_DEV_NODE_NAME "himax-incell"

#define SPI_BITS_PER_WORD 8
#define SPI_BITS_PER_WORD_READ 8
#define SPI_BITS_PER_WORD_WRITE 8

#define SPI_FORMAT_ARRAY_SIZE 3

#define COMM_TEST_RW_LENGTH 8

#define COMM_TEST_RW_RETRY_TIME 3
#define COMM_TEST_RW_RETRY_DELAY_MS 50
#define NO_ERR 0

#define HIMAX_BUS_RETRY_TIMES 10
#define HX_83112A_SERIES_PWON 14
#define HIMAX_NORMAL_DATA_LEN 4
#define HIMAX_NORMAL_ADDR_LEN 4
#define HIMAX_LONG_DATA_LEN 8
#define HIMAX_MAX_DATA_LEN 256
#define HIMAX_HEADER_LEN 4
#define HIMAX_BEGING_ADDR 0x00
#define HIMAX_REG_DATA_LEN 1
#define HIMAX_CONTI_BURST_ADDR 0x13
#define HIMAX_CONTI_BURST_EN 0x31
#define HIMAX_CONTI_BURST_DIS
#define HIMAX_SSOFF_ADDR_FIRST 0x31
#define HIMAX_SSOFF_CMD_FIRST 0x27
#define HIMAX_SSOFF_ADDR_SECOND 0x32
#define HIMAX_SSOFF_CMD_SECOND 0x95
#define HIMAX_AUTO_PLUS_4_ADDR 0x0D
#define HIMAX_AUTO_PLUS_4_EN 0x11
#define HIMAX_AUTO_PLUS_4_DIS 0x10
#define HIMAX_WAKEUP_ADDR 0x08
#define HIMAX_FW_SSOFF 0x0C
#define HIMAX_HEADER_OK_HBYTE 0xA5
#define HIMAX_HEADER_OK_LBYTE 0x5A
#define HIMAX_REG_READ_EN_ADDR 0x0C
#define HIMAX_EVENT_STACK_CMD_ADDR 0x30
#define HIMAX_REG_READ_EN 0x00
#define HIMAX_SYS_RST_ADDR 0x90000018
#define HIMAX_SYS_RST_CMD 0x00000055
#define HIMAX_WTDOG_OFF_ADDR 0x9000800C
#define HIMAX_WTDOG_OFF_CMD 0x0000AC53
#define HIMAX_CHK_FW_STATUS 0x900000A8
#define HIMAX_CLK_ON_ADDR 0x9000005C
#define HIMAX_CLK_ON_CMD 0x000000A5
#define HIMAX_CLK_ON_DELAY_MS 20
#define HIMAX_ICID_ADDR 0x900000D0
#define HIMAX_ICID_83112A 0x83112A00
#define HIMAX_ICID_83102D 0x83102D00
#define HIMAX_DDREG_ADDR 0x90000020
#define HIMAX_DDREG_CMD 0x00000000
#define HIMAX_TCON_RST_ADDR 0x800201E0
#define HIMAX_TCON_RST_LOW 0x00000000
#define HIMAX_102D_TCON_RST_ADDR 0x80020020
#define HIMAX_102D_TCON_RST_ON_CMD 0x00000000
#define HIMAX_102D_TCON_RST_OFF_CMD 0x00000001
#define HIMAX_102D_ADC_RST_ADDR 0x80020094
#define HIMAX_102D_ADC_RST_ON_CMD 0x00000000
#define HIMAX_102D_ADC_RST_OFF_CMD 0x00000001
#define HIMAX_RESET_DELAY_MS 10
#define HIMAX_TCON_RST_HIGH 0x00000001
#define HIMAX_RAWDATA_ADDR 0x10000000
#define HIMAX_RAWDATA_HEADER 0x00005AA5
#define HIMAX_SET_RAWDATA_ADDR 0x800204B4
#define HIMAX_SET_IIR_CMD 0x00000009
#define HIMAX_CHK_KEY_ADDR 0x800070E8
#define HIMAX_SPI_W_BUF_MAX_LEN 6
#define HIMAX_TPCOLOR_LEN 2
#define HIMAX_ACTUAL_PROJECTID_LEN 9
#define HIMAX_HISPEED_SUPPORT_REG 0x80000040
#define HIMAX_HISPEED_SUPPORT_VAL 0x00000001
#define SUSPEND_IN_GESTURE 1
#define RESUME_OUT_GESTURE 0
#define TP_COLOR_SIZE 15
#define INFO_SECTION_NUM 2
#define INFO_START_ADDR 0x20000
#define INFO_PAGE_LENGTH 0x1000
#define NOR_READ_LENGTH 128
#define HX_83102E_ID_PART_1 0x83
#define HX_83102E_ID_PART_2 0x10
#define HX_83102E_ID_PART_3 0x2E
#define ADDR_DD_ICID_DETAIL_HX83102 0xC3
#define ADDR_DD_PEN_EN_HX83102 0xD1
#define DATA_IC_INFO_COMMON_HX83102_0 0x2E
#define DATA_IC_INFO_COMMON_HX83102_1 0x00
#define DATA_IC_INFO_COMMON_HX83102_2 0x50
#define DATA_IC_INFO_SP_HX83102_0 0x27
#define DATA_IC_INFO_SP_HX83102_1 0x77
#define DATA_IC_INFO_SP_HX83102_2 0x7F
#define DATA_IC_INFO_SP_HX83102_2A 0x40
#define WAIT_FOR_SPI_BUS_RESUMED_DELAY 20
#define WAIT_FOR_SPI_BUS_RESUMED_DELAY_QCOM 100
#define WAIT_FOR_SPI_BUS_READ_DELAY 5
#define GESTURE_EVENT_RETRY_TIME 20
#define LOW_8BIT 0x100
#define HIGH_8BIT 0x1000000
#define STR_TYPE 0
#define NUM_TYPE 1
#define FUNC_EN 1
#define FUNC_DIS 0
#define DATA_INIT 0x00000000
#define ADDR_READ_MODE_CHK 0x900000A8
#define ADDR_DD_OSC_HX83102 0x9000009C
#define DATA_DD_OSC_HX83102_EN 0x000000DD
#define ADDR_DD_PW_HX83102 0x90000280
#define DATA_DD_PW_HX83102_EN 0x000000A5
#define ENTER_SAVE_MODE 0x0C
#define REG_WRITE_INTERVAL 20
#define CRC_DATA_THREE 0x00
#define CRC_DATA_TWO 0x99
#define ONE_BYTE_LEN 8
#define LOW_BIT 0x01
#define FW_CRC_TRY_TIMES 100
#define HX_DDREG_FORMAT 0x30000000
#define HX_DDREG_REG_IDX 12
#define HX_DDREG_BANK_IDX 8
#define REFORMAT_SIZE 4
#define MOVE_8BIT 8
#define MOVE_16BIT 16
#define MOVE_24BIT 24
#define ADDR_EN_BURST_MODE 0x13
#define DATA_EN_BURST_MODE 0x31
#define ADDR_AHB 0x0D
#define DATA_AHB 0x10
#define DATA_AHB_AUTO 0x11
#define ONE_BYTE_CMD 1
#define FOUR_BYTE_CMD 4
#define ADDR_CRC 0x80050020
#define ADDR_CRC_DATAMAXLEN_SET 0x80050028
#define ADDR_CRC_STATUS_SET 0x80050000
#define ADDR_CRC_RESULT 0x80050018
#define HX_SLEEP_10MS 10
#define THP_PROJECTID_LEN 9
#define THP_PROJECTID_PRODUCT_NAME_LEN 4
#define THP_PROJECTID_IC_NAME_LEN 2
#define THP_PROJECTID_VENDOR_NAME_LEN 3
#define VENDOR_ID_BOE "130"
#define VENDOR_ID_BOE_1 "131" /* boe module without c21 capacitance */
#define VENDOR_ID_INX "120"
#define VENDOR_NAME_BOE "boe"
#define VENDOR_NAME_INX "cmi"
#define HX_BOE_TX_NUM 32
#define HX_BOE_RX_NUM 48
#define HX_INX_TX_NUM 30
#define HX_INX_RX_NUM 48
#define HX_DB_CLICK 0x01
#define HX_WAKEUP_MEMO 0x02
#define BYTE_2 2
#define OFFSET_GES 14
#define XFER_CNT 10

static uint8_t *g_huawei_project_id;
static uint8_t *g_huawei_cg_color;
static uint8_t *g_huawei_sensor_id;
#ifdef CONFIG_VMAP_STACK
static uint8_t *g_dma_rx_buf;
static uint8_t *g_dma_tx_buf;
static struct spi_transfer *g_transfer;
#endif
__attribute__((weak)) u8 cypress_ts_kit_color[TP_COLOR_SIZE];
static int touch_driver_83112_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len);

enum Hmx_Tp_Error_State {
	FWU_GET_SYSINFO_FAIL = 0,
	FWU_GENERATE_FW_NAME_FAIL,
	FWU_REQUEST_FW_FAIL,
	FWU_FW_CRC_ERROR,
	FWU_FW_WRITE_ERROR,
	TP_WOKR_READ_DATA_ERROR = 5,
	TP_WOKR_CHECKSUM_INFO_ERROR,
	TP_WOKR_CHECKSUM_ALL_ERROR,
	TP_WOKR_HEAD_ERROR,
	TS_UPDATE_STATE_UNDEFINE = 255,
};

enum Temp_Data {
	DATA_0 = 0,
	DATA_1,
	DATA_2,
	DATA_3,
	DATA_4,
	DATA_5,
	DATA_6,
	DATA_7,
	DATA_8,
};

static u32 himax_id_match_table[] = {
	0x83112A00, /* chip HX83112A id */
	0x83112B00, /* chip HX83112B id */
	0x83112C00, /* chip HX83112C id */
	0x83112E00, /* chip HX83112E id */
	0x83102D00, /* chip HX83102D id */
	0x83102E00, /* chip HX83102E id */
};

struct himax_thp_private_data {
	int hx_get_frame_optimized_flag;
	u32 himax_ic_83102_flag;
	u32 himax_ic_hispeed_support;
	u32 himax_gesture_need_lcd_rst;
	u32 himax_gesture_need_msleep_for_qcom;
};
struct himax_thp_private_data thp_private_data;
static struct spi_device *hx_spi_dev;

struct himax_spi_buf {
	uint8_t *w_buf;
	uint8_t *r_buf_a;
	uint8_t *r_buf_b;
} g_himax_spi_buf;

void vendor_assign_data(uint32_t cmd, uint8_t *tmp_value, uint32_t length)
{
	tmp_value[DATA_3] = cmd / HIGH_8BIT;
	tmp_value[DATA_2] = (cmd >> MOVE_16BIT) % LOW_8BIT;
	tmp_value[DATA_1] = (cmd >> MOVE_8BIT) % LOW_8BIT;
	tmp_value[DATA_0] = cmd % LOW_8BIT;
}

void himax_assign_data(uint32_t cmd, uint8_t *tmp_value)
{
	int index = 3;

	tmp_value[index--] = cmd / 0x1000000;
	tmp_value[index--] = (cmd >> 16) % 0x100;
	tmp_value[index--] = (cmd >> 8) % 0x100;
	tmp_value[index--] = cmd % 0x100;
}

static void touch_driver_spi_complete(void *arg)
{
	complete(arg);
}

static int touch_driver_spi_sync(struct thp_device *tdev,
	struct spi_message *message)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int status;

	message->complete = touch_driver_spi_complete;
	message->context = &done;

	if (hx_spi_dev == NULL) {
		status = -ESHUTDOWN;
	} else {
		thp_spi_cs_set(GPIO_HIGH);
		status = thp_spi_sync(hx_spi_dev, message);
	}

	return status;
}

static int touch_driver_spi_read(struct thp_device *tdev,
	uint8_t *command, uint8_t command_len,
	uint8_t *data, unsigned int length, uint8_t to_retry)
{
	struct spi_message message;
#ifndef CONFIG_VMAP_STACK
	struct spi_transfer xfer[2];
#endif
	int retry;
	int error;

	spi_message_init(&message);
#ifdef CONFIG_VMAP_STACK
	if ((!g_dma_rx_buf) || (!g_dma_tx_buf) ||
		(length > THP_MAX_FRAME_SIZE)) {
		thp_log_err("dma_buf kzalloc failed\n");
		return -ENOMEM;
	}
	memset(g_transfer, 0,
		XFER_CNT * sizeof(struct spi_transfer));
	memcpy(g_dma_rx_buf, data, length);
	memcpy(g_dma_tx_buf, command, command_len);

	g_transfer[0].tx_buf = g_dma_tx_buf;
	g_transfer[1].rx_buf = g_dma_rx_buf;
	g_transfer[0].len = command_len;
	spi_message_add_tail(&g_transfer[0], &message);

	g_transfer[1].len = length;
	spi_message_add_tail(&g_transfer[1], &message);
#else
	memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = command;
	xfer[0].len = command_len;
	spi_message_add_tail(&xfer[0], &message);

	xfer[1].rx_buf = data;
	xfer[1].len = length;
	spi_message_add_tail(&xfer[1], &message);
#endif

	for (retry = 0; retry < to_retry; retry++) {
		thp_spi_cs_set(GPIO_HIGH);
		error = thp_spi_sync(hx_spi_dev, &message);
		if (unlikely(error))
			thp_log_err("SPI read error: %d\n", error);
		else
			break;
	}

	if (retry == to_retry) {
		thp_log_err("%s: SPI read error retry over %d\n",
			__func__, to_retry);
		return -EIO;
	}
#ifdef CONFIG_VMAP_STACK
	if (!data) {
		thp_log_err("data is NULL\n");
		return -ENOMEM;
	}
	memcpy(data, g_dma_rx_buf, length);
#endif

	return 0;
}

static int touch_driver_spi_write(struct thp_device *tdev,
	uint8_t *buf, unsigned int length)
{
	int ret;
	struct spi_message message;

#ifndef CONFIG_VMAP_STACK
	struct spi_transfer	t = {
		.tx_buf		= buf,
		.len		= length,
		.bits_per_word = SPI_BITS_PER_WORD_READ,
	};
	spi_message_init(&message);
	spi_message_add_tail(&t, &message);
#else
	memcpy(g_dma_tx_buf, buf, length);

	g_transfer[0].tx_buf = g_dma_tx_buf;
	g_transfer[0].len = length;
	g_transfer[0].bits_per_word = SPI_BITS_PER_WORD_READ;

	spi_message_init(&message);
	spi_message_add_tail(&g_transfer[0], &message);
#endif

	ret = touch_driver_spi_sync(tdev, &message);

	return ret;
}

#define READ_COMMAND_HEAD 0xF3
#define READ_COMMAND_TAIL 0x00

static int touch_driver_83102_bus_read(struct thp_device *tdev,
	uint8_t command, uint8_t *data,
	unsigned int length, uint8_t toretry)
{
	uint8_t spi_format_buf[SPI_FORMAT_ARRAY_SIZE];

	if (!tdev) {
		thp_log_info("%s:dev is invalid\n", __func__);
		return -EINVAL;
	}
	if (!data) {
		thp_log_info("%s:data addr is invalid\n", __func__);
		return -EINVAL;
	}
	if (!length) {
		thp_log_info("%s:read length is invalid\n", __func__);
		return -EINVAL;
	}
	/* 0xF3 is head of command */
	spi_format_buf[DATA_0] = READ_COMMAND_HEAD;
	spi_format_buf[DATA_1] = command;
	/* 0x00 is tail of command */
	spi_format_buf[DATA_2] = READ_COMMAND_TAIL;

	return touch_driver_spi_read(tdev, &spi_format_buf[DATA_0],
		SPI_FORMAT_ARRAY_SIZE, data, length, toretry);
}

static int touch_driver_bus_read(struct thp_device *tdev, uint8_t command,
	uint8_t *data, unsigned int length, uint8_t to_retry)
{
	uint8_t *spi_format_buf = g_himax_spi_buf.r_buf_b;
	struct himax_thp_private_data *himax_p = NULL;
	int ret;
	int index = 0;

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_83102_flag) {
		ret = touch_driver_83102_bus_read(tdev,
			command, data, length, to_retry);
		return ret;
	}
	if (!spi_format_buf) {
		thp_log_info("%s:spi_format_buf is NULL\n", __func__);
		return -EINVAL;
	}
	/* 0xF3 is head of command */
	spi_format_buf[index++] = 0xF3;
	spi_format_buf[index++] = command;
	/* 0x00 is tail of command */
	spi_format_buf[index++] = 0x00;

	return touch_driver_spi_read(tdev, &spi_format_buf[0],
		SPI_FORMAT_ARRAY_SIZE, data, length, to_retry);
}

#define WRITE_COMMAND_HEAD 0xF2
static int touch_driver_83102_bus_write(struct thp_device *tdev,
	uint8_t command, uint8_t *data,
	unsigned int length, uint8_t toretry)
{
	uint8_t spi_format_buf[HIMAX_NORMAL_DATA_LEN + HIMAX_NORMAL_ADDR_LEN + DATA_2];
	int i;

	if (!tdev) {
		thp_log_info("%s:dev is invalid\n", __func__);
		return -EINVAL;
	}
	if (!data) {
		thp_log_info("%s:data addr is invalid\n", __func__);
		return -EINVAL;
	}
	if ((!length) || (length > HIMAX_NORMAL_DATA_LEN + HIMAX_NORMAL_ADDR_LEN)) {
		thp_log_info("%s:read length is invalid\n", __func__);
		return -EINVAL;
	}
	/* 0xF2 is head of command */
	spi_format_buf[DATA_0] = WRITE_COMMAND_HEAD;
	spi_format_buf[DATA_1] = command;

	for (i = DATA_0; i < length; i++)
		spi_format_buf[i + DATA_2] = data[i];

	return touch_driver_spi_write(tdev, spi_format_buf, length + DATA_2);
}

static int touch_driver_bus_write(struct thp_device *tdev, uint8_t command,
	uint8_t *data, unsigned int length, uint8_t to_retry)
{
	uint8_t *spi_format_buf = g_himax_spi_buf.w_buf;
	struct himax_thp_private_data *himax_p = NULL;
	int i;
	int ret;

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_83102_flag) {
		ret = touch_driver_83102_bus_write(tdev,
			command, data, length, to_retry);
		return ret;
	}
	if (!spi_format_buf) {
		thp_log_info("%s:spi_format_buf is NULL\n", __func__);
		return -EINVAL;
	}
	/* 0xF2 is head of command */
	spi_format_buf[0] = 0xF2;
	spi_format_buf[1] = command;

	for (i = 0; i < length; i++)
		spi_format_buf[i + 2] = data[i];

	/* 2 bytes of head */
	return touch_driver_spi_write(tdev, spi_format_buf, length + 2);
}

static int touch_driver_register_read(struct thp_device *tdev,
	uint8_t *read_addr, unsigned int read_length, uint8_t *read_data)
{
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN];
	/* Restore the address */
	unsigned int address;
	int index = 0;

	address = (read_addr[3] << MOVE_24BIT) + (read_addr[2] << MOVE_16BIT) +
		(read_addr[1] << MOVE_8BIT) + read_addr[0];
	tmp_data[index++] = (uint8_t)address;
	tmp_data[index++] = (uint8_t)(address >> MOVE_8BIT);
	tmp_data[index++] = (uint8_t)(address >> MOVE_16BIT);
	tmp_data[index++] = (uint8_t)(address >> MOVE_24BIT);

	if (touch_driver_bus_write(tdev, HIMAX_BEGING_ADDR, tmp_data,
		HIMAX_NORMAL_DATA_LEN, HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c access fail!\n", __func__);
		return -ENOMEM;
	}
	tmp_data[0] = HIMAX_REG_READ_EN;
	if (touch_driver_bus_write(tdev, HIMAX_REG_READ_EN_ADDR, tmp_data, 1,
		HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c access fail!\n", __func__);
		return -ENOMEM;
	}

	if (touch_driver_bus_read(tdev, HIMAX_WAKEUP_ADDR, read_data,
		read_length, HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c access fail!\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

static void touch_driver_register_write(struct thp_device *tdev,
	uint8_t *write_addr, uint8_t len, uint8_t *write_data)
{
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN + HIMAX_NORMAL_ADDR_LEN];

	if (!tdev) {
		thp_log_err("%s:dev null!\n", __func__);
		return;
	}
	if (!write_addr) {
		thp_log_err("%s:write_addr null!\n", __func__);
		return;
	}
	if (!len) {
		thp_log_err("%s:length invalid!\n", __func__);
		return;
	}
	if (!write_data) {
		thp_log_err("%s:write_data null!\n", __func__);
		return;
	}
	tmp_data[DATA_0] = write_addr[DATA_0];
	tmp_data[DATA_1] = write_addr[DATA_1];
	tmp_data[DATA_2] = write_addr[DATA_2];
	tmp_data[DATA_3] = write_addr[DATA_3];
	tmp_data[DATA_4] = write_data[DATA_0];
	tmp_data[DATA_5] = write_data[DATA_1];
	tmp_data[DATA_6] = write_data[DATA_2];
	tmp_data[DATA_7] = write_data[DATA_3];

	if (touch_driver_bus_write(tdev, HIMAX_BEGING_ADDR, tmp_data,
		sizeof(tmp_data), HIMAX_BUS_RETRY_TIMES) < DATA_0) {
		thp_log_err("%s: i2c access fail!\n", __func__);
		return;
	}
}

static void touch_driver_83102_interface_on(struct thp_device *tdev)
{
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN];

	if (!tdev) {
		thp_log_err("%s:dev null!\n", __func__);
		return;
	}
	if (touch_driver_bus_read(tdev, HIMAX_WAKEUP_ADDR, tmp_data,
		HIMAX_NORMAL_DATA_LEN, HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c access fail!\n", __func__);
		return;
	}
}

static void touch_driver_interface_on(struct thp_device *tdev)
{
	uint8_t *tmp_data = g_himax_spi_buf.r_buf_a;
	struct himax_thp_private_data *himax_p = NULL;

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return;
	}
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_83102_flag) {
		touch_driver_83102_interface_on(tdev);
		return;
	}
	if (!tmp_data) {
		thp_log_info("%s:tmp_data is NULL\n", __func__);
		return;
	}
	/* Read a dummy register to wake up I2C */
	if (touch_driver_bus_read(tdev, HIMAX_WAKEUP_ADDR, tmp_data,
		HIMAX_NORMAL_DATA_LEN, HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c access fail!\n", __func__);
		return;
	}
}

#if defined(CONFIG_HMX_DB)

static int touch_driver_regulator_configure(struct device *dev)
{
	int ret;
	struct regulator *reg;
	struct regulator *reg2;

	if (!dev) {
		thp_log_err("%s:dev null!\n", __func__);
		return;
	}
	reg = regulator_get(dev, "vdd");
	if (IS_ERR(reg)) {
		thp_log_err("%s: Failed to get regulator vdd\n",
			__func__);
		ret = PTR_ERR(reg);
		return ret;
	}

	reg2 = regulator_get(dev, "avdd");
	if (IS_ERR(reg2)) {
		thp_log_err("%s: Failed to get regulator avdd\n",
		  __func__);
		ret = PTR_ERR(reg2);
		regulator_put(reg);
		return ret;
	}

	ret = regulator_enable(reg);
	if (ret) {
		thp_log_err("%s: Failed to enable regulator vdd\n",
		  __func__);
		return ret;
	}
	thp_do_time_delay(HIMAX_CLK_ON_DELAY_MS);

	ret = regulator_enable(reg2);
	if (ret) {
		thp_log_err("%s: Failed to enable regulator avdd\n",
		 __func__);
		return ret;
	}
	thp_do_time_delay(HIMAX_CLK_ON_DELAY_MS);

	thp_log_info("%s: configure regulator succeed!\n", __func__);

	return 0;
}
#endif

static int touch_driver_parse_gesture_data(struct thp_device *tdev,
	const u8 *data, unsigned int *gesture_wakeup_value)
{
	struct thp_core_data *cd = NULL;
	int length;
	int tx_num;
	int rx_num;

	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s: null ptr\n", __func__);
		return -EINVAL;
	}
	if (!strncmp(cd->vendor_name,
		VENDOR_NAME_BOE, THP_PROJECTID_VENDOR_NAME_LEN)) {
		tx_num = HX_BOE_TX_NUM;
		rx_num = HX_BOE_RX_NUM;
	} else if (!strncmp(cd->vendor_name,
		VENDOR_NAME_INX, THP_PROJECTID_VENDOR_NAME_LEN)) {
		tx_num = HX_INX_TX_NUM;
		rx_num = HX_INX_RX_NUM;
	} else {
		thp_log_err("%s: not support this vendor_id\n", __func__);
		return -EINVAL;
	}
	length = BYTE_2 * tx_num * rx_num + OFFSET_GES;
	length--;
	if ((data[length] == HX_DB_CLICK) ||
		(data[length - 1] == HX_DB_CLICK)) {
		*gesture_wakeup_value = TS_DOUBLE_CLICK;
	} else if ((data[length] == HX_WAKEUP_MEMO) ||
		(data[length - 1] == HX_WAKEUP_MEMO)) {
		*gesture_wakeup_value = TS_STYLUS_WAKEUP_TO_MEMO;
	} else {
		thp_log_err("%s: not support this gesture\n", __func__);
		return -EINVAL;
	}
	return NO_ERR;
}

static int touch_driver_gesture_report(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	int retval;
	int i;
	u8 *frame_read_buf = NULL;
	struct himax_thp_private_data *himax_p = NULL;

	thp_log_info("%s enter\n", __func__);
	if ((!tdev) || (!tdev->thp_core) ||
		(!tdev->thp_core->sdev) || (!tdev->thp_core->frame_read_buf)) {
		thp_log_err("%s: input dev is null\n", __func__);
		return -EINVAL;
	}
	if ((gesture_wakeup_value == NULL) ||
		(!tdev->thp_core->support_gesture_mode)) {
		thp_log_err("%s, gesture not support\n", __func__);
		return -EINVAL;
	}
	frame_read_buf = (u8 *)tdev->thp_core->frame_read_buf;
	himax_p = tdev->private_data;
	if (himax_p->himax_gesture_need_msleep_for_qcom)
		msleep(WAIT_FOR_SPI_BUS_RESUMED_DELAY_QCOM);
	else
		msleep(WAIT_FOR_SPI_BUS_RESUMED_DELAY);
	/* wait spi bus resume */
	for (i = 0; i < GESTURE_EVENT_RETRY_TIME; i++) {
		retval = touch_driver_83112_get_frame(tdev,
			frame_read_buf, THP_MAX_FRAME_SIZE);
		if (retval) {
			thp_log_err("%s: spi not work normal, ret %d retry\n",
				__func__, retval);
			msleep(WAIT_FOR_SPI_BUS_READ_DELAY);
		} else {
			thp_log_info("%s: spi work normal, got gesture\n",
				__func__);
			break;
		}
	}
	if (retval) {
		thp_log_err("%s: failed read gesture head info, ret %d\n",
			__func__, retval);
		return retval;
	}
	retval = touch_driver_parse_gesture_data(tdev,
		frame_read_buf, gesture_wakeup_value);

	thp_log_info("%s exit\n", __func__);
	return retval;
}

int touch_driver_init(struct thp_device *tdev)
{
	int rc;
	int reg;
	unsigned int value = 0;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *hx83112_node = of_get_child_by_name(cd->thp_node,
						THP_HX83112_DEV_NODE_NAME);
	struct himax_thp_private_data *himax_p = tdev->private_data;

	thp_log_info("Enter %s\n", __func__);

	hx_spi_dev = tdev->sdev;

	thp_log_info("%s: called\n", __func__);

	if (!hx83112_node) {
		thp_log_err("%s: dev not config in dts\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(hx83112_node, "get_frame_optimized_method",
		&value);
	if (rc) {
		himax_p->hx_get_frame_optimized_flag = 0;
		thp_log_err(
			"%s:hx_get_frame_optimized_method_flag not found,use default value\n",
			__func__);
	} else {
		himax_p->hx_get_frame_optimized_flag = value;
		thp_log_info("%s:hx_get_frame_optimized_method_flag %d\n",
			__func__, value);
	}

	rc = of_property_read_u32(hx83112_node,
		"himax_ic_83102_flag", &value);
	if (rc) {
		himax_p->himax_ic_83102_flag = 0;
		thp_log_err("%s: 83102_flag not found\n",
			__func__);
	} else {
		himax_p->himax_ic_83102_flag = value;
		thp_log_info("%s: 9L_flag %d\n",
			__func__, value);
	}

	rc = of_property_read_u32(hx83112_node,
		"himax_ic_hispeed_support", &value);
	if (rc) {
		himax_p->himax_ic_hispeed_support = 0;
		thp_log_err("%s: hispeed_flag not found\n",
			__func__);
	} else {
		himax_p->himax_ic_hispeed_support = value;
		thp_log_info("%s: hispeed_flag %d\n",
			__func__, value);
	}

	rc = of_property_read_u32(hx83112_node,
		"himax_gesture_need_lcd_rst", &value);
	if (rc) {
		himax_p->himax_gesture_need_lcd_rst = 0;
		thp_log_err("%s: need_lcd_rst not found\n",
			__func__);
	} else {
		himax_p->himax_gesture_need_lcd_rst = value;
		thp_log_info("%s: lcd_rst_flag %d\n",
			__func__, value);
	}

	rc = of_property_read_u32(hx83112_node,
		"himax_gesture_need_msleep_for_qcom", &value);
	if (rc) {
		himax_p->himax_gesture_need_msleep_for_qcom = 0;
		thp_log_err("%s: need_msleep not found\n",
			__func__);
	} else {
		himax_p->himax_gesture_need_msleep_for_qcom = value;
		thp_log_info("%s: msleep_for_qcom flag %d\n",
			__func__, value);
	}
	rc = thp_parse_spi_config(hx83112_node, cd);
	if (rc) {
		thp_log_err("%s: spi config parse fail\n", __func__);
		return -ENODEV;
	}

	rc = thp_parse_timing_config(hx83112_node, &tdev->timing_config);
	if (rc) {
		thp_log_err("%s: timing config parse fail\n", __func__);
		return -ENODEV;
	}

#if defined(CONFIG_HMX_DB)
	ret = touch_driver_regulator_configure(&(cd->sdev->dev));
	if (ret) {
		thp_log_err("%s: Failed to configure regulator\n",
			__func__);
		return ret;
	}
#else
	reg = of_get_named_gpio(hx83112_node, "himax,vdd_ana-supply", 0);
	if (reg >= 0)
		rc = gpio_direction_output(reg, 1);
	reg = of_get_named_gpio(hx83112_node, "himax,vcc_i2c-supply", 0);
	if (reg >= 0)
		rc = gpio_direction_output(reg, 1);
#endif

	return 0;
}

static int touch_driver_sense_off(struct thp_device *tdev)
{
	uint8_t tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN] = {0};
	int32_t cnt = 0;
	int ret;

	himax_assign_data(HIMAX_CLK_ON_ADDR, tmp_addr);
	himax_assign_data(HIMAX_CLK_ON_CMD, tmp_data);
	touch_driver_register_write(tdev, tmp_addr,
		HIMAX_NORMAL_ADDR_LEN, tmp_data);

	thp_do_time_delay(HIMAX_CLK_ON_DELAY_MS);

	do {
		himax_assign_data(HIMAX_CONTI_BURST_EN, tmp_data);
		if ((touch_driver_bus_write(tdev,
			HIMAX_CONTI_BURST_ADDR, tmp_data,
			HIMAX_REG_DATA_LEN, HIMAX_BUS_RETRY_TIMES)) < 0)
			return -ENOMEM;

		himax_assign_data(HIMAX_AUTO_PLUS_4_DIS, tmp_data);
		if ((touch_driver_bus_write(tdev,
			HIMAX_AUTO_PLUS_4_ADDR, tmp_data,
			HIMAX_REG_DATA_LEN, HIMAX_BUS_RETRY_TIMES)) < 0)
			return -ENOMEM;

		himax_assign_data(HIMAX_CHK_FW_STATUS, tmp_addr);
		ret = touch_driver_register_read(tdev,
			tmp_addr, HIMAX_NORMAL_DATA_LEN, tmp_data);
		if (ret)
			return ret;
		thp_log_info("%s: Check save mode data[0] = %02X\n",
			__func__, tmp_data[0]);

		if (tmp_data[0] == HIMAX_FW_SSOFF) {
			himax_assign_data(HIMAX_102D_TCON_RST_ADDR, tmp_addr);
			himax_assign_data(HIMAX_102D_TCON_RST_ON_CMD, tmp_data);
			touch_driver_register_write(tdev, tmp_addr,
				HIMAX_NORMAL_ADDR_LEN, tmp_data);
			thp_do_time_delay(HIMAX_RESET_DELAY_MS);
			himax_assign_data(HIMAX_102D_TCON_RST_OFF_CMD,
				tmp_data);
			touch_driver_register_write(tdev, tmp_addr,
				HIMAX_NORMAL_ADDR_LEN, tmp_data);

			himax_assign_data(HIMAX_102D_ADC_RST_ADDR, tmp_addr);
			himax_assign_data(HIMAX_102D_ADC_RST_ON_CMD, tmp_data);
			touch_driver_register_write(tdev, tmp_addr,
				HIMAX_NORMAL_ADDR_LEN, tmp_data);
			thp_do_time_delay(HIMAX_RESET_DELAY_MS);
			himax_assign_data(HIMAX_102D_ADC_RST_OFF_CMD, tmp_data);
			touch_driver_register_write(tdev, tmp_addr,
				HIMAX_NORMAL_ADDR_LEN, tmp_data);
			return NO_ERR;
		}

		himax_assign_data(HIMAX_SSOFF_CMD_FIRST, tmp_data);
		if ((touch_driver_bus_write(tdev,
			HIMAX_SSOFF_ADDR_FIRST, tmp_data,
			HIMAX_REG_DATA_LEN, HIMAX_BUS_RETRY_TIMES)) < 0)
			return -ENOMEM;

		himax_assign_data(HIMAX_SSOFF_CMD_SECOND, tmp_data);
		if ((touch_driver_bus_write(tdev,
			HIMAX_SSOFF_ADDR_SECOND, tmp_data,
			HIMAX_REG_DATA_LEN, HIMAX_BUS_RETRY_TIMES)) < 0)
			return -ENOMEM;

		thp_do_time_delay(HIMAX_RESET_DELAY_MS);
	} while (cnt++ < HIMAX_BUS_RETRY_TIMES);

	return -ENODEV;
}

static int touch_driver_ddreg_read(struct thp_device *tdev, uint8_t cmd,
	int bank_idx, int start_para, int read_size, uint8_t *out)
{
	int i;
	int ret = NO_ERR;
	uint32_t dd_reg_reformat;
	uint32_t bank_idx_reformat = bank_idx * REFORMAT_SIZE;
	uint8_t tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN] = {0};

	dd_reg_reformat = HX_DDREG_FORMAT +
		(cmd << HX_DDREG_REG_IDX) +
		(bank_idx_reformat << HX_DDREG_BANK_IDX) +
		(start_para);

	thp_log_info("%s:reformat addr=0x%08X, cmd=0x%02X\n",
		__func__, dd_reg_reformat, cmd);
	thp_log_info("%s:bank_idx=%d, start_para=%d, read_size=%d\n",
		__func__, bank_idx, start_para, read_size);

	for (i = 0; i < read_size; i++) {
		himax_assign_data(dd_reg_reformat + i,
			tmp_addr);
		ret = touch_driver_register_read(tdev,
			tmp_addr, HIMAX_NORMAL_DATA_LEN, tmp_data);
		if (ret)
			return ret;
		thp_log_info("%s:use %d;[3]=0x%X,[2]=0x%X,[1]=0x%X,[0]=0x%X\n",
			__func__, ((start_para + i) % HIMAX_NORMAL_DATA_LEN),
			tmp_data[DATA_3], tmp_data[DATA_2],
			tmp_data[DATA_1], tmp_data[DATA_0]);
		out[i] = tmp_data[((start_para + i) % HIMAX_NORMAL_DATA_LEN)];
	}
	return ret;
}

static void touch_driver_rw_reg_reformat_com(uint32_t addr,
	uint32_t data, uint8_t *addr_arr, uint8_t *data_arr)
{
	himax_assign_data(addr, addr_arr);
	himax_assign_data(data, data_arr);
}

static int touch_driver_9l_read_ddreg_en(struct thp_device *tdev, int en)
{
	int ret;
	uint8_t tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN] = {0};

	himax_assign_data(ADDR_READ_MODE_CHK, tmp_addr);
	ret = touch_driver_register_read(tdev, tmp_addr,
		HIMAX_NORMAL_DATA_LEN, tmp_data);
	if (ret)
		return ret;
	if (tmp_data[DATA_0] != ENTER_SAVE_MODE) {
		thp_log_info("%s: Doesn't enter save mode, need sense off!\n",
			__func__);
		ret = touch_driver_sense_off(tdev);
		if (ret)
			return ret;
	} else {
		thp_log_info("%s: Already enter into save mode!\n", __func__);
	}
	if (en) {
		thp_log_info("%s: Ready to enable to read ddreg!\n", __func__);
		touch_driver_rw_reg_reformat_com(ADDR_DD_OSC_HX83102,
			DATA_DD_OSC_HX83102_EN, &tmp_addr[DATA_0],
			&tmp_data[DATA_0]);
		touch_driver_register_write(tdev, tmp_addr,
			HIMAX_NORMAL_DATA_LEN, tmp_data);
		msleep(REG_WRITE_INTERVAL);
		touch_driver_rw_reg_reformat_com(ADDR_DD_PW_HX83102,
			DATA_DD_PW_HX83102_EN, &tmp_addr[DATA_0],
			&tmp_data[DATA_0]);
		touch_driver_register_write(tdev, tmp_addr,
			HIMAX_NORMAL_DATA_LEN, tmp_data);
	} else {
		thp_log_info("%s: Ready to disable to read ddreg!\n", __func__);
		touch_driver_rw_reg_reformat_com(ADDR_DD_PW_HX83102,
			DATA_INIT, &tmp_addr[DATA_0], &tmp_data[DATA_0]);
		touch_driver_register_write(tdev, tmp_addr,
			HIMAX_NORMAL_DATA_LEN, tmp_data);
		msleep(REG_WRITE_INTERVAL);
		touch_driver_rw_reg_reformat_com(ADDR_DD_OSC_HX83102,
			DATA_INIT, &tmp_addr[DATA_0], &tmp_data[DATA_0]);
		touch_driver_register_write(tdev, tmp_addr,
			HIMAX_NORMAL_DATA_LEN, tmp_data);
	}
	return ret;
}

static void touch_driver_burst_enable(struct thp_device *tdev,
	uint8_t auto_add_4_byte)
{
	uint8_t tmp_data[DATA_4];

	tmp_data[DATA_0] = DATA_EN_BURST_MODE;
	if (touch_driver_bus_write(tdev, ADDR_EN_BURST_MODE, tmp_data,
		sizeof(tmp_data), HIMAX_BUS_RETRY_TIMES) < DATA_0) {
		thp_log_err("%s: spi access fail!\n", __func__);
		return;
	}

	tmp_data[DATA_0] = (DATA_AHB | auto_add_4_byte);
	if (touch_driver_bus_write(tdev, ADDR_AHB, tmp_data,
		sizeof(tmp_data), HIMAX_BUS_RETRY_TIMES) < DATA_0) {
		thp_log_err("%s: spi access fail!\n", __func__);
		return;
	}
}

static uint32_t touch_driver_hw_check_crc(struct thp_device *tdev,
	uint8_t *start_addr, int reload_length)
{
	uint32_t result = NO_ERR;
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_data[DATA_4] = {0};
	uint32_t mod;
	int cnt;
	int ret;
	uint32_t length = reload_length / DATA_4;

	if (!length) {
		thp_log_err("%s:not invalid length!\n", __func__);
		return -EINVAL;
	}
	if (!start_addr) {
		thp_log_err("%s:start_addr null!\n", __func__);
		return -EINVAL;
	}
	himax_assign_data(ADDR_CRC, tmp_addr);
	touch_driver_register_write(tdev, tmp_addr,
		HIMAX_NORMAL_DATA_LEN, start_addr);

	tmp_data[DATA_3] = CRC_DATA_THREE; tmp_data[DATA_2] = CRC_DATA_TWO;
	tmp_data[DATA_1] = length >> ONE_BYTE_LEN; tmp_data[DATA_0] = length;
	himax_assign_data(ADDR_CRC_DATAMAXLEN_SET, tmp_addr);
	touch_driver_register_write(tdev, tmp_addr,
		HIMAX_NORMAL_DATA_LEN, tmp_data);

	cnt = 0;
	himax_assign_data(ADDR_CRC_STATUS_SET, tmp_addr);
	do {
		mod = ADDR_CRC_STATUS_SET % DATA_4;
		ret = touch_driver_register_read(tdev, tmp_addr,
			FOUR_BYTE_CMD, tmp_data);
		if (ret)
			return ret;
		thp_log_info("%s:tmp_data[3]=%X,tmp_data[2]=%X\n",
			__func__, tmp_data[DATA_3], tmp_data[DATA_2]);
		thp_log_info("%s:tmp_data[1]=%X,tmp_data[0]=%X\n",
			__func__, tmp_data[DATA_1], tmp_data[DATA_0]);
		if ((tmp_data[DATA_0 + mod] & LOW_BIT) == LOW_BIT) {
			thp_log_info("wait for HW ready!\n");
			msleep(HX_SLEEP_10MS);
		} else {
			himax_assign_data(ADDR_CRC_RESULT, tmp_addr);
			ret = touch_driver_register_read(tdev, tmp_addr,
				FOUR_BYTE_CMD, tmp_data);
			if (ret)
				return ret;
			thp_log_info("%s: tmp_data[3]=%X, tmp_data[2]=%X\n",
				__func__, tmp_data[DATA_3], tmp_data[DATA_2]);
			thp_log_info("%s: tmp_data[1]=%X, tmp_data[0]=%X\n",
				__func__, tmp_data[DATA_1], tmp_data[DATA_0]);
			result = ((tmp_data[DATA_3] << MOVE_24BIT) +
				(tmp_data[DATA_2] << MOVE_16BIT) +
				(tmp_data[DATA_1] << MOVE_8BIT) +
				tmp_data[DATA_0]);
			break;
		}
	} while (cnt++ < FW_CRC_TRY_TIMES);

	if (cnt < FW_CRC_TRY_TIMES)
		return result;
	else
		return FWU_FW_CRC_ERROR;
}

static int touch_driver_projectid(uint8_t type, uint32_t len, uint8_t *buffer)
{
	int j;

	if (!len) {
		thp_log_err("%s:not invalid length!\n", __func__);
		return -EINVAL;
	}
	if (!buffer) {
		thp_log_err("%s:buffer null!\n", __func__);
		return -EINVAL;
	}
	if (type == STR_TYPE) {
		g_huawei_project_id = kcalloc((len + 1),
			sizeof(*g_huawei_project_id), GFP_KERNEL);
	} else if (type == NUM_TYPE) {
		g_huawei_project_id = kcalloc(len,
			sizeof(*g_huawei_project_id), GFP_KERNEL);
	} else {
		thp_log_err("%s: project id UNKNOWN type %d\n",
			__func__, type);
	}
	if (g_huawei_project_id) {
		memcpy(g_huawei_project_id,
			buffer + MOVE_8BIT, len);
		thp_log_info("%s: project id: ", __func__);
		if (type == STR_TYPE) {
			g_huawei_project_id[len] = '\0';
			thp_log_info("%s", g_huawei_project_id);
		} else {
			for (j = DATA_0; j < len; j++)
				thp_log_info("0x%02X",
					*(g_huawei_project_id +
					j));
		}
		thp_log_info("\n");
	}
	return NO_ERR;
}

static int touch_driver_cgcolor(uint8_t type, uint32_t len, uint8_t *buffer)
{
	int j;

	if (!len) {
		thp_log_err("%s:not invalid length!\n", __func__);
		return -EINVAL;
	}
	if (buffer == NULL) {
		thp_log_err("%s:buffer null!\n", __func__);
		return -EINVAL;
	}
	if (type == STR_TYPE) {
		g_huawei_cg_color = kcalloc((len + 1),
			sizeof(*g_huawei_cg_color), GFP_KERNEL);
	} else if (type == NUM_TYPE) {
		g_huawei_cg_color = kcalloc(len,
			sizeof(*g_huawei_cg_color), GFP_KERNEL);
	} else {
		thp_log_err("%s: CG color UNKNOWN type %d\n",
			__func__, type);
	}
	if (g_huawei_cg_color) {
		memcpy(g_huawei_cg_color,
			buffer + MOVE_8BIT, len);
		thp_log_info("%s: cg_color: ", __func__);
		if (type == STR_TYPE) {
			g_huawei_cg_color[len] = '\0';
			thp_log_info("%s", g_huawei_cg_color);
		} else {
			for (j = 0; j < len; j++)
				thp_log_info("0x%02X",
					*(g_huawei_cg_color +
					j));
		}
		thp_log_info("\n");
	}
	return NO_ERR;
}

static int touch_driver_sensorid(uint8_t type, uint32_t len, uint8_t *buffer)
{
	int j;

	if (!len) {
		thp_log_err("%s:length invalid!\n", __func__);
		return -EINVAL;
	}
	if (!buffer) {
		thp_log_err("%s:buffer null!\n", __func__);
		return -EINVAL;
	}
	if (type == STR_TYPE) {
		g_huawei_sensor_id = kcalloc((len + 1),
			sizeof(*g_huawei_sensor_id), GFP_KERNEL);
	} else if (type == NUM_TYPE) {
		g_huawei_sensor_id = kcalloc(len,
			sizeof(*g_huawei_sensor_id), GFP_KERNEL);
	} else {
		thp_log_err("%s: sensor id UNKNOWN type %d\n",
			__func__, type);
	}
	if (g_huawei_sensor_id) {
		memcpy(g_huawei_sensor_id,
			buffer + MOVE_8BIT, len);
		thp_log_info("%s: sensor id: ", __func__);
		if (type == STR_TYPE) {
			g_huawei_sensor_id[len] = '\0';
			thp_log_info("%s", g_huawei_sensor_id);
		} else {
			for (j = 0; j < len; j++)
				thp_log_info("0x%02X",
					*(g_huawei_sensor_id +
					j));
		}
		thp_log_info("\n");
	}
	return NO_ERR;
}

static int touch_driver_read_panel_info(struct thp_device *tdev)
{
	uint8_t *buffer = NULL;
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_data[DATA_4] = {0};
	uint8_t tmp_buf[NOR_READ_LENGTH] = {0};
	uint32_t temp_addr;
	int32_t i;
	uint32_t saddr;
	uint32_t len;
	uint8_t title[DATA_3] = {0};
	uint8_t type;
	int ret;
	struct himax_thp_private_data *himax_p = NULL;

	if (!tdev) {
		thp_log_err("%s: not invalid device!\n", __func__);
		return -EINVAL;
	}
	buffer = kcalloc(INFO_PAGE_LENGTH, sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		thp_log_err("%s: Memory allocate FAIL!\n", __func__);
		return -ENOMEM;
	}
	ret = touch_driver_sense_off(tdev);
	if (ret)
		goto err;
	touch_driver_burst_enable(tdev, DATA_0);
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_hispeed_support) {
		himax_assign_data(HIMAX_HISPEED_SUPPORT_REG,
			tmp_addr);
		himax_assign_data(HIMAX_HISPEED_SUPPORT_VAL,
			tmp_data);
		touch_driver_register_write(tdev, tmp_addr,
			HIMAX_NORMAL_ADDR_LEN, tmp_data);
		thp_log_info("%s: enter ic hispeed mode!\n",
			__func__);
	}
	for (i = DATA_0; i < INFO_SECTION_NUM; i++) {
		saddr = INFO_START_ADDR + i * INFO_PAGE_LENGTH;
		tmp_addr[DATA_0] = saddr % LOW_8BIT;
		tmp_addr[DATA_1] = (saddr >> MOVE_8BIT) % LOW_8BIT;
		tmp_addr[DATA_2] = (saddr >> MOVE_16BIT) % LOW_8BIT;
		tmp_addr[DATA_3] = saddr / HIGH_8BIT;
		if (touch_driver_hw_check_crc(tdev,
			tmp_addr, INFO_PAGE_LENGTH) != DATA_0) {
			thp_log_err("%s:section %d CRC FAIL\n",
				__func__, i);
			goto err;
		}
		memset(buffer, DATA_0, INFO_PAGE_LENGTH);
		for (temp_addr = saddr; temp_addr < saddr +
			INFO_PAGE_LENGTH; temp_addr += DATA_4) {
			tmp_addr[DATA_0] = temp_addr % LOW_8BIT;
			tmp_addr[DATA_1] = (temp_addr >> MOVE_8BIT) % LOW_8BIT;
			tmp_addr[DATA_2] = (temp_addr >> MOVE_16BIT) % LOW_8BIT;
			tmp_addr[DATA_3] = temp_addr / HIGH_8BIT;
			ret = touch_driver_register_read(tdev,
				tmp_addr, DATA_4, tmp_buf);
			if (ret)
				goto err;
			memcpy(&buffer[temp_addr - saddr], tmp_buf, DATA_4);
		}
		len = (buffer[DATA_3] << MOVE_24BIT) |
			(buffer[DATA_2] << MOVE_16BIT) |
			(buffer[DATA_1] << MOVE_8BIT) | (buffer[DATA_0]);
		title[DATA_0] = buffer[DATA_4];
		title[DATA_1] = buffer[DATA_5];
		title[DATA_2] = buffer[DATA_6];
		type = buffer[DATA_7];
		if ((title[DATA_0] == DATA_0) && (title[DATA_1] == DATA_1) &&
			(title[DATA_2] == DATA_0)) {
			/* project id */
			ret = touch_driver_projectid(type, len, buffer);
			if (ret) {
				thp_log_err("%s: projectid read fail\n",
					__func__);
				goto err;
			}
		} else if ((title[DATA_0] == DATA_0) &&
			(title[DATA_1] == DATA_0) &&
			(title[DATA_2] == DATA_1)) {
			/* CG color */
			ret = touch_driver_cgcolor(type, len, buffer);
			if (ret) {
				thp_log_err("%s: projectid cgcolor fail\n",
					__func__);
				goto err;
			}
		} else if ((title[DATA_0] == 1) &&
			(title[DATA_1] == 1) && (title[DATA_2] == 1)) {
			/* sensor id */
			ret = touch_driver_sensorid(type, len, buffer);
			if (ret) {
				thp_log_err("%s: sensorid read fail\n",
					__func__);
				goto err;
			}
		} else {
			thp_log_err("%s: UNKNOWN title %X%X%X\n",
				__func__, title[DATA_0],
				title[DATA_1], title[DATA_2]);
			goto err;
		}
	}
	kfree(buffer);
	buffer = NULL;
	return NO_ERR;
err:
	kfree(buffer);
	buffer = NULL;
	return -EINVAL;
}

static int touch_driver_9l_ic_check(struct thp_device *tdev)
{
	int result = NO_ERR;
	uint8_t ic_info[DATA_3];
	uint8_t dd_icid_detail[DATA_8] = {0};
	uint8_t dd_pen_func_check[DATA_1] = {0};

	touch_driver_9l_read_ddreg_en(tdev, FUNC_EN);
	touch_driver_ddreg_read(tdev, ADDR_DD_ICID_DETAIL_HX83102,
		DATA_0, DATA_1, DATA_8, dd_icid_detail);
	thp_log_info("Now [8]=0x%X,[7]=0x%X,[6]=0x%X,[5]=0x%X\n",
		dd_icid_detail[DATA_7], dd_icid_detail[DATA_6],
		dd_icid_detail[DATA_5], dd_icid_detail[DATA_4]);
	thp_log_info("Now [4]=0x%X,[3]=0x%X,[2]=0x%X,[1]=0x%X\n",
		dd_icid_detail[DATA_3], dd_icid_detail[DATA_2],
		dd_icid_detail[DATA_1], dd_icid_detail[DATA_0]);
	touch_driver_ddreg_read(tdev, ADDR_DD_PEN_EN_HX83102,
		DATA_1, DATA_1, DATA_1, dd_pen_func_check);
	thp_log_info("Now [1]=0x%02X\n", dd_pen_func_check[DATA_0]);
	touch_driver_9l_read_ddreg_en(tdev, FUNC_DIS);
	ic_info[DATA_0] = dd_icid_detail[DATA_2];
	ic_info[DATA_1] = dd_icid_detail[DATA_3];
	ic_info[DATA_2] = dd_pen_func_check[DATA_0];
	if ((ic_info[DATA_0] == DATA_IC_INFO_SP_HX83102_0) &&
		(ic_info[DATA_1] == DATA_IC_INFO_SP_HX83102_1) &&
		(ic_info[DATA_2] == DATA_IC_INFO_SP_HX83102_2)) {
		result = 1;
		thp_log_err("%s: It's 9L!\n", __func__);
	} else {
		result = 0;
		thp_log_err("%s: IC can't be recognized\n", __func__);
	}
	return result;
}

static int touch_driver_9l_communication_check(struct thp_device *tdev)
{
	uint8_t tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN] = {0};
	uint8_t ic_name[HIMAX_NORMAL_DATA_LEN] = {0};
	int ret;
	int i;
	int j;

	for (i = 0; i < COMM_TEST_RW_RETRY_TIME; i++) {
		himax_assign_data(HIMAX_ICID_ADDR, tmp_addr);
		ret = touch_driver_register_read(tdev,
			tmp_addr, HIMAX_NORMAL_DATA_LEN, tmp_data);

		for (j = DATA_0; j < ARRAY_SIZE(himax_id_match_table); j++) {
			vendor_assign_data(himax_id_match_table[j],
				ic_name, HIMAX_NORMAL_DATA_LEN);
			if ((tmp_data[DATA_3] == HX_83102E_ID_PART_1) &&
				(tmp_data[DATA_2] == HX_83102E_ID_PART_2) &&
				(tmp_data[DATA_1] == HX_83102E_ID_PART_3) &&
				(touch_driver_9l_ic_check(tdev) ==
				DATA_1)) {
				break;
			} else if ((tmp_data[DATA_3] == ic_name[DATA_3]) &&
				(tmp_data[DATA_2] == ic_name[DATA_2]) &&
				(tmp_data[DATA_1] == ic_name[DATA_1])) {
				break;
			}
		}
		if (j < ARRAY_SIZE(himax_id_match_table)) {
			thp_log_info("vendor ic found\n");
			break;
		}
		thp_log_err("%s:Read driver ID register Fail:\n", __func__);
	}

	if (i == COMM_TEST_RW_RETRY_TIME)
		return -EINVAL;

	return 0;
}

static int touch_driver_83112_communication_check(struct thp_device *tdev)
{
	uint8_t tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	uint8_t *tmp_data = g_himax_spi_buf.r_buf_a;
	uint8_t ic_name[HIMAX_NORMAL_DATA_LEN] = {0};
	struct himax_thp_private_data *himax_p = NULL;
	int ret;
	int i;
	int j;

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_83102_flag) {
		ret = touch_driver_9l_communication_check(tdev);
		return ret;
	}
	if (!tmp_data) {
		thp_log_info("%s:tmp_data is NULL\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < COMM_TEST_RW_RETRY_TIME; i++) {
		himax_assign_data(HIMAX_ICID_ADDR,
			tmp_addr);
		ret = touch_driver_register_read(tdev, tmp_addr,
			COMM_TEST_RW_LENGTH, tmp_data);

		thp_log_info("%s:Read driver IC ID = %X,%X,%X\n", __func__,
			tmp_data[3], tmp_data[2], tmp_data[1]);
		for (j = 0; j < ARRAY_SIZE(himax_id_match_table); j++) {
			himax_assign_data(himax_id_match_table[j],
				ic_name);
			if ((tmp_data[3] == ic_name[3]) &&
				(tmp_data[2] == ic_name[2]) &&
				(tmp_data[1] == ic_name[1]))
				break;
		}
		if (j < ARRAY_SIZE(himax_id_match_table)) {
			thp_log_info("83112 found\n");
			break;
		}
		thp_log_err("%s:Read driver ID register Fail:\n", __func__);
	}

	if (i == COMM_TEST_RW_RETRY_TIME)
		return -EINVAL;

	return 0;
}

static void touch_driver_timing_work(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN];

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return;
	}
	cd = tdev->thp_core;
	thp_log_info("%s:called,do hard reset\n", __func__);
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	mdelay(tdev->timing_config.boot_reset_hi_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return;
	}
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#endif
	tmp_data[0] = HIMAX_SSOFF_CMD_FIRST;
	if (touch_driver_bus_write(tdev, HIMAX_SSOFF_ADDR_FIRST, tmp_data, 1,
		HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c first access fail!\n", __func__);
		return;
	}

	tmp_data[0] = HIMAX_SSOFF_CMD_SECOND;
	if (touch_driver_bus_write(tdev, HIMAX_SSOFF_ADDR_SECOND, tmp_data, 1,
		HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c second access fail!\n", __func__);
		return;
	}
}

int touch_driver_83102_chip_detect(struct thp_device *tdev)
{
	int ret;
#ifdef CONFIG_VMAP_STACK
	size_t size_xfer;
#endif
	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
#ifdef CONFIG_VMAP_STACK
	g_dma_rx_buf = (uint8_t *)(tdev->rx_buff);
	if (!g_dma_rx_buf) {
		thp_log_err("g_dma_rx_buf is null\n");
		return -ENOMEM;
	}
	g_dma_tx_buf = (uint8_t *)(tdev->tx_buff);
	if (!g_dma_tx_buf) {
		thp_log_err("g_dma_tx_buf is null\n");
		return -ENOMEM;
	}
	size_xfer = XFER_CNT * sizeof(struct spi_transfer);
	g_transfer = (struct spi_transfer *)kzalloc(size_xfer, GFP_KERNEL);
	if (!g_transfer) {
		thp_log_err("g_transfer kzalloc failed\n");
		return -ENOMEM;
	}
#endif

	touch_driver_timing_work(tdev);

	thp_bus_lock();
	ret =  touch_driver_83112_communication_check(tdev);
	if (ret) {
		thp_log_err("%s: ic communication check fail\n", __func__);
		thp_bus_unlock();
		return -EINVAL;
	}
	thp_bus_unlock();
	ret =  touch_driver_read_panel_info(tdev);
	if (ret) {
		thp_log_err("%s: read panel info  fail\n", __func__);
		return -EINVAL;
	}
	/* Get tp_color */
	if (g_huawei_cg_color != NULL)
		cypress_ts_kit_color[0] = *g_huawei_cg_color;

	return ret;
}

int touch_driver_83112_chip_detect(struct thp_device *tdev)
{
	int ret;
	struct himax_thp_private_data *himax_p = NULL;

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_83102_flag) {
		ret = touch_driver_83102_chip_detect(tdev);
		return ret;
	}

	g_himax_spi_buf.w_buf = kzalloc(HIMAX_SPI_W_BUF_MAX_LEN, GFP_KERNEL);
	if (!g_himax_spi_buf.w_buf) {
		thp_log_err("%s:w_buf request memory fail\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	g_himax_spi_buf.r_buf_a = kzalloc(COMM_TEST_RW_LENGTH, GFP_KERNEL);
	if (!g_himax_spi_buf.r_buf_a) {
		thp_log_err("%s:r_buf1 request memory fail\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	g_himax_spi_buf.r_buf_b = kzalloc(SPI_FORMAT_ARRAY_SIZE, GFP_KERNEL);
	if (!g_himax_spi_buf.r_buf_b) {
		thp_log_err("%s:r_buf2 request memory fail\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	touch_driver_timing_work(tdev);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		ret = -EINVAL;
		goto exit;
	}
	ret =  touch_driver_83112_communication_check(tdev);
	thp_bus_unlock();

	return ret;

exit:
	kfree(g_himax_spi_buf.w_buf);
	g_himax_spi_buf.w_buf = NULL;
	kfree(g_himax_spi_buf.r_buf_a);
	g_himax_spi_buf.r_buf_a = NULL;
	kfree(g_himax_spi_buf.r_buf_b);
	g_himax_spi_buf.r_buf_b = NULL;
	return ret;
}

static int touch_driver_83102_get_dsram_data(struct thp_device *tdev,
	char *info_data, unsigned int len)
{
	unsigned char tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	unsigned int read_size = len;
	uint8_t *temp_info_data = NULL;
	int ret;

	if (!info_data) {
		thp_log_err("%s:info_data is NULL\n", __func__);
		return -EINVAL;
	}
	temp_info_data = kzalloc(read_size, GFP_KERNEL);
	if (!temp_info_data) {
		thp_log_err("%s: temp_info_data malloc fail\n", __func__);
		return -ENOMEM;
	}
	thp_bus_lock();
	/* 1. turn on burst mode */
	touch_driver_interface_on(tdev);
	/* 2. get RawData from sram */
	himax_assign_data(HIMAX_RAWDATA_ADDR, tmp_addr);
	ret = touch_driver_register_read(tdev, tmp_addr, read_size,
		temp_info_data);
	if (ret) {
		thp_bus_unlock();
		goto exit;
	}
	thp_bus_unlock();

	memcpy(info_data, temp_info_data, read_size);
exit:
	kfree(temp_info_data);
	temp_info_data = NULL;

	return ret;
}

static int touch_driver_get_dsram_data(struct thp_device *tdev,
	char *info_data, unsigned int len)
{
	unsigned char tmp_addr[HIMAX_NORMAL_ADDR_LEN] = {0};
	unsigned int read_size = len;
	uint8_t *temp_info_data = NULL;
	int ret;
	struct himax_thp_private_data *himax_p = tdev->private_data;

	if (!len) {
		thp_log_err("%s:len is invalid\n", __func__);
		return -EINVAL;
	}
	if (!info_data) {
		thp_log_err("%s:info_data is NULL\n", __func__);
		return -EINVAL;
	}
	if (himax_p->himax_ic_83102_flag) {
		ret = touch_driver_83102_get_dsram_data(tdev, info_data, len);
		return ret;
	}
	temp_info_data = kzalloc(read_size, GFP_KERNEL);
	if (!temp_info_data) {
		thp_log_err("%s: temp_info_data malloc fail\n", __func__);
		return -ENOMEM;
	}
	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		kfree(temp_info_data);
		return -EINVAL;
	}

	himax_assign_data(HIMAX_RAWDATA_ADDR, tmp_addr);
	if (himax_p->hx_get_frame_optimized_flag) {
		if (touch_driver_bus_read(tdev, HIMAX_EVENT_STACK_CMD_ADDR,
			temp_info_data, read_size, HIMAX_BUS_RETRY_TIMES) < 0) {
			thp_log_err("%s: spi access fail!\n", __func__);
			ret = -ENOMEM;
		}
	} else {
		touch_driver_interface_on(tdev);
		ret = touch_driver_register_read(tdev, tmp_addr, read_size,
			temp_info_data);
	}

	thp_bus_unlock();

	memcpy(info_data, temp_info_data, read_size);
	kfree(temp_info_data);
	temp_info_data = NULL;

	return ret;
}

static int touch_driver_get_event_stack(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	unsigned int read_size = len;

	if (!len) {
		thp_log_err("%s:len is invalid\n", __func__);
		return -EINVAL;
	}
	if (!buf) {
		thp_log_err("%s:buf is null\n", __func__);
		return -EINVAL;
	}
	if (!tdev) {
		thp_log_err("%s:dev is null\n", __func__);
		return -EINVAL;
	}
	thp_bus_lock();

	if (touch_driver_bus_read(tdev, HIMAX_EVENT_STACK_CMD_ADDR,
		buf, read_size, HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: spi access fail!\n", __func__);
		thp_bus_unlock();
		return -EINVAL;
	}

	thp_bus_unlock();

	return NO_ERR;
}

int touch_driver_83102_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	struct himax_thp_private_data *himax_p = NULL;

	if (!tdev) {
		thp_log_err("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;

	if (!len) {
		thp_log_err("%s: read len illegal\n", __func__);
		return -EINVAL;
	}

	if (himax_p->hx_get_frame_optimized_flag)
		return touch_driver_get_event_stack(tdev, buf, len);
	else
		return touch_driver_get_dsram_data(tdev, buf, len);
}

static int touch_driver_83112_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	int ret;
	struct himax_thp_private_data *himax_p = NULL;

	if (!tdev) {
		thp_log_err("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	if (!buf) {
		thp_log_err("%s: buf null\n", __func__);
		return -EINVAL;
	}
	if (!len) {
		thp_log_err("%s: read len illegal\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;
	if (himax_p->himax_ic_83102_flag) {
		ret = touch_driver_83102_get_frame(tdev, buf, len);
		return ret;
	}
	return touch_driver_get_dsram_data(tdev, buf, len);
}

int touch_driver_get_project_id(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	char temp_buf[THP_PROJECTID_LEN + 1] = {'0'};
	struct thp_core_data *cd = NULL;

	if (!buf) {
		thp_log_err("%s: buf null\n", __func__);
		return -EINVAL;
	}
	if (!len) {
		thp_log_err("%s: len illegal\n", __func__);
		return -EINVAL;
	}
	memcpy(buf, g_huawei_project_id,
		HIMAX_ACTUAL_PROJECTID_LEN);
	buf[HIMAX_ACTUAL_PROJECTID_LEN] = '\0';

	strncpy(temp_buf, buf + THP_PROJECTID_PRODUCT_NAME_LEN +
		THP_PROJECTID_IC_NAME_LEN, THP_PROJECTID_VENDOR_NAME_LEN);

	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s: null ptr\n", __func__);
		return -EINVAL;
	}
	if (!strncmp(temp_buf,
		VENDOR_ID_BOE, THP_PROJECTID_VENDOR_NAME_LEN)) {
		cd->vendor_name = VENDOR_NAME_BOE;
	} else if (!strncmp(temp_buf,
		VENDOR_ID_BOE_1, THP_PROJECTID_VENDOR_NAME_LEN)) {
		cd->vendor_name = VENDOR_NAME_BOE;
	} else if (!strncmp(temp_buf,
		VENDOR_ID_INX, THP_PROJECTID_VENDOR_NAME_LEN)) {
		cd->vendor_name = VENDOR_NAME_INX;
	} else {
		thp_log_err("%s: not support this vendor_id\n", __func__);
		return -EINVAL;
	}

	thp_log_info("projectid:%s\n", buf);
	return NO_ERR;
}

int touch_driver_83112_resume(struct thp_device *tdev)
{
	uint8_t tmp_data[HIMAX_NORMAL_DATA_LEN];
	int rc;
	struct himax_thp_private_data *himax_p = NULL;
	struct thp_core_data *cd = NULL;
	thp_log_debug("%s: called_\n", __func__);

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	mdelay(tdev->timing_config.resume_reset_after_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return 0;
	}
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_high);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
#endif
	himax_p = tdev->private_data;
	if (himax_p->himax_gesture_need_lcd_rst)
		tdev->thp_core->lcd_gesture_mode_support = RESUME_OUT_GESTURE;

	tmp_data[0] = HIMAX_SSOFF_CMD_FIRST;

	rc = thp_bus_lock();
	if (rc < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	if (touch_driver_bus_write(tdev, HIMAX_SSOFF_ADDR_FIRST, tmp_data, 1,
		HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c first access fail!\n", __func__);
			goto ERROR;
	}

	tmp_data[0] = HIMAX_SSOFF_CMD_SECOND;
	if (touch_driver_bus_write(tdev, HIMAX_SSOFF_ADDR_SECOND, tmp_data, 1,
		HIMAX_BUS_RETRY_TIMES) < 0) {
		thp_log_err("%s: i2c second access fail!\n", __func__);
			goto ERROR;
	}
	thp_bus_unlock();

	return 0;

ERROR:
	thp_bus_unlock();
	return -EIO;
}

int touch_driver_83112_suspend(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	struct himax_thp_private_data *himax_p = NULL;

	thp_log_info("%s: called\n", __func__);

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	himax_p = tdev->private_data;
	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s: null ptr\n", __func__);
		return -EINVAL;
	}
	if ((cd->easy_wakeup_info.sleep_mode != TS_GESTURE_MODE) ||
		(!cd->support_gesture_mode)) {
#ifndef CONFIG_HUAWEI_THP_MTK
		gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
		gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
#else
		if (cd->support_pinctrl == 0) {
			thp_log_info("%s: not support pinctrl\n", __func__);
			return 0;
		}
		thp_log_info("%s: power off mode\n", __func__);
		pinctrl_select_state(cd->pctrl, cd->pins_idle);
#endif
		thp_do_time_delay(
			tdev->timing_config.suspend_reset_after_delay_ms);
		ts_kit_gesture_func = TS_POWER_OFF_MODE;
	} else {
		if (himax_p->himax_gesture_need_lcd_rst)
			cd->lcd_gesture_mode_support = SUSPEND_IN_GESTURE;
		ts_kit_gesture_func = TS_GESTURE_MODE;
		thp_log_info("%s: suspend in gesture mode\n", __func__);
	}
	return 0;
}

void touch_driver_83112_exit(struct thp_device *tdev)
{
	struct himax_thp_private_data *himax_p = NULL;

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return;
	}

	himax_p = tdev->private_data;
	thp_log_info("%s: called\n", __func__);

	if (!himax_p->himax_ic_83102_flag) {
		kfree(g_himax_spi_buf.w_buf);
		g_himax_spi_buf.w_buf = NULL;
		kfree(g_himax_spi_buf.r_buf_a);
		g_himax_spi_buf.r_buf_a = NULL;
		kfree(g_himax_spi_buf.r_buf_b);
		g_himax_spi_buf.r_buf_b = NULL;
		goto exit;
	}

exit:
	kfree(tdev->tx_buff);
	tdev->tx_buff = NULL;
	kfree(tdev->rx_buff);
	tdev->rx_buff = NULL;
	kfree(tdev);
	tdev = NULL;
}

static struct thp_device_ops hx83112_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_83112_chip_detect,
	.get_frame = touch_driver_83112_get_frame,
	.get_project_id = touch_driver_get_project_id,
	.chip_gesture_report = touch_driver_gesture_report,
	.resume = touch_driver_83112_resume,
	.suspend = touch_driver_83112_suspend,
	.exit = touch_driver_83112_exit,
};

static int __init touch_driver_module_init(void)
{
	int rc = 0;
	struct thp_device *dev = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s in\n", __func__);

	dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
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

	dev->ic_name = HX83112_IC_NAME;
	dev->dev_node_name = THP_HX83112_DEV_NODE_NAME;
	dev->ops = &hx83112_dev_ops;
	dev->private_data = (void *)&thp_private_data;
	if (cd && cd->fast_booting_solution) {
		thp_log_err("%s: don't support this solution\n", __func__);
		goto err;
	}
	rc = thp_register_dev(dev);
	if (rc) {
		thp_log_err("%s: register fail\n", __func__);
		goto err;
	}

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
#ifdef CONFIG_HUAWEI_THP_QCOM
late_initcall(touch_driver_module_init);
#else
module_init(touch_driver_module_init);
#endif
module_exit(touch_driver_module_exit);
