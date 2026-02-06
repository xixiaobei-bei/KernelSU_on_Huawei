/*
 * m25p80_norflash.c
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mod_devicetable.h>

#include <linux/mtd/cfi.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of_platform.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <linux/amba/pl022.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <securec.h>
#include "hwsensor.h"
#include "sensor_commom.h"
#include "../pmic/hw_pmic.h"
#include "m25p80_norflash.h"

#define M25P_SPI_SPEED_25M 25000000

#define CMD_RET_TRUE   0x55
#define CMD_RET_FALSE  0xAA

#define SECTOR_SZ  256
#define PAGE_SZ    0x1000
#define BLOCK_SZ   0x10000
#define ERASE_WAIT_TIMES       3000
#define BLOCK_ERASE_WAIT_TIMES 30000
#define PROGRAM_WAIT_TIMES 50

#define WORD_8BITS     8
#define WORD_16BITS    16

#define MAX_BUFFER_SIZE 300

struct m25p_spi_plat_data {
	int spi_cs_gpio;
	int irq_gpio;
	int reset_gpio;
	int enable_gpio;
	int chip_type;
	/* spi master config */
	struct pl022_config_chip spidev0_chip_info;
	/* pin control config */
	struct pinctrl          *pinctrl;
	struct pinctrl_state    *pins_default;
	struct pinctrl_state    *pins_idle;
};

struct m25p_spi_priv_data {
	struct spi_device           *spi;
	struct mutex                 busy_lock;
	struct m25p_spi_plat_data    plat_data;
	int       state;
	/*
	 * NOTE: All buffers should be dma-safe buffers
	 * tx_buf :used for 64-Bytes CMD-send or 8K-Bytes Block-send
	 * rx_buf :used for 64-Bytes CMD-recv or 4K-Bytes Block-recv
	 */
	u8       *tx_buf;
	u8       *rx_buf;
	u8       *ext_buf;
};

struct hwnorflash_array_partinfo {
	u32 start_addr;
	u32 end_addr;
	u32 part_len;
};

struct hwnorflash_array_partinfo g_array_part_tab[] = {
	[0] = { /* default */
		.start_addr = 0x00,
		.end_addr  = 0x1FFF,
		.part_len  = 0x2000,
	},
	[1] = { /* holder */
		.start_addr = 0x2000,
		.end_addr  = 0x12DFFF,
		.part_len  = 0x12C000,
	},
	[2] = { /* device */
		.start_addr = 0x12E000,
		.end_addr  = 0x1F5FFF,
		.part_len  = 0xC8000,
	},
	[3] = { /* entirety */
		.start_addr = 0x00,
		.end_addr  = 0x1F5FFF,
		.part_len  = 0x1F6000,
	}
};

static struct m25p_spi_priv_data *g_spi_drv_data;

static int norflash_spi_read_stack(u8 *send, u16 send_len,
	u8 *receive, u16 receive_len)
{
	struct m25p_spi_priv_data *drv_data = g_spi_drv_data;
	struct spi_transfer xfer[2];
	int status;
	int ret;

	if (!send || !receive || !drv_data) {
		cam_err("%s send or receive is NULL", __func__);
		return -1;
	}

	ret = memcpy_s(drv_data->tx_buf, send_len, send, send_len);
	if (ret != 0) {
		cam_err("%s memcpy failed %d", __func__, __LINE__);
		return -EINVAL;
	}

	ret = memset_s(drv_data->rx_buf, receive_len, 0, receive_len);
	if (ret != 0) {
		cam_err("%s memset failed %d", __func__, __LINE__);
		return -EINVAL;
	}
	if (drv_data->tx_buf[send_len - 1] != 0xFF) {
		cam_err("%s the last byter of send buffer must be 0xFF",
			__func__);
		return -1;
	}

	ret = memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
	if (ret != 0) {
		cam_err("%s memset failed %d", __func__, __LINE__);
		return -EINVAL;
	}
	xfer[0].tx_buf = &drv_data->tx_buf[0],
	xfer[0].len = send_len - 1,
	xfer[0].bits_per_word = 8,
	xfer[1].tx_buf = &drv_data->tx_buf[send_len - 1],
	xfer[1].rx_buf = drv_data->rx_buf,
	xfer[1].len = receive_len,

	mutex_lock(&drv_data->busy_lock);
	status = spi_sync_transfer(drv_data->spi, xfer, 2);
	if (status == 0)
		status = receive_len;
	else
		cam_err("Failed to complete SPI transfer, error = %d\n",
			status);

	mutex_unlock(&drv_data->busy_lock);

	ret = memcpy_s(receive, receive_len, drv_data->rx_buf, receive_len);
	if (ret != 0) {
		cam_err("%s memcpy failed %d", __func__, __LINE__);
		return -EINVAL;
	}
	return status;
}

