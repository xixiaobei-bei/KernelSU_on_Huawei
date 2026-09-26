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

#ifndef _CBTSCM_SOFTDECODE_H_
#define _CBTSCM_SOFTDECODE_H_

#include "vos.h"
#include "om_ring_buffer.h"

#ifdef MODEM_FUSION_VERSION
#include "mdrv_diag.h"
#else
#include "msp_diag_comm.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#if defined(PAM_CT_ENABLED) || defined(PAM_BT_ENABLED) || defined(CBT_ENABLED)

#define CBTSCM_DATA_RCV_BUFFER_SIZE (100 * 1024) /* SCM数据接收buffer大小 */
#define CBTSCM_DATA_RCV_PKT_SIZE (8 * 1024)      /* SCM数据接收PKT大小 */

#define CBT_EN_DE_MSG 0x5a5a5a5a

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    VOS_UINT32 dataLen;
    VOS_UINT8 data[0];
}CBT_EnDeMsg;

/*
 * 功能说明: CBTSCM数据接收的控制结构
 */
typedef struct {
    VOS_SEM    smId;
    OM_RING_ID rngOmRbufId;
    VOS_CHAR  *buffer;
} CBTSCM_DataRcvCtrl;

/*
 * 功能说明: CBTSCM软解码模块可维可测信息记录结构
 */
typedef struct {
    VOS_UINT32 dataLen; /* 接收或发送数据长度 */
    VOS_UINT32 num;     /* 接收或发送数据次数 */
} CBTSCM_MaintenanceInfo;

typedef struct {
    VOS_UINT32 semCreatErr;
    VOS_UINT32 semGiveErr;
    VOS_UINT32 ringBufferCreatErr;
    VOS_UINT32 taskIdErr;
    VOS_UINT32 bufferNotEnough;
    VOS_UINT32 ringBufferFlush;
    VOS_UINT32 ringBufferPutErr;
} CBTSCM_SoftdecodeRbInfo;

/*
 * 功能说明: OM模块正常接收可维可测信息记录结构
 */
typedef struct {
    CBTSCM_SoftdecodeRbInfo   rbInfo;
    CBTSCM_MaintenanceInfo    putInfo;
    CBTSCM_MaintenanceInfo    getInfo;
    CBTSCM_MaintenanceInfo    hdlcDecapData;
} CBTSCM_SoftdecodeInfo;

VOS_UINT32 CBTSCM_SoftDecodeDataRcv(VOS_UINT8 *buffer, VOS_UINT32 len);
VOS_UINT32 CBTSCM_SoftDecodeReqRcvTaskInit(VOS_VOID);

#if ((FEATURE_ON == FEATURE_PHONE_SC) && (FEATURE_ON == FEATURE_SET_C1C2_VALUE))
    VOS_UINT32 pmu_dcxo_set(VOS_UINT16 dcxoCtrim, VOS_UINT16 dcxoC2Fix);
#endif

#endif /* end if CBT_ENABLED */

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
