/*
 * soundtrigger_dma_drv.c
 *
 * soundtrigger dma misc driver
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

#include "soundtrigger_dma_drv.h"

#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>
#include <linux/hisi/audio_log.h>

#include <sound/dmaengine_pcm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "slimbus.h"
#include "slimbus_da_combine_v5.h"
#include "dsp_misc.h"
#include "dsp_utils.h"
#include "asp_dma.h"
#include "soundtrigger_event.h"
#include "soundtrigger_socdsp_mailbox.h"
#include "soundtrigger_socdsp_pcm.h"
#include "soundtrigger_ring_buffer.h"
#include "soundtrigger_third_codec_data_proc.h"
#include "soundtrigger_dma_drv_da_combine.h"
#include "soundtrigger_dma_drv_third_codec.h"


#define LOG_TAG "soundtrigger"

#define DRV_NAME "soundtrigger_dma_drv"
#define COMP_SOUNDTRIGGER_DMA_DRV_NAME "hisilicon,soundtrigger_dma_drv"
#define SOUNDTRIGGER_HWLOCK_ID 5
#define MAX_MSG_SIZE 1024

#define SOUNDTRIGGER_CMD_DMA_OPEN  _IO('S', 0x1)
#define SOUNDTRIGGER_CMD_DMA_CLOSE  _IO('S', 0x2)
#define SOUNDTRIGGER_CMD_DMA_READY  _IO('S', 0x3)

#define int_to_addr(low, high) \
	(void *)(uintptr_t)((unsigned long long)(low) | \
	((unsigned long long)(high) << 32))

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

static struct soundtrigger_dma_drv_info *g_dma_drv_info;

static struct soundtrigger_dma_ops *g_dma_ops;

static int32_t get_input_param(unsigned int usr_para_size,
	const void __user *usr_para_addr, unsigned int *krn_para_size, void **krn_para_addr)
{
	void *para_in = NULL;
	unsigned int para_size_in = 0;

	if (!usr_para_addr) {
		AUDIO_LOGE("usr para addr is null");
		return -EINVAL;
	}

	if ((usr_para_size == 0) || (usr_para_size > MAX_MSG_SIZE)) {
		AUDIO_LOGE("usr para size invalid %u, max %d", usr_para_size, MAX_MSG_SIZE);
		return -EINVAL;
	}

	para_size_in = roundup(usr_para_size, 4);

	para_in = kzalloc(para_size_in, GFP_KERNEL);
	if (!para_in) {
		AUDIO_LOGE("kzalloc failed");
		return -ENOMEM;
	}

	if (copy_from_user(para_in, usr_para_addr, usr_para_size)) {
		AUDIO_LOGE("copy from user failed");
		kfree(para_in);
		return -EIO;
	}

	*krn_para_size = para_size_in;
	*krn_para_addr = para_in;

	return 0;
}

static void param_free(void **krn_para_addr)
{
	if (*krn_para_addr)
		kfree(*krn_para_addr);
	else
		AUDIO_LOGW("krn para addr to free is null");
}

static int32_t soundtrigger_pcm_init(struct soundtrigger_pcm_info *pcm_info,
	enum codec_dsp_type dsp_type, uint32_t pcm_channel, uint64_t *cfg_addr)
{
	uint32_t i;
	uint32_t j;
	struct dma_config cfg;

	for (i = 0; i < ARRAY_SIZE(pcm_info->channel); i++) {
		g_dma_ops->get_dma_cfg(&cfg, dsp_type, pcm_channel, i);

		pcm_info->channel[i] = cfg.channel;
		for (j = 0; j < ARRAY_SIZE(pcm_info->dma_cfg[0]); j++) {
			/* remap the soundtrigger address */
			pcm_info->buffer_phy_addr[i][j] = (void *)(uintptr_t)(*cfg_addr);
			pcm_info->buffer[i][j] = ioremap_wc((phys_addr_t)(*cfg_addr),
				pcm_info->buffer_size);
			if (!pcm_info->buffer[i][j]) {
				AUDIO_LOGE("remap buffer failed");
				return -ENOMEM;
			}

			memset(pcm_info->buffer[i][j], 0, pcm_info->buffer_size);
			*cfg_addr += pcm_info->buffer_size;

			pcm_info->lli_dma_phy_addr[i][j] = (void *)(uintptr_t)(*cfg_addr);
			pcm_info->dma_cfg[i][j] = ioremap_wc((phys_addr_t)(*cfg_addr),
				sizeof(*pcm_info->dma_cfg[i][j]));
			if (!pcm_info->dma_cfg[i][j]) {
				AUDIO_LOGE("remap dma config failed");
				return -ENOMEM;
			}

			memset(pcm_info->dma_cfg[i][j], 0, sizeof(*pcm_info->dma_cfg[i][j]));
			*cfg_addr += sizeof(*pcm_info->dma_cfg[i][j]);

			/* set dma config */
			pcm_info->dma_cfg[i][j]->config = cfg.config;
			pcm_info->dma_cfg[i][j]->src_addr = cfg.port;
			pcm_info->dma_cfg[i][j]->a_count = pcm_info->buffer_size;
		}
	}

	return 0;
}

static void soundtrigger_pcm_uinit(struct soundtrigger_pcm_info *pcm_info)
{
	uint32_t i;
	uint32_t j;

	for (i = 0; i < ARRAY_SIZE(pcm_info->channel); i++) {
		for (j = 0; j < ARRAY_SIZE(pcm_info->dma_cfg[0]); j++) {
			if (pcm_info->buffer[i][j]) {
				iounmap(pcm_info->buffer[i][j]);
				pcm_info->buffer[i][j] = NULL;
			}

			if (pcm_info->dma_cfg[i][j]) {
				iounmap(pcm_info->dma_cfg[i][j]);
				pcm_info->dma_cfg[i][j] = NULL;
			}
		}
	}
}

