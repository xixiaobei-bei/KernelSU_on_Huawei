/*
 * soundtrigger_dma_drv_da_combine.c
 *
 * soundtrigger_dma_drv_da_combine is a kernel driver which is used to manager da_combine dma
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

#include "soundtrigger_dma_drv_da_combine.h"

#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/hisi/audio_log.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>

#include "slimbus.h"
#include "slimbus_da_combine_v5.h"
#include "dsp_misc.h"
#include "dsp_utils.h"
#include "asp_dma.h"
#include "soundtrigger_event.h"
#include "soundtrigger_socdsp_mailbox.h"
#include "soundtrigger_socdsp_pcm.h"
#include "soundtrigger_ring_buffer.h"
#include "soundtrigger_dma_drv_comm.h"


#define LOG_TAG "soundtrigger_da_combine"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

static const struct dma_config g_da_combine_v3_st_dma_fast_cfg[STEREO_DMA_PORT_NUM] = {
	{
		.port = DA_COMBINE_V3_FAST_L_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V3_FAST_L_DMA_CONF_NUM,
		.channel = DMA_FAST_LEFT_CH_NUM
	},
	{
		.port = DA_COMBINE_V3_FAST_R_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V3_FAST_R_DMA_CONF_NUM,
		.channel = DMA_FAST_RIGHT_CH_NUM
	}
};

static const struct dma_config g_da_combine_v3_st_dma_normal_cfg[STEREO_DMA_PORT_NUM] = {
	{
		.port = DA_COMBINE_V3_NORMAL_L_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V3_NORMAL_L_DMA_CONF_NUM,
		.channel = DMA_NORMAL_LEFT_CH_NUM
	},
	{
		.port = DA_COMBINE_V3_NORMAL_R_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V3_NORMAL_R_DMA_CONF_NUM,
		.channel = DMA_NORMAL_RIGHT_CH_NUM
	}
};

#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
static const struct dma_config g_da_combine_v5_st_dma_fast_cfg[STEREO_DMA_PORT_NUM] = {
	{
		.port = DA_COMBINE_V5_FAST_L_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V5_FAST_L_DMA_CONF_NUM,
		.channel = DMA_FAST_LEFT_CH_NUM
	},
	{
		.port = 0,
		.config = 0,
		.channel = 0
	}
};

static const struct dma_config g_da_combine_v5_st_dma_normal_cfg[STEREO_DMA_PORT_NUM] = {
	{
		.port = DA_COMBINE_V5_NORMAL_L_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V5_NORMAL_L_DMA_CONF_NUM,
		.channel = DMA_NORMAL_LEFT_CH_NUM
	},
	{
		.port = DA_COMBINE_V5_NORMAL_R_SLMIMBUS_PORT_ADDR,
		.config = DA_COMBINE_V5_NORMAL_R_DMA_CONF_NUM,
		.channel = DMA_NORMAL_RIGHT_CH_NUM
	}
};
#endif

static const struct dma_config *g_st_dma_cfg[CODEC_DA_COMBINE_MAX][PCM_CHL_NUM] = {
	{ g_da_combine_v3_st_dma_fast_cfg, g_da_combine_v3_st_dma_normal_cfg },
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	{ g_da_combine_v5_st_dma_fast_cfg, g_da_combine_v5_st_dma_normal_cfg },
#endif
};

static const struct soundtrigger_pcm_config g_pcm_cfg[CODEC_DA_COMBINE_MAX][PCM_CHL_NUM] = {
	{
		{
			.channels = DMA_PORT_NUM,
			.rate = FAST_TRAN_RATE,
			.frame_len = FAST_FRAME_LEN,
			.byte_count = BYTE_COUNT_EACH_SAMPLE_POINT
		},
		{
			.channels = DMA_PORT_NUM,
			.rate = DA_COMBINE_V3_NORMAL_TRAN_RATE,
			.frame_len = DA_COMBINE_V3_NORMAL_FRAME_LEN,
			.byte_count = BYTE_COUNT_EACH_SAMPLE_POINT
		},
	},
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	{
		{
			.channels = DMA_PORT_NUM,
			.rate = FAST_TRAN_RATE,
			.frame_len = FAST_FRAME_LEN,
			.byte_count = BYTE_COUNT_EACH_SAMPLE_POINT
		},
		{
			.channels = DMA_PORT_NUM,
			.rate = DA_COMBINE_V5_NORMAL_TRAN_RATE,
			.frame_len = DA_COMBINE_V5_NORMAL_FRAME_LEN,
			.byte_count = BYTE_COUNT_EACH_SAMPLE_POINT
		},
	},
#endif
};


static void check_frame_head(uint32_t pcm_index, uint32_t check_length,
	uint32_t index, struct soundtrigger_dma_drv_info *dma_drv_info)
{
	struct fast_tran_info *fast_info = &dma_drv_info->fast_tran_info_left;
	struct normal_tran_info *normal_info = &dma_drv_info->normal_tran_info;

	if (index != (check_length - 1))
		return;

	if ((pcm_index == PCM_FAST) && (fast_info->fast_head_word_count == check_length)) {
		fast_info->fast_head_word_count = 0;
		fast_info->fast_frame_find_flag = FRAME_FIND;
		fast_info->fast_start_addr = 0;
		AUDIO_LOGI("fast channel find head, full head word");
	}

	if ((pcm_index == PCM_NORMAL) &&
		(normal_info->normal_head_word_count == RINGBUF_FRAME_SIZE)) {
		normal_info->normal_head_word_count = 0;
		normal_info->normal_frame_find_flag = FRAME_FIND;
		normal_info->normal_start_addr = 0;
		AUDIO_LOGI("normal channel find head, full head word");
	}
}

static bool find_pcm_fast_start_addr(const uint16_t *input_buffer, uint32_t input_length,
	uint32_t *start_addr, struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t index;
	struct fast_tran_info *fast_info = &dma_drv_info->fast_tran_info_left;

	if ((input_buffer[0] != fast_info->fast_head_frame_word) &&
		(input_buffer[input_length - 1] != fast_info->fast_head_frame_word)) {
		AUDIO_LOGW("don't have fast head frame");
		AUDIO_LOGW("frame word: 0x%x, first data: 0x%x, last data: 0x%x",
			fast_info->fast_head_frame_word, input_buffer[0],
			input_buffer[input_length - 1]);
		return false;
	}

	for (index = 0; index < input_length; index++) {
		if (input_buffer[index] == fast_info->fast_head_frame_word) {
			fast_info->fast_head_word_count++;
			check_frame_head(PCM_FAST, input_length, index, dma_drv_info);
		} else {
			/*
			 * when half of head frame count is 0x5a5a,
			 * the start addr is found
			 */
			if (fast_info->fast_head_word_count >
				(input_length * FAST_HEAD_FRAME_COUNT_FULL_MAGIC / 2)) {
				*start_addr = index;
				fast_info->fast_head_word_count = 0;
				AUDIO_LOGI("fast channel find head, index: %u", index);
				return true;
			}
		}
	}

	return false;
}

