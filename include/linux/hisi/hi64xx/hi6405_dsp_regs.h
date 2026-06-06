/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#ifndef __HI6405_DSP_REGS_H__
#define __HI6405_DSP_REGS_H__

#include "hi64xx_dsp_regs.h"

#define HI6405_OCRAM_BASE_ADDR             (0x10100000)
#define HI6405_ITCM_BASE_ADDR              (0x08000000)
#define HI6405_DTCM_BASE_ADDR              (0x08010000)

#define HI6405_OCRAM_SIZE                  (0x58000)
#define HI6405_ITCM_SIZE                   (0x5000)
#define HI6405_DTCM_SIZE                   (0x20000)

#define HI6405_RINGBUFFER_SIZE             (0x10000)    /* sample rate:16K resolution:16bit 2s*/
#define HI6405_AP_TO_DSP_MSG_SIZE          (0x80)
#define HI6405_AP_DSP_CMD_SIZE             (0x40)
#define HI6405_DSP_TO_AP_MSG_SIZE          (0x80)
#define HI6405_MLIB_PARA_MAX_SIZE          (0x1000)
#define HI6405_DUMP_PANIC_STACK_SIZE       (0x200)
#define HI6405_DUMP_CPUVIEW_SIZE           (0x200)
#define HI6405_SAVE_LOG_SIZE               (0x800)
#define HI6405_SAVE_LOG_SIZE_ES            (0x4000)

#define HI6405_OCRAM1_BASE                  0x10146340
#define HI6405_MLIB_TO_AP_MSG_ADDR          (HI6405_OCRAM1_BASE + 0x19874)
#define HI6405_MLIB_TO_AP_MSG_SIZE          (588)

#define HI6405_RINGBUFFER_ADDR             (HI6405_OCRAM_BASE_ADDR)
#define HI6405_AP_TO_DSP_MSG_ADDR          (HI6405_RINGBUFFER_ADDR + HI6405_RINGBUFFER_SIZE)
#define HI6405_AP_DSP_CMD_ADDR             (HI6405_AP_TO_DSP_MSG_ADDR + HI6405_AP_TO_DSP_MSG_SIZE)
#define HI6405_DSP_TO_AP_MSG_ADDR          (HI6405_AP_DSP_CMD_ADDR + HI6405_AP_DSP_CMD_SIZE)
#define HI6405_MLIBPARA_ADDR               (HI6405_DSP_TO_AP_MSG_ADDR + HI6405_DSP_TO_AP_MSG_SIZE)
#define HI6405_DUMP_PANIC_STACK_ADDR       (HI6405_MLIBPARA_ADDR + HI6405_MLIB_PARA_MAX_SIZE)
#define HI6405_DUMP_CPUVIEW_ADDR           (HI6405_DUMP_PANIC_STACK_ADDR + HI6405_DUMP_PANIC_STACK_SIZE)
#define HI6405_DSP_MSG_STATE_ADDR          (HI6405_DUMP_PANIC_STACK_ADDR + 0x18)
#define HI6405_SAVE_LOG_ADDR               (0x0802f800)

#define HI6405_CMD0_ADDR                   (HI6405_AP_DSP_CMD_ADDR + 0x004 * 0)
#define HI6405_CMD1_ADDR                   (HI6405_AP_DSP_CMD_ADDR + 0x004 * 1)
#define HI6405_CMD2_ADDR                   (HI6405_AP_DSP_CMD_ADDR + 0x004 * 2)
#define HI6405_CMD3_ADDR                   (HI6405_AP_DSP_CMD_ADDR + 0x004 * 3)

#define HI6405_DSP_DSPIF_CLK_EN            (HI64xx_DSP_SCTRL_BASE + 0x0F0)
#define HI6405_DSP_CMD_STAT_VLD            (HI64xx_DSP_SCTRL_BASE + 0x00C)
#define HI6405_DSP_CMD_STAT_VLD_OFFSET     (0)

#endif
