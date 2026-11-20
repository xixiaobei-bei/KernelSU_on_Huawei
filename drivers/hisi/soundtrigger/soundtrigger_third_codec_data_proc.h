/*
 * soundtrigger_third_codec_data_proc.h
 *
 * soundtrigger_third_codec_data_proc process third_codec msbc data.
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

#ifndef __SOUNDTRIGGER_THIRD_CODEC_DATA_PROC_H__
#define __SOUNDTRIGGER_THIRD_CODEC_DATA_PROC_H__

#include <linux/types.h>

#define DMA_THIRD_CODEC_TRAN_COUNT 2

struct soundtrigger_third_codec_fast_tran_info {
	uint32_t *fast_frame_find_flag;
	uint32_t *fast_start_addr;
	int32_t *dma_tran_count;
	uint16_t fast_head_frame_word;
	uint8_t *fast_buffer;
};

ssize_t soundtrigger_third_codec_read_msbc_fast_data(const uint8_t *in_left_buf,
	const uint8_t *in_right_buf, uint32_t in_buf_cnt, char __user *out_user_buf);
void soundtrigger_third_codec_trans_dma_fast_buf(struct soundtrigger_third_codec_fast_tran_info *fast_info_third_codec,
	const uint32_t *pcm_buf, uint32_t pcm_cnt);

#endif /* __SOUNDTRIGGER_THIRD_CODEC_DATA_PROC_H__ */
