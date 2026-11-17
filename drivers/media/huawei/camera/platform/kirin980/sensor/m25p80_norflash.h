/*
 * m25p80_norflash.h
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

#ifndef _M25P80_NORFLASH_H_
#define _M25P80_NORFLASH_H_
int m25p_set_spi_cs_value(u8 val);
int m25p_get_array_part_content(u32 type, void *user_addr, unsigned long size);
int m25p_set_array_part_content(u32 type, void *user_addr, unsigned long size);
int fast_read_nbyte(u32 addr, u8 *rd_buf, int byte_num);
#endif
