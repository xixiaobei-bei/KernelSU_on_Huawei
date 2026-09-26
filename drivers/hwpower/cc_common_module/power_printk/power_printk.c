// SPDX-License-Identifier: GPL-2.0
/*
 * power_printk.c
 *
 * printk interface for power module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <linux/slab.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_printk
HWLOG_REGIST();

#define POWER_PRINTK_U8_STR_LEN         8

void power_print_u8_array(const char *name, const u8 *array, unsigned int row, unsigned int col)
{
	int i, j;
	int str_len;
	char *print_str = NULL;
	char tmp_str[POWER_PRINTK_U8_STR_LEN] = {0};

	if (!name || !array)
		return;

	str_len = row * col * POWER_PRINTK_U8_STR_LEN * sizeof(char);
	print_str = kzalloc(str_len, GFP_KERNEL);
	if (!print_str)
		return;

	for (i = 0; i < row; i++) {
		for (j = 0; j < col; j++) {
			snprintf(tmp_str, POWER_PRINTK_U8_STR_LEN, "0x%02x ", array[i * col + j]);
			strncat(print_str, tmp_str, strlen(tmp_str));
		}
		hwlog_info("%s[%d]: %s\n", name, i, print_str);
		memset(print_str, 0, str_len);
	}

	kfree(print_str);
}
