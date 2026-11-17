/*
 * soundtrigger_dma_drv_comm.h
 *
 * soundtrigger_dma_drv_comm is a kernel driver common operation which is used to manager dma
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

#ifndef __SOUNDTRIGGER_DMA_DRV_COMM_H__
#define __SOUNDTRIGGER_DMA_DRV_COMM_H__

#include "soundtrigger_dma_drv_info.h"

void stop_dma(int32_t pcm_channel, const struct soundtrigger_dma_drv_info *dma_drv_info);
void dump_dma_addr_info(const struct soundtrigger_dma_drv_info *dma_drv_info);
int32_t slimbus_activate(struct soundtrigger_dma_drv_info *dma_drv_info);
int32_t slimbus_deactivate(struct soundtrigger_dma_drv_info *dma_drv_info);

#endif