static void soundtrigger_pcm_adjust(struct soundtrigger_pcm_info *pcm_info)
{
	uint32_t swap_buf_num;
	uint32_t next_addr;
	uint32_t real_pos;
	uint32_t i;
	uint32_t j;

	swap_buf_num = ARRAY_SIZE(pcm_info->dma_cfg[0]);
	for (i = 0; i < ARRAY_SIZE(pcm_info->channel); i++) {
		for (j = 0; j < swap_buf_num; j++) {
			real_pos = (j + 1) % swap_buf_num;
			next_addr = (uint32_t)(uintptr_t)pcm_info->lli_dma_phy_addr[i][real_pos];
			pcm_info->dma_cfg[i][j]->des_addr =
				(uint32_t)(uintptr_t)pcm_info->buffer_phy_addr[i][j];
			pcm_info->dma_cfg[i][j]->lli = drv_dma_lli_link(next_addr);
		}
	}
}

static void clear_dma_config(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t i;
	struct soundtrigger_pcm_info *pcm_info = NULL;

	AUDIO_LOGI("dma config clear");

	for (i = 0; i < ARRAY_SIZE(dma_drv_info->pcm_info); i++) {
		pcm_info = &(dma_drv_info->pcm_info[i]);
		soundtrigger_pcm_uinit(pcm_info);
	}
}

static int32_t set_dma_config(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	int32_t ret;
	uint32_t i;
	struct soundtrigger_pcm_info *pcm_info = NULL;
	enum codec_dsp_type dsp_type = dma_drv_info->type;
	uint64_t cfg_addr = CODEC_DSP_SOUNDTRIGGER_BASE_ADDR;

	for (i = 0; i < ARRAY_SIZE(dma_drv_info->pcm_info); i++) {
		pcm_info = &(dma_drv_info->pcm_info[i]);

		g_dma_ops->get_pcm_cfg(&(pcm_info->pcm_cfg), dsp_type, i);

		pcm_info->buffer_size = pcm_info->pcm_cfg.frame_len * pcm_info->pcm_cfg.byte_count;

		ret = soundtrigger_pcm_init(pcm_info, dsp_type, i, &cfg_addr);
		if (ret != 0) {
			AUDIO_LOGE("soundtrigger pcm init error, ret: %d", ret);
			clear_dma_config(dma_drv_info);
			return ret;
		}

		soundtrigger_pcm_adjust(pcm_info);
	}

	if (g_dma_ops->dump_dma_info != NULL)
		g_dma_ops->dump_dma_info(dma_drv_info);

	AUDIO_LOGI("success");

	return 0;
}

static int32_t check_dma_int_type(unsigned short int_type)
{
	switch (int_type) {
	case ASP_DMA_INT_TYPE_ERR1:
		AUDIO_LOGE("dma interrupt setting error");
		return IRQ_ERR;

	case ASP_DMA_INT_TYPE_ERR2:
		AUDIO_LOGE("dma interrupt transmission error");
		return IRQ_ERR;

	case ASP_DMA_INT_TYPE_ERR3:
		AUDIO_LOGE("dma interrupt lli error");
		return IRQ_ERR;

	case ASP_DMA_INT_TYPE_TC1:
		AUDIO_LOGE("dma interrupt transmission finish");
		return IRQ_FINISH;

	/* dma lli node transit finish interrupt */
	case ASP_DMA_INT_TYPE_TC2:
		break;

	default:
		AUDIO_LOGE("dma interrupt error, int type: %u", int_type);
		return IRQ_ERR;
	}

	return 0;
}

static int32_t soundtrigger_dmac_irq_handler(unsigned short int_type,
	unsigned long para, unsigned int dma_channel)
{
	int32_t ret;
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;

	if (dma_drv_info == NULL) {
		AUDIO_LOGE("dma drv info is null");
		return IRQ_FINISH;
	}

	ret = check_dma_int_type(int_type);
	if (ret != 0) {
		AUDIO_LOGE("dmac channel: %u", dma_channel);
		return ret;
	}

	switch (dma_channel) {
	case DMA_FAST_LEFT_CH_NUM:
		(dma_drv_info->fast_tran_info_left.irq_count)++;
		dma_drv_info->dma_int_fast_left_flag = 1;
		if (!queue_delayed_work(dma_drv_info->delay_wq,
			&dma_drv_info->delay_dma_fast_left_work, msecs_to_jiffies(0)))
			AUDIO_LOGE("fast left lost msg");

		return IRQ_FINISH;

	case DMA_FAST_RIGHT_CH_NUM:
		g_dma_ops->dmac_irq_handle(dma_drv_info);
		queue_delayed_work(dma_drv_info->delay_wq,
			&dma_drv_info->delay_dma_fast_right_work, msecs_to_jiffies(0));
		return IRQ_FINISH;

	case DMA_NORMAL_LEFT_CH_NUM:
		(dma_drv_info->normal_tran_info.irq_count_left)++;
		dma_drv_info->dma_int_nomal_flag = 1;
		queue_delayed_work(dma_drv_info->delay_wq,
			&dma_drv_info->delay_dma_normal_left_work, msecs_to_jiffies(0));
		return IRQ_FINISH;

	case DMA_NORMAL_RIGHT_CH_NUM:
		(dma_drv_info->normal_tran_info.irq_count_right)++;
		queue_delayed_work(dma_drv_info->delay_wq,
			&dma_drv_info->delay_dma_normal_right_work, msecs_to_jiffies(0));
		return IRQ_FINISH;

	default:
		AUDIO_LOGE("dma interrupt error, dma channel: %u", dma_channel);
		return IRQ_ERR;
	}
}

