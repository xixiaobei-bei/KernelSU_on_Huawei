/*
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
#ifndef __BOOT_STAGE_POINT_H__
#define __BOOT_STAGE_POINT_H__

#include <mntn_public_interface.h>
#include <linux/types.h>

#define FPGA 1
int is_bootanim_completed(void);
void set_boot_keypoint(u32 value);
u32 get_boot_keypoint(void);
u32 get_last_boot_keypoint(void);
#endif
