/*
 * hi6405_type.h -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef _HI6405_TYPE_H_
#define _HI6405_TYPE_H_

#include <sound/soc.h>
#include "linux/hisi/hi64xx/hi64xx_resmgr.h"
#include "linux/hisi/hi64xx/hi64xx_mbhc.h"
#include "../../../drivers/hisi/slimbus/slimbus.h"

enum track_state {
	TRACK_FREE = 0,
	TRACK_STARTUP = 1,
};

enum classH_state {
	HP_CLASSH_STATE  = 0x1u,  /* hp high mode(classAB) = 0 low mode(classh) = 1 */
	RCV_CLASSH_STATE = 0x2u,  /* classh_rcv_hp_switch true = 1 false =0 */
	HP_POWER_STATE   = 0x4u,  /* hp power on = 1 power off = 0 */
	RCV_POWER_STATE  = 0x8u,  /* rcv power on = 1 power off = 0 */
};

enum sample_rate {
	SAMPLE_RATE_8K = 0u,
	SAMPLE_RATE_16K = 1u,
	SAMPLE_RATE_32K = 2u,
	SAMPLE_RATE_48K = 4u,
	SAMPLE_RATE_96K = 5u,
	SAMPLE_RATE_192K = 6u,
};

enum src_mode {
	SRC_MODE_1_5 = 4u,
	SRC_MODE_2 = 3u,
	SRC_MODE_3 = 0u,
	SRC_MODE_6 = 2u,
	SRC_MODE_12 = 1u,
};

enum hi6405_irq_type {
	HI6405_IRQ_BUNK1_OCP = 40,
	HI6405_IRQ_BUNK1_SCP = 41,
	HI6405_IRQ_CP1_SHORT = 42,
	HI6405_IRQ_CP2_SHORT = 43,
	HI6405_IRQ_LDO_AVDD18_OCP = 44,
	HI6405_IRQ_LDOP_OCP = 45,
	HI6405_IRQ_LDON_OCP = 46,
};

enum delay_type {
	RANGE_SLEEP = 1,
	MSLEEP,
	MDELAY,
	DELAY_BUTT,
};

struct reg_config {
	unsigned int addr;
	unsigned int mask;
	unsigned int val;
	bool update_bits;
};

struct irq_config {
	enum hi6405_irq_type type;
	irq_handler_t handler;
	const char *name;
	bool enabled;
};

struct reg_seq_config {
	struct reg_config cfg;
	enum delay_type type;
	unsigned int us;
};

/* virtual reg begin */
enum codec_virtual_addr {
	VIR_UP = 0x0,
	VIR_DOWN,
	VIR_CNT,
};

struct hi6405_board_cfg {
	int mic_num;
	bool classh_rcv_hp_switch;
#ifdef CONFIG_HAC_SUPPORT
	int hac_gpio;
#endif
	bool wakeup_hisi_algo_support;
	bool hp_res_detect_enable;
	bool mic_control_sc_freq_enable;
};

struct hi6405_platform_data {
	struct snd_soc_codec *codec;
	struct device_node *node;
	struct hi_cdc_ctrl *cdc_ctrl;
	struct hi64xx_resmgr *resmgr;
	struct hi64xx_irq *irqmgr;
	struct hi64xx_mbhc *mbhc;
	struct hi64xx_mbhc_config mbhc_config;
	struct hi6405_board_cfg board_config;
	spinlock_t v_rw_lock;
	struct mutex impdet_dapm_mutex;
	unsigned int virtual_reg[VIR_CNT];
	bool hsl_power_on;
	bool hsr_power_on;
	int dp_clk_num;
	int cp1_num;
	int cp2_num;
	slimbus_track_param_t voice_up_params;
	slimbus_track_param_t voice_down_params;
	slimbus_track_param_t play_params;
	slimbus_track_param_t capture_params;
	slimbus_track_param_t pa_iv_params;
	slimbus_track_param_t soundtrigger_params;
	enum track_state voiceup_state;
	enum track_state audioup_4mic_state;
	enum classH_state rcv_hp_classH_state;
#ifdef AUDIO_FACTORY_MODE
	int mainmicbias_val;
#endif

};
void write_reg_array(struct snd_soc_codec *codec,
	const struct reg_config *reg_array, size_t len);
void write_reg_seq_array(struct snd_soc_codec *codec,
	const struct reg_seq_config *reg_seq_array, size_t len);
#endif

