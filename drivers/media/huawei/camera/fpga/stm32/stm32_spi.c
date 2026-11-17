/*
 * stm32_spi.c
 *
 * camera driver source file
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

#include <securec.h>
#include "stm32_spi.h"
#include "cam_log.h"
#include "pmic/hw_pmic.h"
#include "huawei_platform/sensor/hw_comm_pmic.h"
#include "platform/sensor_commom.h"
#include <asm/arch_timer.h>
#include <linux/time.h>
#include "hwcam_cfgdev.h"

/* bit7:1 enable low power, 0 disable low power */
#define MCU_STATE_LP            0x80

#define MCU_FW_VER_H            0x00
#define MCU_FW_VER_L            0x01
#define MCU_IRQ_STATE_REG       0x02
#define MCU_STATE_CTRL_REG      0x04
#define MCU_LOW_POWER_NUM_REG   0x06
#define MCU_M1_PLUS_WIDE_REG    0x0E
#define MCU_M2_PLUS_WIDE_REG    0x0F

#define REG_E_LIMIT_VAL         5
#define REG_F_LIMIT_VAL         11

#define LOAD_FW_ERR       0
#define SPI_RW_ERR        1
#define SELF_TEST_ERR     2
#define AGE_PROJ_ERR      3
#define AGE_ILLU_ERR      4
#define OTHER_ERR         5

#define ACK      0x79
#define NACK     0x1F
#define SOF      0x5A
#define SOF_ACK  0xA5

#define READ_MEM_CMD_H  0x11
#define READ_MEM_CMD_L  0xEE

#define WRITE_MEM_CMD_H  0x31
#define WRITE_MEM_CMD_L  0xCE

#define ERASE_MEM_CMD_H  0x44
#define ERASE_MEM_CMD_L  0xBB

#define GOTO_NORMAL_CMD_H  0x21
#define GOTO_NORMAL_CMD_L  0xDE

#define READ_MAX_BYTES 256

#define MEMORY_START_ADDR    0x08000000
#define MEMORY_VERSION_ADDR  0x08000400
#define FW_BIN_VERSION_ADDR  0x0400
#define SECTOR_SIZE          (16 * 1024)

#define NORMAL_WAIT_TIMEOUT  300
#define WRITE_WAIT_TIMEOUT   6000
#define ERASE_WAIT_TIMEOUT   300

#define LOAD_FW_MAX_TIME  100000 /* 3s */
/*
 * only support ov9282 ov9286,
 * if add, please modify here:
 */
#define LOADFW_MAX_TIME   3
#define POWERUP_MAX_TIME  3
#define ENABLE_MAX_TIME   1

#define STM32_DEVICE_NAME        "stm32_spi"
#define STM32_MISC_DEVICE_NAME   "stm32"

#define ONE_SECTOR 250
#define START_ADDR 0x08000000
#define FLASH_FIRMWARE_BIN "odm/etc/STM_AppPrj_FW.bin"

#ifdef MCU_READ_REG_DEBUG
static u8 g_reg;
static u8 g_reg_value;
#endif

static struct stm32_spi_priv_data *g_spi_drv_data;

/*
 * 0x02-reg
 * bit6: M2-1 abnormal (pull up Flash_mask2)
 *      OPA_OUT_B: pulse time more than 11 milliseconds every time
 * bit3: M1-1 abnormal (pull up Flash_mask1)
 *      OPA_OUT_A: pulse time more than 5 milliseconds every time
 * bit5: MCU abnormal
 * bit4: M1-2 abnormal (if exist 9 consecutive M1-1 abnormal pulses,
 * set 1 and pull up Flash_mask1 10s)
 */
static struct err_map g_mcu_irq_err_table[] = {
	/* 927011016 927011017 927011018 927011019 */
	{ 0x40, "M2-1 abnormal", 927011016 }, /* bit6 */
	{ 0x20, "MCU abnormal",  927011017 }, /* bit5 */
	{ 0x08, "M1-2 abnormal", 927011018 }, /* bit4 */
	{ 0x04, "M1-1 abnormal", 927011019 }, /* bit3 */
};

static struct err_map g_mcu_common_err_table[] = {
	/* 927011020 */
	/* undefined abnormal */
	[OTHER_ERR] = { 0x01, "MCU other failed", 927011020 }, /* bit1 */
};

static void mcu_dsm_client_notify(struct stm32_spi_priv_data *drv_data,
	char *err_name, u32 err_code)
{
	if ((!drv_data) || (!err_name)) {
		cam_err("%s drvdata or err_name is NULL", __func__);
		return;
	}

	if (!dsm_client_ocuppy(drv_data->client_stm32)) {
		dsm_client_record(drv_data->client_stm32,
			"[stm32] %s\n", err_name);
		dsm_client_record(drv_data->client_stm32,
			"[stm32] 0x02 = 0x%02x\n", drv_data->last_err_code);
		dsm_client_notify(drv_data->client_stm32, err_code);
		cam_info("[I/DSM] fpga dsm_client_notify success");
	} else {
		cam_info("[I/DSM] fpga dsm_client_notify fail");
	}
}

/*
 * mcu spi read function for bootloader mode
 * send : data to be written
 * receive : data to be read
 */
static int boot_spi_read_reg(struct stm32_spi_priv_data *devdata,
	 u8 *send, u8 *receive, unsigned int len)
{
	int status;
	struct spi_transfer xfer;
	struct spi_message msg;
	unsigned int loop;
	int ret;

	if ((!devdata) || (!send) || (!receive)) {
		cam_err("%s no driver data, or val is NULL", __func__);
		return -ENODEV;
	}

	ret = memcpy_s(&(devdata->tx_buf[0]), len, send, len);
	if (ret != 0) {
		cam_err("%s memcpy_s fail", __func__);
		return -EINVAL;
	}

	for (loop = 0; loop < len; loop++)
		cam_info("devdata->tx_buf[%d] = 0x%02X",
			loop, devdata->tx_buf[loop]);

	ret = memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
	if (ret != 0) {
		cam_err("%s memset_s fail", __func__);
		return -EINVAL;
	}
	xfer.tx_buf = devdata->tx_buf;
	xfer.rx_buf = devdata->rx_buf;
	xfer.len = len;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	status = spi_sync(devdata->spi, &msg);
	if (status) {
		cam_err("%s - sync error: status=%d", __func__, status);
		return status;
	}

	for (loop = 0; loop < len; loop++)
		cam_info("devdata->rx_buf[%d] = 0x%02X",
			loop, devdata->rx_buf[loop]);
	ret = memcpy_s(receive, len, &(devdata->rx_buf[0]), len);
	if (ret != 0) {
		cam_err("%s memcpy_s fail", __func__);
		return -EINVAL;
	}
	return status;
}

/*
 * mcu spi write function for bootloader mode
 * wdata : data to be written
 */
