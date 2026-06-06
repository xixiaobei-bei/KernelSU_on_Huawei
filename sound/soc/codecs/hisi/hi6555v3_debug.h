/*
 * hi6555v3_driver codec driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HI6555V3_DEBUG_H__
#define __HI6555V3_DEBUG_H__

#include "hi6555v3_utility.h"
#include "hicodec_debug.h"

#define PAGE_SocCODEC_BASE_ADDR  0xe8052000
#define PAGE_PmuCODEC_BASE_ADDR  0x000
#define PAGE_PmuHKADC_BASE_ADDR  0x000
#define PAGE_PmuCTRL_BASE_ADDR   0x000

#define HI6555V3_DBG_SOC_CODEC_START (PAGE_SocCODEC_BASE_ADDR + HI6555V3_SoCCODEC_START)
#define HI6555V3_DBG_SOC_CODEC_END   (PAGE_SocCODEC_BASE_ADDR + HI6555V3_SoCCODEC_END)
#define HI6555V3_DBG_PMU_CODEC_START (PAGE_PmuCODEC_BASE_ADDR + HI6555V3_PMUCODEC_START)
#define HI6555V3_DBG_PMU_CODEC_END   (PAGE_PmuCODEC_BASE_ADDR + HI6555V3_PMUCODEC_END)

static struct hicodec_dump_reg_entry hi6555v3_dump_table[] = {
	{"SOC CODEC", HI6555V3_DBG_SOC_CODEC_START, HI6555V3_DBG_SOC_CODEC_END, 4},
	{"PMU CODEC", HI6555V3_DBG_PMU_CODEC_START, HI6555V3_DBG_PMU_CODEC_END, 1},
};

static struct hicodec_dump_reg_info hi6555v3_dump_info = {
	.entry = hi6555v3_dump_table,
	.count = sizeof(hi6555v3_dump_table) / sizeof(struct hicodec_dump_reg_entry),
};

#endif

