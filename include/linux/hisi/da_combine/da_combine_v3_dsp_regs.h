/*
 * da_combine_v3_dsp_regs.h
 *
 * da_combine_v3 codec driver
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.All rights reserved.
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

#ifndef __DA_COMBINE_V3_DSP_REGS_H__
#define __DA_COMBINE_V3_DSP_REGS_H__

#include "da_combine_dsp_regs.h"

/* |~0x10000000~~|~~0x10010000~~~|~0x10010080~|~~0x100100c0~~~|~0x10010140~|~0x10011140~~|~~0x10011340~~| */
/* |~ring buffer~|~ap to dsp msg~|~ap dsp cmd~|~dsp to ap msg~|~mlib para~~|~panic stack~|~dump cpuview~| */
/* |~~~~~64k~~~~~|~~~128 byte~~~~|~~~64 byte~~|~~~128 byte~~~~|~~~~~4k~~~~~|~~~512 byte~~|~~~512 byte~~~| */
#define DA_COMBINE_V3_OCRAM_BASE_ADDR             0x10000000
#define DA_COMBINE_V3_ITCM_BASE_ADDR              0x08000000
#define DA_COMBINE_V3_DTCM_BASE_ADDR              0x08010000

#define DA_COMBINE_V3_OCRAM_SIZE                  0x58000
#define DA_COMBINE_V3_ITCM_SIZE                   0x5000
#define DA_COMBINE_V3_DTCM_SIZE                   0x20000

#define DA_COMBINE_V3_RINGBUFFER_SIZE             0x10000    /* sample rate:16K resolution:16bit 2s*/
#define DA_COMBINE_V3_AP_TO_DSP_MSG_SIZE          0x80
#define DA_COMBINE_V3_AP_DSP_CMD_SIZE             0x40      /* CMD size */
#define DA_COMBINE_V3_DSP_TO_AP_MSG_SIZE          0x80
#define DA_COMBINE_V3_MLIB_PARA_MAX_SIZE          0x1000
#define DA_COMBINE_V3_DUMP_PANIC_STACK_SIZE       0x200      /* store panic static info */
#define DA_COMBINE_V3_DUMP_CPUVIEW_SIZE           0x200      /* store cpuview info */
#define DA_COMBINE_V3_SAVE_LOG_SIZE               0x800
#define DA_COMBINE_V3_SAVE_LOG_SIZE_ES            0x4000

#define DA_COMBINE_V3_OCRAM1_BASE                 0x10046340
#define DA_COMBINE_V3_MLIB_TO_AP_MSG_ADDR         (DA_COMBINE_V3_OCRAM1_BASE + 0x19874)
#define DA_COMBINE_V3_MLIB_TO_AP_MSG_SIZE         588

#define DA_COMBINE_V3_RINGBUFFER_ADDR             DA_COMBINE_V3_OCRAM_BASE_ADDR
#define DA_COMBINE_V3_AP_TO_DSP_MSG_ADDR          (DA_COMBINE_V3_RINGBUFFER_ADDR + DA_COMBINE_V3_RINGBUFFER_SIZE)
#define DA_COMBINE_V3_AP_DSP_CMD_ADDR             (DA_COMBINE_V3_AP_TO_DSP_MSG_ADDR + DA_COMBINE_V3_AP_TO_DSP_MSG_SIZE)
#define DA_COMBINE_V3_DSP_TO_AP_MSG_ADDR          (DA_COMBINE_V3_AP_DSP_CMD_ADDR + DA_COMBINE_V3_AP_DSP_CMD_SIZE)
#define DA_COMBINE_V3_MLIBPARA_ADDR               (DA_COMBINE_V3_DSP_TO_AP_MSG_ADDR + DA_COMBINE_V3_DSP_TO_AP_MSG_SIZE)
#define DA_COMBINE_V3_DUMP_PANIC_STACK_ADDR       (DA_COMBINE_V3_MLIBPARA_ADDR + DA_COMBINE_V3_MLIB_PARA_MAX_SIZE)
#define DA_COMBINE_V3_DUMP_CPUVIEW_ADDR           (DA_COMBINE_V3_DUMP_PANIC_STACK_ADDR + DA_COMBINE_V3_DUMP_PANIC_STACK_SIZE)
#define DA_COMBINE_V3_DSP_MSG_STATE_ADDR          (DA_COMBINE_V3_DUMP_PANIC_STACK_ADDR + 0x18)
#define DA_COMBINE_V3_SAVE_LOG_ADDR               0x0802f800
#define DA_COMBINE_V3_SAVE_LOG_ADDR_ES            0x10058000