static int boot_spi_write_reg(struct stm32_spi_priv_data *devdata,
	u8 *wdata, unsigned int len)
{
	int status;
	int ret;
	struct spi_transfer xfer;
	struct spi_message msg;

	if (!devdata) {
		cam_err("%s devdata is NULL.", __func__);
		return -EINVAL;
	}

	ret = memcpy_s(&(devdata->tx_buf[0]), len, wdata, len);
	if (ret != 0) {
		cam_err("%s memcpy_s fail", __func__);
		return -EINVAL;
	}

	ret = memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
	if (ret != 0) {
		cam_err("%s memset_s fail", __func__);
		return -EINVAL;
	}

	xfer.tx_buf = devdata->tx_buf;
	xfer.rx_buf = NULL;
	xfer.len = len;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	status = spi_sync(devdata->spi, &msg);
	if (status)
		cam_err("%s - sync error: status=%d", __func__, status);

	return status;
}

/*
 * mcu spi read function for normal mode
 * MO: (reg<<2)+0x80 0x00
 * MI: 0x5A val
 */
static int stm32_spi_read_reg(struct stm32_spi_priv_data *devdata,
	u8 reg, u8 *data)
{
	int status;
	u8 send;
	u8 recieve = 0;

	if (!devdata || !data) {
		cam_err("%s no driver data, or val is NULL", __func__);
		return -ENODEV;
	}
	/* normal mode, (reg<<2) + 0x80 firt bit equal to 1 stand for read */
	send = (u8)((reg << 2) + 0x80);
	status = boot_spi_read_reg(devdata, &send, &recieve, 1); /* one byte */
	if (status) {
		cam_err("%s:%d, spi write error", __func__, __LINE__);
		return status;
	}
	send = 0;
	status = boot_spi_read_reg(devdata, &send, &recieve, 1); /* one byte */
	if (status) {
		cam_err("%s:%d, spi write error", __func__, __LINE__);
		return status;
	}
	*data = recieve;
	return status;
}

static int stm32_spi_write_reg(struct stm32_spi_priv_data *devdata,
	u8 reg, u8 val)
{
	int status;
	int ret;
	struct spi_transfer xfer;
	struct spi_message msg;
	const unsigned int buf_size = 2;

	if (!devdata) {
		cam_err("%s devdata is NULL", __func__);
		return -EINVAL;
	}
	/* 2 bytes */
	ret = memset_s(&(devdata->tx_buf[0]), buf_size, 0, buf_size);
	if (ret != 0) {
		cam_err("%s memset_s fail", __func__);
		return -EINVAL;
	}
	/* normal mode, the value is (reg<<2) */
	devdata->tx_buf[0] = (reg << 2); /* the reg starts from third bit */
	devdata->tx_buf[1] = val;
	ret = memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
	if (ret != 0) {
		cam_err("%s memset_s fail", __func__);
		return -EINVAL;
	}
	xfer.tx_buf = devdata->tx_buf;
	xfer.rx_buf = NULL;
	xfer.len = 2; /* 2 bytes */

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	status = spi_sync(devdata->spi, &msg);
	if (status)
		cam_err("%s - sync error: status=%d", __func__, status);

	return status;
}

/* for two or more datas, make xor process each other */
static u8 get_xor_checksum(u8 *data, u8 len)
{
	unsigned int loop;
	u8 sum;

	if (!data) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return 0;
	}
	sum = data[0];
	for (loop = 1; loop < len; loop++)
		sum ^= data[loop];
	return sum;
}

/* for two or more datas, make xor process with 0xFF */
static unsigned int get_nega_checksum(u8 data)
{
	u8 sum = 0xff;

	sum ^= data;
	return sum;
}

/*
 * MO: 0x00 0x00 0x79
 * MI: XX   0x79 XX
 */
static int get_ack_wait_us(struct stm32_spi_priv_data *devdata, int times)
{
	u8 send = 0;
	u8 recieve = 0;
	int loop = 0;

	if (!devdata || times <= 0) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	boot_spi_write_reg(devdata, &send, 1); /* one byte */
	while (loop < times) {
		boot_spi_read_reg(devdata, &send, &recieve, 1); /* one byte */
		if (recieve == ACK || recieve == NACK) {
			send = ACK;
			boot_spi_write_reg(devdata, &send, 1); /* one byte */
			return recieve == ACK ? 0 : -1;
		}
		loop++;
		udelay(10); /* 10ns */
		cam_info("%s:%d loop = %d", __func__, __LINE__, loop);
	}
	return -1;
}

/*
 * MO: 0x00 0x00 0x79
 * MI: XX   0x79 XX
 */
static int get_ack_wait_ms(struct stm32_spi_priv_data *devdata, int times)
{
	u8 send = 0;
	u8 recieve = 0;
	int loop = 0;

	if (!devdata || times <= 0) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	boot_spi_write_reg(devdata, &send, 1); /* one byte */
	while (loop < times) {
		boot_spi_read_reg(devdata, &send, &recieve, 1); /* one byte */
		if (recieve == ACK || recieve == NACK) {
			send = ACK;
			boot_spi_write_reg(devdata, &send, 1); /* one byte */
			return recieve == ACK ? 0 : -1;
		}
		loop++;
		mdelay(10); /* 10ms */
		cam_info("%s:%d loop = %d", __func__, __LINE__, loop);
	}
	return -1;
}

/*
 * bootloader synchronization frame
 * MO: 0x5A (ACK)
 * MI: 0xA5
 */
static int boot_sync_frame(struct stm32_spi_priv_data *devdata)
{
	u8 send;
	u8 recieve = 0;

	cam_info("enter %s", __func__);
	if (!devdata) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	send = SOF;
	boot_spi_read_reg(devdata, &send, &recieve, 1); /* one byte */
	if (recieve == SOF_ACK) {
		if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) == 0) {
			cam_info("%s:%d bootloader sync success",
				__func__, __LINE__);
			return 0;
		}
	}

	cam_info("%s:%d bootloader sync fail", __func__, __LINE__);
	return -1;
}

/*
 * read memory command
 * 0x5A 0x11 0xEE (ACK) start_addr(4byter)+checksum (ACK)
 * number_of_bytes(1byte)+checksum (ACK)
 */
