/*
 * soundtrigger_dma_drv_third_codec.h
 *
 * soundtrigger_dma_drv_third_codec is a kernel driver specific third_codec which is used to manager dma
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef __SOUNDTRIGGER_DMA_DRV_THIRD_CODEC_H__
#define __SOUNDTRIGGER_DMA_DRV_THIRD_CODEC_H__

#include "soundtrigger_dma_drv_interface.h"

struct soundtrigger_dma_ops *get_third_codec_dma_ops(void);

#endif /* __SOUNDTRIGGER_DMA_DRV_THIRD_CODEC_H__ */