static void start_dma(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t pcm_num;
	uint32_t dma_channel;
	uint32_t dma_port_num;
	uint32_t i;
	uint32_t j;
	struct soundtrigger_pcm_info *pcm_info = NULL;

	pcm_num = ARRAY_SIZE(dma_drv_info->pcm_info);
	for (i = 0; i < pcm_num; i++) {
		pcm_info = &(dma_drv_info->pcm_info[i]);
		dma_port_num = ARRAY_SIZE(pcm_info->channel);
		for (j = 0; j < dma_port_num; j++) {
			dma_channel = pcm_info->channel[j];
			asp_dma_config((unsigned short)dma_channel,
				pcm_info->dma_cfg[j][0], soundtrigger_dmac_irq_handler, 0);
			asp_dma_start((unsigned short)dma_channel, pcm_info->dma_cfg[j][0]);
		}
	}
}

static void fast_info_init(struct fast_tran_info *fast_info)
{
	memset(fast_info->fast_buffer, 0, sizeof(uint16_t) * FAST_BUFFER_SIZE);

	fast_info->fast_read_complete_flag = READ_NOT_COMPLETE;
	fast_info->dma_tran_count = 0;
	fast_info->fast_complete_flag = FAST_TRAN_NOT_COMPLETE;
	fast_info->fast_head_frame_word = FRAME_MAGIC_WORD;

	fast_info->fast_head_frame_size = FAST_FRAME_SIZE;
	fast_info->dma_tran_total_count = FAST_TRAN_COUNT;

	fast_info->fast_frame_find_flag = FRAME_NOT_FIND;
	fast_info->irq_count = 0;
	fast_info->read_count = 0;
	fast_info->fast_head_word_count = 0;
}

static void fast_info_deinit(struct fast_tran_info *fast_info)
{
	memset(fast_info->fast_buffer, 0, sizeof(uint16_t) * FAST_BUFFER_SIZE);

	fast_info->irq_count = 0;
	fast_info->read_count = 0;
	fast_info->dma_tran_count = 0;
	fast_info->dma_tran_total_count = 0;
}

static int32_t normal_info_init(struct normal_tran_info *normal_info,
	const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	normal_info->normal_buffer = st_ring_buffer_init(RINGBUF_FRAME_SIZE, RINGBUF_FRAME_COUNT);
	if (normal_info->normal_buffer == NULL) {
		AUDIO_LOGE("ring buffer init failed");
		return -ENOMEM;
	}

	normal_info->normal_head_frame_word = FRAME_MAGIC_WORD;
	if (dma_drv_info->type == CODEC_DA_COMBINE_V3) {
		normal_info->normal_head_frame_size = DA_COMBINE_V3_NORMAL_FRAME_SIZE;
	}
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	else if (dma_drv_info->type == CODEC_DA_COMBINE_V5) {
		normal_info->normal_head_frame_size = DA_COMBINE_V5_NORMAL_FRAME_SIZE;
	}
#endif
	else {
		normal_info->normal_buffer->deinit(normal_info->normal_buffer);
		normal_info->normal_buffer = NULL;
		AUDIO_LOGE("device type is err: %d", dma_drv_info->type);
		return -EINVAL;
	}

	normal_info->normal_frame_find_flag = FRAME_NOT_FIND;
	normal_info->normal_first_frame_read_flag = READ_NOT_COMPLETE;
	normal_info->normal_tran_count = 0;
	normal_info->irq_count_left = 0;
	normal_info->irq_count_right = 0;
	normal_info->read_count_left = 0;
	normal_info->read_count_right = 0;
	normal_info->normal_head_word_count = 0;

	return 0;
}

static void normal_info_deinit(struct normal_tran_info *normal_info)
{
	normal_info->irq_count_left = 0;
	normal_info->irq_count_right = 0;
	normal_info->read_count_left = 0;
	normal_info->read_count_right = 0;
	normal_info->normal_tran_count = 0;

	if (normal_info->normal_buffer) {
		normal_info->normal_buffer->deinit(normal_info->normal_buffer);
		normal_info->normal_buffer = NULL;
	}
}

static int32_t check_fast_info(const struct fast_tran_info *fast_info)
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
		g_dma_ops->stop_dma(g_dma_drv_info);

		AUDIO_LOGE("dma fast channel timeout");
		return -EAGAIN;
	}

	return 0;
}

static int32_t soundtrigger_dma_open(struct st_fast_status *fast_status)
{
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;
	struct fast_tran_info *fast_info_left = &dma_drv_info->fast_tran_info_left;
	struct normal_tran_info *normal_info = &dma_drv_info->normal_tran_info;
	int32_t ret;

	if (dma_drv_info->dma_drv_state != DMA_DRV_NO_INIT)
		return -EAGAIN;

	AUDIO_LOGI("dma open");

	fast_info_init(fast_info_left);
	fast_info_init(&dma_drv_info->fast_tran_info_right);

	ret = normal_info_init(normal_info, dma_drv_info);
	if (ret != 0)
		return ret;

	dma_drv_info->fm_status = fast_status->fm_status;

	if (dma_drv_info->dma_alloc_flag == 0) {
		ret = set_dma_config(dma_drv_info);
		if (ret != 0) {
			normal_info_deinit(normal_info);

			AUDIO_LOGE("set dma config failed, err: %d", ret);
			return ret;
		}

		dma_drv_info->dma_alloc_flag = 1;
	}

	dma_drv_info->dma_int_fast_left_flag = 0;

	if (g_dma_ops->open_callback != NULL)
		g_dma_ops->open_callback(dma_drv_info);

	dma_drv_info->dma_int_nomal_flag = 0;

	AUDIO_LOGI("soundtrigger_dma open aspclk: %d, --",
		clk_get_enable_count(dma_drv_info->asp_subsys_clk));

	ret = clk_prepare_enable(dma_drv_info->asp_subsys_clk);
	if (ret != 0) {
		clear_dma_config(dma_drv_info);
		normal_info_deinit(normal_info);

		AUDIO_LOGE("clk prepare enable failed, ret: %d", ret);
		return ret;
	}

	AUDIO_LOGI("soundtrigger_dma open aspclk: %d, ++",
		clk_get_enable_count(dma_drv_info->asp_subsys_clk));

	dma_drv_info->dma_drv_state = DMA_DRV_INIT;

	return 0;
}

