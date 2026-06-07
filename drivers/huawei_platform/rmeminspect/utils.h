/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: ddr inspect record bad mm head file
 * Author: zhouyubin
 * Create: 2019-05-30
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef UTILS__H
#define UTILS__H

#include "phys_mem.h"
#define FIRST_INDEX  1
#define SECOND_INDEX 2
#define THIRD_INDEX  3
#define FORTH_INDEX  4

extern unsigned long virt_to_phy_u(unsigned long vaddr);
extern int write_hisi_nve_ddrfault(struct user_nve_info *request);
extern int ddr_bbox_diaginfo(const struct user_nve_info *request);

#endif
