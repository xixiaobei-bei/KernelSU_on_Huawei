/*
 * da_combine_v5.h
 *
 * da combine v5 codec driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DA_COMBINE_V5_H__
#define __DA_COMBINE_V5_H__

#include "da_combine_v5_type.h"

#define INVALID_REG 0xE000
#define INVALID_REG_VALUE 0xFFFFFFFF

#define DA_COMBINE_V5_PB_MIN_CHANNELS (2)
#define DA_COMBINE_V5_PB_MAX_CHANNELS (2)
#define DA_COMBINE_V5_CP_MIN_CHANNELS (1)
#define DA_COMBINE_V5_CP_MAX_CHANNELS (6)
#define DA_COMBINE_V5_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S16_BE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S24_BE)
#define DA_COMBINE_V5_RATES SNDRV_PCM_RATE_8000_384000


#define LOG_TAG "DA_combine_v5"


#define MAX_VAL_ON_BIT(bit) ((0x1 << (bit)) - 1)
#define MASK_ON_BIT(bit, offset) (MAX_VAL_ON_BIT(bit) << (offset))

#define CODEC_BASE_ADDR_PAGE_IO  0x1000
#define CODEC_BASE_ADDR_PAGE_CFG 0x7000
#define CODEC_BASE_ADDR_PAGE_ANA 0x7100
#define CODEC_BASE_ADDR_PAGE_DIG 0x7200

#ifdef CONFIG_HISI_DIEID
int da_combine_v5_codec_get_dieid(char *dieid, unsigned int len);
#endif

#endif /* __DA_COMBINE_V5_H__ */
