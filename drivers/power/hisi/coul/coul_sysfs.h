/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: sysfs process func for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_SYSFS_H_
#define _COUL_SYSFS_H_

#include "coul_core.h"

int get_pl_calibration_en(void);
int get_hand_chg_capacity_flag(void);
int get_input_capacity(void);
int coul_create_sysfs(void);
struct device *get_coul_dev_sysfs(void);

#endif