static int32_t soundtrigger_dma_start(struct st_fast_status *fast_status)
{
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;
	int32_t ret;

	AUDIO_LOGI("dma start");

	if (dma_drv_info == NULL) {
		AUDIO_LOGE("soundtrigger dma drv info is null");
		return -ENOENT;
	}

	if (dma_drv_info->dma_drv_state != DMA_DRV_INIT) {
		AUDIO_LOGE("soundtrigger dma drv is not open");
		return -EAGAIN;
	}

	if (dma_drv_info->is_dma_enable) {
		AUDIO_LOGE("soundtrigger dma drv is already enabled");
		return -EAGAIN;
	}

	if (g_dma_ops->start_dma != NULL) {
		ret = g_dma_ops->start_dma(dma_drv_info);
		if (ret != 0)
			return ret;
	}
	__pm_stay_awake(&dma_drv_info->wake_lock);

	if (!queue_delayed_work(dma_drv_info->delay_close_dma_wq,
		&dma_drv_info->delay_close_dma_timeout_work,
		msecs_to_jiffies(TIMEOUT_CLOSE_DMA_MS)))
		AUDIO_LOGE("close dma timeout lost msg");

	start_dma(dma_drv_info);
	dma_drv_info->is_dma_enable = 1;

	return 0;
}

static int32_t dma_info_deinit(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	int32_t ret;
	struct fast_tran_info *fast_info_left = &dma_drv_info->fast_tran_info_left;
	struct normal_tran_info *normal_info = &dma_drv_info->normal_tran_info;

	dma_drv_info->dma_drv_state = DMA_DRV_NO_INIT;

	if (dma_drv_info->is_dma_enable) {
		if (g_dma_ops->deinit_dma != NULL)
			g_dma_ops->deinit_dma(dma_drv_info);

		dma_drv_info->is_dma_enable = 0;
	}

	if (dma_drv_info->is_slimbus_enable) {
		ret = g_dma_ops->deinit_dma_info(dma_drv_info);
		if (ret != 0)
			return ret;

	}

	cancel_delayed_work(&dma_drv_info->delay_dma_fast_left_work);
	cancel_delayed_work(&dma_drv_info->delay_dma_fast_right_work);
	cancel_delayed_work(&dma_drv_info->delay_dma_normal_left_work);
	cancel_delayed_work(&dma_drv_info->delay_dma_normal_right_work);
	flush_workqueue(dma_drv_info->delay_wq);

	fast_info_deinit(fast_info_left);
	fast_info_deinit(&dma_drv_info->fast_tran_info_right);
	normal_info_deinit(normal_info);

	return 0;
}

static int32_t soundtrigger_dma_close(void)
{
	int32_t ret;
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;

	AUDIO_LOGI("dma close");

	if (dma_drv_info == NULL)
		return -ENOENT;

	if (dma_drv_info->dma_drv_state != DMA_DRV_INIT) {
		if (dma_drv_info->wake_lock.active)
			__pm_relax(&dma_drv_info->wake_lock);

		AUDIO_LOGE("soundtrigger dma drv is not open");
		return -EAGAIN;
	}

	ret = dma_info_deinit(dma_drv_info);
	if (ret != 0) {
		AUDIO_LOGE("dma close info deinit error, ret: %d", ret);
		return ret;
	}

	if (dma_drv_info->wake_lock.active)
		__pm_relax(&dma_drv_info->wake_lock);

	AUDIO_LOGI("soundtrigger dma close asp clk: %d, ++",
		clk_get_enable_count(dma_drv_info->asp_subsys_clk));

	clk_disable_unprepare(dma_drv_info->asp_subsys_clk);

	AUDIO_LOGI("soundtrigger dma close asp clk: %d, --",
		clk_get_enable_count(dma_drv_info->asp_subsys_clk));

	return 0;
}

static int32_t dma_fops_open(struct inode *finode, struct file *fd)
{
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;

	if (dma_drv_info == NULL)
		return -ENOENT;

	return 0;
}

static int32_t dma_fops_release(struct inode *finode, struct file *fd)
{
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;

	if (dma_drv_info == NULL)
		return -ENOENT;

	return 0;
}

static ssize_t dma_get_max_read_len(enum codec_dsp_type codec_type,
	size_t *max_read_len, size_t count)
{
	if (codec_type == CODEC_DA_COMBINE_V3) {
		*max_read_len = (RINGBUF_SIZE > DA_COMBINE_V3_NORMAL_FRAME_SIZE) ?
			RINGBUF_SIZE : DA_COMBINE_V3_NORMAL_FRAME_SIZE;
	}
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	else if (codec_type == CODEC_DA_COMBINE_V5) {
		*max_read_len = (RINGBUF_SIZE > DA_COMBINE_V5_NORMAL_FRAME_SIZE) ?
			RINGBUF_SIZE : DA_COMBINE_V5_NORMAL_FRAME_SIZE;
	}
#endif
	else {
		AUDIO_LOGE("type is invalid, codec type: %d", codec_type);
		return -EINVAL;
	}

	if (count < *max_read_len) {
		AUDIO_LOGE("user buffer too short, need %zu", *max_read_len);
		return -EINVAL;
	}

	return 0;
}

