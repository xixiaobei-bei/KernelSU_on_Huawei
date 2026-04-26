/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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



#ifndef PS_LOG_FILE_ID_BASE_H
#define PS_LOG_FILE_ID_BASE_H

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* 模块ID的范围定义*/
#define     LOG_ERRLOG_ID                   0x63

#define     LOG_MIN_MODULE_ID_PS            0x64
#define     LOG_MAX_MODULE_ID_PS            0xff

#define     LOG_MIN_MODULE_ID_ACPU_PS       0x100
#define     LOG_MAX_MODULE_ID_ACPU_PS       0x1ff

#define     LOG_MIN_MODULE_ID_DSP           0x200
#define     LOG_MAX_MODULE_ID_DSP           0x2ff

#define     LOG_MIN_MODULE_ID_HIFI          0x300
#define     LOG_MAX_MODULE_ID_HIFI          0x3ff

#define     LOG_MIN_MODULE_ID_DRV           0x8000
#define     LOG_MAX_MODULE_ID_DRV           0x80ff

#define     LOG_MIN_MODULE_ID_ACPU_DRV      0x8100
#define     LOG_MAX_MODULE_ID_ACPU_DRV      0x81ff

#define     LOG_MIN_MODULE_ID_APP           0x8200
#define     LOG_MAX_MODULE_ID_APP           0x82ff


/* File Id范围定义 */
#define     LOG_MIN_FILE_ID_PS          0x0000
#define     LOG_MAX_FILE_ID_PS          0x07FF

#define     LOG_MIN_FILE_ID_DRV         0x0800
#define     LOG_MAX_FILE_ID_DRV         0x0FFF

#define     LOG_MIN_FILE_ID_MEDIA       0x1000
#define     LOG_MAX_FILE_ID_MEDIA       0x17FF

#define     LOG_MIN_FILE_ID_APP         0x1800
#define     LOG_MAX_FILE_ID_APP         0x1FFF

#define     LOG_MIN_FILE_ID_DSP         0x2000
#define     LOG_MAX_FILE_ID_DSP         0x27FF

#define     LOG_MIN_FILE_ID_MSP         0x2200
#define     LOG_MAX_FILE_ID_MSP         0x29FF

/* 为了降低debug信息，采用新的file id 方案,旧的方案后续需要删除 */
/* 新的组件定义 */
/* 0到9bit file id, 10到15 bit 组件， 高16位保留 */
typedef enum
{
    DRV_TEAM_FILE_ID          = 0x0,
    WAS_TEAM_FILE_ID          = 0x400,
    GAS_TEAM_FILE_ID          = 0x800,
    CAS_TEAM_FILE_ID          = 0xc00,
    CPROC_TEAM_FILE_ID        = 0x1000,
    WTTF_TEAM_FILE_ID         = 0x1400,
    GTTF_TEAM_FILE_ID         = 0x1800,
    CTTF_TEAM_FILE_ID         = 0x1c00,
    TTFCOMM_TEAM_FILE_ID      = 0x2000,
    TRRC_TEAM_FILE_ID         = 0x2400,
    LRRC_TEAM_FILE_ID         = 0x2800,
    TL2_TEAM_FILE_ID          = 0x2c00,
    LL2_TEAM_FILE_ID          = 0x3000,
    TLPSCOMM_TEAM_FILE_ID     = 0x3400,
    LNAS_TEAM_FILE_ID         = 0x3800,
    NAS_TEAM_FILE_ID          = 0x3c00,
    TAF_TEAM_FILE_ID          = 0x4000,
    CNAS_TEAM_FILE_ID         = 0x4600,
    PAM_TEAM_FILE_ID          = 0x4800,
    MSP_TEAM_FILE_ID          = 0x4c00,
    HIFI_TEAM_FILE_ID         = 0x5000,
    GUDSP_TEAM_FILE_ID        = 0x5400,
    TLDSP_TEAM_FILE_ID        = 0x5800,
    CDSP_TEAM_FILE_ID         = 0x5c00,
    APP_TEAM_FILE_ID          = 0x6000,
    ENCODIX_TEAM_FILE_ID      = 0x6400,
    NRRC1_TEAM_FILE_ID        = 0x6800, /* 原NRRC拆分出: nrrc1对应conn责任田 nrrc2对应idle责任田 */
    NRRC2_TEAM_FILE_ID        = 0x6A00,
    NL2_TEAM_FILE_ID          = 0x6C00,
    NRNAS_TEAM_FILE_ID        = 0x7000,
    VL2_TEAM_FILE_ID          = 0x7400,
    IMSA_TEAM_FILE_ID         = 0x7800,
    VRRC_TEAM_FILE_ID         = 0x7C00,
    BST_TEAM_FILE_ID          = 0x8000,

    /* 后续组件继续添加 */
    TEAM_FILE_ID_BUTT         = 0xfc00
}FILE_ID_TEAM_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


