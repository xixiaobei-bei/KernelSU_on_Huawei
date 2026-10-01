/*
 * hi6555v2_driver codec driver.
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HI6555V2_H__
#define __HI6555V2_H__

#include "hi6555v2_utility.h"

enum hi6555v2_codec_virtual_addr{
	HI6555V2_VIR0 = 0x0,
	HI6555V2_VIR1,
	HI6555V2_VIR_CNT,
};

/* virtual reg */
#define HI6555V2_DDR_CODEC_VIR0_ADDR        ((PAGE_VIRCODEC) + (HI6555V2_VIR0))
#define HI6555V2_DDR_CODEC_VIR1_ADDR        ((PAGE_VIRCODEC) + (HI6555V2_VIR1))

#define HI6555V2_HS_INIT_STATUS (0)
#define HI6555V2_HS_DETECTING (1<<2)
#define HI6555V2_HS_WITH_MIC (1<<0)
#define HI6555V2_HS_WITHOUT_MIC (1<<1)


#define HI6555V2_PB_MIN_CHANNELS (2)
#define HI6555V2_PB_MAX_CHANNELS (2)
#define HI6555V2_CP_MIN_CHANNELS (1)
#define HI6555V2_CP_MAX_CHANNELS (6)
#define HI6555V2_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)
#define HI6555V2_RATES SNDRV_PCM_RATE_8000_384000

/* catch size */
#define REG_CACHE_NUM_MAX       1024
#define REG_CACHE_FLAG_WRITE    (0x01 << 16)
#define REG_CACHE_FLAG_READ     (0x02 << 16)
#define REG_CACHE_FLAG_MASK     (0xFF << 16)

#define HI6555V2_DBG_SIZE_PAGE         4096
#define HI6555V2_DBG_SIZE_WIDGET       8192
#define HI6555V2_DBG_SIZE_PATH         16384
#define HI6555V2_DBG_SIZE_CACHE        (24*REG_CACHE_NUM_MAX)

enum hi6555c_platform_type {
	FPGA_PLATFORM = 0,
	UDP_PLATFORM,
	PLATFORM_CNT,
};

#define DACL_SRC2_MODE_MASK  0x7000000
#define DACL_SRC2_MASK 0x1800
#define HPMICBISA_SET_MASK	0x44

#define MICBISA1_SET_MASK	0x22
#define MICBISA2_SET_MASK	0x11

#define MAINPGA_MIC_IN_MASK	0x30
#define MAINPGA_MIC_IN_MAIN	0x10
#define MAINPGA_MIC_IN_HP	0x20

#define HPL_MIXIN_VALUE_MASK	0xC0
#define HPR_MIXIN_VALUE_MASK	0xC
#define EAR_MIXIN_VALUE_MASK	0xC0

#define ADC1_MIXIN_VALUE_MASK	0x33
#define ADC2_MIXIN_VALUE_MASK	0x33
#define ADC3_MIXIN_VALUE_MASK	0x33

#define MUX1_DIN_MASK	0xC
#define MUX2_DIN_MASK	0x30
#define MUX3_DOUT_MASK	0x3
#define MUX4_DOUT_MASK	0xC
#define MUX5_DOUT_MASK	0x30

#define ANA_DACLR_EN_MASK	0x3
#define ANA_CLASSD_EN_MASK 0x9

#define CLASSD_DEFAULT_VOLTAGE    4500      /* default classd voltage config */
#define CLASSD_MIN_VOLTAGE        4500      /* min voltage config */
#define CLASSD_MAX_VOLTAGE        5500      /* here we set 5.5V as the max value */
#define CLASSD_VOLTAGE_DIV		100

#define MUTE_DAC_VALUE_MASK	0xE00000
#define MUTE_DAC_VALUE 		0xE00000

#define MAX_APS_CLK_COUNT		0x2

enum hi6555v2_classd_supply_type{
	CLASSD_SUPPLY_SCHARGER,
	CLASSD_SUPPLY_GPIO,
	CLASSD_SUPPLY_INVALID,
};

struct hi6555v2_reg_page {
	unsigned int page_tag;
	unsigned int page_reg_begin;
	unsigned int page_reg_end;
	const char  *page_name;
};

struct tool_priv {
	unsigned int normal_always;
	unsigned int reg;
	unsigned int value;
	int codec_soc_clk_cnt;
	int scharge_boost_cnt;
};

struct platform_priv {
	struct device *platform_dev;
	bool pm_runtime_support;
};

struct hi6555v2_priv {
	struct snd_soc_codec *codec;
	struct hi6xxx_mbhc *mbhc;
	bool have_dapm;
	bool asp_pd;
	int regulator_pu_count;
	int clk_pu_count;
	spinlock_t lock;
	struct mutex ibias_mutex;
	int ibias_work;
	bool ibias_hsmicbias_en;
	enum hi6555c_platform_type platform_type;
	unsigned int v_codec_reg[HI6555V2_VIR_CNT]; /* virtual codec register on ddr */

	struct regulator *asp_regulator;
	struct regulator *codec_ldo8;
	struct regulator *regulator_schg_boost3;
	struct clk *asp_subsys_clk;
	struct clk *asp_49m_clk;
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;

	bool pm_runtime_support;
	bool need_set_sdm_idel_tone_ctl;
	enum hi6555v2_classd_supply_type classd_supply_type;
	unsigned int gpio_classd;
	unsigned int mute_dacl_reg_val;
	unsigned int mute_dacr_reg_val;
	int classd_scharger_voltage;
};

extern struct snd_soc_codec *hi6555v2_get_codec(void);
extern struct platform_priv *hi6555v2_get_platform_dev(void);
extern void ibias_hsmicbias_enable(struct snd_soc_codec *codec, bool enable);
extern int scharger_flash_bst_vo_config(int config_voltage);

#endif /* __HI6555V2_H__ */

