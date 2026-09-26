/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef __ATLTEVASRSLTPROC_H__
#define __ATLTEVASRSLTPROC_H__

#include "vos.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "taf_v2x_api.h"
#endif
#include "at_mn_interface.h"
#include "AtParse.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_LTEV == FEATURE_ON)
typedef struct {
    VTC_AT_ResultCodeUint32 vtcResultCode;
    AT_RreturnCodeUint32    atReturnCode;
} AT_ConvertVtcResultCodeTbl;

typedef struct {
    VRRC_AT_ResultCodeUint32 vrrcResultCode;
    AT_RreturnCodeUint32     atReturnCode;
} AT_ConvertVrrcResultCodeTbl;

VOS_UINT32 AT_RcvTestModeActiveStateSetCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvTestModeActiveStateQryCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvTestModeModifyL2IdSetCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvTestModeModifyL2IdQryCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvTestModeECloseStateSetCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvTestModeECloseStateQryCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvSidelinkPacketCounterQryCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvResetUtcTimeSetCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvVtcCbrQryCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvSendingDataActionSetCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvSendingDataActionQryCnf(VTC_AT_MsgCnf *msg);
VOS_UINT32 AT_RcvSyncSourceQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvSyncSourceRptSetCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvSyncSourceRptQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvSyncModeQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvSyncModeSetCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvVmacCbrQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvGnssTestStartCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvGnssInfoQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvGnssDebugInfoGetCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvRsuVphyStatQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvRsuVphyStatClrSetCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvRsuVsnrRsrpQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvRsuV2xRssiQryCnf(MN_AT_IndEvt *msg);
VOS_UINT32 AT_RcvVrrcMsgSetTxPowerCnfProc(MN_AT_IndEvt *msg);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
