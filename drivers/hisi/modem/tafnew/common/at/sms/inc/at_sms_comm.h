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
#ifndef __ATSMSCOMM_H__
#define __ATSMSCOMM_H__

#include "vos.h"
#include "mn_msg_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

enum {
    AT_MSG_SERV_STATE_NOT_SUPPORT,
    AT_MSG_SERV_STATE_SUPPORT
};
typedef VOS_UINT8 AT_MSG_ServStateUint8;

typedef struct {
    AT_MSG_ServStateUint8 smsMT;
    AT_MSG_ServStateUint8 smsMO;
    AT_MSG_ServStateUint8 smsBM;
} AT_MSG_Serv;

#define AT_GET_CMS_SMS_ERR_CODE_MAP_TBL_PTR() (g_atCmsSmsErrCodeMapTbl)
#define AT_GET_CMS_SMS_ERR_CODE_MAP_TBL_SIZE() (sizeof(g_atCmsSmsErrCodeMapTbl) / sizeof(AT_CMS_SmsErrCodeMap))

extern MN_MSG_SendAckParm* At_GetAckMsgMem(VOS_VOID);
extern MN_MSG_TsDataInfo* At_GetMsgMem(VOS_VOID);
VOS_UINT32 At_GetSmsStorage(VOS_UINT8 indexNum, MN_MSG_MemStoreUint8 memReadorDelete,
                            MN_MSG_MemStoreUint8 memSendorWrite, MN_MSG_MemStoreUint8 memRcv);

TAF_VOID AT_StubClearSpecificAutoRelyMsg(VOS_UINT8 clientIndex, TAF_UINT32 bufferIndex);
extern TAF_UINT32 At_ChgMnErrCodeToAt(TAF_UINT8 indexNum, TAF_UINT32 mnErrorCode);
extern TAF_VOID At_PrintCsmsInfo(TAF_UINT8 indexNum);
extern TAF_UINT32 At_MsgDeleteCmdProc(TAF_UINT8 indexNum, MN_OPERATION_ID_T opId, MN_MSG_DeleteParam deleteInfo,
                                      TAF_UINT32 deleteTypes);
extern TAF_UINT16 At_MsgPrintVp(MN_MSG_ValidPeriod *validPeriod, TAF_UINT8 *dst);
extern TAF_UINT32 At_SmsPrintScts(const MN_MSG_Timestamp *timeStamp, TAF_UINT8 *dst);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