static ssize_t read_normal_data(struct normal_tran_info *normal_info,
	char __user *buffer)
{
	uint16_t *pcm_buf = NULL;
	static uint16_t static_buffer[RINGBUF_FRAME_LEN];
	int32_t rest_bytes;
	uint32_t vaild_buf_len;

	if ((normal_info->normal_buffer == NULL) ||
		(normal_info->normal_buffer->empty(normal_info->normal_buffer)))
		return -EFAULT;

	if (normal_info->normal_start_addr > RINGBUF_FRAME_LEN) {
		AUDIO_LOGE("normal start addr error: %u", normal_info->normal_start_addr);
		return -EINVAL;
	}

	pcm_buf = kzalloc(RINGBUF_FRAME_SIZE, GFP_KERNEL);
	if (pcm_buf == NULL) {
		AUDIO_LOGE("pcm buffer kzalloc failed");
		return -ENOMEM;
	}

	normal_info->normal_buffer->get(normal_info->normal_buffer, pcm_buf, RINGBUF_FRAME_SIZE);
	vaild_buf_len = RINGBUF_FRAME_LEN - normal_info->normal_start_addr;

	if (normal_info->normal_first_frame_read_flag == READ_NOT_COMPLETE) {
		normal_info->normal_first_frame_read_flag = READ_COMPLETE;
		memcpy(static_buffer, pcm_buf + normal_info->normal_start_addr,
			vaild_buf_len * VALID_BYTE_COUNT_EACH_SAMPLE_POINT);
		kzfree(pcm_buf);
		return -EINVAL;
	}

	memcpy(static_buffer + vaild_buf_len, pcm_buf,
		normal_info->normal_start_addr * VALID_BYTE_COUNT_EACH_SAMPLE_POINT);

	rest_bytes = copy_to_user(buffer, static_buffer, RINGBUF_FRAME_SIZE);
	memcpy(static_buffer, pcm_buf + normal_info->normal_start_addr,
		vaild_buf_len * VALID_BYTE_COUNT_EACH_SAMPLE_POINT);
	kzfree(pcm_buf);
	if (rest_bytes != 0) {
		AUDIO_LOGE("copy to user failed, rest bytes: %d", rest_bytes);
		return -EINVAL;
	}

	return 0;
}