static int boot_read_memory_cmd(struct stm32_spi_priv_data *devdata,
	u32 addr, u8 len, u8 *data)
{
	u8 send[5] = {0}; /* once read max 5 bytes */
	u8 recieve = 0;
	u8 loop;

	/* once read max 256 bytes */
	if (!devdata || !data) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	/* start frame */
	send[0] = SOF;
	boot_spi_write_reg(devdata, send, 1); /* one byte */

	/* command frame */
	send[0] = READ_MEM_CMD_H;
	send[1] = READ_MEM_CMD_L;
	boot_spi_write_reg(devdata, send, 2); /* two bytes */

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, read memory fail", __func__, __LINE__);
		return -1;
	}

	/* start address with checksum */
	send[0] = (addr >> 24) & 0xff;
	send[1] = (addr >> 16) & 0xff;
	send[2] = (addr >> 8) & 0xff;
	send[3] = addr & 0xff;
	/* get 4 bytes checksum */
	send[4] = get_xor_checksum(send, 4);
	/* read 5 bytes */
	boot_spi_write_reg(devdata, send, 5);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, read memory fail", __func__, __LINE__);
		return -1;
	}

	/* number of bytes to be read with checksum */
	send[0] = len - 1; /* N(the number of bytes) - 1 */
	send[1] = get_nega_checksum(send[0]);
	boot_spi_write_reg(devdata, send, 2); /* two bytes */

	if (get_ack_wait_us(devdata, WRITE_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, read memory fail", __func__, __LINE__);
		return -1;
	}

	/* recieve data frame */
	send[0] = 0;
	/* recieve ACK first */
	boot_spi_read_reg(devdata, send, &recieve, 1);
	/* recieve data one by one */
	for (loop = 0; loop < len; loop++)
		boot_spi_read_reg(devdata, send, data + loop, 1);

	return 0;
}

/*
 * write memory command
 * 0x5A 0x31 0xCE (ACK) start_addr(4bytes)+checksum (ACK)
 * data_numbers+data+checksum (ACK)
 * len<=256
 */
static int boot_write_memory_cmd(struct stm32_spi_priv_data *devdata,
	u32 addr, u8 *data, unsigned int len)
{
	u8 send[READ_MAX_BYTES + 2] = {0}; /* once read max 256 bytes */
	u8 temp;
	int ret;

	cam_info("enter %s", __func__);
	if (!devdata || !data || len > READ_MAX_BYTES) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	/* start frame */
	send[0] = SOF;
	boot_spi_write_reg(devdata, send, 1);

	/* command frame */
	send[0] = WRITE_MEM_CMD_H;
	send[1] = WRITE_MEM_CMD_L;
	/* 2 bytes */
	boot_spi_write_reg(devdata, send, 2);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, write memory fail", __func__, __LINE__);
		return -1;
	}

	/* start address with checksum */
	send[0] = (addr >> 24) & 0xff;
	send[1] = (addr >> 16) & 0xff;
	send[2] = (addr >> 8) & 0xff;
	send[3] = addr & 0xff;
	send[4] = get_xor_checksum(send, 4);
	boot_spi_write_reg(devdata, send, 5);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, write memory fail", __func__, __LINE__);
		return -1;
	}

	/* number of bytes to be written, data to be written, with checksum */
	send[0] = len - 1; /* len from 0 start */
	ret = memcpy_s(&(send[1]), len, data, len);
	if (ret != 0) {
		cam_err("%s memcpy_s fail", __func__);
		return -EINVAL;
	}

	temp = len + 1;
	send[temp] = get_xor_checksum(send, temp);
	boot_spi_write_reg(devdata, send, len + 2);

	if (get_ack_wait_ms(devdata, WRITE_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, write memory fail", __func__, __LINE__);
		return -1;
	}

	return 0;
}

/*
 * erase memory command
 * 0x5A  0x44 0xBB (ACK) number_of_pages(MSB first, 2 bytes)+checksum
 * (ACK) page_number(MSB first, each on two bytes)+checksum (ACK)
 */
static int boot_erase_memory_cmd(struct stm32_spi_priv_data *devdata,
	u8 pages, u8 number)
{
	u8 send[3] = {0}; /* once write max 3 bytes */

	cam_info("enter %s", __func__);
	if (!devdata) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	/* start frame */
	send[0] = SOF;
	boot_spi_write_reg(devdata, send, 1);

	/* command frame */
	send[0] = ERASE_MEM_CMD_H;
	send[1] = ERASE_MEM_CMD_L;
	boot_spi_write_reg(devdata, send, 2);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, erase memory fail", __func__, __LINE__);
		return -1;
	}

	/* number of pages to be erased with checksum */
	send[0] = 0;
	send[1] = (pages - 1) & 0xff;
	send[2] = get_xor_checksum(send, 2);
	boot_spi_write_reg(devdata, send, 3);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, erase memory fail", __func__, __LINE__);
		return -1;
	}

	/* page numbers(each on two bytes) with checksum */
	send[0] = 0;
	send[1] = number & 0xff;
	send[2] = get_xor_checksum(send, 2);
	boot_spi_write_reg(devdata, send, 3);

	if (get_ack_wait_ms(devdata, ERASE_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, erase memory fail", __func__, __LINE__);
		return -1;
	}

	return 0;
}

/* enter normal mode from boot mode */
static int boot_go_memory_cmd(struct stm32_spi_priv_data *devdata, u32 addr)
{
	u8 send[5] = {0}; /* once write max 5 bytes */

	cam_info("enter %s", __func__);
	if (!devdata) {
		cam_info("%s:%d invalid param", __func__, __LINE__);
		return -1;
	}

	/* start frame */
	send[0] = SOF;
	boot_spi_write_reg(devdata, send, 1);

	/* command frame */
	send[0] = GOTO_NORMAL_CMD_H;
	send[1] = GOTO_NORMAL_CMD_L;
	boot_spi_write_reg(devdata, send, 2);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, erase memory fail", __func__, __LINE__);
		return -1;
	}

	/* start address with checksum */
	send[0] = (addr >> 24) & 0xff;
	send[1] = (addr >> 16) & 0xff;
	send[2] = (addr >> 8) & 0xff;
	send[3] = addr & 0xff;
	send[4] = get_xor_checksum(send, 4);
	boot_spi_write_reg(devdata, send, 5);

	if (get_ack_wait_us(devdata, NORMAL_WAIT_TIMEOUT) != 0) {
		cam_info("%s:%d, erase memory fail", __func__, __LINE__);
		return -1;
	}
	return 0;
}

/* load firmware bin to mcu flash */
static int mcu_write_memory(struct stm32_spi_priv_data *devdata,
	u8 *buff, int len)
{
	int read_size = len;
	int write_size = 0;
	int one_size;
	int rc;

	cam_info("enter %s", __func__);
	if (!buff || !devdata) {
		cam_info("%s:%d buff is NULL", __func__, __LINE__);
		return -1;
	}

	cam_info("%s:%d read_size = %d", __func__, __LINE__, read_size);
	while (write_size < read_size) {
		if ((read_size - write_size) >= ONE_SECTOR)
			one_size = ONE_SECTOR;
		else
			one_size = read_size - write_size;

		rc = boot_write_memory_cmd(devdata, START_ADDR + write_size,
			buff + write_size, one_size);
		if (rc != 0) {
			cam_err("%s:%d write fail", __func__, __LINE__);
			return -1;
		}
		write_size += one_size;
	}
	return 0;
}

