/*
 * ext_sensorhub_spi.h
 *
 * head file for external sensorhub spi driver
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
#ifndef EXT_SENSORHUB_SPI_H
#define EXT_SENSORHUB_SPI_H

#include <linux/amba/pl022.h>
#include <linux/spi/spi.h>

#define SPI_SPEED_DEFAULT (20 * 1024 * 1024)
#define GPIO_HIGH 1
#define EXT_SENSORHUB_SPI_DEVICE_NAME "ext_sensorhub"

struct spi_config {
	u32 max_speed_hz;
	u16 mode;
	u8 bits_per_word;
	u8 bus_id;
	struct pl022_config_chip pl022_spi_config;
	int cs_gpio;
	struct spi_device *spi_dev;
	int sched_priority;
};

int ext_sensorhub_spi_init(void);

void ext_sensorhub_spi_exit(void);

int ext_sensorhub_spi_write(u8 *buf, u32 len);

int ext_sensorhub_spi_read(u8 *buf, u32 len);

int ext_sensorhub_spi_update(u8 *buf, u32 len, u32 outLen, u8 *txBuf);

#endif /* EXT_SENSORHUB_SPI_H */
