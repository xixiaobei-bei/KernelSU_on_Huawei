/*
 * hi6555_utils.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HI6555_UTILITY_H_
#define _HI6555_UTILITY_H_

#include <sound/soc.h>
#include "hi6555_type.h"
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hicodec_debug.h"
#endif

#ifdef CONFIG_SND_SOC_CODEC_HI6555V3
#include "hi6555v3.h"
#endif

#ifdef CONFIG_SND_SOC_CODEC_HI6555V5
#include "hi6555v5.h"
#endif

#include "hi6555_reg.h"

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
struct hicodec_dump_reg_info *hi6555_reg_get_dump_info(void);
#endif

unsigned int hi6555_reg_read(struct snd_soc_codec *codec, unsigned int reg);
int hi6555_reg_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value);
void hi6555_reg_seq_write_array(struct snd_soc_codec *codec,
	const struct reg_seq_config *reg_seq_array, size_t len);

int hi6555_reg_addr_map(struct platform_device *pdev);
void hi6555_reg_addr_unmap(void);

int hi6555_asp_regulator_enable(struct snd_soc_codec *codec);
int hi6555_asp_clk_enable(struct snd_soc_codec *codec);
int hi6555_asp_resource_enable(struct snd_soc_codec *codec);
void hi6555_asp_regulator_disable(struct snd_soc_codec *codec);
void hi6555_asp_clk_disable(struct snd_soc_codec *codec);
void hi6555_asp_resource_disable(struct snd_soc_codec *codec);

int hi6555_add_codec_resource(struct snd_soc_codec *codec, const struct snd_codec_resource *resource);
#endif