static int32_t check_dma_read_info(struct soundtrigger_dma_drv_info *dma_drv_info,
	char __user *buf)
{
	if (dma_drv_info == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	if (buf == NULL) {
		AUDIO_LOGE("buffer is invalid");
		return -EINVAL;
	}

	if (dma_drv_info->dma_drv_state == DMA_DRV_NO_INIT) {
		AUDIO_LOGE("soundtrigger dma aleady closed");
		return -EINVAL;
	}

	return 0;
}

static ssize_t dma_fops_read(struct file *file,
	char __user *buffer, size_t count, loff_t *f_ops)
{
	size_t max_read_len = 0;
	ssize_t ret;
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;
	struct fast_tran_info *fast_info_left = &dma_drv_info->fast_tran_info_left;
	struct fast_tran_info *fast_info_right = &dma_drv_info->fast_tran_info_right;
	struct normal_tran_info *normal_info = NULL;

	ret = check_dma_read_info(dma_drv_info, buffer);
	if (ret != 0) {
		AUDIO_LOGE("check dma read info error, ret: %zd", ret);
		return ret;
	}

	ret = dma_get_max_read_len(dma_drv_info->type, &max_read_len, count);
	if (ret < 0) {
		AUDIO_LOGE("get max read len error, ret: %zd", ret);
		return ret;
	}

	ret = g_dma_ops->check_fast_complete_flag(dma_drv_info);
	if (ret != 0) {
		AUDIO_LOGE("check fast complete flag error, ret: %zd", ret);
		return ret;
	}

	g_dma_ops->set_dma_int_flag(dma_drv_info);

	if (fast_info_left->fast_read_complete_flag == READ_NOT_COMPLETE) {
		ret = g_dma_ops->read_data(fast_info_left, fast_info_right, buffer, max_read_len);
		if (ret == 0)
			return max_read_len;
	} else {
		normal_info = &(dma_drv_info->normal_tran_info);
		ret = read_normal_data(normal_info, buffer);
		if (ret == 0)
			return RINGBUF_FRAME_SIZE;
	}

	return -EINVAL;
}

static int32_t soundtrigger_dma_fops(uint32_t cmd, struct st_fast_status *fast_status)
{
	int32_t ret = 0;

	if (fast_status == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	if (g_dma_drv_info == NULL) {
		AUDIO_LOGE("dma drv info is null");
		return -EINVAL;
	}

	if (cmd == SOUNDTRIGGER_CMD_DMA_CLOSE) {
		if (cancel_delayed_work_sync(&g_dma_drv_info->delay_close_dma_timeout_work))
			AUDIO_LOGI("cancel timeout dma close work success");
	}

	mutex_lock(&g_dma_drv_info->ioctl_mutex);
	switch (cmd) {
	case SOUNDTRIGGER_CMD_DMA_READY:
		ret = soundtrigger_dma_open(fast_status);
		AUDIO_LOGI("dma open, ret[%d]", ret);
		break;
	case SOUNDTRIGGER_CMD_DMA_OPEN:
		ret = soundtrigger_dma_start(fast_status);
		AUDIO_LOGI("dma start, ret[%d]", ret);
		break;
	case SOUNDTRIGGER_CMD_DMA_CLOSE:
		ret = soundtrigger_dma_close();
		AUDIO_LOGI("dma close, ret[%d]", ret);
		break;
	default:
		AUDIO_LOGE("invalid value, ret[%d]", ret);
		ret = -ENOTTY;
		break;
	}
	mutex_unlock(&g_dma_drv_info->ioctl_mutex);

	return ret;
}

static long dma_fops_ioctl(struct file *fd, uint32_t cmd, uintptr_t arg)
{
	int32_t ret;
	struct st_fast_status *fast_status = NULL;
	struct soundtrigger_io_sync_param param;
	struct da_combine_param_io_buf krn_param;

	if (!(void __user *)arg) {
		AUDIO_LOGE("input error: arg is null");
		return -EINVAL;
	}

	if (copy_from_user(&param, (void __user *)arg, sizeof(param))) {
		AUDIO_LOGE("copy from user failed");
		return -EIO;
	}

	ret = get_input_param(param.para_size_in, int_to_addr(param.para_in_l, param.para_in_h),
		&krn_param.buf_size_in, (void **)&krn_param.buf_in);
	if (ret != 0) {
		AUDIO_LOGE("input error: input param is not valid");
		return -EINVAL;
	}

	fast_status = (struct st_fast_status *)krn_param.buf_in;

	ret = soundtrigger_dma_fops(cmd, fast_status);
	if (ret != 0)
		AUDIO_LOGW("soundtrigger dma fops error");

	param_free((void **)&(krn_param.buf_in));

	return ret;
}

static long dma_fops_ioctl32(struct file *fd, uint32_t cmd, unsigned long arg)
{
	void __user *user_arg = (void __user *)compat_ptr(arg);

	return dma_fops_ioctl(fd, cmd, (uintptr_t)user_arg);
}

void da_combine_soundtrigger_dma_close(void)
{
	int32_t ret;
	struct st_fast_status fast_status = {0};

	ret = soundtrigger_dma_fops(SOUNDTRIGGER_CMD_DMA_CLOSE, &fast_status);
	if (ret != 0)
		AUDIO_LOGW("soundtrigger dma fops error");
}

static void soundtrigger_close_soc_dma(void)
{
	int32_t ret;

	if (g_dma_drv_info == NULL)
		return;

	mutex_lock(&g_dma_drv_info->ioctl_mutex);
	ret = soundtrigger_dma_close();
	if (ret != 0)
		AUDIO_LOGW("dma close failed, ret: %d", ret);
	mutex_unlock(&g_dma_drv_info->ioctl_mutex);
}

static void dma_fast_left_workfunc(struct work_struct *work)
{
	static uint32_t om_fast_count;
	int32_t ret;
	struct soundtrigger_dma_drv_info *dma_drv_info = NULL;
	struct soundtrigger_pcm_info *pcm_info = NULL;
	struct fast_tran_info *fast_info = NULL;

	om_fast_count++;
	if (om_fast_count == 50) {
		om_fast_count = 0;
		AUDIO_LOGI("fast dma irq come");
	}

	dma_drv_info = container_of(work, struct soundtrigger_dma_drv_info,
		delay_dma_fast_left_work.work);
	if (dma_drv_info == NULL) {
		AUDIO_LOGE("dma drv info get error");
		return;
	}

	if (dma_drv_info->dma_drv_state == DMA_DRV_NO_INIT) {
		AUDIO_LOGE("drv is not open, work queue don't process");
		return;
	}

	pcm_info = &dma_drv_info->pcm_info[PCM_FAST];
	if (pcm_info->buffer_size == 0) {
		AUDIO_LOGE("pcm info buffer size is 0");
		return;
	}

	fast_info = &dma_drv_info->fast_tran_info_left;
	ret = check_fast_info(fast_info);
	if (ret != 0) {
		AUDIO_LOGE("check fast info error, ret: %d", ret);
		return;
	}

	g_dma_ops->proc_fast_trans_buff(fast_info, pcm_info, dma_drv_info);
}

static void dma_fast_right_workfunc(struct work_struct *work)
{
	struct soundtrigger_dma_drv_info *dma_drv_info = NULL;
	static uint32_t om_fast_right_count;

	om_fast_right_count++;
	if (om_fast_right_count == 50) {
		om_fast_right_count = 0;
		AUDIO_LOGI("right fast dma irq come");
	}

	dma_drv_info = container_of(work, struct soundtrigger_dma_drv_info,
		delay_dma_fast_right_work.work);

	if (dma_drv_info == NULL) {
		AUDIO_LOGE("right dma drv info null");
		return;
	}

	if (dma_drv_info->dma_drv_state == DMA_DRV_NO_INIT) {
		AUDIO_LOGE("right drv is not open, work queue don't process");
		return;
	}

	g_dma_ops->proc_fast_data(dma_drv_info);
}

static void dma_normal_left_workfunc(struct work_struct *work)
{
	static uint32_t om_normal_count;
	struct soundtrigger_dma_drv_info *dma_drv_info = NULL;

	om_normal_count++;
	if (om_normal_count == 50) {
		om_normal_count = 0;
		AUDIO_LOGI("normal dma irq come");
	}

	dma_drv_info = container_of(work, struct soundtrigger_dma_drv_info,
		delay_dma_normal_left_work.work);
	if (dma_drv_info == NULL) {
		AUDIO_LOGE("dma drv info is null");
		return;
	}

	if (dma_drv_info->dma_drv_state == DMA_DRV_NO_INIT) {
		AUDIO_LOGE("drv is not open, work queue don't process");
		return;
	}

	if (g_dma_ops->proc_normal_data != NULL)
		g_dma_ops->proc_normal_data(dma_drv_info);
}


static void dma_normal_right_workfunc(struct work_struct *work)
{
	struct soundtrigger_dma_drv_info *dma_drv_info =
		container_of(work, struct soundtrigger_dma_drv_info,
		delay_dma_normal_right_work.work);

	if (dma_drv_info == NULL) {
		AUDIO_LOGE("dma drv info null");
		return;
	}

	if (dma_drv_info->dma_drv_state == DMA_DRV_NO_INIT) {
		AUDIO_LOGE("drv is not open, work queue don't process");
		return;
	}

	if (dma_drv_info->normal_tran_info.read_count_right >=
		dma_drv_info->normal_tran_info.irq_count_right)
		return;

	dma_drv_info->normal_tran_info.read_count_right++;
}

static void close_dma_timeout_workfunc(struct work_struct *work)
{
	UNUSED_PARAMETER(work);

	AUDIO_LOGI("timeout close dma");
	soundtrigger_close_soc_dma();

	if (g_dma_ops->close_codec_dma != NULL)
		g_dma_ops->close_codec_dma();
}

static const struct file_operations g_soundtrigger_dma_drv_fops = {
	.owner = THIS_MODULE,
	.open = dma_fops_open,
	.release = dma_fops_release,
	.read = dma_fops_read,
	.unlocked_ioctl = dma_fops_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = dma_fops_ioctl32,
#endif
};

static struct miscdevice g_soundtrigger_dma_drv_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRV_NAME,
	.fops = &g_soundtrigger_dma_drv_fops,
};

int32_t soundtrigger_set_codec_type(enum codec_dsp_type type)
{
	if (g_dma_drv_info == NULL) {
		AUDIO_LOGE("device not init, use default config");
		return -ENOENT;
	}

	g_dma_drv_info->type = type;
	AUDIO_LOGI("dsp type: %d", type);

	return 0;
}

static int soundtrigger_third_codec_set_codec_type(struct device *dev)
{
	return 0;
}

static void soundtrigger_init_dma_ops(enum codec_dsp_type type)
{
	g_dma_ops = get_da_combine_dma_ops();

	AUDIO_LOGI("codec type: %d", type);
}

static int dma_drv_workqueue_init(struct soundtrigger_dma_drv_info *drv_info)
{
	drv_info->delay_wq = create_singlethread_workqueue("soundtrigger_delay_wq");

	if (!drv_info->delay_wq) {
		AUDIO_LOGE("create delay workqueue failed");
		return -ENOMEM;
	}

	drv_info->delay_close_dma_wq = create_singlethread_workqueue("soundtrigger_delay_close_dma_wq");
	if (!drv_info->delay_close_dma_wq) {
		flush_workqueue(drv_info->delay_wq);
		destroy_workqueue(drv_info->delay_wq);
		AUDIO_LOGE("create delay close dma workqueue failed");
		return -ENOMEM;
	}

	INIT_DELAYED_WORK(&drv_info->delay_dma_fast_left_work, dma_fast_left_workfunc);
	INIT_DELAYED_WORK(&drv_info->delay_dma_fast_right_work, dma_fast_right_workfunc);
	INIT_DELAYED_WORK(&drv_info->delay_dma_normal_left_work, dma_normal_left_workfunc);
	INIT_DELAYED_WORK(&drv_info->delay_dma_normal_right_work, dma_normal_right_workfunc);
	INIT_DELAYED_WORK(&drv_info->delay_close_dma_timeout_work, close_dma_timeout_workfunc);

	return 0;
}

static void dma_drv_workqueue_deinit(void)
{
	if (g_dma_drv_info->delay_wq) {
		cancel_delayed_work(&g_dma_drv_info->delay_dma_fast_left_work);
		cancel_delayed_work(&g_dma_drv_info->delay_dma_fast_right_work);
		cancel_delayed_work(&g_dma_drv_info->delay_dma_normal_left_work);
		cancel_delayed_work(&g_dma_drv_info->delay_dma_normal_right_work);
		flush_workqueue(g_dma_drv_info->delay_wq);
		destroy_workqueue(g_dma_drv_info->delay_wq);
	}

	if (g_dma_drv_info->delay_close_dma_wq) {
		cancel_delayed_work(&g_dma_drv_info->delay_close_dma_timeout_work);
		flush_workqueue(g_dma_drv_info->delay_close_dma_wq);
		destroy_workqueue(g_dma_drv_info->delay_close_dma_wq);
	}
}

static int remap_base_addr(struct soundtrigger_dma_drv_info *info, struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	info->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!info->res) {
		AUDIO_LOGE("get res error");
		return -ENOENT;
	}

	info->reg_base_addr = devm_ioremap(dev, info->res->start, resource_size(info->res));
	if (!info->reg_base_addr) {
		AUDIO_LOGE("asp dma addr ioremap failed");
		return -ENOMEM;
	}

	info->slimbus_base_reg_addr = devm_ioremap(dev, SLIMBUS_BASE_REG, SLIMBUS_REG_SIZE);
	if (!info->slimbus_base_reg_addr) {
		AUDIO_LOGE("slimbus ioremap failed");
		return -ENOENT;
	}

	return 0;
}

