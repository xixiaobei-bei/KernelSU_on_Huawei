/* Copyright (c) 2021-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef DPU_DISPLAYENGINE_FINGERPRINT_UTILS_H
#define DPU_DISPLAYENGINE_FINGERPRINT_UTILS_H

#include "hisi_fb.h"

u32 display_engine_local_hbm_get_support(void);
int display_engine_local_hbm_set(int grayscale);

#endif /* DPU_DISPLAYENGINE_FINGERPRINT_UTILS_H */