static uint16_t *alloc_temp_buffer(enum codec_dsp_type codec_type)
{
	uint16_t *temp_buf = NULL;

	if (codec_type == CODEC_DA_COMBINE_V3)
		temp_buf = kzalloc(DA_COMBINE_V3_NORMAL_FRAME_SIZE, GFP_KERNEL);
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	else if (codec_type == CODEC_DA_COMBINE_V5)
		temp_buf = kzalloc(DA_COMBINE_V5_NORMAL_FRAME_SIZE, GFP_KERNEL);
#endif
	else
		temp_buf = NULL;

	return temp_buf;
}

static int32_t alloc_normal_buffer(struct soundtrigger_pcm_info *pcm_info,
	enum codec_dsp_type dsp_type, uint32_t **left_buf, uint16_t **temp_buf, uint16_t **ring_buf)
{
	uint32_t *left_buffer = NULL;
	uint16_t *temp_buffer = NULL;
	uint16_t *ring_buffer = NULL;

	if (pcm_info == NULL)
		return -EINVAL;

	left_buffer = kzalloc(pcm_info->buffer_size, GFP_KERNEL);
	if (left_buffer == NULL) {
		AUDIO_LOGE("normal left buffer kzalloc failed");
		return -ENOMEM;
	}

	temp_buffer = alloc_temp_buffer(dsp_type);
	if (temp_buffer == NULL) {
		kzfree(left_buffer);
		AUDIO_LOGE("normal temp buf kzalloc failed");
		return -ENOMEM;
	}

