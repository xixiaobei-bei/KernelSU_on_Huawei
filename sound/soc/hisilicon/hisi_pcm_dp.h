/*
 * hi3630_asp_dma.h -- ALSA SoC HI3630 ASP DMA driver
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HISI_PCM_DP_H__
#define __HISI_PCM_DP_H__

#include <linux/regulator/consumer.h>

//******************************************************************************
//REG FIELD
//******************************************************************************
#define HDMI_DMA_A	0
#define HDMI_DMA_B	1

#define HDMI_DMA_A_MASK		(0x1 << HDMI_DMA_A)
#define HDMI_DMA_B_MASK 	(0x1 << HDMI_DMA_B)
#define HDMI_IRQ_MASK		0x7
#define ASP_CFG_HDMI_CLK_SEL_I2S 	(0X1)

#define HISI_MIN_DMA_TIME_US      20000
#define HISI_MAX_DMA_TIME_US      22000

#define HISI_PLL6_RATE_48 			393216000
#define HISI_PLL6_RATE_44			361267200

#define HISI_DIV_CLK_48			0xffff000f
#define HISI_DIV_CLK_96			0xffff0007
#define HISI_DIV_CLK_192			0xffff0003
#define HISI_DIV_CLK_NO_SUPPORT			0x0

#define HISI_WORK_DELAY_1MS   ( 33 )    /* 33 equals 1ms */

/*****************************************************************/

enum HISI_HDMI_STATUS {
	STATUS_HDMI_STOP = 0,
	STATUS_HDMI_RUNNING,
};

struct hisi_dp_data {
	struct regulator_bulk_data regu;
	struct device *dev;
	struct clk *dp_audio_pll_clk;
	struct clk *asp_subsys_clk;
	int irq;
	unsigned int pre_time;
};

struct hisi_dp_runtime_data {
	struct hisi_dp_data *pdata;
	spinlock_t lock;
	struct mutex mutex;
	enum HISI_HDMI_STATUS status;
	unsigned int dma_addr;
	unsigned int period_size;
	unsigned int period_cur;
	unsigned int period_next;
};

#endif