static int dma_drv_info_init(struct soundtrigger_dma_drv_info **dma_drv_info, struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct soundtrigger_dma_drv_info *info = NULL;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		AUDIO_LOGE("malloc failed");
		return -ENOMEM;
	}

	ret = remap_base_addr(info, pdev);
	if (ret != 0)
		return ret;

	info->asp_ip = devm_regulator_get(dev, "asp-dmac");
	if (IS_ERR(info->asp_ip)) {
		AUDIO_LOGE("regulator asp dmac failed");
		return -ENOENT;
	}

	info->asp_subsys_clk = devm_clk_get(dev, "clk_asp_subsys");
	if (IS_ERR(info->asp_subsys_clk)) {
		ret = PTR_ERR(info->asp_subsys_clk);
		AUDIO_LOGE("asp subsys clk failed, ret: %d", ret);
		return ret;
	}

	info->hwlock = hwspin_lock_request_specific(SOUNDTRIGGER_HWLOCK_ID);
	if (!info->hwlock) {
		AUDIO_LOGE("get hwspin error");
		return -ENOENT;
	}

	ret = dma_drv_workqueue_init(info);
	if (ret != 0) {
		hwspin_lock_free(info->hwlock);
		return ret;
	}

	wakeup_source_init(&info->wake_lock, "da_combine-soundtrigger");
	mutex_init(&info->ioctl_mutex);
	spin_lock_init(&info->lock);

	info->dev = dev;
	info->dma_drv_state = DMA_DRV_NO_INIT;
	info->dma_alloc_flag = 0;
	info->is_dma_enable = 0;
	info->is_slimbus_enable = 0;

	*dma_drv_info = info;

	return 0;
}