/* read firmware file from phone to load to mcu flash */
static int mcu_load_fw_code(struct stm32_spi_priv_data *devdata,
	const char *file)
{
	mm_segment_t oldfs = get_fs();
	struct file *fp = NULL;
	int file_size;
	int one_size;
	int left;
	loff_t pos;
	u8 *buff = NULL;
	int buff_sz = 0;
	int ret;

	cam_info("enter %s", __func__);
	if ((!devdata) || (!file)) {
		cam_err("%s no driver data, or file is NULL", __func__);
		return -ENODEV;
	}

	set_fs(KERNEL_DS);
	fp = filp_open(file, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(fp)) {
		cam_info("%s open file error", __func__);
		set_fs(oldfs);
		return -EINVAL;
	}
	file_size = (int)(vfs_llseek(fp, 0L, SEEK_END));
	cam_info("%s file_size = %d", __func__, file_size);

	buff = (u8 *)vmalloc(sizeof(u8) * file_size);
	if (!buff) {
		cam_err("%s malloc failed", __func__);
		filp_close(fp, 0);
		set_fs(oldfs);
		return -1;
	}
	ret = memset_s(buff, file_size, 0, file_size);
	if (ret != 0) {
		cam_err("%s memset_s fail", __func__);
		filp_close(fp, 0);
		set_fs(oldfs);
		vfree(buff);
		return -EINVAL;
	}

	vfs_llseek(fp, 0L, SEEK_SET);
	pos = fp->f_pos;
	for (left = file_size; left > 0; left -= SPI_BLOCK_BUF_SIZE) {
		if (left > SPI_BLOCK_BUF_SIZE)
			one_size = SPI_BLOCK_BUF_SIZE;
		else
			one_size = left;

		ret = vfs_read(fp, (char *)(buff + buff_sz), one_size, &pos);
		if (ret < 0) {
			cam_err("%s vfs read error %d", __func__, ret);
			break;
		}
		buff_sz += one_size;
	}
	cam_info("%s buff_sz = %d", __func__, buff_sz);

	filp_close(fp, 0);
	set_fs(oldfs);

	ret = mcu_write_memory(devdata, buff, file_size);
	if (ret != 0)
		cam_err("%s write memory fail", __func__);

	vfree(buff);
	return ret;
}

/*
 * get version from firmware file
 */
static int get_fw_version_from_bin(const char *file, u32 offset,
	u32 *version, u32 *size)
{
	mm_segment_t oldfs = get_fs();
	struct file *fp = NULL;
	unsigned int file_size;
	loff_t pos;
	int ret = 0;
	u8 recieve[2] = {0}; /* version size: two bytes */

	if (!file) {
		cam_err("%s file is NULL", __func__);
		return -ENODEV;
	}

	set_fs(KERNEL_DS);
	fp = filp_open(file, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(fp)) {
		cam_info("%s open file error", __func__);
		set_fs(oldfs);
		return -EINVAL;
	}
	file_size = (int)(vfs_llseek(fp, 0L, SEEK_END));
	cam_info("offset = 0x%X vs file_size = 0x%X", offset, file_size);

	if (offset >= file_size) {
		cam_info("offset = 0x%X vs file_size = 0x%X, fail",
			offset, file_size);
		ret = -1;
		goto BIN_VER_END;
	}

	vfs_llseek(fp, offset, SEEK_SET);
	pos = fp->f_pos;
	ret = vfs_read(fp, (char *)recieve, 2, &pos);
	if (ret < 0) {
		cam_err("%s vfs read error %d", __func__, ret);
		goto BIN_VER_END;
	}

	*size = (u32)file_size;
	*version = recieve[0] + (recieve[1] << 8);
	cam_info("%s version = 0x%04x", __func__, *version);

BIN_VER_END:
	filp_close(fp, 0);
	set_fs(oldfs);
	return ret;
}

/*
 * get version from mcu memory
 */
static int get_fw_version_from_flash(struct stm32_spi_priv_data *devdata,
	u32 offset, u32 *version)
{
	int rc;
	u8 recieve[2] = {0}; /* version size: two bytes */

	rc = boot_read_memory_cmd(devdata, offset, 2, recieve);
	if (rc != 0) {
		cam_info("%s:%d boot_read_memory_cmd fail",
			__func__, __LINE__);
		return -1;
	}

	*version = recieve[0] + (recieve[1] << 8);
	cam_info("%s version = 0x%04X", __func__, *version);
	return 0;
}

/* mcu enable lowpower */
int stm32_spi_enable_lp(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	struct stm32_spi_plat_data *plat_data = NULL;
	int ret;
	u8 val = 0;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}

	cam_info("%s enter", __func__);

	if (drv_data->load_fw_state == LOAD_FW_FAIL) {
		cam_info("%s load fpga firmware fail, will exit", __func__);
		return -1;
	}

	if (drv_data->load_fw_state != LOAD_FW_DONE) {
		cam_info("%s load_fw_state = %d, will exit",
			__func__, drv_data->load_fw_state);
		return 0;
	}

	plat_data = &drv_data->plat_data;
	if (!plat_data) {
		cam_err("%s plat_data is NULL", __func__);
		return -ENODEV;
	}
	/* enable mcu to low power, after the mcu do not work
	 * when strobe gpio rising will wakeup mcu to work again
	 * when mcu in low power state, enable mcu to low power,
	 * mcu do nothing and not reponse
	 */
	ret = stm32_spi_write_reg(drv_data, MCU_STATE_CTRL_REG, MCU_STATE_LP);
	stm32_spi_read_reg(drv_data, MCU_STATE_CTRL_REG, &val);
	/* after disable, spi do not work */
	cam_info("%s MCU_STATE_CTRL_REG, write ret = %d, val = 0x%x",
		__func__, ret, val);

	return ret;
}

int stm32_spi_init(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	struct stm32_spi_plat_data *plat_data = NULL;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}

	cam_info("%s enter", __func__);

	drv_data->power_up_times++;
	drv_data->power_number++;
	if (drv_data->power_number > 1) {
		cam_info("%s stm32 already power up", __func__);
		return 0;
	}

	plat_data = &drv_data->plat_data;
	/* spi cs gpio */
	gpio_direction_output(plat_data->spi_cs_gpio, 1);

	return 0;
}
EXPORT_SYMBOL(stm32_spi_init);

int stm32_spi_exit(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;

	if (drv_data == NULL) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}

	cam_info("%s enter", __func__);
	/* when stm32 power off, mcu entry into lowpower */
	stm32_spi_enable_lp();

	drv_data->power_number--;
	cam_info("%s drv_data->power_number = %d",
		__func__, drv_data->power_number);
	if (drv_data->power_number > 0) {
		cam_info("%s drv_data->power_number = %d, will return",
			__func__, drv_data->power_number);
		return 0;
	}

	drv_data->power_number = 0; /* protection for minus */
	return 0;
}
EXPORT_SYMBOL(stm32_spi_exit);

