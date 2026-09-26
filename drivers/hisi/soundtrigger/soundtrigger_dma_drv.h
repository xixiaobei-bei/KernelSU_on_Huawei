/*
 * soundtrigger_dma_drv.h
 *
 * soundtrigger_dma_drv is a kernel driver which is used to manager dma
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
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

#ifndef __SOUNDTRIGGER_DMA_DRV_H__
#define __SOUNDTRIGGER_DMA_DRV_H__

#include "soundtrigger_dma_drv_info.h"

int32_t soundtrigger_set_codec_type(enum codec_dsp_type type);
void da_combine_soundtrigger_dma_close(void);


#endif /* __SOUNDTRIGGER_DMA_DRV_H__ */

