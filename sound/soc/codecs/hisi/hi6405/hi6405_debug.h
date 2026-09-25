/*
 * hi6405_debug.h -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __HI6405_DEBUG_H__
#define __HI6405_DEBUG_H__

#include "hicodec_debug.h"

#define PAGE_6405_CODEC_BASE_ADDR  0x20000000

#define DBG_PAGE_IO_CODEC_START \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_IO)
#define DBG_PAGE_IO_CODEC_END \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_IO + 0x4c)
#define DBG_PAGE_CFG_CODEC_START \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_CFG)
#define DBG_PAGE_CFG_CODEC_END \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_CFG + 0xff)
#define DBG_PAGE_ANA_CODEC_START \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_ANA)
#define DBG_PAGE_ANA_CODEC_END \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_ANA + 0xFF)
#define DBG_PAGE_DIG_CODEC_START \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_DIG)
#define DBG_PAGE_DIG_CODEC_END \
	(PAGE_6405_CODEC_BASE_ADDR + CODEC_BASE_ADDR_PAGE_DIG + 0x43D)
#endif