int stm32_spi_enable(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	struct stm32_spi_plat_data *plat_data = NULL;
	int ret;
	u8 val = 0;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}

	cam_info("%s enter", __func__);

	if (drv_data->load_fw_state == LOAD_FW_FAIL) {
		cam_info("%s load fpga firmware fail, will exit", __func__);
		return -1;
	}

	if (drv_data->load_fw_state != LOAD_FW_DONE) {
		cam_info("%s load_fw_state = %d, will exit",
			__func__, drv_data->load_fw_state);
		return 0;
	}

	drv_data->enable_number++;
	if (drv_data->enable_number > ENABLE_MAX_TIME) {
		cam_info("%s drv_data->enable_number = %d, will return",
			__func__, drv_data->enable_number);
		return 0;
	}

	plat_data = &drv_data->plat_data;
	if (!plat_data) {
		cam_err("%s plat_data is NULL", __func__);
		return -ENODEV;
	}

	ret =  stm32_spi_read_reg(drv_data, MCU_FW_VER_H, &val);
	cam_info("%s reg = MCU_FW_VER_H, ret = %d, val = 0x%x",
		__func__, ret, val);

	ret =  stm32_spi_read_reg(drv_data, MCU_FW_VER_L, &val);
	cam_info("%s reg = MCU_FW_VER_L, ret = %d, val = 0x%x",
		__func__, ret, val);

	/* firstly clear the irq register */
	ret = stm32_spi_read_reg(drv_data, MCU_IRQ_STATE_REG, &val);
	if (ret) {
		cam_err("%s read reg = 0x%x error, ret = %d",
			__func__, MCU_IRQ_STATE_REG, ret);
		return ret;
	}
	/* enable fpga irq handle */
	enable_irq(drv_data->spi->irq);
	cam_info("%s enable_irq, MCU_IRQ_STATE_REG %d ", __func__, val);
	return 0;
}
EXPORT_SYMBOL(stm32_spi_enable);

int stm32_spi_disable(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	struct stm32_spi_plat_data *plat_data = NULL;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}

	cam_info("%s enter", __func__);
	plat_data = &drv_data->plat_data;
	if (drv_data->load_fw_state != LOAD_FW_DONE) {
		cam_info("%s load_fw_state = %d, will exit",
			__func__, drv_data->load_fw_state);
		return 0;
	}

	drv_data->enable_number--;
	if (drv_data->enable_number > 0) {
		cam_info("%s drv_data->enable_number = %d, will return",
			__func__, drv_data->enable_number);
		return 0;
	}

	/* shutdown fpga irq handle */
	disable_irq(drv_data->spi->irq);

	return 0;
}
EXPORT_SYMBOL(stm32_spi_disable);

static void stm32_spi_force_disable(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	struct stm32_spi_plat_data *plat_data = NULL;

	cam_info("%s enter", __func__);
	/* shutdown fpga irq handle */
	disable_irq(drv_data->spi->irq);

	plat_data = &drv_data->plat_data;
	if (!plat_data) {
		cam_err("%s plat_data is NULL", __func__);
		return;
	}
}

int stm32_spi_init_fun(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;

	cam_info("%s enter", __func__);

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}
	drv_data->enable_number = 0;
	drv_data->power_number = 0;
	drv_data->power_up_times = 0;

	return 0;
}
EXPORT_SYMBOL(stm32_spi_init_fun);

int stm32_spi_close_fun(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;

	cam_info("%s enter, drv_data->enable_number = %d",
		__func__, drv_data->enable_number);

	if (drv_data->enable_number > 0)
		stm32_spi_force_disable();
	stm32_spi_exit();
	return 0;
}
EXPORT_SYMBOL(stm32_spi_close_fun);

int mcu_fw_bin_need_update(struct stm32_spi_priv_data *devdata, u32 *fw_size)
{
	u32 file_ver = 0;
	u32 flash_ver;
	int rc;

	if (!devdata || !fw_size) {
		cam_err("%s no driver data, or fw_size is NULL", __func__);
		return -ENODEV;
	}

	rc = get_fw_version_from_bin(FLASH_FIRMWARE_BIN,
		FW_BIN_VERSION_ADDR, &file_ver, fw_size);
	if (rc < 0 || file_ver == 0x00) {
		cam_err("%s:%d firmware bin invalid", __func__, __LINE__);
		return -1;
	}
	rc = get_fw_version_from_flash(devdata,
		MEMORY_VERSION_ADDR, &flash_ver);
	if (rc != 0) {
		cam_err("%s:%d read firmware version fail", __func__, __LINE__);
		return -1;
	}
	cam_info("%s:%d mcu firmware version %u %u",
		__func__, __LINE__, file_ver, flash_ver);
	if (file_ver != flash_ver) {
		cam_info("%s:%d mcu firmware need update", __func__, __LINE__);
		return 0;
	}
	return -1;
}

int stm32_spi_load_fw_code(struct stm32_spi_priv_data *devdata)
{
	u32 fw_size;
	int sector_number;
	int loop;
	int rc;

	/* mcu firmware need update or not */
	if (mcu_fw_bin_need_update(devdata, &fw_size) != 0) {
		cam_info("%s:%d mcu firmware not need update",
			__func__, __LINE__);
		return 0;
	}

	/* erase process */
	sector_number = (fw_size - 1) / SECTOR_SIZE + 1;
	cam_info("fw_size = %u, sector_number = %d", fw_size, sector_number);
	for (loop = 0; loop < sector_number; loop++) {
		rc = boot_erase_memory_cmd(devdata, 1, loop);
		if (rc != 0) {
			cam_err("%s:%d loop = %d, erase fail",
				__func__, __LINE__, loop);
			return -1;
		}
	}

	/* mcu firmare update */
	rc = mcu_load_fw_code(devdata, FLASH_FIRMWARE_BIN);
	if (rc != 0) {
		cam_err("%s:%d load firmware fail", __func__, __LINE__);
		return -1;
	}

	cam_info("mcu firmare load success");
	return 0;
}

int stm32_spi_load_fw(void)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	int ret;

	cam_info("%s enter", __func__);

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return -ENODEV;
	}

	if (drv_data->load_fw_state != LOAD_FW_NOT_START) {
		cam_info("%s load_fw_state = %d, will exit",
			__func__, drv_data->load_fw_state);
		return 0;
	}
	drv_data->load_fw_state = LOAD_FW_DOING;

	/* bootloader SPI synchronization */
	ret = boot_sync_frame(drv_data);
	if (ret != 0) {
		cam_err("%s boot sync fail", __func__);
		drv_data->load_fw_state = LOAD_FW_FAIL;
		return -1;
	}

	/* mcu load firmware or not */
	ret = stm32_spi_load_fw_code(drv_data);
	if (ret != 0) {
		cam_err("%s - load firmware failed", __func__);
		drv_data->load_fw_state = LOAD_FW_FAIL;
		return -1;
	}

	/* goto normal mode */
	ret = boot_go_memory_cmd(drv_data, MEMORY_START_ADDR);
	if (ret != 0) {
		cam_err("%s goto normal mode fail", __func__);
		drv_data->load_fw_state = LOAD_FW_FAIL;
		return -1;
	}

	drv_data->load_fw_state = LOAD_FW_DONE;
	return 0;
}
EXPORT_SYMBOL(stm32_spi_load_fw);

