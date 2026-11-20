/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * aux grp header
 */

#ifndef AUX_EXTERN_H
#define AUX_EXTERN_H

#ifdef CONFIG_HW_RTG_SCHED
#include "linux/hisi_rtg.h"
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
#include "mtkrtg/mtk_rtg_sched.h"
#endif
#include "hwrtg/aux_info.h"

int sched_rtg_aux(int tid, int enable, const struct aux_info *info);
int set_aux_boost_util(int util);

#endif
