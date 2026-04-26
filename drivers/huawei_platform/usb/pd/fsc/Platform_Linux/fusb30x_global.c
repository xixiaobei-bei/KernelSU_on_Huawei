/*
 * fusb30x_global.c
 *
 * fusb30x_global driver
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

#include "fusb30x_global.h"

struct fusb30x_chip* g_chip = NULL;  // Our driver's relevant data

struct fusb30x_chip* fusb30x_GetChip(void)
{
    return g_chip;      // return a pointer to our structs
}

void fusb30x_SetChip(struct fusb30x_chip* newChip)
{
    g_chip = newChip;   // assign the pointer to our struct
}