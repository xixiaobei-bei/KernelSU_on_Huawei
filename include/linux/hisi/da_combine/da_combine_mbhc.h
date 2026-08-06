/*
 * da_combine_mbhc.h
 *
 * da_combine mbhc driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#ifndef __DA_COMBINE_MBHC_H__
#define __DA_COMBINE_MBHC_H__

#include <linux/hisi/da_combine/da_combine_irq.h>
#include <linux/hisi/da_combine/da_combine_utils.h>
#include <linux/hisi/da_combine/da_combine_resmgr.h>
#include "da_combine_mbhc_rear_jack.h"

enum audio_jack_states {
	AUDIO_JACK_NONE = 0,     /* unpluged */
	AUDIO_JACK_HEADSET,      /* pluged 4-pole headset */
	AUDIO_JACK_HEADPHONE,    /* pluged 3-pole headphone */
	AUDIO_JACK_INVERT,       /* pluged invert 4-pole headset */
	AUDIO_JACK_EXTERN_CABLE  /* pluged extern cable,such as antenna cable */
};

enum hs_voltage_type {
	VOLTAGE_POLE3 = 0,
	VOLTAGE_POLE4,
	VOLTAGE_PLAY,
	VOLTAGE_VOL_UP,
	VOLTAGE_VOL_DOWN,
	VOLTAGE_VOICE_ASSIST,
	VOLTAGE_EXTERN_CABLE,
	VOLTAGE_TYPE_BUTT
};

enum hs_cfg_type {
	HS_DET_INV = 0,
	HS_CTRL,
	HS_COEFFICIENT,
	HS_HOOK_KEY_MAP,
	HS_IRQ_PM,
	HS_VREF_REG_VALUE,
	HS_IRQ_REG0,
	HS_CFG_BUTT
};

struct voltage_type_node {
	unsigned int min;
	unsigned int max;
};

struct da_combine_mbhc_config {
	/* board defination */
	struct voltage_type_node voltage[VOLTAGE_TYPE_BUTT];
	unsigned int hs_cfg[HS_CFG_BUTT];
	bool hs_report_line_in_out;
	bool hs_support_positive_invert_switch;
	bool hs_detect_extern_cable;
	bool analog_hs_unsupport;
};

struct hs_mbhc_reg {
	unsigned int irq_source_reg;
	unsigned int irq_mbhc_2_reg;
};

struct hs_mbhc_func {
	void (*hs_mbhc_on)(struct snd_soc_component *);
	unsigned int (*hs_get_voltage)(struct snd_soc_component *, unsigned int);
	void (*hs_enable_hsdet)(struct snd_soc_component *, struct da_combine_mbhc_config);
	void (*hs_mbhc_off)(struct snd_soc_component *);
};

struct hs_res_detect_func {
	void (*hs_res_detect)(struct snd_soc_component *);
	void (*hs_path_enable)(struct snd_soc_component *);
	void (*hs_path_disable)(struct snd_soc_component *);
	void (*hs_res_calibration)(struct snd_soc_component *);
};

/* defination of public data */
struct da_combine_mbhc {
};

struct da_combine_hs_cfg {
	struct hs_mbhc_reg *mbhc_reg;
	struct hs_mbhc_func *mbhc_func;
	struct hs_res_detect_func *res_detect_func;
};

struct da_combine_mbhc_cfg {
	struct snd_soc_component *codec;
	struct device_node *node;
	struct da_combine_resmgr *resmgr;
	struct da_combine_irq *irqmgr;
	struct da_combine_mbhc **mbhc;
};

int da_combine_mbhc_init(struct da_combine_mbhc_cfg *mbhc_cfg,
	const struct da_combine_hs_cfg *hs_cfg);
void da_combine_mbhc_deinit(struct da_combine_mbhc *mbhc);
void da_combine_irq_mask_btn_irqs(struct da_combine_mbhc *mbhc);
void da_combine_irq_unmask_btn_irqs(struct da_combine_mbhc *mbhc);
bool da_combine_check_saradc_ready_detection(struct snd_soc_component *codec);
void da_combine_set_audio_jack_headset(const struct da_combine_mbhc *mbhc);
void da_combine_set_audio_jack_invert(const struct da_combine_mbhc *mbhc);
int da_combine_get_4_pole_headset_type(const struct da_combine_mbhc *mbhc);
void da_combine_plug_in_detect_wrapper(struct da_combine_mbhc *mbhc);
void da_combine_plug_out_wrapper(struct da_combine_mbhc *mbhc);
bool da_combine_check_headset_pluged_in(void);
bool da_combine_support_hs_irq_pm(void);
void da_combine_disable_hs_irq(struct da_combine_irq *irqmgr);
void da_combine_enable_hs_irq(struct da_combine_irq *irqmgr);
#endif /* __DA_COMBINE_MBHC_H__ */
