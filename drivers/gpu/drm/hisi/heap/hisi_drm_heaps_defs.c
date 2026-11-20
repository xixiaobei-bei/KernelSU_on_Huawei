/*
 * hisi_drm_heaps_defs.c
 *
 * Heaps log manager, isolate from HISI DRM log manager.
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include "hisi_drm_heaps_defs.h"

#include <linux/moduleparam.h>

int hisi_drm_heaps_msg_level = 7;

module_param_named(hisi_drm_heaps_msg_level, hisi_drm_heaps_msg_level, int,
		   0640);

MODULE_PARM_DESC(hisi_drm_heaps_msg_level,
		 "change heaps log level, 4 err, 5 warn, 7 info, 8 debug");