#define DA_COMBINE_V3_CMD0_ADDR                   (DA_COMBINE_V3_AP_DSP_CMD_ADDR + 0x004 * 0)
#define DA_COMBINE_V3_CMD1_ADDR                   (DA_COMBINE_V3_AP_DSP_CMD_ADDR + 0x004 * 1)
#define DA_COMBINE_V3_CMD2_ADDR                   (DA_COMBINE_V3_AP_DSP_CMD_ADDR + 0x004 * 2)
#define DA_COMBINE_V3_CMD3_ADDR                   (DA_COMBINE_V3_AP_DSP_CMD_ADDR + 0x004 * 3)
#define DA_COMBINE_V3_CMD4_ADDR                   (DA_COMBINE_V3_AP_DSP_CMD_ADDR + 0x004 * 4)

/* regs */
#define DA_COMBINE_V3_DSP_I2S_DSPIF_CLK_EN        (DA_COMBINE_DSP_SCTRL_BASE + 0x0F0)
#define DA_COMBINE_V3_DSP_CMD_STAT_VLD            (DA_COMBINE_DSP_SCTRL_BASE + 0x00C)

#define DA_COMBINE_V3_DSP_S1_CTRL_L               (DA_COMBINE_AUDIO_SUB_BASE + 0xAE)
#define DA_COMBINE_V3_DSP_S1_CTRL_H               (DA_COMBINE_AUDIO_SUB_BASE + 0xAF)

#define DA_COMBINE_V3_DSP_S3_CTRL_L               (DA_COMBINE_AUDIO_SUB_BASE + 0xB2)
#define DA_COMBINE_V3_DSP_S3_CTRL_H               (DA_COMBINE_AUDIO_SUB_BASE + 0xB3)

#define DA_COMBINE_V3_SLIM_CTRL_3                 (DA_COMBINE_AUDIO_SUB_BASE + 0xC6)
#define DA_COMBINE_V3_SLIM_CTRL_5                 (DA_COMBINE_AUDIO_SUB_BASE + 0xC8)

#define DA_COMBINE_V3_SLIM_UP_EN1                 (DA_COMBINE_AUDIO_SUB_BASE + 0x1FE)

#define DA_COMBINE_V3_SC_CODEC_MUX_SEL3_1         (DA_COMBINE_AUDIO_SUB_BASE + 0x15)
#define DA_COMBINE_V3_SC_S1_SRC_LR_CTRL_M         (DA_COMBINE_AUDIO_SUB_BASE + 0x2D)
#define DA_COMBINE_V3_SC_S2_SRC_LR_CTRL_M         (DA_COMBINE_AUDIO_SUB_BASE + 0x3C)
#define DA_COMBINE_V3_SC_S3_SRC_LR_CTRL_M         (DA_COMBINE_AUDIO_SUB_BASE + 0x4B)
#define DA_COMBINE_V3_SC_S4_SRC_LR_CTRL_M         (DA_COMBINE_AUDIO_SUB_BASE + 0x4D)

#define DA_COMBINE_V3_I2S_DSPIF_CLK_EN            (DA_COMBINE_CFG_SUB_BASE + 0x045)
#define DA_COMBINE_V3_MAD_BUF_CLK_EN_BIT          6

#define DA_COMBINE_V3_DSP_S2_CTRL_L               (DA_COMBINE_AUDIO_SUB_BASE + 0xB0)
#define DA_COMBINE_V3_DSP_S2_CLK_EN_BIT           3

#endif

