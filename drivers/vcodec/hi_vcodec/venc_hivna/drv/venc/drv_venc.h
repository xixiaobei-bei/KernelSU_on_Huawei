/*
 * drv_venc.h
 *
 * This is for venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DRV_VENC_H__
#define __DRV_VENC_H__

#include "hi_type.h"

#ifdef SUPPORT_VENC_FREQ_CHANGE
extern uint32_t g_venc_freq;
extern struct mutex g_venc_freq_mutex;
#endif

void venc_init_ops(void);

#endif