static void dma_drv_info_deinit(void)
{
	mutex_destroy(&g_dma_drv_info->ioctl_mutex);
	wakeup_source_trash(&g_dma_drv_info->wake_lock);

	dma_drv_workqueue_deinit();

	if (hwspin_lock_free(g_dma_drv_info->hwlock))
		AUDIO_LOGE("free dma drv info hwlock fail");
}

static int soundtrigger_dma_drv_probe(struct platform_device *pdev)
{
	int ret;
	uint32_t channel_num = ONEMIC_CHN;
	struct device *dev = &pdev->dev;
	struct soundtrigger_dma_drv_info *dma_drv_info = NULL;

	if (dev == NULL) {
		AUDIO_LOGE("dev is null");
		return -EINVAL;
	}

	ret = of_property_read_u32(dev->of_node, "sochifi_wakeup_upload_chn_num", &channel_num);
	if (ret == 0)
		AUDIO_LOGI("sochifi wakeup upload channel num: %u", channel_num);

	ret = misc_register(&g_soundtrigger_dma_drv_device);
	if (ret != 0) {
		AUDIO_LOGE("misc registe failed");
		return -EBUSY;
	}

	ret = dma_drv_info_init(&dma_drv_info, pdev);
	if (ret != 0) {
		AUDIO_LOGE("dma driver init failed");
		misc_deregister(&g_soundtrigger_dma_drv_device);
		return ret;
	}

	platform_set_drvdata(pdev, dma_drv_info);

	g_dma_drv_info = dma_drv_info;

	ret = soundtrigger_third_codec_set_codec_type(dev);
	if (ret != 0) {
		AUDIO_LOGE("soundtrigger_third_codec_set_codec_type error, ret: %d", ret);
		dma_drv_info_deinit();

		clear_dma_config(dma_drv_info);
		dma_drv_info->dma_alloc_flag = 0;

		misc_deregister(&g_soundtrigger_dma_drv_device);
		return ret;
	}

	soundtrigger_init_dma_ops(g_dma_drv_info->type);

	if (g_dma_ops->probe_callback != NULL)
		ret = g_dma_ops->probe_callback(channel_num);

	if (ret != 0) {
		dma_drv_info_deinit();

		clear_dma_config(dma_drv_info);
		dma_drv_info->dma_alloc_flag = 0;

		misc_deregister(&g_soundtrigger_dma_drv_device);

		g_dma_drv_info = NULL;

		return ret;
	}

	AUDIO_LOGI("success");
	return 0;
}

static int32_t soundtrigger_dma_drv_remove(struct platform_device *pdev)
{
	struct soundtrigger_dma_drv_info *dma_drv_info = g_dma_drv_info;

	UNUSED_PARAMETER(pdev);

	if (dma_drv_info == NULL) {
		AUDIO_LOGE("dma drv info is null");
		return -ENOENT;
	}

	if (g_dma_ops->dma_drv_remove != NULL)
		g_dma_ops->dma_drv_remove();

	dma_drv_info_deinit();

	clear_dma_config(dma_drv_info);
	dma_drv_info->dma_alloc_flag = 0;

	misc_deregister(&g_soundtrigger_dma_drv_device);

	g_dma_drv_info = NULL;

	return 0;
}

static const struct of_device_id soundtrigger_dma_match_table[] = {
	{ .compatible = COMP_SOUNDTRIGGER_DMA_DRV_NAME, },
	{},
};

MODULE_DEVICE_TABLE(of, soundtrigger_dma_match_table);

static struct platform_driver soundtrigger_dma_driver = {
	.driver = {
		.name = "soundtrigger dma drviver",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(soundtrigger_dma_match_table),
	},
	.probe = soundtrigger_dma_drv_probe,
	.remove = soundtrigger_dma_drv_remove,
};

static int32_t __init soundtrigger_dma_drv_init(void)
{
	AUDIO_LOGI("in");
	return platform_driver_register(&soundtrigger_dma_driver);
}
module_init(soundtrigger_dma_drv_init);

static void __exit soundtrigger_dma_drv_exit(void)
{
	platform_driver_unregister(&soundtrigger_dma_driver);
}
module_exit(soundtrigger_dma_drv_exit);

MODULE_DESCRIPTION("Soundtrigger DMA Driver");
MODULE_LICENSE("GPL v2");

