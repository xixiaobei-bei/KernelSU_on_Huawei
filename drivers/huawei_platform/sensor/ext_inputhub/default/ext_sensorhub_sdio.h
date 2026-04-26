/*
 * ext_sensorhub_sdio.h
 *
 * head file for external sensorhub sdio driver
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

#ifndef EXT_SENSORHUB_SDIO_H
#define EXT_SENSORHUB_SDIO_H

#include <linux/types.h>

#define HISDIO_BLOCK_SIZE	                512
#define HISDIO_VENDOR_ID_HISI			0x12D1	  /* VENDOR ID */
#define HISDIO_PRODUCT_ID_HISI			0x534A	  /* Product 1135 */

void ext_sensorhub_detect_sdio_card(void);

int ext_sensorhub_sdio_func_probe(void);

void ext_sensorhub_sdio_exit(void);

void dw_mci_1135_card_detect_change(void);

int ext_sensorhub_mmc_sdio_dev_write(u8 *buf, u32 len);

int ext_sensorhub_mmc_sdio_dev_read(u8 *buf, u32 len);

int ext_sensorhub_mmc_sdio_get_xfercount(u32 *xfercount);

#endif /* EXT_SENSORHUB_SDIO_H */
