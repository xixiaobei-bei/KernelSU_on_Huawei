/* Himax Android Driver Sample Code for Himax chipset
 *
 * Copyright (C) 2021 Himax Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "himax_platform.h"
#include "himax_ic.h"

int hx_zf_irq_enable_count = 1;
extern struct himax_ts_data *g_himax_zf_ts_data;

extern int himax_tskit_write(uint8_t command, uint8_t *data, uint32_t length);
extern int himax_tskit_read(uint8_t command, uint8_t *data, uint32_t length);

static int hx_spi_read(uint8_t *command, uint8_t *data, uint16_t length, uint8_t toRetry)
{
	int retval = NO_ERR;
	struct spi_device *spi = g_himax_zf_ts_data->spi;
	struct spi_transfer xfer[2] = { {0}, {0} };
	int xfer_size = 0;

	if ((command == NULL) || (data == NULL)) {
		TS_LOG_ERR("%s:command = 0x%p,data = 0x%p\n",
			__func__, command, data);
		return -ENOMEM;
	}

	xfer[0].tx_buf = command;
	xfer[0].rx_buf = NULL;
	xfer[0].len = SPI_FORMAT_ARRAY_SIZE;
	xfer_size = ARRAY_SIZE(xfer);
	xfer[0].cs_change = 0;
	xfer[0].bits_per_word = 8;

	xfer[1].tx_buf = NULL;
	xfer[1].rx_buf = data;
	xfer[1].len = length;

	retval = spi_setup(spi);
	if (retval < 0) {
		TS_LOG_ERR("%s spi setup failed, retval = %d\n",
			__func__, retval);
		return retval;
	}
	return spi_sync_transfer(g_himax_zf_ts_data->spi, xfer, xfer_size);
}

int himax_zf_bus_read(uint8_t command, uint8_t *data, uint16_t length, uint16_t limit_len, uint8_t toRetry)
{
	uint8_t spi_format_buf[SPI_FORMAT_ARRAY_SIZE];

	if (length > limit_len)
		return -EIO;

	/*0xF3 is head of command*/
	spi_format_buf[0] = 0xF3;
	spi_format_buf[1] = command;
	/*0x00 is tail of command*/
	spi_format_buf[2] = 0x00;

	return hx_spi_read(&spi_format_buf[0], data, length, toRetry);
}

static int hx_spi_write(uint8_t *command, uint16_t length, uint8_t toRetry)
{
	int retval = NO_ERR;
	struct spi_device *spi = g_himax_zf_ts_data->spi;
	struct spi_transfer xfer[] = {
		{
			.tx_buf = command,
			.len = length,
		},
	};

	retval = spi_setup(spi);
	if (retval < 0) {
		TS_LOG_ERR("%s spi setup failed, retval = %d\n",
			__func__, retval);
		return retval;
	}
	retval = spi_sync_transfer(g_himax_zf_ts_data->spi, xfer, ARRAY_SIZE(xfer));
	if (retval != 0) {
		TS_LOG_ERR("%s spi_sync_transfer failed, retval = %d\n",
			__func__, retval);
	}
	return retval;
}

int hx_zf_bus_write(uint8_t command, uint8_t *data, uint32_t length, uint32_t limit_len, uint8_t toRetry)
{
	uint8_t spi_format_buf[length + 2];
	int i;

	if (length > limit_len) {
		TS_LOG_ERR("%s:length = %d, limit_len = %d\n",
			__func__, length, limit_len);
		return -EIO;
	}

	/*0xF2 is head of command*/
	spi_format_buf[0] = 0xF2;
	spi_format_buf[1] = command;

	for (i = 0; i < length; i++)
		spi_format_buf[i + 2] = data[i];

	return hx_spi_write(spi_format_buf, length + 2, toRetry);
}

