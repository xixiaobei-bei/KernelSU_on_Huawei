/*
 * soundtrigger_dma_drv_interface.h
 *
 * soundtrigger_dma_drv_interface is a kernel driver interface which is used to manager dma
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

#ifndef __SOUNDTRIGGER_DMA_DRV_INTERFACE_H__
#define __SOUNDTRIGGER_DMA_DRV_INTERFACE_H__

#include "soundtrigger_dma_drv_info.h"

struct soundtrigger_dma_ops {
	int32_t (*probe_callback)(uint32_t channel_num);
	void (*open_callback)(struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*get_dma_cfg)(struct dma_config *cfg, enum codec_dsp_type dsp_type,
			uint32_t pcm_channel, uint32_t offset);
	void (*get_pcm_cfg)(struct soundtrigger_pcm_config *cfg,
			enum codec_dsp_type dsp_type, uint32_t pcm_channel);
	int32_t (*start_dma)(struct soundtrigger_dma_drv_info *dma_drv_info);
	int32_t (*check_fast_complete_flag)(const struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*set_dma_int_flag)(struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*dmac_irq_handle)(struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*proc_fast_data)(struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*proc_fast_trans_buff)(struct fast_tran_info *fast_info,
			struct soundtrigger_pcm_info *pcm_info,
			struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*proc_normal_data)(struct soundtrigger_dma_drv_info *dma_drv_info);
	ssize_t (*read_data)(struct fast_tran_info *fast_info_left,
			struct fast_tran_info *fast_info_right,
			char __user *buffer, size_t max_read_len);
	void (*dump_dma_info)(const struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*deinit_dma)(struct soundtrigger_dma_drv_info *dma_drv_info);
	int32_t (*deinit_dma_info)(struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*close_codec_dma)(void);
	void (*stop_dma)(const struct soundtrigger_dma_drv_info *dma_drv_info);
	void (*dma_drv_remove)(void);
};

#endif /* __SOUNDTRIGGER_DMA_DRV_INTERFACE_H__ */