static void mcu_parse_dmd_error(struct work_struct *work)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	u8 err = g_spi_drv_data->last_err_code;
	unsigned int index;
	bool find = false;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return;
	}

	cam_info("%s err = 0x%x", __func__, err);
	for (index = 0; index < ARRAY_SIZE(g_mcu_irq_err_table); index++) {
		if ((err & g_mcu_irq_err_table[index].err_head) != 0) {
			cam_info("%s: index = %d", __func__, index);
			mcu_dsm_client_notify(drv_data,
				g_mcu_irq_err_table[index].err_name,
				g_mcu_irq_err_table[index].err_num);
			find = true;
		}
	}

	if (find == false) {
		cam_info("%s: index = %d, the err is not matched",
			__func__, index);
		mcu_dsm_client_notify(drv_data,
			g_mcu_common_err_table[OTHER_ERR].err_name,
			g_mcu_common_err_table[OTHER_ERR].err_num);
	}
}

static irqreturn_t mcu_irq_thread(int irq, void *handle)
{
	struct stm32_spi_priv_data *drv_data = handle;
	u8 val = 0x00;
	int ret;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return IRQ_HANDLED;
	}

	drv_data->load_fw_number++;
	if (drv_data->load_fw_state != LOAD_FW_DONE) {
		if ((drv_data->load_fw_number % LOAD_FW_MAX_TIME) == 0) {
			cam_info("%s load_fw_state = %d, will exit",
				__func__, drv_data->load_fw_state);
			drv_data->load_fw_number = 0;
		}
		return IRQ_HANDLED;
	}

	cam_info("%s enter...", __func__);

	drv_data->load_fw_number = 0;

	ret =  stm32_spi_read_reg(drv_data, MCU_IRQ_STATE_REG, &val);
	if (ret)
		cam_err("%s stm32_spi_read_reg error, ret = %d", __func__, ret);
	if (val == 0x00) {
		cam_info("%s reg = 0x02, val = 0x00, will return", __func__);
		return IRQ_HANDLED;
	}
	cam_info("%s reg = 0x02, val = 0x%02x", __func__, val);
	drv_data->last_err_code = val;
	queue_work(drv_data->work_queue, &drv_data->dump_err_work);

	return IRQ_HANDLED;
}

#ifdef MCU_READ_REG_DEBUG
static int hw_mcu_param_check(char *buf, unsigned long *param, int num_of_par)
{
	char *token = NULL;
	unsigned int base;
	int cnt;

	if ((!buf) || (!param)) {
		cam_err("%s buf or param is NULL", __func__);
		return -1;
	}

	token = strsep(&buf, " ");
	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token != NULL) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16;  /* hex */
			else
				base = 10;  /* decimal */
			if (kstrtoul(token, base, &param[cnt]) != 0)
				return -EINVAL;
			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

static ssize_t hw_mcu_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	int rc;
	u8 val = 0x00;
	int ret;

	if ((!drv_data) || (!buf)) {
		cam_err("%s drv_data or buf is NULL", __func__);
		return -1;
	}

	ret =  stm32_spi_read_reg(drv_data, g_reg, &val);
	if (ret)
		cam_err("%s stm32_spi_read_reg error, ret = %d", __func__, ret);
	rc = scnprintf(buf, 0x10, "0x%x\n", val);

	return rc;
}

static ssize_t hw_mcu_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	unsigned long param[2] = {0}; /* reg and value, 2 bytes */
	int rc;

	if ((!drv_data) || (!buf)) {
		cam_err("%s drv_data or buf is NULL", __func__);
		return -1;
	}

	rc = hw_mcu_param_check((char *)buf, param, 2);
	if (rc < 0) {
		cam_err("%s failed to check param", __func__);
		return rc;
	}

	g_reg = (u8)param[0];
	g_reg_value = (u8)param[1];
	cam_info("%s g_reg = %d g_reg_value = %d",
		__func__, g_reg, g_reg_value);

	if (g_reg_value != 0xff)
		stm32_spi_write_reg(drv_data, g_reg, g_reg_value);
	else
		cam_info("%s not allow write", __func__);

	return (ssize_t)count;
}

extern int register_camerafs_attr(struct device_attribute *attr);
static struct device_attribute g_dev_attr_mcu =
	__ATTR(mcureg, 0664, hw_mcu_show, hw_mcu_store);
#endif

static void stm32_spi_cs_set(u32 control)
{
	int ret;
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;
	struct stm32_spi_plat_data *plat_data = NULL;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return;
	}

	plat_data = &drv_data->plat_data;
	if (!plat_data) {
		cam_err("%s plat_data is NULL", __func__);
		return;
	}

	if (control == SSP_CHIP_SELECT) {
		ret = gpio_direction_output(plat_data->spi_cs_gpio, control);
		/* cs steup time at least 10ns */
		ndelay(10);
	} else {
		/* cs hold time at least 4*40ns(@25MHz) */
		ret = gpio_direction_output(plat_data->spi_cs_gpio, control);
		ndelay(200);
	}

	if (ret < 0)
		cam_err("%s: fail to set gpio cs, result = %d", __func__, ret);
}

static int stm32_parse_gpio_config(struct stm32_spi_plat_data *plat_data,
	struct device_node *of_node)
{
	unsigned int value;

	if ((!plat_data) || (!of_node)) {
		cam_err("%s plat_data or of_node is NULL", __func__);
		return -EINVAL;
	}

	value = of_get_named_gpio(of_node, "stm32_spi,cs_gpio", 0);
	if (!gpio_is_valid(value)) {
		cam_err("%s get cs_gpio failed", __func__);
		return -1;
	}
	plat_data->spi_cs_gpio = value;
	cam_info("%s cs_gpio = %d", __func__, plat_data->spi_cs_gpio);

	value = of_get_named_gpio(of_node, "stm32_spi,rst_gpio", 0);
	if (!gpio_is_valid(value)) {
		cam_err("%s get rst_gpio failed", __func__);
		return -1;
	}
	plat_data->reset_gpio = value;
	cam_info("%s reset_gpio = %d", __func__, plat_data->reset_gpio);

	value = of_get_named_gpio(of_node, "stm32_spi,boot_gpio", 0);
	if (!gpio_is_valid(value)) {
		cam_err("%s get boot_gpio failed", __func__);
		return -1;
	}
	plat_data->boot_gpio = value;
	cam_info("%s boot_gpio = %d", __func__, plat_data->boot_gpio);

	value = of_get_named_gpio(of_node, "stm32_spi,strobe_gpio", 0);
	if (!gpio_is_valid(value)) {
		cam_err("%s get boot_gpio failed", __func__);
		return -1;
	}
	plat_data->strobe_gpio = value;
	cam_info("%s strobe_gpio = %d", __func__, plat_data->strobe_gpio);