	ring_buffer = kzalloc(RINGBUF_FRAME_SIZE, GFP_KERNEL);
	if (ring_buffer == NULL) {
		kzfree(temp_buffer);
		kzfree(left_buffer);
		AUDIO_LOGE("normal ring buf kzalloc failed");
		return -ENOMEM;
	}

	*left_buf = left_buffer;
	*temp_buf = temp_buffer;
	*ring_buf = ring_buffer;

	return 0;
}

static void dump_chl_reg_info(void)
{
	AUDIO_LOGI("normal config: %u, fast config: %u",
		dmac_reg_read(ASP_DMA_CX_CONFIG(DMA_NORMAL_LEFT_CH_NUM)),
		dmac_reg_read(ASP_DMA_CX_CONFIG(DMA_FAST_LEFT_CH_NUM))),
	AUDIO_LOGI("fast src: %u, fast dst: %u, fast size: %u",
		dmac_reg_read(ASP_DMA_CX_SRC_ADDR(DMA_FAST_LEFT_CH_NUM)),
		dmac_reg_read(ASP_DMA_CX_DES_ADDR(DMA_FAST_LEFT_CH_NUM)),
		dmac_reg_read(ASP_DMA_CX_CNT0(DMA_FAST_LEFT_CH_NUM)));
}

static bool find_pcm_normal_start_addr(const uint16_t *input_buffer, uint32_t input_length,
	uint32_t *start_addr, struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t index;
	struct normal_tran_info *normal_info = &(dma_drv_info->normal_tran_info);

	if ((input_buffer[0] != normal_info->normal_head_frame_word) &&
		(input_buffer[input_length - 1] != normal_info->normal_head_frame_word))
		return false;

	for (index = 0; index < input_length; index++) {
		if (input_buffer[index] == normal_info->normal_head_frame_word) {
			normal_info->normal_head_word_count++;
			check_frame_head(PCM_NORMAL, input_length, index, dma_drv_info);
		} else {
			/*
			 * when half of head frame count is 0x5a5a,
			 * the start addr is found
			 */
			if (normal_info->normal_head_word_count >
				(input_length * NORMAL_HEAD_FRAME_COUNT_FULL_MAGIC / 2)) {
				*start_addr = index;
				normal_info->normal_head_word_count = 0;
				dump_chl_reg_info();
				dump_dma_addr_info(dma_drv_info);
				AUDIO_LOGI("normal channel find head, index: %u", index);
				return true;
			}

			normal_info->normal_head_word_count = 0;
		}
	}

	return false;
}

static void get_pcm_valid_data(const uint32_t *input_buffer, uint16_t *output_buffer,
	int32_t frame_count)
{
	int32_t count;

	for (count = 0; count < frame_count; count++)
		output_buffer[count] = input_buffer[count] >> 16;
}

static void pcm_mono_48k_to_16k(const uint16_t *input_buffer, uint16_t *output_buffer,
	int32_t output_len)
{
	int32_t count;

	for (count = 0; count < output_len; count++)
		output_buffer[count] = input_buffer[SAMPLE_RATE_44_TO_16 * count];
}

static void dma_normal_trans_buff(struct normal_tran_info *normal_info,
	struct soundtrigger_pcm_info *pcm_info, enum codec_dsp_type dsp_type,
	struct soundtrigger_dma_drv_info *dma_drv_info)
{
	struct ring_buffer_itef *normal_rb = NULL;
	uint32_t *left_buf = NULL;
	uint16_t *temp_buf = NULL;
	uint16_t *ring_buf = NULL;
	uint32_t start_addr = 0;
	uint32_t buf_index;
	int32_t ret;

	normal_rb = normal_info->normal_buffer;
	if (normal_rb == NULL) {
		AUDIO_LOGE("normal ring buffer is null");
		return;
	}

	ret = alloc_normal_buffer(pcm_info, dsp_type, &left_buf, &temp_buf, &ring_buf);
	if (ret != 0)
		return;