static int norflash_spi_write_stack(u8 *send, u16 send_len)
{
	struct m25p_spi_priv_data *drv_data = g_spi_drv_data;
	struct spi_transfer xfer;
	struct spi_message  m;
	int status;
	int ret;

	if (!drv_data || !send) {
		cam_err("%s - drv_data or send is NULL", __func__);
		return -EINVAL;
	}

	ret = memcpy_s(drv_data->tx_buf, send_len, send, send_len);
	if (ret != 0) {
		cam_err("%s memcpy failed %d", __func__, __LINE__);
		return -EINVAL;
	}

	ret = memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
	if (ret != 0) {
		cam_err("%s memcpy failed %d", __func__, __LINE__);
		return -EINVAL;
	}

	xfer.tx_buf = drv_data->tx_buf;
	xfer.rx_buf = NULL;
	xfer.len = send_len;

	mutex_lock(&drv_data->busy_lock);
	spi_message_init(&m);
	spi_message_add_tail(&xfer, &m);
	status = spi_sync(drv_data->spi, &m);
	if (status) {
		cam_err("%s - sync error: status=%d", __func__, status);
		mutex_unlock(&drv_data->busy_lock);
		return status;
	}
	mutex_unlock(&drv_data->busy_lock);

	return status;
}

static void write_en(void)
{
	char buf[2] = {0x06, 0xFF};

	norflash_spi_write_stack(buf, 1);
}

static u8 read_sr(void)
{
	u8 sr = 0;
	char buf[2] = {0x05, 0xFF};

	norflash_spi_read_stack(buf, 2, &sr, 1);
	return sr;
}

/* get chip id */
static void read_id(u8 *id_buf, size_t buf_len)
{
	char buf[2] = {0x9f, 0xFF};

	if (!id_buf) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return;
	}
	norflash_spi_read_stack(buf, 2, id_buf, buf_len);
}

static int toggle_sr(int ms)
{
	u8 sr;
	int i;

	for (i = 0; i < ms; i++) {
		sr = read_sr();
		if ((sr & 0x03) == 0x00)
			return 0x55; /* pass */
		udelay(100); /* delay 100us */
	}
	cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
	return 0xaa; /* over time */
}

static int page_program(u32 addr, u8 *buf, int byte_num, int overtime)
{
	u8 pp_buf[MAX_BUFFER_SIZE];
	int i;

	if (!buf) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa;
	}

	write_en();
	if ((read_sr() & 0x02) != 0x02) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa; /* write_en fail */
	}

	pp_buf[0] = 0x02;
	pp_buf[1] = addr >> 16;
	pp_buf[2] = addr >> 8;
	pp_buf[3] = addr;
	for (i = 0; i < byte_num; i++)
		pp_buf[4 + i] = buf[i];

	norflash_spi_write_stack(pp_buf, (byte_num + 4)); /* 4 bytes */

	if (toggle_sr(overtime) != 0x55) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa; /* over time */
	}

	return 0x55; /* pass */
}

static int sector_erase(u32 addr, int overtime)
{
	u8 se_buf[MAX_BUFFER_SIZE];

	write_en();
	if ((read_sr() & 0x02) != 0x02)
		return 0xaa; /* write_en fail */

	se_buf[0] = 0x20;
	se_buf[1] = addr >> 16;
	se_buf[2] = addr >> 8;
	se_buf[3] = addr;
	norflash_spi_write_stack(se_buf, 4); /* 4 bytes */

	if (toggle_sr(overtime) != 0x55) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa; /* over time */
	}
	return 0x55; /* pass */
}

static int block_erase(u32 addr, int overtime)
{
	u8 bl_buf[MAX_BUFFER_SIZE];

	write_en();
	if ((read_sr() & 0x02) != 0x02)
		return 0xaa; /* write_en fail */

	bl_buf[0] = 0xD8;
	bl_buf[1] = addr >> 16;
	bl_buf[2] = addr >> 8;
	bl_buf[3] = addr;
	norflash_spi_write_stack(bl_buf, 4); /* 4 bytes */

	if (toggle_sr(overtime) != 0x55) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa; /* over time */
	}
	return 0x55; /* pass */
}