void hx_zf_burst_enable(uint8_t auto_add_4_byte)
{
	uint8_t tmp_data[4] = {0};

	tmp_data[0] = DATA_EN_BURST_MODE;
	if (hx_zf_bus_write(ADDR_EN_BURST_MODE, tmp_data, ONE_BYTE_CMD,
		sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}

	tmp_data[0] = (DATA_AHB | auto_add_4_byte);
	if (hx_zf_bus_write(ADDR_AHB, tmp_data, ONE_BYTE_CMD,
		sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}
}

void hx_zf_write_burst(uint8_t *reg_byte, uint8_t *write_data)
{
	int i = 0;
	int j = 0;
	uint8_t data_byte[8] = {0};

	for (i = 0; i < FOUR_BYTE_CMD; i++)
		data_byte[i] = reg_byte[i];
	for (j = 4; j < 2 * FOUR_BYTE_CMD; j++)
		data_byte[j] = write_data[j-4];

	if (hx_zf_bus_write(ADDR_FLASH_BURNED, data_byte, 2 * FOUR_BYTE_CMD,
		sizeof(data_byte), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}
}

#define FW_UPDATE_LEN (9 * 1024)
void hx_zf_flash_write_burst_lenth(uint8_t *reg_byte, uint8_t *write_data, int length)
{
	int i = 0;
	int j = 0;
	static char *fw_update_buf = NULL;

	if (!fw_update_buf) {
		fw_update_buf = kzalloc(FW_UPDATE_LEN, GFP_KERNEL);
		if (!fw_update_buf) {
			TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
			return;
		}
	}
	for (i = 0; i < FOUR_BYTE_CMD; i++)
		fw_update_buf[i] = reg_byte[i];

	for (j = 4; j < length + 4; j++)
		fw_update_buf[j] = write_data[j - 4];

	if (hx_zf_bus_write(ADDR_FLASH_BURNED, fw_update_buf, length + 4, FW_UPDATE_LEN, DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}
}

void hx_zf_register_read(uint8_t *read_addr, int read_length, uint8_t *read_data)
{
	unsigned int i = 0;
	unsigned int address = 0;
	uint8_t tmp_data[4] = {0};

	if (read_length > MAX_READ_LENTH) {
		TS_LOG_ERR("%s: read len over 256!\n", __func__);
		return;
	}
	if (read_length > FOUR_BYTE_CMD)
		hx_zf_burst_enable(1);
	else
		hx_zf_burst_enable(0);

	address = (read_addr[3] << 24) + (read_addr[2] << 16) +
		(read_addr[1] << 8) + read_addr[0];
	i = address;
	tmp_data[0] = (uint8_t)i;
	tmp_data[1] = (uint8_t)(i >> 8);
	tmp_data[2] = (uint8_t)(i >> 16);
	tmp_data[3] = (uint8_t)(i >> 24);
	if (hx_zf_bus_write(ADDR_FLASH_BURNED, tmp_data, FOUR_BYTE_CMD,
		sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}
	tmp_data[0] = DATA_READ_ACCESS;
	if (hx_zf_bus_write(ADDR_READ_ACCESS, tmp_data, ONE_BYTE_CMD,
		sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}

	if (himax_zf_bus_read(DUMMY_REGISTER, read_data, read_length,
		MAX_READ_LENTH, DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}
	if (read_length > FOUR_BYTE_CMD)
		hx_zf_burst_enable(0);
}

void himax_zf_register_write(uint8_t *write_addr, int write_length, uint8_t *write_data)
{
	hx_zf_flash_write_burst_lenth(write_addr, write_data, write_length);
}

int himax_zf_write_read_reg(uint8_t *tmp_addr, uint8_t *tmp_data, uint8_t hb, uint8_t lb)
{
	int cnt = 0;

	do {
		hx_zf_write_burst(tmp_addr, tmp_data);
		msleep(HX_SLEEP_10MS);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);

	} while ((tmp_data[1] != hb && tmp_data[0] != lb) && cnt++ < 100);

	if (cnt == 99)
		return -1;

	TS_LOG_DEBUG("Now register 0x%08X : high byte=0x%02X,low byte=0x%02X\n",
		tmp_addr[3], tmp_data[1], tmp_data[0]);
	return NO_ERR;
}

void himax_zf_int_enable(int irqnum, int enable)
{
	TS_LOG_INFO("S_irqnum=%d, irq_enable_count = %d, enable =%d\n",
		irqnum, hx_zf_irq_enable_count, enable);

	if (enable == 1 && hx_zf_irq_enable_count == 0) {
		enable_irq(irqnum);
		hx_zf_irq_enable_count = 1;
	} else if (enable == 0 && hx_zf_irq_enable_count == 1) {
		disable_irq_nosync(irqnum);
		hx_zf_irq_enable_count = 0;
	}
	TS_LOG_INFO("E_irqnum=%d, irq_enable_count = %d, enable =%d\n",
		irqnum, hx_zf_irq_enable_count, enable);
}

void himax_zf_rst_gpio_set(int pinnum, uint8_t value)
{
	gpio_direction_output(pinnum, value);
}