	value = of_get_named_gpio(of_node, "stm32_spi,irq_gpio", 0);
	if (!gpio_is_valid(value)) {
		cam_err("%s get irq_gpio failed", __func__);
		return -1;
	}
	plat_data->irq_gpio = value;
	cam_info("%s irq_gpio = %d", __func__, plat_data->irq_gpio);

	return 0;
}

int stm32_parse_spi_config(struct stm32_spi_plat_data *plat_data,
	struct device_node *of_node)
{
	int rc;
	unsigned int value;
	struct pl022_config_chip *pl022_spi_config = NULL;

	if (!plat_data || !of_node) {
		cam_err("%s plat_data or of_node is NULL", __func__);
		return -EINVAL;
	}

	pl022_spi_config =
		(struct pl022_config_chip *)&plat_data->spidev0_chip_info;

	pl022_spi_config->hierarchy = 0;
	pl022_spi_config->cs_control = stm32_spi_cs_set;

	rc = of_property_read_u32(of_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		cam_info("%s iface configed %d",
			__func__, pl022_spi_config->iface);
	}

	rc = of_property_read_u32(of_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		cam_info("%s com_mode configed %d",
			__func__, pl022_spi_config->com_mode);
	}

	rc = of_property_read_u32(of_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		cam_info("%s rx_lev_trig configed %d",
			__func__, pl022_spi_config->rx_lev_trig);
	}

	rc = of_property_read_u32(of_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		cam_info("%s tx_lev_trig configed %d",
			__func__, pl022_spi_config->tx_lev_trig);
	}

	rc = of_property_read_u32(of_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		cam_info("%s ctrl_len configed %d",
			__func__, pl022_spi_config->ctrl_len);
	}

	rc = of_property_read_u32(of_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		cam_info("%s wait_stateconfiged %d",
			__func__, pl022_spi_config->wait_state);
	}

	rc = of_property_read_u32(of_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		cam_info("%s duplex %d", __func__, pl022_spi_config->duplex);
	}

#if defined(CONFIG_ARCH_HI3630FPGA) || defined(CONFIG_ARCH_HI3630)
	rc = of_property_read_u32(of_node, "pl022,slave-tx-disable", &value);
	if (!rc) {
		pl022_spi_config->slave_tx_disable = value;
		cam_info("%s slave_tx_disable %d",
			__func__, pl022_spi_config->slave_tx_disable);
	}
#endif

	return 0;
}

static int stm32_spi_get_dt_data(struct device *dev,
	 struct stm32_spi_plat_data *plat_data)
{
	int ret;
	unsigned int value;
	struct device_node *of_node = NULL;

	if (!dev || !dev->of_node || !plat_data) {
		cam_err("%s dev or of_node or plat_data is NULL", __func__);
		return -EINVAL;
	}
	of_node = dev->of_node;

	ret = stm32_parse_gpio_config(plat_data, of_node);
	if (ret) {
		cam_err("%s parse gpio config failed", __func__);
		return -1;
	}

	ret = stm32_parse_spi_config(plat_data, of_node);
	if (ret) {
		cam_err("%s parse spi config failed", __func__);
		return -1;
	}

	ret = of_property_read_u32(of_node, "stm32_spi,chip_type", &value);
	if (ret < 0) {
		cam_err("%s get chip_type failed", __func__);
		return -1;
	}
	plat_data->chip_type = value;
	cam_info("%s chip_type = %d", __func__, plat_data->chip_type);

	/* external clock, currently not used */
	ret = of_property_read_string(of_node, "clock-names",
		&plat_data->fpga_clkname);
	if (ret < 0) {
		cam_err("%s get clock-names failed", __func__);
		return -1;
	}

	return ret;
}

int stm32_spi_checkdevice(void)
{
	int ret = 0;
	struct stm32_spi_priv_data *drv_data = g_spi_drv_data;

	if (!drv_data) {
		cam_err("drv_data is null,maybe probe is wrong,ret = -1");
		return -1;
	}
	if (drv_data->load_fw_state == LOAD_FW_FAIL) {
		ret = 1; /* LOAD_FW_FAIL */
		cam_err("fpga load fw fail,ret = 1");
	}
	/* check bit0 */
	if ((drv_data->last_err_code & 0x01) == 0x01) {
		ret = 2; /* 9 frame error */
		cam_err("fpga 9 frame error,ret = 2");
	}
	return ret;
}
EXPORT_SYMBOL(stm32_spi_checkdevice);

