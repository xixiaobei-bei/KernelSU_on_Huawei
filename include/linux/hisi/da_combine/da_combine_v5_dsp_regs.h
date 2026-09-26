/*
 * da_combine_v5_dsp_regs.h
 *
 * Copyright (c) 2013-2014 Huawei Technologies CO., Ltd.All rights reserved.
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

#ifndef __DA_COMBINE_V5_DSP_REGS_H__
#define __DA_COMBINE_V5_DSP_REGS_H__

#include "da_combine_dsp_regs.h"

#define DA_COMBINE_V5_OCRAM_BASE_ADDR             (0x10100000)
#define DA_COMBINE_V5_ITCM_BASE_ADDR              (0x08000000)
#define DA_COMBINE_V5_DTCM_BASE_ADDR              (0x08010000)

#define DA_COMBINE_V5_OCRAM_SIZE                  (0x58000)
#define DA_COMBINE_V5_ITCM_SIZE                   (0x5000)
#define DA_COMBINE_V5_DTCM_SIZE                   (0x20000)

#define DA_COMBINE_V5_RINGBUFFER_SIZE             (0x10000)    /* sample rate:16K resolution:16bit 2s*/
#define DA_COMBINE_V5_AP_TO_DSP_MSG_SIZE          (0x80)
#define DA_COMBINE_V5_AP_DSP_CMD_SIZE             (0x40)
#define DA_COMBINE_V5_DSP_TO_AP_MSG_SIZE          (0x80)
#define DA_COMBINE_V5_MLIB_PARA_MAX_SIZE          (0x1000)
#define DA_COMBINE_V5_DUMP_PANIC_STACK_SIZE       (0x200)
#define DA_COMBINE_V5_DUMP_CPUVIEW_SIZE           (0x200)
#define DA_COMBINE_V5_SAVE_LOG_SIZE               (0x800)
#define DA_COMBINE_V5_SAVE_LOG_SIZE_ES            (0x4000)

#define DA_COMBINE_V5_OCRAM1_BASE                  0x10146340
#define DA_COMBINE_V5_MLIB_TO_AP_MSG_ADDR          (DA_COMBINE_V5_OCRAM1_BASE + 0x19874)
#define DA_COMBINE_V5_MLIB_TO_AP_MSG_SIZE          (588)

#define DA_COMBINE_V5_RINGBUFFER_ADDR             (DA_COMBINE_V5_OCRAM_BASE_ADDR)
#define DA_COMBINE_V5_AP_TO_DSP_MSG_ADDR          (DA_COMBINE_V5_RINGBUFFER_ADDR + DA_COMBINE_V5_RINGBUFFER_SIZE)
#define DA_COMBINE_V5_AP_DSP_CMD_ADDR             (DA_COMBINE_V5_AP_TO_DSP_MSG_ADDR + DA_COMBINE_V5_AP_TO_DSP_MSG_SIZE)
#define DA_COMBINE_V5_DSP_TO_AP_MSG_ADDR          (DA_COMBINE_V5_AP_DSP_CMD_ADDR + DA_COMBINE_V5_AP_DSP_CMD_SIZE)
#define DA_COMBINE_V5_MLIBPARA_ADDR               (DA_COMBINE_V5_DSP_TO_AP_MSG_ADDR + DA_COMBINE_V5_DSP_TO_AP_MSG_SIZE)
#define DA_COMBINE_V5_DUMP_PANIC_STACK_ADDR       (DA_COMBINE_V5_MLIBPARA_ADDR + DA_COMBINE_V5_MLIB_PARA_MAX_SIZE)
#define DA_COMBINE_V5_DUMP_CPUVIEW_ADDR           (DA_COMBINE_V5_DUMP_PANIC_STACK_ADDR + DA_COMBINE_V5_DUMP_PANIC_STACK_SIZE)
#define DA_COMBINE_V5_DSP_MSG_STATE_ADDR          (DA_COMBINE_V5_DUMP_PANIC_STACK_ADDR + 0x18)
#define DA_COMBINE_V5_SAVE_LOG_ADDR               (0x0802f800)

#define DA_COMBINE_V5_CMD0_ADDR                   (DA_COMBINE_V5_AP_DSP_CMD_ADDR + 0x004 * 0)
#define DA_COMBINE_V5_CMD1_ADDR                   (DA_COMBINE_V5_AP_DSP_CMD_ADDR + 0x004 * 1)
#define DA_COMBINE_V5_CMD2_ADDR                   (DA_COMBINE_V5_AP_DSP_CMD_ADDR + 0x004 * 2)
#define DA_COMBINE_V5_CMD3_ADDR                   (DA_COMBINE_V5_AP_DSP_CMD_ADDR + 0x004 * 3)

#define DA_COMBINE_V5_DSP_DSPIF_CLK_EN            (DA_COMBINE_DSP_SCTRL_BASE + 0x0F0)
#define DA_COMBINE_V5_DSP_CMD_STAT_VLD            (DA_COMBINE_DSP_SCTRL_BASE + 0x00C)
#define DA_COMBINE_V5_DSP_CMD_STAT_VLD_OFFSET     (0)

#endif