int fast_read_nbyte(u32 addr, u8 *rd_buf, int byte_num)
{
	u8 send_buf[MAX_BUFFER_SIZE];
	int status;

	if (!rd_buf) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa;
	}

	send_buf[0] = 0x0B;
	send_buf[1] = addr >> 16;
	send_buf[2] = addr >> 8;
	send_buf[3] = addr;
	send_buf[4] = 0;
	send_buf[5] = 0xFF;

	status = norflash_spi_read_stack(send_buf, 6, rd_buf, byte_num);
	if (status < 0) {
		cam_err("%s [%d]: m25p80_norflash failed", __func__, __LINE__);
		return 0xaa;
	}
	return 0x55; /* pass */
}
EXPORT_SYMBOL(fast_read_nbyte);

int m25p_get_array_part_content(u32 type, void *user_addr, unsigned long size)
{
	int ret;
	u8 *buff = NULL;
	u32 need_sz;
	u32 start_addr = 0;
	u32 one_read_sz;
	u32 read_sz = 0;
	u8 m25p_id[3] = {0}; /* 3 bytes */
	void __user *argp = (void __user *)user_addr;

	cam_info("%s enter, type = %d, size = 0x%x",
		__func__, type, (unsigned int)size);

	if (type > IRSENSOR_ENTIRETY_OTP || user_addr == 0 || size == 0) {
		cam_err("%s type = %d, user_addr = %pK, size = %u, invalid",
			__func__, type, user_addr, (unsigned int)size);
		return -EINVAL;
	}
	if (size < g_array_part_tab[type].part_len) {
		cam_err("%s size = 0x%x, smaller than 0x%x, error", __func__,
			(unsigned int)size,
			(unsigned int)g_array_part_tab[type].part_len);
		return -EINVAL;
	}

	/* check reading correct, value: 0xEF 0x60 0x15 */
	read_id(m25p_id, sizeof(m25p_id));
	cam_info("m25p_id = 0x%x 0x%x 0x%x",
		m25p_id[0], m25p_id[1], m25p_id[2]);

	buff = (u8 *)vmalloc(sizeof(u8) * (size + 1));
	if (!buff) {
		cam_err("%s malloc failed", __func__);
		return -1;
	}
	ret = memset_s(buff, size + 1, 0, size + 1);
	if (ret != 0) {
		cam_err("memset failed %d", __LINE__);
		vfree(buff);
		return -EINVAL;
	}

	need_sz = g_array_part_tab[type].part_len;
	start_addr = g_array_part_tab[type].start_addr;
	while (read_sz < need_sz) {
		one_read_sz = need_sz - read_sz;
		one_read_sz = one_read_sz > SECTOR_SZ ? SECTOR_SZ : one_read_sz;
		ret = fast_read_nbyte(start_addr + read_sz,
			buff + read_sz, one_read_sz);
		if (ret != CMD_RET_TRUE) {
			cam_err("%s [%d]: fast_read_nbyte failed",
				__func__, __LINE__);
			vfree(buff);
			return -1;
		}
		read_sz += one_read_sz;
	}

	if (read_sz != g_array_part_tab[type].part_len) {
		cam_err("%s read_sz = 0x%x VS g_array_part_tab[%d].part_len = 0x%x, failed",
			__func__, read_sz, type,
			g_array_part_tab[type].part_len);
		vfree(buff);
		return -1;
	}

	if (copy_to_user(argp, buff, size)) {
		cam_err("%s copy_to_user failed", __func__);
		vfree(buff);
		return -1;
	}

	cam_info("%s exit, read_sz = 0x%x, get otp info successful",
		__func__, read_sz);
	vfree(buff);
	return 0;
}
EXPORT_SYMBOL(m25p_get_array_part_content);

