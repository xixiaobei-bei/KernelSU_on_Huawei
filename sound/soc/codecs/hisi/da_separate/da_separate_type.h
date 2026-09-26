/*
 * da_separate_type.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_TYPE_H__
#define __DA_SEPARATE_TYPE_H__

#include <linux/platform_device.h>
#include "da_separate_mbhc.h"

enum codec_virtual_addr {
	VIR0 = 0x0,
	VIR1,
	VIR_CNT,
};

#define VIRCODEC_START 0
#define VIRCODEC_END (VIR_CNT - 1)

#define MAX_VAL_ON_BIT(bit) ((0x1 << (bit)) - 1)
#define MASK_ON_BIT(bit, offset) (MAX_VAL_ON_BIT(bit) << (offset))

#define HAC_ENABLE 1
#define GPIO_LEVEL_HIGH 1
#define GPIO_LEVEL_LOW 0
#define PGA_GAIN_STEP 200 /* unit:0.01db */
#define CLEAR_FIFO_DELAY_LEN_MS 1

#define MICBISA1_SET_MASK       0x22
#define MICBISA2_SET_MASK       0x11

/* virtual reg */
#define VIR0_ADDR        ((PAGE_VIRCODEC) + (VIR0))
#define VIR1_ADDR        ((PAGE_VIRCODEC) + (VIR1))

enum da_separate_board_type {
	FPGA_BOARD = 0,
	UDP_BOARD,
	BOARD_TYPE_CNT,
};

enum da_separate_reg_type {
	SOCCODEC = 0,
	ASPCFG,
	AOIOC,
	REG_TYPE_CNT
};

enum pm_runtime_state {
	PM_RUNTIME_SUSPEND,
	PM_RUNTIME_RESUME,
	PM_RUNTIME_BUTT,
};

/* da_separate codec modules */
enum {
	DA_SEPARATE_AUDIO_PATH = 0,
	DA_SEPARATE_BTFM_PATH,
	DA_SEPARATE_CORE_PATH,
	DA_SEPARATE_I2S,
	DA_SEPARATE_MAD_PATH,
	DA_SEPARATE_SMPA_PATH,
	DA_SEPARATE_THIRD_CODEC_PATH,
	DA_SEPARATE_ULTR_PATH,
	DA_SEPARATE_VOICE_PATH,
	DA_SEPARATE_SUB_MODULE_CNT
};

/* customized codec ops type */
enum {
	USB_ANA_LDO_CTL = 0,
};

/* customized codec ops command */
enum {
	STATE_ENABLE = 0,
	STATE_DISABLE,
};

struct platform_data;

struct reg_page {
	unsigned int page_tag;
	unsigned int page_reg_begin;
	unsigned int page_reg_end;
	const char  *page_name;
};

struct codec_pinctrl {
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
};

struct codec_resource {
	struct codec_pinctrl pin;
	struct regulator *asp_regulator;
	struct regulator *pmu_regulator;
	struct clk *asp_subsys_clk;
	struct clk *asp_codec_clk;
	void (*set_classd_supply)(struct snd_soc_component *, bool);
};

struct ibias_data {
	struct mutex ibias_mutex;
	int ibias_work;
	void (*set_ibias)(struct snd_soc_component *, bool);
	bool ibias_hsmicbias_en;
};

struct codec_ops {
	int (*codec_init)(struct snd_soc_component *);
	void (*codec_deinit)(struct snd_soc_component *);
	void (*shutdown)(struct platform_device *);
	void (*pm_asp_reg_init)(struct snd_soc_component *codec);
	struct da_separate_mbhc_ops *mbhc_ops;
	void (*codec_customized_func)(struct platform_data *priv,
		u32 sub_module, u32 key, u32 val);
};

struct codec_pm {
	bool have_dapm;
	bool asp_pd;
	bool pm_runtime_support;
	int regulator_pu_count;
	int clk_pu_count;
};

struct platform_data {
	struct snd_soc_component *codec;
	struct da_separate_mbhc *mbhc;
	struct codec_pm pm;
	struct codec_ops ops;
	struct ibias_data ibias;
	struct codec_resource resource;

	spinlock_t reg_access_lock;
	enum da_separate_board_type board_type;
	unsigned int virtual_reg[VIR_CNT];
	unsigned int gpio_classd;
	unsigned int hac_en_gpio;
	bool classh_clk_enable;
	bool pmu_invalid;

	const char **path_name;
	unsigned int path_name_num;
	void *customized_data;
};

struct snd_codec_resource {
	const struct snd_kcontrol_new *controls;
	unsigned int num_controls;
	const struct snd_soc_dapm_widget *widgets;
	unsigned int num_widgets;
	const struct snd_soc_dapm_route *routes;
	unsigned int num_routes;
};
#endif

