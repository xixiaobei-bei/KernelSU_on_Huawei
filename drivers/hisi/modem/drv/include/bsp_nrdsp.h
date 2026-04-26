/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __BSP_NRDSP_H__
#define __BSP_NRDSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <product_config.h>
#include <osl_types.h>
#include <mdrv_nrdsp.h>

struct nrdsp_sect_desc
{
    u8                               usNo;                   /* 段序号 */
    u8                               ucCrcOriginal;          /* 段的原始CRC校验和 */
    u8                               ucCrc;                  /* 复位时的CRC校验和 */
    u8                               ucTcmType;              /* 段类型: 0-代码(text); 1-rodata; 2-data; 3-bss */
    u8                               ucCoreMask;             /* 加载到哪个NX核(数值bit为1代表对应核上有此段，取值0x01,0x04,0x08,0x10,0x20,0x40,0x80，对应8个NX) */
    u8                               ucLoadType;             /* 加载类型，0表示不需要加载， 1表示上电只需要加载一次， 2表示每次需要加载 */
    u8                               ucStoreType;            /* 保存类型，0表示不需要备份，1表示下电时只需要备份一次，2表示每次都需要备份 */
    u8                               ucSectNeedStore;        /* 系统异常时，段是否需要保存标志，0表示段不需要保存，1表示段需要保存，默认是1 */
    u32                              ulTargetAddr;           /* 加载的目标地址 */
    u32                              ulExecuteAddr;          /* 运行地址 */
    u32                              ulSectSize;             /* 段的大小 */
    u32                              ulFileOffset;           /* 段在文件内的偏移 */
    u8                               ucRsv[8];
};

/* NR物理层镜像头 */
struct nrdsp_bin_header
{
    s8                          acDescription[24];      /* 由工具生成，内容为处理器标记和日期、时间 */
    u32                         ulFileSize;             /* 文件大小 */
    u32                         ulSectNum;              /* 段个数 */
    u8                          ucCrashCore[8];         /* 0:HL1C, 1~4:LL1C DL, 5~6:LL1C UL, 7:SDR */
    u8                          ucDrxFlag;              /* 镜像是否在低功耗下备份，1代表是，0代表否(运行状态)，避免通过不同文件名区分 */
    u8                          ucRsv[23];
    struct nrdsp_sect_desc      astSect[0];             /* 段信息 */
};

struct nrdsp_dump_flag
{
    unsigned int dsp_dump_flag;         /* 异常流程中，记录dsp保存tcm的过程标志 */
};

typedef enum{
    NRDSP_NORMAL = 0,
    NRDSP_WAITI = 1,
}NRDSP_WAIT_MODE;

#if !defined(CONFIG_NRDSP) && !defined(CONFIG_NRDSP_NRFPGA)
static inline void bsp_nrdsp_dump_tcm(char * dst_path)
{
    return;
}
#else
#ifdef __KERNEL__
void bsp_nrdsp_dump_tcm(const char * dst_path);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
