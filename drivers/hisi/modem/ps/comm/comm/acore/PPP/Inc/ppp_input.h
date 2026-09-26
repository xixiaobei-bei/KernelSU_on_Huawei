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

#ifndef __PPP_INPUT_H__
#define __PPP_INPUT_H__

#include "ppp_public.h"
#include "imm_interface.h"
#include "v_typdef.h"
#include "ppp_internal_msg.h"
#include "hdlc_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define PPP_RCV_DATA_EVENT 0x01

#define PPP_SEND_PRIORITY_DATA_TO_AT(pppId, data, pri) \
    do { \
        PPP_ZC_SET_PRIORITY(data, pri); \
        PPPA_SendDlDataToAT(pppId, data); \
    } while (0)

#define PPP_SEND_NORMAL_PRIORITY_DATA_TO_AT(pppId, data) \
    PPP_SEND_PRIORITY_DATA_TO_AT(pppId, data, PPP_DATA_PRIORITY_NORMAL)


enum PPP_DataDir {
    PPP_DATA_DIR_UPLINK = 0,
    PPP_DATA_DIR_DOWNLINK,
};

typedef struct {
    VOS_MSG_HEADER /* ÏûÏ¢Í· */ /* _H2ASN_Skip */
    VOS_UINT32 msgname;
    VOS_UINT16     pppId;
    VOS_UINT16     reserved;
    VOS_UINT32     pppPhase;
    VOS_INT32      lcpState;
    VOS_INT32      ipcpState;
} PPP_EventMntnInfo;

typedef struct {
    VOS_MSG_HEADER
    PPPA_InternalMsgTypeUint16 msgName;
    VOS_UINT16 rsv;
    PPP_DataQStat stat;
} PPPA_DataQStatReport;

extern VOS_UINT32 PPP_SendUlDataToUu(VOS_UINT16 pppId, PPP_Zc *immZc, VOS_UINT16 proto);
extern VOS_VOID PPPA_SendDlDataToAT(PPP_Id pppId, IMM_Zc *data);
extern VOS_VOID PPP_SetupHdlc(VOS_UINT16 pppId, VOS_BOOL ipMode);
extern VOS_VOID PPP_ReleaseHdlc(VOS_UINT16 pppId);
extern VOS_VOID PPP_ClearDataQ(VOS_VOID);
extern VOS_UINT32 PPP_DataQInit(VOS_VOID);
extern VOS_VOID PPP_ProcDataNotify(VOS_VOID);
extern VOS_VOID PPPA_SaveTaskId(VOS_UINT32 taskId);
extern VOS_VOID PPPA_SetUsedFlag(VOS_BOOL isUsed);
extern VOS_BOOL PPPA_GetUsedFlag(VOS_VOID);
extern VOS_VOID PPPA_DataEventProc(VOS_VOID);
extern VOS_VOID PPP_HDLC_ProcIpModeUlData(VOS_UINT16 pppId, PPP_Zc *mem, VOS_UINT16 proto);
extern VOS_VOID PPPA_InputProcEchoReq(VOS_VOID);
extern VOS_VOID PPPA_SetAtDataSender(VOS_ULONG userData, PPP_SendDataToModem sender);
extern VOS_VOID PPPA_ClearAtDataSender(VOS_VOID);
extern VOS_INT PPP_DlPacketProc(PPP_Id pppId, IMM_Zc *immZc);
extern VOS_INT PPP_DlRawDataProc(PPP_Id pppId, IMM_Zc *immZc);
extern VOS_VOID PPPA_DataQStatReportTimerProc(VOS_VOID);
extern VOS_VOID PPPA_ReportDataQStat(VOS_VOID);
extern VOS_VOID PPPA_StartDataQReportTimer(VOS_VOID);
extern VOS_VOID PPPA_StopDataQReportTimer(VOS_VOID);
extern VOS_VOID PPP_StatInputDataFreeCnt(PPP_DataTypeUint8 dataType, VOS_UINT32 cnt);
extern VOS_VOID PPP_StatOutputAllocMemCnt(enum PPP_DataDir dir, VOS_UINT32 cnt);
extern VOS_VOID PPP_StatOutputFreeCntBeforeSnd(VOS_UINT32 cnt);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
