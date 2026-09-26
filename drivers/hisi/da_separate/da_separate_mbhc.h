/*
 * da_separate_mbhc.h -- da_separate mbhc driver
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DA_SEPARATE_MBHC_H__
#define __DA_SEPARATE_MBHC_H__

#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/switch.h>
#include <sound/soc.h>

#ifdef CLT_AUDIO
#define static_t
#else
#define static_t static
#endif

#if defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V3) || \
	defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V5) || \
	(defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V6L)) || \
	defined(CONFIG_HISI_HI6555V500_PMU)
#define DA_SEPARATE_CHARGE_PUMP_CLK_PD CODEC_ANA_RW38_REG
#define DA_SEPARATE_MBHD_VREF_CTRL CODEC_ANA_RW34_REG
#define DA_SEPARATE_HSMICB_CFG CODEC_ANA_RW29_REG
#define IRQ_MASK_OFFSET ANA_IRQ_MASK_0_OFFSET
#else
#define DA_SEPARATE_CHARGE_PUMP_CLK_PD CODEC_ANA_RW37_REG
#define DA_SEPARATE_MBHD_VREF_CTRL CODEC_ANA_RW33_REG
#define DA_SEPARATE_HSMICB_CFG CODEC_ANA_RW28_REG
#define IRQ_MASK_OFFSET ANA_IRQ_MASK_OFFSET
#endif

#define IRQ_MSK_PLUG_OUT (0x1 << (IRQ_MASK_OFFSET + 7))
#define IRQ_MSK_PLUG_IN (0x1 << (IRQ_MASK_OFFSET + 6))
#define IRQ_MSK_ECO_BTN_DOWN (0x1 << (IRQ_MASK_OFFSET + 5))
#define IRQ_MSK_ECO_BTN_UP (0x1 << (IRQ_MASK_OFFSET + 4))
#define IRQ_MSK_COMP_L_BTN_DOWN (0x1 << (IRQ_MASK_OFFSET + 3))
#define IRQ_MSK_COMP_L_BTN_UP (0x1 << (IRQ_MASK_OFFSET + 2))
#define IRQ_MSK_COMP_H_BTN_DOWN (0x1 << (IRQ_MASK_OFFSET + 1))
#define IRQ_MSK_COMP_H_BTN_UP (0x1 << (IRQ_MASK_OFFSET + 0))

#define IRQ_MSK_COMP_H_BTN (IRQ_MSK_COMP_H_BTN_UP | IRQ_MSK_COMP_H_BTN_DOWN)
#define IRQ_MSK_BTN_ECO (IRQ_MSK_ECO_BTN_UP | IRQ_MSK_ECO_BTN_DOWN)
#define IRQ_MSK_BTN_NOR (IRQ_MSK_COMP_L_BTN_UP | IRQ_MSK_COMP_L_BTN_DOWN | IRQ_MSK_COMP_H_BTN)
#define IRQ_MSK_COMP (IRQ_MSK_BTN_NOR | IRQ_MSK_BTN_ECO)
#define IRQ_MSK_HS_ALL (IRQ_MSK_PLUG_OUT | IRQ_MSK_PLUG_IN | IRQ_MSK_COMP)

/* defination of public data */
struct da_separate_mbhc {
};

struct da_separate_mbhc_ops {
	void (*set_ibias_hsmicbias)(struct snd_soc_component *, bool);
	void (*enable_hsd)(struct snd_soc_component *);
};

extern int da_separate_mbhc_init(struct snd_soc_component *codec, struct da_separate_mbhc **mbhc,
	const struct da_separate_mbhc_ops *codec_ops);
extern void da_separate_mbhc_deinit(struct da_separate_mbhc *mbhc);
extern void da_separate_mbhc_set_micbias(struct da_separate_mbhc *mbhc, bool enable);

#endif

