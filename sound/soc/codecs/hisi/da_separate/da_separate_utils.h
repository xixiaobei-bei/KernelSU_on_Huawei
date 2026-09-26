/*
 * da_separate_utils.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_UTILS_H__
#define __DA_SEPARATE_UTILS_H__

#include <sound/soc.h>
#include <linux/version.h>

#include "da_separate_type.h"
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hicodec_debug.h"
#endif

#ifdef CONFIG_SND_SOC_CODEC_DA_SEPARATE_V3
#include "da_separate_v3.h"
#endif

#if (defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V5)) || (defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V6L))
#include "da_separate_v5.h"
#endif

#include "da_separate_reg.h"

enum reg_bit_type {
	REG_BIT0 = 0,
	REG_BIT1,
	REG_BIT2,
	REG_BIT3,
	REG_BIT4,
	REG_BIT5,
	REG_BIT6,
	REG_BIT7,
	REG_BIT8,
	REG_BIT9,
	REG_BIT10,
	REG_BIT11,
	REG_BIT12,
	REG_BIT13,
	REG_BIT14,
	REG_BIT15,
	REG_BIT16,
	REG_BIT17,
	REG_BIT18,
	REG_BIT19,
	REG_BIT20,
	REG_BIT21,
	REG_BIT22,
	REG_BIT23,
	REG_BIT24,
	REG_BIT25,
	REG_BIT26,
	REG_BIT27,
	REG_BIT28,
	REG_BIT29,
	REG_BIT30,
	REG_BIT31,
};

#define ALSA_REG_MASK                0x7FFFFFFF
#define PAGE_SoCCODEC                (ASP_CODEC_REG_BASE & ALSA_REG_MASK)
#define PAGE_ASPCFG                  (ASP_CFG_REG_BASE & ALSA_REG_MASK)
#define PAGE_AO_IOC                  (ASP_AO_IOC_BASE & ALSA_REG_MASK)
#define PAGE_VIRCODEC                0x8000
#define PAGE_TYPE_MASK               0x7FFFF000
#define PAGE_VALUE_MASK              0x00000FFF

#define INVALID_REG_VALUE 0xFFFFFFFF
#define MAX_UINT32 0xFFFFFFFF
#define MAX_INT32 0x7FFFFFFF

enum reg_delay_type {
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

struct reg_seq_config {
	struct reg_config cfg;
	enum reg_delay_type type;
	unsigned int us;
};

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
struct hicodec_dump_reg_info *da_separate_reg_get_dump_info(void);
#endif

unsigned int da_separate_reg_read(struct snd_soc_component *codec, unsigned int reg);
int da_separate_reg_write(struct snd_soc_component *codec, unsigned int reg,
	unsigned int value);
void da_separate_reg_seq_write_array(struct snd_soc_component *codec,
	const struct reg_seq_config *reg_seq_array, size_t len);

int da_separate_reg_addr_map(struct platform_device *pdev);
void da_separate_reg_addr_unmap(void);

int da_separate_asp_regulator_enable(struct snd_soc_component *codec);
int da_separate_asp_clk_enable(struct snd_soc_component *codec);
int da_separate_asp_resource_enable(struct snd_soc_component *codec);
void da_separate_asp_regulator_disable(struct snd_soc_component *codec);
void da_separate_asp_clk_disable(struct snd_soc_component *codec);
void da_separate_asp_resource_disable(struct snd_soc_component *codec);

int da_separate_add_codec_resource(struct snd_soc_component *codec, const struct snd_codec_resource *resource);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
unsigned int da_separate_reg_read_by_codec(struct snd_soc_codec *codec, unsigned int reg);
int da_separate_reg_write_by_codec(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value);
#endif

#endif

