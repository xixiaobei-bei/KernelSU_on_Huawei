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
#ifndef __ATSMSEVENTREPORT_H__
#define __ATSMSEVENTREPORT_H__
#include "at_ctx.h"
#include "AtParse.h"
#include "at_mn_interface.h"
#include "mn_msg_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
typedef TAF_VOID (*AT_SMS_RSP_PROC_FUN)(TAF_UINT8 ucIndex, MN_MSG_EventInfo *pstEvent);
typedef struct {
    TAF_MSG_ErrorUint32  msgErrorCode;
    AT_RreturnCodeUint32 atErrorCode;
} AT_SMS_ErrorCodeMap;

/* 23040 9.2.3.1 bits no 0 and 1 of the first octet of all PDUs */
#define AT_SET_MSG_TP_MTI(fo, ucMti) ((fo) |= ((ucMti) & 0x03))
/* 23040 9.2.3.25  1 bit field located within bit 2 of the first octet of SMS SUBMIT */
#define AT_SET_MSG_TP_RD(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0x04 : 0))
/* 23040 9.2.3.2 bit no 2 of the first octet of SMS DELIVER and SMS STATUS REPORT */
#define AT_SET_MSG_TP_MMS(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0 : 0x04))
/* 23040 9.2.3.3 bit no 3 and 4 of the first octet of SMS SUBMIT */
#define AT_SET_MSG_TP_VPF(fo, ucVpf) ((fo) |= (((ucVpf) << 3) & 0x18))
/* 23040 9.2.3.26 bit no. 5 of the first octet of SMS STATUS REPORT */
#define AT_SET_MSG_TP_SRQ(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0x20 : 0))
/* 23040 9.2.3.4 bit no. 5 of the first octet of SMS DELIVER */
#define AT_SET_MSG_TP_SRI(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0x20 : 0))
/* 23040 9.2.3.5 bit no. 5 of the first octet of SMS SUBMIT and SMS COMMAND */
#define AT_SET_MSG_TP_SRR(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0x20 : 0))
/* 23040 9.2.3.23 1 bit field within bit 6 of the first octet of SMS SUBMIT SMS-SUBMIT-REPORT SMS
 * DELIVER,SMS-DELIVER-REPORT SMS-STATUS-REPORT SMS-COMMAND. */
#define AT_SET_MSG_TP_UDHI(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0x40 : 0))
/* 23040 9.2.3.17 1 bit field, located within bit no 7 of the first octet of both SMS DELIVER and SMS SUBMIT, */
#define AT_SET_MSG_TP_RP(fo, bFlag) ((fo) |= (VOS_UINT8)((TAF_TRUE == (bFlag)) ? 0x80 : 0))

extern TAF_VOID   At_SmsMsgProc(MN_AT_IndEvt *data, TAF_UINT16 len);
extern VOS_VOID At_SmsDeliverProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_SetRcvPathRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_SetCscaCsmpRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_DeleteTestRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_ReadRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_ListRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_WriteSmRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_DeleteRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_SetCnmaRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_SendSmRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_SetCmmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_SetCgsmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_GetCmmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID At_GetCgsmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern VOS_VOID AT_QryCscaRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
extern VOS_VOID   At_SmsStubRspProc(VOS_UINT8 indexNum, MN_MSG_EventInfo *event);
extern TAF_VOID   At_SmsRspNop(TAF_UINT8 indexNum, MN_MSG_EventInfo *event);
VOS_UINT16 AT_PrintSmsLength(MN_MSG_MsgCodingUint8 msgCoding, VOS_UINT32 length, TAF_UINT8 *dst);
extern TAF_UINT32 At_PrintListMsg(VOS_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo,
                                  TAF_UINT8 *dst);
TAF_VOID At_ForwardMsgToTe(MN_MSG_EventUint32 event, MN_MSG_EventInfo *eventOutPara);
extern TAF_VOID AT_StubSaveAutoReplyData(VOS_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo);
extern TAF_VOID At_MsgResultCodeFormat(TAF_UINT8 indexNum, TAF_UINT16 length);
extern TAF_UINT16 At_PrintMsgFo(MN_MSG_TsDataInfo *tsDataInfo, TAF_UINT8 *dst);
extern TAF_UINT16 At_PrintAsciiAddr(MN_MSG_AsciiAddr *addr, TAF_UINT8 *dst);
extern TAF_UINT32 At_ReadNumTypePara(TAF_UINT8 *dst, TAF_UINT8 *src);
extern TAF_UINT16 At_PrintAddrType(MN_MSG_AsciiAddr *addr, TAF_UINT8 *dst);
extern TAF_VOID At_PrintSetCpmsRsp(TAF_UINT8 indexNum);
extern VOS_VOID At_PrintGetCpmsRsp(VOS_UINT8 indexNum);

extern TAF_UINT32 At_SmsPrintState(AT_CmgfMsgFormatUint8 smsFormat, MN_MSG_StatusTypeUint8 status, TAF_UINT8 *dst);


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