	buf_index = normal_info->read_count_left % PCM_SWAP_BUFFER_NUM;
	memcpy(left_buf, (uint32_t *)pcm_info->buffer[0][buf_index], pcm_info->buffer_size);

	normal_info->read_count_left++;

	get_pcm_valid_data(left_buf, temp_buf,
		pcm_info->buffer_size / BYTE_COUNT_EACH_SAMPLE_POINT);
	pcm_mono_48k_to_16k(temp_buf, ring_buf, RINGBUF_FRAME_LEN);

	if (normal_info->normal_frame_find_flag == FRAME_NOT_FIND) {
		if (find_pcm_normal_start_addr(ring_buf,
			RINGBUF_FRAME_LEN, &start_addr, dma_drv_info)) {
			normal_rb->put(normal_rb, ring_buf, RINGBUF_FRAME_SIZE);
			normal_info->normal_frame_find_flag = FRAME_FIND;
			normal_info->normal_start_addr = start_addr;
			normal_info->normal_tran_count++;
		}
	} else {
		normal_rb->put(normal_rb, ring_buf, RINGBUF_FRAME_SIZE);
		normal_info->normal_tran_count++;
	}

	if (normal_info->read_count_left - normal_info->normal_tran_count >=
		NORMAL_CHANNEL_TIMEOUT_READ_COUNT) {
		AUDIO_LOGE("dma normal channel timeout");

		stop_dma(PCM_FAST, dma_drv_info);
		stop_dma(PCM_NORMAL, dma_drv_info);
	}

	kzfree(ring_buf);
	kzfree(temp_buf);
	kzfree(left_buf);
}

static int32_t slimbus_reg_read(struct soundtrigger_dma_drv_info *dma_drv_info,
	uint32_t reg)
{
	unsigned long flag = 0;
	int32_t ret;

	if (hwspin_lock_timeout_irqsave(dma_drv_info->hwlock, HWLOCK_WAIT_TIME, &flag)) {
		AUDIO_LOGE("hwspinlock timeout");
		return 0;
	}

	ret = readl(dma_drv_info->slimbus_base_reg_addr + reg);

	hwspin_unlock_irqrestore(dma_drv_info->hwlock, &flag);

	return ret;
}

static void dump_dma_slimbus_reg_info(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	AUDIO_LOGE("fast config: %x, normal config: %x",
		dmac_reg_read(ASP_DMA_CX_CONFIG(DMA_FAST_LEFT_CH_NUM)),
		dmac_reg_read(ASP_DMA_CX_CONFIG(DMA_NORMAL_LEFT_CH_NUM)));

	AUDIO_LOGE("PORT6_REG_0: %x, PORT6_REG_1: %x, PORT10_REG_0: %x, PORT10_REG_1: %x",
		slimbus_reg_read(dma_drv_info, SLIMBUS_PORT6_REG_0),
		slimbus_reg_read(dma_drv_info, SLIMBUS_PORT6_REG_1),
		slimbus_reg_read(dma_drv_info, SLIMBUS_PORT10_REG_0),
		slimbus_reg_read(dma_drv_info, SLIMBUS_PORT10_REG_1));
}

static void set_da_combine_dma_int_flag(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	if (dma_drv_info->dma_int_fast_left_flag == 0 ||
		dma_drv_info->dma_int_nomal_flag == 0) {
		dump_dma_slimbus_reg_info(dma_drv_info);
		dma_drv_info->dma_int_fast_left_flag = 1;
		dma_drv_info->dma_int_nomal_flag = 1;
	}
}

static void dmac_right_fast_irq_handle(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	(dma_drv_info->fast_tran_info_right.irq_count)++;
}

