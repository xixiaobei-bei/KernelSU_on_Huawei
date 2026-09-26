/*
 * asp_hdmi_dma.h -- asp hdmi dma driver
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASP_HDMI_DMA_H__
#define __ASP_HDMI_DMA_H__

/* ASP_HDMI base addr */
#define ASP_HDMI_DMA_BASE                            (0xE804E400 - 0xE804E400)

/******************************************************************************/
#define ASP_HDMI_TX3                              (ASP_HDMI_DMA_BASE + 0x0)
#define ASP_HDMI_DMA_EN                           (ASP_HDMI_DMA_BASE + 0x4)
#define ASP_HDMI_DMA_STOP                         (ASP_HDMI_DMA_BASE + 0x8)
#define ASP_HDMI_GLB_INT_STATE                    (ASP_HDMI_DMA_BASE + 0xC)
#define ASP_HDMI_INT_STATE                        (ASP_HDMI_DMA_BASE + 0x10)
#define ASP_HDMI_INT_EN                           (ASP_HDMI_DMA_BASE + 0x14)
#define ASP_HDMI_INT_MSK_STATE                    (ASP_HDMI_DMA_BASE + 0x18)
#define ASP_HDMI_INT_CLR                          (ASP_HDMI_DMA_BASE + 0x1C)
#define ASP_HDMI_PCM_NEW_SNG                      (ASP_HDMI_DMA_BASE + 0x20)
#define ASP_HDMI_RSMP                             (ASP_HDMI_DMA_BASE + 0x24)
#define ASP_HDMI_P30CGR                           (ASP_HDMI_DMA_BASE + 0x28)
#define ASP_HDMI_P31CGR                           (ASP_HDMI_DMA_BASE + 0x2C)
#define ASP_HDMI_P32CGR                           (ASP_HDMI_DMA_BASE + 0x30)
#define ASP_HDMI_P33CGR                           (ASP_HDMI_DMA_BASE + 0x34)
#define ASP_HDMI_P34CGR                           (ASP_HDMI_DMA_BASE + 0x38)
#define ASP_HDMI_P35CGR                           (ASP_HDMI_DMA_BASE + 0x3C)
#define ASP_HDMI_P36CGR                           (ASP_HDMI_DMA_BASE + 0x40)
#define ASP_HDMI_P37CGR                           (ASP_HDMI_DMA_BASE + 0x44)
#define ASP_HDMI_A_ADDR                           (ASP_HDMI_DMA_BASE + 0x48)
#define ASP_HDMI_A_LEN                            (ASP_HDMI_DMA_BASE + 0x4C)
#define ASP_HDMI_B_ADDR                           (ASP_HDMI_DMA_BASE + 0x50)
#define ASP_HDMI_B_LEN                            (ASP_HDMI_DMA_BASE + 0x54)
#define ASP_HDMI_SPDIF_SEL                        (ASP_HDMI_DMA_BASE + 0x58)
#define ASP_HDMI_A_LEN_STATE                      (ASP_HDMI_DMA_BASE + 0x5C)
#define ASP_HDMI_B_LEN_STATE                      (ASP_HDMI_DMA_BASE + 0x60)
#define ASP_HDMI_FDS                              (ASP_HDMI_DMA_BASE + 0x64)
#define ASP_HDMI_A_LEN_LFT                        (ASP_HDMI_DMA_BASE + 0x68)
#define ASP_HDMI_B_LEN_LFT                        (ASP_HDMI_DMA_BASE + 0x6C)
#define ASP_HDMI_SECURE                           (ASP_HDMI_DMA_BASE + 0x70)

#define ASP_HDMI_I2S_SET                          (ASP_HDMI_DMA_BASE + 0x41c)
#define ASP_HDMI_I2S_CLR                          (ASP_HDMI_DMA_BASE + 0x420)