static int stm32_spi_probe(struct spi_device *spi)
{
	struct stm32_spi_priv_data *drv_data = NULL;
	struct stm32_spi_plat_data *plat_data = NULL;
	struct hw_comm_pmic_cfg_t fp_pmic_ldo_set;
	uintptr_t addr;
	int ret;

	cam_info("%s enter", __func__);

	drv_data = kmalloc(sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data) {
		cam_err("probe - can not alloc driver data");
		return -ENOMEM;
	}
	ret = memset_s(drv_data, sizeof(*drv_data), 0, sizeof(*drv_data));
	if (ret != 0) {
		cam_err("%s memset_s fail", __func__);
		kfree(drv_data);
		return -EINVAL;
	}

	plat_data = &drv_data->plat_data;
	ret = stm32_spi_get_dt_data(&spi->dev, plat_data);
	if (ret < 0) {
		cam_err("%s failed to stm32_spi_get_dt_data", __func__);
		goto err_no_pinctrl;
	}

	addr = __get_free_pages(GFP_KERNEL | GFP_DMA, get_order(2 * PAGE_SIZE));
	drv_data->tx_buf = (void *)addr;
	addr = __get_free_pages(GFP_KERNEL | GFP_DMA, get_order(2 * PAGE_SIZE));
	drv_data->rx_buf = (void *)addr;
	addr = __get_free_pages(GFP_KERNEL | GFP_DMA, get_order(PAGE_SIZE));
	drv_data->irq_buf = (void *)addr;
	cam_info("%s tx_buf=0x%pK rx_buf=0x%pK irq_buf=0x%pK",
		__func__,
		drv_data->tx_buf, drv_data->rx_buf, drv_data->irq_buf);
	if ((!drv_data->tx_buf) || (!drv_data->rx_buf) ||
		(!drv_data->irq_buf)) {
		cam_err("%s can not alloc dma buf page", __func__);
		ret = -ENOMEM;
		goto err_alloc_buf;
	}

	drv_data->spi = spi;
	mutex_init(&drv_data->busy_lock);
	INIT_WORK(&drv_data->dump_err_work, mcu_parse_dmd_error);
	drv_data->work_queue =
		create_singlethread_workqueue(dev_name(&spi->dev));
	if (!drv_data->work_queue) {
		cam_err("probe - create workqueue error");
		ret = -EBUSY;
		goto err_create_queue;
	}

	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = STM32_SPI_SPEED_8M;
	spi->bits_per_word = 8; /* 8 bits */
	spi->controller_data = &plat_data->spidev0_chip_info;
	ret = spi_setup(spi);
	if (ret < 0) {
		cam_err("probe - setup spi error");
		goto err_spi_setup;
	}

	fp_pmic_ldo_set.pmic_num = 1; /* use second pmic */
	fp_pmic_ldo_set.pmic_power_type = VOUT_LDO_4;
	fp_pmic_ldo_set.pmic_power_voltage = LDO_VOLTAGE_V2P85V;
	fp_pmic_ldo_set.pmic_power_state = POWER_ON;
	ret = hw_pmic_power_cfg(DOT_PMIC_REQ, &fp_pmic_ldo_set);
	if (ret < 0) {
		cam_err("%s failed to hw_pmic_power_cfg DOT_PMIC_REQ, ret=%d",
			__func__, ret);
	}
	mdelay(1);
	cam_info("%s hw_pmic_power_cfg DOT_PMIC_REQ, ret=%d", __func__, ret);

	/* VOUT_MCU_1V8 */
	fp_pmic_ldo_set.pmic_num = 1; /* use second pmic */
	fp_pmic_ldo_set.pmic_power_type = VOUT_LDO_3;
	fp_pmic_ldo_set.pmic_power_voltage = LDO_VOLTAGE_1P8V;
	fp_pmic_ldo_set.pmic_power_state = POWER_ON;
	ret = hw_pmic_power_cfg(DF_PMIC_REQ, &fp_pmic_ldo_set);
	if (ret < 0) {
		cam_err("%s failed to hw_pmic_power_cfg DF_PMIC_REQ, ret=%d",
			__func__, ret);
	}
	mdelay(100); /* delay time 100ms */
	cam_info("%s hw_pmic_power_cfg DF_PMIC_REQ, ret=%d", __func__, ret);
	plat_data->pinctrl =
		devm_pinctrl_get_select(&spi->dev, PINCTRL_STATE_DEFAULT);
	if (!plat_data->pinctrl) {
		cam_err("%s failed to get pinctrl", __func__);
		goto err_dev_attr;
	}

	ret = gpio_request(plat_data->spi_cs_gpio, "stm32_spi_cs");
	if (ret) {
		cam_err("probe - request spi cs gpio error");
		goto err_cs_gpio;
	}
	gpio_direction_output(plat_data->spi_cs_gpio, 1);

	ret = gpio_request(plat_data->strobe_gpio, "stm32_spi_strobe");
	if (ret) {
		cam_err("probe - request reset gpio error");
		goto err_strobe_gpio;
	}

	ret = gpio_request(plat_data->reset_gpio, "stm32_spi_reset");
	if (ret) {
		cam_err("probe - request reset gpio error");
		goto err_reset_gpio;
	}
	gpio_direction_output(plat_data->reset_gpio, 0);
	udelay(100); /* delay tiem 100us */

	ret = gpio_request(plat_data->boot_gpio, "stm32_spi_boot");
	if (ret) {
		cam_err("probe - request enable gpio error");
		goto err_boot_gpio;
	}

	/* enter boot mode */
	gpio_direction_output(plat_data->boot_gpio, 1);
	udelay(100); /* delay tiem 100us */
	gpio_direction_output(plat_data->reset_gpio, 1);
	udelay(100); /* delay tiem 100us */
	/* reset to normal mode */
	gpio_direction_output(plat_data->boot_gpio, 0);

#ifdef MCU_READ_REG_DEBUG
	ret = register_camerafs_attr(&g_dev_attr_mcu);
	if (ret < 0)
		cam_err("%s register_camerafs_attr failed line %d\n",
			 __func__, __LINE__);
#endif

	drv_data->client_stm32 = client_camera_user;

	/* set driver_data to device */
	spi_set_drvdata(spi, drv_data);
	g_spi_drv_data = drv_data;
	/* initialize */
	drv_data->enable_number = 0;
	drv_data->power_number = 0;
	drv_data->power_up_times = 0;
	drv_data->load_fw_number = 0;
	drv_data->load_fw_state = LOAD_FW_NOT_START;
	drv_data->last_err_code = 0;

	ret = gpio_request(plat_data->irq_gpio, "stm32_spi_irq");
	if (ret) {
		cam_err("probe - request irq gpio error");
		goto err_irq_gpio;
	}
	gpio_direction_input(plat_data->irq_gpio);

	spi->irq = gpio_to_irq(plat_data->irq_gpio);
	ret = request_threaded_irq(spi->irq, NULL, mcu_irq_thread,
		IRQF_ONESHOT | IRQF_TRIGGER_RISING,
		"stm32_isp", drv_data);
	if (ret) {
		cam_err("probe - request irq error(%d)", spi->irq);
		goto err_irq_config;
	}
	disable_irq(spi->irq);
	cam_info("stm32 spi probe irq_gpio=%d, irq=%d",
		plat_data->irq_gpio, spi->irq);
	cam_info("stm32 spi probe success");
	return ret;

err_irq_config:
	gpio_free(plat_data->irq_gpio);
err_irq_gpio:
	gpio_free(plat_data->boot_gpio);
err_boot_gpio:
	gpio_free(plat_data->reset_gpio);
err_reset_gpio:
	gpio_free(plat_data->strobe_gpio);
err_strobe_gpio:
	gpio_free(plat_data->spi_cs_gpio);
err_cs_gpio:
err_dev_attr:
err_spi_setup:
	destroy_workqueue(drv_data->work_queue);
err_create_queue:
	free_pages((unsigned long)(uintptr_t)drv_data->tx_buf,
		get_order(2 * PAGE_SIZE));
	free_pages((unsigned long)(uintptr_t)drv_data->rx_buf,
		get_order(2 * PAGE_SIZE));
	free_pages((unsigned long)(uintptr_t)drv_data->irq_buf,
		get_order(PAGE_SIZE));
err_alloc_buf:
err_no_pinctrl:
	kfree(drv_data);
	g_spi_drv_data = NULL;
	return ret;
}

static int stm32_spi_remove(struct spi_device *sdev)
{
	if (sdev && sdev->dev.driver_data) {
		kfree(sdev->dev.driver_data);
		sdev->dev.driver_data = NULL;
	}
	g_spi_drv_data = NULL;
	return 0;
}

static const struct of_device_id g_stm32_spi_dt_ids[] = {
	{ .compatible = "stm32,spi" },
	{},
};

static const struct spi_device_id g_stm32_device_id[] = {
	{ STM32_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, g_stm32_device_id);

static struct spi_driver g_stm32_spi_drv = {
	.probe    = stm32_spi_probe,
	.remove   = stm32_spi_remove,
	.id_table = g_stm32_device_id,
	.driver = {
		.name           = STM32_DEVICE_NAME,
		.owner          = THIS_MODULE,
		.bus            = &spi_bus_type,
		.of_match_table = g_stm32_spi_dt_ids,
	},
};
module_spi_driver(g_stm32_spi_drv);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stm32 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