static void proc_dma_left_fast_trans_buff(struct fast_tran_info *fast_info,
	struct soundtrigger_pcm_info *pcm_info, struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t *pcm_buf = NULL;
	uint32_t buf_index;
	uint32_t frame_size;
	uint32_t start_addr = 0;

	pcm_buf = kzalloc(pcm_info->buffer_size, GFP_KERNEL);
	if (pcm_buf == NULL) {
		AUDIO_LOGE("pcm buffer kzalloc failed");
		return;
	}

	buf_index = fast_info->read_count % PCM_SWAP_BUFFER_NUM;
	memcpy(pcm_buf, (uint32_t *)pcm_info->buffer[0][buf_index], pcm_info->buffer_size);
	fast_info->read_count++;

	frame_size = FAST_FRAME_SIZE / sizeof(uint16_t) * fast_info->dma_tran_count;
	if (fast_info->fast_frame_find_flag == FRAME_NOT_FIND) {
		if (find_pcm_fast_start_addr((uint16_t *)pcm_buf,
			pcm_info->buffer_size / sizeof(uint16_t), &start_addr, dma_drv_info)) {
			fast_info->fast_start_addr = start_addr;
			memcpy(fast_info->fast_buffer + frame_size, pcm_buf, pcm_info->buffer_size);
			fast_info->fast_frame_find_flag = FRAME_FIND;
			fast_info->fast_start_addr = start_addr;
			fast_info->dma_tran_count++;
		}
	} else {
		memcpy(fast_info->fast_buffer + frame_size, pcm_buf, pcm_info->buffer_size);
		fast_info->dma_tran_count++;
	}

	if (fast_info->dma_tran_count == fast_info->dma_tran_total_count) {
		stop_dma(PCM_FAST, dma_drv_info);
		fast_info->fast_complete_flag = FAST_TRAN_COMPLETE;
	}

	kzfree(pcm_buf);

}

static void dma_drv_deinit(void)
{
	soundtrigger_mailbox_deinit();

	soundtrigger_socdsp_pcm_deinit();
}

static void close_da_combine_codec_dma(void)
{
	da_combine_soundtrigger_close_codec_dma();
}

static int32_t slimbus_da_combine_activate(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	int32_t ret;

	ret = slimbus_activate(dma_drv_info);
	if (ret != 0)
		return ret;

	dma_drv_info->is_slimbus_enable = 1;

	return 0;
}
static int32_t slimbus_da_combine_deactivate(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	int32_t ret;

	ret = slimbus_deactivate(dma_drv_info);
	if (ret != 0)
		return ret;

	dma_drv_info->is_slimbus_enable = 0;

	return 0;
}

static void proc_left_normal_dma_data(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	struct soundtrigger_pcm_info *pcm_info = NULL;
	struct normal_tran_info *normal_info = NULL;

	pcm_info = &dma_drv_info->pcm_info[PCM_NORMAL];
	if (pcm_info->buffer_size == 0) {
		AUDIO_LOGE("pcm info buffer size is 0");
		return;
	}

	normal_info = &dma_drv_info->normal_tran_info;
	if (normal_info->read_count_left >= normal_info->irq_count_left)
		return;

	dma_normal_trans_buff(normal_info, pcm_info, dma_drv_info->type, dma_drv_info);
}

static void check_pcm_fast_buffer(struct fast_tran_info *fast_info)
{
	uint16_t *valid_buf = fast_info->fast_buffer + fast_info->fast_start_addr;
	uint32_t valid_buf_len = RINGBUF_SIZE / VALID_BYTE_COUNT_EACH_SAMPLE_POINT;
	uint32_t word_count;
	uint32_t index;

	for (word_count = 0; word_count < valid_buf_len; word_count++) {
		if (valid_buf[valid_buf_len - 1 - word_count] != fast_info->fast_head_frame_word)
			break;
	}

	for (index = 0; index < word_count; index++) {
		valid_buf[valid_buf_len - 1 - index] = valid_buf[valid_buf_len - 1 - word_count];
		AUDIO_LOGI("fast tran slimbus miss sampling point[%u]", word_count);
	}
}

static ssize_t read_fast_data(struct fast_tran_info *fast_info, char __user *buffer,
	size_t max_read_len)
{
	int32_t rest_bytes;

	check_pcm_fast_buffer(fast_info);
	rest_bytes = copy_to_user(buffer, fast_info->fast_buffer + fast_info->fast_start_addr,
		max_read_len);
	fast_info->fast_read_complete_flag = READ_COMPLETE;
	if (rest_bytes != 0) {
		AUDIO_LOGE("copy to user failed, rest bytes: %d", rest_bytes);
		return -EINVAL;
	} else {
		return 0;
	}
}

