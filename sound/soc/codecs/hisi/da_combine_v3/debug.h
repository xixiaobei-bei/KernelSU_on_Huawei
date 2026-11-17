/*
 * debug.h
 *
 * debug
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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

#ifndef __DA_COMBINE_V3_DEBUG_H__
#define __DA_COMBINE_V3_DEBUG_H__

#include "hicodec_debug.h"

#define PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR  0x20000000
#define CODEC_BASE_ADDR_PAGE_IO  0x1000
#define CODEC_BASE_ADDR_PAGE_CFG 0x7000
#define CODEC_BASE_ADDR_PAGE_ANA 0x7100
#define CODEC_BASE_ADDR_PAGE_DIG 0x7200

#define DA_COMBINE_V3_DBG_PAGE_IO_CODEC_START  (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_IO)
#define DA_COMBINE_V3_DBG_PAGE_IO_CODEC_END    (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_IO + 0x7c)
#define DA_COMBINE_V3_DBG_PAGE_CFG_CODEC_START (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_CFG)
#define DA_COMBINE_V3_DBG_PAGE_CFG_CODEC_END   (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_CFG + 0xff)
#define DA_COMBINE_V3_DBG_PAGE_ANA_CODEC_START (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_ANA)
#define DA_COMBINE_V3_DBG_PAGE_ANA_CODEC_END   (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_ANA + 0xd3)
#define DA_COMBINE_V3_DBG_PAGE_DIG_CODEC_START (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_DIG)
#define DA_COMBINE_V3_DBG_PAGE_DIG_CODEC_END   (PAGE_DA_COMBINE_V3_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_DIG + 0x3ff)

#endif