static int m25p_array_part_content_erase(u32 type)
{
	u32 erase_tms;
	u32 erase_size = 0;
	u32 start;
	u32 first_4k_sz;
	u32 count;
	int ret;

	cam_info("%s enter, type = %d",  __func__, type);

	if (type >= IRSENSOR_ENTIRETY_OTP || type == IRSENSOR_DEFAULT_OTP) {
		cam_err("%s type = %d, invalid", __func__, type);
		return -EINVAL;
	}

	start = g_array_part_tab[type].start_addr;
	first_4k_sz = ((start / BLOCK_SZ + 1) * BLOCK_SZ - start) % BLOCK_SZ;
	count = first_4k_sz / PAGE_SZ;

	erase_tms = 0;
	while (erase_tms < count) {
		ret = sector_erase(g_array_part_tab[type].start_addr +
			erase_tms * PAGE_SZ,
			ERASE_WAIT_TIMES);
		if (ret != CMD_RET_TRUE) {
			cam_err("%s [%d]: sector_erase failed",
				__func__, __LINE__);
			return -1;
		}
		erase_tms++;
	}
	erase_size += erase_tms * PAGE_SZ;
	cam_info("%s [%d]: erase_size = 0x%x",
		__func__, __LINE__, erase_size);

	if (((g_array_part_tab[type].start_addr + erase_size) % BLOCK_SZ) != 0) {
		cam_err("%s not integral multiple of BLOCK_SZ", __func__);
		return -1;
	}

	count = (g_array_part_tab[type].part_len - erase_size) / BLOCK_SZ;
	erase_tms = 0;
	while (erase_tms < count) {
		ret = block_erase(g_array_part_tab[type].start_addr +
			erase_size + erase_tms * BLOCK_SZ,
			BLOCK_ERASE_WAIT_TIMES);
		if (ret != CMD_RET_TRUE) {
			cam_err("%s [%d]: block_erase failed",
				__func__, __LINE__);
			return -1;
		}
		erase_tms++;
	}
	erase_size += erase_tms * BLOCK_SZ;
	cam_info("%s [%d]: erase_size = 0x%x",
		__func__, __LINE__, erase_size);

	count = (g_array_part_tab[type].part_len - erase_size) / PAGE_SZ;
	erase_tms = 0;
	while (erase_tms < count) {
		ret = sector_erase(g_array_part_tab[type].start_addr +
			erase_size + erase_tms * PAGE_SZ,
			ERASE_WAIT_TIMES);
		if (ret != CMD_RET_TRUE) {
			cam_err("%s [%d]: sector_erase failed",
				__func__, __LINE__);
			return -1;
		}
		erase_tms++;
	}
	erase_size += erase_tms * PAGE_SZ;
	cam_info("%s [%d]: erase_size = 0x%x",
		__func__, __LINE__, erase_size);

	if (erase_size != g_array_part_tab[type].part_len) {
		cam_err("%s erase_size = 0x%x VS g_array_part_tab[%d].part_len = 0x%x, failed",
			__func__, erase_size, type,
			g_array_part_tab[type].part_len);
		return -1;
	}
	cam_info("%s exit, part_len = 0x%x",  __func__,
		g_array_part_tab[type].part_len);
	return 0;
}

int m25p_set_array_part_content(u32 type, void *user_addr, unsigned long size)
{
	int ret;
	u8 *data = NULL;
	u32 one_write_sz;
	u32 write_sz = 0;
	void __user *argp = (void __user *)user_addr;

	cam_info("%s enter, type = %d, size = 0x%x",  __func__,
		type, (unsigned int)size);

	if (type >= IRSENSOR_ENTIRETY_OTP || type == IRSENSOR_DEFAULT_OTP ||
		user_addr == 0 || size == 0) {
		cam_err("%s type = %d, user_addr = %pK, size = %u, invalid",
			__func__, type, user_addr, (unsigned int)size);
		return -EINVAL;
	}
	if (size < g_array_part_tab[type].part_len) {
		cam_err("%s size = 0x%x, smaller than 0x%x, error", __func__,
			(unsigned int)size,
			(unsigned int)g_array_part_tab[type].part_len);
		return -EINVAL;
	}

	data = (u8 *)vmalloc(sizeof(u8) * (size + 1));
	if (!data) {
		cam_err("%s malloc failed", __func__);
		return -1;
	}
	ret = memset_s(data, size + 1, 0, size + 1);
	if (ret != 0) {
		cam_err("memset failed %d", __LINE__);
		vfree(data);
		return -EINVAL;
	}

	if (copy_from_user(data, argp, size)) {
		cam_err("%s copy_from_user failed", __func__);
		vfree(data);
		return -1;
	}

	if (m25p_array_part_content_erase(type) != 0) {
		cam_err("%s m25p_array_part_content_erase failed", __func__);
		vfree(data);
		return -1;
	}

	size = g_array_part_tab[type].part_len;
	while (write_sz < size) {
		one_write_sz = size - write_sz;
		one_write_sz = one_write_sz > SECTOR_SZ ?
			SECTOR_SZ : one_write_sz;
		ret = page_program(g_array_part_tab[type].start_addr + write_sz,
			data + write_sz,
			one_write_sz,
			PROGRAM_WAIT_TIMES);
		if (ret != CMD_RET_TRUE) {
			cam_err("%s [%d]: page_program failed",
				__func__, __LINE__);
			vfree(data);
			return -1;
		}
		write_sz += one_write_sz;
	}

	if (write_sz != size) {
		cam_err("%s write_sz = 0x%x VS size = 0x%x, write fail",
			__func__, write_sz, (unsigned int)size);
		vfree(data);
		return -1;
	}

	cam_info("%s exit, type = %d, write_sz = 0x%x, set otp info successful",
		__func__, type, write_sz);
	vfree(data);
	return 0;
}
EXPORT_SYMBOL(m25p_set_array_part_content);

