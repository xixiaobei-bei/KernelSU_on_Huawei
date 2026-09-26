/*
 * soundtrigger_dma_drv_third_codec.c
 *
 * soundtrigger_dma_drv_third_codec is a kernel driver which is used to manager third_codec dma
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

#include "soundtrigger_dma_drv_third_codec.h"

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/hisi/audio_log.h>

#include "soundtrigger_third_codec_data_proc.h"
#include "soundtrigger_dma_drv_comm.h"


#define LOG_TAG "soundtrigger_third_codec"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

#define CODEC_TYPE_KIND_NUM 1

static const struct dma_config g_third_codec_st_dma_fast_cfg_left[STEREO_DMA_PORT_NUM] = {
	{
		.port = THIRD_CODEC_FAST_L_PORT_ADDR,
		.config = THIRD_CODEC_FAST_L_DMA_CONF_NUM,
		.channel = DMA_FAST_LEFT_CH_NUM
	},
	{
		.port = THIRD_CODEC_FAST_R_PORT_ADDR,
		.config = THIRD_CODEC_FAST_R_DMA_CONF_NUM,
		.channel = DMA_FAST_RIGHT_CH_NUM
	},
};
static const struct dma_config g_third_codec_st_dma_fast_cfg_right[STEREO_DMA_PORT_NUM] = {
	{
		.port = THIRD_CODEC_FAST_R_PORT_ADDR,
		.config = THIRD_CODEC_FAST_R_DMA_CONF_NUM,
		.channel = DMA_FAST_RIGHT_CH_NUM
	},
	{
		.port = THIRD_CODEC_FAST_L_PORT_ADDR,
		.config = THIRD_CODEC_FAST_L_DMA_CONF_NUM,
		.channel = DMA_FAST_LEFT_CH_NUM
	},
};

static const struct dma_config *g_st_dma_cfg[CODEC_TYPE_KIND_NUM][PCM_CHL_NUM] = {
	{ g_third_codec_st_dma_fast_cfg_left, g_third_codec_st_dma_fast_cfg_right }
};

static const struct soundtrigger_pcm_config g_pcm_cfg[CODEC_TYPE_KIND_NUM][PCM_CHL_NUM] = {
};

static void soundtrigger_third_codec_fast_info_init(const struct fast_tran_info *fast_info,
	struct soundtrigger_third_codec_fast_tran_info *soundtrigger_fast_info_third_codec)
{
	soundtrigger_fast_info_third_codec->fast_frame_find_flag = (uint32_t *)&fast_info->fast_frame_find_flag;
	soundtrigger_fast_info_third_codec->fast_start_addr = (uint32_t *)&fast_info->fast_start_addr;
	soundtrigger_fast_info_third_codec->dma_tran_count = (int32_t *)&fast_info->dma_tran_count;
	soundtrigger_fast_info_third_codec->fast_head_frame_word = fast_info->fast_head_frame_word;
	soundtrigger_fast_info_third_codec->fast_buffer = (uint8_t *)fast_info->fast_buffer;
}

static void dma_fast_trans_buff_third_codec(struct fast_tran_info *fast_info, struct soundtrigger_pcm_info *pcm_info)
{
	uint32_t *pcm_buf = NULL;
	uint32_t buf_index;
	struct soundtrigger_third_codec_fast_tran_info fast_info_third_codec;

	pcm_buf = kzalloc(pcm_info->buffer_size, GFP_KERNEL);
	if (pcm_buf == NULL) {
		AUDIO_LOGE("pcm buffer kzalloc failed");
		return;
	}

	buf_index = fast_info->read_count % PCM_SWAP_BUFFER_NUM;
	memcpy(pcm_buf, (uint32_t *)pcm_info->buffer[0][buf_index], pcm_info->buffer_size);
	fast_info->read_count++;

	soundtrigger_third_codec_fast_info_init(fast_info, &fast_info_third_codec);
	soundtrigger_third_codec_trans_dma_fast_buf(&fast_info_third_codec, pcm_buf, pcm_info->buffer_size);

	kzfree(pcm_buf);
}

static void set_third_codec_dma_int_flag(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	if (dma_drv_info->dma_int_fast_left_flag == 0 ||
		dma_drv_info->dma_int_fast_right_flag == 0) {
		dma_drv_info->dma_int_fast_left_flag = 1;
		dma_drv_info->dma_int_fast_right_flag = 1;
		AUDIO_LOGE("dma int fast left flag or dma int fast right flag is err");
	}
}

static void dmac_right_fast_irq_handle(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	(dma_drv_info->fast_tran_info_right.irq_count)++;
	dma_drv_info->dma_int_fast_right_flag = 1;
}

static void proc_dma_left_fast_trans_buff(struct fast_tran_info *fast_info,
	struct soundtrigger_pcm_info *pcm_info, struct soundtrigger_dma_drv_info *dma_drv_info)
{
	dma_fast_trans_buff_third_codec(fast_info, pcm_info);
	if (fast_info->dma_tran_count == DMA_THIRD_CODEC_TRAN_COUNT) {
		stop_dma(PCM_FAST_LEFT, dma_drv_info);
		fast_info->fast_complete_flag = FAST_TRAN_COMPLETE;
	}
}

static void init_fast_info(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	dma_drv_info->dma_int_fast_right_flag = 0;
	dma_drv_info->fast_tran_info_right.fast_start_addr = 0;
	dma_drv_info->fast_tran_info_left.fast_start_addr = 0;
}

static ssize_t read_fast_data(struct fast_tran_info *fast_info_left,
	struct fast_tran_info *fast_info_right, char __user *buffer, size_t max_read_len)
{
	ssize_t ret = 0;

	if (fast_info_right->fast_read_complete_flag == READ_NOT_COMPLETE) {
		ret = soundtrigger_third_codec_read_msbc_fast_data((uint8_t *)fast_info_left->fast_buffer,
			(uint8_t *)fast_info_right->fast_buffer, FAST_BUFFER_SIZE, buffer);
		fast_info_left->fast_read_complete_flag = READ_COMPLETE;
		fast_info_right->fast_read_complete_flag = READ_COMPLETE;
	}

	return ret;
}

static int32_t check_fast_info(const struct fast_tran_info *fast_info,
	struct soundtrigger_dma_drv_info *dma_drv_info)
{
	if (fast_info->fast_complete_flag == FAST_TRAN_COMPLETE) {
		AUDIO_LOGE("fast transmit complete");
		return -EAGAIN;
	}

	if (fast_info->read_count >= fast_info->irq_count) {
		AUDIO_LOGE("read count %u out of range irq count %u error",
			fast_info->read_count, fast_info->irq_count);
		return -EAGAIN;
	}

	if (fast_info->read_count >= FAST_CHANNEL_TIMEOUT_READ_COUNT) {
		stop_dma(PCM_FAST_LEFT, dma_drv_info);
		stop_dma(PCM_FAST_RIGHT, dma_drv_info);
		AUDIO_LOGE("dma fast channel timeout");
		return -EAGAIN;
	}

	return 0;
}

static void proc_dma_fast_right_data(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	int32_t ret;
	struct soundtrigger_pcm_info *pcm_info = NULL;
	struct fast_tran_info *fast_info = NULL;

	pcm_info = &dma_drv_info->pcm_info[PCM_FAST_RIGHT];
	if (pcm_info->buffer_size == 0) {
		AUDIO_LOGE("right pcm info buffer size is 0");
		return;
	}

	fast_info = &dma_drv_info->fast_tran_info_right;
	ret = check_fast_info(fast_info, dma_drv_info);
	if (ret != 0) {
		AUDIO_LOGE("check fast info error, ret: %d", ret);
		return;
	}

	dma_fast_trans_buff_third_codec(fast_info, pcm_info);
	if (fast_info->dma_tran_count == DMA_THIRD_CODEC_TRAN_COUNT) {
		stop_dma(PCM_FAST_RIGHT, dma_drv_info);
		fast_info->fast_complete_flag = FAST_TRAN_COMPLETE;
	}
}

static void stop_using_dma(const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	stop_dma(PCM_FAST_LEFT, dma_drv_info);
	stop_dma(PCM_FAST_RIGHT, dma_drv_info);
}

static void stop_third_codec_dma(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	if (dma_drv_info->fast_tran_info_left.fast_read_complete_flag != FAST_TRAN_COMPLETE)
		stop_dma(PCM_FAST_LEFT, dma_drv_info);

	if (dma_drv_info->fast_tran_info_right.fast_read_complete_flag != FAST_TRAN_COMPLETE)
		stop_dma(PCM_FAST_RIGHT, dma_drv_info);
}

static int32_t check_dma_fast_complete_flag(const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	const struct fast_tran_info *fast_info_left = &dma_drv_info->fast_tran_info_left;
	const struct fast_tran_info *fast_info_right = &dma_drv_info->fast_tran_info_right;

	if (fast_info_left->fast_complete_flag == FAST_TRAN_NOT_COMPLETE)
		return -EAGAIN;

	if (fast_info_right->fast_complete_flag == FAST_TRAN_NOT_COMPLETE)
		return -EAGAIN;

	return 0;
}

static void get_third_codec_dma_cfg(struct dma_config *cfg, enum codec_dsp_type dsp_type,
	uint32_t pcm_channel, uint32_t offset)
{
	UNUSED_PARAMETER(dsp_type);
	if (cfg == NULL)
		return;

	if (pcm_channel >= PCM_CHL_NUM)
		return;

	if (offset >= DMA_PORT_NUM)
		return;

	cfg->port = (g_st_dma_cfg[0][pcm_channel] + offset)->port;
	cfg->config = (g_st_dma_cfg[0][pcm_channel] + offset)->config;
	cfg->channel = (g_st_dma_cfg[0][pcm_channel] + offset)->channel;
}

static void get_third_codec_pcm_cfg(struct soundtrigger_pcm_config *cfg,
	enum codec_dsp_type dsp_type, uint32_t pcm_channel)
{
	UNUSED_PARAMETER(dsp_type);
	if (cfg == NULL)
		return;

	if (pcm_channel >= PCM_CHL_NUM)
		return;

	memcpy(cfg, &g_pcm_cfg[0][pcm_channel], sizeof(*cfg));
}

static struct soundtrigger_dma_ops g_dma_third_codec_ops = {
	.probe_callback = NULL,
	.open_callback = init_fast_info,
	.get_dma_cfg = get_third_codec_dma_cfg,
	.get_pcm_cfg = get_third_codec_pcm_cfg,
	.start_dma = NULL,
	.check_fast_complete_flag = check_dma_fast_complete_flag,
	.set_dma_int_flag = set_third_codec_dma_int_flag,
	.dmac_irq_handle = dmac_right_fast_irq_handle,
	.proc_fast_data = proc_dma_fast_right_data,
	.proc_fast_trans_buff = proc_dma_left_fast_trans_buff,
	.proc_normal_data = NULL,
	.read_data = read_fast_data,
	.dump_dma_info = NULL,
	.deinit_dma = stop_third_codec_dma,
	.deinit_dma_info = NULL,
	.close_codec_dma = NULL,
	.stop_dma = stop_using_dma,
	.dma_drv_remove = NULL,
};

struct soundtrigger_dma_ops *get_third_codec_dma_ops(void)
{
	return &g_dma_third_codec_ops;
}

