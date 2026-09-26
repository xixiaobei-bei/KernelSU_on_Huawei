/*
 * crc16.h
 *
 * code for external sensorhub channel driver
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

#ifndef HW_CRC16
#define HW_CRC16

static const unsigned short crc_16_seed = 0xffff;
static const unsigned short crc_16_ok = 0x0F47;
static const unsigned short crc_16_ok_neg = 0xF0B8;

#define crc_16_init(xx_crc) ((xx_crc) = crc_16_seed)

#define crc_16_step(xx_crc, xx_c) \
	(((xx_crc) << 8) ^ crc_16_table[(((xx_crc) >> 8) ^ (xx_c)) & 0x00ff])

#define crc_16_finish(xx_crc) ((xx_crc) ^= crc_16_seed)

extern const unsigned short crc_16_table[]; /* Extern for macro (global) */

extern unsigned short crc16_calc(const unsigned char *buffer, int len);

#define crc16_verify(buffer, len) (crc_16_ok == crc16_calc((buffer), (len)))

#endif /* HW_CRC16 */