static ssize_t read_dma_fast_data(struct fast_tran_info *fast_info_left,
	struct fast_tran_info *fast_info_right, char __user *buffer, size_t max_read_len)
{
	UNUSED_PARAMETER(fast_info_right);
	return read_fast_data(fast_info_left, buffer, max_read_len);
}

static void proc_dma_fast_right_data(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	if (dma_drv_info->fast_tran_info_right.read_count >= dma_drv_info->fast_tran_info_right.irq_count)
		return;

	dma_drv_info->fast_tran_info_right.read_count++;
}

static void stop_using_dma(const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	stop_dma(PCM_FAST, dma_drv_info);
	stop_dma(PCM_NORMAL, dma_drv_info);
}

void stop_da_combine_dma(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	if ((&dma_drv_info->fast_tran_info_left)->fast_read_complete_flag != FAST_TRAN_COMPLETE)
		stop_dma(PCM_FAST, dma_drv_info);

	stop_dma(PCM_NORMAL, dma_drv_info);
}

static int32_t check_dma_fast_complete_flag(const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	const struct fast_tran_info *fast_info_left = &dma_drv_info->fast_tran_info_left;

	if (fast_info_left->fast_complete_flag == FAST_TRAN_NOT_COMPLETE)
		return -EAGAIN;

	return 0;
}

static int32_t init_dma_drv_probe_info(uint32_t channel_num)
{
	int32_t ret;

	ret = soundtrigger_socdsp_pcm_init(channel_num);
	if (ret != 0)
		return ret;

	ret = soundtrigger_mailbox_init();
	if (ret != 0) {
		soundtrigger_socdsp_pcm_deinit();
		return ret;
	}

	return 0;
}

static void get_da_combine_dma_cfg(struct dma_config *cfg, enum codec_dsp_type dsp_type,
	uint32_t pcm_channel, uint32_t offset)
{
	if (cfg == NULL)
		return;

	if (dsp_type >= CODEC_DA_COMBINE_MAX)
		return;

	if (pcm_channel >= PCM_CHL_NUM)
		return;

	if (offset >= DMA_PORT_NUM)
		return;

	cfg->port = (g_st_dma_cfg[dsp_type][pcm_channel] + offset)->port;
	cfg->channel = (g_st_dma_cfg[dsp_type][pcm_channel] + offset)->channel;
	cfg->config = (g_st_dma_cfg[dsp_type][pcm_channel] + offset)->config;
}

static void get_da_combine_pcm_cfg(struct soundtrigger_pcm_config *cfg,
	enum codec_dsp_type dsp_type, uint32_t pcm_channel)
{
	if (cfg == NULL)
		return;

	if (dsp_type >= CODEC_DA_COMBINE_MAX)
		return;

	if (pcm_channel >= PCM_CHL_NUM)
		return;

	memcpy(cfg, &g_pcm_cfg[dsp_type][pcm_channel], sizeof(*cfg));
}

static void dump_da_combine_dma_info(const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	dump_dma_addr_info(dma_drv_info);
}

static struct soundtrigger_dma_ops g_dma_da_combine_ops = {
	.probe_callback = init_dma_drv_probe_info,
	.open_callback = NULL,
	.get_dma_cfg = get_da_combine_dma_cfg,
	.get_pcm_cfg = get_da_combine_pcm_cfg,
	.start_dma = slimbus_da_combine_activate,
	.check_fast_complete_flag = check_dma_fast_complete_flag,
	.set_dma_int_flag = set_da_combine_dma_int_flag,
	.dmac_irq_handle = dmac_right_fast_irq_handle,
	.proc_fast_data = proc_dma_fast_right_data,
	.proc_fast_trans_buff = proc_dma_left_fast_trans_buff,
	.proc_normal_data = proc_left_normal_dma_data,
	.read_data = read_dma_fast_data,
	.dump_dma_info = dump_da_combine_dma_info,
	.deinit_dma = stop_da_combine_dma,
	.deinit_dma_info = slimbus_da_combine_deactivate,
	.close_codec_dma = close_da_combine_codec_dma,
	.stop_dma = stop_using_dma,
	.dma_drv_remove = dma_drv_deinit,
};

struct soundtrigger_dma_ops *get_da_combine_dma_ops(void)
{
	return &g_dma_da_combine_ops;
}
