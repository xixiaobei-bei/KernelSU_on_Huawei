/*
 * drv_venc_dpm.h
 *
 * This is for venc dpm.
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

#ifndef __DRV_VENC_DPM_H__
#define __DRV_VENC_DPM_H__

#include "venc_regulator.h"

void venc_dpm_init(struct venc_config *venc_config);
void venc_dpm_deinit(void);
void venc_dpm_freq_select(venc_clk_t clk_type);

#endif

