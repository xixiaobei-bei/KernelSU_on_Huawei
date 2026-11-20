/*
 * soundtrigger_third_codec_data_proc.c
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

#include "soundtrigger_third_codec_data_proc.h"

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/hisi/audio_log.h>

#include "soundtrigger_dma_drv_info.h"

#define LOG_TAG "soundtrigger_third_codec"

#define BYTE_BITS 8
#define SHORT_BITS 16
#define HIGH_24_BITS 24
#define HIGH_24BIT_BYTES 3
#define FRAME_LENGTH 1920
#define INT_BITS 32
#define VALID_FRAME_SIZE 6
#define FRAME_HEAD_SIZE 2
#define BYTE_COUNT_EACH_SAMPLE_POINT 4

#define VALID_FRAME_LENGTH (FRAME_LENGTH * HIGH_24_BITS / INT_BITS)
#define VALID_BUFFER_LENGTH (VALID_FRAME_LENGTH * VALID_FRAME_SIZE)

static void process_msbc_fast_data(const uint8_t *in_left_buf,
	const uint8_t *in_right_buf, uint32_t in_buf_cnt, uint8_t *out_buf)
{
	uint32_t loop_count;
	uint32_t i;

	if (in_buf_cnt < VALID_BUFFER_LENGTH) {
		AUDIO_LOGE("in buf size is invalid, in buf cnt: %u", in_buf_cnt);
		return;
	}

	loop_count = VALID_BUFFER_LENGTH / VALID_FRAME_LENGTH;

	/*
	 * Process frame head. The data in the left channel contains the frame header 5A5A,
	 * and the data in the right channel does not contain the frame header 5A5A
	 */
	in_right_buf += FRAME_HEAD_SIZE;
	memcpy(out_buf, in_left_buf, VALID_FRAME_LENGTH);
	out_buf += VALID_FRAME_LENGTH;
	in_left_buf += VALID_FRAME_LENGTH;
	memcpy(out_buf, in_right_buf, VALID_FRAME_LENGTH - FRAME_HEAD_SIZE);
	out_buf += VALID_FRAME_LENGTH - FRAME_HEAD_SIZE;
	in_right_buf += VALID_FRAME_LENGTH - FRAME_HEAD_SIZE;

	for (i = 1; i < loop_count; i++) {
		memcpy(out_buf, in_left_buf, VALID_FRAME_LENGTH);
		out_buf += VALID_FRAME_LENGTH;
		in_left_buf += VALID_FRAME_LENGTH;
		memcpy(out_buf, in_right_buf, VALID_FRAME_LENGTH);
		out_buf += VALID_FRAME_LENGTH;
		in_right_buf += VALID_FRAME_LENGTH;
	}
}

ssize_t soundtrigger_third_codec_read_msbc_fast_data(const uint8_t *in_left_buf,
	const uint8_t *in_right_buf, uint32_t in_buf_cnt, char __user *out_user_buf)
{
	uint32_t remain_bytes;
	uint8_t *real_data_buf = NULL;

	if (!out_user_buf) {
		AUDIO_LOGE("out user buf is null");
		return -EINVAL;
	}

	if (!in_left_buf || !in_right_buf) {
		AUDIO_LOGE("left buffer or right buffer is null");
		return -EINVAL;
	}

	real_data_buf = (uint8_t *)kzalloc(VALID_BUFFER_LENGTH * DMA_THIRD_CODEC_TRAN_COUNT, GFP_KERNEL);
	if (!real_data_buf) {
		AUDIO_LOGE("kzalloc real data buffer fail");
		return -ENOMEM;
	}

	process_msbc_fast_data(in_left_buf, in_right_buf, in_buf_cnt, real_data_buf);
	remain_bytes = copy_to_user(out_user_buf, real_data_buf, VALID_BUFFER_LENGTH * DMA_THIRD_CODEC_TRAN_COUNT);
	kzfree(real_data_buf);

	if (remain_bytes != 0) {
		AUDIO_LOGE("msbc data copy to user failed, rest bytes: %u", remain_bytes);
		return -EINVAL;
	}

	return 0;
}

static void get_valid_24p_from_24la(const uint32_t *in_buf, uint32_t in_cnt,
	uint8_t *out_buf, uint32_t *out_cnt)
{
	uint32_t val;
	bool find_valid_head = false;
	uint32_t valid_cnt = 0;

	while (in_cnt--) {
		val = *in_buf++ >> BYTE_BITS;
		if (!find_valid_head) {
			if (val == 0)
				continue;
			else
				find_valid_head = true;
		}
		*out_buf++ = val;
		*out_buf++ = val >> BYTE_BITS;
		*out_buf++ = val >> SHORT_BITS;
		valid_cnt++;
	}

	*out_cnt = valid_cnt;
}

static bool find_third_codec_msbc_head_frame_word(const uint16_t *const in_buf,
	uint32_t input_length, uint16_t fast_head_frame_word)
{
	uint32_t i;

	for (i = 0; i < input_length; i++) {
		if (in_buf[i] == fast_head_frame_word) {
			AUDIO_LOGI("msbc data have fast head frame, i: %u, frame word: 0x%x, head frame data: 0x%x",
				i, fast_head_frame_word, in_buf[i]);
			return true;
		}
	}

	return false;
}

void soundtrigger_third_codec_trans_dma_fast_buf(struct soundtrigger_third_codec_fast_tran_info *const fast_info_third_codec,
	const uint32_t *pcm_buf, uint32_t pcm_size)
{
	uint8_t *valid_buf = NULL;
	uint32_t frame_size;
	uint32_t valid_cnt = 0;

	if (!fast_info_third_codec) {
		AUDIO_LOGE("fast info third_codec is null");
		return;
	}

	if (pcm_size > FAST_FRAME_LEN * BYTE_COUNT_EACH_SAMPLE_POINT &&
		pcm_size % sizeof(uint32_t) != 0) {
		AUDIO_LOGE("pcm size is invalid, pcm size: %u", pcm_size);
		return;
	}

	valid_buf = (uint8_t *)kzalloc(pcm_size, GFP_KERNEL);
	if (!valid_buf) {
		AUDIO_LOGE("valid buffer kzalloc failed");
		return;
	}

	get_valid_24p_from_24la(pcm_buf, pcm_size / sizeof(uint32_t), valid_buf, &valid_cnt);
	frame_size = valid_cnt * HIGH_24BIT_BYTES;
	AUDIO_LOGI("valid cnt is %u", valid_cnt);

	if (*fast_info_third_codec->fast_frame_find_flag == FRAME_NOT_FIND) {
		if (find_third_codec_msbc_head_frame_word((uint16_t *)valid_buf,
			pcm_size / sizeof(uint16_t), fast_info_third_codec->fast_head_frame_word)) {
			memcpy(fast_info_third_codec->fast_buffer + HIGH_24BIT_BYTES * (*fast_info_third_codec->fast_start_addr),
				valid_buf, frame_size);
			*fast_info_third_codec->fast_frame_find_flag = FRAME_FIND;
			*fast_info_third_codec->fast_start_addr += valid_cnt;
			(*fast_info_third_codec->dma_tran_count)++;
		}
	} else {
		memcpy(fast_info_third_codec->fast_buffer + HIGH_24BIT_BYTES * (*fast_info_third_codec->fast_start_addr),
			valid_buf, frame_size);
		*fast_info_third_codec->fast_start_addr += valid_cnt;
		(*fast_info_third_codec->dma_tran_count)++;
	}

	kzfree(valid_buf);
}

