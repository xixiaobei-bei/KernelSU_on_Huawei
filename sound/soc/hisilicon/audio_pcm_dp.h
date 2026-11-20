/*
 * audio_pcm_dp.h
 *
 *  alsa soc audio pcm dp driver
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#ifndef __AUIDO_PCM_DP_H__
#define __AUIDO_PCM_DP_H__

#include <linux/regulator/consumer.h>

#define HDMI_DMA_A 0
#define HDMI_DMA_B 1

#define HDMI_DMA_A_MASK           (0x1 << HDMI_DMA_A)
#define HDMI_DMA_B_MASK           (0x1 << HDMI_DMA_B)
#define HDMI_IRQ_MASK             0x7
#define ASP_CFG_HDMI_CLK_SEL_I2S  0x1

#define AUDIO_MIN_DMA_TIME_US      20000
#define AUDIO_MAX_DMA_TIME_US      22000

#define AUDIO_PLL6_RATE_48         393216000
#define AUDIO_PLL6_RATE_44         361267200
#define AUDIO_PLL6_RATE_DEFAULT    24576000

#define AUDIO_DIV_CLK_48           0xffff000f
#define AUDIO_DIV_CLK_96           0xffff0007
#define AUDIO_DIV_CLK_192          0xffff0003
#define AUDIO_DIV_CLK_NO_SUPPORT   0x0

#define AUDIO_WORK_DELAY_1MS       33 /* 33 equals 1ms */

enum audio_hdmi_status {
	STATUS_HDMI_STOP = 0,
	STATUS_HDMI_RUNNING
};

struct audio_dp_data {
	struct regulator_bulk_data regu;
	struct device *dev;
	struct clk *dp_audio_pll_clk;
	struct clk *asp_subsys_clk;
	int irq;
	unsigned int pre_time;
};

struct audio_dp_runtime_data {
	struct audio_dp_data *pdata;
	spinlock_t lock;
	struct mutex mutex;
	enum audio_hdmi_status status;
	unsigned int dma_addr;
	unsigned int period_size;
	unsigned int period_cur;
	unsigned int period_next;
};

int stop_pcm_dp_substream(void);

#endif /* __AUIDO_PCM_DP_H__ */