int m25p_set_spi_cs_value(u8 val)
{
	int ret;
	struct m25p_spi_priv_data *drv_data = g_spi_drv_data;
	struct m25p_spi_plat_data *plat_data = &drv_data->plat_data;

	cam_info("%s val = %d",  __func__, val);

	if (val)
		ret = gpio_direction_output(plat_data->spi_cs_gpio, 1);
	else
		ret = gpio_direction_output(plat_data->spi_cs_gpio, 0);

	if (ret < 0)
		cam_err("%s: set spi cs failed", __func__);

	return ret;
}
EXPORT_SYMBOL(m25p_set_spi_cs_value);

static void m25p_spi_cs_set(u32 control)
{
	int ret;
	struct m25p_spi_priv_data *drv_data = g_spi_drv_data;
	struct m25p_spi_plat_data *plat_data = NULL;

	if (!drv_data) {
		cam_err("%s no driver data", __func__);
		return;
	}
	plat_data = &drv_data->plat_data;

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

int m25p_parse_spi_config(struct m25p_spi_plat_data *plat_data,
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
	pl022_spi_config->cs_control = m25p_spi_cs_set;

	rc = of_property_read_u32(of_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		cam_info("%s iface configed %d", __func__,
			pl022_spi_config->iface);
	}

	rc = of_property_read_u32(of_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		cam_info("%s com_mode configed %d", __func__,
			pl022_spi_config->com_mode);
	}

	rc = of_property_read_u32(of_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		cam_info("%s rx_lev_trig configed %d", __func__,
			pl022_spi_config->rx_lev_trig);
	}

	rc = of_property_read_u32(of_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		cam_info("%s tx_lev_trig configed %d", __func__,
			pl022_spi_config->tx_lev_trig);
	}

	rc = of_property_read_u32(of_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		cam_info("%s ctrl_len configed %d", __func__,
			pl022_spi_config->ctrl_len);
	}

	rc = of_property_read_u32(of_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		cam_info("%s wait_stateconfiged %d", __func__,
			pl022_spi_config->wait_state);
	}

	rc = of_property_read_u32(of_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		cam_info("%s duplex %d", __func__,
			pl022_spi_config->duplex);
	}

#if defined(CONFIG_ARCH_HI3630FPGA) || defined(CONFIG_ARCH_HI3630)
	rc = of_property_read_u32(of_node, "pl022,slave-tx-disable", &value);
	if (!rc) {
		pl022_spi_config->slave_tx_disable = value;
		cam_info("%s slave_tx_disable %d", __func__,
			pl022_spi_config->slave_tx_disable);
	}
#endif

	return 0;
}

static int m25p_spi_get_dt_data(struct device *dev,
	struct m25p_spi_plat_data *plat_data)
{
	int ret;
	unsigned int value;
	struct device_node *of_node = NULL;

	if (!dev || !dev->of_node || !plat_data) {
		cam_err("%s dev or of_node or plat_data is NULL", __func__);
		return -EINVAL;
	}
	of_node = dev->of_node;

	ret = m25p_parse_spi_config(plat_data, of_node);
	if (ret) {
		cam_err("%s parse spi config failed", __func__);
		return -1;
	}

	value = of_get_named_gpio(of_node, "m25p_spi,cs_gpio", 0);
	if (!gpio_is_valid(value)) {
		cam_err("%s get cs_gpio failed", __func__);
		return -1;
	}
	plat_data->spi_cs_gpio = value;
	cam_info("%s cs_gpio = %d", __func__, plat_data->spi_cs_gpio);

	ret = of_property_read_u32(of_node, "m25p_spi,chip_type", &value);
	if (ret < 0) {
		cam_err("%s get chip_type failed", __func__);
		return -1;
	}
	plat_data->chip_type = value;
	cam_info("%s chip_type = %d", __func__, plat_data->chip_type);

	return ret;
}

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int m25p_probe(struct spi_device *spi)
{
	struct m25p_spi_priv_data *drv_data = NULL;
	struct m25p_spi_plat_data *plat_data = NULL;
	uintptr_t addr;
	int ret;

	cam_info("%s enter", __func__);

	drv_data = kmalloc(sizeof(struct m25p_spi_priv_data), GFP_KERNEL);
	if (!drv_data) {
		cam_err("probe - can not alloc driver data");
		return -ENOMEM;
	}
	ret = memset_s(drv_data,
		sizeof(struct m25p_spi_priv_data),
		0,
		sizeof(struct m25p_spi_priv_data));
	if (ret != 0) {
		cam_err("memset failed %d", __LINE__);
		kfree(drv_data);
		return -EINVAL;
	}

	plat_data = &drv_data->plat_data;
	ret = m25p_spi_get_dt_data(&spi->dev, plat_data);
	if (ret < 0) {
		cam_err("%s failed to ice40_spi_get_dt_data", __func__);
		ret = -1;
		goto End;
	}

	addr = __get_free_pages(GFP_KERNEL | GFP_DMA, get_order(2 * PAGE_SIZE));
	drv_data->tx_buf  = (void *)addr;
	addr = __get_free_pages(GFP_KERNEL | GFP_DMA, get_order(2 * PAGE_SIZE));
	drv_data->rx_buf  = (void *)addr;
	cam_info("%s tx_buf=0x%pK rx_buf=0x%pK",
		__func__,
		drv_data->tx_buf, drv_data->rx_buf);
	if (!drv_data->tx_buf || !drv_data->rx_buf) {
		cam_err("%s can not alloc dma buf page", __func__);
		ret = -ENOMEM;
		goto End;
	}

	mutex_init(&drv_data->busy_lock);
	drv_data->spi = spi;
	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = M25P_SPI_SPEED_25M;
	spi->bits_per_word = WORD_8BITS;
	spi->controller_data = &plat_data->spidev0_chip_info;
	ret = spi_setup(spi);
	if (ret < 0) {
		cam_err("probe - setup spi error");
		ret = -1;
		goto End;
	}

	plat_data->pinctrl = devm_pinctrl_get_select(&spi->dev,
	PINCTRL_STATE_DEFAULT);
	if (!plat_data->pinctrl) {
		cam_err("%s failed to set pin", __func__);
		ret = -1;
		goto End;
	}

	ret = gpio_request(plat_data->spi_cs_gpio, "m25p_spi_cs");
	if (ret) {
		cam_err("probe - request spi cs gpio error");
		ret = -1;
		goto End;
	}
	gpio_direction_output(plat_data->spi_cs_gpio, 1);

	/* set driver_data to device */
	spi_set_drvdata(spi, drv_data);
	g_spi_drv_data = drv_data;

End:
	return ret;
}


static int m25p_remove(struct spi_device *spi)
{
	(void)spi;
	return 0;
}

static void m25p_shutdown(struct spi_device *spi)
{
	(void)spi;
}

static const struct of_device_id g_m25p_spi_dt_ids[] = {
	{ .compatible = "m25p80,spi" },
	{},
};

static struct spi_driver g_m25p80_driver = {
	.driver = {
		.name    = "m25p80",
		.owner    = THIS_MODULE,
		.of_match_table = g_m25p_spi_dt_ids,
	},
	.probe    = m25p_probe,
	.remove    = m25p_remove,
	.shutdown    = m25p_shutdown,

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};

module_spi_driver(g_m25p80_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mike Lavender");
MODULE_DESCRIPTION("MTD SPI driver for ST M25Pxx flash chips");
