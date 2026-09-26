/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#ifndef _AT_OUTPUT_PROC_H_
#define _AT_OUTPUT_PROC_H_

#include "vos.h"
#include "taf_type_def.h"
#include "taf_agent_interface.h"
#include "at_ctx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_VOID   AT_DisplayResultData(VOS_UINT8 indexNum, VOS_UINT16 len);
VOS_VOID   AT_DisplaySelResultData(VOS_UINT16 len, VOS_UINT8 count);
VOS_VOID   At_BufferorSendResultData(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 length);
TAF_VOID   At_SendResultData(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT16 len);
VOS_UINT32 AT_GetReturnCodeId(VOS_UINT32 returnCode, VOS_UINT32 *indexNum);

TAF_VOID At_FormatResultData(TAF_UINT8 indexNum, TAF_UINT32 returnCode);
/*
 * 功能描述: 获取GSM模式下行能力等级在速率显示表中的索引
 */
VOS_UINT32 AT_GetRateDisplayIndexForGsm(TAF_AGENT_SysMode *sysMode);
/*
 * 功能描述: 获取WCDMA模式下行能力等级在速率显示表中的索引
 */
VOS_UINT32 AT_GetRateDisplayIndexForWcdma(AT_DOWNLINK_RateCategory *dlRateCategory);
#if (FEATURE_AT_HSUART == FEATURE_ON)
VOS_UINT32           AT_IsSmsRingingTe(VOS_VOID);
VOS_VOID             AT_SmsRingOn(VOS_VOID);
VOS_VOID             AT_SmsRingOff(VOS_VOID);
VOS_VOID             AT_SmsStartRingTe(VOS_UINT32 newSmsFlg);
VOS_VOID             AT_SmsStopRingTe(VOS_VOID);
VOS_VOID             AT_RcvTiSmsRiExpired(REL_TimerMsgBlock *tmrMsg);
VOS_UINT32           AT_IsVoiceRingingTe(VOS_VOID);
AT_UartRiStatusUint8 AT_GetRiStatusByCallId(VOS_UINT8 callId);
VOS_VOID             AT_VoiceRingOn(VOS_UINT8 callId);
VOS_VOID             AT_VoiceRingOff(VOS_UINT8 callId);
VOS_VOID             AT_VoiceStartRingTe(VOS_UINT8 callId);
VOS_VOID             AT_VoiceStopRingTe(VOS_UINT8 callId);
VOS_VOID             AT_RcvTiVoiceRiExpired(REL_TimerMsgBlock *tmrMsg);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_output_proc.h */