#define ASP_HDMI_SIO_CH_DATA_VLD		(ASP_HDMI_DMA_BASE + 0x444)
#define ASP_HDMI_SIO_CH0_STATUS1_L	 (ASP_HDMI_DMA_BASE + 0x448)
#define ASP_HDMI_SIO_CH0_STATUS2_L	 (ASP_HDMI_DMA_BASE + 0x44C)
#define ASP_HDMI_SIO_CH0_STATUS1_R	 (ASP_HDMI_DMA_BASE + 0x478)
#define ASP_HDMI_SIO_CH0_STATUS2_R	 (ASP_HDMI_DMA_BASE + 0x47C)

#define ASP_HDMI_SPDIF_CH0_STATUS1_L	(ASP_HDMI_DMA_BASE + 0x820)
#define ASP_HDMI_SPDIF_CH0_STATUS2_L	(ASP_HDMI_DMA_BASE + 0x824)
#define ASP_HDMI_SPDIF_CH0_STATUS1_R	(ASP_HDMI_DMA_BASE + 0x850)
#define ASP_HDMI_SPDIF_CH0_STATUS2_R	(ASP_HDMI_DMA_BASE + 0x854)

/* spdif_ch 0-3, 1 spdif_ch for 2 audio channels */
#define ASP_HDMI_SPDIF_CH_STATUS1_L(spdif_ch)	(ASP_HDMI_SPDIF_CH0_STATUS1_L + (spdif_ch) * 0x60)
#define ASP_HDMI_SPDIF_CH_STATUS2_L(spdif_ch)	(ASP_HDMI_SPDIF_CH0_STATUS2_L + (spdif_ch) * 0x60)
#define ASP_HDMI_SPDIF_CH_STATUS1_R(spdif_ch)	(ASP_HDMI_SPDIF_CH0_STATUS1_R + (spdif_ch) * 0x60)
#define ASP_HDMI_SPDIF_CH_STATUS2_R(spdif_ch)	(ASP_HDMI_SPDIF_CH0_STATUS2_R + (spdif_ch) * 0x60)

#define ASP_HDMI_SPDIF_CTRL       	(ASP_HDMI_DMA_BASE + 0x800)
#define ASP_HDMI_SPDIF_CONFIG       (ASP_HDMI_DMA_BASE + 0x804)

#define HDMI_DMA_EN_MASK			0x3
#define HDMI_DMA_DISABLE_MASK		0x0
#define HDMI_INT_MASK				0x7
#define HDMI_I2S_SET_MASK			0x901BC
#define HDMI_I2S_CLR_MASK			0x9000C
#define HDMI_TX3_EN_MASK			0x1
#define HDMI_SIO_CONF_MASK		0x0
#define HDMI_SIO_BITWIDTH_MASK	0x2
#define HDMI_SIO_BITWIDTH_24BIT	0xB

#define HDMI_SPDIF_SAMPLE_RATE_MASK	GENMASK(27, 24)
#define HDMI_SPDIF_SAMPLE_RATE_SHIFT	(24)
#define HDMI_SPDIF_CHANNEL_TYPE_MASK		GENMASK(23, 20)
#define HDMI_SPDIF_CHANNEL_TYPE_SHIFT	(20)

#define HDMI_SPDIF_ORIGINAL_SAMPLE_RATE_MASK	GENMASK(7, 4)
#define HDMI_SPDIF_ORIGINAL_SAMPLE_RATE_SHIFT	(4)
#define HDMI_SPDIF_BITWIDTH_16BIT	0x2
#define HDMI_SPDIF_BITWIDTH_24BIT	0xB
#define HDMI_SPDIF_BITWIDTH_MASK	GENMASK(3, 0)
#define HDMI_SPDIF_BITWIDTH_SHIFT	(0)

#define HDMI_SPDIF_SET_MASK	0x3C1

#define HDMI_TX3_EN_BIT	(0)
#define HDMI_A_INT_EN_BIT	(0)
#define HDMI_B_INT_EN_BIT	(1)
#define HDMI_BUS_ERR_EN_BIT	(2)

