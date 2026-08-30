/*
 * A driver for the camera otp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _M25P80_NORFLASH_H_
#define _M25P80_NORFLASH_H_


int m25p_set_spi_cs_value(u8 val);
int m25p_get_array_part_content(u32 type, void *userAddr, unsigned long size);
int m25p_set_array_part_content(u32 type, void *userAddr, unsigned long size);
#endif
