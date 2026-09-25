/*
 * hi6405.h -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __HI6405_H__
#define __HI6405_H__

#include "hi6405_type.h"

#define INVALID_REG 0xE000
#define INVALID_REG_VALUE 0xFFFFFFFF

#define HI6405_PB_MIN_CHANNELS (2)
#define HI6405_PB_MAX_CHANNELS (2)
#define HI6405_CP_MIN_CHANNELS (1)
#define HI6405_CP_MAX_CHANNELS (6)
#define HI6405_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S16_BE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S24_BE)
#define HI6405_RATES SNDRV_PCM_RATE_8000_384000

#define LOG_TAG "hi6405"

#define MAX_VAL_ON_BIT(bit) ((0x1 << (bit)) - 1)
#define MASK_ON_BIT(bit, offset) (MAX_VAL_ON_BIT(bit) << (offset))

#define CODEC_BASE_ADDR_PAGE_IO  0x1000
#define CODEC_BASE_ADDR_PAGE_CFG 0x7000
#define CODEC_BASE_ADDR_PAGE_ANA 0x7100
#define CODEC_BASE_ADDR_PAGE_DIG 0x7200

#ifdef CONFIG_HISI_DIEID
int hi6405_codec_get_dieid(char *dieid, unsigned int len);
#endif

#endif /* __HI6405_H__ */