#define HDMI_PCM_SWITCH_ORDE_BIT		(13)
#define HDMI_PCM_SWITCH_ORDE_MASK	0x1

#define HDMI_SIO_SAMPLE_RATE_BIT	(24)
#define HDMI_SIO_HDCP_BIT	(2)
#define HDMI_SIO_CHANNEL_TYPE_BIT		(20)
#define HDMI_SIO_BITWIDTH_BIT	(0)

#define HDMI_DMA_ADD_VALID_MASK	0x3
#define HDMI_DMA_ADDLEN_VALID_MASK	0x1F

/* TX3 config */
enum tx3_channel_num {
	CHANNEL_NUM_1 = 0,
	CHANNEL_NUM_2,
	CHANNEL_NUM_4,
	CHANNEL_NUM_6,
	CHANNEL_NUM_8,
	CHANNEL_NUM_MAX,
};

enum tx3_bit_width {
	BIT_WIDTH_16 = 0,
	BIT_WIDTH_17,
	BIT_WIDTH_18,
	BIT_WIDTH_19,
	BIT_WIDTH_20,
	BIT_WIDTH_21,
	BIT_WIDTH_22,
	BIT_WIDTH_23,
	BIT_WIDTH_24,
	BIT_WIDTH_MAX,
};

enum tx3_align_type {
	ALIGN_16 = 0,
	ALIGN_32,
	ALIGN_MAX,
};

enum sio_sample_rate {
	SAMPLE_RATE_DEFULT = 0,
	SAMPLE_RATE_32 = SAMPLE_RATE_DEFULT,
	SAMPLE_RATE_44,
	SAMPLE_RATE_48,
	SAMPLE_RATE_88,
	SAMPLE_RATE_96,
	SAMPLE_RATE_176,
	SAMPLE_RATE_192,
	SAMPLE_RATE_MAX,
	SAMPLE_RATE_NO_SUPPORT = SAMPLE_RATE_MAX,
};

typedef struct tx3_config_parameters {
	unsigned int channel_num;	//tx3_channel_num
	unsigned int bit_width;	//tx3_bit_width
	unsigned int align_type;//tx3_align_type
} tx3_config_parameters;

typedef struct sio_config_parameters {
	tx3_config_parameters *tx3_conf;
	unsigned int sample_rate;//sio_sample_rate
	unsigned int div_clk;//sio_div_clk
	bool is_hdcp; //is data copyright protection
} sio_config_parameters;

/**
 * @Description:根据传输的配置参数，配置HDMI TX3
 * @param parames - 配置参数结构体.
 * @return 成功：0，失败：-1
 */
int asp_hdmi_tx3_config(struct tx3_config_parameters parameters);

/**
 * @Description:配置DMA
 * @param addr - DMA读取数据的BUFFER起始地址
 * @param size - 一个BUFFER的大小，注意，HDMI DMA使用AB BUFFER,所以size只代表A/B BUFFER的大小
 * @return 成功：0，失败：<0
 */
int asp_hdmi_dma_config(unsigned int addr, unsigned int size);

/**
 * @Description:ASP HDMI中断清零
 * @param value - reg vaule
 * @return
 */
void asp_hdmi_dma_clear_interrupt(unsigned int value);

/**
 * @Description:读取中断原始状态寄存器
 * @return: 寄存器值
 */
unsigned int asp_hdmi_reg_read_irsr(void);

/**
 * @Description:使能DMA
 * @return: 寄存器值
 */
void asp_hdmi_tx3_enable(void);

void asp_hdmi_tx3_disable(void);

bool asp_hdmi_dma_is_stop(void);

void asp_hdmi_dma_enable(void);

int asp_hdmi_dma_start(void);

void asp_hdmi_dma_stop(void);

/**
 * @Description:config sio reg with parames, if number of  parames value is  0,reg value is default
 * @param value - see struct sio_config_parames
 */
void asp_hdmi_spdif_config(struct sio_config_parameters parameters);
#endif

