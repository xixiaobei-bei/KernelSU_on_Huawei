/*
 * da_combine_dsp_regs.h
 *
 * da_combine codec register
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#ifndef __DA_COMBINE_DSP_REGS_H__
#define __DA_COMBINE_DSP_REGS_H__

#include <soc_acpu_baseaddr_interface.h>
#include "da_combine_regs.h"

#define IMAGE_DOWN_MEM_SIZE                 524288
#define IMAGEDOWN_SIZE_THRESH               1000

#define SLIMBUS_PORT12_ADDR                 SOC_ACPU_SLIMBUS_BASE_ADDR + 0x1300
#define ASP_DMAC_BASE_ADDR                  SOC_ACPU_ASP_DMAC_BASE_ADDR

#define DA_COMBINE_DSP_IF6_ADDR                 0x20016000
#define DA_COMBINE_DMA_CONFIG_OFFSET            0x40
#define DA_COMBINE_DMA_STATUS_OFFSET            0x10

#define DA_COMBINE_DSP_EDMA_BASE                0x20000000
#define DA_COMBINE_DSP_IOSHARE_BASE             0x20001000
#define DA_COMBINE_DSP_WATCHDOG_BASE            0x20002000
#define DA_COMBINE_DSP_SCTRL_BASE               0x20003000
#define DA_COMBINE_DSP_UART_BASE                0x20004000
#define DA_COMBINE_CFG_SUB_BASE                 0x20007000
#define DA_COMBINE_AUDIO_SUB_BASE               0x20007200
#define DA_COMBINE_GPIO0_BASE                   0x20008000
#define DA_COMBINE_MSG_START_ADDR_OFFSET        0xA000

#define DA_COMBINE_CODE_CORE_BASE               (DA_COMBINE_CFG_SUB_BASE + 0x200)

#define UCOM_PROTECT_WORD                   0x5A5A5A5A

#define DA_COMBINE_1BYTE_SUB_START              0x20007000
#define DA_COMBINE_1BYTE_SUB_END                0x20007FFF

#define DA_COMBINE_DMA_CH_STAT                  (DA_COMBINE_DSP_EDMA_BASE + 0x690)
#define DA_COMBINE_CX_CURR_CNT0(ch)             (DA_COMBINE_DSP_EDMA_BASE + 0x704 + 0x10 * (ch))
#define DA_COMBINE_CX_CURR_SRC_ADDR(ch)         (DA_COMBINE_DSP_EDMA_BASE + 0x708 + 0x10 * (ch))
#define DA_COMBINE_CX_CURR_DES_ADDR(ch)         (DA_COMBINE_DSP_EDMA_BASE + 0x70C + 0x10 * (ch))

#define DA_COMBINE_CX_CNT0(ch)                  (DA_COMBINE_DSP_EDMA_BASE + 0x810 + 0x40 * (ch))
#define DA_COMBINE_CX_SRC_ADDR(ch)              (DA_COMBINE_DSP_EDMA_BASE + 0x814 + 0x40 * (ch))
#define DA_COMBINE_CX_DES_ADDR(ch)              (DA_COMBINE_DSP_EDMA_BASE + 0x818 + 0x40 * (ch))
#define DA_COMBINE_CX_CONFIG(ch)                (DA_COMBINE_DSP_EDMA_BASE + 0x81C + 0x40 * (ch))

#define DA_COMBINE_DSP_WATCHDOG_LOCK            (DA_COMBINE_DSP_WATCHDOG_BASE + 0xC00)
#define DA_COMBINE_DSP_WATCHDOG_CONTROL         (DA_COMBINE_DSP_WATCHDOG_BASE + 0x008)
#define DA_COMBINE_DSP_WATCHDOG_INTCLR          (DA_COMBINE_DSP_WATCHDOG_BASE + 0x00C)

#define DA_COMBINE_DSP_WATCHDOG_LOCK_WORD       0x0
#define DA_COMBINE_DSP_WATCHDOG_UNLOCK_WORD     0x1ACCE551
#define DA_COMBINE_DSP_WATCHDOG_CONTROL_DISABLE 0x0
#define DA_COMBINE_DSP_WATCHDOG_CONTROL_ENABLE  0x3
#define DA_COMBINE_DSP_WATCHDOG_INTCLR_WORD     0x4455

#define DA_COMBINE_DSP_MSG_BIT                  0
#define DA_COMBINE_DSP_PLLSWITCH_BIT            1
#define DA_COMBINE_DSP_POWERON_BIT              2
#define DA_COMBINE_DSP_MSG_WITH_CONTENT_BIT     3

#define DA_COMBINE_DSP_SUB_CMD_STATUS           (DA_COMBINE_DSP_SCTRL_BASE + 0x008)
#define DA_COMBINE_DSP_SUB_CMD_STATUS_VLD       (DA_COMBINE_DSP_SCTRL_BASE + 0x00C)

/* 0x20007000 ~ 0x20007037 */
#define DA_COMBINE_DUMP_CFG_SUB_ADDR1           DA_COMBINE_CFG_SUB_BASE
#define DA_COMBINE_DUMP_CFG_SUB_SIZE1           0x38
/* 0x2000703A ~ 0x200070A0 */
#define DA_COMBINE_DUMP_CFG_SUB_ADDR2           (DA_COMBINE_CFG_SUB_BASE + 0x3A)
#define DA_COMBINE_DUMP_CFG_SUB_SIZE2           0x67
/* 0x20007200 ~ 0x200073FF */
#define DA_COMBINE_DUMP_AUDIO_SUB_ADDR          DA_COMBINE_AUDIO_SUB_BASE
#define DA_COMBINE_DUMP_AUDIO_SUB_SIZE          0x200
/* 0x20000000 ~ 0x20000028 */
#define DA_COMBINE_DUMP_DSP_EDMA_ADDR1          DA_COMBINE_DSP_EDMA_BASE
#define DA_COMBINE_DUMP_DSP_EDMA_SIZE1          0x2C
/* 0x20000600 ~ 0x2000069C */
#define DA_COMBINE_DUMP_DSP_EDMA_ADDR2          (DA_COMBINE_DSP_EDMA_BASE + 0x600)
#define DA_COMBINE_DUMP_DSP_EDMA_SIZE2          0xA0
/* 0x20000700 ~ 0x20000BDC */
#define DA_COMBINE_DUMP_DSP_EDMA_ADDR3          (DA_COMBINE_DSP_EDMA_BASE + 0x700)
#define DA_COMBINE_DUMP_DSP_EDMA_SIZE3          0x4E0
/* 0x20002000 ~ 0x20002008 */
#define DA_COMBINE_DUMP_DSP_WATCHDOG_ADDR1      DA_COMBINE_DSP_WATCHDOG_BASE
#define DA_COMBINE_DUMP_DSP_WATCHDOG_SIZE1      0xC
/* 0x20002010 ~ 0x20002014 */
#define DA_COMBINE_DUMP_DSP_WATCHDOG_ADDR2      (DA_COMBINE_DSP_WATCHDOG_BASE + 0x10)
#define DA_COMBINE_DUMP_DSP_WATCHDOG_SIZE2      0x8
/* 0x20003000 ~ 0x20003018 */
#define DA_COMBINE_DUMP_DSP_SCTRL_ADDR1         DA_COMBINE_DSP_SCTRL_BASE
#define DA_COMBINE_DUMP_DSP_SCTRL_SIZE1         0x1C
/* 0x20003100 ~ 0x2000311C */
#define DA_COMBINE_DUMP_DSP_SCTRL_ADDR2         (DA_COMBINE_DSP_SCTRL_BASE + 0x100)
#define DA_COMBINE_DUMP_DSP_SCTRL_SIZE2         0x20
/* 0x20007038 ~ 0x20007039 */
#define DA_COMBINE_DUMP_CFG_SUB_ADDR3           (DA_COMBINE_CFG_SUB_BASE + 0x38)
#define DA_COMBINE_DUMP_CFG_SUB_SIZE3           0x2

#endif/*__DA_COMBINE_DSP_REGS_H__*/

